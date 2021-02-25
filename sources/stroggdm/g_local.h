// g_local.h -- local definitions for game module

#include "q_shared.h"

#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

// Paril, include menus functs
#include "sdm_menus.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"baseq2"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define	svc_stufftext		11

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
//ROGUE
#define AI_WALK_WALLS			0x00008000
#define AI_MANUAL_STEERING		0x00010000
#define AI_TARGET_ANGER			0x00020000
#define AI_DODGING				0x00040000
#define AI_CHARGING				0x00080000
#define AI_HINT_PATH			0x00100000
#define	AI_IGNORE_SHOTS			0x00200000
// PMM - FIXME - last second added for E3 .. there's probably a better way to do this, but
// this works
#define	AI_DO_NOT_COUNT			0x00400000	// set for healed monsters
#define	AI_SPAWNED_CARRIER		0x00800000	// both do_not_count and spawned are set for spawned monsters
#define	AI_SPAWNED_MEDIC_C		0x01000000	// both do_not_count and spawned are set for spawned monsters
#define	AI_SPAWNED_WIDOW		0x02000000	// both do_not_count and spawned are set for spawned monsters
#define AI_SPAWNED_MASK			0x03800000	// mask to catch all three flavors of spawned
#define	AI_BLOCKED				0x04000000	// used by blocked_checkattack: set to say I'm attacking while blocked 
											// (prevents run-attacks)
//ROGUE
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
MOVETYPE_BOUNCE,
MOVETYPE_MORE_BOUNCE,
// RAFAEL
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

	// Paril - Teamplay Frags
	int			land_frags;
	int			air_frags;
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
	void		(*dodge)(edict_t *self, edict_t *other, float eta, trace_t *tr);
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
//ROGUE
	qboolean	(*blocked)(edict_t *self, float dist);
//	edict_t		*last_hint;			// last hint_path the monster touched
	float		last_hint_time;		// last time the monster checked for hintpaths.
	edict_t		*goal_hint;			// which hint_path we're trying to get to
	int			medicTries;
	edict_t		*badMedic1, *badMedic2;	// these medics have declared this monster "unhealable"
	edict_t		*healer;	// this is who is healing this monster
	void		(*duck)(edict_t *self, float eta);
	void		(*unduck)(edict_t *self);
	void		(*sidestep)(edict_t *self);
	//  while abort_duck would be nice, only monsters which duck but don't sidestep would use it .. only the brain
	//  not really worth it.  sidestep is an implied abort_duck
//	void		(*abort_duck)(edict_t *self);
	float		base_height;
	float		next_duck_time;
	float		duck_wait_time;
	edict_t		*last_player_enemy;
	// blindfire stuff .. the boolean says whether the monster will do it, and blind_fire_time is the timing
	// (set in the monster) of the next shot
	qboolean	blindfire;		// will the monster blindfire?
	float		blind_fire_delay;
	vec3_t		blind_fire_target;
	// used by the spawners to not spawn too much and keep track of #s of monsters spawned
	int			monster_slots;
	int			monster_used;
	edict_t		*commander;
	// powerup timers, used by widow, our friend
	float		quad_framenum;
	float		invincible_framenum;
	float		double_framenum;
//ROGUE
} monsterinfo_t;



extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	int	sm_meat_index;
extern	int	snd_fry;

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;


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
// He did the Mods! He did the Monster Mods.
#define MOD_GLADRAIL 34
#define MOD_SOLDBLAST 35
#define MOD_MEDICHYPER 36
#define MOD_TANKBLAST 37
#define MOD_JORGMACH 38
#define MOD_GUNNERCHAIN 39
#define MOD_INFMELEE 40
#define MOD_FLYSPIKES 41
#define MOD_GLADSPIKE 42
#define MOD_BRAINTENTACLE 43
#define MOD_BRAINCLAWS 44
#define MOD_MUTSLASH 45
#define MOD_BERCLUB 46
#define MOD_BERSPIKE 47
#define MOD_IRONHAND 48
#define MOD_STANKCHAIN 49
#define MOD_INFCHAIN 50
#define MOD_BOSS2CH1 51
#define MOD_BOSS2CH2 52
#define MOD_TANKMACHINEGUN 53
#define MOD_SOLDMACHINEGUN 54
#define MOD_SOLDSHOTGUN 55
#define MOD_LASER 56
#define MOD_GRAPPLE 57

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
extern	cvar_t	*spectator_password;
extern	cvar_t	*needpass;
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
// Paril
// Teamplay Code
extern  cvar_t  *teamplay_classes;
extern  cvar_t  *teamplay_landtype;
// NoHook
extern  cvar_t  *nohook;
// No spawning monsters
extern  cvar_t  *no_spawning_monsters;
//Fallin
extern  cvar_t  *fallingdamage;
extern  cvar_t  *logging;

