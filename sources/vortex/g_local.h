// g_local.h -- local definitions for game module
#ifndef G_LOCAL
#define G_LOCAL

//Uncomment this and recompile to get debug printouts.
//The higher number, the more detailed printouts.
//Using this , especially on higher levels, is very lag prone and may cause server overflow.
//#define PRINT_DEBUGINFO 1

#include "q_shared.h"
// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

//ZOID
#include "p_menu.h"
//ZOID

#include "v_shared.h"	//3.0
#include "Spirit.h"		// 3.03+
#include "morph.h"
#include "ally.h" // 3.12
#include "gds.h" // 3.15
#include "scanner.h"

#define FIXED_FT

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"Vortex"//K03 "baseq2"
//#define MAX_NODES	1024
//K03 Begin

//typedef struct nodedata_s nodedata_t;
/*
struct nodedata_s
{
	vec3_t		pos;			// position of the node
	nodedata_t	*next;			// pointer to next node
	nodedata_t	*prev;			// pointer to previous node
	int			type;			// the type of node
	int			flags;			// special node flags
	float		weight;			// weight modifier
};
*/

extern qboolean MonstersInUse;
extern qboolean found_flag;
extern int total_monsters;
extern edict_t *SPREE_DUDE;
extern edict_t *red_base;
extern edict_t *blue_base;
extern int red_flag_caps;
extern int blue_flag_caps;
extern qboolean SPREE_WAR;
extern qboolean INVASION_OTHERSPAWNS_REMOVED;
extern int invasion_difficulty_level;
extern float SPREE_TIME;
extern int average_player_level;
extern int pvm_average_level;
extern int DEFENSE_TEAM;
extern int PREV_DEFENSE_TEAM;
extern long FLAG_FRAMES;
//extern vec3_t nodes[MAX_NODES];//GHz
#include "p_hook.h"
//K03 End

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define	svc_stufftext		11
#define	svc_configstring	13
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
#define	SPAWNFLAG_INVASION_ONLY		0x00001000
//#define SPAWNFLAG_BOMBS				0x00002000 //GHz

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
#define FL_CHATPROTECT			0x00002000	// in chat-protect mode
#define FL_CHASEABLE			0x00004000	// 3.65 indicates non-client ent can be chased
#define FL_CLIGHTNING			0x00008000	// 4.0 indicates entity has already been targetted by CL this frame
#define FL_WORMHOLE				0x00010000	// player is in a wormhole
#define FL_DETECTED				0x00020000	// player was detected
#define FL_CONVERTED			0x00040000	// entity was converted
#define FL_COCOONED				0x00080000	// entitiy is cocooned
#define FL_RESPAWN				0x80000000	// used for item respawning


#ifdef FIXED_FT
#define	FRAMETIME		0.1
#define ft(x)			x
#else
#define FRAMETIME		(1/sv_fps->value)
#define ft(x)			(int)(x*10/sv_fps->value)
#endif

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8 // max number of corpses

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
	AMMO_BULLETS = 1,
	AMMO_SHELLS = 2,
	AMMO_ROCKETS = 3,
	AMMO_GRENADES = 4,  
	AMMO_CELLS = 5, 
	AMMO_SLUGS = 6, 
    // RAFAEL 
	AMMO_MAGSLUG = 7, 
	AMMO_TRAP = 8, 
	// 3.5
	AMMO_GENERATOR = 9
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
//#define AI_TEMP_STAND_GROUND	0x00000002
//#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
//#define AI_PURSUE_NEXT			0x00000020
//#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_IGNORE_NAVI			0x00004000
#define AI_NO_CIRCLE_STRAFE		0x00008000
#define AI_FIND_NAVI			0x00010000
//#define AI_PURSUE_LOWER_GOAL	0x00020000
#define AI_PURSUE_PLAT_GOAL		0x00040000
#define AI_DODGE				0x00080000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

//4.5 monster bonus flags
#define BF_CHAMPION				0x00000001
#define BF_GHOSTLY				0x00000002
#define BF_FANATICAL			0x00000004
#define BF_BERSERKER			0x00000008
#define BF_POSESSED				0x00000010
#define BF_STYGIAN				0x00000020
#define BF_UNIQUE_FIRE			0x00000040
#define BF_UNIQUE_LIGHTNING		0x00000080

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


//3ZB CTF state

#define GETTER		0
#define	DEFENDER	1
#define	SUPPORTER	2
#define	CARRIER		3

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
// RAFAEL
	MOVETYPE_WALLBOUNCE,
	MOVETYPE_SLIDE
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
#define IT_FLAG			128
#define IT_HEALTH		256

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
#define WEAP_PHALANX			12

#define WEAP_BOOMER				13

#define WEAP_DISRUPTOR			12		// PGM
#define WEAP_ETFRIFLE			13		// PGM
#define WEAP_PLASMA				14		// PGM
#define WEAP_PROXLAUNCH			15		// PGM
#define WEAP_CHAINFIST			16		// PGM

#define WEAP_TRAP				17

#define WEAP_GRAPPLE			20

#define WEAP_TOTAL				21

#define MPI_QUAD				21
#define	MPI_PENTA				22
#define MPI_QUADF				23

#define MPI_INDEX				24	//MPI count

#define WEAP_SWORD				25//K03
#define WEAP_20MM				26//GHz

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

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
	int			weapmodel;		// weapon model index (for weapons)
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
	int			real_framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

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

	int			num_bots;
	int			r_monsters;			//4.5 recommended monster value for this map
	qboolean	daytime; //GHz: Is the sun going up or down?
	qboolean    modechange;
	qboolean	pathfinding;
/*	gdsfiles_t	gdsfiles[MAX_CLIENTS];*/

	// experimental monster pathfinding
	//int			total_nodes;
	//nodedata_t	nodes[MAX_NODES];
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
	float		weight;
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;			//BFGのターゲットポイントに不正使用
	vec3_t		end_angles;

	int			sound_start;		//スナイパーのアクティベートフラグ
	int			sound_middle;
	int			sound_end;			//hokutoのクラス

	float		accel;
	float		speed;				//bot 落下時の移動量に不正使用
	float		decel;				//水面滞在時間に不正使用
	float		distance;			//スナイパー用FOV値

	float		wait;

	// state data
	int			state;				//CTFステータスに不正使用
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

//GHz START
typedef struct dmglist_s
{
	edict_t		*player;	// attacker who hurt us
	float		damage;		// total damage done
}dmglist_t;
//GHz END

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self); // called when monster is doing nothing
//	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *attacker, vec3_t dir, int radius);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other); // called when monster acquires a target
//	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;
	float		melee_finished;

