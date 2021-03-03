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

#define Function(f) {#f, f}

mmove_t mmove_reloc;

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},

/*	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},

	{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
	{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
	{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},*/

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},

//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), F_ITEM},

	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

	{0, 0, 0, 0}

};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame (void)
{
	cvar_t	*g_features;
	cvar_t	*sv_features;

	gi.dprintf ("==== InitGame ====\n");

	init_genrand ((unsigned long)time(NULL));

	gi.cvar("time_remaining", "N/A", CVAR_SERVERINFO | CVAR_NOSET);
	gi.cvar("match_type", "N/A", CVAR_SERVERINFO | CVAR_NOSET);
	gi.cvar("Score_B", "N/A", CVAR_SERVERINFO | CVAR_NOSET);
	gi.cvar("Score_A", "N/A", CVAR_SERVERINFO | CVAR_NOSET);

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	//maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	//deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	//coop = gi.cvar ("coop", "0", CVAR_LATCH);
	//skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "8", 0);
	flood_persecond = gi.cvar ("flood_persecond", "8", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	// r1: opentdm cvars
	// flood wave control
	flood_waves = gi.cvar ("flood_waves", "6", 0);
	flood_waves_perminute = gi.cvar ("flood_waves_perminute", "6", 0);
	flood_waves_waitdelay = gi.cvar ("flood_waves_waitdelay", "10", 0);

	g_gamemode = gi.cvar ("g_gamemode", "0", CVAR_LATCH);
	g_gamemode->modified = false;

	g_team_a_name = gi.cvar ("g_team_a_name", "Hometeam", 0);
	g_team_b_name = gi.cvar ("g_team_b_name", "Visitors", 0);
	g_locked_names = gi.cvar ("g_locked_names", "0", 0);

	g_team_a_skin = gi.cvar ("g_team_a_skin", "male/grunt", 0);
	g_team_b_skin = gi.cvar ("g_team_b_skin", "female/athena", 0);
	g_locked_skins = gi.cvar ("g_locked_skins", "0", 0);

	g_admin_password = gi.cvar ("g_admin_password", "", 0);
	g_admin_vote_decide = gi.cvar ("g_admin_vote_decide", "1", 0);
	g_match_time = gi.cvar ("g_match_time", "600", 0);
	g_match_countdown = gi.cvar ("g_match_countdown", "15", 0);
	g_vote_time = gi.cvar ("g_vote_time", "30", 0);
	g_vote_mask = gi.cvar ("g_vote_mask", "-1", 0);
	g_intermission_time = gi.cvar ("g_intermission_time", "5", 0);
	g_force_screenshot = gi.cvar ("g_force_screenshot", "0", 0);
	g_force_record = gi.cvar ("g_force_record", "0", 0);

	g_tdmflags = gi.cvar ("g_tdmflags", "1040", 0);
	g_itdmflags = gi.cvar ("g_itdmflags", "142427", 0);
	g_1v1flags = gi.cvar ("g_1v1flags", "1040", 0);

	g_itemflags = gi.cvar ("g_itemflags", "0", 0);
	g_powerupflags = gi.cvar ("g_powerupflags", "0", 0);

	g_tdm_allow_pick = gi.cvar ("g_tdm_allow_pick", "0", 0);

	g_fast_weap_switch = gi.cvar ("g_fast_weap_switch", "0", 0);
	g_teleporter_nofreeze = gi.cvar ("g_teleporter_nofreeze", "0", 0);

	g_tie_mode = gi.cvar ("g_tie_mode", "1", 0);
	// wision: 60 as default overtime for all leagues
	g_overtime = gi.cvar ("g_overtime", "60", 0);
	// wision: default from other mods (battle)..
	// low values are making raping easy on small maps in duels
	// shouldn't even be configurable probably
	g_respawn_time = gi.cvar ("g_respawn_time", "5", 0);

	//max timeout when called via cmd
	g_max_timeout = gi.cvar ("g_max_timeout", "300", 0);

	//max timeout via implicit timeout in 1v1 disconnect
	g_1v1_timeout = gi.cvar ("g_1v1_timeout", "90", 0);

	//allow all chat or only players?
	g_chat_mode = gi.cvar ("g_chat_mode", "0", 0);

	g_idle_time = gi.cvar ("g_idle_time", "300", 0);

	g_http_enabled = gi.cvar ("g_http_enabled", "1", 0);
	g_http_bind = gi.cvar ("g_http_bind", "", 0);
	g_http_proxy = gi.cvar ("g_http_proxy", "", 0);

	g_debug_spawns = gi.cvar ("g_debug_spawns", "0", 0);

	g_max_players_per_team = gi.cvar ("g_max_players_per_team", "4", 0);

	g_maplistfile = gi.cvar ("g_maplistfile", "", 0);
	//g_motd_message = gi.cvar ("g_motd_message", "==========================\\n|    Quake 2 OpenTDM!    |\\n|------------------------|\\n|   http://opentdm.net/  |\\n==========================" , 0);
	//default off since its super annoying
	g_motd_message = gi.cvar ("g_motd_message", "" , 0);

	g_bugs = gi.cvar ("g_bugs", "0", 0);
	g_allow_name_change_during_match = gi.cvar ("g_allow_name_change_during_match", "1", 0);

	g_allow_vote_config = gi.cvar ("g_allow_vote_config", "1", 0);

	g_command_mask = gi.cvar ("g_command_mask", "-1", 0);

	g_auto_rejoin_match = gi.cvar ("g_auto_rejoin_match", "1", 0);
	g_auto_rejoin_map = gi.cvar ("g_auto_rejoin_map", "1", 0);

	g_1v1_spawn_mode = gi.cvar ("g_1v1_spawn_mode", "1", 0);
	g_tdm_spawn_mode = gi.cvar ("g_tdm_spawn_mode", "1", 0);
	
	g_http_path = gi.cvar ("g_http_path", "/api/", CVAR_NOSET);
	g_http_domain = gi.cvar ("g_http_domain", "opentdm.net", CVAR_NOSET);

	// items
	InitItems ();	

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = (int)maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

	//game/server feature suppotr - export what we support and read what the server supports

	//ensure it has NOSET if it didn't exist
	g_features = gi.cvar ("g_features", "0", CVAR_NOSET);
	gi.cvar_forceset ("g_features", va("%d", GMF_CLIENTNUM | GMF_WANT_ALL_DISCONNECTS | GMF_PROPERINUSE | GMF_MVDSPEC));

	//init server features
	sv_features = gi.cvar ("sv_features", NULL, 0);
	if (sv_features)
		game.server_features = (int)sv_features->value;
	else
		game.server_features = 0;

	TDM_Init ();
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
void DummyWrite (const char *filename, qboolean autosave)
{
}

void DummyRead (const char *filename)
{
}

//==========================================================

