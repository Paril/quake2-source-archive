#define MAX_INI_EFFECTS			256
#define MAX_INI_MUNITIONS		96
#define MAX_INI_WEAPONS			64
#define MAX_INI_SUPPLIES		64
#define MAX_INI_BUILDINGS		64

#define MAX_MAP_EFFECTS			16
#define MAX_MAP_MUNITIONS		16
#define MAX_MAP_WEAPONS			8
#define MAX_MAP_SUPPLIES		8
#define MAX_MAP_BUILDINGS		8

#define MAX_EFFECTS				(MAX_INI_EFFECTS + MAX_MAP_EFFECTS)
#define MAX_MUNITIONS			(MAX_INI_MUNITIONS + MAX_MAP_MUNITIONS)
#define MAX_WEAPONS				(MAX_INI_WEAPONS + MAX_MAP_WEAPONS)
#define MAX_SUPPLIES			(MAX_INI_SUPPLIES + MAX_MAP_SUPPLIES)
#define MAX_BUILDINGS			(MAX_INI_BUILDINGS + MAX_MAP_BUILDINGS)

#define MAX_BUILDING_WEAPONS	4
#define MAX_VOTEMAPS			32
#define MAX_TEAMS				8

#define MAX_BLDG				12
#define MAX_BLDG_POINTS			900
#define	MAX_BLDG_ATTACH			4
#define MAX_BLDG_WEAPONS		4

#define ASSIGN_LEAST_PLAYERS	1
#define ASSIGN_LEAST_SCORE		2
#define ASSIGN_RANDOM			3

#define WS_RIFLE		0
#define WS_PISTOL		1
#define WS_HIP			2
#define WS_PUMP			3
#define WS_SHOULDER		4
#define WS_SHOULDER_ALT	5
#define WS_HEAD_TARGET	6

#define FT_PELLET		1
#define FT_FLIGHT		2
#define FT_TOSS			3

#define TT_DETONATE		0x01
#define TT_BOUNCE		0x02
#define TT_STICK		0x04
#define TT_DET_TOUCH	0x10
#define	TT_DET_TTL		0x20
#define TT_DET_SKY		0x40

#define FE_DRUNK		0x01
#define FE_HOMING		0x02
#define FE_BFG			0x04

#define PM_NOENCUMBRANCE		0x0001
#define PM_TEAMSCORES			0x0002
#define PM_FORCESKINS			0x0004
#define PM_TEAMDAMAGE			0x0008
#define PM_ELIMINATION			0x0010
#define PM_INFINITEAMMO			0x0020
#define PM_NORELOADS			0x0040
#define PM_DEFENSERETURN		0x0080
#define PM_HALFDAMAGE			0x0100
#define PM_STABLEWEAPONS		0x0200

/* New FLIGHT/TOUCH types ??
DET_IMPACT
DET_TOUCH
DET_TIMER

FLIGHT_BOUNCE
FLIGHT_STICK
FLIGHT_
*/

#define WT_SEMI			0x00
#define WT_AUTO			0x01
#define WT_BURST		0x02
#define WT_NOIDLEPAUSE	0x04
#define WT_NOTUSABLE	0x08
#define WT_NOAMMO		0x10
#define WT_COUNTRELOAD	0x20

enum {
	// Core effects
	BE_NONE,
	BE_SOUND,
	BE_MODEL,
	BE_MUZZLEFLASH,
	BE_TEMPENTITY,
	BE_PUSH,
	BE_TINT,
	BE_CONFUSION,
	BE_BLIND,
	BE_DAMAGE,

	BE_STATEDMG,
	BE_STATEPROT,
	BE_STATEGRAV,
	BE_STATESWIM,

	BE_RADIUS,
	BE_LOOP,

	// Custom effects
	BE_RADIUSDAMAGE,
	BE_GRAVITY,
	BE_CONCUSSION,
	BE_FLASH,
	BE_SHRAPNEL,
	BE_MIRV,
	BE_FLARE,
	BE_SPINNER,
	BE_SMOKE,
	BE_QUAKE,
	BE_AFFECTSENSOR,
	BE_DRONE
};

#define EFF_SWAPORIGINS			0x80000000	// Swap Effect Source and Destination Origins

#define TRIG_RADIO		5	// Remote activation by user

#define ITEM_ANY		0
#define ITEM_WEAPON		1
#define ITEM_AMMO		2
#define ITEM_SUPPLY		3

#define SUP_FLASHLIGHT			0x0001	// Flashlight is a generic pointer, also used for laser-sights, etc.
#define SUP_GRAPPLE				0x0002
#define SUP_GRENADE				0x0004
#define SUP_MSGJAMMER			0x0008	// prevents enemy players from hearing thier own teams messages
#define SUP_TIMER				0x0010	// Trigger: variable delay timer
#define SUP_TRIPWIRE			0x0020	// Trigger: point-to-point - crossing this line causes the trigger to activate
#define SUP_PROXY				0x0040	// Trigger: Activates on an objects motion, objects must be traveling at a certain speed to active
#define SUP_VIDSENSOR			0x0080	// Trigger: Activates when an object is within the triggers FOV.
#define SUP_TOOL				0x0100	// Tool:	Tools are required for constructing/maintaining items
#define SUP_ARMOUR				0x0200	// Armour:	Does what armour does - different types based on flags
#define SUP_SIGHT				0x0400  // Sight:	Zoom/Targeting/HUD Abilities
#define SUP_SCUBA				0x0800	// SCUBA:	Increased air-time, and improved swimming performance
#define SUP_UNIFORM				0x1000	// Uniform: Allows the player to impersonate another team, and avoid team-sensative triggers
#define SUP_SABOTAGE			0x2000	// Allows player to sabotage enemy buildings, causing the malfunction
#define SUP_MEDIKIT				0x4000	// Allows the player to heal himself based on some specific criteria
#define SUP_GOAL				0x8000	// Goal:    Some triggers may only be activated by players who carry items w/ this mask