extern	cvar_t	*stroggflags;
//ZOID
extern	cvar_t	*capturelimit;
extern	cvar_t	*instantweap;

//ZOID
extern	qboolean	is_quad;
//ZOID
//ZOID
/*
==============================

Monster Bans
31 Total
==============================
*/
extern	cvar_t	*ban_soldier;
extern	cvar_t	*ban_enforcer;
extern	cvar_t	*ban_tank;
extern	cvar_t	*ban_ctank;
extern	cvar_t	*ban_medic;
extern	cvar_t	*ban_mediccommander;
extern	cvar_t	*ban_gunner;
extern	cvar_t	*ban_gladiator;
extern	cvar_t	*ban_supertank;
extern	cvar_t	*ban_flyer;
extern	cvar_t	*ban_shark;
extern	cvar_t	*ban_floater;
extern	cvar_t	*ban_berserk;
extern	cvar_t	*ban_ironmaiden;
extern	cvar_t	*ban_boss2;
extern	cvar_t	*ban_carrier;
extern	cvar_t	*ban_widow1;
extern	cvar_t	*ban_widow2;
extern	cvar_t	*ban_brain;
extern	cvar_t	*ban_mutant;
extern	cvar_t	*ban_parasite;
extern	cvar_t	*ban_jorg;
extern	cvar_t	*ban_makron;
extern	cvar_t	*ban_icarus;
extern	cvar_t	*ban_betasoldier;
extern	cvar_t	*ban_betabrain;
extern	cvar_t	*ban_betaironmaiden;
extern	cvar_t	*ban_betasupertank;
extern	cvar_t	*ban_betagladiator;
extern	cvar_t	*ban_gekk;
extern	cvar_t	*ban_daedalus;
extern	cvar_t	*ban_stalker;
extern	cvar_t	*ban_fixbot;
extern	cvar_t  *ban_stormt;
extern	cvar_t  *ban_deathtank;
extern	cvar_t  *ban_lasergladiator;
extern	cvar_t  *ban_flyerhornet;
extern	cvar_t  *ban_traitorm;
extern	cvar_t  *ban_traitorf;
extern	cvar_t  *ban_hypertank;
// Health limits!
// 31 Total
extern	cvar_t	*soldier_health;
extern	cvar_t	*enforcer_health;
extern	cvar_t	*tank_health;
extern	cvar_t	*ctank_health;
extern	cvar_t	*medic_health;
extern	cvar_t	*mediccommander_health;
extern	cvar_t	*gunner_health;
extern	cvar_t	*gladiator_health;
extern	cvar_t	*supertank_health;
extern	cvar_t	*flyer_health;
extern	cvar_t	*shark_health;
extern	cvar_t	*floater_health;
extern	cvar_t	*berserk_health;
extern	cvar_t	*ironmaiden_health;
extern	cvar_t	*boss2_health;
extern	cvar_t	*carrier_health;
extern	cvar_t	*widow1_health;
extern	cvar_t	*widow2_health;
extern	cvar_t	*brain_health;
extern	cvar_t	*mutant_health;
extern	cvar_t	*parasite_health;
extern	cvar_t	*jorg_health;
extern	cvar_t	*makron_health;
extern	cvar_t	*icarus_health;
extern	cvar_t	*betasoldier_health;
extern	cvar_t	*betabrain_health;
extern	cvar_t	*betaironmaiden_health;
extern	cvar_t	*betasupertank_health;
extern	cvar_t	*betagladiator_health;
extern	cvar_t	*gekk_health;
extern	cvar_t	*daedalus_health;
extern	cvar_t	*stalker_health;
extern	cvar_t	*fixbot_health;
extern	cvar_t  *stormt_health;
extern	cvar_t  *deathtank_health;
extern	cvar_t  *lasergladiator_health;
extern	cvar_t  *flyerhornet_health;
extern	cvar_t  *traitorm_health;
extern	cvar_t  *traitorf_health;
extern	cvar_t  *hypertank_health;

extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;


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
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
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
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect

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
void ThrowClientHead(edict_t *self);
void ThrowGib(edict_t *self,char *gibname);
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

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin, edict_t *owner);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_proxgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_fire (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);

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
void G_SetSpectatorStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);

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
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);

//
// g_chase.c
//
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6


enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s {
	struct pmenu_s *entries;
	int cur;
	int num;
	void *arg;
} pmenuhnd_t;

typedef void (*SelectFunc_t)(edict_t *ent, pmenuhnd_t *hnd);

typedef struct pmenu_s {
	char *text;
	int align;
	SelectFunc_t SelectFunc;
} pmenu_t;


// Paril
// Abilities
typedef struct
{
	// Points
	int powerpoints;
	// Classy
	int soldier_haste;
	int tank_dualrockets;
	int gunner_forcegrenades;
	int gladiator_wallpierce;
	int floater_fragmulti;
	int berserk_shield;
	int chick_triplespread;
	int brain_proxies;
	int mutant_nightvision;
	int icarus_explobolts;
	int soldierb_speed;
	int bbrain_tentacles;
	int dchick_plasma;
	int dsupertank_ir;
	int daed_ir;
	int stalker_homing;
	int flyer_kaboom;
} client_abilities_t;

//Paril, states
typedef enum
{
	BOT_STAND,  // Standing
	BOT_ROAM,	// Running, seeking
	BOT_HUNT,	// Running, hunting your ass down
	BOT_ENEMY,	// Has an enemy, is shooting
	BOT_STRAFING // Strafing
} bot_state_t; // Made this for states
// Paril

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

	int stalker_ceiling;

	client_abilities_t abilities;

	int skin_pain;
	int speedclass;

	// Monsters
	edict_t *monster1;
	edict_t *monster2;
	edict_t *monster3;
	edict_t *monster4;
	edict_t *monster5;
	edict_t *monster6;
	// Selected
	edict_t *selected;

	// Paril
	int bot_state;
	float end_strafe;
	int weapontype;
	// Paril

	int streak;
} client_persistant_t;

// Paril, Profile
typedef struct
{
	char *age;
} client_profile_t;

typedef struct
{
	int has_party;
	char *name;
	char *owner;
	edict_t *member[9];
} party_s;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	qboolean	spectator;			// client is a spectator

	int			class;   // added for the class variable
		// MUCE: added for jetpack thrusting.
	qboolean        thrusting;              // 1 on 0 off
	float           next_thrust_sound;
	qboolean		tread;
	int class_speed;
	float           nextfootstep;
	float			nextidlesound;
	int mutant_jumped;
	float nextdrown;

	// Idle/Walk animation stuff
	float nexttankidle;
	float nexttankstep;

	// Paril 
	// Teamplay Code
	int landtype; // 0 is Air, 1 is Ground. Shark = Air.


	int spawned_monsters;
	float next_weld_sound;
	int walking;
	int bloody;

	float nextturn;
	client_profile_t profile;

//ZOID
	int			ctf_team;			// CTF team
	int			ctf_state;
	float		ctf_lasthurtcarrier;
	float		ctf_lastreturnedflag;
	float		ctf_flagsince;
	float		ctf_lastfraggedcarrier;
	qboolean	id_state;
	float		lastidtime;
	qboolean	voted; // for elections
	qboolean	ready;
	qboolean	admin;
	struct ghost_s *ghost; // for ghost codes
//ZOID
	int lvl;
	int exp;

	char password[64]; // Make sure this stays in sync.
	int loaded; // Has he loaded yet? Used for PutClientInServer.

	party_s party;
	int in_party;
	party_s current_party;
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

//ZOID
	qboolean	inmenu;				// in menu
	pmenuhnd_t	*menu;				// current menu
    pmenu_t         current_menu[1536]; // Dirty
//ZOID
    pmenu_t         *devmenu; //Paril Devmenu

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

	int max_speed; // Stuff by Speed

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;
	qboolean	anim_walk;

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
	 // CCH: new variables for airstrikes
 int            airstrike_called;
 vec3_t         airstrike_entry;
 float          airstrike_time;
	 // CCH: new variables for airstrikes
 int            lightn_called;
 vec3_t         lightn_entry;
 float          lightn_time;


	qboolean		tread;

   /*ATTILA begin*/
   float Jet_framenum;   /*burn out time when jet is activated*/
   float Jet_remaining;  /*remaining fuel time*/
   float Jet_next_think; 
   /*ATTILA end*/

//ZOID
	void		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_regentime;		// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
	edict_t		*chase_target;
	qboolean	update_chase;
	float		menutime;			// time to update menu
	qboolean	menudirty;
//ZOID
	float		tracker_pain_framenum;


    //Grapple Variables
    edict_t        *hook;        
    edict_t        *hook_touch;  
    qboolean    on_hook;      
    int             hook_frame;
	// new hook
