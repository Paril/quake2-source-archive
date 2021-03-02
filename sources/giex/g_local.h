// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "g_classids.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"giex"

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


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80
#define BODY_QUEUE_SIZE		8

#define GIEX_PUTYPES 100 // Max number of powerup types, does not affect file size of char saves any more
#define GIEX_MONSTER_PLAYERLEVEL_MULT		2
#define GIEX_MONSTER_PLAYERLEVEL_MULT_POW	0.5
// ---
// these need to be semi-static, since they actually affect size of charsave
#define GIEX_NUMCLASSES 8 // Max number of classes

#define GIEX_BASEITEMS 8 // Number of base types of items
#define GIEX_ITEM_WEAPON 0
#define GIEX_ITEM_ARMOR 1
#define GIEX_ITEM_HELMET 2
#define GIEX_ITEM_AMULET 3
#define GIEX_ITEM_BELT 4
#define GIEX_ITEM_BACKPACK 5

//These defines may be OR'ed together with an item's 'type' to indicate a special "class affinity" for that item
#define GIEX_ITEM_SOLDIER 256
#define GIEX_ITEM_TANK 512
#define GIEX_ITEM_CLERIC 1024
#define GIEX_ITEM_MAGE 2048
#define GIEX_ITEM_VAMPIRE 4096

#define GIEX_PUPERCHAR 10 // Max powerups plugged in per character
#define GIEX_PUCARRYPERCHAR 20 // Max powerups carriable per character
#define GIEX_ITEMPERCHAR 30 // Max items per character
#define GIEX_ENCHPERITEM 10 // Max enchantments per item

#define GIEX_AUTOPERITEM 5 // Max automatic powerups for item
#define GIEX_PUPERITEM 10 // Max pluggable powerups per item

#define GIEX_NUMCHARSTATS 20 // Max number of character stat "slots"
#define GIEX_STAT_LOWER_PLAYER_DEATHS 0 //Stat slot for total number of deaths by lower level players
#define GIEX_STAT_HIGHER_PLAYER_DEATHS 1 //Stat slot for total number of deaths by higher level players
#define GIEX_STAT_MONSTER_DEATHS 2 //Stat slot for total number of deaths by monsters
#define GIEX_STAT_OTHER_DEATHS 3 //Stat slot for total number of deaths by other (suicide, lava etc)
#define GIEX_STAT_LOWER_PLAYER_KILLS 4 //Stat slot for total number of kills on lower level players
#define GIEX_STAT_HIGHER_PLAYER_KILLS 5 //Stat slot for total number of kills on higher level players
#define GIEX_STAT_LOW_MONSTER_KILLS 6 //Stat slot for total number of kills on monsters levels 0 - 7
#define GIEX_STAT_MED_MONSTER_KILLS 7 //Stat slot for total number of kills on monsters levels 8 - 15
#define GIEX_STAT_HI_MONSTER_KILLS 8 //Stat slot for total number of kills on monsters levels 16 - 23
#define GIEX_STAT_VHI_MONSTER_KILLS 9 //Stat slot for total number of kills on monsters levels 24 - 30
// ---

typedef enum {
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum {
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum {
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS,
	AMMO_BLASTER
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_CLOSE				1 // Added
#define RANGE_NEAR				2
#define RANGE_MID				3
#define RANGE_FAR				4

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

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

#define MONSTER_CORPSE_TIMEOUT	getMonsterCorpseTimeout()

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
typedef enum {
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
MOVETYPE_FLYRICOCHET
} movetype_t;


/*
typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;
*/

// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32

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

typedef struct gitem_s {
	unsigned int	classid;
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
typedef struct {
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
	int			craze; // this can be set from 0 to 11 by a vote, and is then counted down each map change that is caused by hitting time- or fraglimit. When 10, game goes into "crazed" mode, 10x faster monster spawnings, halved respawn times, shorter timelimit...
	int			monsterhunt; // like craze, but this activates the monster hunt game mode

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct {
	int			framenum;
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

	edict_t		*sight_client;	// cycles through players
	edict_t		*sight_monster;	// cycles through monsters

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
	float		red_team_bonus;
	float		green_team_bonus;

	int			vote_type;
	char		vote_target[32];
	float		vote_time;

	float		hunt_endtime;

	int			light_level; // 0 - total darkness, 12 - normal light
	int			stealthcount; // number of monsters on map with Stealth

	int			monspawncount;
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct {
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
	unsigned int itemid;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;


typedef struct {
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


typedef struct {
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct {
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct {
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
	int			aim_frames; // number of frames that the monster has aimed at target, used to focus bullet spread
	int			lefty;
	float			strafe_speed; // used internally by ai_run_circle
	int		prefered_range;
	float		idle_time;
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;

	float		cts_any;
	float		cts_class1;
	float		cts_class2;
	float		cts_class3;

	float		item_any;
	float		itemmult_class1;
	float		itemmult_class2;
	float		itemmult_class3;
	float		itemmult_class4;

	char		name[128];
	int		level;
	int		skill;
	unsigned short ability;
	qboolean	aggressive;

	float healtime;
	float shardtime;
	float jumptime;
	float jetpacktime;

	float strolltime;
	float goal_strollyaw;

	int superchamp;
} monsterinfo_t;

#define GIEX_MABILITY_HEALTH		0x00001
#define GIEX_MABILITY_SPEED			0x00002
#define GIEX_MABILITY_DAMAGE		0x00004
#define GIEX_MABILITY_RES_EXPL		0x00008
#define GIEX_MABILITY_RES_IMPACT	0x00010
#define GIEX_MABILITY_RES_ENERGY	0x00020
#define GIEX_MABILITY_RES_FMAGIC	0x00040
#define GIEX_MABILITY_RES_BMAGIC	0x00080
#define GIEX_MABILITY_RES_LMAGIC	0x00100
#define GIEX_MABILITY_REGENAURA		0x00200
#define GIEX_MABILITY_SHARDARMOR	0x00400
#define GIEX_MABILITY_STEALTH		0x00800
#define GIEX_MABILITY_ARMORPIERCE	0x01000
#define GIEX_MABILITY_MANABURN		0x02000
#define GIEX_MABILITY_SILENCED		0x04000

#define GIEX_ITEMENCH_DAMAGE		0x00001
#define GIEX_ITEMENCH_HASTE			0x00002
#define GIEX_ITEMENCH_ARMOR			0x00100
#define GIEX_ITEMENCH_ABSORB		0x00200


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
#define MOD_UNKNOWN				0
#define MOD_BLASTER				1
#define MOD_SHOTGUN				2
#define MOD_SSHOTGUN				3
#define MOD_MACHINEGUN			4
#define MOD_CHAINGUN				5
#define MOD_GRENADE				6
#define MOD_G_SPLASH				7
#define MOD_ROCKET				8
#define MOD_R_SPLASH				9
#define MOD_HYPERBLASTER		10
#define MOD_RAILGUN				11
#define MOD_BFG_LASER			12
#define MOD_BFG_BLAST			13
#define MOD_BFG_EFFECT			14
#define MOD_HANDGRENADE			15
#define MOD_HG_SPLASH			16
#define MOD_WATER					17
#define MOD_SLIME					18
#define MOD_LAVA					19
#define MOD_CRUSH					20
#define MOD_TELEFRAG				21
#define MOD_FALLING				22
#define MOD_SUICIDE				23
#define MOD_HELD_GRENADE		24
#define MOD_EXPLOSIVE			25
#define MOD_BARREL				26
#define MOD_BOMB				27
#define MOD_EXIT				28
#define MOD_SPLASH				29
#define MOD_TARGET_LASER		30
#define MOD_TRIGGER_HURT		31
#define MOD_HIT					32
#define MOD_TARGET_BLASTER		33
#define MOD_INFERNO				34
#define MOD_FIREBALL			35
#define MOD_PLAGUEBOMB			36
#define MOD_CORPSEEXPLOSION		37
#define MOD_LASERMINE			38
#define MOD_DRAIN				39
#define MOD_LEECH				40
#define MOD_SACRIFICE			41
#define MOD_BLACKHOLE			42
#define MOD_SPORE				43
#define MOD_GIBRAIN				44
#define MOD_LIGHTNING			45
#define MOD_FIREBOLT			46
#define MOD_TANKSTRIKE			47
#define MOD_SPARK				48
#define MOD_BOLT				49
#define MOD_LASERBEAM			50
#define MOD_ARM					51
#define MOD_BITE				52
#define MOD_DRONE				53
#define MOD_BURST				54
#define MOD_INCENDIARY_GREN		55
#define MOD_INCENDIARY_GRENPOP	56
#define MOD_INCENDIARY			57
#define MOD_BLAZE				58
#define MOD_SWARM				59
#define MOD_STORM				60
#define MOD_MAGIC				0x4000000	// damage dealt by player spell
#define MOD_FRIENDLY_FIRE	0x8000000

#define VOTE_TM				1
#define VOTE_WM				2
#define VOTE_FFA			3
#define VOTE_TDM			4
#define VOTE_PVM			5
#define VOTE_DOM			6
#define VOTE_END			7
#define VOTE_KICK			8
#define VOTE_MUTE			9
#define VOTE_CRAZE			10
#define VOTE_HUNT			11
#define VOTE_MONSTER		12

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
extern	cvar_t	*dmmonsters;
extern	cvar_t	*coop;
extern	cvar_t	*teams;
extern	cvar_t	*teamdamage;
extern	cvar_t	*idletimeout;
extern	cvar_t	*charpath;
extern	cvar_t	*mapspath;
extern	cvar_t	*cdpspath;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*autoskill;
extern	cvar_t	*baseskill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*admin_password;
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
extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;

//r1: added
extern	cvar_t	*ipbans;

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

typedef struct {
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
void pluginPowerup(edict_t *ent, int pu, int item);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
gitem_t	*FindItemByClassid (unsigned int classid);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void Think_Weapon (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count, qboolean apply_amconv);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
gitem_t *getRandomAmmo(void);

//
// g_maprotation.c
//
void printMapList(edict_t *ent);
char *getNextMap(const char *oldmap);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *G_FindClassId (edict_t *from, unsigned int match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *findclosest (edict_t *from, vec3_t org, unsigned int classid, int rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);
char *getAbrevValue(long in);

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
int T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
int T_RadiusDamage (edict_t *target, edict_t *inflictor, edict_t *attacker, float damage, float mindamage, edict_t *ignore, float radius, qboolean dmgfalloff, int mod);

// Special
void assignToTeam(edict_t *ent);


// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK	0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_25_PIERCE		0x00000040	// damage pierces 25% armor
#define DAMAGE_50_PIERCE		0x00000080	// damage pierces 50% armor
#define DAMAGE_75_PIERCE		0x00000100	// damage pierces 75% armor
#define DAMAGE_100_PIERCE		0x00000200	// damage pierces 100% armor (tank armors may have over 100% absorb)
#define DAMAGE_10_MANABURN		0x00000400	// 10% of damage is removed from target and half of that is given to attacker
#define DAMAGE_25_MANABURN		0x00000800	// 25% of damage is removed from target and half of that is given to attacker
#define DAMAGE_50_MANABURN		0x00001000	// 50% of damage is removed from target and half of that is given to attacker
#define DAMAGE_100_MANABURN		0x00002000	// 100% of damage is removed from target and half of that is given to attacker
#define DAMAGE_200_MANABURN		0x00004000	// 200% of damage is removed from target and half of that is given to attacker
#define DAMAGE_400_MANABURN		0x00008000	// 400% of damage is removed from target and half of that is given to attacker
#define DAMAGE_50_MANALEECH		0x00010000	// 50% of damage is given as bonus to attacker mana
#define DAMAGE_100_MANALEECH	0x00020000	// 100% of damage is given as bonus to attacker mana
#define DAMAGE_200_MANALEECH	0x00040000	// 200% of damage is given as bonus to attacker mana

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

//
// g_monster.c
//
float getMonsterBonus(edict_t *self);
void monster_skip2frames(edict_t *self);
void monster_skip4frames(edict_t *self);
void monster_skip8frames(edict_t *self);
void monster_fire_greenmissile(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_incendiary(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int incendiary_damage, int speed, int flashtype);
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
void monster_corpse_think(edict_t *ent);
int getMonsterCorpseTimeout(void);

//
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
edict_t *ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);
void BecomeExplosion2 (edict_t *self);
void addStealth(void);
void removeStealth(void);

//
// g_ai.c
//
void AI_SetSightClient (void);

void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_jetpack (edict_t *self, float dist);
void ai_stopjetpack (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
void ai_run_slide(edict_t *self, float distance);
void ai_run_circle(edict_t *self, float distance);
int range (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean infront_aim (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void check_dodge_area(edict_t *origin, vec3_t centre, int radius, float jump);
void check_dodge_rocket(edict_t *self, vec3_t start, vec3_t dir, int speed, int range, int radius, float jump, float dodge);
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed, int range, float jump, float dodge);
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float gravity, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_incendiary(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int incendiary_damage, int speed, float gravity, float timer, float damage_radius);
edict_t *fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_armblast (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int range, int mod);

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
int SV_FilterPacket (char *from);
//qboolean SV_FilterPacket (char *from);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_client.c
//
void kick(edict_t *player);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void G_SetSpectatorStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void showGiexPlugins(edict_t *ent);
void showGiexUnplug(edict_t *ent);
void showGiexInventory(edict_t *ent);
void showGiexItemMenu(edict_t *ent);
void showGiexPowerups(edict_t *ent);
void showGiexPowerupMenu(edict_t *ent);
void showGiexMainMenu(edict_t *ent);
void showGiexHelpMenu(edict_t *ent);
void showGiexCommandsMenu(edict_t *ent);
void showGiexClassesMenu(edict_t *ent);
void showEnterNewPwdMenu(edict_t *ent);
void closeGiexMenu(edict_t *ent);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
qboolean M_MoveToGoal (edict_t *ent, float dist);
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
void logmsg(char *message);

//
// g_chase.c
//
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

//
// g_fileio.c
//
int newCharacter(edict_t *ent, int classId, char *password);
int loadCharacter(edict_t *ent, char *password);
void saveCharacter(edict_t *ent);

//
// g_magic.c
//
float getMagicBonuses(edict_t *ent, int spell);
int getAuraSlot(edict_t *ent, int aura_skillnum);
int getFreeAuraSlot(edict_t *ent);
int getAuraLevel(edict_t *ent, int aura_skillnum);
void giveAura(edict_t *caster, edict_t *target, int aura_skillnum, int auralevel, float auratime);
void checkAuras(edict_t *ent);
void castSpell(edict_t *self);
void castAura(edict_t *self);
void createLaser(vec3_t start, vec3_t end, int frames, int width, int color);

//r1: bans
void CheckIPBanList (void);

#define	MAX_IPFILTERS		1024

#define	IPF_NO_ERROR			0
#define	IPF_ERR_BAD_IP			1
#define IPF_ERR_BANS_FULL		2
#define	IPF_ERR_BAD_TIME		3
#define	IPF_ERR_BAD_REASON_LEN	4
#define	IPF_ERR_BAD_REASON_CHAR	5
#define	IPF_ERR_ALREADY_BANNED	6
#define	IPF_OK_BUT_NO_SAVE		7

typedef struct ipban_s {
	int				ip[4];			// ip to ban
	unsigned int	expiretime;		// epoch time
	char			reason[33];	// reason why this IP range is banned
	char			banner[16];		// name of person who placed ban
} ipban_t;

extern	ipban_t			ipfilters[MAX_IPFILTERS];

//
// g_item.c
//
void checkAmmoAmounts (edict_t *ent);

//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6

typedef struct {
	char		name[32];
	int			baseexp;
	float		expreqmult;
	int			maxlevel;
} classinfo_t;

typedef struct {
	char				name[32];
	qboolean			inuse;
	int				basecost;
	int				maxlevel;
	unsigned char	isspell; //0 = not spell, 1 = mage bonus, 2 = cleric bonus, 3 = mage+cleric bonus
	unsigned char	classreq[GIEX_NUMCLASSES];
	float				reqperlvl[GIEX_NUMCLASSES];
	int				bonus_level;
} powerupinfo_t;

typedef struct {
	char				name[32];
	unsigned short	type;
	gitem_t			*item;
	unsigned int	itemid;
	char			modelname[64];
	float			rarity;
	unsigned char	auto_putype[5];
	unsigned char	auto_pulvl[5];
	unsigned char	maxpowerups;
	unsigned char	putype[GIEX_PUPERITEM]; //List of "compatible" powerups
	char			pumaxlvl[GIEX_PUPERITEM]; //Maxlevel modifier (0 for same as powerupinfo)
	float			pucost[GIEX_PUPERITEM]; //Basecost multiplier (1.0 for same as powerupinfo)
	unsigned char	classreq[GIEX_NUMCLASSES];
	float			arg1; // Base damage/armor
	float			arg2;	// Additional damage/armor per level
	float			arg3; // Firerate for weapons
	float			arg4;
	float			arg5;
	float			arg6;
	float			arg7;
	float			arg8;
	float			arg9;
	float			arg10;
} iteminfo_t;

//
// g_giexitems.c
//
int getWornItem(edict_t *ent, int basetype);
const char *getBaseTypeName(int num);
iteminfo_t *getItemInfo(int num);
void spawnItem(edict_t *targ, edict_t *attacker);
void restructureItems(edict_t *ent);
int getItemPowerupInfoSlot(iteminfo_t *info, int putype);
iteminfo_t *getWornItemInfo(edict_t *ent, int basetype);
int getItemPowerupLevel(edict_t *ent, int item, int powerup);
int getWornItemPowerupLevel(edict_t *ent, int basetype, int powerup);
int getItemBaseLevelReq(edict_t *ent, int item, int classId);
int getItemLevelReq(edict_t *ent, int item, int classId);
int getWornItemLevelReq(edict_t *ent, int basetype, int classId);
int getItemSocketedCount(edict_t *ent, int item);
int getItemPowerupLevel(edict_t *ent, int item, int powerup);
void updateItemLevels(edict_t *ent);
void Cmd_listItems(edict_t *ent);
void Cmd_wieldItem(edict_t *ent);
void Cmd_wornItems(edict_t *ent);
void showItemInfo(edict_t *ent, int item);

//
// g_powerups.c
//
int getPowerupCost(edict_t *ent, int slot);
int getItemPowerupCost(edict_t *ent, int item, int slot);
int getClassExpLeft(edict_t *ent, int classId);
classinfo_t *getClassInfo(int num);
powerupinfo_t *getPowerupInfo(int num);
void restructurePowerups(edict_t *ent);
void restructurePlugins(edict_t *ent);
int getPluginCount(edict_t *ent);
void spawnAmmo(edict_t *targ, edict_t *attacker);
void spawnPowerup(edict_t *targ, edict_t *attacker);
int getRandomPowerupByClass(int cl);
int getPowerupClass(int num);
void deductExp(edict_t *attacker, edict_t *target);
void addExp(edict_t *self, edict_t *target, int damage);
void ApplyMax(edict_t *self);
void giveExpToAll(double amount);

//
// p_weapon.c
//

int getWeaponPierce(edict_t *ent, int item);
int getBlasterDamage(edict_t *ent, int item, iteminfo_t *info);
int getBlasterSpeed(edict_t *ent, int item, iteminfo_t *info);
int getBlasterConsumption(edict_t *ent, int item, iteminfo_t *info);
int getBlasterFirerate(edict_t *ent, int item, iteminfo_t *info);
int getBlasterPierce(edict_t *ent, int item, iteminfo_t *info);
int getShotgunTotalDamage(edict_t *ent, int item, iteminfo_t *info);
int getShotgunCount(edict_t *ent, int item, iteminfo_t *info);
int getShotgunSpread(edict_t *ent, int item, iteminfo_t *info);
int getShotgunFirerate(edict_t *ent, int item, iteminfo_t *info);
int getSuperShotgunTotalDamage(edict_t *ent, int item, iteminfo_t *info);
int getSuperShotgunCount(edict_t *ent, int item, iteminfo_t *info);
int getSuperShotgunSpread(edict_t *ent, int item, iteminfo_t *info);
int getSuperShotgunFirerate(edict_t *ent, int item, iteminfo_t *info);
int getSuperShotgunBarrelDelay(edict_t *ent, int item, iteminfo_t *info);
int getMachinegunDamage(edict_t *ent, int item, iteminfo_t *info);
int getMachinegunSpread(edict_t *ent, int item, iteminfo_t *info);
int getMachinegunFirerate(edict_t *ent, int item, iteminfo_t *info);
int getChaingunDamage(edict_t *ent, int item, iteminfo_t *info);
int getChaingunSpread(edict_t *ent, int item, iteminfo_t *info);
int getChaingunTotalDamage(edict_t *ent, int item, iteminfo_t *info);

typedef struct {
	unsigned char	activeClass;
	unsigned char	classLevel[GIEX_NUMCLASSES];
	unsigned int	classExp[GIEX_NUMCLASSES];

	unsigned char	putype[GIEX_PUPERCHAR]; // Plugged in powerups
	unsigned char	pumaxlvl[GIEX_PUPERCHAR];
	unsigned char	pucurlvl[GIEX_PUPERCHAR];
	int				puexp[GIEX_PUPERCHAR];

	char			wornItem[GIEX_BASEITEMS]; // -1 for no item
	char			activatingItem[GIEX_BASEITEMS]; // -1 for no item
	unsigned char	itemId[GIEX_ITEMPERCHAR];
	unsigned char	item_putype[GIEX_ITEMPERCHAR][GIEX_PUPERITEM];
	unsigned char	item_pumaxlvl[GIEX_ITEMPERCHAR][GIEX_PUPERITEM];
	unsigned char	item_pucurlvl[GIEX_ITEMPERCHAR][GIEX_PUPERITEM];
	int				item_puexp[GIEX_ITEMPERCHAR][GIEX_PUPERITEM];
	unsigned char	item_enchtype[GIEX_ITEMPERCHAR][GIEX_ENCHPERITEM];
	unsigned char	item_enchlvl[GIEX_ITEMPERCHAR][GIEX_ENCHPERITEM];

	unsigned char	powerup_type[GIEX_PUCARRYPERCHAR]; // Carried powerups
	unsigned char	powerup_level[GIEX_PUCARRYPERCHAR];
	unsigned char	active_powerup;
	unsigned char	quest_level;
	unsigned char	quest_type;
	unsigned char	quest_counter;

	int				stats[GIEX_NUMCHARSTATS]; // Statistics

	qboolean	autopickup;
	qboolean	autoplugin;
	char	password[32];
} skills_t;

typedef struct {
	unsigned char	putype[GIEX_PUPERITEM]; // Plugged in powerups
	unsigned char	pumaxlvl[GIEX_PUPERITEM];
	unsigned char	pucurlvl[GIEX_PUPERITEM];
	int				puexp[GIEX_PUPERITEM];
} itempuinfo_t;

// client data that stays across multiple level loads
typedef struct {
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

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;
	int			selspell;

	qboolean	spectator;			// client is a spectator
	qboolean	loggedin;
	char		vote;

	qboolean	muted;
	char		ip[32];

	skills_t	skills; //TODO: rename struct and variable

	gitem_t		*weapon;
	gitem_t		*old_weapon;
	char		lastweapon;

	float		add_health;
	float		add_exp;
	float		add_classexp;

	int			idletime;
	int	expRemain;
	int	puexpRemain[GIEX_PUPERCHAR];
	int	item_puexpRemain[GIEX_ITEMPERCHAR][GIEX_PUPERITEM];

	float		itemchangetime;
	int		itemchanging;		//Base item id for which item is changing right now.

	qboolean	entering_pwd;
	char		entered_pwd[32];

	unsigned char	skill[GIEX_PUTYPES]; // Used to sum up all the powerup levels
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct {
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	qboolean	spectator;			// client is a spectator
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s {
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
	short		showmenu;
	short		menuchoice[5];

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

	qboolean	castspell;
	float		spelltime;
	float		magregentime;

	int		max_armor;
	int		max_magic;
	int		magic;

	float		get_powerup;
	float		id_powerup;

	unsigned char	aura_type[5];
	unsigned char	aura_level[5];
	edict_t	*aura_caster[5];
	float		aura_time[5];
	float		aura_refreshtime; //When changing to a (new) aura, this is the time where it "kicks in".

	float	barf_time;
	int		barf_frames;

	int		playerdamage;
	edict_t	*flashlight;

	float	menu_delay_time;
	float	damage_time;
};


struct edict_s {
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
	unsigned int	classid;
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

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;
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
	int			dmg;			// used for players to indicate how much drowning
	int			radius_dmg;		// used as level indicator for players
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;			//used as team indicator for clients and dompoints

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
	itempuinfo_t	itempuinfo;
//	float		frozen;
};
