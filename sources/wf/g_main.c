//Main Routine

#define WFMAIN	1

#include "g_local.h"
#include "stdlog.h"	//	StdLog - Mark Davies

//ERASER START
#include "bot_procs.h"
#include "p_trail.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
//ERASER END

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
cvar_t	*needpass;
cvar_t	*sv_maplist;

//WF
cvar_t	*wfflags;
//cvar_t  *classdef;
cvar_t  *wfconfig;
cvar_t	*gamedir;
cvar_t	*mock;
cvar_t	*mod;
cvar_t  *filterban;

int		wfdebug = 0;
int		flag1dropped = 0;
int		flag2dropped = 0;

 
//WF

//ZOID
cvar_t	*capturelimit;
//ZOID
cvar_t  *wfpassword;
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

//ERASER START
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
cvar_t  *bot_melee;//acrid added
cvar_t	*players_per_team;
cvar_t	*addteam;
cvar_t	*teamplay;
cvar_t	*ctf_auto_teams;
cvar_t	*ctf_special_teams; // ~JLH
cvar_t	*ctf_humanonly_teams; // ~JLH
//cvar_t	*grapple;

//ERASER END

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


void WriteTrail ();//ERASER
void ShutdownGame (void)
{
	WriteTrail();//ERASER
	gi.dprintf ("==== ShutdownGame ====\n");

        sl_GameEnd( &gi, level );       // StdLog

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
		if (!ent->inuse || !ent->client || ent->bot_client)//ERASER || ent->bot_client
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
//WF & LAC
//	int i;
//WF
///ERASER START Q2 Camera Begin REMOVE THIS
    EnitityListClean();
///Q2 Camera End

	TOTALDROPPEDAMMO = 0;
	TOTALWORLDFLAMES = 0;

	// MAP MOD
/*
	if (map_mod_)
	{
		char *nextmap;

		if (nextmap = map_mod_next_map())
		{	// only use a map_mod map if the current level is known
			strcpy(level.nextmap,nextmap);
		}
	}
*/
	// MAP MOD

	//ERASER END
	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname;
	}
//WF & LAC+++ 
	// if you also want this to happen in co-op, you will probably 
	// have to put similar code in ExitLevel(). 
	else if (maplist.active) 
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		MaplistNextMap(ent);
		maplist.warning_given = false;	//clear warning flag

	}
//WF & LAC+++ 
	else if (level.nextmap[0])
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.nextmap;
	}
	else
	{	// search for a changelevel
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
	WriteTrail();//ERASER
}


/*
=================
WFEndDMLevel

End the level and go to selected map
=================
*/
void WFEndDMLevel (char *mapname)
{
	edict_t		*ent;
    EnitityListClean();

	TOTALDROPPEDAMMO = 0;
	TOTALWORLDFLAMES = 0;

gi.dprintf("Ref is switching to map '%s'\n", mapname);
	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	ent->map = mapname;
	BeginIntermission (ent);
	WriteTrail();//ERASER
}

/*
=================
CheckNeedPass
=================
*/
void CheckNeedPass (void)
{
	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if (wfpassword->modified ) 
	{
		wfpassword->modified = false;

		need = 0;

		if (*wfpassword->string && Q_stricmp(wfpassword->string, "none"))
			need |= 1;

		gi.cvar_set("needpass", va("%d", need));
	}
}
//WF34 END
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
//WF24 START - Map voting
		//give them the 1 minute warning
		if (((int)wfflags->value & WF_MAP_VOTE) &&
			(level.time >= (timelimit->value - 1)*60) &&
			(maplist.warning_given == false))
				{
					gi.bprintf (PRINT_MEDIUM,"-- One Minute Warning: Type 'vote' to pick next map --\n");
					maplist.warning_given = true;
				}


//WF24 E WF ADDED ELSE BELOW
		else if (level.time >= timelimit->value*60)
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}
//WF IS A BIT DIF FRAGLIMIT VALUE LINE IS HERE
//ERASER CO	if (fraglimit->value)
//ERASER CO	{
//ZOID
		if (ctf->value) {
			if (CTFCheckRules()) {
				EndDMLevel ();
			}
		}
//ZOID
	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{//ERASER USES MY_ WAS GI.
				my_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}

//WF - Map voting
			//give them the frag limit warning
			if (((int)wfflags->value & WF_MAP_VOTE) &&
			(cl->resp.score >= (fraglimit->value-3)) &&
			(maplist.warning_given == false))
			{
				gi.bprintf (PRINT_MEDIUM,"-- Frag Limit Warning (3 left): Type 'vote' to pick next map --\n");
				maplist.warning_given = true;
			}


//WF24 E

		}
	}

