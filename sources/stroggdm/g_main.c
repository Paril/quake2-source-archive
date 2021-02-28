
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
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
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

// Paril
// Teamplay Code
cvar_t  *teamplay_classes;
cvar_t  *teamplay_landtype;
// Nohook
cvar_t  *nohook;
// No spawning monsters
cvar_t  *no_spawning_monsters;
//Falling Damage
cvar_t  *fallingdamage;
cvar_t  *logging;

// Monster Bans
// 31 Total
cvar_t	*ban_soldier;
cvar_t	*ban_enforcer;
cvar_t	*ban_tank;
cvar_t	*ban_ctank;
cvar_t	*ban_medic;
cvar_t	*ban_mediccommander;
cvar_t	*ban_gunner;
cvar_t	*ban_gladiator;
cvar_t	*ban_supertank;
cvar_t	*ban_flyer;
cvar_t	*ban_shark;
cvar_t	*ban_floater;
cvar_t	*ban_berserk;
cvar_t	*ban_ironmaiden;
cvar_t	*ban_boss2;
cvar_t	*ban_carrier;
cvar_t	*ban_widow1;
cvar_t	*ban_widow2;
cvar_t	*ban_brain;
cvar_t	*ban_mutant;
cvar_t	*ban_parasite;
cvar_t	*ban_jorg;
cvar_t	*ban_makron;
cvar_t	*ban_icarus;
cvar_t	*ban_betasoldier;
cvar_t	*ban_betabrain;
cvar_t	*ban_betaironmaiden;
cvar_t	*ban_betasupertank;
cvar_t	*ban_betagladiator;
cvar_t	*ban_gekk;
cvar_t	*ban_daedalus;
cvar_t	*ban_stalker;
cvar_t	*ban_fixbot;
	cvar_t  *ban_stormt;
	cvar_t  *ban_deathtank;
	cvar_t  *ban_lasergladiator;
	cvar_t  *ban_flyerhornet;
	cvar_t  *ban_traitorm;
	cvar_t  *ban_traitorf;
	cvar_t  *ban_hypertank;

// Health limits!
// 31 Total
	cvar_t	*soldier_health;
	cvar_t	*enforcer_health;
	cvar_t	*tank_health;
	cvar_t	*ctank_health;
	cvar_t	*medic_health;
	cvar_t	*mediccommander_health;
	cvar_t	*gunner_health;
	cvar_t	*gladiator_health;
	cvar_t	*supertank_health;
	cvar_t	*flyer_health;
	cvar_t	*shark_health;
	cvar_t	*floater_health;
	cvar_t	*berserk_health;
	cvar_t	*ironmaiden_health;
	cvar_t	*boss2_health;
	cvar_t	*carrier_health;
	cvar_t	*widow1_health;
	cvar_t	*widow2_health;
	cvar_t	*brain_health;
	cvar_t	*mutant_health;
	cvar_t	*parasite_health;
	cvar_t	*jorg_health;
	cvar_t	*makron_health;
	cvar_t	*icarus_health;
	cvar_t	*betasoldier_health;
	cvar_t	*betabrain_health;
	cvar_t	*betaironmaiden_health;
	cvar_t	*betasupertank_health;
	cvar_t	*betagladiator_health;
	cvar_t	*gekk_health;
	cvar_t	*daedalus_health;
	cvar_t	*stalker_health;
	cvar_t	*fixbot_health;
	cvar_t  *stormt_health;
	cvar_t  *deathtank_health;
	cvar_t  *lasergladiator_health;
	cvar_t  *flyerhornet_health;
	cvar_t  *traitorm_health;
	cvar_t  *traitorf_health;
	cvar_t  *hypertank_health;

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

cvar_t	*sv_maplist;

cvar_t	*stroggflags;

//ZOID
cvar_t	*capturelimit;
cvar_t	*instantweap;
//ZOID

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
FILE *logged;

void ShutdownGame (void)
{
    char tmpbuf[20];
	char buf2[80];
	edict_t *client;
	int i;
    _strtime(tmpbuf);
	_strdate(buf2);

	if (logging->value)
	{
		// StroggDM Logging
		gi.dprintf ("Logging Stopped.\n");
		fprintf (logged, "Logging stopped at %s, on %s\n\n\n\n", tmpbuf, buf2);
		fclose (logged);
	}

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		WRITE_PLAYER_STATS(client);
	}

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

	if (logging->value)
	{
		fprintf (logged, "There was an error in the game! Information: %s\n", text);
		fprintf (logged, "Please email this error to Paril so he may fix it.\n");
	}
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
float countdown;
int monsters_in_map;
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;

	if (!countdown)
		countdown = level.time + 6.35;

	//gi.dprintf ("%i\n", monsters_in_map);

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	if ((int)(stroggflags->value) & SF_MONSTER_OPP)
	{
		if (countdown < level.time)
		{
			// Paril, add monsters when not either of these.
			int t;

			t = rndnum (0, 22);
			if (t == 15)
				t = 16;
			if (monsters_in_map < 8)
				Spawn_Random_Monster (t);

			countdown = level.time + 6.35;
		}
	}

//ZOID
	if (ctf->value && CTFCheckRules()) {
		EndDMLevel ();
		return;
	}
	if (CTFInMatch())
		return; // no checking in match mode
//ZOID

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			safe_bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
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
				safe_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
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
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
                //Wyrm: chasecam remove 
                ChasecamRemove(ent);
	}

}

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

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	AI_SetSightClient ();

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
			// Paril
			//continue;
			// Commented out continue, for bots..
			// Paril
		}

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

