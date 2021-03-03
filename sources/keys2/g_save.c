
#include "g_local.h"
#include "g_map_mod.h"


#ifdef AMIGA
#include <exec/types.h>
#include <dos/dos.h>

BOOL FindAddressInSegList(BPTR Seglist, void *Address, ULONG* Offset, UBYTE *Segment)
{
    UBYTE SegNum = 0;
    ULONG *SegmentBase;
    ULONG SegmentLength;

    do
    {
	SegmentBase = (ULONG *)((ULONG)Seglist*4);
	Seglist = (BPTR)(*SegmentBase);
	SegmentLength = *(SegmentBase-1);

	if ((void*)SegmentBase < Address && Address < (void *)(SegmentBase+SegmentLength))
	{
	    // Found
	    *Offset = (ULONG)Address-(ULONG)SegmentBase;
	    *Segment = (UBYTE)SegNum;
	    return TRUE;
	}

	SegNum++;
    } while (Seglist);

    return FALSE;
}

BOOL ExpandAddressInSegList(BPTR Seglist, void **Address, ULONG Offset, UBYTE Segment)
{
    ULONG *SegmentBase;
    ULONG SegmentLength;

    Segment++;

    while (Segment && Seglist)
    {
	SegmentBase = (ULONG *)((ULONG)Seglist*4);
	Seglist = (BPTR)(*SegmentBase);
	SegmentLength = *(SegmentBase-1);
	Segment--;
    }

    if (Segment) return FALSE; // Ran out of segments

    *Address = (void *)(((char *)SegmentBase) + Offset);
    return TRUE;
}

#endif


//K2:begin
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
//K2:End

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

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

