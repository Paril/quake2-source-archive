
#include "g_local.h"
// StroggDM Logging Functions
#include <time.h>
struct tm *localtime( const time_t *time );
// Loading
//#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
//#include <winbase.h>
//#include <stdio.h>
//#include <string>
//#include <fstream>
// StroggDM Logging Functions

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

	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
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
	// ROGUE
	{"bad_area", FOFS(bad_area), F_EDICT},
	// while the hint_path stuff could be reassembled on the fly, no reason to be different
	{"hint_chain", FOFS(hint_chain), F_EDICT},
	{"monster_hint_chain", FOFS(monster_hint_chain), F_EDICT},
	{"target_hint_chain", FOFS(target_hint_chain), F_EDICT},
	//
	{"goal_hint", FOFS(monsterinfo.goal_hint), F_EDICT},
	{"badMedic1", FOFS(monsterinfo.badMedic1), F_EDICT},
	{"badMedic2", FOFS(monsterinfo.badMedic2), F_EDICT},
	{"last_player_enemy", FOFS(monsterinfo.last_player_enemy), F_EDICT},
	{"commander", FOFS(monsterinfo.commander), F_EDICT},
	{"blocked", FOFS(monsterinfo.blocked), F_MMOVE, FFL_NOSPAWN},
	{"duck", FOFS(monsterinfo.duck), F_MMOVE, FFL_NOSPAWN},
	{"unduck", FOFS(monsterinfo.unduck), F_MMOVE, FFL_NOSPAWN},
	{"sidestep", FOFS(monsterinfo.sidestep), F_MMOVE, FFL_NOSPAWN},
	// ROGUE
	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},

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

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), F_LSTRING},
                   
	{"sight_client", LLOFS(sight_client), F_EDICT},
	{"sight_entity", LLOFS(sight_entity), F_EDICT},
	{"sound_entity", LLOFS(sound_entity), F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

int GetLineFromFile(FILE *in, char s[]);
int SearchFile(FILE *in, char s[]);

// StroggDM Logging
void InitLogging (void)
{
    char tmpbuf[20];
	char buf2[80];


	_strtime(tmpbuf);
	_strdate(buf2);

	if (!logging->value)
		return;
	if ((logged = fopen("stroggdm/logged.txt", "a")) == NULL)
		gi.dprintf ("StroggDM Error: Cannot open current logging file, creating one..");

	gi.dprintf ("StroggDM Logging Initiated\n");
	fprintf (logged, "Logging started at %s, on %s.\n", tmpbuf, buf2);
}
// StroggDM Logging


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
	gi.dprintf ("==== InitGame ====\n");

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
	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// Paril
	// Teamplay Code
	teamplay_classes = gi.cvar ("teamplay_classes", "0", CVAR_SERVERINFO|CVAR_LATCH);
	teamplay_landtype = gi.cvar ("teamplay_landtype", "0", CVAR_SERVERINFO|CVAR_LATCH);
	// Nohook 
	nohook = gi.cvar ("nohook", "0", CVAR_SERVERINFO|CVAR_LATCH);
	// No spawning monsters
	no_spawning_monsters = gi.cvar ("no_spawning_monsters", "0", CVAR_SERVERINFO|CVAR_LATCH);
	//Falling Damage
	fallingdamage = gi.cvar ("fallingdamage", "0", CVAR_SERVERINFO|CVAR_LATCH);
	logging = gi.cvar ("logging", "0", CVAR_SERVERINFO|CVAR_LATCH);
	// Monster bans
	// Whew..
	ban_soldier = gi.cvar ("ban_soldier", "0", CVAR_LATCH);
	ban_enforcer = gi.cvar ("ban_enforcer", "0", CVAR_LATCH);
	ban_tank = gi.cvar ("ban_tank", "0", CVAR_LATCH);
	ban_ctank = gi.cvar ("ban_ctank", "0", CVAR_LATCH);
	ban_medic = gi.cvar ("ban_medic", "0", CVAR_LATCH);
	ban_mediccommander = gi.cvar ("ban_mediccommander", "0", CVAR_LATCH);
	ban_gunner = gi.cvar ("ban_gunner", "0", CVAR_LATCH);
	ban_gladiator = gi.cvar ("ban_gladiator", "0", CVAR_LATCH);
	ban_supertank = gi.cvar ("ban_supertank", "0", CVAR_LATCH);
	ban_flyer = gi.cvar ("ban_flyer", "0", CVAR_LATCH);
	ban_shark = gi.cvar ("ban_shark", "0", CVAR_LATCH);
	ban_floater = gi.cvar ("ban_floater", "0", CVAR_LATCH);
	ban_berserk = gi.cvar ("ban_berserk", "0", CVAR_LATCH);
	ban_ironmaiden = gi.cvar ("ban_ironmaiden", "0", CVAR_LATCH);
	ban_boss2 = gi.cvar ("ban_boss2", "0", CVAR_LATCH);
	ban_carrier = gi.cvar ("ban_carrier", "0", CVAR_LATCH);
	ban_widow1 = gi.cvar ("ban_widow1", "0", CVAR_LATCH);
	ban_widow2 = gi.cvar ("ban_widow2", "0", CVAR_LATCH);
	ban_brain = gi.cvar ("ban_brain", "0", CVAR_LATCH);
	ban_mutant = gi.cvar ("ban_mutant", "0", CVAR_LATCH);
	ban_parasite = gi.cvar ("ban_parasite", "0", CVAR_LATCH);
	ban_jorg = gi.cvar ("ban_jorg", "0", CVAR_LATCH);
	ban_makron = gi.cvar ("ban_makron", "0", CVAR_LATCH);
	ban_icarus = gi.cvar ("ban_icarus", "0", CVAR_LATCH);
	ban_betasoldier = gi.cvar ("ban_betasoldier", "0", CVAR_LATCH);
	ban_betabrain = gi.cvar ("ban_betabrain", "0", CVAR_LATCH);
	ban_betaironmaiden = gi.cvar ("ban_betaironmaiden", "0", CVAR_LATCH);
	ban_betasupertank = gi.cvar ("ban_betasupertank", "0", CVAR_LATCH);
	ban_betagladiator = gi.cvar ("ban_betagladiator", "0", CVAR_LATCH);
	ban_gekk = gi.cvar ("ban_gekk", "0", CVAR_LATCH);
	ban_daedalus = gi.cvar ("ban_daedalus", "0", CVAR_LATCH);
	ban_stalker = gi.cvar ("ban_stalker", "0", CVAR_LATCH);
	ban_fixbot = gi.cvar ("ban_fixbot", "0", CVAR_LATCH);

	ban_stormt = gi.cvar ("ban_stormt", "0", CVAR_LATCH);
	ban_deathtank = gi.cvar ("ban_deathtank", "0", CVAR_LATCH);
	ban_lasergladiator = gi.cvar ("ban_lasergladiator", "0", CVAR_LATCH);
	ban_flyerhornet = gi.cvar ("ban_flyerhornet", "0", CVAR_LATCH);
	ban_traitorm = gi.cvar ("ban_traitorm", "0", CVAR_LATCH);
	ban_traitorf = gi.cvar ("ban_traitorf", "0", CVAR_LATCH);
	ban_hypertank = gi.cvar ("ban_hypertank", "0", CVAR_LATCH);
	// Monster healths
	// Oh my god. The things people want :(
	soldier_health = gi.cvar ("soldier_health", "70", CVAR_LATCH);
	enforcer_health = gi.cvar ("enforcer_health", "100", CVAR_LATCH);
	tank_health = gi.cvar ("tank_health", "650", CVAR_LATCH);
	ctank_health = gi.cvar ("ctank_health", "800", CVAR_LATCH);
	medic_health = gi.cvar ("medic_health", "300", CVAR_LATCH);
	mediccommander_health = gi.cvar ("mediccommander_health", "265", CVAR_LATCH);
	gunner_health = gi.cvar ("gunner_health", "175", CVAR_LATCH);
	gladiator_health = gi.cvar ("gladiator_health", "400", CVAR_LATCH);
	supertank_health = gi.cvar ("supertank_health", "900", CVAR_LATCH);
	flyer_health = gi.cvar ("flyer_health", "100", CVAR_LATCH);
	shark_health = gi.cvar ("shark_health", "175", CVAR_LATCH);
	floater_health = gi.cvar ("floater_health", "200", CVAR_LATCH);
	berserk_health = gi.cvar ("berserk_health", "240", CVAR_LATCH);
	ironmaiden_health = gi.cvar ("ironmaiden_health", "175", CVAR_LATCH);
	boss2_health = gi.cvar ("boss2_health", "1250", CVAR_LATCH);
	carrier_health = gi.cvar ("carrier_health", "900", CVAR_LATCH);
	widow1_health = gi.cvar ("widow1_health", "2000", CVAR_LATCH);
	widow2_health = gi.cvar ("widow2_health", "2200", CVAR_LATCH);
	brain_health = gi.cvar ("brain_health", "550", CVAR_LATCH);
	mutant_health = gi.cvar ("mutant_health", "800", CVAR_LATCH);
	parasite_health = gi.cvar ("parasite_health", "175", CVAR_LATCH);
	jorg_health = gi.cvar ("jorg_health", "2000", CVAR_LATCH);
	makron_health = gi.cvar ("makron_health", "1700", CVAR_LATCH);
	icarus_health = gi.cvar ("icarus_health", "85", CVAR_LATCH);
	betasoldier_health = gi.cvar ("betasoldier_health", "55", CVAR_LATCH);
	betabrain_health = gi.cvar ("betabrain_health", "400", CVAR_LATCH);
	betaironmaiden_health = gi.cvar ("betaironmaiden_health", "125", CVAR_LATCH);
	betasupertank_health = gi.cvar ("betasupertank_health", "765", CVAR_LATCH);
	betagladiator_health = gi.cvar ("betagladiator_health", "400", CVAR_LATCH);
	gekk_health = gi.cvar ("gekk_health", "375", CVAR_LATCH);
	daedalus_health = gi.cvar ("daedalus_health", "100", CVAR_LATCH);
	stalker_health = gi.cvar ("stalker_health", "235", CVAR_LATCH);
	fixbot_health = gi.cvar ("fixbot_health", "100", CVAR_LATCH);

	stormt_health = gi.cvar ("stormt_health", "185", CVAR_LATCH);
	deathtank_health = gi.cvar ("deathtank_health", "1000", CVAR_LATCH);
	lasergladiator_health = gi.cvar ("lasergladiator_health", "540", CVAR_LATCH);
	flyerhornet_health = gi.cvar ("flyerhornet_health", "200", CVAR_LATCH);
	traitorm_health = gi.cvar ("traitorm_health", "250", CVAR_LATCH);
	traitorf_health = gi.cvar ("traitorf_health", "250", CVAR_LATCH);
	hypertank_health = gi.cvar ("hypertank_health", "2400", CVAR_LATCH);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	stroggflags = gi.cvar ("stroggflags", "0", CVAR_SERVERINFO);

//ZOID
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
	instantweap = gi.cvar ("instantweap", "0", CVAR_SERVERINFO);
	ctf = gi.cvar("ctf", "0", CVAR_SERVERINFO | CVAR_LATCH);
//ZOID

//ZOID
//This game.dll only supports deathmatch
// Not THIS gamex86.dll - Paril
	//if (!deathmatch->value) {
		//gi.dprintf("Forcing deathmatch.");
		//gi.cvar_set("deathmatch", "1");
	//}
	//force coop off
	//if (coop->value)
		//gi.cvar_set("coop", "0");
//ZOID

	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

	// StroggDM Logging
	InitLogging ();
	// StroggDM Logging

	//if (!LoadLibrary("stroggdm\\gamex86.dll"))
		//gi.dprintf ("Couldn't be loaded.\n");
//ZOID
	CTFInit();
//ZOID

	//gi.dprintf ("Setup_Random_Monsters() start!\n");

	//Setup_Random_Monsters ();
}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

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
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	//relative to code segment
	case F_FUNCTION:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - ((byte *)InitGame);
		*(int *)p = index;
		break;

	//relative to data segment
	case F_MMOVE:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - (byte *)&mmove_reloc;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}


void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
		if ( *(char **)p )
		{
			len = strlen(*(char **)p) + 1;
			fwrite (*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

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
			*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	//relative to code segment
	case F_FUNCTION:
		index = *(int *)p;
		if ( index == 0 )
			*(byte **)p = NULL;
		else
			*(byte **)p = ((byte *)InitGame) + index;
		break;

	//relative to data segment
	case F_MMOVE:
		index = *(int *)p;
		if (index == 0)
			*(byte **)p = NULL;
		else
			*(byte **)p = (byte *)&mmove_reloc + index;
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
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
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags (TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;

	fread (ent, sizeof(*ent), 1, f);

	for (field=fields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread (&base, sizeof(base), 1, f);
#ifdef _WIN32
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte *)base) - ((byte *)InitGame));
#endif

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
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
