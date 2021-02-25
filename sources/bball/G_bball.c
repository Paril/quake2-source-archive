#include "g_local.h"
#include "m_player.h"
#include "q_devels.h"

//help function, returns length disregarding height
vec_t FlatLength(vec3_t v)
{
	int		i;
	float	length;
	
	length = 0;
	for (i=0 ; i< 2 ; i++)
		length += v[i]*v[i];
	length = sqrt (length);		// FIXME

	return length;
}

//===============================================================
//
//	Basketball Actions
//
//===============================================================
int PointValue(int team, vec3_t shooter)
{
	//find point value of shot
	edict_t	*rim;
	vec3_t length,point1,point2;

	if (team == TEAM1)
		rim = G_Find (NULL, FOFS(targetname), "rim1");
	else
		rim = G_Find (NULL, FOFS(targetname), "rim2");

	if (!rim)
	{
		gi.dprintf ("Couldn't find rim marker\n");
		return 2;
	}
	else
	{
		VectorSubtract(shooter, rim->s.origin, length);
		
		if (FlatLength(length) >= THREE_DISTANCE)
			return 3;
		else
			return 2;
	}
}

static void Basketball_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int points;
	
	ent->style = NOT_GOOD;	
	
	if (surf && (surf->flags & SURF_SLICK))
	{
		if (surf->value == 1)
		{
			points = PointValue(TEAM1,ent->pos1);
			level.ballteam1 += points;
			if(ent->owner->client)
			{
				gi.bprintf(PRINT_HIGH, "%s scores %i points for the %s\n",ent->owner->client->pers.netname ,points, team1name->string);
				ent->owner->client->resp.score += points;
			}
			else if(ent->owner->bot)
			{
				gi.bprintf(PRINT_HIGH, "%s scores %i points for the %s\n",ent->owner->bot->pers.netname ,points, team1name->string);
				ent->owner->bot->resp.score += points;
			}
			G_FreeEdict (ent);
			Inbound(TEAM2);
			return;
		}
		else if (surf->value == 2)
		{
			points = PointValue(TEAM2,ent->pos1);
			level.ballteam2 += points;
			if(ent->owner->client)
			{
				gi.bprintf(PRINT_HIGH, "%s scores %i points for the %s\n",ent->owner->client->pers.netname ,points, team2name->string);
				ent->owner->client->resp.score += points;
			}
			else if(ent->owner->bot)
			{
				gi.bprintf(PRINT_HIGH, "%s scores %i points for the %s\n",ent->owner->bot->pers.netname ,points, team2name->string);
				ent->owner->bot->resp.score += points;
			}
			G_FreeEdict (ent);
			Inbound(TEAM1);
			return;
		}

		//set last scorer
		level.lastscorer = ent->owner;
	}

	//player rarely will get ball this way
	if(other->inuse && (other->client || other->bot) &&
		other->health > 0 && other->movetype != MOVETYPE_NOCLIP)
	{
		int index = ITEM_INDEX(FindItem("BasketBalls"));
		
		if(other->client)
		{
			if (other->client->pers.inventory[index] == 0)//M.S.Fix use add_ammo?
				other->client->pers.inventory[index]++;//add ball M.S.
		}
		else if(other->bot)
		{
			if (other->bot->pers.inventory[index] == 0)//M.S.Fix use add_ammo?
				other->bot->pers.inventory[index]++;//add ball M.S.
		}
		G_FreeEdict (ent);
	}
	else //we are bouncing off something
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/bball1a.wav"), 1, ATTN_NORM, 0);
}

static void Sitting_Think (edict_t *ent)
{
	edict_t *blip = NULL;
	edict_t *best = NULL;
	vec3_t	vec, start;
	trace_t	tr;
	float	bestdist = PICKUP_RANGE;
	float	playerdist;
	int		i;

	if (!ent->timestamp)
	{
		ent->timestamp = level.time;
		ent->count = 1; //toggle message as ok
	}

	for (i=0 ; i<game.maxclients+game.maxbots ; i++)
	{
		blip = g_edicts + 1 + i;

		if( (!blip->inuse) || (!blip->client && !blip->bot) ||
			(blip->health <= 0) || (blip->movetype == MOVETYPE_NOCLIP))
			continue;

		VectorSubtract(ent->s.origin, blip->s.origin, vec );
		playerdist = VectorLength(vec);
		if (playerdist > bestdist)
			continue;
		else
		{
			best = blip;
			bestdist = playerdist;
		}
	}
		
	//see if ball just left their hands
	if(best && !(best == ent->owner &&
		(level.time - ent->timestamp) < .5))
	{
		//make sure wall isn't between ball and player
		//trace won't hit ent->owner, so don't give ignore ent (ok cause bball has no bbox?)
		tr = gi.trace(ent->s.origin, NULL, NULL, best->s.origin, NULL, MASK_BBALL);
		
		if(tr.ent == best)
		{		
			//give them ball
			if (best->client)
			{
				if (best->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] == 0)//M.S.Fix use add_ammo?
					best->client->pers.inventory[ITEM_INDEX(FindItem("Basketballs"))]++;//add ball M.S.
			}
			else if(best->bot)
			{
				if (best->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] == 0)//M.S.Fix use add_ammo?
					best->bot->pers.inventory[ITEM_INDEX(FindItem("Basketballs"))]++;//add ball M.S.
				//set next action time
				best->bot->action_time = level.time + .2;
			}

			G_FreeEdict (ent);
			return;
		}
	}

	ent->wait = level.time - ent->timestamp;
	if (ent->wait > IDLE_TIME && ent->count)
	{
		gi.bprintf(PRINT_HIGH, "Call jumpball if basketball is stuck\n");
		ent->count = 0;
	}

	ent->nextthink = level.time + .1;
}