//++TeT WF Flag Pickup Delay Countdown

	if (level.time <= 30)//eraser change all gi. to my_
	{
		if ( level.time == 1)
		{
			my_bprintf (PRINT_MEDIUM,"-- 30 Seconds Till Flag Pick Up Allowed --\n");
		}
		if ( level.time == 10)
		{
			my_bprintf (PRINT_MEDIUM,"-- 20 Seconds Till Flag Pick Up Allowed --\n");
		}
		if ( level.time == 15)
		{
			my_bprintf (PRINT_MEDIUM,"-- 15 Seconds Till Flag Pick Up Allowed --\n");
		}
		if ( level.time == 20)
		{
			my_bprintf (PRINT_MEDIUM,"-- 10 Seconds Till Flag Pick Up Allowed --\n");
		}
		if ( level.time == 25)
		{
			my_bprintf (PRINT_MEDIUM,"-- 5 Seconds Till Flag Pick Up Allowed --\n");
		}
		if ( level.time == 26)
		{
			my_bprintf (PRINT_MEDIUM,"4\n");
		}
		if ( level.time == 27)
		{
			my_bprintf (PRINT_MEDIUM,"3\n");
		}
		if ( level.time == 28)
		{
			my_bprintf (PRINT_MEDIUM,"2\n");
		}
		if ( level.time == 29)
		{
			my_bprintf (PRINT_MEDIUM,"1\n");
		}
		if ( level.time == 30)
		{
			my_bprintf (PRINT_MEDIUM,"Flag Pick Up Allowed\n");
		}
	}
//--TeT WF Flag Pickup Delay Countdown

}
//ERASER START
char	respawn_bots[64][256];
bot_team_t	*respawn_bot_teams[64];
int		respawn_ctf_teams[64];
extern int force_team;
qboolean	respawn_init = false, respawn_flag = false;
//ERASER END

/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i,j;//ERASER ADDED J
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

//ERASER START
	memset(respawn_bots, 0, sizeof(respawn_bots));
	memset(respawn_bot_teams, 0, sizeof(bot_teams));

	// clear some things before going to next level
	j = 0;
//ERASER END
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
//ERASER START
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
//ERASER END

//ZOID
	CTFInit();
//ZOID

}
//ERASER START
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

extern int	force_team;

