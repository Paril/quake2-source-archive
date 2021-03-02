#include "g_local.h"
#include "m_player.h"
#include "laser.h"

void CTFDeadDropFlag(edict_t *self);
void CheatChecking (edict_t *ent, usercmd_t *ucmd);
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void Check_ClassSkin (edict_t *ent); // perfom skin changing penalty
void ClassFunction (edict_t *ent, int i);
void CTFOpenClassMenu(edict_t *ent);
void Cmd_Team_f (edict_t *ent);
void Grenade_Explode (edict_t *ent);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SP_misc_teleporter_dest (edict_t *ent);
void CTFOpenTeamMenu (edict_t *ent);
void weapon_grenade_fire (edict_t *self, qboolean held);
void ChasecamRemove (edict_t *ent, char *opt);

qboolean SV_FilterPacket (char *from);
extern int MeanOfDeath;

edict_t *SelectTeamSpawnPoint (edict_t *ent);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}

// BEGIN FUNNAMES

char* low2high (char *in)
{
	int i = 0;
	char *out=gi.TagMalloc(sizeof(in), TAG_LEVEL);

	for (i = 0; in[i]; i++)
		if (in[i] != '\n') out[i] = in[i] + 128;
		else out[i] = in[i];

	out[i] = 0;
	return out;
}

