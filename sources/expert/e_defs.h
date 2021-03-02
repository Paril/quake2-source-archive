/**
 * e_defs.h
 * Used for definitions used by Expert code primarily 
 * in files from the original game DLL source distribution
 * and in e_game.c.
 *
 * Also includes header files from the subsystems of Expert;
 * the .c files in those subsystems include g_local.h, which
 * includes this file.
 */

// Version of the Expert modification
#define EXPERT_VERSION "3.2"

// additional DM Flags
#define DF_ARMOR_PROTECT					(1 << 18) 		// 262144

#define NUM_SETTINGS						19

// FIXME : teamflags?
// expflags cvar flag meanings
#define EXPERT_WEAPONS						(1 << 0)		// 1
#define EXPERT_BALANCED_ITEMS				(1 << 1)		// 2
#define EXPERT_FREE_GEAR					(1 << 2)		// 4
#define EXPERT_POWERUPS						(1 << 3)		// 8
#define EXPERT_NO_POWERUPS					(1 << 4)		// 16
#define EXPERT_HOOK							(1 << 5)		// 32
#define	EXPERT_NO_HACKS						(1 << 6)		// 64
#define EXPERT_PLAYERID						(1 << 7)		// 128
#define EXPERT_ENFORCED_TEAMS			 	(1 << 8)		// 256
#define EXPERT_FAIR_TEAMS					(1 << 9)		// 512
#define EXPERT_NO_TEAM_SWITCH				(1 << 10)		// 1024
#define EXPERT_POGO							(1 << 11)		// 2048
#define EXPERT_SLOW_HOOK					(1 << 12)		// 4096
#define EXPERT_SKY_SOLID					(1 << 13)		// 8192
#define EXPERT_NO_PLATS						(1 << 14)		// 16384
#define EXPERT_CTF_TEAM_DISTRIBUTION		(1 << 15)		// 32768
#define EXPERT_ALTERNATE_RESTORE			(1 << 16)		// 65536
#define EXPERT_AMMO_REGEN					(1 << 17)		// 131072
#define EXPERT_SHORT_HOOK					(1 << 18)		// 262144
#define EXPERT_RADAR						(1 << 19)		// 524288

extern	char e_bits[NUM_SETTINGS][25];

// arenaflags cvar flag meanings
#define EXPERT_ENABLE_ARENA					(1 << 0)		// 1

// utilflags cvar flag meanings
#define EXPERT_ENABLE_GIBSTAT_LOGGING		(1 << 0)		// 1	Log to GibStat compat. file
#define EXPERT_DISABLE_CLIENT_OBITUARIES	(1 << 1)		// 2	Disable Context Obit code
//#define EXPERT_RESERVED					(1 << 2)		// 4	Reserved for future use
#define EXPERT_DISABLE_MOTD					(1 << 3)		// 8	Disable MOTD
#define EXPERT_DISABLE_SPAWNMSGS			(1 << 4)		// 16	Disable Spawn messages
#define EXPERT_DISABLE_BOT_DETECT			(1 << 5)		// 32	Disable Bot Detection
#define EXPERT_ENABLE_PLAYER_FORWARDING		(1 << 6)		// 64	Forward players to another 
															//		server when full
#define EXPERT_GIBSTAT_PER_GAME				(1 << 7)		// 128  Log each map to a separate
															//		log file

// flags cvar shortcuts
#define expflags ((int)sv_expflags->value)
#define utilflags ((int)sv_utilflags->value)
#define arenaflags ((int)sv_arenaflags->value)

// other cvar shortcuts
#define pace ((float)sv_pace->value)
#define lethality ((float)sv_lethality->value)

#ifdef _WIN32
// MSVC++ #pragma to disable warnings about unreferenced formal parameters,
// constant conditional expressions, and assignments in conditionals
#pragma warning(disable : 4100 4127)
// MSVC++ #pragma to disable warnings about vars being used before being initialized.
// As it turns out, they DO get initialized, but the compiler isn't smart enough to
// figure it out..
#pragma warning(disable : 4701)
// MSVC++ #pragma to disable warnings about wacky typecasts. Admittedly dangerous,
// use at your own risk
#pragma warning(disable : 4054)
#endif

// Possible game types
enum gametype_t {
	GAME_SINGLE,
	GAME_DM,
	GAME_TEAMPLAY,
	GAME_CTF,
	GAME_ARENA,
} gametype;