//	vec3_t		saved_goal;
	int			search_frames; // number of frames enemy has not been visible
	int			stuck_frames;	// number of frames monster has been stuck in-place
	vec3_t		stuck_org;		// location used for comparison to determine if monster is stuck
	float		selected_time; // time monster flashes after being selected
	float		teleport_delay; // time before drone can teleport again
	float		trail_time;
	vec3_t		last_sighting; // last known position of enemy
//	int			attack_state;
	int			lefty;
	float		idle_delay; // how often idle func is called
	int			idle_frames; // number of frames monster has been idle
	int			air_frames;	// how many frames monster has been off the ground
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;
	int			max_armor;
	int			control_cost;
	int			cost;
	int			level;				// used to determine monster toughness
	int			jumpup;				// max height we can jump up
	int			jumpdn;				// max height we can jump down
	int			radius;				// radius (if any) if projectile explosion
	int			regen_delay1;		// level.framenum when we can regenerate again
	int			regen_delay2;		// secondary regen level.framenum when we can regenerate again
	int			regen_delay3;		// secondary regen level.framenum when we can regenerate again
	int			upkeep_delay;		// frames before upkeep is checked again
	int			inv_framenum;		// frame when quad+invuln wears out, used to prevent spawn camping in invasion mode
	int			nextattack;			// used for mutant to check for next attack frame
	int			bonus_flags;		//4.5 used for special bonus flags (e.g. champion, unique monster bonuses)
	int			waypoint[1000];		// pathfinding node indices leading to final destination
	int			nextWaypoint;		// next waypoint index to follow
	int			numWaypoints;		// total number of waypoints
	edict_t		*attacker;			// edict that triggered our dodge routines
	edict_t		*leader;			// edict that we have been commanded to follow
	vec3_t		spot1;				// position we have been commanded to move to/defend
	vec3_t		spot2;				// position we have been commanded to move to/defend
	vec3_t		dir;				// direction of oncoming projectile
	float		eta;				// time projectile will impact
	float		dodge_time;			// delay before we can dodge again
	float		sight_range;		// 3.56 how far the drone can see to acquire targets
	float		bump_delay;			// delay before we can make another course-correction
	float		resurrected_time;	// time when resurrection from a medic is complete
	float		path_time;			// time when monster can compute a path
	edict_t		*lastGoal;			// last goal we were chasing, used for deciding when to re-compute paths
//	qboolean	melee;				// whether or not the monster should circle strafe
	dmglist_t	dmglist[MAX_CLIENTS];		// keep track of damage by players
	qboolean	slots_freed;		// true if player slots have been refunded prior to removal
} monsterinfo_t;

extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern int	sm_meat_index;
extern int	snd_fry;

extern int	jacket_armor_index; 
extern int	combat_armor_index; 
extern int	body_armor_index;  
extern int	power_cube_index;  
extern int	flag_index;  
extern int	red_flag_index;
extern int	blue_flag_index;
extern int  halo_index;
extern int	resistance_index;
extern int	strength_index;
extern int	regeneration_index;
extern int	haste_index;
  
//ammo  
extern int	bullet_index;  
extern int	shell_index;  
extern int	grenade_index;  
extern int	rocket_index;  
extern int	slug_index;  
extern int	cell_index;

//pre searched items
gitem_t	*Fdi_GRAPPLE;
gitem_t	*Fdi_BLASTER;
gitem_t *Fdi_SHOTGUN;
gitem_t *Fdi_SUPERSHOTGUN;
gitem_t *Fdi_MACHINEGUN;
gitem_t *Fdi_CHAINGUN;
gitem_t *Fdi_GRENADES;
gitem_t *Fdi_GRENADELAUNCHER;
gitem_t *Fdi_ROCKETLAUNCHER;
gitem_t *Fdi_HYPERBLASTER;
gitem_t *Fdi_RAILGUN;
gitem_t *Fdi_BFG;
gitem_t *Fdi_PHALANX;
gitem_t *Fdi_BOOMER;
gitem_t *Fdi_TRAP;
gitem_t *Fdi_20MM;

gitem_t *Fdi_SHELLS;
gitem_t *Fdi_BULLETS;
gitem_t *Fdi_CELLS;
gitem_t *Fdi_ROCKETS;
gitem_t *Fdi_SLUGS;
gitem_t *Fdi_MAGSLUGS;
gitem_t *Fdi_TBALL;
gitem_t	*Fdi_POWERCUBE;

int headindex;
int	skullindex;

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
#define MOD_GRAPPLE			34
// RAFAEL 14-APR-98
#define MOD_RIPPER			35
#define MOD_PHALANX			36
#define MOD_BRAINTENTACLE	37
#define MOD_BLASTOFF		38
#define MOD_GEKK			39
#define MOD_TRAP			40
// END 14-APR-98
//K03 Begin
#define MOD_BURN			41
#define MOD_LIGHTNING		42
#define MOD_SWORD			43
#define MOD_LASER_DEFENSE	44
#define MOD_SENTRY			45
#define MOD_SENTRY_ROCKET	46
#define MOD_CORPSEEXPLODE	47
#define MOD_DECOY			48
#define MOD_BOMBS			49
#define MOD_CANNON			50
#define MOD_EXPLODINGARMOR	51
#define MOD_PARASITE		52
#define MOD_SNIPER			53
#define MOD_SUPPLYSTATION	54
#define MOD_CACODEMON_FIREBALL	55
#define MOD_PLAGUE			56
#define MOD_SKULL			57
#define MOD_CORPSEEATER		58
#define MOD_TENTACLE		59
#define MOD_BEAM			60
#define MOD_MAGICBOLT		61
#define MOD_CRIPPLE			62
#define MOD_NOVA			63
#define MOD_IRON_MAIDEN		64
//#define MOD_EXPLODING_DECOY	65	//4.0
// player boss
#define MOD_MAKRON_CHAINGUN	66
#define MOD_MAKRON_BFG		67
#define MOD_MAKRON_CRUSH	68
#define MOD_MUTANT			69
#define MOD_SPIKE			70
#define MOD_MINDABSORB		71
#define MOD_LIFE_DRAIN		72
#define MOD_PROXY			73
#define MOD_NAPALM			74
#define MOD_TANK_PUNCH		75
#define MOD_FREEZE			76	//4.0
#define MOD_TANK_BULLET		77
#define MOD_METEOR			78
#define MOD_AUTOCANNON		79
#define MOD_HAMMER			80
#define MOD_BLACK_HOLE		81
#define MOD_FIRETOTEM		82	//4.1
#define MOD_WATERTOTEM		83	//4.1
#define MOD_BERSERK_PUNCH	84
#define MOD_BERSERK_SLASH	85
#define MOD_BERSERK_CRUSH	86
#define MOD_BLEED			87
#define MOD_CALTROPS		88
#define MOD_SPIKEGRENADE	89
#define MOD_EMP				90
#define MOD_FIREBALL		91
#define MOD_PLASMABOLT		92
#define MOD_LIGHTNING_STORM	93
#define MOD_MIRV			94
#define MOD_OBSTACLE		95
#define MOD_GAS				96
#define MOD_SPORE			97
#define MOD_ACID			98
#define MOD_ICEBOLT			99//4.4
#define MOD_UNHOLYGROUND	100//4.4
#define MOD_SELFDESTRUCT	101
//K03 End
#define MOD_FRIENDLY_FIRE	0x8000000
#define MOD_FMEDICPACK		150

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
//ZOID
extern	cvar_t	*capturelimit;
//ZOID
extern	cvar_t	*password;
extern	cvar_t	*spectator_password;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

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
extern	cvar_t	*maxspectators;