// END FUNNAMES


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!G_ClientExists(ent))
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	ff = meansOfDeath & MOD_FRIENDLY_FIRE;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	message = NULL;
	message2 = "";

	if (mod == MOD_ELSE)
		return;

	switch (mod)
	{
	case MOD_SUICIDE:

    if ((int)(ripflags->value) & RF_SUICIDE_KICK)
       message = "kicked for suiciding";
    else
	   message = "suicides";
    break;
	case MOD_FALLING:
		message = "cratered";
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
	case MOD_SLIME:
		message = "melted";
		break;
	case MOD_LAVA:
		message = "does a back flip into the lava";
		break;
	case MOD_EXPLOSIVE:
	case MOD_BARREL:
		message = "blew up";
		break;
	case MOD_EXIT:
		message = "found a way out";
		break;
	case MOD_TARGET_LASER:
		message = "watches Jean-Michel Jarre laser-show";
		break;
       case MOD_ZYLON_GAS: 
		message = "was gassed"; 
		break; 
	case MOD_TARGET_BLASTER:
		message = "got blasted";
		break;
	case MOD_BOMB:
	case MOD_SPLASH:
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
	}

	if (attacker == self)
	{
		switch (mod)
		{
		case MOD_HELD_GRENADE:
			message = "tried to put the pin back in";
			break;
		case MOD_HG_SPLASH:
		case MOD_G_SPLASH:
			if (IsFemale(self))
				message = "tripped on her own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_R_SPLASH:
			if (IsFemale(self))
				message = "blew herself up";
			else
				message = "blew himself up";
			break;
		case MOD_BFG_BLAST:
			message = "should have used a smaller gun";
			break;
           case MOD_FIRE_SPLASH:
           case MOD_FIREBALL:
			   if (IsFemale(self))
				   message = "burned herself";
			   else
				   message = "burned himself";
	   	   break;
        case MOD_ON_FIRE:
           message = "became toast";
		   break;
        case MOD_PIPEBOMB:
           message = "searches for his legs";
	       break;
		default:
			if (IsFemale(self))
				message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);

			self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_ELSE:
				message = "was killed by";
       			break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_PLASMA:
				message = "was charbroiled by";
				message2 = "'s plasma grenade";
				break;
			case MOD_ELECTRIC:
				message = "was caught in act by";
				message2 = "'s plasma rifle";
				break;
			case MOD_RAILBOMB:
				message = "was perforated by";
				message2 = "'s rail grenade";
				break;
            case MOD_ZYLON_GAS: 
				message = "couldn't hide from"; 
				message2 = "'s leapfrog grenade"; 
				break; 
            case MOD_FIRE_SPLASH:
				message = "was scorched by";
				break;
            case MOD_ON_FIRE:
                message = "was cremated by";
				break;
            case MOD_FIREBALL:
                message = "got flamed by";
				break;
            case MOD_PIPEBOMB:
                message = "flies with";
				message2 = "'s pipebomb";
				break;
			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s ", self->client->pers.netname);

				gi.bprintf (PRINT_MEDIUM,"%s", message);

				gi.bprintf (PRINT_MEDIUM," %s ", attacker->client->pers.netname);

				gi.bprintf (PRINT_MEDIUM,"%s\n", message2);

				if (ff)
					attacker->client->resp.score--;
				else
				{
                   if (!(int) ripflags->value & RF_NO_FRAG)
					   attacker->client->resp.score++;
				}
			}
			  return;
		}


	if (stricmp(attacker->classname, "sentry") == 0)
	{
		if (MeanOfDeath == 1)
			gi.bprintf (PRINT_MEDIUM, "%s was cut in half by %s's sentrygun\n", self->client->pers.netname, attacker->creator->client->pers.netname);
		else if (MeanOfDeath == 2)
		{
			if (self->health < -40) // gib
				gi.bprintf (PRINT_MEDIUM, "%s was gibbed by %s's sentrygun\n", self->client->pers.netname, attacker->creator->client->pers.netname);
			else
				gi.bprintf (PRINT_MEDIUM, "%s rides %s's sentrygun rocket\n", self->client->pers.netname, attacker->creator->client->pers.netname);
		}
		else if (MeanOfDeath == 3)
		{
			if (self->health < -40) // gib
				gi.bprintf (PRINT_MEDIUM, "%s was perforated by %s's sentrygun\n", self->client->pers.netname, attacker->creator->client->pers.netname);
			else
				gi.bprintf (PRINT_MEDIUM, "%s was railed by %s's sentrygun\n", self->client->pers.netname, attacker->creator->client->pers.netname);
		}

		MeanOfDeath = 0;

		if (attacker->creator != NULL)
		{
            if (!(int)ripflags->value & RF_NO_FRAG)
      			attacker->creator->client->resp.score++;
		}
		return;
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);

	self->client->resp.score--;
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Flame Launcher") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Flamethrower") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Pipebomb Launcher") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*blip = NULL;
	int n;

	while ((blip = G_Find (blip, FOFS(classname), "pipebomb")) != NULL)
	{
		if (G_EntExists (blip->owner) && (blip->owner != self))
			continue;

		blip->think (blip);
	}	
	
	if (G_EntExists(self->lasersight))
		G_FreeEdict (self->lasersight);

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked ctf flag

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;
	self->client->pers.selected_item = 0;
	self->client->SonicDamage = 0;

	self->maxs[2] = -8;
    self->ripstate = 0;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

		CTFFragBonuses(self, inflictor, attacker);
        CTFDeadDropFlag(self);
		
        if (self->client->chasetoggle)
            ChasecamRemove (self, "off");

		if (!self->client->showscores)
			Cmd_Help_f(self);

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			self->client->pers.inventory[n] = 0;
		}
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
    	for (n= 0; n < 4; n++)
				ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}


	self->deadflag = DEAD_DEAD;
	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	memset (&client->pers, 0, sizeof(client->pers));

	client->pers.max_health		= 1;
	client->pers.health	= client->pers.max_health;

	client->pers.max_bullets	= 0;
	client->pers.max_shells		= 0;
	client->pers.max_rockets	= 0;
	client->pers.max_grenades	= 0;
	client->pers.max_cells		= 0;
	client->pers.max_slugs		= 0;

    // Init Flood Protection Variables 
    client->flood_num_msgs = 0; 
    client->flood_post_time = 0; 
    client->flood_timer = 0; 

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint1 (edict_t *ent)
{
	edict_t	*spot = NULL;

    if (teamplay->value)
        spot = SelectTeamSpawnPoint (ent);
	else if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

}

void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

    spot = SelectTeamSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				spot = world;

				//gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 2;
	VectorCopy (spot->s.angles, angles);
}

