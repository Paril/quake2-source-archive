
#include "g_local.h"

#include "bot_procs.h"
#include "g_map_mod.h"
#include "p_trail.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
//ZOID
cvar_t	*capturelimit;
//ZOID
cvar_t	*passwd;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;

cvar_t	*dedicated;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

cvar_t	*bot_num;
cvar_t	*bot_name;
cvar_t	*bot_allow_client_commands;
cvar_t	*bot_free_clients;
cvar_t	*bot_debug;
cvar_t	*bot_show_connect_info;
cvar_t	*bot_calc_nodes;
cvar_t	*bot_debug_nodes;
cvar_t	*bot_auto_skill;
cvar_t	*bot_drop;
cvar_t	*bot_chat;
cvar_t	*bot_optimize;
cvar_t	*bot_tarzan;

cvar_t	*players_per_team;
cvar_t	*addteam;
cvar_t	*teamplay;
cvar_t	*ctf_auto_teams;
cvar_t	*grapple;

cvar_t	*view_weapons;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);


//===================================================================


void WriteTrail ();
void ShutdownGame (void)
{
	WriteTrail();

	gi.dprintf ("==== ShutdownGame ====\n");

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client || ent->bot_client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;

  	///Q2 Camera Begin
    EnitityListClean();
	///Q2 Camera End

	// MAP MOD
	if (map_mod_)
	{
		char *nextmap;

		if (nextmap = map_mod_next_map())
		{	// only use a map_mod map if the current level is known
			strcpy(level.nextmap,nextmap);
		}
	}
	// MAP MOD

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname;
	}
	else if (level.nextmap[0])
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.nextmap;
	}
	else
	{	// search for a changeleve
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = level.mapname;
		}
	}

	BeginIntermission (ent);

	WriteTrail();
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			my_bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
//ZOID
		if (ctf->value) {
			if (CTFCheckRules()) {
				EndDMLevel ();
			}
		}
//ZOID
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				my_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
}

char	respawn_bots[64][256];
bot_team_t	*respawn_bot_teams[64];
int		respawn_ctf_teams[64];
extern int force_team;
qboolean	respawn_init = false, respawn_flag = false;

/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i,j;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	memset(respawn_bots, 0, sizeof(respawn_bots));
	memset(respawn_bot_teams, 0, sizeof(bot_teams));

	// clear some things before going to next level
	j = 0;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;

		// save the list of bots for respawning in next level
		if (ent->bot_client)
		{	// add to list
			Com_sprintf (respawn_bots[j], sizeof(respawn_bots[j]), ent->client->pers.netname);
			if (teamplay->value || !ctf->value)
				respawn_bot_teams[j] = ent->client->team;
			if (ctf->value)
				respawn_ctf_teams[j] = ent->client->resp.ctf_team;

			j++;
		}
	}

	respawn_flag = true;

//ZOID
	CTFInit();
//ZOID
}

void	RemoveDroppedItem(edict_t	*ent);

qboolean AllItemsHaveMovetarget(edict_t *list_head)
{
	edict_t	*trav;

	trav = list_head;
	while (trav)
	{
		if (!trav->movetarget && (trav->think != RemoveDroppedItem))
			return false;

		trav = trav->node_target;
	}

	return true;
}

/*
================
CheckNodeCalcDisable

  Checks at set intervals to see if we can safely disable node calc.
================
*/
void CheckNodeCalcDisable(edict_t *ent)
{
	// check all item lists, to see if they have a movetarget (visible node)
	// BUT only check static items, dropped items don't count
	if (!AllItemsHaveMovetarget(weapons_head))
		goto failure;
	if (!AllItemsHaveMovetarget(health_head))
		goto failure;
	if (!AllItemsHaveMovetarget(bonus_head))
		goto failure;
	if (!AllItemsHaveMovetarget(ammo_head))
		goto failure;

	// all items are visible from a node, so we're done

	gi.cvar_set("bot_calc_nodes", "0");
	gi.dprintf("Dynamic node calculation DISABLED\n");

	G_FreeEdict(ent);
	return;

failure:

	ent->nextthink = level.time + 15;
}

