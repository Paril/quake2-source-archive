#include "g_local.h"
#include "m_player.h"

#define	MAX_BOT_NAMES	9
char *bot_names[MAX_BOT_NAMES] =
{
	"Hellrot",
	"Tokay",
	"Killme",
	"Disruptor",
	"Adrianator",
	"Rambear",
	"Titus",
	"Bitterman",
	"Shade"
};

//////////////////////////////////////	
//	BotAI
//////////////////////////////////////	

//spot for offensive basket cuts
vec3_t bot1_off_spot[3];
vec3_t bot2_off_spot[3];

typedef struct botai_s
{
	edict_t		*target;
	int			targetflag; //BT_NONE, BT_CLIENT, etc...
//	edict_t		*players[32];
	int			poss;		//which team has ball
	edict_t		*rim1;		//rims set by InitBotAI when game begins
	edict_t		*rim2;
} botai_t;

//our global botAI information
static botai_t botAI;

#define BOT_RUNSPEED	30.0
#define BOT_SLIDESPEED	15.0
#define BOT_DEF_SPEED	22.5
#define BOT_DEF_DIST	60.0
#define BOT_STEAL_TIME	1.2
#define BOT_SHOT_OPEN   75.0
#define BOT_ZONE_RANGE	250.0

static qboolean avoid(edict_t *ent)
{
	edict_t	*blip;
	vec3_t	vec, forward;
	int		i, team;
	qboolean avoid;
	
	AngleVectors (ent->s.angles, forward, NULL, NULL);

	for(i = 0; i<game.maxclients+game.maxbots; i++)
	{
		blip = g_edicts+i+1;
		if(blip->bot)
			team = blip->bot->resp.team;
		else if(blip->client)
			team = blip->client->resp.team;

		if(blip->inuse && blip->movetype != MOVETYPE_NOCLIP &&
			team != ent->bot->resp.team)
		{
			VectorSubtract(blip->s.origin, ent->s.origin, vec);
			if(DotProduct(vec, forward) > 0 && FlatLength(vec) < 100)
				return true;
		}
	}
	return false;
}

void bot_move(edict_t *ent, edict_t *target, vec3_t spot, int movetype)
{
	vec3_t	dist, move;
	float	ofs;

	VectorSubtract(spot, ent->s.origin, dist);
	
	//are we close enough to target?
	if(FlatLength(dist) < 30)
	{
		VectorSubtract(target->s.origin, ent->s.origin, dist);
		ent->ideal_yaw = vectoyaw(dist);
		M_ChangeYaw(ent); //always face the target
		
		ent->speed = 0; //speed here only flags if we're moving
		return;
	}

	switch(movetype)
	{
	case BOT_MOVE_GUARD: //we will slide instead of turn and run
		//face the ball
		VectorSubtract(target->s.origin, ent->s.origin, dist);
		ent->ideal_yaw = vectoyaw(dist);
		M_ChangeYaw(ent);

		if (!ent->groundentity)
			return;
		
		//do our slide
		VectorSubtract(spot, ent->s.origin, dist);
		VectorNormalize(dist);
		VectorScale(dist, BOT_DEF_SPEED, dist);
		
		move[0] = dist[0];
		move[1] = dist[1];
		move[2] = 0;

		if(SV_movestep(ent, move, true))
			ent->speed = 1;
		else
			ent->speed = 0;
		break;
	case BOT_MOVE_NORMAL: //use the turn and run technique
		ent->ideal_yaw = vectoyaw(dist);
		M_ChangeYaw (ent);
		ent->speed = 1;
		
		if(!M_walkmove (ent, ent->s.angles[YAW], BOT_RUNSPEED))
		{	
			if (ent->monsterinfo.lefty)
				ofs = 90;
			else
				ofs = -90;
			
			if (M_walkmove (ent, ent->ideal_yaw + ofs, BOT_SLIDESPEED))
				return;
				
			ent->monsterinfo.lefty = 1 - ent->monsterinfo.lefty;
			
			if(!M_walkmove (ent, ent->ideal_yaw - ofs, BOT_SLIDESPEED))
				ent->speed = 0;
		}
		break;
	case BOT_MOVE_DRIBBLE: //avoid defenders
		ent->ideal_yaw = vectoyaw(dist);
		M_ChangeYaw (ent);
		ent->speed = 1;
		
		if(avoid(ent) || !M_walkmove (ent, ent->s.angles[YAW], BOT_RUNSPEED))
		{	
			if (ent->monsterinfo.lefty)
				ofs = 90;
			else
				ofs = -90;
			
			if (M_walkmove (ent, ent->ideal_yaw + ofs, BOT_SLIDESPEED))
				return;
				
			ent->monsterinfo.lefty = 1 - ent->monsterinfo.lefty;
			
			if(!M_walkmove (ent, ent->ideal_yaw - ofs, BOT_SLIDESPEED))
				ent->speed = 0;
		}
		break;
	}
}

