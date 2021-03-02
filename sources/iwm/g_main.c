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
cvar_t	*iwm_gamemode;
cvar_t *cg_startweap;
cvar_t *cg_hints;
cvar_t *iwm_version;
cvar_t	*day;
cvar_t	*servermessage;
cvar_t	*iwm_editor;

//ZOID
cvar_t	*capturelimit;
cvar_t	*instantweap;
//ZOID

cvar_t *bfg_ban;
cvar_t *railgun_ban;
cvar_t *hyperblaster_ban;
cvar_t *rl_ban;
cvar_t *grenade_ban;
cvar_t *gl_ban;
cvar_t *chaingun_ban;
cvar_t *machinegun_ban;
cvar_t *ssg_ban;
cvar_t *shotgun_ban;
cvar_t *blaster_ban;
cvar_t *feature_ban;
cvar_t *item_ban;


//JABot [start]
//cvar_t	*bot_showpath;
//cvar_t	*bot_showcombat;
//cvar_t	*bot_showsrgoal;
//cvar_t	*bot_showlrgoal;
//cvar_t	*bot_debugmonster;
//[end]
int daycycletime;
int lightlevel;
int oldlightlevel;
int flies;

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;

char *currentmodelindexes[255];
int nummodelindexes;
char *modelindex_notamodel[255];

char *currentsoundindexes[255];
int numsoundindexes;

char *currentimageindexes[255];
int numimageindexes;
gitem_t *flag1_item;
gitem_t *flag2_item;

int level_gibs;
char *entities_glb;

FILE *filpt;

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

#ifdef _WIN32
extern HMODULE hModule;
#endif

void ShutdownGame (void)
{
#ifdef _WIN32
	gi.dprintf ("Freed editor DLL.\n");
	FreeLibrary (hModule);
#endif
	gi.dprintf ("==== ShutdownGame ====\n");

	if (filpt)
		fclose (filpt);

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}

// New gi.setmodel so that the modelindex
// will go through my code first.. like a bouncer
// or something.
void R_SetModel (edict_t *e, char *s)
{
	ModelIndex (s);
//	gi.dprintf ("Brush model of type %s was indexed, model %s\n", e->classname, s);
	if (s == NULL)
	{
		gi.dprintf ("%s tried to get a null setmodel.\n", e->classname);
		return;
	}

	gi.old_setmodel (e, s);
}

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
void debug_print (char *fmt, ...);