#define TOOL_BUILD				0x0001	// required to create a new building
#define TOOL_REPAIR				0x0002	// required to repair a damaged building
#define TOOL_ADJUST				0x0004	// required to modify a buildings parameters
#define TOOL_DISMANTLE			0x0008	// required to dismantle a building

// supply flags
#define SF_MOUNTED				0x0001	// This item is mounted to something else
#define SF_INFRARED				0x0001	// Allows VidSensors to see IR emitting targets

#define AS_DESTROY				0x0001	// Sensor is destroyed
#define AS_TIMEDDISABLE			0x0002	// Sensor is disabled for some time
#define AS_RANDOMDISABLE		0x0004	// Sensor is disabled for some random time
#define AS_TARGETORIGIN			0x0008	// Target the Effect origin
#define AS_TARGETRANDOM			0x0010	// Target some random point near the Effect origin

#define SF_ID_NAME				0x0010	// Scope ID's Target Name
#define SF_ID_TEAM				0x0020	// Scope ID's Target Team Name
#define SF_ID_WEAPON			0x0040	// Scope ID's Target Selected Weapon
#define SF_ID_IFF				0x0080	// Scope displays IFF crosshairs
#define SF_ID_RANGE				0x0100	// Scope displays Range To Target
#define SF_ID_STARLIGHT			0x0200  // Scope allows player to see targets in total darkness

// must be bitwise for building::allow_attach to work
#define BLDG_EFXBOX				0x0001
#define BLDG_MOUNT				0x0002
#define BLDG_GUN				0x0004
#define BLDG_CAMERA				0x0008
#define BLDG_MSGDECODER			0x0020	// allows the owning team to hear enemy messages
#define BLDG_LASERGATE			0x0200  // Laser-based defense barrier
#define BLDG_SPOTTER			0x0400	// Mountable spotlight - unlimited powerconsumption, not portable.

#define BLDG_REMOTE_TRIGGER		0x0010
#define BLDG_MSGTRANSMITTER		0x0040	// allows the transmittion of decoded messages and diagnostic info from guns
#define BLDG_DIAGBOX			0x0080	// relays diagnostic info (ammo, health, etc) on buildings to thier owner, requires a transmitter
#define BLDG_RADIOTRIG			0x0100	// Trigger: Building that uses its owner when used by a player

#define	BLDG_ATTACH_FLOORS			0x0001
#define BLDG_ATTACH_WALLS			0x0002
#define	BLDG_ATTACH_CEILINGS		0x0004
#define BLDG_ATTACH_PLAYERS			0x0008
#define BLDG_ANGLES_SURFACE			0x0010
#define	BLDG_YAW_FACEPLAYER			0x0020
#define BLDG_PITCH_FULL				0x0040
#define BLDG_PITCH_PARTIAL			0x0080
#define BLDG_ALLOW_PLAYERBUILD		0x0100
#define BLDG_ALLOW_PLAYERDESTROY	0x0200
#define BLDG_DEFAULT_OFF			0x0400
#define BLDG_ALLOW_PLAYERMOVE		0x1000	// player can move building, once built
#define BLDG_ALLOW_PLAYERROTATE		0x2000  // player can rotate building, once built
#define BLDG_ALLOW_PLAYERPILOT		0x4000	// player can pilot building, once built
#define BLDG_USE_ON_DESTROY			0x8000	// use the building before its destroyed?

#define BLDG_STATE_DISABLED			0	// Building is disabled, and cannot be placed online (must be repaired)
#define BLDG_STATE_OFFLINE			1	// Building is usable, and can be placed online (usually for map-based buildings)
#define BLDG_STATE_ONLINE			2	// Building is online, and can be activated (default state, idle)
#define BLDG_STATE_ACTIVATED		3	// Building is online, and has been activated/triggered (triggered state)

#define TEAM_CAMERA		1
#define TEAM_GUN		2
#define TEAM_MOUNT		3

#define PERSONAL_CHARGE	1
#define PERSONAL_SENTRY 2

#define SUPPLY_SF_RESPAWN	0x00000001

#define GOAL_ATBASE			0
#define GOAL_CARRIED		1
#define GOAL_DROPPED		2

#define LOC_NONE		0x00
#define LOC_LEGS		0x01
#define LOC_CHEST		0x02
#define LOC_HEAD		0x04

enum
{
	ARMOUR_SPARKS,
	ARMOUR_SHELL,
	ARMOUR_SHIELD,
	ARMOUR_FADE
};

enum
{
	MOD_KILLOTHER,
	MOD_KILLTEAM,
	MOD_KILLSELF,
	MOD_BLDG_KILLOTHER,
	MOD_BLDG_KILLTEAM,
	MOD_BLDG_KILLSELF,
	MOD_BLDG_KILLOWNER,
	MOD_WORLD
};

enum
{
	PPM_IDLE,
	PPM_RUN,
	PPM_ATTACK,
	PPM_PAIN,
	PPM_DIE,
	PPM_C_IDLE = 10,
	PPM_C_RUN,
	PPM_C_ATTACK,
	PPM_C_PAIN,
	PPM_C_DIE,
	PPM_JUMP = 100
};

// player_state->stats[] indexes

#define STAT_HEALTH_ICON		0
#define	STAT_HEALTH				1
#define	STAT_AMMO_ICON			2
#define	STAT_AMMO				3
#define	STAT_ARMOR_ICON			4
#define	STAT_ARMOR				5

// DEADLODE
#define STAT_ID_NAME			6
#define STAT_ID_TEAM			7
#define STAT_ID_WEAPON			8
#define STAT_ID_RANGE			9
#define STAT_ID_HITLOC			10
#define STAT_TRIGGER			11
#define STAT_GOALITEM			12
// DEADLODE

#define	STAT_LAYOUTS			13
#define	STAT_FRAGS				14
#define	STAT_FLASHES			15		// cleared each frame, 1 = health, 2 = armor

