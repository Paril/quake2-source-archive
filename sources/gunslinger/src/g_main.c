#include "g_local.h"
#include "z_gq.h"

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

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

cvar_t	*sv_maplist;

// Gunslinger Quake2 cvars -- Stone
cvar_t	*gamename;
cvar_t	*playmode;
cvar_t	*teamplay;
cvar_t	*team_damage;
cvar_t	*force_join;
cvar_t	*use_ctf_skins;
cvar_t	*clear_teams;
cvar_t	*number_of_teams;
cvar_t	*team1_name;
cvar_t	*team2_name;
cvar_t	*team3_name;
cvar_t	*team4_name;
cvar_t	*drop_badge;
cvar_t	*drop_bag;
cvar_t	*ready_all;
cvar_t	*use_classes;
cvar_t	*announce_deaths;
cvar_t	*respawn_invuln;
cvar_t	*weapons_unique;
cvar_t	*drop_all_on_death;
cvar_t	*bullet_ricochet;
cvar_t	*item_weight;
cvar_t	*hit_locations;
cvar_t	*number_of_money_bags;
cvar_t	*starting_cash;

cvar_t	*allow_hands_of_lightning;
cvar_t	*allow_strength_of_the_bear;
cvar_t	*allow_flesh_of_the_salamander;
cvar_t	*allow_aura_of_pain;
cvar_t	*allow_tongue_of_the_leech;
cvar_t	*allow_tail_of_the_scorpion;
cvar_t	*allow_shroud_of_darkness;
cvar_t	*artifact_min_timeout;
cvar_t	*artifact_max_timeout;
cvar_t	*artifact_glow;

cvar_t	*allow_cannon;
cvar_t	*allow_sheriff_badge;
cvar_t	*allow_marshal_badge;
cvar_t	*allow_bandolier;
cvar_t	*allow_pack;
cvar_t	*allow_lantern;
cvar_t	*allow_bandages;
cvar_t	*allow_laudanum;
cvar_t	*max_holes;

// Bot detection
cvar_t	*sv_botdetection;
cvar_t	*allow_download;
cvar_t	*allow_download_players;
cvar_t	*allow_download_models;
cvar_t	*allowed_models;
cvar_t	*bot_talk;
// End of Gunslinger cvars -- Stone

team_t		teamdata[MAX_TEAMS];
int			number_of_spawns;
int			number_of_bases;
int			*playerlist;	// Sorted player list
qboolean	matchstats;	// Intermission stats
int			intermissionframenum;
int			preptimeframe;
edict_t		*firsthole;
edict_t		*lasthole;
int			holes;
int			gameoverframe;		// For Last Man Standing

// End of Gunslinger globals -- Stone

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
int PlayerSort (void const *a, void const *b);

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
	int		i, cnt;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	if (deathmatch->value)
		cnt=maxclients->value*2;
	else
		cnt=maxclients->value;
	for (i=0 ; i<cnt ; i++)		// Doubled for antibot
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
	int i;
	
	// RPS Menu
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = g_edicts + 1 + i;
		if ((int)playmode->value==PM_BADGE_WARS)
			game.clients[i].team=1;
		RPS_MenuClose(ent);
	}

	// RPS Menu end

	//removes the anti bot -- Stone
	for_each_player(ent, i)		// this is in q_devels,h file
	{
		if (ent->flags & FL_ANTIBOT)
			BotDisconnect(ent);
	}

	if (antibot)
		antibot = false;
	// -- End

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
		// Stone
		float offset=0;
		if ((int)playmode->value == PM_LAST_MAN_STANDING) {
			if (preptimeframe>level.framenum) {
				offset=level.time+1;
			} else {
				offset=preptimeframe/10;
			}
		}
		if (level.time >= offset + (timelimit->value*60))
		{
//			debugmsg("Leveltime is %f, preptimeframe is %i\n", level.time, preptimeframe);
			gi_bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		if (teamplay->value) {
			for (i=0; i<(int)number_of_teams->value; i++) {
				if (teamdata[i].score >= fraglimit->value)
				{
					gi_bprintf (PRINT_HIGH, "Team score limit hit.\n");
					EndDMLevel ();
					return;
				}
				
			}
		}
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;
			
			if (cl->resp.score >= fraglimit->value)
			{
				gi_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && (gameoverframe==level.framenum)) {
		EndDMLevel();
	}
}

