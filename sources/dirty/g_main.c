#include "g_local.h"

// GRIM - Only needed for LCC. Take out otherwise
#ifdef __LCC__
#include <stdio.h>

int stdcall;
DLLMain(void *hinstDll,unsigned long dwReason,void *reserved)
{
	return(1);
}
#endif
// GRIM

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

// GRIM
// Dirty
int     total_shit;
int     total_gibs;
int     total_debris;
int     knife_number; // Knives have a 'smart' limiter.
int     items_number; // Dropped items have a 'smart' limiter.
int     instant_lead;
int     total_lead;

int     max_teams; // Specified by info_teams first, then flags,
                // then teams->value. Doubles as an indicator that teams
                // are indeed on.
qboolean turns_on; // Set by target_endgames or match->value
qboolean goals_exist; // Set by target_endgames or match->value
// Dirty
int     TypeOfDamage;
int     HitLocation;
// GRIM

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

// GRIM
cvar_t  *bhole;
cvar_t  *blood;
cvar_t  *ctf;
cvar_t  *bflags;
cvar_t  *realflags;

cvar_t  *teams;
cvar_t  *goal_limit;
cvar_t  *round_timelimit;
cvar_t  *forcejoin;
cvar_t  *match;

// Dirty - Limits
cvar_t  *junk;
cvar_t  *gibs;
cvar_t  *debris;
cvar_t  *bhole;
cvar_t  *blood;
cvar_t  *sv_jumpgrav;
cvar_t  *max_knives;
cvar_t  *max_items;
cvar_t  *sv_itemchoices;
cvar_t  *sv_weaponchoices;
cvar_t  *bandages;
cvar_t  *teamkill;
// Dirty
// GRIM

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


void ShutdownGame (void)
{
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

// Paranoid
/* FIX ME - Dirty
qboolean TeamMap (void)
{
        teams_t *team = NULL;
        int     i, j, t;

        i = j = t = 0;
        if (level.team1nextmap[0])
                i++;
        if (level.team2nextmap[0])
                i++;
        if (level.team3nextmap[0])
                i++;
        if (level.team4nextmap[0])
                i++;

        if (i < 1)
                return false;

        while (i > 0)
        {
                team = GetTeamByIndex (i);
                if (team->total_score > j)
                {
                        j = team->total_score;
                        t = i;
                }
                i--;
        }

        if (t)
        {
                switch (t)
                {
                        case 1:
                                BeginIntermission (CreateTargetChangeLevel (level.team1nextmap));
                        case 2:
                                BeginIntermission (CreateTargetChangeLevel (level.team2nextmap));
                        case 3:
                                BeginIntermission (CreateTargetChangeLevel (level.team3nextmap));
                        case 4:
                                BeginIntermission (CreateTargetChangeLevel (level.team4nextmap));
                }
                return true;
        }
        return false;
}
*/
// Paranoid

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
	if (*sv_maplist->string)
        {
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);
		while (t != NULL)
                {
			if (Q_stricmp(t, level.mapname) == 0)
                        {
				// it's in the list, go to the next one
				t = strtok(NULL, seps);
				if (t == NULL)
                                { // end of list, go to first one
					if (f == NULL) // there isn't a first one, same level
						BeginIntermission (CreateTargetChangeLevel (level.mapname) );
					else
						BeginIntermission (CreateTargetChangeLevel (f) );
                                }
                                else
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
	else
        {	// search for a changelevel
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
void CheckDMRules (void)
{
	int			i;
        //gclient_t       *cl; // GRIM

        // GRIM
	if (level.intermissiontime)
        {
                if (level.time > (level.intermissiontime + 35))
                        level.exitintermission = true; // 35 seconds is plenty
		return;
        }
        // GRIM

	if (!deathmatch->value)
		return;

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

        // GRIM
        if (turns_on)
        {
                if (level.match_state == MATCH_CHECKING)
                {
                        if (level.match_time < level.time)
                                CheckMatchStart();
                }

                if (level.match_state == MATCH_PRE_START) // Begin match
                {
                        if (level.match_time < level.time)
                        {
                                //gi.dprintf ("MATCH_PRE_START\n");
                                SetupMatch();
                        }
                }

                if (level.match_state == MATCH_START) // Begin match
                {
                        if (level.match_time < level.time)
                        {
                                //gi.dprintf ("MATCH_START\n");
                                SoundToAllPlayers(gi.soundindex("fight.wav"), 1);
                                //SoundToAllPlayers(gi.soundindex("comment/fight.wav"), 1);
                                // Is this gonna crash this?
                                //HideAllSpots();

                                if (round_timelimit->value)
                                {
                                        i = round_timelimit->value;
                                        if (i < 3) // Min 3 minutes
                                                i = 3;
                                }
                                else
                                        i = 15;

                                level.match_time = level.time + (i*60);
                                level.match_state = MATCH_DURING;
                        }
                }

                // During a match, just check if over...
                if (level.match_state == MATCH_DURING)
                {
                        if (level.match_time < level.time)
                               MatchOutOfTime();
                        //else
                        //      MatchOverCheck();
                }

                // Match has finished, display winner...
                if (level.match_state == MATCH_FINISHED)
                {
                        if (level.match_time < level.time)
                        {
                                MatchWinner();
                                if (level.message)
                                {
                                        gi.bprintf (PRINT_MEDIUM,"%s\n", level.message);
                                        //DisplayCenterMessageAll(level.message);
                                        //gi.dprintf ("DisplayCenterMessageAll(level.message)\n");
                                }
                                PracticeReset();
                                //gi.dprintf ("MATCH IS OVER\n");
                                level.match_time = level.time + 2.5;
                                level.match_state = MATCH_CHECKING;
                        }
                }
        }
        // GRIM

        /*
	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
        */
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
        // GRIM
        // MONSTER CODE
        //if (!deathmatch->value)
        //        AI_SetSightClient ();
        // GRIM
        instant_lead = 0; // Dirty - Cleared each frame

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
                        // GRIM
                        // MONSTER CODE
                        /*
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
                        */
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
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
