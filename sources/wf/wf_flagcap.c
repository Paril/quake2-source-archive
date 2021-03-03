/*==============================================================================
The Weapons Factory - 
Team Fortress Style Flag Capture Functions
Original code by Gregg Reno
==============================================================================*/
#include "g_local.h"
#include "stdlog.h"	//	StdLog - Mark Davies

extern gitem_t *flag1_item;
extern gitem_t *flag2_item;
extern ctfgame_t ctfgame;

void CTFResetFlags(void);
void CTFResetFlag(int ctf_team);
void WFRemoveDisguise(edict_t *ent);
int PlayerChangeScore(edict_t *self, int points);

qboolean WFUnbalanced_Teams(edict_t *flag, edict_t *player)
{
	int team_limit;
	int flagteamcount = 0, playerteamcount = 0;
	int i;
	int diff;
	edict_t *target;
	int flagteam;

	team_limit = wf_game.unbalanced_limit;

	// figure out what team this flag is
	if (strcmp(flag->classname, "item_flag_team1") == 0)
		flagteam = CTF_TEAM1;
	else if (strcmp(flag->classname, "item_flag_team2") == 0)
		flagteam = CTF_TEAM2;
	else 
		flagteam = 0;


	//Is any limit set?
	if (team_limit <= 0) 
	{
//gi.dprintf("No Limit Set\n");
		return false;
	}

	//Teams must be set
	if ((flagteam == 0) || (player->wf_team == 0)) 
	{
//gi.dprintf("Someone doesn't have their wf_team flag set: %d, %d\n",
//		   flagteam, player->wf_team );
		return false;
	}

	//Must be on different teams
	if (flagteam == player->wf_team == 0)

	//Count players on each team
	for (i = 1; i <= maxclients->value; i++) 
	{
		target = &g_edicts[i];

		if (!target->inuse)
			continue;

		if (target->wf_team == flagteam )
			flagteamcount++;

		if (target->wf_team == player->wf_team)
			playerteamcount++;
	}

	diff = playerteamcount - flagteamcount;
//gi.dprintf("Diff=%d, playercount = %d, flagcount = %d, team limit = %d\n",
//	diff, playerteamcount,flagteamcount,team_limit);

	if (diff >= team_limit)
	{
		safe_cprintf(player, PRINT_HIGH, "Sorry, you can't pick up flag.  Teams are not balanced.\n");
		return true;
	}
	else 
		return false;
}

qboolean WFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	gitem_t *flag_item, *enemy_flag_item;

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
	{
		ctf_team = CTF_TEAM1;
	}
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
	{
		ctf_team = CTF_TEAM2;
	}
	else 
	{
		safe_cprintf(other, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1) 
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} 
	else 
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	//Cant create decoy in observer/spectator mode
	if (other->solid == SOLID_NOT)
	{
		safe_cprintf(other, PRINT_HIGH, "Nice try, but observers can't pick up the flag!.\n");
		return false;
	}


	if (other->client->resp.ctf_team != CTF_TEAM1 &&
		other->client->resp.ctf_team != CTF_TEAM2)
	{
		safe_cprintf(other, PRINT_HIGH, "I don't know how you did it, but you don't have a team!.\n");
		return false;
	}

	//Note - you can't pick up your own flag - just the enemy's
	if (ctf_team != other->client->resp.ctf_team) 
	{

		// hey, its not our flag, pick it up
		if (PlayerChangeScore(other,CTF_FLAG_BONUS))
		{
			gi.bprintf(PRINT_HIGH, "%s got the %s flag!\n",
				other->client->pers.netname, CTFTeamName(ctf_team));

			// Log Flag Pickup - MarkDavies
			sl_LogScore( &gi,
				other->client->pers.netname,
				NULL,
				"F Pickup",
				-1,
				CTF_FLAG_BONUS);
		}

		if (ctf_team == CTF_TEAM1)
			flag1dropped = 0;		//clear the dropped flag flag
		else if (ctf_team == CTF_TEAM2)
			flag2dropped = 0;

		other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
		other->client->resp.ctf_flagsince = level.time;

		//Remove player disguise
		if (other->disguised) WFRemoveDisguise(other);

		// pick up the flag
		// if it's not a dropped flag, we just make is disappear
		// if it's dropped, it will be removed by the pickup caller
		if (!(ent->spawnflags & DROPPED_ITEM)) 
		{
			ent->flags |= FL_RESPAWN;
			ent->svflags |= SVF_NOCLIENT;
			ent->solid = SOLID_NOT;
		}
		return true;
	}
	return false;
}