void BallShowCheck(edict_t *ent)
{
	int i;
	
	if(ent->client->pers.weapon->weapmodel != WEAP_HANDS)
		return;
	
	if(!ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->s.modelindex2 = 0;
	}
	else
	{
		ent->s.modelindex2 = 255;
		i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
}

//===============================================================
//
//	Ball control functions: Jumpball and Inbounds
//
//===============================================================

void Cmd_Ballmove_f (edict_t *ent)
{
	edict_t 	*ball = g_edicts;

	//find ball edicts
	for ( ; ball < &g_edicts[globals.num_edicts]; ball++)
	{
		if((!ball->inuse) || (ball->clipmask != MASK_BBALL))
			continue;
		if (ball->wait < IDLE_TIME)
		{
			gi.cprintf(ent, PRINT_HIGH, "Jumpball not allowed yet\n");
			return;
		}

		G_FreeEdict(ball);
		Jumpball();
		return;
	}
	gi.cprintf(ent, PRINT_HIGH	, "Ball not free\n");
}

void Cmd_Autoshoot_f (edict_t *ent)
{
	if(deathmatch->value)
		gi.cprintf(ent, PRINT_HIGH, "Server only can set autoshoot\n");
	else
	{
		if( (int)dmflags->value & DF_AUTO_SHOOT)
		{
			dmflags->value += DF_AUTO_SHOOT;
			gi.bprintf(PRINT_HIGH, "Autoshoot is off\n");
		}
		else
		{
			dmflags->value -= DF_AUTO_SHOOT;
			gi.bprintf(PRINT_HIGH, "Autoshoot is on\n");
		}
	}
}

void Jumpball()
{
	static int poss = TEAM1;

	if (poss == TEAM1)
	{
		gi.bprintf(PRINT_HIGH, "Jumpball, possession goes to %s\n",team1name->string);
		Inbound(TEAM1);
		poss = TEAM2;
	}
	else if (poss == TEAM2)
	{
		gi.bprintf(PRINT_HIGH, "Jumpball, possession goes to %s\n",team2name->string);
		Inbound(TEAM2);
		poss = TEAM1;
	}
}

qboolean Court_Zone (edict_t *ent)
{
	edict_t *zone;

	zone = G_Find (NULL, FOFS(classname), "func_court");
	
	if (!zone)
	{
		//gi.dprintf("No court zone found\n");
		return true;
	}
	
	//check to see if in zone box
	if (ent->s.origin[0] > zone->absmin[0])
		if (ent->s.origin[1] > zone->absmin[1])
			if (ent->s.origin[2] > zone->absmin[2])
				if (ent->s.origin[0] < zone->absmax[0])
					if (ent->s.origin[1] < zone->absmax[1])
						if (ent->s.origin[2] < zone->absmax[2])
							return true;
	
	return false;		// all clear
}

void Create_ball(int team)
{

	edict_t	*bball;
	edict_t *dest = NULL;

	if(team == TEAM1)
		dest = G_Find (NULL, FOFS(targetname), "inbound1");
	else if(team == TEAM2)
		dest = G_Find (NULL, FOFS(targetname), "inbound2");

	if (!dest)
	{
		gi.dprintf ("Couldn't find inbound destination\n");
		return;
	}
	
	bball = G_Spawn();
	VectorCopy (dest->s.origin, bball->s.origin);
	VectorSet (bball->avelocity, 300, 300, 300);
	bball->movetype = MOVETYPE_BALLBOUNCE;
	bball->clipmask = MASK_BBALL;
	bball->solid = SOLID_BBOX;
	VectorSet (bball->mins, 0,0,-10);
	VectorSet (bball->maxs, 0,0,0);
	bball->s.modelindex = gi.modelindex ("models/objects/bball/tris.md2");
	bball->owner = NULL;
	bball->touch = Basketball_Touch;
	bball->nextthink = level.time + .1;
	bball->think = Sitting_Think;
	bball->classname = "basketball";
	bball->spawnflags = 1;
	bball->timestamp = 0;
	bball->s.effects = EF_HYPERBLASTER;
	bball->style = STILL_GOOD;

	// draw the teleport splash at the destination
	bball->s.event = EV_PLAYER_TELEPORT;

	gi.linkentity (bball);

}

void InboundNow(edict_t *ent)
{
	int i, playerteam, team;
	edict_t *player;
	edict_t *other = NULL;
	edict_t	*dest;

	//don't inbound after time is up
	if (level.intermissiontime)
		return;
	
	team = ent->style;
	//done with timer now
	G_FreeEdict(ent);

	//search for other team member on court first
	for (i=0 ; i<game.maxclients+game.maxbots ; i++)
	{
		player = g_edicts + 1 + i;
		
		if((!player->inuse) || (!player->client && !player->bot)
			|| (player->health <= 0) || (!Court_Zone(player)) )
			continue;
		
		if(player->client)
			playerteam = player->client->resp.team;
		else if(player->bot)
			playerteam = player->bot->resp.team;
		if(playerteam == team)
		{
			other = player;
			break;
		}
	}

	//search for other team member anywhere
	if(!other)
	{
		for (i=0 ; i<game.maxclients+game.maxbots ; i++)
		{
			player = g_edicts + 1 + i;

			if((!player->inuse) || (!player->client && !player->bot)
				|| (player->health <= 0))
				continue;
			if(player->client)
				playerteam = player->client->resp.team;
			else if(player->bot)
				playerteam = player->bot->resp.team;
			if(playerteam == team)
			{
				other = player;
				break;
			}
		}
	}

	//search for anyone
	if(!other)
	{
		if(deathmatch->value)
			gi.bprintf(PRINT_HIGH,"Found no one on other team to inbound\n");
		
		for (i=0 ; i<game.maxclients+game.maxbots ; i++)
		{
			player = g_edicts + 1 + i;

			if((!player->inuse) || (!player->client && !player->bot)
				|| (player->health <= 0) || (player->movetype == MOVETYPE_NOCLIP))
				continue;
			other = player;
			break;
		}
	}

	//no one is found, make new ball
	if(!other)
	{
		gi.bprintf(PRINT_HIGH, "Found no one to inbound ball\n");
		Create_ball(team);
		return;
	}

	//give them ball
	if(other->client)
	{
		if (other->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] == 0)//M.S.Fix use add_ammo?
			other->client->pers.inventory[ITEM_INDEX(FindItem("Basketballs"))]++;
	}
	else if(other->bot)
	{
		if (other->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] == 0)//M.S.Fix use add_ammo?
			other->bot->pers.inventory[ITEM_INDEX(FindItem("Basketballs"))]++;
	}

	//don't teleport in single player, so we're done
	if(!deathmatch->value)
		return;

	if(team == TEAM1)
		dest = G_Find (NULL, FOFS(targetname), "inbound1");
	else
		dest = G_Find (NULL, FOFS(targetname), "inbound2");

	if (!dest)
	{
		gi.dprintf ("Couldn't find inbound destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 9;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	if(other->client)
	{
		other->client->ps.pmove.pm_time = 160>>3;		// hold time
		other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
		// set angles
		for (i=0 ; i<3 ; i++)
			other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
	}

	// draw the teleport splash at the destination
	other->s.event = EV_PLAYER_TELEPORT;


	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	
	if(other->client)
	{
		VectorCopy (dest->s.angles, other->client->ps.viewangles);
		VectorCopy (dest->s.angles, other->client->v_angle);
		//change their weapon to hands
		other->client->newweapon = FindItem("Hands");
		ChangeWeapon(other);	
	}

	// kill anything at the destination
	if (!KillBox (other))
	{
	}
	
	gi.linkentity (other);
}

void Inbound(int team)
{
	edict_t *timer;

	timer = G_Spawn();
	timer->svflags = SVF_NOCLIENT;
	timer->style = team;
	timer->think = InboundNow;
	timer->nextthink = level.time + 1.5; //timer at 1.5 seconds

	//for bot ai, when other team inbounding get back on d
	level.inbounding = team;
}

qboolean BallStillGood()
{
	edict_t 	*ball = g_edicts;

	//find ball edicts
	for ( ; ball < &g_edicts[globals.num_edicts]; ball++)
	{
		if((!ball->inuse) || (ball->clipmask != MASK_BBALL))
			continue;
		if (ball->style == NOT_GOOD)
			return false;
		else
			return true;
	}
	//no ball loose, return false
	return false;
}

//==============================================================
//
//	BBall Weapons
//
//==============================================================

//From p_weapon.c, fire needed in functions
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}