/*
================
SelectTeamSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectTeamSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	switch (ent->client->resp.s_team)
	{
	case 1:
		cname = "info_player_team1";
		break;
	case 2:
		cname = "info_player_team2";
		break;
/*
	default:
		return SelectRandomDeathmatchSpawnPoint();
*/
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;


	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
    body->svflags = ent->svflags | SVF_MONSTER | SVF_DEADMONSTER;
    body->monsterinfo.aiflags |= AI_GOOD_GUY;
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = NULL;
	body->s.modelindex2 = 0;
	body->movetype = ent->movetype;

    if (ent->health > 0)
        body->health = 0;
    else
        body->health = ent->health;
    body->gib_health = ent->gib_health;
    body->mass = ent->mass;
    body->burnout = ent->burnout;

    if (ent->burner)
    {
		body->burner = ent->burner;
        body->burner->enemy = body;
        ent->burner = NULL;
    }
	
    body->fireflags = ent->fireflags & ~FIREFLAG_DELTA_VIEW;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);

}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			stuffcmd (ent, "spectator 0\n");
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			stuffcmd(ent, "spectator 0\n");
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			stuffcmd (ent, "spectator 1\n");
			return;
		}
	}

	// clear client on respawn
	ent->client->resp.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) 
	{
		gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
        CTFDeadDropFlag(ent);
		ent->client->resp.s_team = 0;
		ent->playerclass = ent->lastclass = 0;
	}
	else
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void respawn (edict_t *self)
{
// Added by WarZone - Begin 
     if (self->client->oldplayer) 
         G_FreeEdict (self->client->oldplayer); 

     if (self->client->chasecam) 
         G_FreeEdict (self->client->chasecam); 
// Added by WarZone - End 

	if (self->movetype != MOVETYPE_NOCLIP)
		CopyToBodyQue (self);

	self->svflags &= ~SVF_NOCLIENT;

	ClassFunction (self, self->lastclass);

	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void AppearMenu (entity *menu)
{
	Cmd_Team_f (menu->owner);
	G_FreeEdict (menu);
}

void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {0, 0, 0};
	vec3_t	maxs = {0, 0, 0};
	vec3_t	spawn_origin, spawn_angles;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	char		userinfo[MAX_INFO_STRING];
	FILE *motd_file;
	char motd[500];
	char line[80];
	char s[32];
    cvar_t *game_t = gi.cvar ("gamedir", "mod-1", CVAR_SERVERINFO);
	entity *menu;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client

    SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	resp = ent->client->resp;
	memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant (ent->client);
	ClientUserinfoChanged (ent, userinfo);

	// clear everything but the persistant data
	saved = ent->client->pers;
	memset (ent->client, 0, sizeof(*ent->client));
	ent->client->pers = saved;
	ent->client->resp = resp;
	if (ent->client->pers.health <= 0)
		InitClientPersistant (ent->client);

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_NO;
	ent->client = &game.clients[ent-g_edicts-1];
	ent->movetype = MOVETYPE_NOCLIP;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_NOT;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->model = "";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->playerclass = 0;
	ent->ripstate = 0;
	ent->svflags |= SVF_NOCLIENT;
	ent->teamstate = 0;
	ent->burnout = 0;
    ent->gib_health = -41;
    ent->fireflags = 18 | FIREFLAG_DOWN | FIREFLAG_DELTA_VIEW | FIREFLAG_IGNITE;
    ent->style = 1;
	ent->ClassSpeed = 200;

	if (((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// spawn a spectator
	if (ent->client->pers.spectator) {
		ent->client->chase_target = NULL;

		ent->client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		return;
	}
	else
		ent->client->resp.spectator = false;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(ent->client->ps));

	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;

	ent->client->ps.fov = 90;

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.frame = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 2;	// make sure off ground

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - ent->client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
	VectorCopy (ent->s.angles, ent->client->v_angle);

	ent->client->blindBase = 0;
    ent->client->blindTime = 0;
	ent->client->respawn_time = level.time;
	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	strcpy (s, game_t->string);
	strcat (s, "/motd.txt");
	// STEVE changed this bit : read the motd from a file
	if (motd_file = fopen(s, "r"))
	{
		// we successfully opened the file "motd.txt"
		if (fgets(motd, 500, motd_file) )
		{
			// we successfully read a line from "motd.txt" into motd
			// ... read the remaining lines now
			while ( fgets(line, 80, motd_file) )
			{				// add each new line to motd, to create a BIG message string.
				// we are using strcat: STRing conCATenation function here.
				strcat(motd, line);
			}			// print our message.
			gi.centerprintf (ent, "Rip II 2.2\nwww.captured.com/rip2/\n%s\n", motd);
		}
		// be good now ! ... close the file
		fclose(motd_file);
	}
	else
		gi.centerprintf (ent, "Rip II 2.2\nwww.captured.com/rip2/\n");

	menu = G_Spawn();
	menu->owner = ent;
	menu->think = AppearMenu;
	menu->nextthink = level.time + 2.4;
	gi.linkentity (menu);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	game_import_t s;

	s = gi;

	G_InitEdict (ent);

	InitClientResp (ent->client);

    // locate ent at a spawn point
    PutClientInServer (ent);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
}



/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;
    gclient_t *client;

	client = ent->client;

	ent->client = game.clients + (ent - g_edicts - 1);

	ClientBeginDeathmatch (ent);
	return;

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
/*        if (ent->client->pers.funname[0])
			gi.bprintf (PRINT_HIGH, "%s entered the world\n", ent->client->pers.funname);
		else*/
			gi.bprintf (PRINT_HIGH, "%s entered the world\n", ent->client->pers.netname);
		}
	}
	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (*s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	if (ent->ripstate & STATE_SKIN)
	{
		// Have they changed their skin?
		if (stricmp (s, ent->skincheck)) // yes, the have
		{
			Info_SetValueForKey(ent->client->pers.userinfo, "rejmsg", "Kicked for changing skin.");
			Rip_SetSkin (ent);
			gi.bprintf (PRINT_HIGH, "%s was kicked for changing his skin\n", ent->client->pers.netname);
			stuffcmd (ent, "disconnect\n");
			return;
		}
	}

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
		ent->client->pers.hand = atoi(s);

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (strcmp(password->string, value))
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
//	if (deathmatch->value && strcmp(value, "0"))
	if (*value && strcmp(value, "0"))
	{
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value))
		{
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
		{
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;
		}

		if (numspec >= maxspectators->value)
		{
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	}
	else
	{
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value))
		{
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}

// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	ent->client->pers.radio_power = 0;

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ent->client->resp.spectator = false;

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;

    Menu_Init (ent);
    return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!G_ClientExists(ent))
		return;
 	
	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

	if (ent->client->chasecam)
		G_FreeEdict (ent->client->chasecam);

	if (ent->client->oldplayer)
		G_FreeEdict (ent->client->oldplayer);

	CTFDeadDropFlag(ent);
	// send effect
	muzzleflash (ent, MZ_LOGOUT);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag || (ent->ripstate & STATE_FEINING))
		client->ps.pmove.pm_type = PM_DEAD;
	else if (ent->teamstate & STATE_BUILDING)
		client->ps.pmove.pm_type = PM_FREEZE;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
	}

    if (ucmd->forwardmove > ent->ClassSpeed)
	{
		ucmd->forwardmove = ent->ClassSpeed;
		Rip_SetSpeed(ent);
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{ 
     	VectorCopy (pm.viewangles, client->v_angle); 
		VectorCopy (pm.viewangles, client->ps.viewangles); 
	}

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;

			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	if (ent->client->chasetoggle)
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;

	if (level.intermissiontime)
		return;
	
	client = ent->client;

	if (client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			if ( ( client->latched_buttons & BUTTON_ATTACK ) ||
				(((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	client->latched_buttons = 0;
}