// DEADLODE
#define STAT_CLIP				16	//27
#define	STAT_WEAPON_STRING		17	//28
#define STAT_AMMO_STRING		18	//29
#define STAT_PLAYER_STRING		19	//30
#define STAT_DL2_RESOURCES		20	//31

#define STAT_TEAMICON			21
#define STAT_TEAMSCORE			22

#define STAT_TEAM1_SCORE		23
#define STAT_TEAM2_SCORE		24
#define STAT_TEAM3_SCORE		25
#define STAT_TEAM4_SCORE		26

// DEADLODE

// DEADLODE MENU ID's
enum {
	ID_JOIN,
	ID_CREDITS,
	ID_MAIN,
	ID_INVENTORY,
	ID_SHOP,
	ID_WEAPONS,
	ID_WEAPONSTATS,
	ID_AMMOTYPES,
	ID_AMMOVARIANTS,
	ID_AMMOSTATS,
	ID_AMMOQUANTITY,
	ID_SUPPLIES,
	ID_SUPPLYSTATS,
	ID_SUPPLYQUANTITY,
	ID_PERSONALOBJECTS,
	ID_TEAMOBJECTS,
	ID_OBJECTPROPERTIES,
	ID_SELECTOBJECTS,
	ID_PROFILES,
	ID_RADIOMESSAGES,
	ID_BUILDINGINFO,
	ID_TRIGGERINFO,
	ID_CONSTRUCTION,
	ID_MAPVOTING,
	ID_SELECTUNIFORM,
	ID_STATS,
	ID_ALTMENU,
	ID_MOTD,
	ID_PLAYER,
	ID_CONFIG,
	ID_MAPMENU = 100
};

// properties of Quake2 font
#define	FONT_WIDTH	8
#define	FONT_HEIGHT	8

typedef struct
{
	void	**children;
	void	*parent;
} family_t;

//
// fields are needed for spawning from the entity string
// and saving / loading games - moved from g_local.h to deadlode.h
//
#define FFL_SPAWNTEMP		1

typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLE,			// like F_VECTOR, but anglemod()'s as well
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char		*name;
	int			ofs;
	fieldtype_t	type;
	int			flags;
	int			arraysize;
	int			elementsize;
} field_t;

extern	field_t fields[];

typedef struct {
	char	*name;
	int		value;
} ident_t;
extern ident_t	identSet[];

typedef struct {
	char	*name;
	int		s_frame;
	int		e_frame;
} ppmframes_t;
extern ppmframes_t s_standFrames[];
extern ppmframes_t c_standFrames[];
extern ppmframes_t s_runFrames[];
extern ppmframes_t c_runFrames[];
extern ppmframes_t s_attackFrames[];
extern ppmframes_t c_attackFrames[];
extern ppmframes_t s_painFrames[];
extern ppmframes_t c_painFrames[];
extern ppmframes_t s_dieFrames[];
extern ppmframes_t c_dieFrames[];
extern ppmframes_t jumpFrames[];


typedef struct {
	char	*name;
	char	*base_name;		// Weapon Compatability name
	int		ftype;			// ammo type FT_PELLET/FLIGHT/TOSS
	float	weight;			// Weight per round
	float	range;			// Base range/speed for this ammo
	float	damage;			// Base damage for this ammo
	int		pellet_count;	// Number of pellets per round
	float	hspread;		// horizontal pellet spread
	float	vspread;		// vertical pellet spread

	// Instant/Pellet options
	int		trail_effect;	// EF_* for FT_FLIGHT/FT_TOSS, TE_* for FT_PELLET
	int		render_effect;	// RF_*

	// Flight and Lobb/Toss Projectile options
	int		ttype;			// Touch type : Bounce, Drop, Stick
	int		flight_effect;	// FE_NORMAL, FE_DRUNK, FE_HOMING, FE_LASERS
	vec3_t	track_speed;	// Turning speed for FE_DRUNK and FE_HOMING
	int		track_fov;		// Targeting FOV for FE_HOMING
	float	duration;		// TTL for flight/lobb/toss projectiles
	char	*sound;			// Looping flight sound effect;
	char	*model;			// Model for projectiles
	int		skinnum;		// Skin number for model
	float	gravity;		// Gravity effects for projectile (FT_TOSS only)
	// Detonation/Impact Effects
	char	*effect;		// det/impact effects
	char	*pic_s;			// 8x8 icon in inventory
	char	*pic_b;			// 24x24 icon in status bar
} ammo_t;
extern ammo_t	ammo[MAX_MUNITIONS];
extern field_t	dl_ammoflds[];

#define ITEMAMMO(x) ((ammo_t *)(x->item))
#define AMMO_INDEX(x)	((x) - ammo);
#define	AOFS(x) (int)&(((ammo_t *)0)->x)