extern	cvar_t	*filterban;

//ponpoko
extern	cvar_t	*gamepath;
extern	cvar_t	*chedit;
extern	cvar_t	*vwep;
extern	float	spawncycle;
//ponpoko

//K03 Begin

extern	cvar_t	*killboxspawn;
extern cvar_t *save_path;
extern cvar_t *particles;
extern cvar_t *sentry_lev1_model;
extern cvar_t *sentry_lev2_model;
extern cvar_t *sentry_lev3_model;

extern cvar_t *start_level;
extern cvar_t *start_nextlevel;
extern cvar_t *nextlevel_mult;

extern cvar_t *vrx_creditmult;
extern cvar_t *vrx_pointmult;


extern cvar_t *sv_maplist;
extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;
extern cvar_t *gamedir;
extern cvar_t *dm_monsters;
extern cvar_t *reconnect_ip;
extern cvar_t *invasion_enabled;

extern cvar_t *vrx_password;
extern cvar_t *min_level;
extern cvar_t *max_level;
extern cvar_t *check_dupeip;
extern cvar_t *check_dupename;
extern cvar_t *newbie_protection;
extern cvar_t *debuginfo;
extern cvar_t *pvm;
extern cvar_t *hw;
extern cvar_t *trading;
extern cvar_t *tradingmode_enabled;
extern cvar_t *ptr;
extern cvar_t *domination;
extern cvar_t *ctf;
extern cvar_t *ffa;
extern cvar_t *invasion;
extern cvar_t *nolag;
extern cvar_t *pvm_respawntime;
extern cvar_t *pvm_monstermult;
extern cvar_t *ffa_respawntime;
extern cvar_t *ffa_monstermult;
extern cvar_t *server_email;
extern cvar_t *adminpass;
extern cvar_t *team1_skin;
extern cvar_t *team2_skin;
extern cvar_t *enforce_class_skins;
extern cvar_t *class1_skin;
extern cvar_t *class2_skin;
extern cvar_t *class3_skin;
extern cvar_t *class4_skin;
extern cvar_t *class5_skin;
extern cvar_t *class6_skin;
extern cvar_t *class7_skin;
extern cvar_t *class8_skin;
extern cvar_t *class9_skin;
extern cvar_t *class10_skin;
extern cvar_t *class11_skin;
extern cvar_t *class12_skin;
extern cvar_t *class1_model;
extern cvar_t *class2_model;
extern cvar_t *class3_model;
extern cvar_t *class4_model;
extern cvar_t *class5_model;
extern cvar_t *class6_model;
extern cvar_t *class7_model;
extern cvar_t *class8_model;
extern cvar_t *class9_model;
extern cvar_t *class10_model;
extern cvar_t *class11_model;
extern cvar_t *class12_model;
extern cvar_t *ctf_enable_balanced_fc;
extern cvar_t *voting;
extern cvar_t *game_path;
extern cvar_t *allies;
extern cvar_t *pregame_time;
extern cvar_t *world_min_bullets;
extern cvar_t *world_min_cells;
extern cvar_t *world_min_shells;
extern cvar_t *world_min_grenades;
extern cvar_t *world_min_rockets;
extern cvar_t *world_min_slugs;
//K03 End

//ZOID
extern	qboolean	is_quad;
//ZOID

// az begin
extern cvar_t *savemethod;
extern cvar_t *tbi;

extern cvar_t  *sv_fps;

extern cvar_t *vrx_pvppointmult;
extern cvar_t *vrx_pvmpointmult;

extern cvar_t *vrx_sub10mult;
extern cvar_t *vrx_over10mult;

extern cvar_t *adminctrl;
extern cvar_t *generalabmode;