qboolean dunk_check (edict_t *ent)
{
	int i, playerteam, otherteam;
	edict_t *blip;
	vec3_t vec;

	if(ent->client)
		playerteam = ent->client->resp.team;
	else if(ent->bot)
		return false;//playerteam = ent->bot->resp.team;
	
	for (i=0 ; i<game.maxclients+game.maxbots ; i++)
	{
		blip = g_edicts + 1 + i;

		if( (!blip->inuse) || (!blip->client && !blip->bot)
			|| (blip->health <= 0) )

		VectorSubtract(ent->s.origin, blip->s.origin, vec );
		
		if(blip->client)
			otherteam = blip->client->resp.team;
		else if(blip->bot)
			otherteam = blip->bot->resp.team;
		
		if ((VectorLength( vec ) < OPEN_FOR_DUNK)
			&& (playerteam != otherteam))
			return false;
	}

	return true;
}

void weapon_bball_dunk (edict_t *ent, edict_t *rim)
{
	int i;
	vec3_t rim_front, point, forward;
	
	//find rim front
	AngleVectors(rim->s.angles,forward,NULL,NULL);
	VectorInverse(forward);
	VectorScale(forward, 48, point);
	VectorAdd(rim->s.origin, point, rim_front);
	
	// unlink to make sure it can't possibly interfere with stuff
	gi.unlinkentity (ent);
	
	// set angles
	if(ent->client)
	{
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(rim->s.angles[i] - ent->client->resp.cmd_angles[i]);
	}
	
	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = rim->s.angles[YAW];
	ent->s.angles[ROLL] = 0;
	if(ent->client)
	{
		VectorCopy (rim->s.angles, ent->client->ps.viewangles);
		VectorCopy (rim->s.angles, ent->client->v_angle);
	}

	//move player to front of rim
	ent->s.origin[0] = rim_front[0];
	ent->s.origin[1] = rim_front[1];
	ent->s.old_origin[0] = rim_front[0];
	ent->s.old_origin[1] = rim_front[1];
	
	// clear the velocity
	VectorClear (ent->velocity);
	if(ent->client)
	{
		ent->client->ps.pmove.pm_time = 160>>3;	// hold time
		ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	}
	
	gi.linkentity(ent);

	//jump now (according to height difference)
	//M.S.Fix (this should consider acceleration, but close enough?)
	ent->velocity[2] = 3.5*(rim_front[2] - ent->s.origin[2]);

	//play dunk animation
	if(ent->client)
	{
		ent->client->anim_priority = ANIM_DUNK;
		ent->client->anim_end = FRAME_balldunk10;
	}
	else if(ent->bot)
	{
		ent->bot->anim_priority = ANIM_DUNK;
		ent->bot->anim_end = FRAME_balldunk10;
	}
	ent->s.frame = FRAME_balldunk01;
}

