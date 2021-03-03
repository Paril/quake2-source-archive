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

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"OpenTDM"

#ifdef OPENTDM_RELEASE
#define	OPENTDM_VERSION	"1.0"
#else
#define	OPENTDM_VERSION "$Revision$"
//dummy string to force g_local.h commit: asdf
#endif

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_sound			9
#define	svc_stufftext		11
#define	svc_configstring	13

// player_state->stats[] indexes
#define STAT_HEALTH_ICON		0
#define	STAT_HEALTH				1
#define	STAT_AMMO_ICON			2
#define	STAT_AMMO				3
#define	STAT_ARMOR_ICON			4
#define	STAT_ARMOR				5
#define	STAT_SELECTED_ICON		6
#define	STAT_PICKUP_ICON		7
#define	STAT_PICKUP_STRING		8
#define	STAT_TIMER_ICON			9
#define	STAT_TIMER				10
#define	STAT_HELPICON			11
#define	STAT_SELECTED_ITEM		12
#define	STAT_LAYOUTS			13
// frag count for server browsers
#define	STAT_FRAGS				14
#define	STAT_FLASHES			15		// cleared each frame, 1 = health, 2 = armor
#define STAT_CHASE				16
#define STAT_SPECTATOR			17

#define STAT_FIRST_TEAM_NAME_INDEX		18
#define STAT_SECOND_TEAM_NAME_INDEX		19

#define STAT_FIRST_TEAM_STATUS_INDEX	20
#define STAT_SECOND_TEAM_STATUS_INDEX	21

#define STAT_TIME_REMAINING				22

#define STAT_FIRST_TEAM_SCORE			23
#define STAT_SECOND_TEAM_SCORE			24

#define	STAT_TIMEOUT_STRING_INDEX		25

#define	STAT_GAME_STATUS_STRING_INDEX	26

#define	STAT_ID_VIEW_INDEX				27

#define	STAT_VOTE_STRING_INDEX			28

// another timer for pent so it doesn't overwrite quad
#define	STAT_TIMER_PENT_ICON			29
#define	STAT_TIMER_PENT					30
// frags showed in the hud
#define	STAT_SCORE					31
// maximum 31!

// dmflags->value flags
#define	DF_NO_HEALTH		0x00000001	// 1
#define	DF_NO_ITEMS			0x00000002	// 2
#define	DF_WEAPONS_STAY		0x00000004	// 4
#define	DF_NO_FALLING		0x00000008	// 8
#define	DF_INSTANT_ITEMS	0x00000010	// 16
#define	DF_SAME_LEVEL		0x00000020	// 32

//teams handled specially in opentdm, don't need this
//#define DF_SKINTEAMS		0x00000040	// 64
//#define DF_MODELTEAMS		0x00000080	// 128

#define DF_NO_FRIENDLY_FIRE	0x00000100	// 256
#define	DF_SPAWN_FARTHEST	0x00000200	// 512
#define DF_FORCE_RESPAWN	0x00000400	// 1024
#define DF_NO_ARMOR			0x00000800	// 2048
#define DF_ALLOW_EXIT		0x00001000	// 4096
#define DF_INFINITE_AMMO	0x00002000	// 8192
#define DF_QUAD_DROP		0x00004000	// 16384
#define DF_FIXED_FOV		0x00008000	// 32768

#define GAMEMODE_TDM	0
#define GAMEMODE_ITDM	1
#define GAMEMODE_1V1	2

#define CS_TDM_SPECTATOR_STRINGS (CS_GENERAL + MAX_CLIENTS)

#define CS_TDM_TEAM_A_NAME	(CS_GENERAL + 0)
#define CS_TDM_TEAM_B_NAME	(CS_GENERAL + 1)

#define CS_TDM_TEAM_A_STATUS	(CS_GENERAL + 2)
#define CS_TDM_TEAM_B_STATUS	(CS_GENERAL + 3)

#define	CS_TDM_TIMELIMIT_STRING	(CS_GENERAL + 4)

#define CS_TDM_TIMEOUT_STRING	(CS_GENERAL + 5)

