
#include "g_local.h"


#ifdef AMIGA
#include "dll.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>
int exe_found=0;
#endif

//ERASER
#include "bot_procs.h"
#include "g_map_mod.h"
#include "p_trail.h"
//ERASER

//K2:Begin
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
//K2:End

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
cvar_t	*password;
cvar_t	*spectator_password;
cvar_t	*needpass;
cvar_t	*maxclients;
cvar_t	*maxspectators;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*filterban;

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

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

cvar_t	*sv_maplist;

//K2:Begin - Server cvars
cvar_t	*giveshotgun;			//0 off 1 on
cvar_t	*givesupershotgun;		//0 off 1 on
cvar_t	*givemachinegun;		//0 off 1 on
cvar_t	*givechaingun;			//0 off 1 on
cvar_t	*givegrenadelauncher;	//0 off 1 on
cvar_t	*giverocketlauncher;	//0 off 1 on
cvar_t	*giverailgun;			//0 off 1 on
cvar_t	*givehyperblaster;		//0 off 1 on
cvar_t	*givebfg;				//0 off 1 on

cvar_t	*nobfg;					//0 off 1 on
cvar_t	*noshotgun;				//0 off 1 on
cvar_t	*nosupershotgun;		//0 off 1 on
cvar_t	*nomachinegun;			//0 off 1 on
cvar_t	*nochaingun;			//0 off 1 on
cvar_t	*nogrenadelauncher;		//0 off 1 on
cvar_t	*norocketlauncher;		//0 off 1 on
cvar_t	*nohyperblaster;		//0 off 1 on
cvar_t	*norailgun;				//0 off 1 on
cvar_t	*nomegahealth;			//0 off 1 on
cvar_t	*noquad;				//0 off 1 on
cvar_t	*noinvulnerability;		//0 off 1 on
cvar_t	*swaat;				//0 off 1 on

cvar_t	*startinghealth;		//100 - 1000
cvar_t	*startingshells;		//100 - 1000
cvar_t	*startingbullets;		//100 - 1000
cvar_t	*startinggrenades;		//100 - 1000
cvar_t	*startingrockets;		//100 - 1000
cvar_t	*startingslugs;			//100 - 1000
cvar_t	*startingcells;			//100 - 1000
cvar_t	*maxhealth;				//100 - 1000
cvar_t	*maxshells;				//100 - 1000
cvar_t	*maxbullets;			//100 - 1000
cvar_t	*maxgrenades;			//100 - 1000
cvar_t	*maxrockets;			//100 - 1000
cvar_t	*maxslugs;				//100 - 1000
cvar_t	*maxcells;				//100 - 1000
cvar_t	*startingweapon;		//100 - 1000
cvar_t	*startingarmorcount;	//100 - 1000
cvar_t	*startingarmortype;		//100 - 1000

cvar_t	*hook_time;				//0 = infinite 999 max
cvar_t	*hook_speed;			//100 - 1400 Hook velocity
cvar_t	*hook_damage;			//2 - 50 Damage done by hook
cvar_t	*pull_speed;			//100 - 1400 How fast the player is pulled;
cvar_t	*skyhook;

cvar_t	*protecttime;			//5 - 30
cvar_t	*gibtime;				//10 - 60
cvar_t	*burntime;				//10 - 60				
cvar_t	*blindtime;				//???
cvar_t	*freezetime;				//???
cvar_t	*flash_radius;			//???
cvar_t	*freeze_radius;			//???
cvar_t	*pickuptime;		//???
cvar_t	*gibdamage;				//2 - 10
cvar_t	*burndamage;			//2 - 10

cvar_t	*regentime;				//30 - 300
cvar_t	*hastetime;				//30 - 300
cvar_t	*futilitytime;			//30 - 300
cvar_t	*inflictiontime;		//30 - 300
cvar_t	*bfktime;				//30 - 300
cvar_t	*stealthtime;			//30 - 300
cvar_t	*homingtime;			//30 - 300
cvar_t	*antitime;				//30 - 300

cvar_t	*regeneration;	//0 - 4 for number of keys
cvar_t	*haste	;		//0 - 4 for number of keys
cvar_t	*futility;		//0 - 4 for number of keys
cvar_t	*infliction;		//0 - 4 for number of keys
cvar_t	*bfk;			//0 - 4 for number of keys
cvar_t	*stealth;		//0 - 4 for number of keys
cvar_t	*antikey;		//0 - 4 for number of keys
cvar_t	*homing;			//0 - 4 for number of keys