// enforce ctf_special_teams with real humans
// what enforce_ctf_special_teams does is keep the teams even with the human players
// based on a multiplier ctf_special_teams.
// if one human enters the games then one bot times ctf_special_teams is added.
// normally you would set ctf_special_teams = 1. However, if ctf_special_teams = 2
// then 2 bots for every 1 human is added until the connections are used up.
// If on the otherhand a human enters the other team then the teams are 
// automatically evened up. However, if ctf_humanonly_teams is set to 1 then we 
// only allow humans in one team and no bots. Human in this case may start in
// any team. But any other human that tries to enter into the game on  the
// other team will be forced to join other team.
//
static qboolean enforce_ctf_special_teams()
{
	qboolean	bRet = FALSE;
	int			i;
	static float	s_iLastTime = 0;

	if (level.time < s_iLastTime)
		s_iLastTime = 0;

	if ( ctf->value && ctf_special_teams->value )
	{
		bRet = TRUE;

		if ( s_iLastTime < (level.time - 1) )
		{
			extern void CTFJoinTeam(edict_t *ent, int desired_team);
			qboolean	bRet = FALSE;		// set to true if this is valid command
			qboolean	bHumanOnly = ctf_humanonly_teams->value;
			int			iTeam1Count=0;		// total current team players
			int			iTeam2Count=0;
			int			iTeam1NeedCount=0;	// how many bots we need to even it out
			int			iTeam2NeedCount=0;
			int			iTeam1Humans=0;		// number of humans in game
			int			iTeam2Humans=0;
            double		dSpecialTeams = ctf_special_teams->value;
			int			iMaxPlayers = maxclients->value;
			edict_t		*pDropBotTeam1=NULL; // player we drop from team is needed
			edict_t		*pDropBotTeam2=NULL;
			edict_t		*pNewBot;			// new bot created


			// count number of players on each team
			for (i=0; i<num_players; i++)
			{
				if (players[i]->client->resp.ctf_team == CTF_TEAM1)
				{
					iTeam1Count++;

					if (players[i]->bot_client)
					{
						if (!pDropBotTeam1 || (players[i]->client->resp.score < pDropBotTeam1->client->resp.score))
							pDropBotTeam1 = players[i];
					}
					else
						iTeam1Humans++;
				}
				else if (players[i]->client->resp.ctf_team == CTF_TEAM2)
				{
					iTeam2Count++;

					if (players[i]->bot_client)
					{
						if (!pDropBotTeam2 || (players[i]->client->resp.score < pDropBotTeam2->client->resp.score))
							pDropBotTeam2 = players[i];
					}
					else
						iTeam2Humans++;
				}
			}
			
			// Range check special to 1+
			if ( dSpecialTeams < 1 )
				dSpecialTeams = 1;
			// First pass, set teams to max of other

			if (iTeam1Count > iTeam2Count)
				iTeam1NeedCount = iTeam2NeedCount = iTeam1Count; // even teams
			else
				iTeam1NeedCount = iTeam2NeedCount = iTeam2Count; // even teams

			if ( iTeam1Humans > 0 && iTeam2Humans > 0 )
			{
				// Do the following only if we have set ctf_humanonly_teams
				if ( bHumanOnly && force_team != CTF_NOTEAM )
				{
					// scan all players and look for human in the wrong place
					for ( i = 0; i < num_players; i++ )
						if ( !players[i]->bot_client ) // if this is a human
							if ( players[i]->client->resp.ctf_team == force_team ) // if they are on bot team
							{
								if ( force_team == CTF_TEAM1 ) // switch them to the other team
									CTFJoinTeam( players[i], CTF_TEAM2 );
								else
									CTFJoinTeam( players[i], CTF_TEAM1 );
								return TRUE; // we need to restart the count again
							}
				}
			}
			else if ( iTeam1Humans == 0 && iTeam2Humans == 0 )
			{
				iTeam1NeedCount = iTeam2NeedCount = 0;	// no humans in game no playing!
				force_team = CTF_NOTEAM;
			}
			else
				if ( iTeam1Humans == 0 || iTeam2Humans == 0 )
				{
					if ( iTeam1Humans == 0 ) // no humans on team 1
					{
						force_team = CTF_TEAM1;
						iTeam1NeedCount = (int) ( (double) iTeam2Humans * dSpecialTeams );
						// if human only then don't allow bot to be created in Team2
						if ( bHumanOnly )
							iTeam2NeedCount = iTeam2Count;
						// Adjust BOT if multiplier is >1 and <2 you could have 1.5 bots per teams
						if ( iTeam1NeedCount == 1 && ( dSpecialTeams > 1 && dSpecialTeams < 2 ) )
							iTeam1NeedCount++;
						// Range check this to make sure we are not over doing it
						if ( iTeam1NeedCount + iTeam2Count > iMaxPlayers )
							iTeam1NeedCount = iMaxPlayers - iTeam2Count;
					}
					else
					{	// no humans on team 2
						force_team = CTF_TEAM2;
						iTeam2NeedCount = (int) ( (double) iTeam1Humans * dSpecialTeams );
						// if human only then don't allow bot to be created in Team1
						if ( bHumanOnly )
							iTeam1NeedCount = iTeam1Count;
						// Adjust BOT if multiplier is >1 and <2 you could have 1.5 bots per team
						if ( iTeam2NeedCount == 1 && ( dSpecialTeams > 1 && dSpecialTeams < 2 ) )
							iTeam2NeedCount++;
						// Range check this to make sure we are not over doing it
						if ( iTeam2NeedCount + iTeam1Count > iMaxPlayers )
							iTeam2NeedCount = iMaxPlayers - iTeam1Count;
					}
				}
			// Let add bots to team 1 if necessary
			if ( iTeam1Count !=  iTeam1NeedCount )
			{
				pNewBot = NULL;
				if (iTeam1Count > iTeam1NeedCount )
				{
					if (pDropBotTeam1) 
						botDisconnect(pDropBotTeam1);  // drop it
				}
				else
				botDebugPrint("SPAWN_BOT NEW BOT1(ACRID)\n");
					pNewBot = spawn_bot(NULL); // the game needs more players
				// make sure its on the right team
				//if ( pNewBot &&  pNewBot->client->resp.ctf_team != CTF_TEAM1 )
				//		CTFJoinTeam( pNewBot, CTF_TEAM1 );
			}
			// Let add bots to team 2 if necessary
			if ( iTeam2Count !=  iTeam2NeedCount )
			{
				// Lets fix up team 2
				pNewBot = NULL;
				if ( iTeam2Count > iTeam2NeedCount )
				{
					if (pDropBotTeam2)
						botDisconnect(pDropBotTeam2); // drop it
				}
				else	// the game needs more players
		botDebugPrint("SPAWN_BOT NEW BOT 2(ACRID)\n");
					pNewBot = spawn_bot(NULL);
				// make sure its on the right team
				//if ( pNewBot &&  pNewBot->client->resp.ctf_team != CTF_TEAM2 )
				//	CTFJoinTeam( pNewBot, CTF_TEAM2 );
			}
			s_iLastTime = level.time;
		}
	}
	return bRet;
}
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