extern cvar_t *vrx_pvpcreditmult;
extern cvar_t *vrx_pvmcreditmult;
// az end

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
void FL_make (edict_t *self);

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
qboolean Add_Ammo (edict_t *ent, gitem_t *item, float count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
edict_t *Spawn_Item (gitem_t *item);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *findclosestreticle (edict_t *prev_ed, edict_t *ent, float rad);
edict_t *findreticle (edict_t *from, edict_t *ent, float range, int degrees, qboolean vis);
edict_t *findclosestradius (edict_t *prev_ed, vec3_t org, float rad);//GHz
edict_t *findclosestradius1 (edict_t *prev_ed, vec3_t org, float rad);//GHz
edict_t *G_FindEntityByMtype (int mtype, edict_t *from);//GHz
float Get2dDistance (vec3_t v1, vec3_t v2);//GHz
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);
void	G_FreeAnyEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);
qboolean G_EntIsAlive (edict_t *ent);//GHz
qboolean G_IsValidLocation (edict_t *ignore, vec3_t point, vec3_t mins, vec3_t maxs);//GHz
qboolean G_IsClearPath (edict_t *ignore, int mask, vec3_t spot1, vec3_t spot2);
qboolean G_ClientExists(edict_t *player);
qboolean visible1 (edict_t *ent1, edict_t *ent2);
qboolean G_CanUseAbilities (edict_t *ent, int ability_lvl, int pc_cost);
qboolean V_CanUseAbilities (edict_t *ent, int ability_index, int ability_cost, qboolean print_msg);
qboolean G_ValidTarget (edict_t *self, edict_t *target, qboolean vis);
qboolean G_ValidTargetEnt (edict_t *target, qboolean alive);
qboolean G_ValidAlliedTarget (edict_t *self, edict_t *target, qboolean vis);//4.1 Archer
edict_t *G_GetClient(edict_t *ent);
qboolean G_GetSpawnLocation (edict_t *ent, float range, vec3_t mins, vec3_t maxs, vec3_t start);
void G_DrawBoundingBox (edict_t *ent);
void G_DrawLaserBBox (edict_t *ent, int laser_color, int laser_size);
void G_DrawLaser (edict_t *ent, vec3_t v1, vec3_t v2, int laser_color, int laser_size);
void G_ResetPlayerState (edict_t *ent); // 3.78
int G_GetNumSummonable (edict_t *ent, char *classname); // 3.9
void G_EntMidPoint (edict_t *ent, vec3_t point);
void G_EntViewPoint (edict_t *ent, vec3_t point); //4.55
qboolean G_ClearShot (edict_t *shooter, vec3_t start, edict_t *target);
float distance (vec3_t p1, vec3_t p2);
void G_RunFrames (edict_t *ent, int start_frame, int end_frame, qboolean reverse);
void AngleCheck (float *val);
int Get_KindWeapon (gitem_t	*it);
void stuffcmd(edict_t *ent, char *s);
void Add_ctfteam_exp (edict_t *ent, int points);
void check_for_levelup (edict_t *ent);
edict_t *FindPlayerByName(char *name);	//4.0 was (const char *name);
edict_t *FindPlayer(char *s);
edict_t *InitMonsterEntity (qboolean manual_spawn);
edict_t *G_GetSummoner (edict_t *ent);
void InitJoinedQueue (void);
void AddJoinedQueue (edict_t *ent);
qboolean TeleportNearArea (edict_t *ent, vec3_t point, int area_size, qboolean air);
qboolean HasActiveCurse (edict_t *ent, int curse_type);
qboolean HasActiveAura (edict_t *ent, int aura_type);
void RemoveCurse (edict_t *ent, edict_t *curse_ent);
edict_t *G_FindCurseByType (edict_t *ent, int curse_type);
edict_t *G_FindAuraByType (edict_t *ent, int aura_type);
qboolean AddCurse (edict_t *owner, edict_t *targ, edict_t *curse_ent, int type, float duration);
void ShowGun(edict_t *ent);
void EndDMLevel (void);
void StartGame (edict_t *ent);
int VortexAddCredits(edict_t *ent, float level_diff, int bonus, qboolean client);
void RemoveAllAuras (edict_t *ent);
void RemoveAllCurses (edict_t *ent);
void PTRInit (void);
void OpenPTRJoinMenu (edict_t *ent);
void AssignTeamSkin (edict_t *ent, char *s);
qboolean HasFlag (edict_t *ent);// 3.7
qboolean ClientCanConnect (edict_t *ent, char *userinfo);
void dom_awardpoints (void);
void dom_dropflag (edict_t *ent, gitem_t *item);
void dom_checkforflag (edict_t *ent);
void dom_fragaward (edict_t *attacker, edict_t *target);
qboolean nearfov (edict_t *ent, edict_t *other, int vertical_degrees, int horizontal_degrees);
void StuffPlayerCmds (edict_t *ent); //3.50
qboolean G_StuffPlayerCmds (edict_t *ent, char *s);
int floattoint (float input); //3.50
void SpawnFlames (edict_t *self, vec3_t start, int num_flames, int damage, int toss_speed); // 3.6
void ThrowFlame (edict_t *ent, vec3_t start, vec3_t forward, float dist, int speed, int damage, int ttl);//4.2
void G_PrintGreenText (char *text); // 3.7
char *HiPrint(char *text);
qboolean G_IsSpectator (edict_t *ent);// 3.7
void G_TeleportNearbyEntities(vec3_t point, float radius, qboolean vis, edict_t *ignore); // 3.7

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);

// RAFAEL
void Weapon_Ionripper (edict_t *ent);
void Weapon_Phalanx (edict_t *ent);
void Weapon_Trap (edict_t *ent);

//K03 Begin
void Weapon_Sword (edict_t *ent);

//
// g_combat.c
//
int OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
int T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, float damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_PIERCING			0x00000040
#define DAMAGE_NO_ABILITIES		0x00000080	// damage is not affected by abilities or armor

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

// blaster projectile types
#define BLASTER_PROJ_BOLT		1
#define BLASTER_PROJ_BLAST		2

//
// g_monster.c
//
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int hspread, int vspread, int count, int flashtype);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int proj_type, float duration, qboolean bounce, int flashtype);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);
void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);
void monster_fire_sword (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype); //Ghz
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
void M_SetEffects (edict_t *ent);
void M_MoveFrame (edict_t *self);
void M_WorldEffects (edict_t *ent);
edict_t *SpawnDrone (edict_t *ent, int drone_type, qboolean worldspawn);

//
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);
void BecomeTE(edict_t *self);//GHz
void BecomeBigExplosion(edict_t *self);//GHz
int HighestLevelPlayer(void);//GHz
int LowestLevelPlayer(void);
int PvMHighestLevelPlayer(void);//GHz
int PvMLowestLevelPlayer(void);
int ActivePlayers (void);//Apple
int vrx_GetMonsterCost(int mtype);//GHz
int vrx_GetMonsterControlCost(int mtype);//GHz
void VortexRemovePlayerSummonables(edict_t *self);//GHz

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
qboolean infov (edict_t *self, edict_t *other, int degrees);

