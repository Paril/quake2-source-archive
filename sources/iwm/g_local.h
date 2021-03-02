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
// g_local.h -- local definitions for game module

#include "q_shared.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN	//non-MFC
#include <windows.h>
#else
#define OutputDebugString
#endif

// add non-standard min/max macros if not Windows platform
#ifndef _WIN32
#ifndef __cplusplus
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#endif

// Paril: IWMBot
#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

//ZOID
#include "p_menu.h"
//ZOID

// Paril
#include "iwm_banning.h"

#define VERSION	"2.25"	//QW// single instance of version string

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION		"IWM "VERSION	//deathmatch

// protocol bytes that can be directly added to messages
#define	svc_bad					0
#define	svc_muzzleflash			1
#define	svc_muzzleflash2		2
#define	svc_temp_entity			3
#define	svc_layout				4
#define	svc_inventory			5
#define	svc_nop					6
#define	svc_disconnect			7
#define	svc_reconnect			8
#define	svc_sound				9
#define	svc_print				10
#define	svc_stufftext			11
#define	svc_serverdata			12
#define	svc_configstring		13
#define	svc_spawnbaseline		14
#define	svc_centerprintf		15
#define	svc_download			16
#define	svc_playerinfo			17
#define	svc_packetentities		18
#define	svc_deltapacketentities	19
#define	svc_frame				20

#define MAX_THINKS 8

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000
// Paril
#define SPAWNFLAG_NO_SPINNING		0x00002000
#define SPAWNFLAG_NO_DROPTOFLOOR	0x00004000

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active
#define FL_RESPAWN				0x80000000	// used for item respawning

//ROGUE
#define FL_MECHANICAL			0x00002000	// entity is mechanical, use sparks not blood
#define FL_SAM_RAIMI			0x00004000	// entity is in sam raimi cam mode
#define FL_DISGUISED			0x00008000	// entity is in disguise, monsters will not recognize.
#define	FL_NOGIB				0x00010000	// player has been vaporized by a nuke, drop no gibs
//ROGUE


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum 
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000
#define AI_CAR					0x00008000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_SHARD				4

// power armor types
#define POWER_ARMOR_NONE		0
#define POWER_ARMOR_SCREEN		1
#define POWER_ARMOR_SHIELD		2

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff


// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2


// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact

MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	// extra size to monsters
MOVETYPE_RUBBER,
MOVETYPE_BOUNCE,
MOVETYPE_STUCK,
MOVETYPE_VEHICLE,
MOVETYPE_PUSHABLE,
MOVETYPE_BOUNCENOSTOP,
MOVETYPE_BOUNCESTAY,
MOVETYPE_WALLBOUNCE
} movetype_t;



typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;


// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32
//ZOID
#define IT_TECH			64
//ZOID
#define IT_HEALTH		128
#define IT_FLAG			256

// gitem_t->weapmodel for weapons indicates model index
#define WEAP_BLASTER			1 
#define WEAP_SHOTGUN			2 
#define WEAP_SUPERSHOTGUN		3 
#define WEAP_MACHINEGUN			4 
#define WEAP_CHAINGUN			5 
#define WEAP_GRENADES			6 
#define WEAP_GRENADELAUNCHER	7 
#define WEAP_ROCKETLAUNCHER		8 
#define WEAP_HYPERBLASTER		9 
#define WEAP_RAILGUN			10
#define WEAP_BFG				11
#define WEAP_GRAPPLE			12
//JABot[start]
#define WEAP_NONE			0
#define WEAP_TOTAL			13
//JABot[end]

typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use

	char		*owned_item;

	long int bannumber;
	int weapnum;
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit
	char		forcemap[MAX_QPATH];	// go here

	// intermission state
	float		intermissiontime;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	int			power_cubes;		// ugly necessity for coop

	// Teamplay
	int team1_frags;
	int team2_frags;

	int dayoff;
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;

//	int			weight;//JABot
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);

	qboolean	is_blocked;
} moveinfo_t;


typedef struct
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;
} monsterinfo_t;



extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	int	sm_meat_index;
extern	int	snd_fry;

// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN		3
#define MOD_MACHINEGUN		4
#define MOD_CHAINGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_HYPERBLASTER	10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
#define MOD_HANDGRENADE		15
#define MOD_HG_SPLASH		16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE	24
#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER	33
#define MOD_FRIENDLY_FIRE	0x8000000
#define MOD_RUBBERBALL 34
#define MOD_ROCKETPOD 35
#define MOD_DEVASTATOR 36
#define MOD_RAILGUN_BOUNCE 37
#define MOD_SHARD 38
#define MOD_GRENADELASER 39
#define MOD_NEEDLER 40
#define MOD_FIREFLY 41
#define MOD_FOOBY 42
#define MOD_BALLOON 43
#define MOD_SCREWDRIVER 44
#define MOD_WOMD 45
#define MOD_BEE 46
#define MOD_BEE_SPLASH 47
#define MOD_BASKETBALL 48
#define MOD_BEACHBALL 49
#define	MOD_FORCE_SPLASH 50
#define MOD_HELD_FORCE 51
#define MOD_FORCE_FALLING 52
#define MOD_JACK 53
#define MOD_JACK_SPLASH 54
#define MOD_ZAPPED 55
#define MOD_PACMAN 56
#define MOD_PLASMA 57
#define MOD_MACE 58
#define MOD_DRILL 59
#define MOD_LIGHTNING_BOLTDISCHARGE 60
#define MOD_THUNDERBOLT 61
#define MOD_LIGHTNING_DIRECTDISCHARGE 62
#define MOD_PLASMA_SPLASH 63
#define MOD_ACIDDROP 64
#define MOD_DISKETTE 65
#define MOD_DISK 66
#define MOD_FIRESMALL 67
#define MOD_GAS 68
#define MOD_BURNT 69
#define MOD_SPIKEBOMB_HIT 70
#define MOD_LAVABALL 71
#define MOD_LAVABALL_SPLASH 72
#define MOD_SHOCKWAVE 73
#define MOD_INFESTED 74
#define MOD_ARTILLERY 75
#define MOD_BLADE 76
#define MOD_DKHAMMER 77
#define MOD_DETONATOR 78
#define MOD_HOOK			79
#define MOD_GRAPPLE			79
//========
//ROGUE
#define MOD_CHAINFIST			80
#define MOD_DISINTEGRATOR		81
#define MOD_ETF_RIFLE			82
#define MOD_BLASTER2			83
#define MOD_HEATBEAM			84
#define MOD_TESLA				85
#define MOD_PROX				86
#define MOD_TRACKER				87
//ROGUE
//========
#define MOD_RIPPER				88
#define MOD_PHALANX				89
#define MOD_TRAP				90
#define MOD_PIG_GIB             91
#define MOD_PIG_REMOVE          92
#define MOD_PIG_TRANSFORM       93
#define MOD_PIG_GRAVITY         94
#define MOD_HEATLASER			95
#define MOD_CHAINLINK			96
#define MOD_STAPLE				97
#define MOD_BEAMCANNON3			98
#define MOD_BEAMCANNON2			99
#define MOD_BEAMCANNON			100
#define	MOD_AGM_FLING		101
#define MOD_AGM_SMASH		102
#define MOD_AGM_DROP		103
#define MOD_AGM_THROW		104
#define MOD_AGM_HIT			105
#define MOD_AGM_LAVA_HELD	106
#define MOD_AGM_LAVA_DROP	107
#define MOD_AGM_SLIME_HELD	108
#define MOD_AGM_SLIME_DROP	109
#define MOD_AGM_WATER_HELD	110
#define MOD_AGM_TRIG_HURT	111
#define	MOD_AGM_TARG_LASER	112
#define MOD_AGM_FEEDBACK	113
#define MOD_AGM_BEAM_REF	114
#define MOD_AGM_DISRUPT		115
#define MOD_LZAP			116
#define MOD_VEHICLE			117
#define MOD_STAR			118

extern	int	meansOfDeath;


extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*filterban;

extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;
extern	cvar_t	*iwm_gamemode;
extern	cvar_t	*servermessage;
extern cvar_t *cg_startweap;
extern cvar_t *iwm_version;
extern cvar_t *cg_hints;
extern	cvar_t	*iwm_editor;

//ZOID
extern	cvar_t	*capturelimit;
extern	cvar_t	*instantweap;
//ZOID

extern	cvar_t *bfg_ban;
extern	cvar_t *railgun_ban;
extern	cvar_t *hyperblaster_ban;
extern	cvar_t *rl_ban;
extern	cvar_t *grenade_ban;
extern	cvar_t *gl_ban;
extern	cvar_t *chaingun_ban;
extern	cvar_t *machinegun_ban;
extern	cvar_t *ssg_ban;
extern	cvar_t *shotgun_ban;
extern	cvar_t *blaster_ban;
extern	cvar_t *feature_ban;
extern	cvar_t *item_ban;

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1
#define FFL_NOSPAWN			2

typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;


extern	field_t fields[];
extern	gitem_t	itemlist[];


//
// g_cmds.c
//
qboolean CheckFlood(edict_t *ent);
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
//gitem_t	*FindItem (char *pickup_name);
//gitem_t	*FindItemByClassname (char *classname);
#ifdef _WIN32
extern __declspec(dllexport) gitem_t	*FindItemByClassname (char *classname);
extern __declspec(dllexport) gitem_t	*FindItem (char *pickup_name);
extern __declspec(dllexport) void SpawnItem (edict_t *ent, gitem_t *item);
#else
gitem_t	*FindItemByClassname (char *classname);
gitem_t	*FindItem (char *pickup_name);
void SpawnItem (edict_t *ent, gitem_t *item);
#endif
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);

#ifdef _WIN32
extern __declspec(dllexport) edict_t *findradius (edict_t *from, vec3_t org, float rad);
extern __declspec(dllexport) edict_t *G_Spawn (void);
#else
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_Spawn (void);
#endif

edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);
void vectoangles2 (vec3_t value1, vec3_t angles);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
//ROGUE
#define DAMAGE_DESTROY_ARMOR	0x00000040	// damage is done to armor and health.
#define DAMAGE_NO_REG_ARMOR		0x00000080	// damage skips regular armor
#define DAMAGE_NO_POWER_ARMOR	0x00000100	// damage skips power armor
//ROGUE

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

//
// g_monster.c
//
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);
void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);
void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void swimmonster_start (edict_t *self);
void flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);
void M_FlyCheck (edict_t *self);
void M_CheckGround (edict_t *ent);

//
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

//
// g_ai.c
//
void AI_SetSightClient (void);

void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

#define __int64 

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int ric);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

//
// g_ptrail.c
//
void PlayerTrail_Init (void);
void PlayerTrail_Add (vec3_t spot);
void PlayerTrail_New (vec3_t spot);
edict_t *PlayerTrail_PickFirst (edict_t *self);
edict_t *PlayerTrail_PickNext (edict_t *self);
edict_t	*PlayerTrail_LastSpot (void);


//
// g_client.c
//
void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);

//
// g_player.c
//
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_svcmds.c
//
void	ServerCommand (void);
qboolean SV_FilterPacket (char *from);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);
//void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

//
// g_main.c
//
void ClientDisconnect (edict_t *ent);
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
void EndDMLevel (void);


//
// FIXME: figure out where these belong and reorganize them
//
qboolean Bot_CheckAttack (edict_t *ent, usercmd_t *cmd);
qboolean IsInfested (edict_t *infested);
void RemoveInfests (edict_t *infested);
void CheckSoundIndexes (edict_t *ent);
void CheckImageIndexes (edict_t *ent);
void Cmd_TelePuddle (edict_t *ent);
void RepelMagnets (edict_t *self);
void DestroyMagnets (edict_t *self);
void AttractMagnets (edict_t *self);
void CreateStickingFire (edict_t *ent, vec3_t origin, edict_t *other);
qboolean SV_RunThink (edict_t *ent);
qboolean SV_RunThink2 (edict_t *ent);
qboolean SV_RunThink3 (edict_t *ent);
qboolean SV_RunThink4 (edict_t *ent);
void G_MuzzleFlash(int rec_no,vec3_t origin,int flashnum);
void Frag_Get (edict_t *fragger, int frags);
void ThrowGib2(edict_t *self,char *gibname);
void ThrowBodyParts(edict_t *self);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void stuffcmd(edict_t *ent, char *s);
void Svcmd_Bot_f(void);
void MakeFreezeExplosion (edict_t *owner, vec3_t origin);
void MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (edict_t *owner, vec3_t origin, int Backwards);
void P_ProjectSource_Dev (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result, int left);
void Cmd_AGMFling_f(edict_t *ent);
void ThrowClientHead2(edict_t *self);
void fire_blaster2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void MonsterObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void CheckGibOverflow (void);
void Bot_Respawn(edict_t *ent);
qboolean findspawnpoint (edict_t *ent);
void CTFApplyRegenerationTech(edict_t *ent);
void VelocityForDamage (int damage, vec3_t v);
void ClipGibVelocity (edict_t *ent);



