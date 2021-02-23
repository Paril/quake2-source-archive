// g_local.h -- local definitions for game module

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"
#include "q_devels.h"
#include "z_menu.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"gunslinger 1.2"

//#define debugmode 1

#ifdef debugmode
	#define debugmsg gi.dprintf
#else
	#define debugmsg //gi.dprintf
#endif


// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_disconnect		7
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
#define FL_LANTERN				0x00002000	// the player's lantern is on
#define	FL_READY				0x00004000	// player is ready to play
#define FL_ANTIBOT				0x00008000	// Used for BotCrusher bot detection
#define FL_RESPAWN				0x80000000	// used for item respawning

#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level

#define BODY_QUEUE_SIZE		8

#define MELEE_DISTANCE		80

static vec3_t VEC_ORIGIN = {0,0,0};

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
	WEAPON_FIRING,
	WEAPON_ALT_FIRING,
	WEAPON_RELOADING,
	WEAPON_THROWING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS45LC,
	AMMO_SHELLS,
	AMMO_HATCHETS,
	AMMO_COCKTAILS,
	AMMO_BULLETS3030,
	AMMO_BULLETS4570,
	AMMO_CANNONBALLS
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
MOVETYPE_BOUNCE
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
#define	IT_ARTIFACT		64		// Stone

// gitem_t->weapmodel for weapons indicates model index
#define WEAP_BOWIE_KNIFE		1 
#define WEAP_REVOLVER			2 
#define WEAP_DOUBLEREVOLVER		3 
#define WEAP_COACHGUN			4 
#define WEAP_PUMPACTIONSHOTGUN	5 
#define WEAP_COCKTAIL			6 
#define WEAP_HATCHET			7 
#define WEAP_WINCHESTER_RIFLE	8 
#define WEAP_SHARPS_RIFLE		9
#define WEAP_CANNON				10
#define TEAM_RED				12
#define TEAM_BLUE				13
#define TEAM_GREEN				14
#define TEAM_WHITE				15

typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*reload_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	int			quantity2;		// for alt fire modes
	int			capacity;		// ammo carried before reloading; 0 means weapon is the ammo
	int			reloadtime;		// Delay between each bullet loaded
	float		weight;			// Mass of the item: for slowing players
	int			price;			// Price
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

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;


// means of death
#define MOD_UNKNOWN			0
#define MOD_BOWIE_KNIFE		1
#define MOD_REVOLVER		2
#define MOD_DREVOLVER		3
#define MOD_SHOTGUN			4
#define MOD_SSHOTGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_WINCHESTER_RIFLE	10
#define MOD_SHARPS_RIFLE	11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
#define MOD_HANDGRENADE		15	// Direct Coctail hit
#define MOD_HG_SPLASH		16	// Cocktail splash & shrapnel
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
#define MOD_MELEE			34
#define MOD_HATCHET			35
#define MOD_CANNON_BLAST	36	// Cannon splash
#define MOD_CANNON_EFFECT	37	// Cannon direct hit
#define MOD_FIRE			38
#define MOD_POISON			39
#define MOD_HEAD_SHOT		0x4000000
#define MOD_FRIENDLY_FIRE	0x8000000

extern	int	meansOfDeath;


extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

// random() - Random decimal number between 0 and 1
#define random()	((rand () & 0x7fff) / ((float)0x7fff))
// crandom() - Random decimal between -1 and 1
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
extern	cvar_t	*maxspectators;

extern	cvar_t	*zk_logonly;	// Z-bot detection -- Stone

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;

// Gunslinger Quake2 globals -- Stone
extern	cvar_t	*gamename;
extern	cvar_t	*playmode;
extern	cvar_t	*teamplay;
extern	cvar_t	*team_damage;
extern	cvar_t	*force_join;
extern	cvar_t	*use_ctf_skins;
extern	cvar_t	*clear_teams;
extern	cvar_t	*number_of_teams;
extern	cvar_t	*team1_name;
extern	cvar_t	*team2_name;
extern	cvar_t	*team3_name;
extern	cvar_t	*team4_name;
extern	cvar_t	*drop_badge;
extern	cvar_t	*drop_bag;
extern	cvar_t	*ready_all;
extern	cvar_t	*use_classes;
extern	cvar_t	*announce_deaths;
extern	cvar_t	*respawn_invuln;
extern	cvar_t	*weapons_unique;
extern	cvar_t	*drop_all_on_death;
extern	cvar_t	*bullet_ricochet;
extern	cvar_t	*item_weight;
extern	cvar_t	*hit_locations;
extern	cvar_t	*number_of_money_bags;
extern	cvar_t	*starting_cash;