void bot_defense_spots(edict_t	*teamlist[MAX_BOTS], int team, int size)
{
	vec3_t	spotlist[5];
	edict_t	*rim;
	vec3_t	spot, forward, right;
	int i;

	//find rim to defend
	if(team == TEAM1)
		rim = botAI.rim2;
	else
		rim = botAI.rim1;

	AngleVectors(rim->s.angles,forward,right,NULL);
	VectorInverse(forward);
	VectorInverse(right);
	VectorCopy(rim->s.origin, spot);

	//choose zone/inital defense spots
	switch(size)
	{
	case 1:
		VectorMA(spot, 300, forward, spotlist[0]);//1
		break;
	case 2:
		VectorMA(spot, 300, forward, spotlist[0]);//1
		VectorMA(spot, 100, forward, spotlist[1]);//2
		break;
	case 3:
		VectorMA(spot, 300, forward, spotlist[0]);//1
		VectorMA(spot, 50, forward, spotlist[1]);//2
		VectorMA(spotlist[1], -200, right, spotlist[1]);
		VectorMA(spot, 50, forward, spotlist[2]);//3
		VectorMA(spotlist[2], 200, right, spotlist[2]);
		break;
	case 4:
		VectorMA(spot, 300, forward, spotlist[0]);//1
		VectorMA(spotlist[0], -100, right, spotlist[0]);
		VectorMA(spot, 300, forward, spotlist[1]);//2
		VectorMA(spotlist[1], 100, right, spotlist[1]);
		VectorMA(spot, 50, forward, spotlist[2]);//3
		VectorMA(spotlist[2], -200, right, spotlist[2]);
		VectorMA(spot, 50, forward, spotlist[3]);//4
		VectorMA(spotlist[3], 200, right, spotlist[3]);
		break;
	default:
		VectorMA(spot, 300, forward, spotlist[0]);//1
		VectorMA(spotlist[0], -100, right, spotlist[0]);
		VectorMA(spot, 300, forward, spotlist[1]);//2
		VectorMA(spotlist[1], 100, right, spotlist[1]);
		VectorMA(spot, 50, forward, spotlist[2]);//3
		VectorMA(spotlist[2], -300, right, spotlist[2]);
		VectorMA(spot, 50, forward, spotlist[3]);//4
		VectorMA(spot, 50, forward, spotlist[4]);//5
		VectorMA(spotlist[4], 300, right, spotlist[4]);
		break;
	}

	//assign spots
	for(i = 0; i < size && i < 5; i++)
		VectorCopy(spotlist[i], teamlist[i]->bot->defense_spot);
}

void bot_defense_matchup()
{
	int i, team1size, team2size;
	edict_t *ent;
	edict_t	*team1list[MAX_BOTS];
	edict_t *team2list[MAX_BOTS];

	team1size = team2size = 0;

	//put players in list
	for (i=0 ; i<game.maxbots ; i++)
	{
		ent = g_edicts + game.maxclients + 1 + i;

		if(ent->inuse)
		{
			if(ent->bot->resp.team == TEAM1)
			{
				team1list[team1size] = ent;
				team1size++;
			}
			else if(ent->bot->resp.team == TEAM2)
			{
				team2list[team2size] = ent;
				team2size++;
			}
		}
	}

	//give defensive spots on court
	bot_defense_spots(team1list, TEAM1, team1size);
	bot_defense_spots(team2list, TEAM2, team2size);

	//man2man if equal teams
	//this is more for 1 player vs. 1 player with bot teammates
	if(team1size == team2size)
	{
		//assign marks
		for(i = 0; i < team1size; i++)
		{
			team1list[i]->bot->mark = team2list[i];
			team1list[i]->bot->defense_mode = BD_MAN2MAN;
			//same for other team
			team2list[i]->bot->mark = team1list[i];
			team2list[i]->bot->defense_mode = BD_MAN2MAN;
		}
	}
	else
	{
		for(i = 0; i < team1size; i++)
		{
			if(i == 0) //he is point man
				team1list[i]->bot->defense_mode = BD_POINT;
			else
				team1list[i]->bot->defense_mode = BD_ZONE;
			team1list[i]->bot->mark = NULL;
		}
		for(i = 0; i < team2size; i++)
		{
			if(i == 0) //he is point man
				team2list[i]->bot->defense_mode = BD_POINT;	
			else
				team2list[i]->bot->defense_mode = BD_ZONE;
			team2list[i]->bot->mark = NULL;
		}
	}
}

edict_t *bot_zone_mark(edict_t *ent)
{
	edict_t *blip;
	edict_t	*best = NULL;
	vec3_t	vec;
	float	playerdist;
	float	bestdist = BOT_ZONE_RANGE;
	int		i, team;
	
	for (i=0 ; i<game.maxclients+game.maxbots ; i++)
	{
		blip = g_edicts + 1 + i;

		if( (!blip->inuse) || (!blip->client && !blip->bot) ||
			(blip->health <= 0) || (blip->movetype == MOVETYPE_NOCLIP))
			continue;

		VectorSubtract(ent->s.origin, blip->s.origin, vec );
		playerdist = VectorLength(vec);
		team = (blip->client) ? blip->client->resp.team : blip->bot->resp.team;
		if (team == ent->bot->resp.team || playerdist > bestdist)
			continue;
		else
		{
			best = blip;
			bestdist = playerdist;
		}
	}

	return best;
}