void weapon_bball_autoshoot (edict_t *ent, edict_t *rim)
{
	vec3_t	offset, forward, right, start, length;
	float	speed, diff, distance;

	vec3_t temp, l_angles;
	float angle, height, random, p_angle;

	//find start point
	VectorSet(offset, 8, 8, ent->viewheight-8);
	if(ent->client)
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	}
	else if(ent->bot)
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
	}

	VectorSubtract(rim->s.origin, start, length);
	distance = FlatLength(length);

	vectoangles(length, l_angles);
	p_angle = anglemod(ent->s.angles[1] - l_angles[1]);

	//see if facing basket
	if (p_angle < 45 || p_angle > 315 || ent->bot)
	{
		if (distance < 100.0)
			angle = (4*M_PI)/9.0;
		else if (distance < 225.0)
			angle = (7.0*M_PI)/18.0;
		else if (distance < 425.0)
			angle = M_PI/3.0;
		else
			angle = (5*M_PI)/18.0;

		VectorNormalize2(length, temp);
		temp[2] = tan(angle);
		VectorNormalize2(temp, forward);
		height = rim->s.origin[2] - start[2];

		if (d_bball->value)
			gi.dprintf("Real distance: %f Height: %f Angle: %f\n",distance, height, angle);

		//false physics compensation
		distance += (40 + distance * .02);

		speed = distance / (cos(angle)*sqrt(2.0*(1.0/sv_gravity->value)*(distance*tan(angle)-height)));

		if (difficulty->value > 10)
			diff = 10;
		else if (difficulty->value < 0)
			diff = 0;
		else
			diff = difficulty->value;

		random = ((random()/10.0) - .05) * ((3.0/10.0) * (diff + 1) + (7.0/10.0));
		speed += distance * random;
	
		if(speed > BBALL_MAXSPEED)
			speed = BBALL_MAXSPEED;
		
		if (d_bball->value)
			gi.dprintf("Fixed distance: %f Speed %f Random: %f\n",distance, speed, random);

		fire_bball(ent, start, forward, speed, true);
		if(ent->client)
		{
			ent->client->pers.inventory[ent->client->ammo_index]--;
			ent->client->bball_time = level.time + 1.0;
		}
		else if(ent->bot)
		{
			ent->bot->pers.inventory[ent->bot->ammo_index]--;
		}
	}
	else
	{	//just toss it out because we aren't facing basket
		fire_bball (ent, start, forward, BBALL_MINSPEED, false);
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->bball_time = level.time + 1.0;
	}
}