#define CS_TDM_GAME_STATUS		(CS_GENERAL + 6)

//per client!
#define CS_TDM_ID_VIEW			(CS_GENERAL + 7)

#define CS_TDM_VOTE_STRING		(CS_GENERAL + 8)

//#define CS_TDM_TEAM_A_PIC		(CS_GENERAL + 5)
//#define CS_TDM_TEAM_B_PIC		(CS_GENERAL + 6)


// otdm flags
//i don't like the way this is done, gametype should be global, not a flag
//#define DF_MODE_TDM			0x00010000	// 65536
//#define DF_MODE_ITDM		0x00020000	// 131072
//#define DF_MODE_1V1			0x00040000	// 262144

// RAFAEL
/*
#define	DF_QUADFIRE_DROP	0x00010000	// 65536

//ROGUE
#define DF_NO_MINES			0x00020000
#define DF_NO_STACK_DOUBLE	0x00040000
#define DF_NO_NUKES			0x00080000
#define DF_NO_SPHERES		0x00100000
*/
//ROGUE

enum
{
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenu_s
{
	const char	*text;
	int			align;
	void		*arg;
	void		(*SelectFunc)(edict_t *ent);
} pmenu_t;

typedef struct pmenuhnd_s
{
	pmenu_t			*entries;
	int				cur;
	int				num;
	qboolean		dynamic;
	qboolean		active;
} pmenuhnd_t;

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, qboolean dynamic);
void PMenu_Close(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);

//==================================================================

// view pitching times
#define DAMAGE_TIME		(0.5f * (1 * SERVER_FPS))
#define	FALL_TIME		(0.3f * (1 * SERVER_FPS))

//#define DAMAGE_TIME		0.5f
//#define	FALL_TIME		0.3f

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
#define	FL_NOCLIP_PROJECTILE	0x00002000	// projectile hack
#define FL_RESPAWN				0x80000000	// used for item respawning

//#define	FRAMETIME		0.05f
extern float	FRAMETIME;
extern int		SERVER_FPS;

//define for variable frametime support
#define SECS_TO_FRAMES(seconds)	(int)((seconds)* SERVER_FPS)
#define FRAMES_TO_SECS(frames)	(int)((frames) * FRAMETIME)

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

/*typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;*/

//r1: this is kind of ugly.. but meh
#define AMMO_SHELLS	(1<<11)
#define	AMMO_BULLETS	(1<<12)
#define AMMO_GRENADES			(1<<13)
#define AMMO_ROCKETS			(1<<14)
#define	AMMO_CELLS		(1<<15)
#define	AMMO_SLUGS		(1<<16)

typedef enum
{
	ITEM_NULL,
	ITEM_ITEM_ARMOR_BODY,
	ITEM_ITEM_ARMOR_COMBAT,
	ITEM_ITEM_ARMOR_JACKET,
	ITEM_ITEM_ARMOR_SHARD,
	ITEM_ITEM_POWER_SCREEN,
	ITEM_ITEM_POWER_SHIELD,
	ITEM_WEAPON_BLASTER,
	ITEM_WEAPON_SHOTGUN,
	ITEM_WEAPON_SUPERSHOTGUN,
	ITEM_WEAPON_MACHINEGUN,
	ITEM_WEAPON_CHAINGUN,
	ITEM_AMMO_GRENADES,
	ITEM_WEAPON_GRENADELAUNCHER,
	ITEM_WEAPON_ROCKETLAUNCHER,
	ITEM_WEAPON_HYPERBLASTER,
	ITEM_WEAPON_RAILGUN,
	ITEM_WEAPON_BFG,
	ITEM_AMMO_SHELLS,
	ITEM_AMMO_BULLETS,
	ITEM_AMMO_CELLS,
	ITEM_AMMO_ROCKETS,
	ITEM_AMMO_SLUGS,
	ITEM_ITEM_QUAD,
	ITEM_ITEM_INVULNERABILITY,
	ITEM_ITEM_SILENCER,
	ITEM_ITEM_BREATHER,
	ITEM_ITEM_ENVIRO,
	ITEM_ITEM_ANCIENT_HEAD,
	ITEM_ITEM_ADRENALINE,
	ITEM_ITEM_BANDOLIER,
	ITEM_ITEM_PACK,
	ITEM_ITEM_HEALTH,
} itemindices_t;

