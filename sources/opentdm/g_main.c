/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

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
int			g_max_clients;

//cvar_t	*deathmatch;
//cvar_t	*coop;
cvar_t	*dmflags;
//cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*password;
cvar_t	*spectator_password;
cvar_t	*maxclients;
//cvar_t	*maxspectators;
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

cvar_t	*flood_waves;
cvar_t	*flood_waves_perminute;
cvar_t	*flood_waves_waitdelay;

cvar_t	*sv_maplist;

cvar_t	*g_team_a_name;
cvar_t	*g_team_b_name;
cvar_t	*g_locked_names;

cvar_t	*g_team_a_skin;
cvar_t	*g_team_b_skin;
cvar_t	*g_locked_skins;

cvar_t	*g_admin_password;
cvar_t	*g_admin_vote_decide;
cvar_t	*g_match_time;
cvar_t	*g_match_countdown;
cvar_t	*g_vote_time;
cvar_t	*g_vote_mask;
cvar_t	*g_intermission_time;
cvar_t	*g_force_screenshot;
cvar_t	*g_force_record;

cvar_t	*g_tdmflags;
cvar_t	*g_itdmflags;
cvar_t	*g_1v1flags;

cvar_t	*g_itemflags;
cvar_t	*g_powerupflags;

cvar_t	*g_tdm_allow_pick;

cvar_t	*g_fast_weap_switch;
cvar_t	*g_teleporter_nofreeze;
cvar_t	*g_overtime;
cvar_t	*g_tie_mode;
cvar_t	*g_gamemode;
cvar_t	*g_respawn_time;
cvar_t	*g_max_timeout;
cvar_t	*g_1v1_timeout;
cvar_t	*g_chat_mode;
cvar_t	*g_idle_time;

cvar_t	*g_http_enabled;
cvar_t	*g_http_bind;
cvar_t	*g_http_proxy;

cvar_t	*g_http_path;
cvar_t	*g_http_domain;

cvar_t	*g_debug_spawns;

cvar_t	*g_max_players_per_team;

cvar_t	*g_maplistfile;
cvar_t	*g_motd_message;

cvar_t	*g_bugs;

cvar_t	*g_allow_name_change_during_match;

cvar_t	*g_allow_vote_config;

cvar_t	*g_command_mask;

cvar_t	*g_auto_rejoin_match;
cvar_t	*g_auto_rejoin_map;

cvar_t	*g_1v1_spawn_mode;
cvar_t	*g_tdm_spawn_mode;

void SpawnEntities (const char *mapname, const char *entities, const char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void DummyWrite (const char *filename, qboolean autosave);
void DummyRead (const char *filename);
void InitGame (void);
void G_RunFrame (void);


//===================================================================


void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	gi.cvar_forceset ("g_features", "0");

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
game_export_t __attribute__ ((visibility("default"), externally_visible)) *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = DummyWrite;
	globals.ReadGame = DummyRead;
	globals.WriteLevel = DummyRead;
	globals.ReadLevel = DummyRead;

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
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text)-1, error, argptr);
	va_end (argptr);

	gi.error ("%s", text);
}

void Com_Printf (const char *msg, int level, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, level);
	vsnprintf (text, sizeof(text)-1, msg, argptr);
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

	//run now to copy results out to spectators
	for (i=0 ; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client)
			continue;

		if (ent->client->chase_mode == CHASE_LOCK)
		{
			UpdateLockCam (ent);
			continue;
		}
		else if (!ent->client->chase_target)
			continue;

		//this copies before the stats
		UpdateChaseCam (ent);
	}

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}


	//run now to copy results out to spectators
	for (i=0 ; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client || !ent->client->chase_target)
			continue;

		//this copies after the stats
		G_SetSpectatorStats (ent);
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
				{
					// end of list, go to first one
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
	{
		// search for a changelevel
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{
			// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
		BeginIntermission (ent);
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

	if (tdm_match_status == MM_SCOREBOARD)
		return;

	if (timelimit->value)
	{
		if (level.time >= timelimit->value * 600)
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		for (i=0 ; i < game.maxclients; i++)
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
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	/*int		i;
	edict_t	*ent;*/
	char	command [256];

	if (level.changemap)
	{
		Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
		gi.AddCommandString (command);
		level.exitintermission = 2;
	}
	else
		level.exitintermission = 0;
	
	level.intermissionframe = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	/*for (i=0 ; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->max_health)
			ent->health = ent->max_health;
	}*/

	//this should be called implicitly on new map, check
	//TDM_ResetGameState ();
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

	//FIXME: shouldn't we increment framenum after the game has run, in preparation for the next frame?
	//otherwise usercmds that arrive after we return will still use the old framenum, which seems wrong.
	//level.realframenum++;

	if (tdm_match_status != MM_TIMEOUT)
	{
		//level.framenum++;
		//level.time = level.framenum;// * FRAMETIME;

		// exit intermissions

		if (level.exitintermission)
		{
			if (level.exitintermission == 2)
			{
				//if we got here, the map change from ExitLevel didn't work
				gi.bprintf (PRINT_CHAT, "ERROR: Map '%s' was not found on the server.\n", level.changemap);
				level.exitintermission = 0;
				TDM_ResetGameState ();
			}
			else
			{
				ExitLevel ();
				return;
			}
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
			}

			if (i > 0 && i <= game.maxclients)
			{
				ClientBeginServerFrame (ent);
				continue;
			}

			G_RunEntity (ent);
		}
	}

	HTTP_RunDownloads ();

	TDM_UpdateConfigStrings (false);

	TDM_CheckTimes ();
	
	// see if it is time to end a deathmatch
	//CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	level.realframenum++;

	if (tdm_match_status != MM_TIMEOUT)
	{
		level.framenum++;
		level.time = level.framenum;// * FRAMETIME;
	}
}