cvar_t	*droppable	;		//0 off 1 on
cvar_t	*playershells;		//0 off 1 on
cvar_t	*keyshells	;		//0 off 1 on
cvar_t	*respawntime;		//1 or 2 minutes
cvar_t	*qwfraglog	;		//0 off 1 on

cvar_t	*levelcycle	;		//0 off 1 random 2 sequential
cvar_t	*resetlevels;

cvar_t	*pickupannounce;		//0 off 1 on


cvar_t	*gibgun	;			//0 off 1 on
cvar_t	*flashgrenades;		//0 off 1 on
cvar_t	*firegrenades;		//0 off 1 on
cvar_t	*freezegrenades;
cvar_t	*firerockets;		//0 off 1 on
cvar_t	*drunkrockets;		//0 off 1 on
cvar_t	*hominghyperblaster;		//0 off 1 on

cvar_t	*motd1;
cvar_t	*motd2;
cvar_t	*motd3;
cvar_t	*motd4;
cvar_t	*motd5;
cvar_t	*motd6;
cvar_t	*motd7;
cvar_t	*nextleveldelay;

cvar_t	*allowfeigning;		//0 off 1 on
cvar_t	*usevwep;			//0 off 1 on
cvar_t	*damagemultiply;		//0 off 1 on

//weapon damage
cvar_t	*bfgdamage;
cvar_t	*raildamage;
cvar_t	*supershotgundamage;
cvar_t	*shotgundamage;
cvar_t	*chaingundamage;
cvar_t	*machinegundamage;
cvar_t	*hyperdamage;
cvar_t	*blasterdamage;
cvar_t	*rocketdamage;
cvar_t	*rocketradiusdamage;
cvar_t	*rocketdamageradius;
cvar_t	*grenadelauncherdamage;
cvar_t	*handgrenadedamage;

cvar_t	*totalstealth;
cvar_t  *nozbots;
cvar_t  *maxbots;
cvar_t	*botfraglogging;
cvar_t  *connectlogging;

//Global Vars
float	k2_keyframes;
int		k2_timeleft;
int		k2_fragsleft;
int		k2_capsleft;
float	nextlevelstart;
float	qversion;
FILE	*QWLogFile;

//K2:End

//ERASER
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
cvar_t	*ctf_special_teams; // ~JLH  //ERASER
cvar_t	*ctf_humanonly_teams; // ~JLH //ERASER
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


//ERASER
void WriteTrail ();
void ShutdownGame (void)
{
	WriteTrail();

	//K2:Begin - GSLogmod
	sl_GameEnd( &gi, level );	// StdLog - Mark Davies
	//K2:End
	
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

#ifdef AMIGA
	if (!exe_found) return 0;
#endif

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
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
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
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

  	///Q2 Camera Begin
    EnitityListClean();
	///Q2 Camera End

	sl_GameEnd( &gi, level );	// StdLog - Mark Davies

	//K2:Begin - Level Cycling
	if ( !K2_CheckLevelCycle() ) 
	//K2:End
	{		
	
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
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
	
		// see if it's in the map list
		if (*sv_maplist->string) {
			s = strdup(sv_maplist->string);
			f = NULL;
			t = strtok(s, seps);
			while (t != NULL) {
				if (Q_stricmp(t, level.mapname) == 0) {
					// it's in the list, go to the next one
					t = strtok(NULL, seps);
					if (t == NULL) { // end of list, go to first one
						if (f == NULL) // there isn't a first one, same level
							BeginIntermission (CreateTargetChangeLevel (level.mapname) );
						else
							BeginIntermission (CreateTargetChangeLevel (f) );
					} else
						BeginIntermission (CreateTargetChangeLevel (t) );
					free(s);
					return;
				}
				
				if (!f)
					f = t;
				
				t = strtok(NULL, seps);
			}
			
			free(s);
		}

		if (level.nextmap[0]) // go to a specific map
			BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
		else {	// search for a changelevel
			ent = G_Find (NULL, FOFS(classname), "target_changelevel");
			if (!ent)
			{	// the map designer didn't include a changelevel,
				// so create a fake ent that goes back to the same level
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
				return;
			}
			BeginIntermission (ent);
		}
	}

	//ERASER
	WriteTrail();

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
	if (password->modified || spectator_password->modified) 
	{
		password->modified = spectator_password->modified = false;

		need = 0;

		if (*password->string && Q_stricmp(password->string, "none"))
			need |= 1;
		if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
			need |= 2;

		gi.cvar_set("needpass", va("%d", need));
	}
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

	//K2:Begin - Rank - 
	int			sorted[MAX_CLIENTS];
	int			sortedscores[MAX_CLIENTS];
	edict_t		*cl_ent;
	int			j, k, high=0, score, total=0;
		
	if(!ctf->value)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;
			score = game.clients[i].resp.score;
			for (j=0 ; j<total ; j++)
			{
				if (score > sortedscores[j])
					break;
			}
			for (k=total ; k>j ; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		// Update the client rank
		for(i=0;i<total;i++)
		{
			cl = &game.clients[sorted[i]];
			cl->rank = i + 1;
		}
	
	}
	//K2:End

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
		
		//K2:Begin - Timelimit coundown
		k2_timeleft = (int)(timelimit->value - (level.time/60) );
		if ( ((timelimit->value*60) - level.time) < 60)
			k2_timeleft = (int)(timelimit->value*60 - level.time);
		//K2:End
	}

	if (ctf->value) {
		if (CTFCheckRules()) {
			EndDMLevel ();
		}
	}

	if (fraglimit->value)
	{
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
			
			//K2:Begin - Fraglimit Countdown
			if (!ctf->value)
			{
				if(cl->resp.score > high)
				{	
					high = cl->resp.score;
					k2_fragsleft = (int)(fraglimit->value - high);
				}
				
				if( (k2_fragsleft > fraglimit->value) || (high == 0))
					k2_fragsleft = (int)fraglimit->value;
				
			}
			//K2:End
		}
	}
}