#define GETITEM(x) (itemlist + (x))

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

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

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

#define	TDM_MAX_MAP_SPAWNPOINTS	64

typedef struct matchinfo_s matchinfo_t;
typedef struct teamplayer_s teamplayer_t;

typedef struct gitem_s
{
	const char	*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, const struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, const struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	const char	*pickup_sound;
	const char	*world_model;
	int			world_model_flags;
	const char	*view_model;

	// client side info
	const char	*icon;
	const char	*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	//const char	*ammo;			// for weapons
	int			ammoindex;
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	const char	*precaches;		// string of all models, sounds, and images this item will use
	const char	*shortname;
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	gclient_t	*clients;		// [maxclients]

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	char		gamedir[MAX_QPATH];

	int			server_features;
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	unsigned	time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	int			intermissionframe;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	int			pic_health;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	//tdm stuff
	int				match_start_framenum;
	int				match_end_framenum;
	int				match_score_end_framenum;
	int				next_ready_nag_framenum;
	int				match_resume_framenum;
	const char		*entity_string;
	teamplayer_t	*tdm_timeout_caller;
	int				realframenum;
	int				last_tdm_match_status;
	qboolean		tdm_pseudo_1v1mode;
	int				timeout_end_framenum;
	int				warmup_start_framenum;

	int				numspawns;
	edict_t			*spawns[TDM_MAX_MAP_SPAWNPOINTS];

	time_t			spawntime;
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

/*extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;*/


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
//if altering any of these, be sure to update the weaponToTDMG lookup table!!

extern	int	meansOfDeath;


extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

/*#ifdef random
#undef random
#endif

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0f * (random() - 0.5f))*/

extern	cvar_t	*maxentities;
//extern	cvar_t	*deathmatch;
//extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
//extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*spectator_password;
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
//extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*flood_waves;
extern	cvar_t	*flood_waves_perminute;
extern	cvar_t	*flood_waves_waitdelay;

extern	cvar_t	*sv_maplist;

extern	cvar_t	*g_team_a_name;
extern	cvar_t	*g_team_b_name;
extern	cvar_t	*g_locked_names;

extern	cvar_t	*g_team_a_skin;
extern	cvar_t	*g_team_b_skin;
extern	cvar_t	*g_locked_skins;

extern	cvar_t	*g_admin_password;
extern	cvar_t	*g_admin_vote_decide;
extern	cvar_t	*g_match_time;
extern	cvar_t	*g_match_countdown;
extern	cvar_t	*g_vote_time;
extern	cvar_t	*g_vote_mask;
extern	cvar_t	*g_intermission_time;
extern	cvar_t	*g_force_screenshot;
extern	cvar_t	*g_force_record;

extern	cvar_t	*g_tdmflags;
extern	cvar_t	*g_itdmflags;
extern	cvar_t	*g_1v1flags;

extern	cvar_t	*g_itemflags;
extern	cvar_t	*g_powerupflags;

extern	cvar_t	*g_tdm_allow_pick;

extern	cvar_t	*g_fast_weap_switch;
extern	cvar_t	*g_teleporter_nofreeze;

extern	cvar_t	*g_overtime;
extern	cvar_t	*g_tie_mode;
extern	cvar_t	*g_gamemode;
extern	cvar_t	*g_respawn_time;
extern	cvar_t	*g_max_timeout;
extern	cvar_t	*g_1v1_timeout;
extern	cvar_t	*g_chat_mode;
extern	cvar_t	*g_idle_time;

extern	cvar_t	*g_http_enabled;
extern	cvar_t	*g_http_bind;
extern	cvar_t	*g_http_proxy;
extern	cvar_t	*g_http_path;
extern	cvar_t	*g_http_domain;

extern	cvar_t	*g_debug_spawns;

extern	cvar_t	*g_maplistfile;
extern	cvar_t	*g_motd_message;

extern	cvar_t	*g_max_players_per_team;

extern	cvar_t	*g_bugs;

extern	cvar_t	*g_allow_name_change_during_match;

extern	cvar_t	*g_allow_vote_config;

extern	cvar_t	*g_command_mask;

extern	cvar_t	*g_auto_rejoin_match;
extern	cvar_t	*g_auto_rejoin_map;

extern	cvar_t	*g_1v1_spawn_mode;
extern	cvar_t	*g_tdm_spawn_mode;

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
extern	const gitem_t	itemlist[];

// a sound without an ent or pos will be a local only sound
#define	SND_VOLUME		(1<<0)		// a byte
#define	SND_ATTENUATION	(1<<1)		// a byte
#define	SND_POS			(1<<2)		// three coordinates
#define	SND_ENT			(1<<3)		// a short 0-2: channel, 3-12: entity
#define	SND_OFFSET		(1<<4)		// a byte, msec offset from frame start

#define DEFAULT_SOUND_PACKET_VOLUME	1.0f
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0f

//
// g_cmds.c
//
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0);