//
// drone_ai.c
//
void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_run1 (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int proj_type, int mod, float duration, qboolean bounce);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int radius_damage);
edict_t *fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int radius_damage, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
// RAFAEL
void fire_ionripper (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_trap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_smartrocket (edict_t *self, edict_t *target, vec3_t start, vec3_t dir, int damage, int speed, int turn_speed, float damage_radius, int radius_damage);

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

// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);

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
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
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

//jabot
#include "ai/ai.h"

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5

// ### Hentai ### BEGIN
#define ANIM_REVERSE	6
// ### Hentai ### END

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
	qboolean	powerArmorActive;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;
	// RAFAEL
	int			max_magslug;
	int			max_trap;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;

	qboolean	spectator;			// client is a spectator

	//K03 Begin
	int			max_powercubes;
	int			max_tballs;
	char		orginal_netname[16];
	char		current_ip[16];
	//4.0 ctf stuff
    float		ctf_assist_frag;	// used to give the player a "kill the flag carrier" assist
	float		ctf_assist_return;	// used to give the player a "return the flag" assist
	int			scanner_active;
	//K03 End
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
//ponko
	int			context;
//ponko
	vec3_t		cmd_angles;			// angles sent over in the last command
	int			game_helpchanged;
	int			helpchanged;

	qboolean	spectator;			// client is a spectator

	//K03 Begin
	int frags;
	//K03 End
	qboolean HasVoted; //GHz
	int voteType; // 1 yes, 2 no, 0 neither
	float VoteTimeout;

	// 3.5 delayed stuffcmd commands
	char	stuffbuf[500];			// commands that will be stuffed to client; delay prevents overflow
	char	*stuffptr;
} client_respawn_t;

//K03 Begin
#include "g_abilities.h"
//K03 End
#include "menu.h"
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
//ZOID
	qboolean	inmenu;				// in menu
	pmenuhnd_t	*menu;				// current menu
//ZOID
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
	int			grenade_delay;
	// RAFAEL
	float		quadfire_framenum;
	int	burst_count;
	float		trap_time;
	
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		respawn_time;		// can respawn when time > this

//ZOID
	void		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_regentime;		// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
	edict_t		*chase_target;
	edict_t		*idtarget;
	qboolean	update_chase;
//ZOID
	//K03 Begin
	float		flood_locktill;		// locked from talking
	float		flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said
	qboolean        thrusting;
	int				thrustdrain;
    float           next_thrust_sound;

	qboolean        cloakable;
	qboolean        cloaking;
	float           cloaktime;
	int             cloakdrain;

	qboolean		boosted;		//Talent: Leap Attack - true if player used boost and is still airbourne
	float healthregen_timee; //lepe
	float healthregen_time;
	float armorregen_time;

	int             hook_state;
	qboolean		firebeam;//GHz
	float			beamtime;//GHz
    edict_t       *hook;
	int				chasecam_mode;

	int  bfg_blend;

	//K03 End
	int			weapon_mode;//GHz
	int			last_weapon_mode;
	int			refire_frames;
	int			idle_frames;		// number of frames player has been standing still and not firing
	int			still_frames;		// number of frames player has been standing still (used for idle kick)
	qboolean	lowlight;
	float		tball_delay;
	float		ability_delay;
	float		disconnect_time;
	float		rune_delay;			// time when we can pick up runes again (to prevent hogging)
	float		snipertime;
	float		oldfov;
	float		oldspeed; // GHz: used for flyer for comparison (impact with object)

	qboolean		trading;		// is player trading?
	qboolean		trade_off;		// is the player blocking trades?
	qboolean		trade_accepted;	// has player accepted trade?
	qboolean		trade_final;	// is the player in the final trade menu?
	edict_t			*menutarget;	// ent stats we are viewing with menu (ent->other is just for clients)
	menusystem_t	menustorage;	// stores menu data

	int			vamp_counter;		// used to track vamped health per second
	int			vamp_frames;		// used for vamp delay

	int			lock_frames;		// how many frames player's cursor has been on lock_target
	edict_t		*lock_target;		// entity player has locked his cursor on

	// v3.12 ally menu stuff
	edict_t		*allytarget;		// player we are trying to ally with
	qboolean	ally_accept;		// have we accepted the alliance?
	qboolean	allying;			// is the player trying to ally with someone?
	float		ally_time;			// when did we begin invitation?

	// 3.5 some abilties don't use power cubes, and instead rely on a charge
	int			charge_index;		// index of ability charge we're showing
	float		charge_time;		// level time the index is reset
	float		charge_regentime;	// time when the ability can begin recharging
	float		menu_delay;			// time before we can cycle thru next menu option (to prevent overflow!)
	float		ammo_regentime;		// next ammo regen tick
	float		wormhole_time;		// must exit wormhole by this time

	qboolean	jump;
	qboolean	show_allyinfo;		// displays ally info data (health/armor bars)

	qboolean	waiting_to_join;	// this player has indicated that they want to join the game (used for teamplay queues)
	float		waiting_time;		// the exact time when the player indicated they wanted to join
	int			showGridDebug;		// show grid debug information (0=off,1=grid,2=children)
	float		lastCommand;		// 'double click' delay for monster commands
	vec3_t		lastPosition;		// last selected position for monster command
	edict_t		*lastEnt;			// last selected entity for monster command
};

qboolean ActiveAura (aura_t *aura, int type);

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
	int			v_flags;			//3.0 New flag variable (so nothing else previously coded is screwed up)

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
//ponko
	edict_t		*union_ent;			//union item
	edict_t		*trainteam;			//train team
	int			arena;				//arena
//ponko
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

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		knockweapon_debounce_time;	// time when knockweapon code can be run again
	float		last_move_time;

	long		health;
	long		max_health;
	int			gib_health;
	int			deadflag;

	float		powerarmor_time;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;
	qboolean exploded; // don't explode more than once at death. lol

	edict_t		*chain;
	edict_t		*prev_chain;
	edict_t		*memchain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	int			noise_index;
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

	// jabot (vrxcl/newvrx)
	ai_handle_t ai; 

	// RAFAEL
	int			orders;

	//K03 Begin
	int			packitems[MAX_ITEMS];
	float		PlasmaDelay;
	float			holdtime;
	qboolean	slow;
	qboolean	superspeed;
	qboolean	sucking;//GHz
	qboolean	antigrav;
	qboolean	automag; // az: magmining self?
	qboolean	manacharging; // az: charging mana?
	int	lockon;

	int FrameShot;
	int Slower;
	skills_t myskills;
	int haste_num;
	int	rocket_shots;

	int shots_hit;
	int shots;
	float lastkill;
	int nfer;
	float lastdmg;
	float lasthurt; // last time we took non-world damage
	int	lastsound; // last frame we made a sound
	int dmg_counter;
	int flipping;              // flipping data
	edict_t *creator;
	//sentry stuff
	edict_t *sentry;
	edict_t *selectedsentry;
	edict_t *standowner;
	float sentrydelay;
	edict_t *lasersight;
	float lasthbshot;
	edict_t *decoy; 
	edict_t *flashlight;
	int mtype; // Type of Monstersee M_* defines.. (M_HOVER, etc)
	int atype;	//3.0 used for new curses
	int num_sentries;
	int num_monsters;
	int num_monsters_real;
	int num_lasers;
	int max_pipes;
	int	num_armor; // 3.5 keep track of armor bombs out
	int num_proxy; // 3.6 keep track of proxy grenades out
	int	num_napalm; // 3.6 keep track of napalm grenades out
	int num_spikegrenades; // number of spike greandes out
	int num_autocannon; //4.1 keep track of live autocannons
	int num_caltrops;//4.2 keep track of live caltrops
	int num_fmedi; // lepi
	int num_box; //lepi
	int num_detectors; // number of live detectors
	int	num_spikers;
	int num_gasser;
	int num_obstacle;

	int	num_spikeball;
	int	num_laserplatforms; //4.4 Talent: Laser Platform
	int num_hammers; //4.4 Talent: Boomerang
	int	health_cache;				// accumulated health that entity will recover
	int health_cache_nextframe;		// the next server frame we will attempt to transfer from health_cache to entity's health
	int armor_cache;
	int	armor_cache_nextframe;
	qboolean	spikeball_follow;
	qboolean	spikeball_recall;
	int shield;			//4.2 shielding value, 0=no shield, 1=front, 2=whole body
	float shield_activate_time; // when shield will activate
	int movetype_prev; // previous movetype, used by V_Push()
	int movetype_frame;	// server frame to restore old movetype

	//K03 End
	edict_t		*selected[4];
	edict_t		*other;
	edict_t		*supplystation;
	edict_t		*magmine;