typedef struct {
	char	*name;
	char	*base_ammo;			// The base ammo type required for this weapon to function
	int		player_select;		// Player can have this weapon (sentries use guns too)
	float	damage_mod;			// Damage modifier for this weapon
	float	range_mod;			// Range Modifier for this weapon
	float	accuracy;			// Accuacy for this weapon
	float	reliability;		// Chance that weapon will fail/jam
	float	weight;				// Weapon weight
	float	size;				// weapon size;
	int		rnds_chambered;		// Rounds fired each fire_frame.  must be > 0
	int		clip_size;			// Number of rounds between reloads (0 = breach weapon)
								// **NOTE that players don't actually carry clips as items
	int		use_count;			// Number of times this weapon can be reloaded before its discarded (0 = infinate)
	float	reload_time;		// Time to reload
	float	cyclic_rate;		// Time between rounds, ignored for breach weapons
	int		recoil;				// view-deviation/kick from recoil in degrees
								// values larger than 30 will MOVE players in addition to kick
	int		flags;				// WEAP_AUTO/WEAP_SEMI, WEAP_BURST, etc
	int		burst_cnt;			// Rounds for Burst weapons, ignored UNLESS WEAP_BURST is set, uses cyclic_rate for rate of fire
	char	*view_model;		// Model the player sees
	char	*player_model;		// Model other players see
	int		frame_raise;		// Start frame of weapon activate/reload
	int		frame_raise_num;	// Number of frames in raise sequence
	int		frame_fire;			// Start frame of weapon firing
	int		frame_fire_num;		// Number of frames in firing sequence
	int		frame_idle;			// Start frame of weapon idling
	int		frame_idle_num;		// Number of frames in idle sequence
	int		frame_drop;			// Start frame of weapon drop
	int		frame_drop_num;		// Number of frames in drop sequence
	int		frame_reload;		// Start frame of weapon reload
	int		frame_reload_num;	// Number of frames in reload sequence
	char	*snd_raise;			// Sound to play when weapon is activated
	char	*snd_fire;			// Sound to play when weapon is fired
	char	*snd_idle;			// Sound to play when weapon is idle;
	char	*snd_drop;			// Sound to play when weapon is deactivated
	char	*snd_reload;		// Sound to play when weapon is reloaded
	int		s_load_index;		//
	int		fire_frame;			// Animation frame that weapon fires on (relative to frame_fire)
								// 0 = don't use fire-frames frames
	vec3_t	fire_offset;		// Projection offset for this weapon, relative to the default Q2 offset.
	int		reload_pause_frame;	// Reload frame to pause on when the reload time takes longer then the frameset.
	char	*pic_s;				// 8x8 icon in inventory
	char	*pic_b;				// 24x24 icon in status bar
	char	*category;			// Category Name

	char	*ppmJump;			// VWEAP Animation style
	char	*ppmIdle_stand;		// VWEAP Animation style
	char	*ppmRun_stand;		// VWEAP Animation style
	char	*ppmAttack_stand;	// VWEAP Animation style
	char	*ppmPain_stand;		// VWEAP Animation style
	char	*ppmDie_stand;		// VWEAP Animation style
	char	*ppmIdle_crouch;	// VWEAP Animation style
	char	*ppmRun_crouch;		// VWEAP Animation style
	char	*ppmAttack_crouch;	// VWEAP Animation style
	char	*ppmPain_crouch;	// VWEAP Animation style
	char	*ppmDie_crouch;		// VWEAP Animation style
} weap_t;
extern weap_t	weapons[MAX_WEAPONS];
extern field_t dl_weapflds[];

#define ITEMWEAP(x) ((weap_t *)(x->item))
#define WEAP_INDEX(x)	((x) - weapons);
#define	WOFS(x) (int)&(((weap_t *)0)->x)

typedef struct {
	char *name;
	int type;
	char *damage;
	vec3_t offset;
	vec3_t spread;
	int range;
	float duration;
	float delay;
	float speed;
	int health;
	int	flags;
	int	dmgflags;
	int count;
	/*
	int affect_players;
	int affect_projectiles;
	int affect_buildings;
	int affect_items;
	int affect_liquid;
	int affect_thruwalls;
	*/
	char *model;
	int skinnum;
	int frame;
	int num_frames;
	char *sound;
	char *effect;
} effect_t;

extern effect_t	effects[MAX_EFFECTS];
extern field_t dl_efxflds[];
#define	EOFS(x) (int)&(((effect_t *)0)->x)

typedef struct {
	char		*name;
	int			type;
	int			flags;
	int			tool;
	float		weight;
	float		duration;
	float		delay;
	float		health;
	float		impact_prot;
	float		pierce_prot;
	float		blast_prot;
	float		energy_prot;
	int			range;
	int			damage;
	vec3_t		offset;
	int			skinnum;
	int			frame;
	char		*model;
	char		*effect;
	char		*s_use;
	char		*pic_s;
	char		*pic_b;
	char		*category;			// Category Name
	char		*defaultcmd;		// Default AltCmd
} supply_t;
extern supply_t supplies[MAX_SUPPLIES];
extern field_t dl_supflds[];

#define ITEMSUPPLY(x) ((supply_t *)(x->item))
#define SUPPLY_INDEX(x) ((x) - supplies)
#define	SUPOFS(x) (int)&(((supply_t *)0)->x)

typedef struct {
	char		*name;
	int			type;
	int			subtype;
	int			flags;
	int			use_count;
	int			req_tools;
	int			min_points;
	int			max_points;
	float		weight;
	float		build_time;
	float		duration;
	vec3_t		mins;
	vec3_t		maxs;
	int			min_dist;
	int			max_dist;
	char		*base_model;
	vec3_t		base_offset;
	char		*model;
	int			skin;
	int			health;
	int			range;
	int			fov;
	int			damage;
	int			frame_build;
	int			frame_build_num;
	int			frame_idle;
	int			frame_idle_num;
	int			frame_action;
	int			frame_action_num;
	int			frame_die;
	int			frame_die_num;
	int			fire1_frame;
	int			fire2_frame;
	struct
	{
		char	*name;
		vec3_t	offset;
		char	*ammo;
	} weapon[MAX_BLDG_WEAPONS];
	struct
	{
		int		type;
		vec3_t	offset;
		vec3_t	angles;
		vec3_t	min;
		vec3_t	max;
		vec3_t	speed;
		vec3_t	trackspeed;
	} attach[MAX_BLDG_ATTACH];
	char		*trigger;
	vec3_t		trigger_offset;
	vec3_t		trigger_angles;
	char		*s_start;
	char		*s_done;
	char		*s_idle;
	float		s_idle_time;
	char		*s_die;
	char		*s_use;
	char		*effect;
	char		*upgrade;
	char		*pic_s;
	char		*pic_b;
	char		*category;			// Category Name
} building_t;
extern building_t buildings[MAX_BUILDINGS];
extern field_t dl_bldflds[];

#define ITEMBLDG(x) ((building_t *)(x->item))
#define BUILDING_INDEX(x) ((x) - buildings)
#define	BLDOFS(x) (int)&(((building_t *)0)->x)