qboolean PlayerNameExists(char *name)
{
	int i,j;
	char	testname[128];

	for (i=0; i<num_players; i++)
	{
		strcpy(testname, players[i]->client->pers.netname);

		for (j=0; testname[j]; j++)
		{
			if (testname[j] == '[')
			{
				testname[j] = 0;
				break;
			}
		}

#ifdef _WIN32
		if (!_stricmp(testname, name))
#else
		if (!strcasecmp(testname, name))
#endif
			return true;
	}

	return false;
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

qboolean execed_launcher_cfg=false;

void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;
	static last_ctf_teams=0;

	if (paused)
		return;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// check for disabling node calc.
	if ((bot_calc_nodes->value) && (level.time > 300) && (!check_nodes_done))
	{	// start checking for disabling node calc
		check_nodes_done = G_Spawn();
		check_nodes_done->think = CheckNodeCalcDisable;
		check_nodes_done->nextthink = level.time + 0.1;
	}

	if (!execed_launcher_cfg && (level.time > 1) && the_client)
	{
		stuffcmd(the_client, "exec launcher.cfg");
		execed_launcher_cfg = true;
	}

	// do BOT stuff

	roam_calls_this_frame = 0;
	bestdirection_callsthisframe = 0;

	if (level.time > 8)
	{	// give the clients some time to reconnect
		if (!respawn_init)
		{
			memset(respawn_bots, 0, 64*256);
			respawn_init = true;
		}
		else if (respawn_flag)
		{	// we need to spawn some bots from the previous level
			int count=0;
			static float	last_spawn=0;

			if (last_spawn > (level.time - 0.3))
				goto no_spawnbots;

			last_spawn = level.time;

			for (i=0; i<64; i++)
			{
				if (respawn_bots[i][0] == '\0')
					continue;

				force_team = respawn_ctf_teams[i];
				ent = spawn_bot(respawn_bots[i]);
				force_team = -1;

				if (ent && respawn_bot_teams[i])
				{
					ent->client->team = respawn_bot_teams[i];
				}

				// set name
				if (ent)
					strcpy(ent->client->pers.netname, respawn_bots[i]);

				respawn_bots[i][0] = '\0';

				if (++count > 2)
					goto no_spawnbots;
			}

			respawn_flag = false;

			goto no_spawnbots;
		}

		// enforce ctf_auto_teams as best as possible, and keep teams even
		if (ctf->value && ctf_auto_teams->value)
		{
			if (last_ctf_teams < (level.time - 1))
			{
				int team1=0, team2=0;
				edict_t *bot_team1=NULL, *bot_team2=NULL;

				// count number of players on each team
				for (i=0; i<num_players; i++)
				{
					if (players[i]->client->resp.ctf_team == CTF_TEAM1)
					{
						team1++;

						if (players[i]->bot_client)
						{
							if (!bot_team1 || (players[i]->client->resp.score < bot_team1->client->resp.score))
								bot_team1 = players[i];
						}
					}
					else if (players[i]->client->resp.ctf_team == CTF_TEAM2)
					{
						team2++;

						if (players[i]->bot_client)
						{
							if (!bot_team2 || (players[i]->client->resp.score < bot_team2->client->resp.score))
								bot_team2 = players[i];
						}
					}
				}

				if (team1 != team2)
				{
					if (team1 > team2)
					{
						if (team1 > ctf_auto_teams->value)
						{
							if (bot_team1)
							{		// drop it
								botDisconnect(bot_team1);
							}
							else	// add a bot to the game, which will get added to team2
							{
								spawn_bot(NULL);
							}
						}
						else	// the game needs more players
						{
							spawn_bot(NULL);
						}
					}
					else	// team2 has more players than team1
					{
						if (team2 > ctf_auto_teams->value)
						{
							if (bot_team2)
							{		// drop it
								botDisconnect(bot_team2);
							}
							else	// add a bot to the game, which will get added to team1
							{
								spawn_bot(NULL);
							}
						}
						else	// the game needs more players
						{
							spawn_bot(NULL);
						}
					}
				}
				else	// teams are equal
				{
					if (team1 > ctf_auto_teams->value)
					{
						if (bot_team1 && bot_team2)
						{
							// drop the lowest scoring bot from team1 & team2
							botDisconnect(bot_team1);
							botDisconnect(bot_team2);
						}
					}
					else if (team1 < ctf_auto_teams->value)
					{
						spawn_bot(NULL);
						spawn_bot(NULL);
					}
				}
			}
		}
		else if (((spawn_bots > 0) || (bot_count < bot_num->value)) && (last_bot_spawn < (level.time - 0.5)))
		{
			if (num_players < (maxclients->value - bot_free_clients->value))
			{
				spawn_bot(NULL);
			}
			else
			{
//				gi.dprintf("Cannot spawn bot: not enough free client spaces (bot_free_clients = %i)\n", bot_free_clients->value);
			}

			if (spawn_bots > 0)
				spawn_bots--;
		}
		else if (strlen(bot_name->string) > 1)
		{
			if (num_players < (maxclients->value - bot_free_clients->value))
			{
				spawn_bot(bot_name->string);
			}
			else
			{
				gi.dprintf("Cannot spawn bot: not enough free client spaces (bot_free_clients = %i)\n", bot_free_clients->value);
			}

			gi.cvar_set("bot_name", "");
		}

		if (!ctf->value && teamplay->value && (strlen(addteam->string) > 0))
		{	// spawn the new team
			// find the team
			i=0;
			while (i<MAX_TEAMS)
			{
				if (!bot_teams[i])
					break;

#ifdef _WIN32
				if (!_stricmp(bot_teams[i]->teamname, addteam->string) || !_stricmp(bot_teams[i]->abbrev, addteam->string))
#else
				if (!strcasecmp(bot_teams[i]->teamname, addteam->string) || !strcasecmp(bot_teams[i]->abbrev, addteam->string))
#endif
				{	// found the team, so add the bots
					bot_teams[i]->ingame = true;	// bots will be added automatically (below)
					break;
				}

				i++;
			}

			if (i == MAX_TEAMS)
			{
				gi.dprintf("Team \"%s\" does not exist.\n", addteam->string);
			}

			gi.cvar_set("addteam", "");
		}

		if (players_per_team->value > MAX_PLAYERS_PER_TEAM)
		{
			char str[16];

			sprintf(str, "%i", MAX_PLAYERS_PER_TEAM);
			gi.cvar_set("players_per_team", str);

			gi.dprintf("MAX_PLAYERS_PER_TEAM = %i\n", MAX_PLAYERS_PER_TEAM);
		}

		// check for a team that needs more players
	if (!ctf->value && teamplay->value)
		for (i=0; i<MAX_TEAMS; i++)
		{
			int bot;
			edict_t *newbot;

			if (!bot_teams[i])
				break;

			if (!bot_teams[i]->ingame)
				continue;

			if (bot_teams[i]->num_players < players_per_team->value)
			{	// try to spawn a new bot

				for (bot=0; bot<players_per_team->value; bot++)
				{
					if (bot_teams[i]->bots[bot])
					{
						if (bot_teams[i]->bots[bot]->ingame_count)
							continue;

						if (PlayerNameExists(bot_teams[i]->bots[bot]->name))
							continue;

						if (strlen(bot_teams[i]->default_skin) > 0)
						{	// overwrite this bot's skin, with the team's default
							strcpy(bot_teams[i]->bots[bot]->skin, bot_teams[i]->default_skin);
						}

						newbot = spawn_bot(bot_teams[i]->bots[bot]->name);

						if (newbot)
						{
							char userinfo[MAX_INFO_STRING];

							bot_teams[i]->num_players++;
							bot_teams[i]->num_bots++;
							newbot->client->team = bot_teams[i];

							// add the abbreviation to the name
							strcat(newbot->client->pers.netname, "[");
							strcat(newbot->client->pers.netname, bot_teams[i]->abbrev);
							strcat(newbot->client->pers.netname, "]");

							// add bot's name to userinfo
							strcpy(userinfo, newbot->client->pers.userinfo);
							Info_SetValueForKey (userinfo, "name", newbot->client->pers.netname);
							ClientUserinfoChanged(newbot, userinfo);

							if (bot_teams[i]->num_players == players_per_team->value)
								break;	// we have enough players
						}
					}
				}
			}
		}
	}

	if ((bot_count > 0) && (num_players > (maxclients->value - bot_free_clients->value)))
	{	// drop a bot to free a client spot
		edict_t	*trav, *lowest=NULL;
		int	i;

		// drop the lowest scoring bot
		for (i=0; i < num_players; i++)
		{
			trav = players[i];

			if (!trav->bot_client)
				continue;

			if (!lowest || (trav->client->resp.score < lowest->client->resp.score))
			{
				lowest = trav;
			}
		}

		if (lowest)		// if NULL, then must be full of real players
		{
			botDisconnect(lowest);
		}
	}

	// Enforce players_per_team->value
	if (!ctf->value && teamplay->value)
	{
		int t;

		for (t=0; t<MAX_TEAMS; t++)
		{
			if (!bot_teams[t])
				break;

			if (bot_teams[t]->ingame)
			{
				if (bot_teams[t]->num_players > players_per_team->value)
				{	// kick the lowest scoring bot from this team
					edict_t	*trav, *lowest=NULL;

					// drop the lowest scoring bot
					for (i=0; i < num_players; i++)
					{
						trav = players[i];

						if (!trav->bot_client || (trav->client->team != bot_teams[t]))
							continue;

						if (!lowest || (trav->client->resp.score < lowest->client->resp.score))
						{
							lowest = trav;
						}
					}

					if (lowest)		// if NULL, then must be full of real players
					{
						botDisconnect(lowest);
					}
				}

				if ((level.time - (float)floor(level.time)) == 0)	// only check every second
				{
					// start a team grouping?
					if (bot_teams[t]->last_grouping < (level.time - 15))
					{	// check for a new grouping

						for (i=0; i<num_players; i++)
						{
							if (	(players[i]->bot_client)
								 && (players[i]->client->team == bot_teams[t])
								 && (players[i]->health > 80)
								 &&	(players[i]->waterlevel == 0)
								 && (players[i]->bot_fire != botBlaster)
								 && (players[i]->bot_fire != botShotgun)
								 && (!players[i]->target_ent)
								 && (!players[i]->enemy)
								 && (!players[i]->movetarget || (entdist(players[i], players[i]->movetarget) > 400)))
							{	// this bot shall start a gathering
								TeamGroup(players[i]);
								break;
							}
						}
					}
				}
			}
		}
	}

	if (strlen(bot_drop->string) > 0)
	{	// kill the bot
		edict_t	*trav;
		int	i;

		// locate the bot
		for (i=0; i < num_players; i++)
		{
			trav = players[i];

			if (!trav->bot_client)
				continue;

#ifdef _WIN32
			if (!_stricmp(trav->client->pers.netname, bot_drop->string))
#else
			if (!strcasecmp(trav->client->pers.netname, bot_drop->string))
#endif
			{
				botDisconnect(trav);
			}
		}

		gi.cvar_set("bot_drop", "");
	}

no_spawnbots:

	// run bot animations
	if (num_players >= 1)
	{
		for (i=0; i< num_players; i++)
		{
			if (players[i]->bot_client)
			{
				bot_AnimateFrames(players[i]);

				// check for a bot that has stopped thinking
				if (players[i]->nextthink < (level.time - 0.2))
					players[i]->nextthink = level.time + 0.1;
			}
		}
	}

	// end BOT stuff

	// choose a client for monsters to target this frame
//	AI_SetSightClient ();

	if (level.intermissiontime && (level.intermissiontime < (level.time + 10)))
	{
		level.exitintermission = true;
	}

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		if (!ent->s.old_origin[0] && !ent->s.old_origin[1] && !ent->s.old_origin[2])
			VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}

		if ((i > 0) && (i <= maxclients->value) && !ent->bot_client)
		{
			ClientBeginServerFrame (ent);
			VectorCopy (ent->s.origin, ent->s.old_origin);
			continue;
		}

		VectorCopy (ent->s.origin, ent->s.old_origin);

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	if (dedicated->value)
		OptimizeRouteCache();
}