void bot_defense (edict_t *ent, edict_t *target)
{
	vec3_t	spot, dist, temp, up;
	edict_t	*rim, *markup;
	int		shootstate;
	
	if(ent->bot->resp.team == TEAM1) //find which rim to defend
		rim = botAI.rim2;
	else
		rim = botAI.rim1;

	switch(ent->bot->defense_mode)
	{
	case BD_ZONE:
		markup = bot_zone_mark(ent);
		break;
	case BD_MAN2MAN:
		markup = ent->bot->mark;
		break;
	case BD_POINT:
		markup = target;
	}
	
	if(botAI.targetflag)
		VectorSubtract(target->s.origin, rim->s.origin, dist);
	else
		VectorSet(dist, 999, 999, 0); //make us go on d
	//only play D in halfcourt
	if(FlatLength(dist) > THREE_DISTANCE+200 || !markup)
	{
		//go to initial defensive position
		if(markup && markup == target) //anticipate drive to basket
		{
			VectorNormalize2(dist, temp);
			VectorMA(rim->s.origin, THREE_DISTANCE, temp, spot);
			bot_move(ent, target, spot, BOT_MOVE_NORMAL);
		}
		else //just go to position
			bot_move(ent, target, ent->bot->defense_spot, BOT_MOVE_NORMAL);

	}
	else	//play D between man and rim
	{
		//block shot
		if(markup->bot)
			shootstate = markup->bot->weaponstate;
		else if(markup->client)
			shootstate = markup->client->weaponstate;

		if(shootstate == WEAPON_FIRING && ent->groundentity)
		{
			ent->velocity[2] = 200;
			return;
		}
					
		//try to steal ball
		if(markup == target && level.time >= ent->bot->action_time
			&& !(rand() & 7)) //only successful about .8 seconds
		{
			weapon_bball_steal(ent);
			ent->bot->action_time = level.time + BOT_STEAL_TIME;
		}
		else if(ent->bot->action_time > level.time + BOT_STEAL_TIME)
			ent->bot->action_time = level.time + BOT_STEAL_TIME;
		
		//let bot get juked by another bot, it helps game flow
		if(ent->bot->defense_juked)
		{
			if(ent->bot->defense_juked++ > 10) //juked for 1 second
				ent->bot->defense_juked = 0;
			else
			{
				ent->speed = 0;
				return; //stand still and look stupid
			}
		}
		else if(markup->bot && !(rand() & 7)) //juked every .8 seconds
			ent->bot->defense_juked = 1;
						
		//project defensive position
		VectorNormalize2(dist, temp);
		VectorInverse(temp);
		VectorMA(markup->s.origin, BOT_DEF_DIST, temp, spot);
		bot_move(ent, markup, spot, BOT_MOVE_GUARD);
	}
}

qboolean bot_offense_shoot(edict_t *ent, edict_t *rim)
{
	vec3_t	forward, end, vec;
	trace_t	tr;
	edict_t	*blip;
	float	dist;
	int		i, team;

	AngleVectors (ent->s.angles, forward, NULL, NULL);
	VectorMA (ent->s.origin, BOT_SHOT_OPEN, forward, end);  //calculates the range vector
	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SHOT);

	//find basket distance
	VectorSubtract(ent->s.origin, rim->s.origin, vec);
	dist = FlatLength(vec);
	
	if(dist > 375 || dist < 40) //no long bombs or hitting net bottom
		return false;
	
	//see if we are open
	if (tr.ent && ((tr.ent->bot && tr.ent->bot->resp.team != ent->bot->resp.team)
		|| (tr.ent->client && tr.ent->client->resp.team != ent->bot->resp.team)) )
		return false;
	else if(dist < 150)
	{	//shoot now			
		weapon_bball_fire (ent);
		ent->s.frame = FRAME_ballshot1-1;
		ent->bot->anim_end = FRAME_ballshot4;
		return true;
	}
	else
	{	//see if we can get a better shot
		for(i = 0; i<game.maxclients+game.maxbots; i++)
		{
			blip = g_edicts+i+1;
			if(blip->bot)
				team = blip->bot->resp.team;
			else if(blip->client)
				team = blip->client->resp.team;

			if(blip->inuse && blip->movetype != MOVETYPE_NOCLIP &&
				team != ent->bot->resp.team)
			{
				VectorSubtract(blip->s.origin, ent->s.origin, vec);
				if(DotProduct(vec, forward) > 0 && FlatLength(vec) < 150)
				{ //shoot now			
					weapon_bball_fire (ent);
					ent->s.frame = FRAME_ballshot1-1;
					ent->bot->anim_end = FRAME_ballshot4;
					return true;
				}
			}
		}
	}
	return false;
}