//============================================================================

//#include "ai\ai.h"//JABot

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6


#define AWARD_IMPRESSIVE	0
#define AWARD_EXCELLENT		1
#define AWARD_ASSIST		2
#define AWARD_DEFENCE		3
#define AWARD_ACCURACY		4

#define MAX_AWARDS		24

typedef struct
{
	int awards [MAX_AWARDS];
	float award_framenum [MAX_AWARDS];
	int awards_base [MAX_AWARDS];
	int award_needed [MAX_AWARDS];
} player_awards_t;

// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			savedFlags;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;

	qboolean	spectator;			// client is a spectator

	// Paril
	 qboolean	view_rocket; // okay to guide a rocket?
	 qboolean	guided; //fire guided rockets?

	 int grenadetype;
	 int flies;
	 int womded;
	 int temp_nofall;
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	qboolean	spectator;			// client is a spectator

	player_awards_t awards;

   edict_t *artillery;
   edict_t *artillery_dest;
   int artillery_placed;
   edict_t *artillery_base;

   edict_t *puddle_dest;
   edict_t *puddle_tele;
   int layed_tele;

//ZOID
	int			ctf_team;			// CTF team
	int			ctf_state;
	float		ctf_lasthurtcarrier;
	float		ctf_lastreturnedflag;
	float		ctf_flagsince;
	float		ctf_lastfraggedcarrier;
	qboolean	id_state;
	qboolean	voted; // for elections
	qboolean	ready;
	qboolean	admin;
	struct ghost_s *ghost; // for ghost codes
	int layed_t;
	//ZOID
	
	float      entertime;      // level.time the client entered the game
	int         votes_started; 
	// Paril: Teamplay stuff goes here.
	// FIXME: This stuff is re-used for different gamemodes. Is this a good idea?
	int team;
	int deaths;
	float seconds;
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	showhelpicon;

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	qboolean	weapon_thunk;

	gitem_t		*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	float		quad_framenum;
	float		invincible_framenum;
	float		breather_framenum;
	float		enviro_framenum;

	qboolean	grenade_blew_up;
	float		grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		flood_locktill;		// locked from talking
	float		flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	float		respawn_time;		// can respawn when time > this

	edict_t		*chase_target;		// player we are chasing
	qboolean	update_chase;		// need to update chase info?

	// new hook
	int hookstate; // Hook states flags 
	int hooktype; // 1=CableGrapple, 2=LaserGrapple 
	
	int num_hooks;

	float blindTime, blindBase;
	float force_framenum;

	int old_gunindex;

	vec3_t old_origin;
	int rc_left, rc_forward;

	edict_t *clone1, *clone2, *clone3, *clone4;
	int clones;
	int			vehicle_framenum;	// last time player engaged or disengaged vehicle
	usercmd_t	ucmd;				// Lazarus: Copied for convenience in ClientThink;
	int use;

//ZOID
	void		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_regentime;		// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
//	edict_t		*chase_target;
//	qboolean	update_chase;
	float		menutime;			// time to update menu
	qboolean	menudirty;
//ZOID	qboolean showing_message_test;
	float ir_framenum;
	pmove_t pmove;
	int forwardmove;
	float reg_framenum;
//	float ctf_regentime;
	gitem_t *temp_oldweapon;
	float dkhammer_framenum;
	int old_weaponstate;
	int old_gunframe;
	float boots_framenum;
	float cloak_framenum;

   /*ATTILA begin*/
   float	Jet_framenum;   /*burn out time when jet is activated*/
   float	Jet_remaining;  /*remaining fuel time*/
   float	Jet_next_think; 
   /*ATTILA end*/

   float deflector_framenum;
   int weap_s;
   int teleing;
//ZOID
	qboolean	inmenu;				// in menu
	pmenuhnd_t	*menu;				// current menu
//ZOID

	float tracker_pain_framenum;

	int volcanoed;
	int chamber;

// CYBERSLASH
	float		notarg_debounce_time;   // used to stop a stream of "no target" msgs
	qboolean	pigged;                 // true if not in the player model for a pig reason