//Something touched home base
void Home_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int i;
	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item;

	//Did someone try to put something on top of the base?  If so, blow it up!
	if (!strcmp(other->classname, "SentryGun") )
		{
			turret_remove(other->creator);
		}
	if (!strcmp(other->classname, "SentryStand") )
		{
			turret_remove(other->standowner);
		}
	if (!strcmp(other->classname, "depot") )
		{
			if (other->owner) other->owner->supply = NULL;
			//G_FreeEdict(other);
			BecomeExplosion1 (other);
		}
	if (!strcmp(other->classname, "healingdepot") )
		{
			if (other->owner) other->owner->supply = NULL;
			//G_FreeEdict(other);
			BecomeExplosion1 (other);
		}

	//Did a player touch?
	if (!other->client)
		return;

	//Home base and player must be on same team
	if (ent->wf_team != other->wf_team)
		return;

	// same team, if the flag at base, check to see if he has the enemy flag
	if (other->wf_team == CTF_TEAM1) 
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else 
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	//Does the player have the enemy flag?
	if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) 
	{
		//Yes - TOUCHDOWN!
		other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

		ctfgame.last_flag_capture = level.time;
		ctfgame.last_capture_team = other->wf_team;
		if (other->wf_team == CTF_TEAM1)
			ctfgame.team1++;
		else
			ctfgame.team2++;

		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);

		// other gets another 10 frag bonus
		if (PlayerChangeScore(other,CTF_CAPTURE_BONUS))
		{
			gi.bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
				other->client->pers.netname, CTFOtherTeamName(other->wf_team));

			// Log Flag Capture - MarkDavies
			sl_LogScore( &gi,
				other->client->pers.netname,
				NULL,
				"F Capture",
				-1,
				CTF_CAPTURE_BONUS);
		}
		//execute the trigger if it exists
		if (ent->orig_map_entity)
		{
			G_UseTargets (ent->orig_map_entity, other);
//if (wfdebug) gi.dprintf("Using Target %s\n", ent->orig_map_entity->target);
		}
		else
		{
//if (wfdebug) gi.dprintf("No Target\n");
		}

		// Ok, let's do the player loop, hand out the bonuses
		for (i = 1; i <= maxclients->value; i++) 
		{
			player = &g_edicts[i];
			if (!player->inuse)
				continue;
			if (!player->client)
				continue;
			if (!other->client)
				continue;
			if (player->client->resp.ctf_team != other->client->resp.ctf_team)
			{
				player->client->resp.ctf_lasthurtcarrier = -5;
			}
			else if (player->client->resp.ctf_team == other->client->resp.ctf_team) 
			{
				if (player != other)
				{
					if (PlayerChangeScore(player,CTF_TEAM_BONUS))
					{
						// Log Flag Capture Team Score - MarkDavies
						sl_LogScore( &gi,
							player->client->pers.netname,
							NULL,
							"Team Score",
							-1,
							CTF_TEAM_BONUS);
					}
				}
				// award extra points for capture assists
				if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time) 
				{
					if (PlayerChangeScore(player,CTF_RETURN_FLAG_ASSIST_BONUS))
					{
						gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);

						// Log Flag Capture Team Score - MarkDavies
						sl_LogScore( &gi,
							player->client->pers.netname,
							NULL,
							"F Return Assist",
							-1,
							CTF_RETURN_FLAG_ASSIST_BONUS);
					}

				}
				if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time) 
				{
					if (PlayerChangeScore(player,CTF_FRAG_CARRIER_ASSIST_BONUS))
					{

						gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
						// Log Flag Capture Frag Assist Score - MarkDavies
						sl_LogScore( &gi,
							player->client->pers.netname,
							NULL,
							"FC Frag Assist",
							-1,	
							CTF_FRAG_CARRIER_ASSIST_BONUS);
					}

				}
			}
		}
		if (ent->wf_team == CTF_TEAM1)
			CTFResetFlag(CTF_TEAM2);
		else
			CTFResetFlag(CTF_TEAM1);
	}
}