void weapon_bball_fire (edict_t *ent)
{
	edict_t	*rim;
	vec3_t	length;
	float	distance, timer, speed;
	vec3_t	offset, forward, right, start;

	//find distance
	if ((ent->client && ent->client->resp.team == TEAM1) ||
		(ent->bot && ent->bot->resp.team == TEAM1))
		rim = G_Find (NULL, FOFS(targetname), "rim1");
	else
		rim = G_Find (NULL, FOFS(targetname), "rim2");

	if (!rim)
	{
		gi.dprintf ("Couldn't find rim marker\n");
		distance = 999999;
	}
	else
	{
		VectorSubtract(rim->s.origin, ent->s.origin, length);
		distance = FlatLength(length);
	}
	
	//do dunk if close enough
	if ( !((int)dmflags->value & DF_NO_DUNKS) && distance < DUNK_RANGE
		&& rim->s.origin[2] > ent->s.origin[2] && dunk_check(ent))
	{
		weapon_bball_dunk(ent, rim);
		return;
	}
		
	//separate modes
	if ( (int)dmflags->value & DF_AUTO_SHOOT || ent->bot)//bot always autoshoot
	{	//autoshoot
		if (!rim)
		{
			gi.dprintf ("Must have rim marker for autoshoot\n");
			gi.sound (ent, CHAN_BODY, gi.soundindex ("player/male/bump1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		weapon_bball_autoshoot(ent, rim);
	}
	else
	{	//manual shot
		VectorSet(offset, 8, 8, ent->viewheight-8);
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		
		timer = ent->client->bball_time - level.time;
		speed = BBALL_MINSPEED + (BBALL_TIMER - timer) * ((BBALL_MAXSPEED - BBALL_MINSPEED) / BBALL_TIMER);
		fire_bball (ent, start, forward, speed, false);
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->bball_time = level.time + 1.0;
	}
}

void Cmd_Passball_f (edict_t *ent)
{
	vec3_t	offset, temp;
	vec3_t	forward, right, angles;
	vec3_t	start;
	edict_t	*blip;
	trace_t	tr;
	int i;

	if (ent->client->weaponstate == WEAPON_FIRING  ||
		ent->client->anim_priority == ANIM_DUNK)
		return;
		
	if ((ent->client->pers.weapon->weapmodel == WEAP_HANDS)
		&& (ent->client->pers.inventory[ent->client->ammo_index] > 0))
	{
		VectorSet(offset, 8, 8, ent->viewheight-8);
		VectorCopy(ent->client->v_angle,temp);
		temp[0] = -15.0;
		AngleVectors (temp, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bball (ent, start, forward, 800, true);

		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else if(game.maxbots) //bot will pass to player
	{
		for (i=0 ; i<game.maxbots ; i++)
		{
			blip = g_edicts + game.maxclients + 1 + i;

			if(blip->bot->resp.team == ent->client->resp.team &&
				blip->bot->pers.inventory[blip->bot->ammo_index])
			{
				tr = gi.trace (blip->s.origin, blip->mins, blip->maxs, ent->s.origin, blip, MASK_SHOT);
				if(tr.ent == ent) //trace hit player
				{
					VectorSubtract(ent->s.origin, blip->s.origin, forward);
					vectoangles(forward, angles);
					blip->s.angles[YAW] = angles[YAW];
					blip->s.angles[PITCH] = -15.0;

					VectorSet(offset, 8, 8, blip->viewheight-8);
					AngleVectors (blip->s.angles, forward, right, NULL);
					G_ProjectSource(blip->s.origin, offset, forward, right, start);
					fire_bball (blip, start, forward, 800, true);
					blip->bot->pers.inventory[blip->bot->ammo_index]--;
				}
			}
		}
	}
}

// Fire_bball function
void fire_bball (edict_t *self, vec3_t start, vec3_t aimdir, float speed, qboolean autoshot)
{
	edict_t	*bball;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	bball = G_Spawn();
	VectorCopy (self->s.origin, bball->pos1);
	VectorCopy (start, bball->s.origin);
	VectorScale (aimdir, speed, bball->velocity);
	if (!autoshot)
	{
		VectorMA (bball->velocity, 200 + crandom() * 10.0, up, bball->velocity);
		VectorMA (bball->velocity, crandom() * 10.0, right, bball->velocity);
	}
	VectorSet (bball->avelocity, 300, 300, 300);
	bball->movetype = MOVETYPE_BALLBOUNCE;
	bball->clipmask = MASK_BBALL;
	bball->solid = SOLID_BBOX;
	VectorSet (bball->mins, 0,0,-10);
	VectorSet (bball->maxs, 0,0,0);
	bball->s.modelindex = gi.modelindex ("models/objects/bball/tris.md2");
	bball->owner = self;
	bball->touch = Basketball_Touch;
	bball->nextthink = level.time + .1;
	bball->think = Sitting_Think;
	bball->classname = "basketball";
	bball->spawnflags = 1;
	bball->timestamp = 0;
	bball->style = STILL_GOOD;
	if(deathmatch->value)
		bball->s.effects = EF_HYPERBLASTER;

	gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (bball);
}

void steal_bball(edict_t *ent, vec3_t start, vec3_t aimdir)
{
	trace_t 	tr;
	vec3_t 		end;
	int bball = ITEM_INDEX(FindItem("Basketballs"));



	// Figure out what is in front of us, if anything
	VectorMA (start, STEAL_RANGE, aimdir, end);  //calculates the range vector
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
								// figures out what is in front of the player up till "end"
	// Figure out what to do about what we hit, if anything
	if (tr.fraction < 1.0)
	{
		if ((tr.ent->client || tr.ent->bot) && tr.ent->health > 0)
		{
			if ( (tr.ent->client && tr.ent->client->pers.inventory[bball])
				|| (tr.ent->bot &&tr.ent->bot->pers.inventory[bball]) )
			{
				//check some conditions (bot always uses hands)
				if(tr.ent->bot || (tr.ent->client->pers.weapon &&
					tr.ent->client->pers.weapon->weapmodel == WEAP_HANDS) )
				{
					if(tr.ent->client)
					{
						if(tr.ent->client->anim_priority == ANIM_DUNK)
							return;
						if(tr.ent->client->weaponstate == WEAPON_FIRING)
							return;
					}
					else if(tr.ent->bot)
					{
						if(tr.ent->bot->anim_priority == ANIM_DUNK)
							return;
						if(tr.ent->bot->weaponstate == WEAPON_FIRING)
							return;
					}
				}				
				
				//steal ball
				if(tr.ent->client)
					tr.ent->client->pers.inventory[bball]--;
				else if(tr.ent->bot)
				{
					tr.ent->bot->pers.inventory[bball]--;
					//don't let bot steal it back right away
					tr.ent->bot->action_time = level.time + 0.8;
				}
				
				if (ent->client && ent->client->pers.inventory[bball] == 0)//M.S.Fix use add_ammo?
					ent->client->pers.inventory[bball]++;
				else if(ent->bot && ent->bot->pers.inventory[bball] == 0)
					ent->bot->pers.inventory[bball]++;
			}
		}
	}
}

void weapon_bball_steal(edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	
	if(ent->client)
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	}
	else if(ent->bot)
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
	}

	steal_bball (ent, start, forward);
}

void Weapon_Bball (edict_t *ent)
{
	qboolean hasball;
	
	if(ent->client->pers.inventory[ent->client->ammo_index])
		hasball = true;
	else
		hasball = false;
	
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == BBALL_DEACTIVATE_LAST
			|| ent->client->ps.gunframe == HAND_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		//for vwep (Hentai)
		else if((BBALL_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4
			|| (HAND_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}
		//vwep

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if(!hasball && ent->client->ps.gunframe == 0)
			ent->client->ps.gunframe = HAND_ACTIVATE_FIRST;
				
		if (ent->client->ps.gunframe == BBALL_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = BBALL_IDLE_FIRST;
			return;
		}
		if (ent->client->ps.gunframe == HAND_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = HAND_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}
	
	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		if(hasball)
			ent->client->ps.gunframe = BBALL_DEACTIVATE_FIRST;
		else
			ent->client->ps.gunframe = HAND_DEACTIVATE_FIRST;
		
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (hasball && ent->client->ps.gunframe > HAND_FIRE_LAST)
			ent->client->ps.gunframe = BBALL_IDLE_FIRST;

		if (!hasball && ent->client->ps.gunframe < HAND_FIRE_FIRST)
			ent->client->ps.gunframe = HAND_IDLE_FIRST;
		
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (hasball)
			{
				ent->client->ps.gunframe = BBALL_FIRE_FIRST + 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->bball_time = 0;
				
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					//ent->s.frame = FRAME_crattak1-1;
					//ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_ballshot1-1;
					ent->client->anim_end = FRAME_ballshot4;
				}

			}
			else
			{
				ent->client->ps.gunframe = HAND_FIRE_FIRST + 1;
				ent->client->weaponstate = WEAPON_STEALING;

			}
			return;
		}

		if(ent->client->ps.gunframe == HAND_IDLE_LAST)
			return;

		// Ridah, added, with lower volume, and attenuation
		if(ent->client->ps.gunframe == BBALL_DRIBBLE_FRAME)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/bball1a.wav"), 0.2, 3, 0);
		
		if (++ent->client->ps.gunframe > BBALL_IDLE_LAST)
			ent->client->ps.gunframe = BBALL_IDLE_FIRST;
	
		// Ridah, if running, speed up 
		if (VectorLength( ent->velocity ) > 160)
		{
			// Ridah, added, with lower volume, and attenuation
			if(ent->client->ps.gunframe == BBALL_DRIBBLE_FRAME)
				gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/bball1a.wav"), 0.2, 3, 0);

			if (++ent->client->ps.gunframe > BBALL_IDLE_LAST)
				ent->client->ps.gunframe = BBALL_IDLE_FIRST;
		}
		
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{

		if (ent->client->ps.gunframe == BBALL_FIRE_FIRST + 1)
		{
			if (!ent->client->bball_time)
			{
				ent->client->bball_time = level.time + BBALL_TIMER + 0.2;
			}
			// they waited too long, fire it
			if (!ent->client->grenade_blew_up && level.time >= ent->client->bball_time)
			{
				ent->client->weapon_sound = 0;
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK && !ent->client->grenade_blew_up
				&& !((int)dmflags->value & DF_AUTO_SHOOT))
			{
          		if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
					ent->s.frame = FRAME_ballshot2;
				ent->client->ball_meter++;
				return;
			}

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->bball_time)
				{
					ent->client->ps.gunframe = BBALL_FIRE_FIRST + 2;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == BBALL_FIRE_FIRST + 2)
		{
			ent->client->ball_meter = 0;
			ent->client->weapon_sound = 0;
			weapon_bball_fire (ent);
		}

		if ((ent->client->ps.gunframe == BBALL_FIRE_LAST) && (level.time < ent->client->bball_time))
			return;

		if ((ent->client->ps.gunframe == BBALL_FIRE_LAST) && (!hasball))
		{
			ent->client->ps.gunframe = HAND_IDLE_FIRST;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		ent->client->ps.gunframe++;


		if (ent->client->ps.gunframe == BBALL_IDLE_FIRST)
			ent->client->weaponstate = WEAPON_READY;
	}

	if (ent->client->weaponstate == WEAPON_STEALING)
	{
		if(ent->client->ps.gunframe > HAND_FIRE_FIRST + 4
			&& ent->client->ps.gunframe < HAND_FIRE_FIRST + 9)
			weapon_bball_steal(ent);

		ent->client->ps.gunframe++;
		if(ent->client->ps.gunframe == HAND_IDLE_FIRST)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//================================================================
//
//	Court, Scoreboard and begin game drop ball
//
//================================================================

/*QUAKED func_court (.5 .5 .5) ?
court area

*/
void func_court_think(edict_t *self)
{
	int i;
	edict_t *blip;
	
	if(!((int)dmflags->value & DF_KEEP_IN_COURT))
		return;
	
	//bots should never leave the court
	for (i=0; i<game.maxclients; i++)
	{
		blip = &g_edicts[i+1];

		if (!blip->inuse)
			continue;
		if (blip->health <= 0)
			continue;
		
		//see if player with ball is in court
		if(blip->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))]
			&& !Court_Zone(blip))
		{
			// remove ball, kill him, then inbound
			blip->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] = 0;
			
			T_Damage (blip, blip, blip, vec3_origin, blip->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_BALL_LEAVE);

			if(blip->client->resp.team == TEAM1)
				Inbound(TEAM2);
			else
				Inbound(TEAM1);
		}
	}

	//half-second should be enough
	self->nextthink = level.time + .5;
}

void SP_func_court(edict_t *self)
{
	if(!deathmatch->value)
	{
		self->think = NULL;
		return;
	}
	
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;

	self->think = func_court_think;

	self->nextthink = level.time + .5;
}

/*QUAKED func_scoreclock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

*/

#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_scoreclock_format_countdown (edict_t *self)
{
	int minutes, seconds, countdown;

	countdown = (60*timelimit->value - (level.time - self->timestamp));

	if(countdown >0)
	{
		minutes = countdown/60;
		seconds = countdown%60;
	}
	else
	{
		minutes = seconds = 0;
	}
	
	Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", minutes, seconds);

	if (self->message[3] == ' ')
		self->message[3] = '0';
}

void func_scoreclock_think (edict_t *self)
{
	func_scoreclock_format_countdown (self);
	
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	self->nextthink = level.time + 1;
}

void SP_func_scoreclock (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);
	self->timestamp = level.time;

	self->think = func_scoreclock_think;
	self->nextthink = level.time + 1;
}

/*QUAKED func_score (0 0 1) (-8 -8 -8) (8 8 8)
target a target_string with this

"style"	1	Team1's Score
		2	Team2's Score

*/

#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_score_format_countdown (edict_t *self)
{
	if (self->style == 1)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%3i", level.ballteam1);
		return;
	}
	else if (self->style == 2)
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%3i", level.ballteam2);
}