//GHz START
	// rune stuff
	edict_t		*trade_with;
	item_t		*trade_item[3];
	float		msg_time;
	//3.0 rune stuff
	item_t		vrxitem;
	// teamplay
	int			teamnum;
	float		incontrol_time;
	aura_t		aura[3];
	aura_t		curse[3];
	edict_t		*skull;
	que_t		auras[QUE_MAXSIZE];
	que_t		curses[QUE_MAXSIZE];
	float		corpseeater_time;
	int			parasite_frames;
	edict_t		*parasite_target;
	// 3.03+ spirit
	edict_t		*spirit;
	// 3.12 curse effects
	int			curse_dir;
	int			curse_delay;
	//4.0 "chilled effect"
	int			chill_level;
	float		chill_time;
	//4.0 "manashield"
	qboolean	manashield;
	//4.0
	edict_t		*megahealth;
	edict_t		*spawn;					// available invasion-mode spawn point
	float		holywaterProtection;	//holy water gives a few seconds of curse immunity
	//4.1
	edict_t		*totem1;
	edict_t		*totem2;
	edict_t		*mirror1;
	edict_t		*mirror2;
	edict_t		*healer;
	edict_t		*cocoon;
	edict_t		*holyground;			//Talent: Holy/Unholy Ground
	int			mirroredPosition;
	int			dim_vision_delay;		//Talent: Dim Vision - next server frame that chance trigger is rolled
	float		fury_time;
	float		slowed_factor;
	float		slowed_time;
	float		detected_time;
	float		empeffect_time;
	float		cocoon_time;
	float		cocoon_factor;
	int			showPathDebug;			// show path debug information (0=off,1=on)


#ifndef NO_GDS

#ifndef GDS_NOMULTITHREADING
	/*volatile */int ThreadStatus; // vrxchile 3.0
	/*volatile */int PlayerID;
#endif

#endif

	float		removetime; //4.07 time to auto-remove
	edict_t		*prev_owner; // for conversion
//GHz END
	edict_t *prev_navi;
	edict_t *laser;
};

#include "auras.h"
//ZOID
#include "g_ctf.h"
//ZOID


typedef struct 
{ 
   int  nummaps;          // number of maps in list
   mapdata_t maps[MAX_MAPS];
   char mapnames[MAX_MAPS][MAX_MAPNAME_LEN]; 
   char rotationflag;     // set to ML_ROTATE_* 
   int  currentmap;       // index to current map 
   int  votes[MAX_MAPS];
   qboolean  voteonly[MAX_MAPS];  //True if this map should only be
								  //voted on, but not in rotation
   qboolean warning_given; // Have we given people a warning to vote?
   int	active;				//is map list active?
   int people_voted;
   int sounds[4];
} maplist_t; 
maplist_t		maplist;

//3.0 runes you can buy
typedef struct armoryRune_s
{
	item_t	rune;
} armoryRune_t;

 armoryRune_t WeaponRunes[20];
 armoryRune_t AbilityRunes[20];
 armoryRune_t ComboRunes[20];

//end runes you can buy

//3.0 map lists
v_maplist_t maplist_PVP;
v_maplist_t maplist_DOM;
v_maplist_t maplist_PVM;
v_maplist_t	maplist_CTF;
v_maplist_t maplist_FFA;
v_maplist_t maplist_INV;
v_maplist_t maplist_TRA;
v_maplist_t maplist_INH;
v_maplist_t maplist_VHW;
v_maplist_t maplist_TBI;
//end new map lists

// teamplay stuff
typedef struct
{
	char	name [16];
	int		team;
	float	time;
}joined_t;
joined_t	players[MAX_CLIENTS];
joined_t *GetJoinedSlot (edict_t *ent);
void ClearJoinedSlot (joined_t *slot);

int GetRandom(int min,int max);
void stuffcmd(edict_t *e, char *s);
void ApplyThrust (edict_t *ent);
int OpenConfigFile(edict_t *ent);
void WriteMyCharacter(edict_t *ent);
void modify_max(edict_t *ent);
void modify_health(edict_t *ent);
void SaveCharacter (edict_t *ent);
void WriteToLogfile (edict_t *ent, char *s);
void WriteToLogFile (char *char_name, char *s) ;
void WriteServerMsg (char *s, char *error_string, qboolean print_msg, qboolean save_to_logfile);

void cloak(edict_t *ent); 
qboolean G_EntExists(edict_t *ent);
void TossClientBackpack(edict_t *player, edict_t *attacker);
int MAX_ARMOR(edict_t *ent);
int MAX_HEALTH(edict_t *ent);
int MAX_POWERCUBES(edict_t *ent);
int MAX_BULLETS(edict_t *ent);
int MAX_SHELLS(edict_t *ent);
int MAX_ROCKETS(edict_t *ent);
int MAX_GRENADES(edict_t *ent);
int MAX_CELLS(edict_t *ent);
int MAX_POWERCUBES(edict_t *ent);
int MAX_SLUGS(edict_t *ent);