//
// g_items.c
//
void PrecacheItem (const gitem_t *it);
void InitItems (void);
void SetItemNames (void);
const gitem_t	*FindItem (const char *pickup_name);
//const gitem_t	*FindItemByClassname (const char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, const gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SetVWepInfo (edict_t *ent);
void SpawnItem (edict_t *ent, const gitem_t *item);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
const gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, const gitem_t *item, int count);
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

void	G_StuffCmd (edict_t *e, const char *fmt, ...);
void	G_UnicastSound (edict_t *ent, int index, qboolean reliable);
void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);
char	*G_StripQuotes (char *string);

char	*G_CopyString (const char *in);

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

//
// g_main.c
//
void EndDMLevel (void);

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
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other, int mask);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);


//
// g_client.c
//
void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
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
qboolean SV_FilterPacket (const char *from);

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
void ValidateSelectedItem (edict_t *ent);
//void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void DeathmatchScoreboard (edict_t *ent);

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
// g_chase.c
//
void UpdateLockCam(edict_t *ent);
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);
void DisableChaseCam (edict_t *ent);
void NextChaseMode (edict_t *ent);
void SetChase (edict_t *ent, edict_t *target);

//opentdm

typedef enum
{
	MM_INVALID,
	MM_WARMUP,
	MM_COUNTDOWN,
	MM_PLAYING,
	MM_TIMEOUT,
	MM_OVERTIME,
	MM_SUDDEN_DEATH,
	MM_SCOREBOARD,
} matchmode_t;

void ParseEntityString (qboolean respawn);

void TDM_Init (void);
qboolean TDM_SetupClient (edict_t *ent);
void TDM_TeamsChanged (void);
void TDM_ShowTeamMenu (edict_t *ent);
void TDM_UpdateConfigStrings (qboolean forceUpdate);
void TDM_SetInitialItems (edict_t *ent);
void TDM_SetCaptain (int team, edict_t *ent);
qboolean TDM_Command (const char *cmd, edict_t *ent);
void TDM_CheckMatchStart (void);
void TDM_CheckTimes (void);
void TDM_ResetGameState (void);
char *TDM_ScoreBoardString (edict_t *ent);
void TDM_MapChanged (void);
void TDM_LeftTeam (edict_t *ent, qboolean notify);
void TDM_Disconnected (edict_t *ent);
qboolean TDM_ServerCommand (const char *cmd);
void TDM_UpdateTeamNames (void);
void TDM_SetupSounds (void);
char *TDM_SetColorText (char *buffer);
void TDM_PlayerNameChanged (edict_t *ent);
void TDM_WeaponFired (edict_t *ent);
void TDM_Error (const char *fmt, ...);

void TDM_BeginDamage (void);
void TDM_Damage (edict_t *ent, edict_t *victim, edict_t *inflictor, int damage);
void TDM_EndDamage (void);

void TDM_ItemSpawned (edict_t *ent);
void TDM_ItemGrabbed (edict_t *ent, edict_t *player);