// CYBERSLASH
	float cloak_turninvis_framenum;
	int beam_diam;
	int beam_diamcount;
	float hint_framenum;

	edict_t		*agm_target;
	float		agm_range;
	int			agm_charge;
	qboolean	agm_on;
	qboolean	agm_push;
	qboolean	agm_pull;
	qboolean	agm_showcharge;
	qboolean	agm_tripped;

	edict_t *laser_links[30];

	float star_framenum;
	float star_waitflash;

	vec3_t rc_stay_here_origin;

	float seconds;
	pmenu_t menuc[1536];
};
typedef void (*Fx)(edict_t *ent); // pointer to function
struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf
	
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		nextthink2;
	void		(*think2)(edict_t *self);

	float		nextthink3;
	void		(*think3)(edict_t *self);

	float		nextthink4;
	void		(*think4)(edict_t *self);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;
	float		gravity_debounce_time;		// used by item_ movement commands to prevent
											// monsters from dropping to floor

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	float		powerarmor_time;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	float		teleport_time;

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	// Paril
	edict_t		*chasetarget; //the rocket to guide
	int			charge; //acceleration of the rocket

	edict_t *vehicle; // Monster 'Gladiator' Vehicle
	edict_t *transformer; // 'Gladiator' Vehicle's Rider..
	qboolean is_transformed; // ent is transformed..
	edict_t *laz_vehicle;

	// Paril, Rot. Las. Gr.
	// How many times will it move upwards?
	// I don't know, look at the code.
	int times;
	edict_t *other;

	// Stick.
	edict_t *stuckentity;
	vec3_t oldstuckorigin;
	vec3_t oldstuckangles;

	cplane_t *tempplane;
	csurface_t *tempsurf;
	
	// Flashlight
	edict_t *flashlight;

	vec3_t tempdir;
	int tempspeed;

	vec3_t oldvelocity;
	int inair;

	float gravity_lift;

	edict_t *infester;
	float infest_debounce_time;

	float DrunkTime;
	float DizzyYaw;
	float DizzyPitch;
	float DizzyRoll;

	int frozen;
	float frozentime;
	vec3_t tempvector;
	int tempint;

	edict_t *forcer;
	int old_health;

	int weapnum;

	edict_t *fire_entity;
	float fire_framenum;
	float fire_hurtframenum;
	int fract;
	edict_t *temp_other;
	float		radius;
	vec3_t		org_size;		// Initial size of the vehicle bounding box,

	vec3_t origin_offset;

	float headache_framenum;
	int can_teleport_through_destinations;
	edict_t *tempent;
	edict_t *carpet;
	int carpet_active;
	qboolean bounced_off_player;
	int tempint2;

//	ai_handle_t		ai;		//JABot
	// Paril: Bot stuff
	// FIXME: Put this in it's own thingy one day.
	qboolean is_bot;
	float bot_wandertime;
	edict_t *bot_goal;
	qboolean bot_hasgoal;
	edict_t *bot_playergoal;

	int bot_usingautomatic;
	int has_flashlight;
	float tracker_framenum;
	edict_t *tempent2;
	vec3_t tempvec;
	float tempfloat;
	
	int bot_ping;
	int bot_favwep;
	edict_t *freeze_ent;
	int is_gib;
	edict_t *get_monster_ent;
	int is_spawned;
	char *name;

	edict_t		*agm_enemy;
	qboolean	held_by_agm;
	qboolean	flung_by_agm;			// player used "fling" command
	qboolean	thrown_by_agm;			// victim given high velocity by an AGM, but not via "fling" cmd

	qboolean lightning_chain;
	char *tempchar1;
	edict_t *ed_cleaner;
	qboolean deflected;
	float burning_framenum;
	edict_t *burner_entity;
	edict_t *last_heater;
	int spawned_item;
	edict_t *laserer;
	
	qboolean lagged;
	float latency_time;

	// Think system!
	Fx thinks[MAX_THINKS];
	float nextthinks[MAX_THINKS];
};

//#include "tutorial.h"
#endif

// Paril
// Routine #1: MACRO - Random Number
// Returns a random number between y and z.
#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))


qboolean G_EntExists(edict_t *ent);
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin );
qboolean G_ClientInGame(edict_t *ent);
qboolean G_ClientNotDead(edict_t *ent);