void Home_Think (edict_t *ent)
{
	//For some reason, entity needs a think function
	ent->nextthink = level.time + 5.0;
}


void Create_Home_Base(edict_t *flag, int team)
{
	edict_t *home;

	home = G_Spawn ();

	home->wf_team = team;

//gi.dprintf("--Create Home: %s.  Target = %s, ent = %d\n", flag->classname, flag->target, flag);

	//Save a pointer to the flag/home base entity so the
	//triggers will work
	if ((flag->target) && (flag->target[0] != 0))
		home->orig_map_entity = flag;

	VectorCopy(flag->s.origin, home->s.origin);
	home->s.origin[2] += 40;
	//home->classname="depot";
	home->takedamage=DAMAGE_NO;
	home->movetype= MOVETYPE_TOSS;
	home->mass = 200;
	home->solid = SOLID_BBOX;
	home->clipmask=MASK_ALL;
	home->deadflag =DEAD_NO;
	home->clipmask = MASK_SHOT;
	home->model = flag->model;
	home->s.modelindex = gi.modelindex ("models/objects/flagbase/tris.md2");

	if (team == CTF_TEAM1)
	{
		home->classname="home_team1";
		home->s.skinnum = 0;
	}
	else
	{
		home->classname="home_team2";
		home->s.skinnum = 1;
	}
	
	home->solid = SOLID_BBOX;

	VectorSet (home->mins, -20, -20, -15);
	VectorSet (home->maxs, 20, 20, -13);
	home->s.frame =0;
	home->waterlevel = 0;
	home->watertype=0;
	home->health= 100;
	home->max_health =100;
	home->gib_health = -80;
//	home->die = supplydepot_die;
	home->owner = flag;
	home->dmg = 100;
	home->dmg_radius = 100;
//	home->touch = SupplyTouch;
//	home->think =SupplyThink;
	home->touch = Home_Touch;
	home->think = Home_Think;
	home->nextthink = level.time + 1.0;
	VectorClear (home->velocity);
	gi.linkentity (home);

//	gi.dprintf("FLAG CREATED.\n");
}


void Create_All_Home_Bases (void)
{
	edict_t	*spot;

	spot = NULL;

	//Team 1 home base

	//First try home base (flag return) position
	spot = G_Find (spot, FOFS(classname), "item_flagreturn_team1");

	//Next try flag spot
	if (!spot) spot = G_Find (spot, FOFS(classname), "item_flag_team1");

	if (!spot)
	{
		gi.dprintf("WARNING - Could not place flag return entity for team 1!\n");
	}
	else
	{
		Create_Home_Base(spot, CTF_TEAM1);
	}


	//Team 2 home base

	//First try home base (flag return) position
	spot = NULL;
	spot = G_Find (spot, FOFS(classname), "item_flagreturn_team2");

	//Next try flag spot
	if (!spot) spot = G_Find (spot, FOFS(classname), "item_flag_team2");

	if (!spot)
	{
		gi.dprintf("WARNING - Could not place flag return entity for team 2!\n");
	}
	else
	{  
		if (strcmp(spot->classname, "item_flagreturn_team2") == 0)//4/99
		spot->s.skinnum = 1;//3/99 small fix for g_items.c/bot

		Create_Home_Base(spot, CTF_TEAM2);
	}


}