struct item_s;
typedef struct item_s 
{
	int			itemtype;	// ITEM_AMMO, ITEM_WEAPON, ITEM_SUPPLY, ITEM_EQUIPMENT, etc
	void		*item;		// index to item (weap_t, ammo_t, or equip_t)
	int			priority;	// Item priority, use this item before another item of same type w/ lower priority
	int			quantity;	// Number of that item
	int			maximum;	// Quantity should never exceed this value
	int			usecnt;		// number of times this item has been used.
	int			clipcnt;	// rounds remaining in current clip - for ITEM_WEAPON only
	ammo_t		*cliptype;	// the type of rounds in current clip - for ITEM_WEAPON only
	int			totalrnds;	// total number of available rounds prior to a reload - for ITEM_WEAPON only
	float		reloadrate;	// Frames per round when reloading
	int			status;		// current status info for this item, ST_INUSE, etc...
	float		heat;		// current item heat - applies mostly to weapons and equipment
	char		*altcmd;	// Alternate command that can be used to activate this item (ie, hotkeys).
	struct item_s *depend;	// item dependancy - points to another item_t
	void		*prev;		// Previous item in inventory
	void		*next;		// Next item in inventory
} item_t;

struct team_s;
typedef struct object_s
{
	char		*name;			// description for menus
	char		*type_name;		// displayed in parentheses in menus
	int			type;			// object type: camera, etc.
	edict_t		*entity;		// edict the object is tied to
	edict_t		*owner;			// client the object belongs to
	struct	team_s	*team;		// team the object belongs to
	family_t	family;			// Parent and children hierarchy
	qboolean	team_visible;	// TRUE if owner has allowed others to see/use this object
//	int			level;			// hierarchy in object list
	char		*pic_s;			// pic in menu
	char		*pic_b;			// pic in status bar
} object_t;

typedef struct team_s
{
	char	team_no;	// team number
	char	*team_name;	// team name
	int		players;	// number of players
	int		totalfrags;	// Total frags for this team
	int		score;		// team score
	char	*pic_s;		// menu team pic
	char	*pic_b;		// status bar team pic
	char	*banner;	// Team Banner
	char	*sbanner;	// Small Team Banner
	char	*skin;		// Team Skin
	char	*ctfflag;	// Team Flag
	char	*capwav;	// Capture Wav

	family_t objects;	
} team_t;

typedef struct {
	int		total_kills;
	int		total_deaths;
	int		total_suicides;
	int		carriers_killed;
	int		chasers_killed;
	int		games;
	float	game_time;
	float	carry_time;
	float	empty_time;
	float	total_damage;
	float	total_skill;
	float	defensive_skill;
	float	offensive_skill;
	float	role_potential;
	float	role_effectiveness;
} pstats_t;

// struct radio_s;
typedef struct radio_s {
	int		playing;
	float	len;
	float	expire;
	char	*msg;
	struct radio_s *prev;
	struct radio_s *next;	// Not required but we're paranoid and don't want to get lost..
} radio_t;

extern team_t	teams[MAX_TEAMS];
#define	TMOFS(x) (int)&(((object_t *)0)->x)
#define	PMOFS(x) (int)&(((object_t *)0)->x)

#ifndef _INIFILE__H
#define _INIFILE__H
#endif

#define _TRUE_ 1
#define _FALSE_ 0

///////////////////////////////////////////////
// Structure for an Ini file entry. Do NOT use these
// on their own.
///////////////////////////////////////////////
typedef struct {
	int first_non_space;
	int equals_position;
	int last_letter;
	int first_after_equals;
	int last_after_equals;

	int number_of_entries;
	char **entry_names;		// the names of the entries in the section
	char **entry_values;	// the values of each entry.
} IniEntry;

///////////////////////////////////////////////////////////////
// Structure for an Ini file. This is the one you use.
///////////////////////////////////////////////////////////////
typedef struct {
	int open_bracket_pos;
	int close_bracket_pos;

	char ini_file_read;		// whether the ini file was opened or not
	int number_of_lines;	// the number of lines in the .ini file
	int number_of_sections;	// the number of individual sections in the .ini file

	char *filename;			// copy of the source filename
	char **inifile;			// copy of the .ini file
	char **section_names;	// the names of the sections in the file
	long int *section_pos;	// the lines of the file the sections start on
	IniEntry **entries;		// the entries for each section
} IniFile;

// system_t contains all of the server parameters for a game,
// Initially read from an INI file, maps may then modify certain
// parameters within the structure to fit map specific goals
typedef struct dlsys_s {
	qboolean	player_profiles;
	qboolean	auto_team;
	qboolean	map_voting;
	qboolean	force_motd;
	int			playmode;
	int			num_teams;
	float		respawn_delay;
	team_t		teams[MAX_TEAMS];
	cvar_t		*gamedir;
	cvar_t		*inidir;
	IniFile		*sysIni;
} dlsys_t;

// extern static dlsys_t *dlsys;
extern char *motdLines[255];
extern int numMOTDLines;

extern int blue_targ_sup, blue_sup;
extern int red_targ_sup, red_sup;

extern char *ED_NewString (char *string);
extern qboolean IsFemale(edict_t *ent);

// dl_weapons.c
extern ammo_t	*GetAmmoByName(char *name);
extern weap_t	*GetWeaponByName(char *name);
extern effect_t	*GetEffectByName(char *name);
extern ammo_t	*GetCompatibleAmmo(char *name);

extern void fire_pellet (edict_t *self, edict_t *attacker, vec3_t start, vec3_t aimdir, float range, float damage, ammo_t *amm);
extern edict_t *fire_projectile(edict_t *self, edict_t *attacker, vec3_t start, vec3_t aimdir, float speed, float damg, ammo_t *amm, weap_t *weap);
extern void GenericMountedWeapon(edict_t *ent, edict_t *attacker, vec3_t origin, vec3_t dir, weap_t *weap, ammo_t *amm);