int hookstate; // Hook states flags 
int hooktype; // 1=CableGrapple, 2=LaserGrapple 

int num_hooks;

int cammaxdistance;
//Wyrm: chasecam
        int             chasetoggle;
        edict_t         *chasecam;
        edict_t         *oldplayer;

		int playing_dead;
  /* WonderSlug --Added For Kamikaze Mode */

  int          kamikaze_mode;

  float        kamikaze_framenum;

  float        kamikaze_timeleft;

  /* WonderSlug End */
  int sentry;

  edict_t *sentry_base;
  edict_t *sentry_sent;

  char entered_password[64]; // What did he enter? Paril. Used to check with resp.password!
  int asking_for_pass;
};

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
        // add for advanced thinking:
        float          nextthink2;
        void           (*think2)(edict_t *self);
        // end of add



	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;

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
  int mtype;
  int ismonster;

  //char *teamname; // Monster teams!
  edict_t *team_owner; // We'll do it this way..

  int monstermap;

//=========
//ROGUE
	int			plat2flags;
	vec3_t		offset;
	vec3_t		gravityVector;
	edict_t		*bad_area;
	edict_t		*hint_chain;
	edict_t		*monster_hint_chain;
	edict_t		*target_hint_chain;
	int			hint_chain_id;
	// FIXME - debug help!
	float		lastMoveTime;
//ROGUE
//=========

//Wyrm: chasecam
        int             chasedist1;
        int             chasedist2;

	edict_t *other;
	edict_t *head;

	// Paril
	qboolean is_bot;
	// Paril

	// freezer code
	char oldskin[60];
	qboolean frozen;
	qboolean frozenbody;
	int frozentime;
	//WF ACRID 3/99

    edict_t                    *sentry_base;
    edict_t                    *sentry_weapon;

	int sent_weapon;

	int need_to_set;
	int lvl;
	int exp;
	int score;
};

void G_Spawn_Explosion (int type, vec3_t start, vec3_t origin);


FILE *logged;

// Paril, global stuff
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void safe_bprintf (int printlevel, char *fmt, ...);
void safe_centerprintf (edict_t *ent, char *fmt, ...);
// Paril
void Bot_Spawn (edict_t *ent);	// Define it
void Bot_Create_AssignTeam (void);
void Bot_Create_CTF (int team);
void Bot_Create (void);			// Needs header file!
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles);
void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage);
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles);
edict_t *Bot_FindEnemy(edict_t *ent);
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other);
float ACEIT_ItemNeed(edict_t *self, int item);
int ACEIT_ClassnameToIndex(char *classname);
qboolean ACEIT_IsReachable(edict_t *self, vec3_t goal);
void ACEAI_PickShortRangeGoal(edict_t *self, usercmd_t *cmd);
qboolean Bot_CanMove (edict_t *self);
void Bot_ChangeWeapon_DontUseBlaster (edict_t *ent);
void Bot_ChangeWeapon_Accordingly (edict_t *ent, int change_event);

qboolean ClientConnect (edict_t *ent, char *userinfo); // Define aswell
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles); // Define
void ClientThink (edict_t *ent, usercmd_t *ucmd);
void CopyToBodyQue (edict_t *ent);
void ClientDisconnect (edict_t *ent);
void Svcmd_Bot_f(void);