qboolean bot_offense_pass(edict_t *ent, edict_t *rim)
{
	vec3_t	forward, right, start, offset, dist, angles;
	trace_t	tr;
	edict_t	*blip;
	float	mydist, hisdist, passdist;
	int		i;
	int		goodteam = ent->bot->resp.team;

	//don't pass until in offense
	VectorSubtract(ent->s.origin, rim->s.origin, dist);
	if(FlatLength(dist) > 450)
		return false;	
	
	//find teammate, attempt pass
	for(i = 0; i<game.maxclients+game.maxbots; i++)
	{
		blip = g_edicts + i + 1;

		if(blip->inuse && blip->health > 0)
		{
			if((blip->bot && blip->bot->resp.team == goodteam) || 
				(blip->client && blip->client->resp.team == goodteam))
			{
				tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, blip->s.origin, ent, MASK_SHOT);
				
				VectorSubtract(blip->s.origin, rim->s.origin, dist);
				hisdist = FlatLength(dist);
				VectorSubtract(ent->s.origin, rim->s.origin, dist);
				mydist = FlatLength(dist);
				VectorSubtract(blip->s.origin, ent->s.origin, dist);
				passdist = FlatLength(dist);

				if(tr.ent == blip && passdist < 250 &&
					passdist > 70 && hisdist < mydist)
				{ //make the pass
					//face player
					vectoangles(dist, angles);
					ent->s.angles[YAW] = angles[YAW];
					ent->s.angles[PITCH] = -15.0;

					VectorSet(offset, 8, 8, ent->viewheight-8);
					AngleVectors (ent->s.angles, forward, right, NULL);
					G_ProjectSource(ent->s.origin, offset, forward, right, start);
					fire_bball (ent, start, forward, 800, true);
					ent->bot->pers.inventory[ent->bot->ammo_index]--;
					return true;
				}
			}
		}
	}

	return false;
}

void bot_offense_cuts(edict_t *ent, edict_t *rim, vec3_t spot)
{
	vec3_t dist;
	int choice;
	
	switch (ent->bot->offense_cut)
	{
	case BC_CUT_TO:
		VectorSubtract(ent->s.origin, rim->s.origin, dist);
		if(FlatLength(dist) < 30)
			ent->bot->offense_cut = BC_CUT_BACK;
		else
			VectorCopy(rim->s.origin, spot);
		break;
	case BC_CUT_BACK:
		VectorSubtract(ent->s.origin, ent->bot->offense_spot, dist);
		if(FlatLength(dist) < 30)
			ent->bot->offense_cut = BC_CUT_NONE;
		else
			VectorCopy(ent->bot->offense_spot, spot);
		break;
	case BC_CUT_NONE:
		choice = rand() % 3;
		if(ent->bot->resp.team == TEAM1)
			VectorCopy(bot1_off_spot[choice], ent->bot->offense_spot);
		else
			VectorCopy(bot2_off_spot[choice], ent->bot->offense_spot);
		ent->bot->offense_cut = BC_CUT_TO;
	}
}

void bot_offense (edict_t *ent, edict_t *target)
{
	vec3_t	dist, spot, forward;
	edict_t *rim;

	if(ent->bot->resp.team == TEAM1) //find which rim is ours
		rim = botAI.rim1;
	else
		rim = botAI.rim2;

	VectorSubtract(ent->s.origin, rim->s.origin, dist);
	
	if(ent->bot->pers.inventory[ent->bot->ammo_index])
	{
		if(level.time >= ent->bot->action_time)
		{
			if(FlatLength(dist) > 250)
			{//attempt pass first
				if(bot_offense_pass(ent, rim))
					return;
				else if(bot_offense_shoot(ent, rim))
					return;
			}
			else
			{//attempt shot first
				if(bot_offense_shoot(ent, rim))
					return;
				else if(bot_offense_pass(ent, rim))
					return;
			}
		}
		//didn't pass or shoot, so dribble
		//goal is near front of rim
		AngleVectors(rim->s.angles,forward,NULL,NULL);
		VectorInverse(forward);
		VectorMA(rim->s.origin, 64, forward, spot);
		bot_move(ent, rim, spot, BOT_MOVE_DRIBBLE);
	}
	else //make cuts to the basket
	{
		bot_offense_cuts(ent, rim, spot);
		bot_move(ent, target, spot, BOT_MOVE_NORMAL);
	}
}

void bot_looseball(edict_t *ent, edict_t *target)
{
	vec3_t	dist, spot;
	float	height;
	
	VectorCopy(target->s.origin, spot);
	VectorSubtract(spot, ent->s.origin, dist);
	height = spot[2] - ent->s.origin[2];
	if(FlatLength(dist) < 40 && height > 40 && ent->groundentity)
	{
		ent->velocity[2] = 200;
		return;
	}

	bot_move(ent, target, spot, BOT_MOVE_NORMAL);
}

void PutBotInServer(edict_t *ent);

void bot_respawn (edict_t *self)
{
	CopyToBodyQue (self);
	PutBotInServer (self);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

	self->bot->respawn_time = level.time;
}