extern	cvar_t	*allow_hands_of_lightning;
extern	cvar_t	*allow_strength_of_the_bear;
extern	cvar_t	*allow_flesh_of_the_salamander;
extern	cvar_t	*allow_aura_of_pain;
extern	cvar_t	*allow_tongue_of_the_leech;
extern	cvar_t	*allow_tail_of_the_scorpion;
extern	cvar_t	*allow_shroud_of_darkness;

extern	cvar_t	*artifact_min_timeout;
extern	cvar_t	*artifact_max_timeout;
extern	cvar_t	*artifact_glow;

extern	cvar_t	*allow_cannon;
extern	cvar_t	*allow_sheriff_badge;
extern	cvar_t	*allow_marshal_badge;
extern	cvar_t	*allow_bandolier;
extern	cvar_t	*allow_pack;
extern	cvar_t	*allow_lantern;
extern	cvar_t	*allow_bandages;
extern	cvar_t	*allow_laudanum;
extern	cvar_t	*max_holes;

// Bot detection
extern	cvar_t	*sv_botdetection; 
extern	cvar_t	*allow_download;
extern	cvar_t	*allow_download_players;
extern	cvar_t	*allow_download_models;
extern	cvar_t	*allowed_models;
extern	cvar_t	*bot_talk;

// End of Gunslinger cvars -- Stone


// Gunslinger globals
#define MAX_TEAMS 4
extern	team_t		teamdata[MAX_TEAMS];
extern	int			number_of_spawns;	// Count these as the level is loaded
extern	int			number_of_bases;	// Count these as the level is loaded
extern	int			*playerlist;		// Sorted player list
extern	qboolean	matchstats;		// Intermission stats
extern	int			intermissionframenum;
extern	int			preptimeframe;
extern	edict_t		*firsthole;
extern	edict_t		*lasthole;
extern	int			holes;
extern	int			gameoverframe;		// For Last Man Standing

// Definitions for Game Stats
#define	MAX_STANDARD	9

#define	GSTAT_HEADSHOT	0
#define	GSTAT_STREAK	1
#define	GSTAT_DEATHS	2
#define	GSTAT_BAGS		3
#define	GSTAT_SUICIDES	4
#define	GSTAT_KILLS		5
#define	GSTAT_TEAMKILLS	6
#define GSTAT_SHOTS		7
#define	GSTAT_HITS		8

#define	MAX_SPECIAL		6

// For Badge Wars
#define	GSTAT_SHERIFFTIME	0
#define	GSTAT_SHERIFFKILLS	1
#define	GSTAT_OUTLAWKILLS	2

// For The Big Heist
#define	GSTAT_RAIDS		0
#define	GSTAT_DEPOSITS	1
#define	GSTAT_BAGKILLS	2

// For Capture the Flag
#define	GSTAT_CAPTURES	0
#define	GSTAT_FLAGGRABS	1
#define	GSTAT_RETURNS	2
#define	GSTAT_FLAGKILLS	3

// For King of the Hill
#define	GSTAT_CAPTURES	0

// End Gunslinger globals

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
void CalcWeight (edict_t *ent);
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void Make_Dropped (edict_t *ent, gitem_t *item);
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
int		flagstate(char *value);
void	printvector(vec3_t vec);
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

#define DEFAULT_BULLET_HSPREAD	200
#define DEFAULT_BULLET_VSPREAD	200
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_DAMAGE	10

#define DEFAULT_WIN94_SPREAD	100
#define DEFAULT_SHARPS_SPREAD	0

// Bowie knife information.  Added by Stone
#define KNIFE_STAB_DAMAGE	25
#define KNIFE_STAB_RANGE	60
#define KNIFE_SLASH_DAMAGE	15
#define KNIFE_SLASH_RANGE	40