// dl_effects.c
extern void apply_effects(char *effect, edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t src_org, vec3_t dest_org, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm);

// dl_menus.c
pmenu_t MakeMenuItem(char *mformat, ...);

extern void PreviousMenu(edict_t *ent, pmenu_t *p);
extern void ExitMenu(edict_t *ent, pmenu_t *p);
extern void OpenTeamMenu(edict_t *ent, pmenu_t *p);
extern void OpenShopMenu(edict_t *ent, pmenu_t *p);
extern void OpenMainMenu(edict_t *ent, pmenu_t *p);
extern void OpenRadioMenu(edict_t *ent, pmenu_t *p);
extern void OpenSelectObjectsMenu(edict_t *ent, pmenu_t *p);
extern void OpenBuildingMenu(edict_t *ent, pmenu_t *p);
extern void OpenBuildingMenu(edict_t *ent, pmenu_t *p);
extern void OpenTriggerMenu(edict_t *ent, pmenu_t *p);

extern void ShowPlayerMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowConfigMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowTeamMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowShopMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowMainMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowRadioMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowSelectObjectsMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowBuildMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowTriggerMenu(edict_t *ent, pmenuhnd_t *hnd);
extern void ShowTargetMenu(edict_t *ent, pmenuhnd_t *hnd);

void ShowBuildingMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowAmmoStats(edict_t *ent, pmenuhnd_t *hnd);
void ShowWeaponStats(edict_t *ent, pmenuhnd_t *hnd);
void ShowAmmoVariantsMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowAmmoTypesMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowWeaponsMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowSuppliesMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowSupplyStats(edict_t *ent, pmenuhnd_t *hnd);
void ShowSupplyQuantityMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowInventoryMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowAmmoQuantityMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowProfilesMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowPersonalMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowObjectPropertiesMenu (edict_t *ent, pmenuhnd_t *hnd);
void ShowMapVoteMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowTeamSelectionMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowUniformSelectionMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowPlayerStatsMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowAlternateMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowCreditsMenu(edict_t *ent, pmenuhnd_t *hnd);
void ShowMOTDMenu(edict_t *ent, pmenuhnd_t *hnd);

// dl_items.c
extern supply_t *GetSupplyByName(char *name);

extern void AddToTeamObjects (team_t *team, int type, edict_t *ent, edict_t *owner, char *name, char *type_name, qboolean visible);
extern void RemoveFromTeamObjects (object_t *obj);
extern object_t *FindTeamObject(family_t *family, int fieldofs, int match);

extern void AddChild(family_t *family, void *child);
extern void RemoveChild(family_t *family, void *child);

extern void Grapple_Release(edict_t *ent);
extern void Grapple_DoMovement(edict_t *ent, pmove_t *pm);

extern edict_t *SetTrigger(edict_t *owner, edict_t *anchor, vec3_t pos, vec3_t dir, edict_t *target, vec3_t endpos, supply_t *type);
extern void Tool_Use(edict_t *ent, supply_t *supl);
extern void ReclaimTrigger(edict_t *player, edict_t *trig);
extern void Grenade_Use(edict_t *ent, supply_t *type);
extern void SmartScope_Think(edict_t *ent);
extern void SmartScope_Use(edict_t *ent, supply_t *supl);
extern void	CheckDisguise(edict_t *ent);
extern void	Uniform_Use(edict_t *ent, supply_t *supl);
extern void UseSupply(edict_t *ent, char *name);
extern void Supply_Use(edict_t *ent, supply_t *sup);

extern qboolean IsMsgJammed(edict_t *ent);
extern void Radio_Tune(edict_t *ent, int channel);
extern void Radio_Chat(edict_t *ent, char *msg);
extern void Radio_Use(edict_t *ent, char *msg);
extern void Radio_CheckQue(edict_t *ent);
extern void Voice_Use(edict_t *ent, char *msg);

// dl_utils.c
extern int  GetMD2FrameSet(const char *filename, const char *frameset, int *start, int *end);
extern void GetPPMFrameSet(gclient_t *client, int ppmState, int *start, int *end);
extern float GetWAVLength(const edict_t *ent, const char *wavfile);

extern int TempEntType(int te);
extern void CastExplosion(int te, const edict_t *ent, vec3_t origin);
extern void CastFountain (int te, vec3_t origin, vec3_t normal, int count, int color, int misc);
extern void CastTrail (int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t dest, vec3_t offset, int misc);
extern void MCastTempEnt(int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, int mcast);
extern void UCastTempEnt(int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, const edict_t *utarg);
extern void MuzzleFlash (const edict_t *ent, int flags, multicast_t who);
extern void MuzzleFlash2 (const edict_t *ent, int flags, multicast_t who);
extern void unicastSound(const edict_t *player, int soundIndex, float volume);

/*
extern int TempEntType(int te);
extern void CastExplosion(int te, edict_t *ent, vec3_t origin);
extern void CastFountain (int te, vec3_t origin, vec3_t normal, int count, int color, int misc);
extern void CastTrail (int te, edict_t *ent, edict_t *targ, vec3_t origin, vec3_t dest, vec3_t offset, int misc);
extern void MCastTempEnt(int te, edict_t *ent, edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, int mcast);
extern void UCastTempEnt(int te, edict_t *ent, edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, edict_t *utarg);
*/

extern void DeadLodeInit();
extern int	IsIdent(const char *str);
extern int	DL_GetIdentValue(const char *ident, int *result);
extern void DL_StrToType(const char *value, byte *b, field_t *f);
extern float DL_FindFarSide (vec3_t origin, vec3_t dir, vec3_t fside, float maxdist);
extern void DL_CalcBlend(edict_t *ent);
extern void SV_AddBlend (float r, float g, float b, float a, float *v_blend);

extern char *ReadEntFile(const char *mapname);
extern void WriteEntFile(const char *mapname, const char *entities);