#define COLOR_RED	  0xf2f2f0f0//red
#define COLOR_GREEN	  0xd0d1d2d3//green
#define COLOR_BLUE	  0xf3f3f1f1//blue
#define COLOR_YELLOW	  0xdcdddedf//yellow
#define COLOR_YELLOW2	  0xe0e1e2e3//yellow strobe
#define COLOR_PURPLE	  0x80818283//dark purple
#define COLOR_BLUE2	  0x70717273//light blue
#define COLOR_GREEN2	  0x90919293//different green
#define COLOR_PURPLE2	  0xb0b1b2b3//purple
#define COLOR_RED2	  0x40414243//different red
#define COLOR_ORANGE	  0xe2e5e3e6//orange
#define COLOR_MIX	  0xd0f1d3f3//mixture
#define COLOR_INNER1	  0xf2f3f0f1//inner = red, outer = blue
#define COLOR_INNER2	  0xf3f2f1f0//inner = blue, outer = red
#define COLOR_INNER3	  0xdad0dcd2//inner = green, outer = yellow
#define COLOR_INNER4	  0xd0dad2dc //inner = yellow, outer = green

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

extern qboolean	is_quad;
extern byte		is_silenced;

extern edict_t *firsthole, *lasthole;
extern int holes;

edict_t *CreateEntity (vec3_t n_origin, vec3_t n_angles, int n_movetype, int n_clipmask, int n_solid, int n_effects, int n_renderfx, int n_usebbox, vec3_t n_mins, vec3_t n_maxs, edict_t *n_owner);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result, int left);
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void Grenade_Explode (edict_t *ent);
int ISGL (edict_t *ent);
void NoAmmoWeaponChange (edict_t *ent);
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin ) ;
void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin ) ;
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin) ;
void G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin ) ;
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) ;
void fire_melee (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod);
void freeze_player(edict_t *ent);
void unfreeze_player(edict_t *ent);


extern int daycycletime;
extern int lightlevel;
extern int oldlightlevel;
extern cvar_t *day;
extern int flies;

extern int	jacket_armor_index;
extern int	combat_armor_index;
extern int	body_armor_index;
extern int	power_screen_index;
extern int	power_shield_index;

void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );
void Jet_ApplyRolling( edict_t *ent, vec3_t right );
void Jet_ApplySparks ( edict_t *ent );
void Jet_ApplyLifting( edict_t *ent );
void Jet_BecomeExplosion( edict_t *ent, int damage );
qboolean Jet_AvoidGround( edict_t *ent );
qboolean Jet_Active( edict_t *ent );

#ifdef WIN32
extern __declspec(dllexport) int ModelIndex (char *index);
extern __declspec(dllexport) int SoundIndex (char *index);
extern __declspec(dllexport) int ImageIndex (char *index);
#else
int ModelIndex (char *index);
int SoundIndex (char *index);
int ImageIndex (char *index);
#endif

extern char *currentmodelindexes[255];
extern int nummodelindexes;
extern char *modelindex_notamodel[255];

extern char *currentsoundindexes[255];
extern int numsoundindexes;

extern char *currentimageindexes[255];
extern int numimageindexes;

//};

//ZOID
#include "g_ctf.h"
//ZOID
extern gitem_t *flag1_item;
extern gitem_t *flag2_item;
void SendSoundToAll (int soundindex, float volume);
void unicastSound(edict_t *player, int sound_index, float volume);
void safe_centerprintf (edict_t *ent, char *fmt, ...);

#ifdef _WIN32
extern __declspec(dllexport) void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
extern __declspec(dllexport) void safe_bprintf (int printlevel, char *fmt, ...);
#else
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void safe_bprintf (int printlevel, char *fmt, ...);
#endif

void pig_checksplat (edict_t *ent);

typedef void (*ff12)(edict_t *ent, char *cmd, char mapname[MAX_QPATH], float time, int framenum, char *the_entities);
extern ff12 EditorCommands;
extern int level_gibs;
extern int total_monsters;
extern char *entities_glb;

void Nuke_Quake (edict_t *self);
void Follow_Owner_Infest (edict_t *s);
void R_SetModel (edict_t *e, char *s);
extern cvar_t	*display_writes;
gitem_t *RandomizeItem (gitem_t *item);
void RunThinks (edict_t *ent);
void OpenTeamMenu (edict_t *ent, pmenuhnd_t *p);
edict_t *CopyToBodyQue(edict_t *ent);

#include "paril_write.h"
#include "maplist.h"
#include "l_voting.h"

#include "paril_logging.h"
#include "paril_tp.h"

extern FILE *filpt;

float AmountToLag (edict_t *self);
qboolean IsLagged (edict_t *self);