void func_score_think (edict_t *self)
{
	func_score_format_countdown (self);
	
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);
	
	self->nextthink = level.time + 1;
}

void SP_func_score (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);

	self->think = func_score_think;

	self->nextthink = level.time + 1;
}


/*QUAKED misc_dropball (0 0 1) (-8 -8 -8) (8 8 8)
*/

void misc_dropball_drop(edict_t *self)
{
	edict_t	*bball;

	bball = G_Spawn();
	VectorCopy (self->s.origin, bball->s.origin);
	VectorSet (bball->avelocity, 300, 300, 300);
	bball->movetype = MOVETYPE_BALLBOUNCE;
	bball->clipmask = MASK_BBALL;
	bball->solid = SOLID_BBOX;
	VectorSet (bball->mins, 0,0,-10);
	VectorSet (bball->maxs, 0,0,0);
	bball->s.modelindex = gi.modelindex ("models/objects/bball/tris.md2");
	bball->owner = self;
	bball->touch = Basketball_Touch;
	bball->nextthink = level.time + .1;
	bball->think = Sitting_Think;
	bball->classname = "basketball";
	bball->spawnflags = 1;
	bball->timestamp = 0;
	bball->s.effects = EF_HYPERBLASTER;
	bball->style = STILL_GOOD;

	gi.linkentity (bball);

	level.drophack = true;
}