qboolean execed_launcher_cfg=false;
//ERASER END
/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;
	static float last_ctf_teams=0;//ERASER START

	if (paused)
		return;

	if (level.time < last_ctf_teams)
		last_ctf_teams = 0;
//ERASER END
	level.framenum++;
	level.time = level.framenum*FRAMETIME;
//ERASER START
	// check for disabling node calc.
	if ((bot_calc_nodes->value) && (level.time > 300) && (!check_nodes_done))
	{	// start checking for disabling node calc
		check_nodes_done = G_Spawn();
		check_nodes_done->think = CheckNodeCalcDisable;
		check_nodes_done->nextthink = level.time + 0.1;
	}

	if (!execed_launcher_cfg && (level.time > 1))
	{
		gi.AddCommandString("exec launcher.cfg\n");
		execed_launcher_cfg = true;
	}

	// do BOT stuff

	roam_calls_this_frame = 0;
	bestdirection_callsthisframe = 0;

	if (level.time < last_bot_spawn)
		last_bot_spawn = 0;

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

			if (level.time < last_spawn)
				last_spawn = 0;

			if (last_spawn > (level.time - 0.3))
				goto no_spawnbots;

			last_spawn = level.time;

			for (i=0; i<64; i++)
			{
				if (respawn_bots[i][0] == '\0')
					continue;

				force_team = respawn_ctf_teams[i];
				ent = spawn_bot(respawn_bots[i]);
		botDebugPrint("SPAWN_BOT FORCE TEAM (ACRID)\n");
				force_team = CTF_NOTEAM; //~~JLH

				if (ent && respawn_bot_teams[i])
				{
					ent->client->team = respawn_bot_teams[i];
				}

				// set name
				if (ent)
					strcpy(ent->client->pers.netname, respawn_bots[i]);

				respawn_bots[i][0] = '\0';
		botDebugPrint("SPAWN_BOT NULL 8(ACRID)\n");
				if (++count > 2)
					goto no_spawnbots;
			}

			respawn_flag = false;

			goto no_spawnbots;
		}
//MIGHT BE THE SPAWN TROUBLE FIXME ACRID
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
		botDebugPrint("SPAWN_BOT NULL 1(ACRID)\n");
								spawn_bot(NULL);
							}
						}
						else	// the game needs more players
						    if (team1 < ctf_auto_teams->value ) // ~~JLH fix respawn forever
							{
		botDebugPrint("SPAWN_BOT NULL 2(ACRID)\n");
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
		botDebugPrint("SPAWN_BOT NULL 3(ACRID)\n");
								spawn_bot(NULL);
							}
						}
						else	// the game needs more players
						    if (team2 < ctf_auto_teams->value ) // ~~JLH fix respawn forever
							{
		botDebugPrint("SPAWN_BOT NULL 4(ACRID)\n");
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
		botDebugPrint("SPAWN_BOT NULL DOUBLE(ACRID)\n");
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
		botDebugPrint("SPAWN_BOT NULL 6(ACRID)\n");

				spawn_bot(NULL);
			}
			else
			{
//				gi.dprintf("Cannot spawn bot: not enough free client spaces (bot_free_clients = %i)\n", (int) bot_free_clients->value);
			}

			if (spawn_bots > 0)
				spawn_bots--;
			
					botDebugPrint("SPAWN_BOT NULL 9(ACRID)\n");
		}
		else if (strlen(bot_name->string) > 1)
		{
			if (num_players < (maxclients->value - bot_free_clients->value))
			{
		botDebugPrint("SPAWN_BOT NULL 7(ACRID)\n");
				spawn_bot(bot_name->string);
			}
			else
			{
				gi.dprintf("Cannot spawn bot: not enough free client spaces (bot_free_clients = %i)\n", (int)bot_free_clients->value);
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
		botDebugPrint("SPAWN_BOT SKIN 1(ACRID)\n");
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
//ERASER END

	// choose a client for monsters to target this frame
//	AI_SetSightClient ();//WF USES THIS LINE

//ERASER START
	if (level.intermissiontime && (level.intermissiontime < (level.time - 10)))
	{
		level.exitintermission = true;
	}
//ERASER END

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
//ERASER LINE BELOW
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

		if ((i > 0) && (i <= maxclients->value) && !ent->bot_client)//ERASER ADDED && !ent->bot_client)
		{
			ClientBeginServerFrame (ent);
			VectorCopy (ent->s.origin, ent->s.old_origin);//ERASER
			continue;
		}

		VectorCopy (ent->s.origin, ent->s.old_origin);//ERASER

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated WF34
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	if (dedicated->value)//ERASER
		OptimizeRouteCache();//ERASER

}