extern void ReadAmmoIniFile(const char *infile, qboolean clean);
extern void ReadWeaponsIniFile(const char *infile, qboolean clean);
extern void ReadEffectsIniFile(const char *infile, qboolean clean);
extern void ReadSuppliesIniFile(const char *infile, qboolean clean);
extern void ReadBuildingsIniFile(const char *infile, qboolean clean);
// extern void ReadMeansOfDeathIniFile(const char *infile, qboolean clean);
extern void ValidateIniEntries(void);

extern void ProjectClient (vec3_t dest, edict_t *ent, qboolean offhand);
extern void VectorRotate(vec3_t in, vec3_t angles, vec3_t out);
extern void BBoxRotate(vec3_t in, vec3_t angles, vec3_t minOut, vec3_t maxOut);
extern void stick(edict_t *self, edict_t *other);
extern void DL_ChangeAngles (vec3_t start, vec3_t ideal, vec3_t speed, vec3_t dest);
extern qboolean LineOfSight (vec3_t start, vec3_t end, int mask);
extern float AngleDiff (float angle1, float angle2);
extern void AnglesDiff (vec3_t angle1, vec3_t angle2, vec3_t angleOut);
extern void AnglesMod(vec3_t in, vec3_t out);
extern int InFOV (vec3_t origin, vec3_t dir, vec3_t targ, float fov, qboolean dof);
extern edict_t *FindInFOV(vec3_t origin, vec3_t dir, float fov, int maxdist);
extern char *strnsub(const char *pszString, const char *pszPattern, const char *pszReplacement, int iMaxLength);
extern int GetHitLocation (vec3_t point, edict_t *ent);
extern char *MeansOfDeath (const char *in, const char *victim, const char *killer, const char *device, const char *weapname, const char*ammoname, const char *basename, const char *locname, qboolean female);
extern char *GetDeathMessage (int mod, const char *section, const char *entry);
extern void stuffcmd(const edict_t *ent, const char *s);
extern int sub_count(const char *in);
extern char *sub_string(const char *in, char *out, int n);

extern void SearchDeadBodies(edict_t *self);
extern char *PicNParseMsg (const char *in, const char *a_player, const char *a_team, const char *o_team);
extern void RecountTeamPlayers(void);
extern team_t *AssignToTeam(void);
extern qboolean DL_StartClient(edict_t *ent);
extern char *DL_AssignSkin(edict_t *ent, char *s);

extern int DL_strcmp(const char *s1, const char *s2, int len, qboolean casesen);
extern char *DL_strcpy(char *dest, const char *src, int max);
extern qboolean DL_FileExist(const char *filename);

//dl_ini.c
extern void *TagReAlloc(void *in, size_t newsize, int tag);
extern IniFile *Ini_ReadIniFile(const char *filename);
extern int Ini_WriteIniFile(IniFile *iniFile);
extern int FindSection(IniFile *iniFile, const char *section);
extern char**	Ini_SectionEntries(IniFile *iniFile, const char *section);
extern int		Ini_NumberOfEntries(IniFile *iniFile, const char *section);
extern int		Ini_RemoveEntry(IniFile *iniFile, const char *section, const char *entry);
extern int		Ini_RemoveSection(IniFile *iniFile, const char *section);
extern void		Ini_FreeIniFile(IniFile *iniFile);
extern char*	Ini_ReadString(IniFile *iniFile, const char *section, const char *entry, char* def);
extern double	Ini_ReadDouble(IniFile *iniFile, const char *section, const char *entry, double def);
extern int		Ini_ReadInteger(IniFile *iniFile, const char *section, const char *entry, int def);
extern void		Ini_WriteString(IniFile *iniFile, const char *section, const char *entry, char *val);
extern void		Ini_WriteDouble(IniFile *iniFile, const char *section, const char *entry, double val);
extern void		Ini_WriteInteger(IniFile *iniFile, const char *section, const char *entry, int val);

// dl_eval.c
extern int evaluate(char *line, double *val, float damage, float distance, float range);


// dl_inventory.c
extern char *GetItemName(item_t *item);
extern float GetItemWeight(item_t *item);
extern item_t *AddToInventory(edict_t *ent, int type, void *item, int priority, int count, int max, item_t *depend, char *altcmd);
extern int CountUniqueItems(edict_t *ent);
extern int CountTotalItems(edict_t *ent);
extern item_t *FindInventoryItem(edict_t *ent, void *item);
extern item_t *FindItemByName(edict_t *ent, const char *name, int type);
extern item_t *FindItemByAltCmd(edict_t *ent, const char *name, int type);
extern item_t *RemoveInventoryItem(edict_t *ent, item_t *item, int num);
extern item_t *MoveToInventory(edict_t *from, item_t *item, int num, edict_t *to);
extern void ClearInventory(edict_t *ent);
extern void CleanInventory(edict_t *ent);

extern item_t *FindPrevWeapon(edict_t *ent, item_t *curweap);
extern item_t *FindNextWeapon(edict_t *ent, item_t *curweap);
extern item_t *FindPreferedWeapon(edict_t *ent);
extern qboolean IsAmmoCompatible(weap_t *weap, ammo_t *ammo);
extern ammo_t *FindPrevCompatibleAmmo(edict_t *ent, ammo_t *curammo);
extern ammo_t *FindNextCompatibleAmmo(edict_t *ent, ammo_t *curammo);
extern item_t *FindPreferedCompatibleAmmo(edict_t *ent, char *ammoType);

extern float GetInventoryWeight(edict_t *ent);