#define HATCHET_SLASH_DAMAGE	35
#define HATCHET_MELEE_RANGE		80


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

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_melee (edict_t *self, vec3_t start, vec3_t aimdir, int range, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_cocktail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void drop_cocktail (edict_t *self, vec3_t start, int damage, float damage_radius);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_hatchet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_cannon (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_cannon2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
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
void G_SetSpectatorStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void DeathmatchScoreboard (edict_t *ent, qboolean unicast);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int FRAME_FIRE2_FIRST, int FRAME_FIRE2_LAST, int *pause_frames, int *fire_frames, int *fire_frames2, void (*fire)(edict_t *ent), void (*fire2)(edict_t *ent));

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
	int			carried[MAX_ITEMS];
	int			weight;
	int			cash;				// For buying equipment

	// ammo capacities
	int			max_bullets45LC;
	int			max_shells;
	int			max_hatchets;
	int			max_cocktails;
	int			max_bullets3030;
	int			max_bullets4570;
	int			max_cannonballs;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;

	qboolean	spectator;			// client is a spectator

	int			hit_antibot;		//for keeping track of bot hits
	qboolean	allowed_download;
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	int			lastkills;			// Kills at time of last death
	vec3_t		cmd_angles;			// angles sent over in the last command

	qboolean	spectator;			// client is a spectator

	// Stone
	int			special[MAX_SPECIAL];	// for the Game Stats screen
	int			standard[MAX_STANDARD];	// For keeping track of personal records
	int			lastrank;				// For ranking announcements

	byte		is_bot;
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
	float		respawn_framenum;

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

	// Added by Stone
	struct	player_menu_s	menu_data;

	qboolean	alt_fire_on;		// Shooting alternate mode?
	qboolean	reloading;			// Currently Reloading?
	int			reloadcount;
	qboolean	zoomed;				// Are they zoomed in?
//	float		unzoomsensitivity;	// Unzoomed mouse sensitivity
//	float		zoomsensitivity;	// Zoomed mouse sensitivity
	float		oldfov;	
	int			oldhand;
	int			team;				// Team number
	int			badge_counter;		// For scoring while holding the badge
	float		last_artifact_msg;	// Time of last notification
	gitem_t		*artifact;			// Pointer to the artifact carried
	float		artifact_expire;	// Time when artifact expires
	edict_t		*poison;			// For poisoned entities
	edict_t		*satellite;			// For special effects on a player that
									// players should see but don't if the 
									// effect is set on their actual edict_t
	char		*stuffcmd;			// Delayed stuffcmd string
	edict_t		*idplayer;			// Player currently id'd
	qboolean	idon;				// Is ID enabled?
	qboolean	showid;				// Is an ID currently being showed?

/*	// Eraser
	///Q2 Camera Begin
    qboolean    bIsCamera;
    int         iMode;
    edict_t     *pTarget;  
    qboolean    bWatchingTheDead;
    vec3_t      vDeadOrigin;
    double       fXYLag;
    double       fZLag;
    double       fAngleLag;
	///Q2 Camera End

	// Eraser additions
	bot_team_t	*team;
	float		firing_delay;
	float		latency;
	lag_trail_t	*lag_trail;			// stores the last 1 second of movements for lag simulation
	lag_trail_t *lag_angles;
	qboolean		ctf_has_tech;
	float		slide_time;
	float		reached_goal_time;
	// Eraser end*/
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
};

// sv_botdetection flags
#define BOT_LOG 1
#define BOT_KICK 2
#define BOT_NOTIFY 4
#define BOT_IMPULSE 8
#define BOT_BAN 16

extern		int botdetection;
float		move_bot_time;
float		bot_next_msg;
qboolean	antibot;

void		MoveBot();
void		BotDetection(edict_t *ent, usercmd_t *ucmd);
void		OnBotDetection(edict_t *ent, char *msg);
qboolean	G_EntExists(edict_t *ent);
void		ravenspawnantibot (void);
void		BotDisconnect (edict_t *ent);
char		*CheckModel (char intext[64]);

void		gi_cprintf(edict_t *ent, int printlevel, char *fmt, ...);
void		gi_centerprintf(edict_t *ent, char *fmt, ...) ;
void		gi_bprintf(int printlevel, char *fmt, ...);
void		gi_sprintf(int printlevel, char *fmt, ...);

#define DPRINT0(m) gi.dprintf(m)
#define DPRINT1(m, a) gi.dprintf(m, a)
#define DPRINT2(m, a, b) gi.dprintf(m, a, b)
#define DPRINT3(m, a, b, c) gi.dprintf(m, a, b, c)
#define DPRINT6(m, a, b, c, d, e, f) gi.dprintf(m, a, b, c, d, e, f)
#define BPRINT0(m) gi_bprintf(PRINT_MEDIUM, m)
#define BPRINT1(m, a) gi_bprintf(PRINT_MEDIUM, m, a)
#define BPRINT2(m, a, b) gi_bprintf(PRINT_MEDIUM, m, a, b)
#define BPRINT3(m, a, b, c) gi_bprintf(PRINT_MEDIUM, m, a, b, c)
#define BPRINT4(m, a, b, c, d) gi_bprintf(PRINT_MEDIUM, m, a, b, c, d)
#define CPRINT0(c, m) if (!c->flags & FL_ANTIBOT) gi.cprintf(c, PRINT_MEDIUM, m)
#define CPRINT1(c, m, a) if (!c->flags & FL_ANTIBOT) gi.cprintf(c, PRINT_MEDIUM, m, a)
#define CPRINT2(c, m, a, b) if (!c->flags & FL_ANTIBOT) gi.cprintf(c, PRINT_MEDIUM, m, a, b)
#define PRINT0(m) 
#define PRINT1(m, a) 
#define PRINT2(m, a, b) 
#define PRINT3(m, a, b, c) 