void bot_think (edict_t *ent)
{
	edict_t	*target;
	int bball = ITEM_INDEX(FindItem("Basketballs"));

	target = botAI.target;
	
	// Offense
	if(botAI.poss == ent->bot->resp.team && botAI.targetflag)
		bot_offense(ent, target);
	// Loose Ball
	else if(botAI.targetflag == BT_BALL)
		bot_looseball(ent, target);
	// Defense	
	else if(botAI.poss != NOTEAM)
		bot_defense(ent, target);
	else if(botAI.targetflag == BT_NONE) //jumpball
		bot_looseball(ent, target);

	// Bot Respawning	
	if (ent->deadflag)
	{
		if(level.time > ent->bot->respawn_time)
			bot_respawn(ent);
	}

	ent->nextthink = level.time + .1;
}

void BotUpdateAI(void)
{
	int ball, i;
	edict_t *blip;
	vec3_t path, normal, angles;
	static float	hacktime; //for lazy ass hack
	static qboolean	beginhack;
	
	ball = ITEM_INDEX(FindItem("Basketballs"));

	//do safety check
	if(!botAI.rim1 || !botAI.rim2)
	{
		//gi.dprintf("No rims on map, bots can't think\n");
		return;
	}
	
	//find client with ball
	for (i=0 ; i<game.maxclients ; i++)
	{
		blip = g_edicts + 1 + i;
		
		if( blip->inuse && blip->client && blip->client->pers.inventory[ball])
		{
			botAI.poss = (blip->client->resp.team == TEAM1) ? TEAM1 : TEAM2;
			botAI.target = blip;
			botAI.targetflag = BT_CLIENT;
			beginhack = false;
			return;
		}
	}
	
	//find bot with ball
	for (i=0 ; i<game.maxbots ; i++)
	{
		blip = g_edicts + game.maxclients + 1 + i;
		
		if( blip->inuse && blip->bot && blip->bot->pers.inventory[ball] )
		{
			botAI.poss = (blip->bot->resp.team == TEAM1) ? TEAM1 : TEAM2;
			botAI.target = blip;
			botAI.targetflag = BT_BOT;
			beginhack = false;
			return;
		}
	}
	
	//find free ball
	for (blip = g_edicts; blip < &g_edicts[globals.num_edicts]; blip++)
	{
		if((!blip->inuse) || (blip->clipmask != MASK_BBALL))
			continue;

		botAI.poss = NOTEAM;
		botAI.target = blip;
		botAI.targetflag = BT_BALL;
		beginhack = false;
		return;
	}
	
	//begin lazy ass hack
	if(!beginhack)
	{
		hacktime = level.time;
		if(level.drophack)
			beginhack = true;
	}
	else if(level.time > (hacktime + 7.0) )
	{	//my crappy programming lost the ball, oh well
		gi.bprintf(PRINT_HIGH, "Court confusion!\n");
		hacktime = level.time;
		Jumpball();
	}
	//end lazy ass hack
	
	//see if inbounding
	if(level.inbounding)
	{
		botAI.poss = level.inbounding;
		botAI.targetflag = BT_NONE;
		//target is rim close to inbounding point
		botAI.target = (level.inbounding - 1) ? botAI.rim1 : botAI.rim2;
		return;
	}
	
	//none of these, then jumpball?
	botAI.target = G_Find (NULL, FOFS(classname), "misc_dropball");
	botAI.targetflag = BT_NONE;
}

void InitBotAI(void)
{
	vec3_t	forward, right;
	
	botAI.rim1 = G_Find (NULL, FOFS(targetname), "rim1");
	botAI.rim2 = G_Find (NULL, FOFS(targetname), "rim2");

	//give warning
	if(!botAI.rim1 || !botAI.rim2)
	{
		gi.dprintf("rims not on map, bots can't think\n");
		return;
	}

	//team1 spots
	AngleVectors(botAI.rim1->s.angles,forward,right,NULL);
	VectorInverse(forward);
	VectorInverse(right);

	VectorMA(botAI.rim1->s.origin, 300, forward, bot1_off_spot[0]);
	VectorMA(botAI.rim1->s.origin, 150, forward, bot1_off_spot[1]);
	VectorMA(bot1_off_spot[1], -250, right, bot1_off_spot[1]);
	VectorMA(botAI.rim1->s.origin, 150, forward, bot1_off_spot[2]);
	VectorMA(bot1_off_spot[2], 250, right, bot1_off_spot[2]);

	//team2 spots
	AngleVectors(botAI.rim2->s.angles,forward,right,NULL);
	VectorInverse(forward);
	VectorInverse(right);

	VectorMA(botAI.rim2->s.origin, 300, forward, bot2_off_spot[0]);
	VectorMA(botAI.rim2->s.origin, 150, forward, bot2_off_spot[1]);
	VectorMA(bot2_off_spot[1], -250, right, bot2_off_spot[1]);
	VectorMA(botAI.rim2->s.origin, 150, forward, bot2_off_spot[2]);
	VectorMA(bot2_off_spot[2], 250, right, bot2_off_spot[2]);
}

