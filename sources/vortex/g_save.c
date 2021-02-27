#include "g_local.h"

//K03 Begin
void InitializeGDS(void);
//#define LOCK_DEFAULTS 1
//K03 End

cvar_t				*bot_showpath;
cvar_t				*bot_showcombat;
cvar_t				*bot_showsrgoal;
cvar_t				*bot_showlrgoal;
cvar_t				*bot_debugmonster;

field_t fields[] = {
	{ "classname", FOFS(classname), F_LSTRING },
	{ "origin", FOFS(s.origin), F_VECTOR },
	{ "model", FOFS(model), F_LSTRING },
	{ "spawnflags", FOFS(spawnflags), F_INT },
	{ "speed", FOFS(speed), F_FLOAT },
	{ "accel", FOFS(accel), F_FLOAT },
	{ "decel", FOFS(decel), F_FLOAT },
	{ "target", FOFS(target), F_LSTRING },
	{ "targetname", FOFS(targetname), F_LSTRING },
	{ "pathtarget", FOFS(pathtarget), F_LSTRING },
	{ "deathtarget", FOFS(deathtarget), F_LSTRING },
	{ "killtarget", FOFS(killtarget), F_LSTRING },
	{ "combattarget", FOFS(combattarget), F_LSTRING },
	{ "message", FOFS(message), F_LSTRING },
	{ "team", FOFS(team), F_LSTRING },
	{ "wait", FOFS(wait), F_FLOAT },
	{ "delay", FOFS(delay), F_FLOAT },
	{ "random", FOFS(random), F_FLOAT },
	{ "move_origin", FOFS(move_origin), F_VECTOR },
	{ "move_angles", FOFS(move_angles), F_VECTOR },
	{ "style", FOFS(style), F_INT },
	{ "count", FOFS(count), F_INT },
	{ "health", FOFS(health), F_INT },
	{ "sounds", FOFS(sounds), F_INT },
	{ "light", 0, F_IGNORE },
	{ "dmg", FOFS(dmg), F_INT },
	{ "angles", FOFS(s.angles), F_VECTOR },
	{ "angle", FOFS(s.angles), F_ANGLEHACK },
	{ "mass", FOFS(mass), F_INT },
	{ "volume", FOFS(volume), F_FLOAT },
	{ "attenuation", FOFS(attenuation), F_FLOAT },
	{ "map", FOFS(map), F_LSTRING },
	//arena
	{ "arena", FOFS(arena), F_INT },
	//end arena
	// temp spawn vars -- only valid when the spawn function is called
	{ "lip", STOFS(lip), F_INT, FFL_SPAWNTEMP },
	{ "distance", STOFS(distance), F_INT, FFL_SPAWNTEMP },
	{ "height", STOFS(height), F_INT, FFL_SPAWNTEMP },
	{ "noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP },
	{ "pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP },
	{ "item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP },
	{ "gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP },
	{ "sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP },
	{ "skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP },
	{ "skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP },
	{ "minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP },
	{ "maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP },
	{ "minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP },
	{ "maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP },
	{ "nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP },
	{ NULL, 0, F_IGNORE, FFL_SPAWNTEMP }
};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

field_t		savefields[] =
{
	{ "", FOFS(classname), F_LSTRING },
	{ "", FOFS(target), F_LSTRING },
	{ "", FOFS(targetname), F_LSTRING },
	{ "", FOFS(killtarget), F_LSTRING },
	{ "", FOFS(team), F_LSTRING },
	{ "", FOFS(pathtarget), F_LSTRING },
	{ "", FOFS(deathtarget), F_LSTRING },
	{ "", FOFS(combattarget), F_LSTRING },
	{ "", FOFS(model), F_LSTRING },
	{ "", FOFS(map), F_LSTRING },
	{ "", FOFS(message), F_LSTRING },

	{ "", FOFS(client), F_CLIENT },
	{ "", FOFS(item), F_ITEM },

	{ "", FOFS(goalentity), F_EDICT },
	{ "", FOFS(movetarget), F_EDICT },
	{ "", FOFS(enemy), F_EDICT },
	{ "", FOFS(oldenemy), F_EDICT },
	{ "", FOFS(activator), F_EDICT },
	{ "", FOFS(groundentity), F_EDICT },
	{ "", FOFS(teamchain), F_EDICT },
	{ "", FOFS(teammaster), F_EDICT },
	{ "", FOFS(owner), F_EDICT },
	// {"", FOFS(mynoise), F_EDICT},
	// {"", FOFS(mynoise2), F_EDICT},
	{ "", FOFS(target_ent), F_EDICT },
	{ "", FOFS(chain), F_EDICT },

	{ NULL, 0, F_INT }
};

field_t		levelfields[] =
{
	{ "", LLOFS(changemap), F_LSTRING },

	{ "", LLOFS(sight_client), F_EDICT },
	{ "", LLOFS(sight_entity), F_EDICT },
	{ "", LLOFS(sound_entity), F_EDICT },
	{ "", LLOFS(sound2_entity), F_EDICT },

	{ NULL, 0, F_INT }
};

field_t		clientfields[] =
{
	{ "", CLOFS(pers.weapon), F_ITEM },
	{ "", CLOFS(pers.lastweapon), F_ITEM },
	{ "", CLOFS(newweapon), F_ITEM },

	{ NULL, 0, F_INT }
};

//K03 Begin
int config_map_list()
{
	int  numberOfMapsInFile = 0;
	char sMapName[80];
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

	// see if it's in the map list
	if (*sv_maplist->string) {
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);
		while (t != NULL) {
			if (t != NULL)
			{
				sprintf(sMapName, "%s", t);
				strncpy(maplist.mapnames[numberOfMapsInFile], sMapName, MAX_MAPNAME_LEN);
				maplist.voteonly[numberOfMapsInFile] = false;
				numberOfMapsInFile++;
			}
			else
				break;
			t = strtok(NULL, seps);
		}
		free(s);
	}

	if (numberOfMapsInFile == 0)
	{
		gi.dprintf("Config: sv_maplist not defined.\n");
		return 0;  // abnormal exit -- no maps in file
	}

	maplist.nummaps = numberOfMapsInFile;
	if (maplist.nummaps)
		maplist.active = 1;

	//maplist.rotationflag = 0;
	maplist.currentmap = -1;
	//	FillMapNames(); //prepare menu
	return 1;     // normal exit
}
//K03 End

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/

// az begin
void InitHash();
void InitializeAbilityList();
void CheckDir(char* path);

// q2pro gmf
#define GMF_VARIABLE_FPS            0x00000800
// az end

void InitGame(void)
{
	gi.dprintf("==== InitGame ====\n");


	//K03 Begin
	srand((unsigned)time(0));

	gamedir = gi.cvar("gamedir", "", CVAR_SERVERINFO);
	save_path = gi.cvar("save_path", va("%s//characters", gamedir->string), CVAR_LATCH);
	//K03 End


	// az begin
#ifndef FIXED_FT
	gi.cvar_forceset("g_features", va("%d", GMF_VARIABLE_FPS));
#endif

	savemethod = gi.cvar("savemethod", "3", 0);
	generalabmode = gi.cvar("generalabmode", "0", CVAR_LATCH);
	game_path = gi.cvar("game", ".", CVAR_LATCH);

#ifdef Q2PRO_COMPATIBILITY
	gi.cvar_forceset("sv_allow_map", "2");
#endif

	InitLuaSettings();
	CheckDir(va("%s/settings", gamedir->string));

	// Before anything else, prepare TagMalloc's mutexes and a mysql connection
	if (savemethod->value == 2)
	{
#if (!defined NO_GDS)
		V_GDS_StartConn(); // start connection to db
#else
		gi.dprintf("This server does not support MySQL. Forcing offline saving via SQLite.\n");
		gi.cvar_forceset("savemethod", "3");
#endif
	}
	else
	{
		gi.dprintf("DB: Using offline character saving (via %s)\n", savemethod->value == 1 ? "Binary" : "SQLite");
		CheckDir(save_path->string);
	}

	if (savemethod->value == 3)
	{
		V_VSFU_StartConn(); // open up sqlite single connection database
	}

#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	Mem_PrepareMutexes();
#endif

#ifdef CMD_USEHASH
	InitHash();
#endif

	InitializeAbilityList();
	// az end

	gun_x = gi.cvar("gun_x", "0", 0);
	gun_y = gi.cvar("gun_y", "0", 0);
	gun_z = gi.cvar("gun_z", "0", 0);

#ifndef FIXED_FT
	sv_fps = gi.cvar("sv_fps", "10", CVAR_LATCH);
#endif

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

	// latched vars
	// sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar("coop", "0", CVAR_LATCH);
	skill = gi.cvar("skill", "1", CVAR_SERVERINFO);
	maxentities = gi.cvar("maxentities", "1024", CVAR_LATCH);
	//chain edit flag
	//vwep support
	vwep = gi.cvar("vwep", "1", CVAR_LATCH);
	//ZOID
	//This game.dll only supports deathmatch
	/*if (!deathmatch->value)
	{
	gi.dprintf("Forcing deathmatch.\n");
	gi.cvar_set("deathmatch", "1");
	}
	//force coop off
	if (coop->value)
	gi.cvar_set("coop", "0");
	*/
	//ZOID

	// change anytime vars
	dmflags = gi.cvar("dmflags", "0", 0);
	fraglimit = gi.cvar("fraglimit", "100", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "21", CVAR_SERVERINFO);
	password = gi.cvar("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar("spectator_password", "", CVAR_USERINFO);
	maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
	g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);

	filterban = gi.cvar("filterban", "1", 0);

	run_pitch = gi.cvar("run_pitch", "0.002", 0);
	run_roll = gi.cvar("run_roll", "0.005", 0);
	bob_up = gi.cvar("bob_up", "0.005", 0);
	bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar("bob_roll", "0.002", 0);

	//K03 Begin

	killboxspawn = gi.cvar("killboxatspawn", "1", 0);
	particles = gi.cvar("particles", "0", 0);

	// az 3.0
	hw = gi.cvar("hw", "0", CVAR_LATCH);

	sentry_lev1_model = gi.cvar("sentry_lev1_model", "models/sentry/turret1/tris.md2", CVAR_LATCH);
	sentry_lev2_model = gi.cvar("sentry_lev2_model", "models/sentry/turret2/tris.md2", CVAR_LATCH);
	sentry_lev3_model = gi.cvar("sentry_lev3_model", "models/sentry/turret3/tris.md2", CVAR_LATCH);
	//teamplay = gi.cvar("teamplay","0",CVAR_SERVERINFO);
	vrx_password = gi.cvar("vrx_password", "", CVAR_ARCHIVE | CVAR_USERINFO);
	dm_monsters = gi.cvar("dm_monsters", "8", CVAR_SERVERINFO);
	// vrxchile v1.3 ally half server
	allies = gi.cvar("allies", "1", CVAR_SERVERINFO | CVAR_LATCH);
	reconnect_ip = gi.cvar("reconnect_ip", "0", CVAR_ARCHIVE);
	//locational_damage = gi.cvar("locational_damage","0",CVAR_LATCH);
	// flood control
	flood_msgs = gi.cvar("flood_msgs", "4", 0);
	flood_persecond = gi.cvar("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);
	//maplist value
	sv_maplist = gi.cvar("sv_maplist", "", CVAR_LATCH);
	min_level = gi.cvar("min_level", "0", 0);
	max_level = gi.cvar("max_level", "1000", 0);
	check_dupeip = gi.cvar("check_dupeip", "0", 0);
	check_dupename = gi.cvar("check_dupename", "1", 0);
	newbie_protection = gi.cvar("newbie_protection", "1.8", CVAR_ARCHIVE);
	debuginfo = gi.cvar("debuginfo", "0", CVAR_ARCHIVE);
	pvm = gi.cvar("pvm", "0", CVAR_LATCH);
	ffa = gi.cvar("ffa", "1", CVAR_LATCH);
	ptr = gi.cvar("ptr", "0", CVAR_LATCH);
	domination = gi.cvar("domination", "0", CVAR_LATCH);
	ctf = gi.cvar("ctf", "0", CVAR_LATCH);
	invasion = gi.cvar("invasion", "0", CVAR_LATCH);
	nolag = gi.cvar("nolag", "1", 0);

	// az v2.5 vrxchile trading mode
	trading = gi.cvar("trading", "0", 0);
	tradingmode_enabled = gi.cvar("tradingmode_enabled", "0", CVAR_LATCH);
	tbi = gi.cvar("tbi", "0", CVAR_LATCH); // enable tbi mode
	// az end
	pvm_respawntime = gi.cvar("pvm_respawntime", "10", 0);
	pvm_monstermult = gi.cvar("pvm_monstermult", "1.0", 0);
	ffa_respawntime = gi.cvar("ffa_respawntime", "20", 0);
	ffa_monstermult = gi.cvar("ffa_monstermult", "1.0", 0);
	team1_skin = gi.cvar("team1_skin", "female/ctf_r", 0);
	team2_skin = gi.cvar("team2_skin", "male/ctf_b", 0);

	// class skins
	enforce_class_skins = gi.cvar("enforce_class_skins", "1", CVAR_LATCH);

	class1_model = gi.cvar("class1_model", "terran", CVAR_LATCH); // soldier
	class2_model = gi.cvar("class2_model", "xenoid", CVAR_LATCH); // polt
	class3_model = gi.cvar("class3_model", "caleb", CVAR_LATCH); // vampire
	class4_model = gi.cvar("class4_model", "linfang", CVAR_LATCH); // mage
	class5_model = gi.cvar("class5_model", "terminator", CVAR_LATCH); // engy
	class6_model = gi.cvar("class6_model", "pknight", CVAR_LATCH); // knight
	class7_model = gi.cvar("class7_model", "alita", CVAR_LATCH); // cleric

	if (generalabmode->value)
		class8_model = gi.cvar("class8_model", "ratamahatta", CVAR_LATCH); // weapon master
	else
		class8_model = gi.cvar("class8_model", "marine", CVAR_LATCH); // apprentice

	class9_model = gi.cvar("class9_model", "zumlin", CVAR_LATCH); // necromancer
	class10_model = gi.cvar("class10_model", "bauul", CVAR_LATCH); // shaman
	class11_model = gi.cvar("class11_model", "xenoid", CVAR_LATCH); // alien
	class12_model = gi.cvar("class12_model", "ddz", CVAR_LATCH); // kamikaze



	class1_skin = gi.cvar("class1_skin", "blue", CVAR_LATCH); // soldier
	class2_skin = gi.cvar("class2_skin", "xenotype1", CVAR_LATCH); // polt
	class3_skin = gi.cvar("class3_skin", "caleb_undead", CVAR_LATCH); // vampire
	class4_skin = gi.cvar("class4_skin", "ctf_r", CVAR_LATCH); // mage
	class5_skin = gi.cvar("class5_skin", "blood", CVAR_LATCH); // engy
	class6_skin = gi.cvar("class6_skin", "knight", CVAR_LATCH); // knight
	class7_skin = gi.cvar("class7_skin", "alita", CVAR_LATCH); // cleric
	if (generalabmode->value)
		class8_skin = gi.cvar("class8_skin", "ratamahatta", CVAR_LATCH); // weapon master
	else
		class8_skin = gi.cvar("class8_skin", "brownie", CVAR_LATCH); // apprentice
	class9_skin = gi.cvar("class9_skin", "zumlin", CVAR_LATCH); // necromancer
	class10_skin = gi.cvar("class10_skin", "bauul", CVAR_LATCH); // shaman
	class11_skin = gi.cvar("class11_skin", "xenotype1", CVAR_LATCH); // alien
	class12_skin = gi.cvar("class12_skin", "ctf_g", CVAR_LATCH); // kamikaze

	//Municion MUNICION munición ammo AMMO Ammo

	// minimum map ammo
	world_min_shells = gi.cvar("world_min_shells", "1", 0);
	world_min_bullets = gi.cvar("world_min_bullets", "1", 0);
	world_min_rockets = gi.cvar("world_min_rockets", "1", 0);
	world_min_grenades = gi.cvar("world_min_grenades", "1", 0);
	world_min_cells = gi.cvar("world_min_cells", "1", 0);
	world_min_slugs = gi.cvar("world_min_slugs", "1", 0);

	// enable special rules for flag carrier in CTF mode
	ctf_enable_balanced_fc = gi.cvar("ctf_enable_balanced_fc", "1", CVAR_LATCH);
	adminctrl = gi.cvar("admin_forcevote", "0", CVAR_LATCH);

	voting = gi.cvar("voting", "1", CVAR_SERVERINFO);
	pregame_time = gi.cvar("pregame_time", "60.0", 0);
#ifndef LOCK_DEFAULTS
	nextlevel_mult = gi.cvar("nextlevel_mult", "1.5", CVAR_LATCH);
	if (nextlevel_mult->value < 1.5)
		nextlevel_mult->value = 1.5;

	start_level = gi.cvar("start_level", "0", CVAR_LATCH);
	start_nextlevel = gi.cvar("start_nextlevel", "1000", CVAR_LATCH);

	invasion_enabled = gi.cvar("invasion_enabled", "1", CVAR_LATCH);
	vrx_pointmult = gi.cvar("vrx_pointmult", "1.0", CVAR_SERVERINFO/* | CVAR_LATCH*/);
	vrx_pvppointmult = gi.cvar("vrx_pvppointmult", "1.2", 0); // 1.5 is TOO much.
	vrx_pvmpointmult = gi.cvar("vrx_pvmpointmult", "1.0", 0);
	vrx_sub10mult = gi.cvar("vrx_sub10mult", "0.9", 0);
	vrx_over10mult = gi.cvar("vrx_over10mult", "0.7", 0);

	vrx_creditmult = gi.cvar("vrx_creditmult", "1.5", 0/* | CVAR_LATCH*/);
	vrx_pvpcreditmult = gi.cvar("vrx_pvpcreditmult", "2.0", 0);
	vrx_pvmcreditmult = gi.cvar("vrx_pvmcreditmult", "1.0", 0);
	adminpass = gi.cvar("adminpass", "", CVAR_ARCHIVE);
#else
	gi.cvar("timescale", "1", CVAR_NOSET);
	gi.cvar_forceset("timescale", "1");

	nextlevel_mult = gi.cvar("nextlevel_mult", "1.5", CVAR_NOSET);
	gi.cvar_forceset("nextlevel_mult", "1.5");

	start_level = gi.cvar("start_level", "0", CVAR_SERVERINFO | CVAR_NOSET);
	start_nextlevel = gi.cvar("start_nextlevel", "1000", CVAR_NOSET);
	gi.cvar_forceset("start_level", "0");
	gi.cvar_forceset("start_nextlevel", "1000");

	vrx_pointmult = gi.cvar("vrx_pointmult", "1.5", CVAR_NOSET);
	vrx_pvppointmult = gi.cvar("vrx_pvppointmult", "1.5", CVAR_SERVERINFO | CVAR_NOSET);
	vrx_pvmpointmult = gi.cvar("vrx_pvmpointmult", "1.0", CVAR_SERVERINFO | CVAR_NOSET);
	vrx_creditmult = gi.cvar("vrx_creditmult", "2.0", CVAR_NOSET);
	vrx_pvpcreditmult = gi.cvar("vrx_pvpcreditmult", "2.0", CVAR_SERVERINFO | CVAR_NOSET);
	vrx_pvmcreditmult = gi.cvar("vrx_pvmcreditmult", "2.0", CVAR_SERVERINFO | CVAR_NOSET);
	gi.cvar_forceset("vrx_pointmult", "1.5");
	gi.cvar_forceset("vrx_creditmult", "2.0");
	vrx_sub10mult = gi.cvar("vrx_sub10mult", "1.5", CVAR_NOSET);
	vrx_over10mult = gi.cvar("vrx_over10mult", "0.65", CVAR_NOSET);

	vrx_creditmult = gi.cvar("vrx_creditmult", "2.0", CVAR_NOSET/* | CVAR_LATCH*/);
	vrx_pvpcreditmult = gi.cvar("vrx_pvpcreditmult", "3.0", CVAR_NOSET);
	vrx_pvmcreditmult = gi.cvar("vrx_pvmcreditmult", "1.0", CVAR_NOSET);

	adminpass = gi.cvar("adminpass", "", CVAR_NOSET);
	gi.cvar_forceset("adminpass", "");
#endif
	//K03 End

	// items
	InitItems();

	Com_sprintf(game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf(game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts = V_Malloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = V_Malloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;

	//3.0 Load the custom map lists
	if (v_LoadMapList(MAPMODE_PVP) && v_LoadMapList(MAPMODE_PVM) && v_LoadMapList(MAPMODE_INV)
		&& v_LoadMapList(MAPMODE_DOM) && v_LoadMapList(MAPMODE_CTF) && v_LoadMapList(MAPMODE_FFA)
		&& v_LoadMapList(MAPMODE_TRA) && v_LoadMapList(MAPMODE_INH) && v_LoadMapList(MAPMODE_VHW)
		&& v_LoadMapList(MAPMODE_TBI))
		gi.dprintf("INFO: Vortex Custom Map Lists loaded successfully\n");
	else
		gi.dprintf("WARNING: Error loading custom map lists\n");

	//3.0 Load the armory
	LoadArmory();

	//K03 Begin
	config_map_list();

	MonstersInUse = false; // Set this value here..
	InitializeTeamNumbers(); // for allies

	//K03 End

	// az begin
	AI_Init();
	// az end
}

//=========================================================

void WriteField1(FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if (*(char **)p)
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if (*(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if (*(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if (*(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	default:
		gi.error("WriteEdict: unknown field type");
	}
}

void WriteField2(FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
	case F_GSTRING:
		if (*(char **)p)
		{
			len = strlen(*(char **)p) + 1;
			fwrite(*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField(FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = V_Malloc(len, TAG_LEVEL);
			fread(*(char **)p, len, 1, f);
		}
		break;
	case F_GSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = V_Malloc(len, TAG_GAME);
			fread(*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if (index == -1)
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if (index == -1)
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if (index == -1)
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	default:
		gi.error("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient(FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field = clientfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = clientfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient(FILE *f, gclient_t *client)
{
	field_t		*field;

	fread(client, sizeof(*client), 1, f);

	for (field = clientfields; field->name; field++)
	{
		ReadField(f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame(char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData();

	f = fopen(filename, "wb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	memset(str, 0, sizeof(str));
	strcpy(str, __DATE__);
	fwrite(str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i = 0; i<game.maxclients; i++)
		WriteClient(f, &game.clients[i]);

	fclose(f);
}

void ReadGame(char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags(TAG_GAME);

	f = fopen(filename, "rb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	fread(str, sizeof(str), 1, f);
	if (strcmp(str, __DATE__))
	{
		fclose(f);
		gi.error("Savegame from an older version.\n");
	}

	g_edicts = V_Malloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread(&game, sizeof(game), 1, f);
	game.clients = V_Malloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i = 0; i<game.maxclients; i++)
		ReadClient(f, &game.clients[i]);

	fclose(f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict(FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field = savefields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = savefields; field->name; field++)
	{
		WriteField2(f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals(FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field = levelfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = levelfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict(FILE *f, edict_t *ent)
{
	field_t		*field;

	fread(ent, sizeof(*ent), 1, f);

	for (field = savefields; field->name; field++)
	{
		ReadField(f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals(FILE *f)
{
	field_t		*field;

	fread(&level, sizeof(level), 1, f);

	for (field = levelfields; field->name; field++)
	{
		ReadField(f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel(char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	f = fopen(filename, "wb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite(&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite(&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals(f);

	// write out all the entities
	for (i = 0; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite(&i, sizeof(i), 1, f);
		WriteEdict(f, ent);
	}
	i = -1;
	fwrite(&i, sizeof(i), 1, f);

	fclose(f);
}


/*
=================
ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel(char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	f = fopen(filename, "rb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags(TAG_LEVEL);

	// wipe all the entities
	memset(g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value + 1;

	// check edict size
	fread(&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread(&base, sizeof(base), 1, f);
	if (base != (void *)InitGame)
	{
		fclose(f);
		gi.error("ReadLevel: function pointers have moved");
	}

	// load the level locals
	ReadLevelLocals(f);

	// load all the entities
	while (1)
	{
		if (fread(&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose(f);
			gi.error("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum + 1;

		ent = &g_edicts[entnum];
		ReadEdict(f, ent);

		// let the server rebuild world links for this ent
		memset(&ent->area, 0, sizeof(ent->area));
		gi.linkentity(ent);
	}

	fclose(f);

	// mark all clients as unconnected
	for (i = 0; i<maxclients->value; i++)
	{
		ent = &g_edicts[i + 1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i = 0; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}