// dl_profiles.c
extern float GetProfileWeight(IniFile *IHnd, int max);
extern char *ValidateProfileName(char *in, char *out);
extern void LoadProfile(edict_t *ent, IniFile *IHnd, int flags, float factor);
extern void LoadSystemProfile(edict_t *ent, char *profile, int flags, float factor);
extern qboolean OpenPlayerProfile(edict_t *ent);
// extern void WriteUserProfile(edict_t *ent, char *key);
extern void AddItemToProfile(edict_t *ent, char *itemName, int type, int count, int max, int priority, char *depend, char *altcmd);
extern void DL_Cmd_AddItem(edict_t *ent);
extern void DL_Cmd_DropItem(edict_t *ent);
extern void DL_Cmd_ClearProfile(edict_t *ent);
extern void DL_Cmd_ShowProfile(edict_t *ent);
extern void DL_Cmd_SetAltCmd(edict_t *ent);
extern void DL_DoPlayerStats(edict_t *targ, edict_t *attacker, int mod);
extern void DL_ReadPlayerStats(edict_t *ent);
extern void DL_WritePlayerStats(edict_t *ent);

// dl_buildings.c
extern void Camera_update (edict_t *ent);
extern void Camera_clientupdate (edict_t *ent);
extern edict_t *Camera_next (edict_t *ent);
extern void Camera_toggle(edict_t *ent);
extern void Camera_select (edict_t *ent, edict_t *camera);

extern building_t *GetBuildingByName(const char *name);
extern void ReleaseBuildings(edict_t *ent);
extern int CheckBuildingPoints(edict_t *ent);
extern void Building_pain(edict_t *self, edict_t *other, float kick, int damage);
extern void Building_finish(edict_t *building);
extern int CanBuildBuilding (edict_t *ent, building_t *type);
extern qboolean Building_build (edict_t *ent, building_t *type);
extern void MakeBuilding(edict_t *ent, char *name);
extern void SelectBuilding(edict_t *ent, pmenu_t *p);

// dl_map.c
extern void SP_dl_player_start(edict_t *self);
extern edict_t *SelectDLTeamSpawnPoint (edict_t *ent);
extern void ReturnGoalItem(edict_t *ent);
extern void DropGoalItem(edict_t *ent);
extern void Touch_Supply (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void Think_Supply (edict_t *self);
extern void SP_dl_ctfflag1(edict_t *ent);
extern void SP_dl_ctfflag2(edict_t *ent);
extern void SP_dl_item_supply(edict_t *ent);
extern void SP_dl_trigger_model(edict_t *ent);
extern void SP_dl_target_resupply(edict_t *ent);
extern void SP_dl_target_teleport(edict_t *ent);
extern void SP_dl_target_bubblespawn(edict_t *ent);
extern void SP_dl_target_award(edict_t *ent);
extern void SP_dl_force_lights(edict_t *ent);
extern void SP_dl_target_radio(edict_t *ent);
extern void SP_dl_target_menu(edict_t *ent);
extern void SP_dl_trigger_sequence(edict_t *ent);
extern void SP_dl_trigger_random (edict_t *self);
extern void SP_dl_trigger_timer(edict_t *ent);
extern void SP_dl_target_shooter (edict_t *self);
extern void SP_dl_target_effect (edict_t *self);
extern void SP_dl_item_effect (edict_t *self);
extern void SP_dl_target_giveitem (edict_t *self);

extern void SP_dl_func_rangetarget (edict_t *ent);


extern int g_mzflash;

extern int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);


// dl_admin.c
extern int DL_GetMapVotes(const char *name);
extern const char *DL_TallyMapVotes(void);


// PM (11/29/1998)
// dl_light.c (flames, torches, lavaballs, etc.)
extern void SP_light_torch_small_walltorch (edict_t *ent);
extern void SP_light_torch_small_bowltorch (edict_t *ent);
extern void SP_light_flame_large_yellow (edict_t *ent);
extern void SP_light_flame_small_yellow (edict_t *ent);
extern void SP_light_flame_small_white (edict_t *ent);
extern void SP_misc_fireball (edict_t *ent);

#define FRAME_FIRST_SMALLIGNITE   0
#define FRAME_LAST_SMALLIGNITE    2
#define FRAME_FIRST_SMALLFIRE     3
#define FRAME_LAST_SMALLFIRE     14
#define FRAME_FIRST_LARGEIGNITE  15
#define FRAME_LAST_LARGEIGNITE   20
#define FRAME_FIRST_LARGEFIRE    21
#define FRAME_LAST_LARGEFIRE     32
#define FRAME_FIRST_FLARE        33
#define FRAME_LAST_FLARE         38
#define FRAME_FLAMEOUT           39

#define FRAME_FIRST_TORCH         0
#define FRAME_LAST_BOWLTORCH      5
#define FRAME_LAST_WALLTORCH     11

#define SKIN_FIRE       0
#define SKIN_FIRE_BLUE  1
#define SKIN_FIRE_RED   2

#define MD2_FIRE        gi.modelindex("models/flames/fire/tris.md2")
#define MD2_WALLTORCH   gi.modelindex("models/flames/w_torch/tris.md2")
#define MD2_BOWLTORCH   gi.modelindex("models/flames/b_torch/tris.md2")
#define MD2_LAVABALL    gi.modelindex("models/flames/lavaball/tris.md2")

#define WAV_FIRE        gi.soundindex ("flames/fire.wav")

// PM (11/29/1998)
// min/max
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

// PAK file structs
#define inbounds(x, min, max) ((x) >= (min) && (x) <= (max))

typedef struct
{
	FILE	*fd;
	size_t	start;
	size_t	end;
	qboolean text;
} PFILE;

typedef struct
{
	char	ident[4];
	size_t	toc_offset;
	size_t	toc_size;
} pak_header_t;

typedef struct
{
	char	filename[56];
	size_t	offset;
	size_t	size;
} pak_tocentry_t;

#define TOC_SIZE	64

extern PFILE *pfopen(const char *name, const char *mode);
extern int pfseek(PFILE *pakfile, long offset, int mode);
extern int pftell(PFILE *pakfile);
extern size_t pfread(void *buffer, size_t size, size_t count, PFILE *pakfile);
extern char *pfgets(char *string, int n, PFILE *pakfile);
extern int pfclose(PFILE *pakfile);


extern void DL_DebugLog(char *string);