/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i, j;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	j = 0;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
//		debugmsg("ExitLevel %i\n", i);
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
		if (clear_teams->value)
			ent->client->team=0;

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
	int		count;

	debugmsg("G_RunFrame\n");
	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	if (!deathmatch->value)
		AI_SetSightClient ();

//	gi.error("Stopped at G_RunFrame\n");
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
		if (deathmatch->value)
			count=maxclients->value*2;
		else
			count=maxclients->value;
		if ((i > 0) && (i <= count))	// Doubled for antibot
		{
			ClientBeginServerFrame (ent);
			VectorCopy (ent->s.origin, ent->s.old_origin);
			continue;
		}

		G_RunEntity (ent);
	}
	MoveBot();

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// Gunslinger game logic -- Stone
	if (deathmatch->value) {
		// At the end of every frame, we recalculate rankings.  We'll use 
		// this information for all ranking related features: the scoreboards,
		// the player list, and the new HUD.  We do this because I am anal 
		// about CPU cycles, and there's no need to duplicate effort! -- Stone
		count = 0;
		for (i = 0 ; i < maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->client)
				continue;
			if (!ent->inuse)
				continue;
			if (!ent->client->pers.connected) {
				debugmsg("Not connected: %s\n", ent->client->pers.netname);
				continue;
			}
//			debugmsg("%s deadflag %i\n", ent->client->pers.netname, ent->deadflag);
			if ((int)playmode->value != PM_LAST_MAN_STANDING) {
//				if (ent->client->resp.spectator)
//					continue;
				if (!ent->client->team) {
					debugmsg("No team: %s\n", ent->client->pers.netname);
					continue;
				}
			} else if (ent->client->resp.spectator && !ent->client->resp.standard[GSTAT_DEATHS])
				continue;
			playerlist[count] = i;
			debugmsg("Score %s: %i\n", ent->client->pers.netname, ent->client->resp.score);
			count++;
		}

//		debugmsg("There are %i players\n",count);
		playerlist[count] = -1;
		// sort by frags
		qsort (playerlist, count, sizeof(playerlist[0]), PlayerSort);
	}

	// Let's give points to teams that deserve them
	if (deathmatch->value && ((int)playmode->value == PM_BIG_HEIST) 
		&& (level.intermissiontime == 0) 
		&& (playerlist[0]!=-1)
//		&& (level.framenum % 100 == 0)
		)
	{
		float low=9999.0;
//		debugmsg("Team points...\n");
		for (i=0 ;i<(int)number_of_teams->value; i++) {
			if (teamdata[i].vault->count < low)
				low=teamdata[i].vault->count;
		}
//		debugmsg("Low: %i\n", low);
		for (i=0 ;i<(int)number_of_teams->value; i++) {
			if (teamdata[i].vault->count > low)
				teamdata[i].score += (teamdata[i].vault->count - low)/100.0;
		}
	}

	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe >= level.framenum)) {
		if (ready_all->value) {
			int			i;
			edict_t		*ent;
			qboolean	allset=true;
			for (i=0 ; i<maxclients->value ; i++) {
				ent = g_edicts + 1 + i;
				if (!ent->inuse || !ent->client)
					continue;
				if (ent->client->resp.spectator)
					continue;
				if (!ent->client->team)
					continue;
				if (ent->deadflag)
					continue;
				if (!(ent->flags & FL_READY)) {
					allset=false;
					if (level.framenum % 100 == 0)
						gi_bprintf(PRINT_HIGH, "%s is not ready yet.\n", ent->client->pers.netname);
				}
			}
			if (allset) {
			} else {
				preptimeframe=level.framenum+35990;
			}
		}
		if (GQ_TeamsLeft()<2) {
			preptimeframe++;
			if ((level.framenum % 200) == 0)
				gi_bprintf(PRINT_HIGH, "Waiting for more players to join.\n");
		} else if (preptimeframe==level.framenum) {
			GQ_StartLastManStanding();
		} else if (((preptimeframe-level.framenum) < 100) && ((preptimeframe-level.framenum) % 10 == 0)) {
			gi_bprintf(PRINT_HIGH, "Game starts in %i seconds\n", (preptimeframe-level.framenum)/10);
		} else if ((preptimeframe-level.framenum) % 100 == 0) {
			gi_bprintf(PRINT_HIGH, "Game starts in %i seconds\n", (preptimeframe-level.framenum)/10);
		}
	}
	// End of Gunslinger game logic -- Stone

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