void PMenu_CloseIt(edict_t *ent);
void OpenClassMenu (edict_t *ent, pmenuhnd_t *p);
void BeSoldier (edict_t *ent, pmenuhnd_t *p);
void BeEnforcer (edict_t *ent, pmenuhnd_t *p);
void BeTank (edict_t *ent, pmenuhnd_t *p);
void BeMedic (edict_t *ent, pmenuhnd_t *p);
void BeGunner (edict_t *ent, pmenuhnd_t *p);
void BeGladiator (edict_t *ent, pmenuhnd_t *p);
void BeSupertank (edict_t *ent, pmenuhnd_t *p);
void BeFlyer (edict_t *ent, pmenuhnd_t *p);
void BeBerserk (edict_t *ent, pmenuhnd_t *p);
void BeIcarus (edict_t *ent, pmenuhnd_t *p);
void BeChick (edict_t *ent, pmenuhnd_t *p);
void OpenGroundMenu (edict_t *ent, pmenuhnd_t *p);
void OpenFlyMenu (edict_t *ent, pmenuhnd_t *p);
void GoBack (edict_t *ent, pmenuhnd_t *p);
void EndObserverMode_Walking(edict_t* ent);
void EndObserverMode_Flying(edict_t* ent);
void EndObserverMode_Tread(edict_t* ent);
void GoToGround2Menu (edict_t *ent, pmenuhnd_t *p);
void BeBoss2 (edict_t *ent, pmenuhnd_t *p);
void BeBrain (edict_t *ent, pmenuhnd_t *p);
void BeMutant (edict_t *ent, pmenuhnd_t *p);
void BeJorg (edict_t *ent, pmenuhnd_t *p);
void BeMakron (edict_t *ent, pmenuhnd_t *p);
void BeFloater (edict_t *ent, pmenuhnd_t *p);
void BeShark (edict_t *ent, pmenuhnd_t *p);
void OpenSwimMenu (edict_t *ent, pmenuhnd_t *p);
void BeParasite (edict_t *ent, pmenuhnd_t *p);
void BeBETASoldier (edict_t *ent, pmenuhnd_t *p);
void GoToGround3Menu (edict_t *ent, pmenuhnd_t *p);
void GoToGround4Menu (edict_t *ent, pmenuhnd_t *p);
void BeBETAGlad (edict_t *ent, pmenuhnd_t *p);
void BeBETABrain (edict_t *ent, pmenuhnd_t *p);
void BeBETAMaiden (edict_t *ent, pmenuhnd_t *p);
void BeBETASupertank (edict_t *ent, pmenuhnd_t *p);
void BeDaedalus (edict_t *ent, pmenuhnd_t *p);
void BeGekk (edict_t *ent, pmenuhnd_t *p);
void BeStalker (edict_t *ent, pmenuhnd_t *p);
void BeMedicCommander (edict_t *ent, pmenuhnd_t *p);
void BeCarrier (edict_t *ent, pmenuhnd_t *p);
void BeBlackWidow (edict_t *ent, pmenuhnd_t *p);
void BeBlackWidow2 (edict_t *ent, pmenuhnd_t *p);
void BeFixbot (edict_t *ent, pmenuhnd_t *p);
void BeCTank (edict_t *ent, pmenuhnd_t *p);
void BeDeathTank (edict_t *ent, pmenuhnd_t *p);
void BeLaserGladiator (edict_t *ent, pmenuhnd_t *p);
void BeFlyerHornet (edict_t *ent, pmenuhnd_t *p);
void BeSoldierStorm (edict_t *ent, pmenuhnd_t *p);
void OpenGroundTRMenuGround (edict_t *ent, pmenuhnd_t *p);
void OpenGroundGZMenuGround (edict_t *ent, pmenuhnd_t *p);
void OpenGroundQ2MenuGround (edict_t *ent, pmenuhnd_t *p);
void OpenGroundTRMenuFly (edict_t *ent, pmenuhnd_t *p);
void OpenGroundGZMenuFly (edict_t *ent, pmenuhnd_t *p);
void OpenGroundQ2MenuFly (edict_t *ent, pmenuhnd_t *p);
void OpenGroundRoseMenu (edict_t *ent, pmenuhnd_t *p);
void OpenFlyRoseMenu (edict_t *ent, pmenuhnd_t *p);
void CloseThisMenu (edict_t *ent, pmenuhnd_t *p);
void BeMaleTraitor (edict_t *ent, pmenuhnd_t *p);
void BeFemaleTraitor (edict_t *ent, pmenuhnd_t *p);
void BeHypertank (edict_t *ent, pmenuhnd_t *p);
pmenuhnd_t *PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, void *arg);


//ZOID
#include "g_ctf.h"
//ZOID

void Sentry_FaceEnemy(edict_t *self);
void Sentry_Seek(edict_t *self);
void Create_Sentry (edict_t *ent);
void Base_Explode(edict_t *base);
void sentry_fire(edict_t * sentry);
void Base_Die(edict_t *base, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);



// Monsters on map
int monsters_in_map;

void Setup_Random_Monsters(void);
void SpawnMonsters(edict_t *ent);
void ReSpawnMonster(edict_t *self);
void Cmd_DevMenu_f(char *scommand, edict_t *ent);

void Check_Levelup (edict_t *ent);


int modelindexes;
char *modelindexes_l[255];

#include "sdm_rpg_playerdata.h"

// ANYTHING YOU WISH TO SAVE MUST ALSO BE ADDED TO THE BELOW STRUCTURE
struct player_save
{
	//MISC
	int version;
	// PASSWORD
	char password[64];
	int player_password;
	// EXPERIENCE
	int health;
	int score; 
	int exp;
	int lvl;
	int class;
} DAT; 