// gametype string for spawn message
#define GAMESTRING	(gametype == GAME_CTF ? "CTF" : (gametype == GAME_TEAMPLAY ? "Teams" : (gametype == GAME_ARENA ? "Arena" : "DM")))

// gametype string for "mode" cvar for server browsers
#define MODESTRING	(gametype == GAME_CTF ? "ctf" : (gametype == GAME_TEAMPLAY ? "teamplay" : (gametype == GAME_ARENA ? "arena" : "deathmatch")))

#define 	NEVER_CONNECTED			0
#define		CONNECTED				1
#define		DISCONNECTED			2

// Client resolutions possible
// Not used, but may be in the future..
typedef enum res_t {
	RES_320_240,
	RES_400_300,
	RES_512_384,
	RES_640_480,
	RES_800_600,
	RES_960_720,
	RES_1024_768,
	RES_1152_864,
	RES_1280_960,
	RES_1600_1200,
} res_t;

// Filename for logs and MOTD
#define EXPERT_MOTD_FILENAME				"motd.txt"

// Fraction of splash damage the firer takes
#define EXPERT_SPLASH_FRACTION			 	0.5

// Modified damages, radii, and spreads for weapons

// Constrain shot spreads with Expert weapons
#define EXPERT_MACHINEGUN_HSPREAD			75
#define EXPERT_MACHINEGUN_VSPREAD			75
#define EXPERT_CHAINGUN_HSPREAD				500
#define EXPERT_CHAINGUN_VSPREAD				500
#define EXPERT_MACHINEGUN_DAMAGE			6	
#define EXPERT_CHAINGUN_DAMAGE				4	

// Increase the shot count and damage for the assault shotgun
#define EXPERT_DEATHMATCH_SHOTGUN_COUNT		16
#define EXPERT_SHOTGUN_DAMAGE				5	

#define EXPERT_SSHOTGUN_COUNT				16

// Hand grenades explode only after being thrown,
// a fixed length of time after the throw.  They take
// less time to throw and are thrown at higher speed,
// but do less damage.  The damage done drops off 
// slowly from the center of the explosion. 
#define EXPERT_HG_EXPLODE_TIMER				0.5
#define EXPERT_HG_SPEED_TIMER				1.5
#define EXPERT_HG_MINSPEED					400
#define EXPERT_HG_MAXSPEED					1200
#define EXPERT_HG_DAMAGE					85
#define EXPERT_HG_RADIUS					120	
#define EXPERT_HG_DROPOFF					0.3

#define EXPERT_GRENADELAUNCHER_DAMAGE		100

#define EXPERT_RAILGUN_DAMAGE				40
#define EXPERT_RAILGUN_KICK					60

#define EXPERT_SPREAD_BLASTER_DAMAGE		25
#define EXPERT_SPREAD_BLASTER_SPEED			1000

// radius damage should decline evenly from direct impact.
#define EXPERT_ROCKET_DAMAGE				100	
#define EXPERT_ROCKET_RADIUS				120	

#define EXPERT_HYPER_BLASTER_DAMAGE			14
#define EXPERT_HYPERBLASTER_SPEED			1000

#define EXPERT_BFG_RADIUS					500
#define EXPERT_BFG_SPEED					300
#define EXPERT_BFG_DIVIDER					10
// The radius for BFG lasers
#define EXPERT_BFG_LASER_RADIUS				512

// Modified health sizes
#define BALANCED_HEALTH_SMALL				10
#define BALANCED_HEALTH_MEDIUM				15
#define BALANCED_HEALTH_LARGE				20

// Modified ammo quantities
#define BALANCED_ROCKET_QUANTITY			10
#define BALANCED_SLUG_QUANTITY				25	

// Maximum Vampire Artifact health
#define MAX_VAMPIRE_HEALTH					180
// max under alternate restore, which has 
// higher normal max health
#define ALTERNATE_MAX_VAMPIRE_HEALTH		210

// Balanced armor stats are handled in g_items.c

// The amount of armor value points that shards are equal to under balanced items.
// Divide by armor resistance (.3, .6, .8) to yield how many points to add to armor.
// Three shards are equivalent to a full suit of armor.
#define BALANCED_SHARD_POINTS				16

// How long a dropped item stays in the world
#define BALANCED_DROPPED_ITEM_TIME			30

// bot detection

// Zbot-specific checks
#define BOT_CHECK_BANG_STUFFCMD				1
// General checks
#define BOT_CHECK_CLIENT_CVAR				2
#define BOT_CHECK_USERINFO_CVAR				4

