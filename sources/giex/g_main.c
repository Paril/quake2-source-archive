
#include "g_local.h"

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
cvar_t	*dmmonsters;
cvar_t	*coop;
cvar_t	*teams;
cvar_t	*teamdamage;
cvar_t	*idletimeout;
cvar_t	*charpath;
cvar_t	*mapspath;
cvar_t	*cdpspath;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*autoskill;
cvar_t	*baseskill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*password;
cvar_t	*admin_password;
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

//r1: added
cvar_t	*ipbans;

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


int SV_Cmd_WriteIP_f (void);
void ShutdownGame (void)
{
	int		i;
	edict_t	*ent;
	FILE *file;

	gi.dprintf ("==== ShutdownGame ====\n");

	i = SV_Cmd_WriteIP_f();
	if (i == -1) {
		gi.dprintf("Failed to write banlist\n");
	} else {
		gi.dprintf("Wrote %d IPs to banlist\n", i);
	}
	if ((file = fopen("entities.txt", "a")) == NULL) {
		gi.dprintf("Failed to open entity file\n");
	} else {
		fprintf(file, "=======================\n=======================\n=======================\n");
		for (i=0 ; i < MAX_EDICTS ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;
			fprintf(file, "%d: %s\n", i, ent->classname);
//			gi.dprintf("%d: %s\n", i, ent->classname);
		}
		fclose(file);
	}
	// Save character data
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		saveCharacter(ent);
	}

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
		if (!ent->inuse || !ent->client)
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
edict_t *CreateTargetChangeLevel(char *map) {
	edict_t *ent;

	ent = G_Spawn ();
	ent->classid = CI_T_CHANGELEVEL;
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
=================
CountdownCraze

Decreases the "craze" counter at each time- or fraglimit.
=================
*/
void CountdownCraze(void) {
	if (game.craze > 0)
		game.craze--;
	if (game.monsterhunt > 0)
		game.monsterhunt--;
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

	// stay on same level flag
/*	if ((int)dmflags->value & DF_SAME_LEVEL) {
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}*/

	// see if it's in the map list
	if (*sv_maplist->string) {
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);
		while (t != NULL) {
			if (Q_strcasecmp(t, level.mapname) == 0) {
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

	if (deathmatch->value) {
		char *map = getNextMap(level.mapname);
		gi.bprintf(PRINT_HIGH, "Next map: %s\n", map);
		BeginIntermission (CreateTargetChangeLevel (map) );
	} else if (level.nextmap[0]) { // go to a specific map
		BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
	} else {	// search for a changelevel
		ent = G_FindClassId (NULL, CI_T_CHANGELEVEL);
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
		BeginIntermission (ent);
	}
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

		if (*password->string && Q_strcasecmp(password->string, "none"))
			need |= 1;
		if (*spectator_password->string && Q_strcasecmp(spectator_password->string, "none"))
			need |= 2;

		gi.cvar_set("needpass", va("%d", need));
	}
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void) {
	int			i;
	gclient_t	*cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	if (game.monsterhunt == 10) {
		if ((level.total_monsters - level.killed_monsters) < 1) {
			if ((level.hunt_endtime == 0) && (level.time > 45)) {
				level.hunt_endtime = level.time + 30;
			}
		} else {
			level.hunt_endtime = 0;
		}
		if (ceil((level.hunt_endtime - level.time) * 100) == 1000) {
			if (level.total_monsters > 0)
				gi.bprintf(PRINT_HIGH, "Congratulations!\n");
			gi.bprintf(PRINT_HIGH, "Map ends in 10 seconds..\n");
		}
		if ((level.hunt_endtime > 0) && (level.time > level.hunt_endtime)) {
			CountdownCraze();
			EndDMLevel ();
			return;
		}
	}

	if ((timelimit->value) && (game.monsterhunt != 10)) {
		float time_mult = 1.0;
		if (game.craze == 10)
			time_mult = 0.5;

		if (level.time >= timelimit->value*60*time_mult) {
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			CountdownCraze();
			EndDMLevel ();
			return;
		}
	}

	if ((fraglimit->value) && (game.monsterhunt != 10)) {
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				CountdownCraze();
				EndDMLevel ();
				return;
			}
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void) {
	int		i, sum = 0, num_players = 0, newskill;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;

		sum += ent->radius_dmg;
		num_players++;

		saveCharacter(ent);
	}

	if (coop->value && autoskill->value) {
		newskill = (int) ceil(0.75 * (baseskill->value * (pow((float) sum/(float) num_players, GIEX_MONSTER_PLAYERLEVEL_MULT_POW) * GIEX_MONSTER_PLAYERLEVEL_MULT)));
		if (newskill > 30)
			newskill = 30;
		if (newskill != skill->value) {
			gi.bprintf(PRINT_HIGH, "Changing skill to %d\n", newskill);
			sprintf(command, "%d", newskill);
			gi.cvar_forceset("skill", command);
		}
	}

}

int getVotes(int v) {
	int i;
	edict_t *scan;
	int result = 0;

	for (i=0 ; i<maxclients->value ; i++) {
		scan = g_edicts + 1 + i;
		if (!scan->inuse)
			continue;
		if (!scan->client)
			continue;
		if (scan->client->pers.vote == v)
			result++;
	}
	return result;
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/

void assignAllToTeams() {
	int i, j, k, total, score;
	int sorted[MAX_CLIENTS];
	int	sortedscores[MAX_CLIENTS];
	edict_t *scan;

	total = 0;
	for (i=0 ; i<game.maxclients ; i++) {
		scan = g_edicts + 1 + i;
		if (!scan->inuse || game.clients[i].resp.spectator)
			continue;
		scan->count = 0;
		score = scan->radius_dmg;
		for (j=0 ; j<total ; j++) {
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--) {
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}
	for (i=0 ; i<total ; i++) {
		scan = g_edicts + 1 + sorted[i];
		assignToTeam(scan);
	}
}

void spawnDMMonster(void);
void decLight(void);
void incLight(void);
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent, *scan;
	char	command [256];

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	if (deathmatch->value && teams->value && (teams->value != 3) && (level.framenum == 150)) {
		assignAllToTeams();
	}


	// choose a client for monsters to target this frame
	AI_SetSightClient ();

	// exit intermissions

	if ((level.exitintermission) || (level.intermissiontime && (level.time > level.intermissiontime + 300.0))) {
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

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
		}

		G_RunEntity (ent);
	}

	if ((level.vote_type != 0) && (level.vote_time < level.time)) {
		int yesvotes = getVotes(2);
		int novotes = getVotes(1);
		if (yesvotes > novotes) {
			gi.bprintf(PRINT_HIGH, "Poll passed (%d to %d)\n", yesvotes, novotes);
			if (level.vote_type == VOTE_TM) {
				float newskill;
				if (baseskill->value < 1)
					newskill = baseskill->value + 0.2;
				else
					newskill = baseskill->value + 0.5;
				if (newskill > 5)
					newskill = 5;
				gi.bprintf(PRINT_HIGH, "Setting baseskill to %.1f\n", newskill);
				sprintf(command, "%f", newskill);
				gi.cvar_set("baseskill", command);
			} else if (level.vote_type == VOTE_WM) {
				float newskill;
				if (baseskill->value > 1)
					newskill = baseskill->value - 0.5;
				else
					newskill = baseskill->value - 0.2;
				if (newskill < 0)
					newskill = 0;
				gi.bprintf(PRINT_HIGH, "Setting baseskill to %.1f\n", newskill);
				sprintf(command, "%f", newskill);
				gi.cvar_set("baseskill", command);
			} else if (level.vote_type == VOTE_FFA) {
				gi.bprintf(PRINT_HIGH, "Restarting map in Free for all mode\n");
				sprintf(command, "0");
				gi.cvar_forceset("teams", command);
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			} else if (level.vote_type == VOTE_TDM) {
				gi.bprintf(PRINT_HIGH, "Restarting map in Team deathmatch mode\n");
				sprintf(command, "1");
				gi.cvar_forceset("teams", command);
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			} else if (level.vote_type == VOTE_DOM) {
				gi.bprintf(PRINT_HIGH, "Restarting map in Domination mode\n");
				sprintf(command, "2");
				gi.cvar_forceset("teams", command);
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			} else if (level.vote_type == VOTE_PVM) {
				gi.bprintf(PRINT_HIGH, "Restarting map in Players vs Monsters mode\n");
				sprintf(command, "3");
				gi.cvar_forceset("teams", command);
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			} else if (level.vote_type == VOTE_END) {
				gi.bprintf(PRINT_HIGH, "Aborting map..\n");
				if (game.craze == 10)
					game.craze--;
				if (game.monsterhunt == 10)
					game.monsterhunt--;
				EndDMLevel();
			} else if (level.vote_type == VOTE_CRAZE) {
				gi.bprintf(PRINT_HIGH, "Going into craze next map!\n");
				game.craze = 11;
			} else if (level.vote_type == VOTE_HUNT) {
				gi.bprintf(PRINT_HIGH, "Going for some monster hunting next map!\n");
				game.monsterhunt = 11;
			} else if (level.vote_type == VOTE_KICK) {
				//TODO: Add check to make sure vote is aborted if client voted to be kicked disconnects
				edict_t *targ_ent = g_edicts + 1 + level.vote_target[0];
				gi.bprintf(PRINT_HIGH, "Kicking %s.\n", targ_ent->client->pers.netname);
				kick(targ_ent);
			} else if (level.vote_type == VOTE_MUTE) {
				//TODO: Add check to make sure vote is aborted if client voted to be muted disconnects
				edict_t *targ_ent = g_edicts + 1 + level.vote_target[0];
				gi.bprintf(PRINT_HIGH, "%s is muted.\n", targ_ent->client->pers.netname);
				targ_ent->client->pers.muted = true;
			} else if (level.vote_type == VOTE_MONSTER) {
				if (dmmonsters->value) {
					gi.bprintf(PRINT_HIGH, "Not spawning any more monsters\n");
					sprintf(command, "0");
				} else {
					gi.bprintf(PRINT_HIGH, "Starting to spawn monsters\n");
					sprintf(command, "1");
				}
				gi.cvar_forceset("dmmonsters", command);
			} else {
				gi.bprintf(PRINT_HIGH, "...but this is an unknown poll type!\n");
			}
		} else {
			gi.bprintf(PRINT_HIGH, "Poll did not pass (%d to %d)\n", yesvotes, novotes);
		}
		level.vote_type = 0;
		for (i=0 ; i<maxclients->value ; i++) {
			scan = g_edicts + 1 + i;
			if (!scan->inuse)
				continue;
			if (!scan->client)
				continue;
			scan->client->pers.vote = 0;
		}
	}

	// see if a monster should be spawned
	spawnDMMonster();

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	if (((int)ceil(level.time * 10 - 0.5) % 3) == 0) {
		if ((level.stealthcount > 0) && (level.light_level > 0)) {
			decLight();
		}
		if ((level.stealthcount < 1) && (level.light_level < 12)) {
			incLight();
		}
	}
}

void logmsg(char *message) {
	FILE *logfile = NULL;
	if ((logfile = fopen("giex/giexlog.txt", "a")) == NULL) {
		gi.dprintf("Failed to open log file!\n");
		return;
	}
	fprintf(logfile, "%s", message);
	fclose(logfile);
	logfile = NULL;
}