qboolean StartClient(edict_t *ent);
void ChaseCam(edict_t *ent);
void JoinTheGame(edict_t *ent);
void OpenJoinMenu(edict_t *ent);
void OpenCombatMenu (edict_t *ent, int lastline);//4.5
qboolean StartClient(edict_t *ent);
void OpenJoinMenu(edict_t *ent);
int config_map_list();
void ClearMapVotes();
int MapMaxVotes();
void VoteForMap(int i);
void DumpMapVotes();
void ClearMapList();
void DisplayMaplistUsage(edict_t *ent);
void MaplistNextMap(edict_t *ent);
void Cmd_Maplist_f (edict_t *ent);
char *TeamName (edict_t *ent);

void KickPlayerBack(edict_t *ent);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void GetScorePosition ();
void Give_respawnweapon(edict_t *ent, int weaponID);
void Give_respawnitems(edict_t *ent);
qboolean findspawnpoint (edict_t *ent);
void Pick_respawnweapon(edict_t *ent);

#define for_each_player(JOE_BLOGGS,INDEX)				\
for(INDEX=1;INDEX<=maxclients->value;INDEX++)			\
	if ((JOE_BLOGGS=&g_edicts[i]) && JOE_BLOGGS->inuse && JOE_BLOGGS->client)
int total_players();
void Cmd_CreateBreather_f(edict_t *ent);
void Cmd_CreateEnviro_f(edict_t *ent);
void Cmd_CreateInvin_f(edict_t *ent);
void Cmd_CreateQuad_f(edict_t *ent);
void Cmd_TeleportPlayer(edict_t *ent);
void Teleport_them(edict_t *ent);
void Cmd_CreateHealth_f(edict_t *ent);
void Check_full(edict_t *ent);
void Cmd_AmmoStealer_f(edict_t *ent);
void MonsterAim(edict_t *self, float accuracy, int projectile_speed, qboolean rocket, int flash_number, vec3_t forward, vec3_t start);
float entdist(edict_t *ent1, edict_t *ent2);
void Cmd_Upgrade_Weapon(edict_t *ent, int weapon);
void Cmd_Upgrade_Ability(edict_t *ent, int ability);
void burn_person(edict_t *target, edict_t *owner, int damage);
void Remove_Player_Flames (edict_t *ent);
void Remove_Flame(edict_t *ent);
void Fire_Think (edict_t *self);
void freeze_player(edict_t *ent);
void laser_start (edict_t *ent, vec3_t start, vec3_t aimdir, int	color, float laserend, float delete_time, int width);
void hook_laser_think (edict_t *self);
void Cmd_Salvation(edict_t *ent);
void Cmd_Redemption(edict_t *ent);
void Cmd_BloodSuckerPlayer(edict_t *ent);
void Cmd_BoostPlayer(edict_t *ent);
void Cmd_RaySpell(edict_t *ent);
void Cmd_SlowPlayer(edict_t *ent);
void Cmd_BurnPlayer(edict_t *ent);
void Cmd_Drone_f (edict_t *ent);
void Cmd_PlayerToParasite_f (edict_t *ent);
void Cmd_MiniSentry_f (edict_t *ent);
void Cmd_CreateSupplyStation_f (edict_t *ent);
void Cmd_Decoy_f (edict_t *ent);
qboolean M_Regenerate (edict_t *self, int regen_frames, int delay, float mult, qboolean regen_health, qboolean regen_armor, qboolean regen_ammo, int *nextframe);
qboolean M_NeedRegen (edict_t *ent);
qboolean M_IgnoreInferiorTarget (edict_t *self, edict_t *target);//4.5
qboolean M_MeleeAttack (edict_t *self, float range, int damage, int knockback);
qboolean M_ValidMedicTarget (edict_t *self, edict_t *target);
qboolean M_Upkeep (edict_t *self, int delay, int upkeep_cost);
void M_FindPath (edict_t *self, vec3_t goalpos, qboolean compute_path_now);
void M_Remove (edict_t *self, qboolean refund, qboolean effect);
qboolean M_SetBoundingBox (int mtype, vec3_t boxmin, vec3_t boxmax);
qboolean M_Initialize (edict_t *ent, edict_t *monster);
void M_Notify (edict_t *monster);
void M_BodyThink (edict_t *self);
void M_PrepBodyRemoval (edict_t *self);
char *GetMonsterKindString (int mtype);
void PrintNumEntities (qboolean list);

#define Laser_Red 0xf2f2f0f0 // bright red
#define Laser_Green 0xd0d1d2d3 // bright green
#define Laser_Blue 0xf3f3f1f1 // bright blue
#define Laser_Yellow 0xdcdddedf // bright yellow
#define Laser_YellowS 0xe0e1e2e3 // yellow strobe
#define Laser_DkPurple 0x80818283 // dark purple
#define Laser_LtBlue 0x70717273 // light blue
#define Laser_Green2 0x90919293 // different green
#define Laser_Purple 0xb0b1b2b3 // purple
#define Laser_Red2 0x40414243 // different red
#define Laser_Orange 0xe2e5e3e6 // orange
#define Laser_Mix 0xd0f1d3f3 // mixture
#define Laser_RedBlue 0xf2f3f0f1 // inner = red, outer = blue
#define Laser_BlueRed 0xf3f2f1f0 // inner = blue, outer = red
#define Laser_GreenY 0xdad0dcd2 // inner = green, outer = yellow
#define Laser_YellowG 0xd0dad2dc // inner = yellow, outer = green

qboolean G_IsDeathmatch(edict_t *self);
qboolean G_Spawn_Monster(float secs);
void ai_run_slide(edict_t *self, float distance);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);
qboolean SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void Cmd_CreateArmor_f(edict_t *ent);
void Ammo_Regen(edict_t *ent);
void Special_Regen(edict_t *ent);
void CTF_SummonableCheck (edict_t *self);

// FLIP status:
#define FLIP_SHELL                  1
#define FLIP_WATER                  2
void Use_Lasers (edict_t *ent, gitem_t *item);
void Cmd_LaserSight_f(edict_t *ent);

//==========================================
//========= TYPES OF MONSTERS ==============
//==========================================