// Mask indicating all tests passed
#define BOT_CHECK_FLAGS_MASK	BOT_CHECK_BANG_STUFFCMD	

// Expert: needed for armor info replacement
extern gitem_armor_t balanced_bodyarmor_info;
extern gitem_armor_t balanced_combatarmor_info;
extern gitem_armor_t balanced_jacketarmor_info;

// Expert: additional cvars
extern cvar_t *version;		// DLL version
extern cvar_t *gamedir;		// Game directory

// bitflag settings cvars
extern cvar_t *ctf;		// CTF flags
extern cvar_t *flagtrack;		// FlagTrack flags
extern cvar_t *sv_expflags;	// Expert flags
extern cvar_t *sv_utilflags;	// Obituary/GibStat/Utility flags
extern cvar_t *sv_arenaflags;	// Expert Arena flags

// value settings cvars
extern cvar_t *capturelimit;	// Captures before level ends
extern cvar_t *levelCycle;		// Number of teams
extern cvar_t *sv_numteams;		// Directory to get level-specific .cfgs from
extern cvar_t *sv_pace;			// Game pace multiplier
extern cvar_t *sv_lethality;	// Damage multiplier

// Expert: Obituary/Context CVars
extern cvar_t *sv_giblog;		// GibStats Log Filename

// What action to take when a bot is detected
extern cvar_t *botaction;

// Expert: Server pausing code
extern cvar_t *sv_paused;		// Paused cvar

// Player forwarding
extern cvar_t *forwardAddress;
extern cvar_t *forwardMessage;

// 
// e_game.c
// 
// These functions exist almost exclusively to package large stretches 
// of code changes in the main game source, to make merging with new
// versions easier.
// 
void ExpertGameInits();
void ExpertLevelScripting();
void ExpertLevelInits();
void ExpertPlayerDelayedInits(edict_t *player);
void ExpertPlayerLevelInits(edict_t *player);

void InitCmds(edict_t *player);
void ClientLeavePlay(edict_t *player);
void ExpertPlayerDisconnect(edict_t *player);
qboolean ExpertInhibit(edict_t *ent);
void ExpertItemListChanges();
void LoadCustomEntmap(char *mapname, char **entities);

void ExpertBotDetect(edict_t *player);
qboolean canPickupArkOfLife(edict_t *player);
void pickupArkOfLife(edict_t *ark, edict_t *player);
void ExpertPickupDroppedWeapon(edict_t *weapon, edict_t *player);
void ExpertAddToDroppedWeapon(edict_t *weapon, edict_t *player);
void alternateRestoreKill(edict_t *attacker, edict_t *target);
void damageRestore(edict_t *attacker, edict_t *targ, int damage);
void regen(edict_t *player);

// VWEP
void ShowGun(edict_t *ent);

void ItemEffects(edict_t *dropped);

gitem_t *giveFreeGear(gclient_t *client);
// FIXME : this function moves normal function (default shard points)
// out of the main source.
int ShardPoints(int armor_index);

void Cmd_SwitchFire_f(edict_t *player);
#define		WAVE_FLIPOFF		0
#define		WAVE_SALUTE			1
#define		WAVE_TAUNT			2	
#define		WAVE_WAVE			3	
#define		WAVE_POINT			4	
void wave(edict_t *ent, int waveNum);

// FIXME : every subsystem should have it's own separate header file
// and should be self-contained to as great an extent as possible, so that they
// can be broken off and packaged separately with ease.

// subsystems

// Utils: should only be dependant on unmodified game src.
#include "darray.h"
#include "list.h"
#include "props.h"

#include "e_util.h"

// CTF code
#include "g_ctf.h"

// Observer/Chasecam code
#include "g_observe.h"

// FlagTrack code
#include "e_ftrack.h"

// Custom obituary system
#include "e_obit.h"

// Gibstats logging
#include "e_gbstat.h"

// Layouts
#include "e_overlay.h"
#include "e_matrix.h"

// Grappling hook
#include "e_grapple.h"

// Arena subsystem
#include "e_arena.h"

// Teamplay system
#include "e_team.h"

//
// e_motd.c
//
void InitMOTD(void);
void DisplayMOTD(edict_t *client);
void DisplayRespawnLine(edict_t *client);
void DisplaySettings(edict_t *client);

//
// e_id.c
//
void Cmd_ID_f (edict_t *ent);
void SetIDView(edict_t *ent);

extern props_t *gProperties;