void misc_dropball_timer(edict_t *self)
{
	int i;
	edict_t *ent;

	self->count--;
		
	//play countdown sound
	if (self->count == 10)
		gi.sound(self,CHAN_VOICE|CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
	
	for (i=0; i<game.maxclients; i++)
	{
		ent = &g_edicts[i+1];

		if (!ent->inuse || !ent->client)
			continue;

		ent->client->ps.stats[STAT_JUMP_COUNT] = self->count;
	}	

	if (!self->count)
	{
		misc_dropball_drop(self);
		return;
	}

	self->nextthink = level.time + 1;
}


void misc_dropball_think(edict_t *self)
{
	edict_t *blip;
	int size1 = 0;
	int size2 = 0;
	int i;
	
	for (i=0 ; i<game.maxclients+game.maxbots ; i++)
	{
		blip = g_edicts + 1 + i;

		if( (!blip->inuse) || (!blip->client && !blip->bot)
			|| (blip->health <= 0) )
			continue;
		if(blip->client)
		{
			if (blip->client->resp.team == TEAM1)
				size1++;
			else if (blip->client->resp.team == TEAM2)
				size2++;
		}
		else if(blip->bot)
		{
			if (blip->bot->resp.team == TEAM1)
				size1++;
			else if (blip->bot->resp.team == TEAM2)
				size2++;
		}
	}	
	
	if( (int)dmflags->value & DF_EVEN_JUMPBALL)
	{
		if((size1 && size2) || d_bball->value)
			self->think = misc_dropball_timer;
	}
	else if(size1 || size2 || d_bball->value)
		self->think = misc_dropball_timer;

	self->nextthink = level.time + 1;
}

void SP_misc_dropball (edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	else
	{
		self->count = 11; //countdown time
		self->think = misc_dropball_think;
		self->nextthink = level.time + 1;
	}
}

//
//rim guard is spawned by SP_path_corner
//
void rim_guard_kill (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other->inuse && (other->client || other->bot) &&
		other->health > 0 && other->movetype != MOVETYPE_NOCLIP)
		//kill them
		T_Damage (other, other, other, vec3_origin, other->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_GOALTEND);
}	