/*field_t		savefields[] =
{
	{"", FOFS(classname), F_LSTRING},
	{"", FOFS(target), F_LSTRING},
	{"", FOFS(targetname), F_LSTRING},
	{"", FOFS(killtarget), F_LSTRING},
	{"", FOFS(team), F_LSTRING},
	{"", FOFS(pathtarget), F_LSTRING},
	{"", FOFS(deathtarget), F_LSTRING},
	{"", FOFS(combattarget), F_LSTRING},
	{"", FOFS(model), F_LSTRING},
	{"", FOFS(map), F_LSTRING},
	{"", FOFS(message), F_LSTRING},

	{"", FOFS(client), F_CLIENT},
	{"", FOFS(item), F_ITEM},

	{"", FOFS(goalentity), F_EDICT},
	{"", FOFS(movetarget), F_EDICT},
	{"", FOFS(enemy), F_EDICT},
	{"", FOFS(oldenemy), F_EDICT},
	{"", FOFS(activator), F_EDICT},
	{"", FOFS(groundentity), F_EDICT},
	{"", FOFS(teamchain), F_EDICT},
	{"", FOFS(teammaster), F_EDICT},
	{"", FOFS(owner), F_EDICT},
	{"", FOFS(mynoise), F_EDICT},
	{"", FOFS(mynoise2), F_EDICT},
	{"", FOFS(target_ent), F_EDICT},
	{"", FOFS(chain), F_EDICT},

	{NULL, 0, F_INT}
};
*/

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

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is begun
============
*/
void InitGame (void)
{
	//K2:Begin
	FILE	*lockfile;
	char	lockfilename[50]="";
	cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
	cvar_t *stdlogfile = gi.cvar("stdlogfile","0",CVAR_LATCH);

	gi.dprintf ("\n%s by Rich Shetina\nrshetina@planetquake.com\nhttp://www.planetquake.com/keys2\n",GAMEVERSION);
	gi.dprintf ("\nEraser Bot mod by Ryan 'Ridah' Feltrin\n<http://impact.frag.com>\n\n"); 
	if(stdlogfile->value)
		K2_FixGSLogFile();
	//K2:End
	
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

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	//K2:Begin
	//This game.dll only supports deathmatch
	
	if (!deathmatch->value) {
		gi.dprintf("Forcing deathmatch.");
		gi.cvar_set("deathmatch", "1");
	}
	//force coop off
	if (coop->value)
		gi.cvar_set("coop", "0");
	//K2:End
		
//ZOID
	ctf = gi.cvar("ctf","0",CVAR_SERVERINFO|CVAR_LATCH);
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
//ZOID
	
	//K2:Begin - Keys2 server cvars
	protecttime = gi.cvar("protecttime","10.0",0);
	gibtime		= gi.cvar("gibtime","30.0",0);
	burntime	= gi.cvar("burntime","20.0",0);
	blindtime	= gi.cvar("blindtime","5",0);  
	freezetime	= gi.cvar("freezetime","5",0);  
	flash_radius = gi.cvar("flash_radius","512",0);
	freeze_radius = gi.cvar("freeze_radius","512",0);
	pickuptime = gi.cvar("pickuptime","20.0",0);
	gibdamage	= gi.cvar("gibdamage","8",0);
	burndamage	= gi.cvar("burndamage","8",0);
	motd1		= gi.cvar("motd1","",0);
	motd2		= gi.cvar("motd2","",0);
	motd3		= gi.cvar("motd3","",0);
	motd4		= gi.cvar("motd4","",0);
	motd5		= gi.cvar("motd5","",0);
	motd6		= gi.cvar("motd6","",0);
	motd7		= gi.cvar("motd7","",0);
	nextleveldelay = gi.cvar("nextleveldelay","10",0);
	nextlevelstart = 9999999;

	regentime	= gi.cvar("regentime","60",0);
	hastetime	= gi.cvar("hastetime","60",0);
	futilitytime	= gi.cvar("futilitytime","60",0);
	inflictiontime	= gi.cvar("inflictiontime","60",0);
	bfktime		= gi.cvar("bfktime","30",0);
	stealthtime	= gi.cvar("stealthtime","60",0);
	homingtime	= gi.cvar("homingtime","30",0);
	antitime	= gi.cvar("antitime","60",0);

	nobfg = gi.cvar("nobfg","0",CVAR_LATCH);
	noshotgun = gi.cvar("noshotgun","0",CVAR_LATCH);
	nosupershotgun = gi.cvar("nosupershotgun","0",CVAR_LATCH);
	nomachinegun = gi.cvar("nomachinegun","0",CVAR_LATCH);
	nochaingun = gi.cvar("nochaingun","0",CVAR_LATCH);
	nogrenadelauncher = gi.cvar("nogrenadelauncher","0",CVAR_LATCH);
	norocketlauncher = gi.cvar("norocketlauncher","0",CVAR_LATCH);
	nohyperblaster = gi.cvar("nohyperblaster","0",CVAR_LATCH);
	norailgun = gi.cvar("norailgun","0",CVAR_LATCH);
	nomegahealth = gi.cvar("nomegahealth","0",CVAR_LATCH);
	noquad = gi.cvar("noquad","0",CVAR_LATCH);
	noinvulnerability = gi.cvar("noinvulnerability","0",CVAR_LATCH);
	
	swaat = gi.cvar("swaat","0",CVAR_LATCH);
	
	giveshotgun = gi.cvar("giveshotgun","0",0);
	givesupershotgun = gi.cvar("givesupershotgun","0",0);
	givemachinegun = gi.cvar("givemachinegun","0",0);
	givechaingun = gi.cvar("givechaingun","0",0);
	givegrenadelauncher = gi.cvar("givegrenadelauncher","0",0);
	giverocketlauncher = gi.cvar("giverocketlauncher","0",0);
	giverailgun = gi.cvar("giverailgun","0",0);
	givehyperblaster = gi.cvar("givehyperblaster","0",0);
	givebfg = gi.cvar("givebfg","0",0);

	startinghealth  = gi.cvar("startinghealth","100",0);
	startingshells  = gi.cvar("startingshells","0",0);
	startingbullets = gi.cvar("startingbullets","0",0);
	startinggrenades = gi.cvar("startinggrenades","0",0);
	startingrockets= gi.cvar("startingrockets","0",0);
	startingslugs = gi.cvar("startingslugs","0",0);
	startingcells= gi.cvar("startingcells","0",0);
	maxhealth = gi.cvar("maxhealth","100",0);
	maxshells = gi.cvar("maxshells","100",0);
	maxbullets = gi.cvar("maxbullets","200",0);
	maxgrenades = gi.cvar("maxgrenades","50",0);
	maxrockets = gi.cvar("maxrockets","50",0);
	maxslugs = gi.cvar("maxslugs","50",0);
	maxcells = gi.cvar("maxcells","200",0);
	startingweapon=gi.cvar("startingweapon","1",0);
	startingarmorcount=gi.cvar("startingarmorcount","0",0);
	startingarmortype=gi.cvar("startingarmortype","1",0);
	
	hook_time	= gi.cvar("hook_time","10",0);
	hook_speed	= gi.cvar("hook_speed","1400",0);
	hook_damage	= gi.cvar("hook_damage","2",0);	
	pull_speed	= gi.cvar("pull_speed","650",0);
	skyhook		= gi.cvar("skyhook","0",0);
	regeneration	= gi.cvar("regeneration","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	haste			= gi.cvar("haste","1",CVAR_SERVERINFO);			//0 - 4 for number of keys
	futility		= gi.cvar("futility","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	infliction		= gi.cvar("infliction","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	bfk				= gi.cvar("bfk","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	stealth			= gi.cvar("stealth","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	antikey			= gi.cvar("antikey","1",CVAR_SERVERINFO);//0 - 4 for number of keys
	homing			= gi.cvar("homing","1",CVAR_SERVERINFO);//0 - 4 for number of keys

	droppable		= gi.cvar("droppable","1",0);//0 off 1 on
	playershells	= gi.cvar("playershells","1",0);	//0 off 1 on
	keyshells		= gi.cvar("keyshells","1",0);	//0 off 1 on
	respawntime		= gi.cvar("respawntime","60",0);//1 or 2 minutes
	qwfraglog		= gi.cvar("qwfraglog","0",CVAR_LATCH);	//0 off 1 on

	levelcycle		= gi.cvar("levelcycle","0",CVAR_NOSET);	//0 off 1 sequential 2 random
	resetlevels		= gi.cvar("resetlevels","0",0);
	pickupannounce	= gi.cvar("pickupannounce","1",0);	//0 off 1 on
	
	gibgun			= gi.cvar("gibgun","1",0);	//0 off 1 on
	flashgrenades	= gi.cvar("flashgrenades","1",0);	//0 off 1 on
	firegrenades	= gi.cvar("firegrenades","1",0);	//0 off 1 on
	freezegrenades	= gi.cvar("freezegrenades","1",0);	//0 off 1 on
	firerockets		= gi.cvar("firerockets","1",0);//0 off 1 on
	drunkrockets	= gi.cvar("drunkrockets","1",0);	//0 off 1 on
	hominghyperblaster		= gi.cvar("hominghyperblaster","1",0);//0 off 1 on
	
	allowfeigning	= gi.cvar("allowfeigning","1",0);	//0 off 1 on
	usevwep			= gi.cvar("usevwep","0",CVAR_LATCH);//0 off 1 on
	damagemultiply	= gi.cvar("damagemultiply","0",0);	//0 off 1 on

	bfgdamage				= gi.cvar("bfgdamage","200",0);
	raildamage				= gi.cvar("raildamage","100",0);
	supershotgundamage		= gi.cvar("supershotgundamage","6",0);
	shotgundamage			= gi.cvar("shotgundamage","4",0);
	chaingundamage			= gi.cvar("chaingundamage","6",0);
	machinegundamage		= gi.cvar("machinegundamage","8",0);
	hyperdamage				= gi.cvar("hyperdamage","15",0);
	blasterdamage			= gi.cvar("blasterdamage","15",0);
	rocketdamage			= gi.cvar("rocketdamage","100",0);
	rocketradiusdamage		= gi.cvar("rocketradiusdamage","120",0);
	rocketdamageradius		= gi.cvar("rocketdamageradius","120",0);
	grenadelauncherdamage	= gi.cvar("grenadelauncherdamage","120",0);	
	handgrenadedamage		= gi.cvar("handgrenadedamage","125",0);

	totalstealth			= gi.cvar("totalstealth","0",0);
	nozbots					= gi.cvar("nozbots","0",0);
	botfraglogging			= gi.cvar("botfraglogging","0",0);
	connectlogging			= gi.cvar("connectlogging","0",0);
		
	//Get Q2 Version
	sscanf(gi.cvar("version", 0, 0)->string, "%f", &qversion);
	//K2:End

	//ERASER
	// bot commands
	bot_num = gi.cvar ("bot_num", "0", 0);
	bot_name = gi.cvar ("bot_name", "", 0);
	bot_allow_client_commands = gi.cvar ("bot_allow_client_commands", "0", CVAR_ARCHIVE);
	bot_free_clients = gi.cvar ("bot_free_clients", "0", CVAR_ARCHIVE);
	bot_debug = gi.cvar ("bot_debug", "0", 0);
	bot_show_connect_info = gi.cvar ("bot_show_connect_info", "1", CVAR_ARCHIVE);
	bot_calc_nodes = gi.cvar ("bot_calc_nodes", "1", 0);
	bot_debug_nodes = gi.cvar ("bot_debug_nodes", "0", 0);
	bot_auto_skill = gi.cvar ("bot_auto_skill", "0", CVAR_ARCHIVE);
	bot_drop = gi.cvar ("bot_drop", "", 0);
	bot_chat = gi.cvar ("bot_chat", "1", CVAR_ARCHIVE);
	bot_optimize = gi.cvar ("bot_optimize", "1200", 0);
	bot_tarzan = gi.cvar ("bot_tarzan", "0", CVAR_ARCHIVE);
	maxbots = gi.cvar("maxbots","0",0);
	players_per_team = gi.cvar ("players_per_team", "4", CVAR_LATCH|CVAR_ARCHIVE);
	addteam = gi.cvar ("addteam", "", 0);
	teamplay = gi.cvar ("teamplay", "0", CVAR_LATCH|CVAR_ARCHIVE);
	ctf_auto_teams = gi.cvar ("ctf_auto_teams", "0", 0);
	ctf_special_teams = gi.cvar ("ctf_special_teams", "0", 0); //~~JLH
	ctf_humanonly_teams = gi.cvar ("ctf_humanonly_teams", "0", 0); //~~JLH
    // Diable ctf_auto_teams if ctf_special_teams is enabled //~~JLH
    if ( ctf_auto_teams->value > 0 && ctf_special_teams->value > 0 )  // ~~JLH
		gi.cvar_set ("ctf_auto_teams", "0");
	grapple = gi.cvar ("grapple", "0", CVAR_LATCH);
	//ERASER

	//view_weapons = gi.cvar ("view_weapons", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);

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

    
	//K2:Begin - Init Keys2 Stuff and Read in Level Cycle file
	//Open Keys2 Lockfile
	strcpy(lockfilename,gamed->string);
	strcat(lockfilename,"/keys2.lock");
	lockfile=fopen(lockfilename,"w");
	
	if(ctf->value)
	{
		sl_Logging( &gi, "Keys2CTF" );	// StdLog - Mark Davies (Only required to set patch name)
		gi.dprintf("CTF Server Mode\n");
	}
	else
	{
		sl_Logging( &gi, "Keys2DM" );	// StdLog - Mark Davies (Only required to set patch name)
		gi.dprintf("DM Server Mode\n");
	}

	if (levelcycle->value)
	{
		if(ctf->value)
			K2_ReadCTFLevelCycleFile(); 
		else
			K2_ReadDMLevelCycleFile(); 
	}
	else
		// MAP MOD
		map_mod_set_up();
		// MAP MOD

	//QWLogging
	//frag_#.log with lines \frager\fragee\ 
	if( qwfraglog->value )
	{
		cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
		char	filename[50]="";
		qboolean	all_files_used = false;
		int i;
	
		memset(filename,0,sizeof(filename));

		//Find next log file
		for(i=0;i<99;i++)
		{
			sprintf(filename,"%s/frag_%i.log",gamed->string,i);
			QWLogFile = fopen(filename,"r");
			
			//File does not exist, so we can use it
			if(!QWLogFile)
			{
				all_files_used = false;
				break;
			}
			
			all_files_used = true;
			fclose(QWLogFile);
		}

		//Create new logfile
#ifndef AMIGA
		QWLogFile = fopen(filename,"w+t");
#else
		QWLogFile = fopen(filename,"w");
#endif

		if (!QWLogFile)
		{
			gi.dprintf("Unable to create new QW Log File (%s)\nQW Logging disabled\n",filename);
			gi.cvar_set("qwfraglog","0");
		}
		else
			gi.dprintf("QW Logging ON: logged to %s \n",filename);
	}
		
		
	//K2:End

//ZOID
	CTFInit();
//ZOID

}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;


#ifdef AMIGA
	extern ULONG SegList;
	ULONG   Offset;
	UBYTE   Segment;
	ULONG   Index;
	void *  Address;
#define PACK_OFFSET(segment, offset) ((((ULONG)segment)<<24) | (offset & 0xffffff))
#endif
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

#ifndef AMIGA
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
#else
	case F_FUNCTION:
	case F_MMOVE:
		if (*(byte **)p == NULL)
		{
			Offset = 0;
			Segment = 0xFF;
		}
		else
		{
			Address = (void *) (*(byte **)p);
			if (FindAddressInSegList(SegList, Address, &Offset, &Segment) == FALSE)
			{
			    gi.error("WriteEdict: Unknown code position (not in SegList)");
			}
		}
		*(ULONG *)p = PACK_OFFSET(Segment, Offset);
		break;
#endif

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


#ifdef AMIGA
	extern ULONG SegList;
	ULONG   Offset;
	UBYTE   Segment;
	ULONG   Index;
	void *  Address;
#define UNPACK_OFFSET(x)    (x & 0xFFFFFF)
#define UNPACK_SEGMENT(x)   ((x & 0xFF000000)>>24)
#endif

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

#ifndef AMIGA
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
#else
	//relative to code segment
	case F_FUNCTION:
	case F_MMOVE:
		Index = *(ULONG *)p;
		if ( Index == 0 )
			*(byte **)p = NULL;
		else
		{
			Offset = UNPACK_OFFSET(Index);
			Segment = UNPACK_SEGMENT(Index);
			if (Segment == 0xFF)
			{
			    Address = 0;
			}
			else if (ExpandAddressInSegList(SegList, &Address, Offset, Segment) == FALSE)
			{
			    gi.error ("ReadEdict: can't translate offset/segment to code position");
			}

			*(byte **)p = (byte *)Address;
		}

		break;

#endif

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