void TDM_MacroExpand (edict_t *ent, char *text, int maxlength);
void ToggleChaseCam (edict_t *ent);
void TDM_UpdateSpectatorsOnEvent (int spec_mode, edict_t *target, edict_t *killer);

void CountPlayers (void);
void UpdatePlayerTeamMenu (edict_t *ent);
void UpdateTeamMenu (void);

extern matchmode_t	tdm_match_status;
extern qboolean		tdm_settings_not_default;
//extern pmenu_t joinmenu[];
#define MENUSIZE_JOINMENU 18

void HTTP_RunDownloads (void);

//============================================================================

#define	TEAM_SPEC	0
#define TEAM_A		1
#define TEAM_B		2
#define	MAX_TEAMS	3

//combined, these must be < 64 plus some slop for other configstring text
#define	MAX_TEAMNAME_LENGTH 16
#define	MAX_TEAMSKIN_LENGTH	24

//this structure is used to hold information about match
//participants until the match is over, when it is copied
//to oldmatch. this allows us to track stats/scores even
//for disconnected players and such.
enum
{
	TDMG_INVALID,
	TDMG_WORLD,
	TDMG_BLASTER,
	TDMG_SHOTGUN,
	TDMG_SSHOTGUN,
	TDMG_MACHINEGUN,
	TDMG_CHAINGUN,
	TDMG_HANDGRENADE,
	TDMG_GRENADELAUNCHER,
	TDMG_ROCKETLAUNCHER,
	TDMG_HYPERBLASTER,
	TDMG_RAILGUN,
	TDMG_BFG10K,
	TDMG_MAX,
};

struct teamplayer_s
{
	char		name[16];
	unsigned	team;
	unsigned	ping;
	unsigned	joincode;

	//actual players killed
	unsigned	enemy_kills;
	unsigned	team_kills;
	unsigned	suicides;
	unsigned	deaths;
	unsigned	telefrags;

	//individual weapons
	unsigned	shots_fired[TDMG_MAX];
	unsigned	shots_hit[TDMG_MAX];

	//incremented each time a kill is made with a weapon or a death is incurred by one
	unsigned	killweapons[TDMG_MAX];
	unsigned	deathweapons[TDMG_MAX];

	unsigned	quad_kills;
	unsigned	quad_deaths;
	unsigned	pent_kills;
	unsigned	pent_deaths;

	//total damage per weapon dealt/received
	unsigned	damage_dealt[TDMG_MAX];
	unsigned	damage_received[TDMG_MAX];

	unsigned	quad_dealt;
	unsigned	quad_recvd;
	unsigned	pent_dealt;
	unsigned	pent_recvd;

	//count damage dealt/received on teammates
	unsigned	team_dealt;
	unsigned	team_recvd;

	unsigned	items_collected[MAX_ITEMS];

	//pointer to the entity - NULL if they disconnected
	edict_t		*client;

	//preserved for joincode
	edict_t		*saved_entity;
	gclient_t	*saved_client;

	//link to matchinfo
	matchinfo_t	*matchinfo;
};

struct matchinfo_s
{
	int				game_mode;
	int				timelimit;
	int				winning_team;
	int				scores[MAX_TEAMS];
	char			mapname[MAX_QPATH];
	char			scoreboard_string[1400];
	unsigned		item_spawn_count[MAX_ITEMS];

	char			teamnames[MAX_TEAMS][MAX_TEAMNAME_LENGTH];

	//base of teamplayers array
	teamplayer_t	*teamplayers;
	teamplayer_t	*captains[MAX_TEAMS];
	int				num_teamplayers;
	qboolean		is1v1;
	int				max_players_per_team;
};

extern matchinfo_t	current_matchinfo;
extern matchinfo_t	old_matchinfo;

typedef enum
{
	VOTE_NOT_ENOUGH_VOTES,
	VOTE_NOT_SUCCESS,
	VOTE_SUCCESS,
} vote_success_t;