game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;
	InitLogging();

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

	gi.old_setmodel = gi.setmodel;
	gi.setmodel = R_SetModel;
	/*gi.old_multicast = gi.multicast;
	gi.old_unicast = gi.unicast;
	gi.unicast = Unicast;
	gi.multicast = Multicast;
	gi.old_WriteAngle = gi.WriteAngle;
	gi.old_WriteByte = gi.WriteByte;
	gi.old_WriteChar = gi.WriteChar;
	gi.old_WriteDir = gi.WriteDir;
	gi.old_WriteFloat = gi.WriteFloat;
	gi.old_WriteLong = gi.WriteLong;
	gi.old_WritePosition = gi.WritePosition;
	gi.old_WriteShort = gi.WriteShort;
	gi.old_WriteString = gi.WriteString;
	gi.WriteAngle = WriteAngle;
	gi.WriteByte = WriteByte;
	gi.WriteChar = WriteChar;
	gi.WriteDir = WriteDir;
	gi.WriteFloat = WriteFloat;
	gi.WriteLong = WriteLong;
	gi.WritePosition = WritePosition;
	gi.WriteShort = WriteShort;
	gi.WriteString = WriteString;*/
	gi.old_dprintf = gi.dprintf;
	gi.dprintf = debug_print;

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

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
/*void EndDMLevel (void)
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
}*/
void EndDMLevel (void)
{
   edict_t   *ent;

   Voting_KillVoting();
   
        // stay on same level flag
        if ((int)dmflags->value & DF_SAME_LEVEL)
           ent = CreateTargetChangeLevel(level.mapname);
       
        // get the next one out of the maplist
        else if (Maplist_Next()) //returns TRUE if maplist in use and map exists
           ent = CreateTargetChangeLevel(level.nextmap);
       
        // go to a specific map
        else if (level.nextmap[0])
           ent = CreateTargetChangeLevel(level.nextmap);
       
        // search for a changelevel within the current map.
        // this is for the single-player or coop game.
        else
        {
           ent = G_Find (NULL, FOFS(classname), "target_changelevel");
           if (!ent)
              // the map designer didn't include a changelevel,
              // so create a fake ent that goes back to the same level
              ent = CreateTargetChangeLevel(level.mapname);
        }
       
        gi.dprintf ("Map is changing to %s\n", level.nextmap);
       
        BeginIntermission (ent);
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

	Voting_CheckVoting();

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
			SendSoundToAll (SoundIndex("announcer/timelimit.wav"), 1.0);
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
				SendSoundToAll (SoundIndex("announcer/fraglimit.wav"), 1.0);
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

	//JABot[start] (Disconnect all bots before changing map)
//	BOT_RemoveBot("all");
	//[end]

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

// Paril
// Makes sure there are no more than 25 gibs present at once. Excludes heads.
#define MAX_LEVEL_GIBS 25
void CheckGibOverflow (void)
{
	edict_t *gib;
	int j = 0;

	while (level_gibs > MAX_LEVEL_GIBS)
	{
		for (gib = g_edicts + 1; gib < &g_edicts[game.maxentities]; gib++)
		{
			// Exit statement.
			if (j == 2000)
				goto exit;
			j++;
			// Stop whole statement if 25 gibs present now.
			if (level_gibs < MAX_LEVEL_GIBS+1)
				return;
			if (!gib->classname)
				continue;
			if (Q_stricmp(gib->classname, "gib") == 1)
				continue;
			if (!gib->s.effects & EF_GIB)
				continue;
			if (gib->is_gib != 1)
				continue;

			//gi.dprintf ("CheckGibOverflow freed a %s!\n", gib->classname);

			G_FreeEdict (gib);
		}
	}

exit:
	return;
}

void CheckLightningChains (edict_t *ent);

void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;
	char    lights[2]; // new line

	CheckGibOverflow();

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	/*if (day->value && level.dayoff)
		level.dayoff = 1;
	else
		level.dayoff = 0;*/

	// new code starts here  
	if (day->value)    
	{
		daycycletime = (int)((day->value) * 600 + 1);  
		level.dayoff = 1;
	}
	else    
		daycycletime = 0; 
	if (daycycletime > 0)  
	{    
		oldlightlevel = lightlevel;    
		lightlevel = level.framenum % (daycycletime);    
		if (lightlevel > (daycycletime / 2))       
			lightlevel = daycycletime - lightlevel;    
		lightlevel = 52 - (int)(((float) lightlevel / (float) (daycycletime/2)) * 52);    
		if (lightlevel <= 13)       
			lightlevel = 0;    
		else if (lightlevel >= 35)       
			lightlevel = 25;    
		else       
			lightlevel = lightlevel - 13; 

		lightlevel = 'c' + lightlevel; 
		if (lightlevel > 'z') 
			lightlevel = 'z' - (lightlevel - 'z');    
		
		lights[0] = lightlevel;    
		lights[1] = '\0';    
		gi.configstring(CS_LIGHTS+0, lights); 
	}
	else if (!daycycletime && level.dayoff)
	{
		lightlevel = 'm';
		oldlightlevel = 'm';
		lights[0] = lightlevel;    
		lights[1] = '\0';  
		gi.configstring(CS_LIGHTS+0, lights); 
		level.dayoff = 0;
	}
	
	if (day->value == -1)
	{
		lightlevel = 'a';
		oldlightlevel = 'a';
		lights[0] = lightlevel;    
		lights[1] = '\0';  
		gi.configstring(CS_LIGHTS+0, lights); 
	}

	// new code ends here

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
			//JABot[start]
//			if ( ent->ai.is_bot )
//				G_RunEntity (ent);
			//[end]
		//	continue;
		}

		G_RunEntity (ent);

		if (ent->lightning_chain == true)
			CheckLightningChains(ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	//JABot[start]
//	AITools_Frame();	//give think time to AI debug tools
	//[end]
}