void bot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->bot->weapon_sound = 0;

	self->maxs[2] = -8;

	self->svflags |= SVF_DEADMONSTER;

	//drop ball when dead
	if(self->bot->pers.inventory[ITEM_INDEX(FindItem("Basketballs"))])
	{
		vec3_t	offset, temp, forward, right;

		VectorSet(offset, 8, 8, self->viewheight-8);
		VectorAdd(self->s.origin, offset, offset);
		VectorCopy(self->s.angles,temp);
		temp[0] = -60.0;
		AngleVectors (temp, forward, right, NULL);

		fire_bball (self, offset, forward, 200, false);
	}
		
	//get rid of ball, causes null model errors
	self->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] = 0;

	if (!self->deadflag)
	{
		self->bot->respawn_time = level.time + 1.0;
//		ClientObituary (self, inflictor, attacker);
	}

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->bot->anim_priority = ANIM_DEATH;
		self->bot->anim_end = 0;

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->bot->anim_priority = ANIM_DEATH;
			switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->bot->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->bot->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->bot->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

void bot_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->bot->anim_priority < ANIM_PAIN && self->s.modelindex == 255)
	{
		static int		i;
	
		self->bot->anim_priority = ANIM_PAIN;
		
		i = (i+1)%3;
		switch (i)
		{
		case 0:
			self->s.frame = FRAME_pain101;
			self->bot->anim_end = FRAME_pain104;
			break;
		case 1:
			self->s.frame = FRAME_pain201;
			self->bot->anim_end = FRAME_pain204;
			break;
		case 2:
			self->s.frame = FRAME_pain301;
			self->bot->anim_end = FRAME_pain204;
			break;
		}
	}

	// play an apropriate pain sound
	if (level.time > self->pain_debounce_time)
	{
		int r,l;

		r = 1 + (rand()&1);
		self->pain_debounce_time = level.time + 0.7;
		if (self->health < 25)
			l = 25;
		else if (self->health < 50)
			l = 50;
		else if (self->health < 75)
			l = 75;
		else
			l = 100;
		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
	}

}

//============================================================
//
//	bot frame handling, skins, weapon, etc.
//
//============================================================

void G_SetBotEvent (edict_t *ent)
{
//FIXME fix footstep, add ballbounce here?
	if (ent->s.event)
		return;

//	if ( ent->groundentity && xyspeed > 225)
//	{
//		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
//			ent->s.event = EV_FOOTSTEP;
//	}
}

void G_SetBotEffects(edict_t *ent)
{
	ent->s.effects &= ~(EF_FLAG1 | EF_FLAG2);
	if (ent->health > 0)
	{
		if (ent->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))]
			&& ent->bot->resp.team == 1)
			ent->s.effects |= EF_FLAG1;
		if (ent->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))]
			&& ent->bot->resp.team == 2)
			ent->s.effects |= EF_FLAG2;
	}
}

void BotBallShowCheck(edict_t *ent)
{
	int i;
	
	if(!ent->bot->pers.inventory[ent->bot->ammo_index])
	{
		ent->s.modelindex2 = 0;
	}
	else
	{
		ent->s.modelindex2 = 255;
		i = ((ent->bot->pers.weapon->weapmodel & 0xff) << 8);
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
}

void G_SetBotFrame (edict_t *ent)
{
	float		xyspeed;
	gbot_t		*bot;
	qboolean	run;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	bot = ent->bot;

	//hands are always up, so update weapons model every frame
	//check for health first or we will crash
	if(ent->health > 0)
		BotBallShowCheck(ent);

	//see if running
	if(ent->speed)
		run = true;
	else
		run = false;

	// check for stand and stop/go transitions
	if (run != bot->anim_run && bot->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && bot->anim_priority <= ANIM_WAVE
		//No newanim if dunking
		&& bot->anim_priority != ANIM_DUNK)
		goto newanim;

	if(bot->anim_priority == ANIM_REVERSE)
	{
		if(ent->s.frame > bot->anim_end)
		{
			ent->s.frame--;
			return;
		}
	}
	else if (ent->s.frame < bot->anim_end)
	{	// continue an animation
		ent->s.frame++;
		return;
	}

	if (bot->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (bot->anim_priority == ANIM_JUMP)
	{
		if (!ent->groundentity)
			return;		// stay there
		ent->bot->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->bot->anim_end = FRAME_jump6;
		return;
	}

newanim:
	//for dunk scoring, only place I could find for this code
	if (bot->anim_priority == ANIM_DUNK)
	{
		ent->bot->pers.inventory[ent->bot->ammo_index]--;
		ent->bot->resp.score += 2;

		if (ent->bot->resp.team == 1)
		{
			
			level.ballteam1 += 2;
			gi.bprintf(PRINT_HIGH, "%s slams 2 points for %s\n",ent->bot->pers.netname, team1name->string);
			Inbound(2);
		}
		else
		{
			level.ballteam2 += 2;
			gi.bprintf(PRINT_HIGH, "%s slams 2 points for %s\n",ent->bot->pers.netname, team2name->string);
			Inbound(1);
		}

		//set last scorer
		level.lastscorer = ent;
	}

	// return to either a running or standing frame
	bot->anim_priority = ANIM_BASIC;
	bot->anim_run = run;

	if (!ent->groundentity)
	{
		bot->anim_priority = ANIM_JUMP;
		if (ent->s.frame != FRAME_jump2)
			ent->s.frame = FRAME_jump1;
		bot->anim_end = FRAME_jump2;
	}
	else if (run)
	{	// running
		//use player frames here
		if ((ent->bot->pers.weapon->weapmodel == WEAP_HANDS)
				&& (ent->bot->pers.inventory[ent->bot->ammo_index]))
		{
			ent->s.frame = FRAME_ballrun1;
			bot->anim_end = FRAME_ballrun6;
		}
		else
		{
			ent->s.frame = FRAME_run1;
			bot->anim_end = FRAME_run6;
		}
		//M.S.
	}
	else
	{	// standing
		//use player frames here
		if ((ent->bot->pers.weapon->weapmodel == WEAP_HANDS)
				&& (ent->bot->pers.inventory[ent->bot->ammo_index]))
		{
			ent->s.frame = FRAME_ballstand01;
			bot->anim_end = FRAME_ballstand10;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			bot->anim_end = FRAME_stand40;
		}
		//M.S.
	}
}

void BotEndServerFrame(edict_t *ent)
{
//FIXME
//	G_CheckChaseStats(ent);

//	G_SetBotEvent (ent);

	G_SetBotEffects (ent);

	G_SetBotFrame (ent);
}

void BotEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<game.maxbots ; i++)
	{
		ent = g_edicts + game.maxclients + 1 + i;

		if (!ent->inuse || !ent->bot)
			continue;
		BotEndServerFrame (ent);
	}

	BotUpdateAI();
}