// TYPE OF MONSTER ---- HEALTH
#define M_SOLDIERLT 1 // 20
#define M_SOLDIER 2 // 30
#define M_SOLDIERSS 3 // 40
#define M_FLIPPER 4 // 50
#define M_FLYER 5 // 50
#define M_INFANTRY 6 // 100
#define M_INSANE 7 // 100 - Crazy Marine
#define M_GUNNER 8 // 175
#define M_CHICK 9 // 175
#define M_PARASITE 10 // 175
#define M_FLOATER 11 // 200
#define M_HOVER 12 // 240
#define M_BERSERK 13 // 240
#define M_MEDIC 14 // 300
#define M_MUTANT 15 // 300
#define M_BRAIN 16 // 300
#define M_GLADIATOR 17 // 400
#define M_TANK 18 // 750
#define M_SUPERTANK 19 // 1500
#define M_BOSS2 20 // 2000
#define M_JORG 21 // 3000
#define M_MAKRON 22 // 3000
#define M_COMMANDER 23
#define M_MINISENTRY	100
#define M_SENTRY		101
#define M_BFG_SENTRY	102
#define	M_MYPARASITE	103
#define M_FORCEWALL		104
#define M_DECOY			105
#define M_RETARD		107 // teamplay
#define M_SKULL			108
#define M_YINSPIRIT		109
#define M_YANGSPIRIT	110
#define M_BALANCESPIRIT	111
#define M_AUTOCANNON	112
#define M_DETECTOR		113
#define M_MIRROR		114
#define M_SUPPLYSTATION	115
#define M_MIRV			116 // need this to differentiate from normal grenade
#define M_HEALER		117
#define M_SPIKER		118
#define M_OBSTACLE		119
#define M_BOX 350
#define M_GASSER		120
#define M_SPIKEBALL		121
#define M_COCOON		122
#define M_LASERPLATFORM	123
#define M_ALARM			124
#define M_LASER			125
#define M_PROXY			126
#define M_MAGMINE		127
#define M_SPIKE_GRENADE	128
#define M_HOLYGROUND	129
#define M_WORLDSPAWN	130
#define P_TANK			200
#define MORPH_MUTANT	400
#define MORPH_CACODEMON	401
#define MORPH_TANK		402
#define MORPH_BRAIN		403
#define MORPH_FLYER		404
#define MORPH_MEDIC		405
#define MORPH_BERSERK	406
#define BOSS_TANK		501
#define BOSS_MAKRON		502
#define INVASION_PLAYERSPAWN	700
#define INVASION_NAVI			701
#define INVASION_MONSTERSPAWN	702
#define	PLAYER_NAVI				703
#define INVASION_DEFENDERSPAWN	704
#define CTF_PLAYERSPAWN			705
#define TBI_PLAYERSPAWN			706 // Team Based Invasion PlayerSpawn.
#define M_COMBAT_POINT			800 // temporary entity for monster navigation
#define FUNC_DOOR				900
//4.1 Archer
#define TOTEM_FIRE		605
#define TOTEM_WATER		606
#define TOTEM_AIR		607
#define TOTEM_EARTH		608
#define TOTEM_NATURE	609
#define TOTEM_DARKNESS	610

#define AURA_HOLYFREEZE 201
#define AURA_SALVATION	202
#define AURA_HOLYSHOCK	203
#define AURA_MANASHIELD	204
#define CURSE_FROZEN	301
#define CURSE_BURN		302
#define CURSE_BOMBS		303
#define CURSE_PLAGUE	304
#define BLEEDING		305
#define POISON			306

#define	FUNC_PLAT		1

#define VectorEmpty(a)        ((a[0]==0)&&(a[1]==0)&&(a[2]==0))
#define SENTRY_UPRIGHT		1
#define SENTRY_FLIPPED		2
int V_GetRuneWeaponPts(edict_t *ent, item_t *rune);
int V_GetRuneAbilityPts(edict_t *ent, item_t *rune);
qboolean SavePlayer(edict_t *ent);
qboolean IsNewbieBasher (edict_t *player);
qboolean TeleportNearTarget (edict_t *self, edict_t *target, float dist);
qboolean FindValidSpawnPoint (edict_t *ent, qboolean air);
void ValidateAngles (vec3_t angles);
int InJoinedQueue (edict_t *ent);
qboolean IsABoss(edict_t *ent);
qboolean IsBossTeam (edict_t *ent);
void AddBossExp (edict_t *attacker, edict_t *target);
void AwardBossKill (edict_t *boss);
void CreateRandomPlayerBoss (qboolean find_new_candidate);
qboolean BossExists (void);
int numNearbyEntities (edict_t *ent, float dist, qboolean vis);
void RemoveLasers (edict_t *ent);
int V_AddFinalExp (edict_t *player, int exp);
//K03 End

//NewB
#include "runes.h"
//NewB
#include "special_items.h"
#include "ctf.h" // 3.7
#endif

//r1: terminating strncpy
#define Q_strncpy(dst, src, len) \
do { \
	strncpy ((dst), (src), (len)); \
	(dst)[(len)] = 0; \
} while (0)

//az begin
void SaveAllPlayers();
// v_sqlite_character.c
qboolean VSF_SavePlayer(edict_t *player, char *path, qboolean fileexists, char* playername);
qboolean VSF_LoadPlayer(edict_t *player, char* path);

// v_sqlite_unidb.c
void VSFU_SavePlayer(edict_t *player);
void VSFU_SaveRunes(edict_t *player);
qboolean VSFU_LoadPlayer(edict_t *player);
void V_VSFU_StartConn();
void V_VSFU_Cleanup();
int VSFU_GetID(char *playername);

// missing definitions
void OpenModeMenu(edict_t *ent);
qboolean CanJoinGame(edict_t *ent, int returned);
qboolean ToggleSecondary (edict_t *ent, gitem_t *item, qboolean printmsg);

#ifndef NO_GDS
void SaveCharacterQuit (edict_t *ent);
#endif

// New AutoStuff
void V_AutoStuff(edict_t* ent);

// new command system
#include "v_cmd.h"

// 3.0 new holy vortex mode
void hw_init();
void hw_awardpoints (void);
void hw_dropflag (edict_t *ent, gitem_t *item);
qboolean hw_pickupflag (edict_t *ent, edict_t *other);
void hw_spawnflag (void);
float hw_getdamagefactor(edict_t *targ, edict_t* attacker);

qboolean V_VoteInProgress();

/* active drones linked list */ 

edict_t *DroneList_Iterate();
edict_t *DroneList_Next(edict_t *ent);
void DroneList_Insert(edict_t* new_ent);
void DroneList_Remove(edict_t *ent);

// 3.4 new team vs invasion mode
edict_t* TBI_FindSpawn(edict_t *ent);
void InitTBI();

#include "v_luasettings.h"

//az end

#ifndef min
#define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif