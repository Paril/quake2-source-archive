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

cvar_t	*filterban;

//ZOID
cvar_t	*capturelimit;
//ZOID
cvar_t	*password;
cvar_t	*spectator_password;
cvar_t	*maxclients;
cvar_t	*maxspectators;
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
cvar_t  *sv_fps;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

//ponpoko
cvar_t	*gamepath;
cvar_t	*vwep;
cvar_t	*sv_maplist;
float	spawncycle;
//ponpoko

//K03 Begin
cvar_t *killboxspawn;
cvar_t *save_path;
cvar_t *particles;

cvar_t *sentry_lev1_model;
cvar_t *sentry_lev2_model;
cvar_t *sentry_lev3_model;
cvar_t *nextlevel_mult;
cvar_t *vrx_creditmult;
cvar_t *vrx_pvpcreditmult;
cvar_t *vrx_pvmcreditmult;

cvar_t *invasion_enabled;
cvar_t *start_level;
cvar_t *start_nextlevel;

cvar_t *vrx_pointmult;
cvar_t *vrx_pvppointmult;
cvar_t *vrx_pvmpointmult;
cvar_t *vrx_sub10mult;
cvar_t *vrx_over10mult;

cvar_t *flood_msgs;
cvar_t *flood_persecond;
cvar_t *flood_waitdelay;
qboolean MonstersInUse;
int total_monsters;
edict_t *SPREE_DUDE;
edict_t	*red_base; // 3.7 CTF
edict_t *blue_base; // 3.7 CTF
int red_flag_caps;
int blue_flag_caps;
int	DEFENSE_TEAM;
int PREV_DEFENSE_TEAM;
long FLAG_FRAMES;
float SPREE_TIME;
int average_player_level;
int pvm_average_level;
qboolean SPREE_WAR;
qboolean INVASION_OTHERSPAWNS_REMOVED;
int invasion_difficulty_level;
qboolean found_flag;

cvar_t *gamedir;
cvar_t *dm_monsters;
cvar_t *pvm_respawntime;
cvar_t *pvm_monstermult;
cvar_t *ffa_respawntime;
cvar_t *ffa_monstermult;
cvar_t *server_email;
cvar_t *reconnect_ip;
cvar_t *vrx_password;
cvar_t *min_level;
cvar_t *max_level;
cvar_t *check_dupeip;
cvar_t *check_dupename;
cvar_t *newbie_protection;
cvar_t *pvm;
cvar_t *trading;
cvar_t *tradingmode_enabled;
cvar_t *ptr;
cvar_t *ffa;
cvar_t *domination;
cvar_t *hw; // vrxchile 3.0
cvar_t *tbi; // vrxchile 3.4
cvar_t *ctf;
cvar_t *invasion;
cvar_t *nolag;
cvar_t *debuginfo;
cvar_t *adminpass;
cvar_t *team1_skin;
cvar_t *team2_skin;
cvar_t *voting;
cvar_t *game_path;
cvar_t *allies;
cvar_t *pregame_time;

// class skins
cvar_t *enforce_class_skins;
cvar_t *class1_skin;
cvar_t *class2_skin;
cvar_t *class3_skin;
cvar_t *class4_skin;
cvar_t *class5_skin;
cvar_t *class6_skin;
cvar_t *class7_skin;
cvar_t *class8_skin;
cvar_t *class9_skin;
cvar_t *class10_skin;
cvar_t *class11_skin;
cvar_t *class12_skin;

cvar_t *class1_model;
cvar_t *class2_model;
cvar_t *class3_model;
cvar_t *class4_model;
cvar_t *class5_model;
cvar_t *class6_model;
cvar_t *class7_model;
cvar_t *class8_model;
cvar_t *class9_model;
cvar_t *class10_model;
cvar_t *class11_model;
cvar_t *class12_model;

// world spawn ammo
cvar_t *world_min_bullets;
cvar_t *world_min_cells;
cvar_t *world_min_shells;
cvar_t *world_min_grenades;
cvar_t *world_min_rockets;
cvar_t *world_min_slugs;

cvar_t *ctf_enable_balanced_fc;

// Force vote control
cvar_t* adminctrl;
// 4x style ab system
cvar_t *generalabmode;
//K03 End

void SpawnEntities(char *mapname, char *entities, char *spawnpoint);
void ClientThink(edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void ClientBegin(edict_t *ent, qboolean loadgame);
void ClientCommand(edict_t *ent);
void RunEntity(edict_t *ent);
void WriteGame(char *filename);
void ReadGame(char *filename);
void WriteLevel(char *filename);
void ReadLevel(char *filename);
void InitGame(void);
void G_RunFrame(void);
void dom_init(void);
void dom_awardpoints(void);
void PTRCheckJoinedQue(void);

//===================================================================


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
void ShutdownGame(void)
{
	//K03 Begin
	int i;
	edict_t *ent;


	for_each_player(ent, i)
	{
		//ent->myskills.inuse = 0;
#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
		if (savemethod->value == 2)
			SaveCharacterQuit(ent); //WriteMyCharacter(ent);
		else
#endif
			SaveCharacter(ent);

	}


	//K03 End
	gi.dprintf("==== ShutdownGame ====\n");

#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	GDS_FinishThread();
#endif

	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);
}

#ifndef _WINDOWS
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
game_export_t *GetGameAPI(game_import_t *import)
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
void Sys_Error(char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	gi.error(ERR_FATAL, "%s", text);
}

void Com_Printf(char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, msg);
	vsprintf(text, msg, argptr);
	va_end(argptr);

	gi.dprintf("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames(void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i = 0; i<maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame(ent);
	}

}
//K03 Begin
/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	gi.dprintf("Next map is %s.\n", ent->map);
	return ent;
}
//K03 End


/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/

void VortexEndLevel(void)
{
	int		i;
	edict_t *tempent;

	INV_AwardPlayers();

	gi.dprintf("Vortex is shutting down...\n");

	CTF_ShutDown(); // 3.7 shut down CTF, remove flags and bases
	clearallmenus();
	InitJoinedQueue();
	InitializeTeamNumbers(); // for allies

	for_each_player(tempent, i)
	{
		PM_RemoveMonster(tempent);
		VortexRemovePlayerSummonables(tempent);
		tempent->myskills.streak = 0;
#ifndef NO_GDS
		if (savemethod->value == 2)
			SaveCharacterQuit(tempent);
		else
#endif
			SaveCharacter(tempent);
		if (G_EntExists(tempent))
			WriteToLogfile(tempent, "Logged out.\n");
		else
			WriteToLogfile(tempent, "Disconnected from server.\n");
	}

	gi.dprintf("INFO: All players saved.\n");

	SPREE_WAR = false;
	SPREE_DUDE = NULL;
	INVASION_OTHERSPAWNS_REMOVED = false;

	gi.dprintf("OK!\n");
}

void EndDMLevel(void)
{
	edict_t *ent;
	int found_map = 0;
	static const char *seps = " ,\n\r";
	//GHz START
	int modenum = 0;
	VortexEndLevel();
	//GHz END

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		//BeginIntermission (CreateTargetChangeLevel(level.mapname));
		VortexBeginIntermission(level.mapname);
		return;
	}

	//3.0 Begin new voting/mapchange code
	if (voting->value)
	{
		int mode = V_AttemptModeChange(true);
		v_maplist_t *maplist;
		int mapnum;
		qboolean changing = false; // vrc 2.32: A small technical thing and q2pro server.

		//Is the game mode changing?
		if (mode)
		{
			//Alert everyone
			switch (mode)
			{
			case MAPMODE_PVP:
				if (pvm->value || domination->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Player Vs. Player (PvP) mode!\n");
					changing = true;
				}
				break;
			case MAPMODE_PVM:
				if (!pvm->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Player Vs. Monster (PvM) mode!\n");
				}
				changing = true; // q2pro hack
				break;
			case MAPMODE_DOM:
				if (!domination->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Domination mode!\n");
					changing = true;
				}
				break;
			case MAPMODE_CTF:
				if (!ctf->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to CTF mode!\n");
					changing = true;
				}
				break;
			case MAPMODE_FFA:
				if (!ffa->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Free For All (FFA) mode!\n");
					changing = true;
				}
				break;
			case MAPMODE_INV:
				if (!invasion->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Invasion mode!\n");
				}
				changing = true;
				break;
			case MAPMODE_TRA:
				if (!trading->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Trading mode!\n");
					changing = true;
				}
				break;
			case MAPMODE_VHW:
				if (!hw->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Vortex Holywars!\n");
					changing = true;
				}
				break;
			case MAPMODE_TBI:
				if (!tbi->value)
				{
					gi.bprintf(PRINT_HIGH, "Switching to Destroy The Spawn mode!\n");
					changing = true;
				}
				break;
			}

			level.modechange = changing;

			//gi.dprintf("changing to mode %d\n", mode);

			//Select the map with the most votes
			mapnum = FindBestMap(mode);

			//gi.dprintf("mapnum=%d\n",mapnum);

			//Point to the correct map list
			maplist = GetMapList(mode);

			if (mapnum == -1)
			{
				//Select a random map for this game mode
				mapnum = GetRandom(0, maplist->nummaps - 1);
			}

			//Change the map/mode
			V_ChangeMap(maplist, mapnum, mode);
		}
		else
		{
			//gi.dprintf("mode not changing\n");

			//Find out what current game mode is
			if (pvm->value)
			{
				if (invasion->value == 1)
					mode = MAPMODE_INV;
				else if (invasion->value == 2)
					mode = MAPMODE_INH;
				else
					mode = MAPMODE_PVM;
			}
			else if (domination->value)
				mode = MAPMODE_DOM;
			else if (ctf->value)
				mode = MAPMODE_CTF;
			else if (ffa->value)
				mode = MAPMODE_FFA;
			else if (hw->value)
				mode = MAPMODE_VHW;
			else if (tbi->value)
				mode = MAPMODE_TBI;
			else mode = MAPMODE_PVP;

			if (tradingmode_enabled->value && JoinedPlayers() == 0)
			{
				mode = MAPMODE_TRA; // default to trading mode when no people's in
			}

			//Point to the correct map list
			maplist = GetMapList(mode);

			//Try to find a map that was voted for
			mapnum = FindBestMap(mode);

			if (mapnum == -1)
			{
				int i = 0;

				//Select a random map for this game mode
				while (1)
				{
					int max = maplist->nummaps - 1;
					// get a random map index from the map list
					if (max <= 0)
					{
						mapnum = 0;
						gi.dprintf("Maplist for mode %d has no maps.\n", mode);
						break;
					}
					mapnum = GetRandom(0, maplist->nummaps - 1);

					// is this the same map?
					if (Q_stricmp(level.mapname, maplist->maps[mapnum].name) != 0)
						break;
					// don't crash
					if (i > 1000)
						break;
					i++;
				}
			}
			//else
			//gi.dprintf("picking best map\n");

			//Change the map/mode
			V_ChangeMap(maplist, mapnum, mode);
		}
	}

	//3.0 end voting (doomie)

	//This should always be true now
	if (level.nextmap)
	{	// go to a specific map
		//BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
		VortexBeginIntermission(level.nextmap);
	}
	else
	{
		// search for a changelevel
		ent = G_Find(NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			//BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			VortexBeginIntermission(level.nextmap);
			return;
		}
		BeginIntermission(ent);
	}
}


/*
=================
CheckNeedPass
=================
*/
void CheckNeedPass(void)
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

void SP_target_speaker(edict_t *ent);

void CheckDMRules(void)
{
	int			i, check;//K03
	float		totaltime = ((timelimit->value * 60) - level.time);//K03 added totaltime
	gclient_t	*cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	if (timelimit->value)
	{
		//K03 Begin
		//Spawn monsters every 120 seconds
		check = (int)(totaltime) % 120;

		if (voting->value && (level.time >= (timelimit->value - 3) * 60) &&
			(maplist.warning_given == false))
		{
			if (!invasion->value)
				G_PrintGreenText(va("***** 3 Minute Warning: Type 'vote' to place your vote for the next map and game type *****\n"));
			else
				G_PrintGreenText("***** Only 3 minutes left! *****\n");
			maplist.warning_given = true;
		}



		if (level.time >= ((float)timelimit->value*60.0 - 30.0))
		{
			if (maplist.sounds[1] != 1)
			{
				gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("invasion/30sec.wav"), 1, ATTN_NONE, 0);
				maplist.sounds[1] = 1;
			}
		}

		if (level.time >= ((float)(timelimit->value*60.0) - 20.0))
		{
			if (maplist.sounds[2] != 1)
			{
				gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("invasion/20sec.wav"), 1, ATTN_NONE, 0);
				maplist.sounds[2] = 1;
			}
		}

		if (level.time >= (((float)(timelimit->value)*60.0) - 10.0))
		{
			if (maplist.sounds[3] != 1)
			{
				gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
				maplist.sounds[3] = 1;
			}
		}

		//K03 End
		if (level.time >= timelimit->value * 60)
		{
			gi.bprintf(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel();

			if (invasion->value > 1) // we hit timelimit on hard mode invasion = we win
			{
				int i, num_winners = 0;
				edict_t *speaker, *player;

				for (i = 0; i<game.maxclients; i++)
				{
					player = g_edicts + 1 + i;
					if (!player->inuse)
						continue;
					if (!G_IsSpectator(player)) // if players actually played..
					{
						if (player->client && player->client->pers.score)
							num_winners++;
					}
				}

				if (num_winners) // then we have a hard victory, woot!
				{
					speaker = G_Spawn();
					st.noise = "invasion/hard_victory.wav";
					speaker->spawnflags |= 1;
					speaker->attenuation = 1;
					speaker->volume = 1;
					VectorCopy(level.intermission_origin, speaker->s.origin);
					SP_target_speaker(speaker);
				}

			}

			return;
		}
	}

	if (fraglimit->value)
	{
		for (i = 0; i<maxclients->value; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i + 1].inuse)
				continue;

			//K03 Begin

			if (voting->value && (cl->resp.frags >= (fraglimit->value - 5)) &&
				(maplist.warning_given == false))
			{
				G_PrintGreenText("***** 5 Frags Remaining: Type 'vote' to place your vote for the next map and game type *****");
				maplist.warning_given = true;
			}
			//K03 End

			if (cl->resp.frags >= fraglimit->value)
			{
				gi.bprintf(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel();
				return;
			}
		}
	}

	/* az: moved the invasion check to info_player_invasion_death */
}


/*
=============
ExitLevel
=============
*/
void ExitLevel(void)
{
	int		i;
	edict_t	*ent;
	char	command[256];

	//JABot[start] (Disconnect all bots before changing map)
	BOT_RemoveBot("all");
	//[end]

	//GHz START
	VortexEndLevel();
	//GHz END
	if (level.changemap)
	{
#ifdef Q2PRO_COMPATIBILITY
		Com_sprintf(command, sizeof(command), "map \"%s\"\n", level.changemap);
#else
		Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
#endif
	}
	else if (level.nextmap)
	{
#ifdef Q2PRO_COMPATIBILITY
		Com_sprintf(command, sizeof(command), "map \"%s\"\n", level.nextmap);
#else
		Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.nextmap);
#endif
	}
	else
	{
		//default to q2dm1 and give an error
		gi.dprintf("ERROR in ExitLevel()!!\nlevel.changemap = %s\nlevel.nextmap = %s\n", level.changemap, level.nextmap);
		Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", "q2dm1");
	}

	gi.AddCommandString(command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames();

	// clear some things before going to next level
	for (i = 0; i<maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}
}

void tech_spawnall(void);

void G_RunPregame()
{
	edict_t *ent;
	int i;
	if (level.time <= pregame_time->value && !trading->value)
	{
		if (level.time == pregame_time->value - 30) {
			gi.bprintf(PRINT_HIGH, "30 seconds left of pre-game\n");

			for (i = 0; i < maxclients->value; i++) {
				ent = &g_edicts[i];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				//	if (ent->client->disconnect_time > 0)
				//continue;

				safe_cprintf(ent, PRINT_HIGH, "You will not be able to access the Armory in the game\n");
			}
		}
		if (level.time == pregame_time->value - 10)
			gi.bprintf(PRINT_HIGH, "10 seconds left of pre-game\n");
		if (level.time == pregame_time->value - 5)
		{
			gi.bprintf(PRINT_HIGH, "5 seconds\n");
			gi.sound(world, CHAN_VOICE, gi.soundindex("5_0.wav"), 1, ATTN_NONE, 0);
		}
		if (level.time == pregame_time->value - 4)
			gi.bprintf(PRINT_HIGH, "4\n");
		if (level.time == pregame_time->value - 3)
			gi.bprintf(PRINT_HIGH, "3\n");
		if (level.time == pregame_time->value - 2)
			gi.bprintf(PRINT_HIGH, "2\n");
		if (level.time == pregame_time->value - 1)
			gi.bprintf(PRINT_HIGH, "1\n");
		if (level.time == pregame_time->value) {
			gi.bprintf(PRINT_HIGH, "Game commences!\n");

			if (!invasion->value && !hw->value)
				gi.sound(world, CHAN_VOICE, gi.soundindex("misc/fight.wav"), 1, ATTN_NONE, 0);
			else if (invasion->value)
				gi.sound(world, CHAN_VOICE, gi.soundindex("invasion/fight_invasion.wav"), 1, ATTN_NONE, 0);
			else if (hw->value)
				gi.sound(world, CHAN_VOICE, gi.soundindex("hw/hw_spawn.wav"), 1, ATTN_NONE, 0);

			tech_spawnall();

			if (domination->value)
				dom_init();

			if (ctf->value)
				CTF_Init();
			// az begin
			if (hw->value)
				hw_init();

			if (tbi->value)
				InitTBI();

			for (i = 0; i < maxclients->value; i++) {
				ent = &g_edicts[i];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;

				//r1: fixed illegal effects being set
				ent->s.effects &= ~EF_COLOR_SHELL;
				ent->s.renderfx &= ~RF_SHELL_RED;

				//RemoveAllCurses(ent);
				//RemoveAllAuras(ent);
				AuraRemove(ent, 0);
				CurseRemove(ent, 0);
				ent->Slower = level.time - 1;
			}
		}
	}
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/

void thinkDisconnect(edict_t *ent);
void G_InitEdict(edict_t *e);
void check_for_levelup(edict_t *ent);//K03
void RunVotes();
void G_RunFrame(void)
{
	int		i;//j;
	static int ofs;
	static float next_fragadd = 0;
	qboolean delta = false;
	edict_t	*ent;
	qboolean haveflag;

#if (defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	ProcessQueue(NULL);
#endif

#ifndef FIXED_FT
	level.real_framenum++;

	if (!(level.real_framenum % (int)(sv_fps->value / 10)))
	{
		delta = true;
		level.framenum++;
	}
#else
	level.framenum++;
#endif

	level.time = level.framenum*FRAMETIME;

#ifndef FIXED_FT
	if (!delta)
		return;
#endif

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel();
		return;
	}

	if (spawncycle < level.time)
		spawncycle = level.time + FRAMETIME * 10;
	if (spawncycle < 130)
		spawncycle = 130;

	RunVotes();

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	haveflag = false;
	ent = &g_edicts[0];
	for (i = 0; i<globals.num_edicts; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		//4.0 reset chainlightning flag
		ent->flags &= ~FL_CLIGHTNING;

		level.current_entity = ent;

		VectorCopy(ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if (!(ent->flags & (FL_SWIM | FL_FLY)) && (ent->svflags & SVF_MONSTER))
			{
				M_CheckGround(ent);
			}
		}


		if (i > 0 && i <= maxclients->value && !(ent->svflags & SVF_MONSTER))
		{
			ClientBeginServerFrame(ent);

			// JABot[start]
			if (ent->ai.is_bot)
				G_RunEntity(ent);
			//[end]

			continue;
		}

		G_RunEntity(ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules();

	// see if needpass needs updated
	CheckNeedPass();

	// build the playerstate_t structures for all players
	ClientEndServerFrames();

	//JABot[start]
	AITools_Frame();
	//[end]

	//3.0 Remove votes by players who left the server
	//Every 5 minutes
#ifdef OLD_VOTE_SYSTEM // Paril
	if (!(level.framenum % 3000))
		CheckPlayerVotes();
#endif
	//3.0 END 

	G_RunPregame();

	if ((pvm->value || ffa->value) && !(level.framenum % 10))
		CreateRandomPlayerBoss(false);

	// az end
	if (domination->value && (level.time > pregame_time->value))
		dom_awardpoints();

	if (hw->value && (level.time > pregame_time->value))
		hw_awardpoints();

	PTRCheckJoinedQue();
	INV_SpawnPlayers();
}