typedef struct vote_s
{
	qboolean		active;
	qboolean		applying;
	unsigned		flags;
	edict_t			*victim;
	edict_t			*initiator;
	int				end_frame;
	vote_success_t	success;

	unsigned		newtimelimit;
	unsigned		newweaponflags;
	unsigned		newpowerupflags;
	char			newmap[MAX_QPATH];
	int				telemode;
	int				switchmode;
	int				tiemode;
	int				gamemode;
	int				newchatmode;
	int				bugs;
	unsigned		overtimemins;
	char			configname[MAX_QPATH];
	char			*vote_string;

	edict_t			*last_initiator;
	unsigned		last_vote_end_frame;

	int				spawn_mode;
} vote_t;

typedef struct vote_menu_s
{
	unsigned		timelimit;
	unsigned		bfg;
	unsigned		powerups;
	unsigned		gamemode;
	unsigned		chat;
	unsigned		bugs;
	int				overtime;
	int				map_index;		// index in array
	int				cfg_index;		// index in array
	char			map[MAX_QPATH];
	char			config[MAX_QPATH];
	edict_t			*kick;

	char			string_gamemode[32];
	char			string_map[32];
	char			string_config[32];

	char			string_timelimit[32];
	char			string_overtime[32];
	char			string_powerups[32];
	char			string_bfg[32];

	char			string_kick[32];
	char			string_chat[32];
	char			string_bugs[32];

	qboolean		decrease;
	qboolean		show;
} vote_menu_t;

typedef struct join_menu_s
{
	char			string_teamJoinText[MAX_TEAMS][32];
} join_menu_t;

enum
{
	SND_DEATH1,
	SND_DEATH2,
	SND_DEATH3,
	SND_DEATH4,
	SND_FALL1,
	SND_FALL2,
	SND_GURP1,
	SND_GURP2,
	SND_JUMP1,
	SND_PAIN25_1,
	SND_PAIN25_2,
	SND_PAIN50_1,
	SND_PAIN50_2,
	SND_PAIN75_1,
	SND_PAIN75_2,
	SND_PAIN100_1,
	SND_PAIN100_2,
	SND_MAX,
};

typedef struct
{
	int			players;
	int			score;
	char		name[MAX_TEAMNAME_LENGTH];
	char		skin[MAX_TEAMSKIN_LENGTH];
	char		statname[32];
	char		statstatus[16];
	qboolean	locked;
	qboolean	speclocked;
	qboolean	ready;
	edict_t		*captain;
} teaminfo_t;

extern int soundcache[MAX_SOUNDS];

typedef enum
{
	JS_FIRST_JOIN,
	JS_JOINED,
} joinstate_t;

typedef enum
{
	VOTE_HOLD,
	VOTE_YES,
	VOTE_NO
} player_vote_t;

typedef enum
{
	DL_NONE,
	DL_CONFIG,
	DL_PLAYER_CONFIG,
	DL_POST_STATS,
} dltype_t;

typedef struct tdm_download_s
{
	edict_t		*initiator;
	unsigned	unique_id;
	dltype_t	type;
	char		name[32];
	char		path[1024];
	void		(*onFinish)(struct tdm_download_s *, int, byte *, int);
	qboolean	inuse;
} tdm_download_t;

typedef struct
{
	int			auto_record;
	int			auto_screenshot;
	char		teamskin[64];
	char		enemyskin[64];

	int			id_highlight;
	int			id_x;
	int			id_y;

	qboolean	loaded;
} playerconfig_t;

extern matchmode_t	tdm_match_status;
extern teaminfo_t	teaminfo[MAX_TEAMS];

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6

enum
{
	CHASE_EYES,
	CHASE_THIRDPERSON,
	CHASE_FREE,
	CHASE_MAX,
	CHASE_LOCK,
};

enum
{
	SPEC_NONE,
	SPEC_INVUL,
	SPEC_QUAD,
	SPEC_KILLER,
	SPEC_LEADER,
};

typedef enum
{
	GRENADE_NONE,
	GRENADE_BLEW_UP,
	GRENADE_THROWN,
} grenade_state_t;

// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	char		ip[24];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet
	qboolean	admin;

	unsigned		team;
	joinstate_t		joinstate;
	const gitem_t	*last_weapon;
	qboolean		shown_motd;
	qboolean		disable_id_view;
	qboolean		specinvite[MAX_TEAMS];

	unsigned		mute_frame;		// mute player while game framenum < this
	qboolean		mvdclient;

	// persist menu across death
	pmenuhnd_t		menu;

	// stored values displayed in the menu
	vote_menu_t		votemenu_values;
	join_menu_t		joinmenu_values;

	// we need separate menus for all players
	pmenu_t			votemenu[19];
	pmenu_t			joinmenu[19];

	// per-client download handle (configs)
	tdm_download_t	download;
	playerconfig_t	config;
	unsigned		uniqueid;
} client_persistant_t;

typedef struct
{
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	unsigned	flood_locktill;		// locked from talking
	unsigned	flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	unsigned	flood_waves_locktill;		// locked from using waves
	unsigned	flood_waves_when[10];		// when wave were made
	int			flood_waves_whenhead;		// head pointer for when made

	qboolean		ready;
	unsigned		last_command_frame;
	edict_t			*last_invited_by;
	player_vote_t	vote;
	teamplayer_t	*teamplayerinfo;

	edict_t			*last_id_client;
	int				last_id_health;
	int				last_id_armor;
	int				last_id_powerarmor;

	int				spec_mode;				// bitmask
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;
	int				clientNum;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;

	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
	qboolean	showoldscores;			// set layout stat
	qboolean	showmotd;			// set layout stat

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	qboolean	weapon_thunk;

	const gitem_t		*newweapon;

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
	vec3_t		kick_angles_final;	// weapon kicks
	vec3_t		kick_origin;
	vec3_t		kick_origin_final;
	unsigned	kick_origin_start;
	unsigned	kick_origin_end;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	unsigned	next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	unsigned	quad_framenum;
	unsigned	invincible_framenum;
	unsigned	breather_framenum;
	unsigned	enviro_framenum;

	grenade_state_t	grenade_state;
	unsigned	grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	unsigned	pickup_msg_time;

	unsigned	respawn_framenum;	// can respawn when time > this

	edict_t		*chase_target;		// player we are chasing
	qboolean	update_chase;		// need to update chase info?
	int			chase_mode;

	// values saved and restored from edicts when changing levels
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

	const gitem_t	*weapon;
	const gitem_t	*lastweapon;

	unsigned	last_command_frame;
	unsigned	last_activity_frame;

	int32		last_model_frame;
	int			next_animation_frame;
	unsigned	next_weapon_think;
	vec3_t		saved_angles;
};

typedef enum
{
	ENT_INVALID,
	ENT_FUNC_DOOR,
	ENT_FUNC_DOOR_ROTATING,
	ENT_FUNC_AREAPORTAL,
	ENT_GRENADE,
	ENT_FUNC_TRAIN,
	ENT_DOOR_TRIGGER,
	ENT_PLAT_TRIGGER,
	ENT_BODYQUE,
	ENT_GHOST,
	ENT_GIB,
} enttype_t;

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

	const char	*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//

	char		*message;
	const char	*classname;
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
	unsigned	air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	unsigned	nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	unsigned	touch_debounce_time;		// are all these legit?  do we need more/less of them?
	unsigned	pain_debounce_time;
	unsigned	damage_debounce_time;
	unsigned	fly_sound_debounce_time;	//move to clientinfo
	unsigned	last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	unsigned	powerarmor_time;

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

	const gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	//monsterinfo_t	monsterinfo;

	enttype_t	enttype;
};

//server features
// server is able to read clientNum field from gclient_s struct and hide appropriate entity from client
// game DLL fills clientNum with useful information (current POV index)
#define GMF_CLIENTNUM   1

// game DLL always sets 'inuse' flag properly allowing the server to reject entities quickly
#define GMF_PROPERINUSE 2

// server will set '\mvdspec\<version>' key/value pair in userinfo string if (and only if) client is dummy MVD client (this client represents all MVD spectators and is needed for scoreboard support, etc)
#define GMF_MVDSPEC     4

// inform game DLL of disconnects between level changes
#define GMF_WANT_ALL_DISCONNECTS 8