//============================================================
//
//	bot spawning/killing etc.
//
//============================================================

void BotEvenTeams(void)
{
	edict_t *ent;
	int i, team, team1size, team2size;
	qboolean failed = false;

	team1size = team2size = 0;
	for(i = 0; i<game.maxclients+game.maxbots; i++)
	{
		ent = g_edicts + 1 + i;

		if(ent->inuse && ent->solid)
		{
			team = (ent->client) ? ent->client->resp.team : ent->bot->resp.team;
			if(team == TEAM1)
				team1size++;
			else if(team == TEAM2)
				team2size++;
		}
	}	
	
	while(team1size != fixteamsize->value && !failed)
	{
		if(team1size < fixteamsize->value)
		{
			if(!SP_bball_bot(TEAM1))
				failed = true;
			team1size++;
		}
		else if(team1size > fixteamsize->value)
		{
			if(!Kill_bball_bot(TEAM1))
				failed = true;
			team1size--;
		}
		else
			failed = true;
	}

	while(team2size != fixteamsize->value)
	{
		if(team2size < fixteamsize->value)
		{
			if(!SP_bball_bot(TEAM2))
				return;
			team2size++;
		}
		else if(team2size > fixteamsize->value)
		{
			if(!Kill_bball_bot(TEAM2))
				return;
			team2size--;
		}
		else
			return;
	}
}

void AssignBotSkin(edict_t *ent)
{
	int botnum;
	char t[64];

	strcpy(t, "male/");

	//this is our bot number variable
	botnum = ent-g_edicts-1;;

	switch (ent->bot->resp.team)
	{
		case TEAM1:
			gi.configstring (CS_PLAYERSKINS+botnum,
				va("%s\\%s%s", ent->bot->pers.netname, t, TEAM1_SKIN) );
			break;
		case TEAM2:
			gi.configstring (CS_PLAYERSKINS+botnum,
				va("%s\\%s%s", ent->bot->pers.netname, t, TEAM2_SKIN) );
			break;
		default:
			gi.configstring (CS_PLAYERSKINS+botnum, 
				va("%s\\%s%s", ent->bot->pers.netname, t, TEAM1_SKIN) );
			break;
	}
}

void FetchBotEntData (edict_t *ent)
{
	ent->health = ent->bot->pers.health;
	ent->max_health = ent->bot->pers.max_health;
	ent->flags |= ent->bot->pers.savedFlags;
}

void InitBotPersistant (gbot_t *bot)
{
	gitem_t		*item;

	memset (&bot->pers, 0, sizeof(bot->pers));

	//New weapon selection
	item = FindItem("Hands");
	bot->pers.selected_item = ITEM_INDEX(item);
	bot->pers.inventory[bot->pers.selected_item] = 1;
	bot->pers.weapon = item;
	bot->ammo_index = ITEM_INDEX(FindItem("Basketballs"));

	bot->pers.health			= 100;
	bot->pers.max_health		= 100;

	bot->pers.max_bullets	= 200;
	bot->pers.max_shells		= 100;
	bot->pers.max_rockets	= 50;
	bot->pers.max_grenades	= 50;
	bot->pers.max_cells		= 200;
	bot->pers.max_slugs		= 50;
	bot->pers.max_bballs		= 1;

	bot->pers.connected = true;
}

void InitBotResp (gbot_t *bot)
{
	int		team_val;

	team_val = bot->resp.team;
	memset (&bot->resp, 0, sizeof(bot->resp));
	bot->resp.team = team_val;
	bot->resp.enterframe = level.framenum;
}