//ERASER
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
	
	//K2:Begin
	nextlevelstart = 9999999;
	//K2:End
	
	ClientEndServerFrames ();

	//ERASER
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

		//ERASER
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

//ERASER
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
//ERASER

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
	static float last_ctf_teams=0;

	if (paused)
		return;

	if (level.time < last_ctf_teams)
		last_ctf_teams = 0;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	//ERASER
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
				force_team = CTF_NOTEAM; //~~JLH

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
						    if (team1 < ctf_auto_teams->value ) // ~~JLH fix respawn forever
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
						    if (team2 < ctf_auto_teams->value ) // ~~JLH fix respawn forever
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
//				gi.dprintf("Cannot spawn bot: not enough free client spaces (bot_free_clients = %i)\n", (int) bot_free_clients->value);
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

	//ERASER  end BOT stuff

	// choose a client for monsters to target this frame
	//	AI_SetSightClient ();

	if (level.intermissiontime && (level.intermissiontime < (level.time - 10)))
	{
		level.exitintermission = true;
	}

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	//K2:Begin - Next level start time - Automatically switch to next level
	//           after xx amount of time
	if (nextlevelstart < level.time)
	{
		ExitLevel ();
		return;
	}
	//K2:End


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

		//if (!ent->s.old_origin[0] && !ent->s.old_origin[1] && !ent->s.old_origin[2])
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
			//VectorCopy (ent->s.origin, ent->s.old_origin);
			continue;
		}

		//VectorCopy (ent->s.origin, ent->s.old_origin);

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();


	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	//ERASER
	if (dedicated->value)
		OptimizeRouteCache();
	

}

#ifdef AMIGA
void SetExeName(char *name)
{
	exe_found=1;
}
#endif

void* __saveds dllFindResource(int id, char *pType)
{
    return NULL;
}

void* __saveds dllLoadResource(void *pHandle)
{
    return NULL;
}

void __saveds dllFreeResource(void *pHandle)
{
    return;
}

ULONG SegList;

dll_tExportSymbol DLL_ExportSymbols[]=
{
    {dllFindResource,"dllFindResource"},
    {dllLoadResource,"dllLoadResource"},
    {dllFreeResource,"dllFreeResource"},
   {(void *)GetGameAPI,"GetGameAPI"},
   {(void *)SetExeName,"SetExeName"},
   {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[]=
{
	{0,0,0,0}
};

int __saveds DLL_Init(void)
{
  	struct CommandLineInterface *pCLI = Cli();
    if (!pCLI) return 0;
    SegList = (ULONG)(pCLI->cli_Module);
    return 1L;
}

void __saveds DLL_DeInit(void)
{
}

