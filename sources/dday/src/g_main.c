/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_main.c,v $
 *   $Revision: 1.15 $
 *   $Date: 2002/07/23 21:11:37 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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
#include "g_cmds.h"
#include "g_dll.h"

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

//Flyingnun Password
cvar_t  *flyingnun_password;

//KMM: 9/15  CVARS
//cvar_t	*scope_setting;				// Sniperscope FOV
//cvar_t	*auto_reload;				// change magazine automaticly?
//cvar_t	*auto_weapon_change;		// change weapon when out of ammo?
//cvar_t	*observer_on_death;			// if set, go to observer when dead
//cvar_t	*heal_wounds;			// if set, health fixes chest and stomach wounds
//cvar_t	*flame_shots;				// the number of shots a flamethrower shoots
cvar_t  *RI;						// reinforcement interval -- the length of time before reinforcements are spawned.
cvar_t	*team_kill;					// friendly fire toggle (default off)
cvar_t	*class_limits;				// enable class limits
cvar_t	*spawn_camp_check;			// enable spawn camping checks
cvar_t	*spawn_camp_time;			// the amount of time after spawn to check for spawn camps
cvar_t	*invuln_medic;				// if medics are invulnerable
//cvar_t	*teamgren;					// enables "team grenade mode" where the grenades are team-conscience, a soldier can not pick up his own team's grenade
cvar_t	*death_msg;					// when and how the death msgs are displayed
cvar_t	*level_wait;				// pause time at beginning and end of games
cvar_t	*invuln_spawn;				// how long player is invulnerable after spawn

cvar_t	*arty_delay;				// seconds for artillary to position
cvar_t	*arty_time;					// seconds between each volley 
cvar_t  *arty_max;					// number of shots to be fired in each volley

//bcass start - easter_egg cvar, AGAIN
cvar_t	*easter_egg;				//turns easter eggs on and off
//bcass end

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

//faf
cvar_t  *player_scores;
cvar_t  *campaign_mode;
cvar_t  *max_gibs;
cvar_t  *extra_anims;//faf
cvar_t  *force_auto_select;

cvar_t  *allied_password;
cvar_t  *axis_password;


//cvar_t	*crosshair;

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
void CleanUpCmds();
void PBM_KillAllFires (void);
void ClearUserDLLs(void);
void LevelExitUserDLLs(void);



//===================================================================


//=============================================================
// So the Server can execute a console command for the Client
// as if the client typed it into their console themselves.
//============================================================
void stuffcmd(edict_t *ent, char *s) 
{
	gi.WriteByte(11);
	gi.WriteString(s);
	gi.unicast(ent, true);
}


void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	CleanUpCmds();
	ClearUserDLLs();

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
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;
	int i = 0;

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname;
	}

//	else if ((int)dmflags->value & DF_MAP_LIST)  // maplist active? 
	else if ((int)dmflags->value & DF_MAP_LIST  && maplist.nummaps > 0)  // faf: fixes crash
	{ 
		switch (maplist.rotationflag)        // choose next map in list 
		{ 
		case ML_ROTATE_SEQ:        // sequential rotation 
			i = (maplist.currentmap + 1) % maplist.nummaps; 
			break; 
		
		case ML_ROTATE_RANDOM:     // random rotation 
			i = (int) (random() * maplist.nummaps); 
			break;
		
		default:       // should never happen, but set to first map if it does 
			i = 0; 
		} // end switch 


		maplist.currentmap = i; 
		
		ent = G_Spawn (); 
		ent->classname = "target_changelevel"; 
		if (maplist.mapnames[i] && !level.nextmap[0])
			ent->map = maplist.mapnames[i]; 
		else if (level.nextmap[0])
			ent->map = level.nextmap;
		else
			ent->map = level.mapname;
	} 
	
	else if (Last_Team_Winner <= 1 && team_list[Last_Team_Winner] && team_list[Last_Team_Winner]->nextmap)
	{
		ent= G_Spawn();
		ent->classname = "target_changelevel";
		ent->map = team_list[Last_Team_Winner]->nextmap;
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
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i=0,tempscore=0;
	float		delay=0.0;
	qboolean Is_Game_Over=false;
	gclient_t	*cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;


	for(i=0; i < MAX_TEAMS;i++) {
		if (!team_list[i])
			break;
		
		if (team_list[i]->time_to_win) {

			delay = (team_list[i]->time_to_win - level.time);
		
		/*	gi.dprintf("time_to_win [%i] = %f\n", i, team_list[i]->time_to_win);
			gi.dprintf("delay       [%i] = %f\n", i, delay);
			gi.dprintf("level.time  [%i] = %f\n", i, level.time); */

			if (delay <= 0) {
				gi.bprintf( PRINT_HIGH, "Team %s is victorious!\n", team_list[i]->teamname);
				Last_Team_Winner=i;
				EndDMLevel ();
				break;
			} 

			else if (delay == 30)
				gi.bprintf ( PRINT_HIGH, "30 seconds left before team %s wins the battle!\n", team_list[i]->teamname);
			else if (delay == 60)
				gi.bprintf ( PRINT_HIGH, "1 minute left before team %s wins the battle!\n", team_list[i]->teamname);
			else if (delay == 300)
				gi.bprintf ( PRINT_HIGH, "5 minutes left before team %s wins the battle!\n", team_list[i]->teamname);
		}

		if ((team_list[i]->need_kills > 0) && team_list[i]->kills >= team_list[i]->need_kills) 
		{
			gi.bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i kills)!\n", 
				team_list[i]->teamname, 
				team_list[i]->kills,
				team_list[i]->need_kills);

			Last_Team_Winner=i;
			EndDMLevel ();
			break;
		}

		
		if ((team_list[i]->need_points > 0) && team_list[i]->score >= team_list[i]->need_points) 
		{
			gi.bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i points)!\n", 
				team_list[i]->teamname, 
				team_list[i]->score,
				team_list[i]->need_points);

			Last_Team_Winner=i;
			EndDMLevel ();
			break;
		}

	}

	if(Is_Game_Over)
	{
		for(i=0;i<MAX_TEAMS;i++)
		{
			if(team_list[i]->score > tempscore) 
			{
				tempscore=team_list[i]->score;
				Last_Team_Winner=i;
			}
		}
		gi.bprintf (PRINT_HIGH, "%s is victorius.\n",team_list[Last_Team_Winner]->teamname);
		EndDMLevel ();
		return;
	}

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
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
	int		i;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();
/*-----/ PM /-----/ NEW:  Remove all flames before leaving level. /-----*/
    PBM_KillAllFires ();
/*----------------------------------------------------------------------*/


	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}

	LevelExitUserDLLs();
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
	//AI_SetSightClient ();

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
		/*if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}*/

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
		}

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