void SP_rim_guard (edict_t *rim)
{
	edict_t *guard;
	
	if(!deathmatch->value)
		return;
	
	guard = G_Spawn();
	VectorCopy(rim->s.origin, guard->s.origin);
	VectorSet (guard->mins, -16, -16, 0);
	VectorSet (guard->maxs, 16, 16, 32);
	guard->movetype = MOVETYPE_NONE;
	guard->solid = SOLID_TRIGGER;
	guard->touch = rim_guard_kill;
	guard->svflags |= SVF_NOCLIENT;
	guard->classname = "rimguard";
	
	gi.linkentity (guard);
}

/*QUAKED func_scoretrigger (0 0 1) (-8 -8 -8) (8 8 8) ? MULTIPLE

If MULTIPLE, will trigger for multiples of "wait" value

"wait" : points scored to set off trigger
"style" : which team must score "wait" points, 0 for either team
*/

void trigger_functrigger(edict_t *ent)
{
	G_UseTargets (ent, level.lastscorer);

	if(ent->spawnflags & 2)
		ent->count++;
	else
		G_FreeEdict(ent);
}


void scoretrigger_think(edict_t *ent)
{
	switch(ent->style)
	{
	case TEAM1:
		if( (level.ballteam1 - ent->count * ent->wait) > ent->wait)
			trigger_functrigger(ent);
		break;
	case TEAM2:
		if( (level.ballteam2 - ent->count * ent->wait) > ent->wait)
			trigger_functrigger(ent);
		break;
	default:
		if( ((level.ballteam1 - ent->count * ent->wait) > ent->wait) ||
			((level.ballteam2 - ent->count * ent->wait) > ent->wait) )
			trigger_functrigger(ent);
	}

	//set nextthink if we haven't been freed
	if(ent->inuse)
		ent->nextthink = level.time + .5;
}

void SP_func_scoretrigger(edict_t *ent)
{
	if(!ent->wait)
	{
		gi.dprintf("func_scoretrigger without 'wait' value\n");
		G_FreeEdict(ent);
	}

	ent->svflags |= SVF_NOCLIENT;

	ent->think = scoretrigger_think;
	ent->nextthink = .5;
}