void PutBotInServer(edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	vec3_t	spawn_origin, spawn_angles;
	client_persistant_t	saved;
	client_respawn_t	resp;
	gbot_t	*bot;
	char	*name;
	int		index;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this bot
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-game.maxclients-1;
	bot = ent->bot;

	// deathmatch wipes most bot data every spawn
	resp = bot->resp;
	memset (bot, 0, sizeof(*bot));
	InitBotPersistant(bot);
	bot->resp = resp;
	//give them their name
	name = bot_names[(index)%MAX_BOT_NAMES];
	strncpy (bot->pers.netname, name, sizeof(bot->pers.netname)-1);

	AssignBotSkin(ent);
	
	// copy some data from the bot to the entity
	FetchBotEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->bot = &game.bots[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_STEP;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "bot";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_MONSTERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = bot_pain;
	ent->die = bot_die;
	ent->think = bot_think;
	ent->nextthink = level.time + .1;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags |= SVF_MONSTER;
	ent->s.renderfx |= RF_FRAMELERP;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->yaw_speed = 60;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	// weapon will always be hands and will start without ball
	ent->s.modelindex2 = 0;		// custom gun model
	// sknum is botnum and weapon number
	ent->s.skinnum = ent-g_edicts-1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);
}

qboolean SP_bball_bot(int Team)
{
	edict_t		*ent;
	int			i;
	
	//find open bot edict
	for (i=0 ; i<game.maxbots ; i++)
	{
		ent = g_edicts + game.maxclients + 1 + i;
		
		if(ent->inuse)
			continue;
		else
		{
			G_InitEdict (ent);
			InitBotResp (ent->bot);
			ent->bot->resp.team = Team;
			PutBotInServer (ent);
			//reassign defensive matchups
			bot_defense_matchup();
			return true;
		}
	}
	
	//no open spots
	gi.dprintf("Reached bot limit\n");
	return false;
}

qboolean Kill_bball_bot(int Team)
{
	edict_t		*ent;
	int			i;
	int			botnum;	

	for (i=0 ; i<game.maxbots ; i++)
	{
		ent = g_edicts + game.maxclients + 1 + i;

		if(ent->inuse && ent->bot->resp.team == Team)
		{
			// send effect
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGOUT);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.unlinkentity (ent);
			ent->s.modelindex = 0;
			ent->s.modelindex2 = 0;
			ent->solid = SOLID_NOT;
			ent->inuse = false;
			ent->classname = "emptybot";
			ent->bot->pers.connected = false;

			botnum = ent-g_edicts-1;
			gi.configstring (CS_PLAYERSKINS+botnum, "");

			//reassign defensive matchups
			bot_defense_matchup();

			//See if bot left with the ball
			if (ent->bot->pers.inventory[ITEM_INDEX(FindItem("basketballs"))])
			{
				gi.bprintf (PRINT_HIGH, "%s left with the ball\n", ent->bot->pers.netname);
				//opposite team inbounds
				Inbound(3 - (ent->bot->resp.team));
			}
			return true;
		}
	}

	gi.dprintf("no bots on %s to remove\n", TeamName(Team));
	return false;
}

void SVCmd_KillBot_f(void)
{
	int team, amount, i;

	//see if team was specified
	if (gi.argc() < 3)
	{
		gi.dprintf("specify bot's team! ex: bot red\n");
		return;
	}
	else
	{
		if (Q_stricmp(gi.argv(2), "red") == 0)
			team = TEAM1;
		else if (Q_stricmp(gi.argv(2), "blue") == 0)
			team = TEAM2;
		else
		{
			gi.dprintf("unknown team\n");
			return;
		}
			
		if (gi.argc() == 4)
			amount = atoi(gi.argv(3));
		else
			amount = 1;
	}

	for(i = 0; i < amount; i++)
	{
		if(!Kill_bball_bot(team))
			return;
	}
}

void SVCmd_SpawnBot_f(void)
{
	int team, amount, i;

	//see if team was specified
	//console entry can look like this: sv bot red 3
	if (gi.argc() < 3)
	{
		gi.dprintf("specify bot's team! ex: bot red\n");
		return;
	}
	else
	{
		if (Q_stricmp(gi.argv(2), "red") == 0)
			team = TEAM1;
		else if (Q_stricmp(gi.argv(2), "blue") == 0)
			team = TEAM2;
		else
		{
			gi.dprintf("unknown team! ex: bot red\n");
			return;
		}
			
		if (gi.argc() == 4)
			amount = atoi(gi.argv(3));
		else
			amount = 1;
	}

	for(i = 0; i < amount; i++)
	{
		if(!SP_bball_bot(team))
			return;
	}
}

void Cmd_BotList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i=0 ; i<game.maxbots ; i++)
	{
		e2 = g_edicts + game.maxclients + 1 + i;

		if (!e2->inuse || !e2->bot)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d  BOT %3d %s\n",
			(level.framenum - e2->bot->resp.enterframe) / 600,
			((level.framenum - e2->bot->resp.enterframe) % 600)/10,
			e2->bot->resp.score,
			e2->bot->pers.netname);
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}
