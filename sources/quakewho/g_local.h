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
#pragma once

#include "q_shared.h"

#include "game.h"

extern	game_import_t	gi;
extern	game_export_t	globals;

// the "gameversion" client command will print this plus compile date
constexpr const char *GAMEVERSION = "quakewho";

// time in milliseconds
using gtime_t = uint64_t;

// protocol bytes that can be directly added to messages
enum
{
	SVC_BAD,

	SVC_MUZZLEFLASH,
	SVC_MUZZLEFLASH2,
	SVC_TEMP_ENTITY,
	SVC_LAYOUT,
	SVC_INVENTORY,
	SVC_NOP,
	SVC_DISCONNECT,
	SVC_RECONNECT,
	SVC_SOUND,					// <see code>
	SVC_PRINT,					// [uint8_t] id [string] null terminated string
	SVC_STUFFTEXT,				// [string] stuffed into client's console buffer, should be \n terminated
	SVC_SERVERDATA,				// [long] protocol ...
	SVC_CONFIGSTRING,			// [int16_t] [string]
	SVC_SPAWNBASELINE,		
	SVC_CENTERPRINT,			// [string] to put in center of the screen
	SVC_DOWNLOAD,				// [int16_t] size [size bytes]
	SVC_PLAYERINFO,				// variable
	SVC_PACKETENTITIES,			// [...]
	SVC_DELTAPACKETENTITIES,	// [...]
	SVC_FRAME,

	SVC_MAX_ENTTYPES
};

//==================================================================

enum soundflags_t : uint8_t
{
	SND_VOLUME          = bit(0),  // a byte
	SND_ATTENUATION     = bit(1),  // a byte
	SND_POS             = bit(2),  // three coordinates
	SND_ENT             = bit(3),  // a short 0-2: channel, 3-12: entity
	SND_OFFSET          = bit(4)  // a byte, msec offset from frame start
};

MAKE_BITFLAGS(soundflags_t);

constexpr vec_t DEFAULT_SOUND_PACKET_VOLUME				= 1.0f;
constexpr soundattn_t DEFAULT_SOUND_PACKET_ATTENUATION	= ATTN_NORM;

//==================================================================

// view pitching times
constexpr gtime_t DAMAGE_TIME		= 500u;
constexpr gtime_t FALL_TIME			= 300u;

// edict->flags
enum edictflags_t : uint32_t
{
	FL_NONE					= 0,
	FL_INWATER				= bit(0),
	FL_GODMODE				= bit(1),
	FL_PARTIALGROUND		= bit(2),	// not all corners are valid
	FL_TEAMSLAVE			= bit(3),	// not the first on the team
	FL_NO_KNOCKBACK			= bit(4)
};

MAKE_BITFLAGS(edictflags_t);

constexpr gtime_t	FRAME_MS		= 100;
constexpr vec_t		FRAME_S			= 0.1f;

// memory tags to allow dynamic memory to be cleaned up
constexpr int32_t TAG_GAME	= 765;		// clear when unloading the dll
constexpr int32_t TAG_LEVEL	= 766;		// clear when loading a new level

enum weaponstate_t : uint8_t
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
};

enum
{
	//AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_GRENADES,
	
	AMMO_TOTAL,
	AMMO_NONE = AMMO_TOTAL
};

using ammo_t = uint8_t;

constexpr int32_t DEFAULT_HEALTH = 200;

enum playerteam_t : uint8_t
{
	TEAM_NONE,

	TEAM_HIDERS,
	TEAM_HUNTERS
};

//gib types
enum gibtype_t : bool
{
	GIB_ORGANIC,
	GIB_METALLIC
};

// handedness values
enum handedness_t : uint8_t
{
	RIGHT_HANDED,
	LEFT_HANDED,
	CENTER_HANDED
};

// edict->movetype values
enum movetype_t : uint8_t
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
};

// gitem_t->weapmodel for weapons indicates model index
enum
{
	WEAP_BLASTER,
	WEAP_SHOTGUN,
	//WEAP_MACHINEGUN,
	WEAP_GRENADELAUNCHER,
	WEAP_TOTAL
};

using gitem_weapmodel_t = uint8_t;

struct gitem_t
{
	void				(*use)(edict_t &ent, gitem_t *item);
	void				(*weaponthink)(edict_t &ent);
	const char			*view_model;
	const char			*vwep_model;
	const char			*icon;
	const char			*pickup_name;
	uint16_t			quantity;
	ammo_t				ammo;
	gitem_weapmodel_t	weapmodel;
	const char			*precaches;
};

#include "entity_iterator.hpp"

//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
extern struct game_locals_t
{
	std::vector<gclient_t>	clients;		// [maxclients]

	// store latched cvars here that we want to get at often
	bool					cheats_enabled;

	inline edict_t &world()
	{
		return *globals.entities.pool;
	}

	struct entity_list_t
	{
		entity_iterator begin() { return entity_iterator(0); }
		entity_iterator end() { return entity_iterator(globals.entities.num); }

		entity_range range(const size_t &start, const size_t &end = -1)
		{
			if (end == static_cast<size_t>(-1))
				return entity_range(start, globals.entities.num);

			return entity_range(start, end);
		}
	} entities;

	struct player_list_t
	{
		entity_iterator begin() { return entity_iterator(1); }
		entity_iterator end();
	} players;
} game;

enum gamestate_t
{
	GAMESTATE_WAITING, // waiting on ready
	GAMESTATE_SPAWNING, // waiting on monster spawns
	GAMESTATE_PLAYING, // playing
	GAMESTATE_INTERMISSION // intermission between rounds
};

//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
extern struct level_locals_t
{
	uint64_t	framenum;
	gtime_t		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	gtime_t		intermissiontime;		// time the intermission was started
	char		*changemap;
	int32_t		exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	imageindex_t	pic_health;

	edict_ref	current_entity;	// entity running from G_RunFrame
	int32_t		body_que;			// dead bodies

	gamestate_t state;
	gtime_t state_time;
	std::unordered_set<struct nav_grid_node*> skip_points;
	std::vector<edict_ref> monsters;
	gtime_t control_delay;
	gtime_t monster_die_time;
	gtime_t last_radar_left;
	gtime_t radar_time;
	size_t max_monsters, end_max_monsters;
	gtime_t monster_kill_time;
	gtime_t round_end;
	bool countdown_sound;
} level;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
extern struct spawn_temp_t
{
	// world vars
	char		*sky;
	vec_t		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;

	int32_t		lip;
	int32_t		distance;
	int32_t		height;
	char		*noise;
	gtime_t		pausetime;
	char		*item;
	char		*gravity;

	vec_t		minyaw;
	vec_t		maxyaw;
	vec_t		minpitch;
	vec_t		maxpitch;
} st;

enum movestate_t : uint8_t
{
	STATE_TOP,
	STATE_BOTTOM,
	STATE_UP,
	STATE_DOWN
};

struct moveinfo_t
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	soundindex_t sound_start;
	soundindex_t sound_middle;
	soundindex_t sound_end;

	vec_t		accel;
	vec_t		speed;
	vec_t		decel;
	vec_t		distance;

	gtime_t		wait;

	// state data
	movestate_t	state;
	vec3_t		dir;
	vec_t		current_speed;
	vec_t		move_speed;
	vec_t		next_speed;
	vec_t		remaining_distance;
	vec_t		decel_distance;
	void		(*endfunc)(edict_t &self);
};


struct mframe_t
{
	void	(*aifunc)(edict_t &self, const vec_t &dist);
	vec_t	dist;
	void	(*thinkfunc)(edict_t &self);
};

using mframe_list_t = std::initializer_list<mframe_t>;

struct mmove_t
{
	int32_t			firstframe;
	int32_t			lastframe;
	mframe_list_t	frame;
	void			(*endfunc)(edict_t &self);
};

struct monsterinfo_t
{
	const mmove_t	*currentmove;
	int32_t			nextframe;
	vec_t			scale;

	void			(*stand)(edict_t &self);
	void			(*idle)(edict_t &self);
	void			(*search)(edict_t &self);
	void			(*walk)(edict_t &self);
	void			(*run)(edict_t &self);

	vec_t			idle_time;
	int32_t			linkcount;

	gtime_t			next_runwalk_check, should_stand_check;
	int32_t			undamaged_skin, damaged_skin;
	gtime_t			death_time;

	void			(*die)(edict_t &self);
	const char		*name;

	gtime_t			stubborn_check_time, stubborn_time;
	gtime_t			slow_turn_check, slow_turn_time, slow_turn_speed;
	gtime_t			follow_check, follow_time;
	edict_ref		follow_ent;
	bool			follow_direction;
	bool			hunter_visible;
};

extern	modelindex_t	sm_meat_index;
extern	soundindex_t	snd_fry;

#define	FOFS(x)		offsetof(edict_t, x)
#define	STOFS(x)	offsetof(spawn_temp_t, x)
#define	LLOFS(x)	offsetof(level_locals_t, x)
#define	CLOFS(x)	offsetof(gclient_t, x)

extern	dmflags_t	dmflags;
extern	cvar_t		*fraglimit;
extern	cvar_t		*timelimit;
extern	cvar_t		*roundlimit;
extern	cvar_t		*password;
extern	cvar_t		*spectator_password;
extern	cvar_t		*needpass;
extern	cvar_t		*g_select_empty;
extern	cvar_t		*dedicated;
					
extern	cvar_t		*sv_gravity;
extern	cvar_t		*sv_maxvelocity;
					
extern	cvar_t		*gun_x, *gun_y, *gun_z;
extern	cvar_t		*sv_rollspeed;
extern	cvar_t		*sv_rollangle;
					
extern	cvar_t		*run_pitch;
extern	cvar_t		*run_roll;
extern	cvar_t		*bob_up;
extern	cvar_t		*bob_pitch;
extern	cvar_t		*bob_roll;
					
extern	cvar_t		*maxspectators;
					
extern	cvar_t		*flood_msgs;
extern	cvar_t		*flood_persecond;
extern	cvar_t		*flood_waitdelay;
					
extern	cvar_t		*sv_maplist;

#define g_edicts	globals.entities.pool

// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
const uint32_t	SPAWNFLAG_NOT_EASY			= bit(8);
const uint32_t	SPAWNFLAG_NOT_MEDIUM		= bit(9);
const uint32_t	SPAWNFLAG_NOT_HARD			= bit(10);
const uint32_t	SPAWNFLAG_NOT_DEATHMATCH	= bit(11);
const uint32_t	SPAWNFLAG_NOT_COOP			= bit(12);

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
enum fieldflags_t : uint8_t
{
	FFL_NONE = 0,
	FFL_SPAWNTEMP = bit(0)
};

MAKE_BITFLAGS(fieldflags_t);

enum fieldtype_t : uint8_t
{
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_VECTOR,
	F_ANGLEHACK,
	F_TIME,
	F_IGNORE
};

struct field_t
{
	std::string_view	name;
	ptrdiff_t			ofs;
	fieldtype_t			type;
	fieldflags_t		flags;
};

extern gitem_t g_weapons[WEAP_TOTAL];

//
// g_items.c
//
void InitItems ();
void ChangeWeapon (edict_t &ent);
void Think_Weapon (edict_t &ent);

//
// g_utils.c
//
bool KillBox (edict_t &ent);

constexpr vec3_t G_ProjectSource (const vec3_t &point, const vec3_t &distance, const vec3_t &forward, const vec3_t &right, const vec3_t &up)
{
	return {
		point[0] + (forward[0] * distance[0]) + (right[0] * distance[1]),
		point[1] + (forward[1] * distance[0]) + (right[1] * distance[1]),
		point[2] + (forward[2] * distance[0]) + (right[2] * distance[1]) + distance[2]
	};
}

edict_ref G_Find (const edict_ref &from, const ptrdiff_t &fieldofs, const char *match);
edict_ref findradius (const edict_ref &from, const vec3_t &org, const vec_t &rad);
edict_ref G_PickTarget (const char *targetname);
void	G_UseTargets (edict_t &ent, edict_t &activator);
void	G_SetMovedir (vec3_t &angles, vec3_t &movedir);

edict_t &G_InitEdict (edict_t &e);
edict_t	&G_Spawn ();
void	G_FreeEdict (edict_t &e);

void	G_TouchTriggers (edict_t &ent);

char	*vtos (const vec3_t &v);

//
// g_spawn.c
//
void SpawnEntities (const char *mapname, const char *entities, const char *spawnpoint);
void ClientThink (edict_t &ent, const usercmd_t &cmd);
qboolean ClientConnect (edict_t &ent, char *userinfo);
void ClientUserinfoChanged (edict_t &ent, const char *userinfo);
void ClientDisconnect (edict_t &ent);
void ClientBegin (edict_t &ent);
void ClientCommand (edict_t &ent);
void ED_CallSpawn (edict_t &ent);
void Cmd_Pos_f(const edict_t &ent);
void DrawPoints();

constexpr brushcontents_t MASK_CLIP = MASK_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP;

using grid_array = array3_t<uint8_t>;

const uint32_t spawn_grid_size = 64;
const uint32_t grid_max = 8192 / spawn_grid_size;

template<>
struct std::hash<grid_array>
{
	size_t operator()(const grid_array& _Keyval) const
	{
		union {
			size_t p;
			grid_array xyz;
		};
		
		xyz = _Keyval;
		return p;
	}
};

template<>
struct std::hash<vec3_t>
{
	size_t operator()(const vec3_t& _Keyval) const
	{
		union {
			size_t p;
			grid_array xyz;
		};
		
		xyz = {
			static_cast<uint8_t>((_Keyval[0] - -4096) / spawn_grid_size),
			static_cast<uint8_t>((_Keyval[1] - -4096) / spawn_grid_size),
			static_cast<uint8_t>((_Keyval[2] - -4096) / spawn_grid_size)
		};

		return p;
	}
};

struct nav_grid_node
{
	grid_array grid_position;
	vec3_t position;
	std::unordered_set<grid_array> connections;
};

extern std::unordered_map<grid_array, nav_grid_node> grid;
extern std::vector<std::pair<grid_array, nav_grid_node*>> nav_points;

using SP_SpawnFunc = void (*)(edict_t &);

struct monster_def_t
{
	SP_SpawnFunc func;
	const char *name;
};

monster_def_t &G_RandomMonster();

//
// g_combat.c
//

// damage flags
enum damageflag_t : uint8_t
{
	DAMAGE_NONE				= 0,
	DAMAGE_RADIUS			= bit(0),	// damage was indirect
	DAMAGE_NO_ARMOR			= bit(1),	// armour does not protect from this damage
	DAMAGE_ENERGY			= bit(2),	// damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		= bit(3),	// do not affect velocity, just view angles
	DAMAGE_BULLET			= bit(4),   // damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	= bit(5),   // armor, shields, invulnerability, and godmode have no effect
};

MAKE_BITFLAGS(damageflag_t);

bool OnSameTeam (const edict_t &ent1, const edict_t &ent2);
bool CanDamage (const edict_t &targ, const edict_t &inflictor);
void T_Damage (edict_t &targ, edict_t &inflictor, edict_t &attacker, const vec3_t &dir, const vec3_t &point, const vec3_t &normal, int32_t damage, int32_t knockback, damageflag_t dflags);
void T_RadiusDamage (edict_t &inflictor, edict_t &attacker, const vec_t &damage, const edict_ref &ignore, const vec_t &radius);

constexpr int32_t DEFAULT_BULLET_HSPREAD	= 300;
constexpr int32_t DEFAULT_BULLET_VSPREAD	= 500;
constexpr int32_t DEFAULT_SHOTGUN_HSPREAD	= 1000;
constexpr int32_t DEFAULT_SHOTGUN_VSPREAD	= 500;
constexpr int32_t DEFAULT_DEATHMATCH_SHOTGUN_COUNT	= 12;
constexpr int32_t DEFAULT_SHOTGUN_COUNT	= 12;
constexpr int32_t DEFAULT_SSHOTGUN_COUNT	= 20;

//
// g_monster.c
//
void M_droptofloor (edict_t &ent);
void walkmonster_start (edict_t &self);
void M_CatagorizePosition (edict_t &ent);
void M_CheckGround (edict_t &ent);
bool M_FidgetCheck (edict_t &ent, const int &percent);
void Unpossess(edict_t &player);

//
// g_misc.c
//
void BecomeExplosion1(edict_t &self);

//
// g_ai.c
//
void ai_stand (edict_t &self, const vec_t &dist);
void ai_move (edict_t &self, const vec_t &dist);
void ai_walk (edict_t &self, const vec_t &dist);
void ai_run (edict_t &self, const vec_t &dist);

bool infront (const edict_t &self, const edict_t &other);
bool visible (const edict_t &self, const edict_t &other);
bool FacingIdeal(const edict_t &self);

//
// g_weapon.c
//
void fire_bullet (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &kick, const int32_t &hspread, const int32_t &vspread);
void fire_shotgun (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &kick, const int32_t &hspread, const int32_t &vspread, const int32_t &count);
void fire_blaster (edict_t &self, const vec3_t &start, const vec3_t &dir, const int32_t &damage, const int32_t &speed, const entity_effects_t &effect);
void fire_grenade (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &speed, const gtime_t &timer, const vec_t &damage_radius);

//
// p_weapon.c
//
void EnsureGoodPosition(edict_t &whomst);
void Use_Weapon (edict_t &ent, gitem_t *inv);
void Weapon_Blaster (edict_t &ent);
void Weapon_Shotgun (edict_t &ent);
void Weapon_Machinegun (edict_t &ent);
void Weapon_GrenadeLauncher (edict_t &ent);
void Possess(edict_t &player, edict_t &monster, const bool &clone);

//
// g_client.c
//
void respawn (edict_t &ent);
void BeginIntermission (edict_t &targ);
void PutClientInServer (edict_t &ent);
void ClientBeginServerFrame (edict_t &ent);
void Cmd_Help_f (edict_t &ent);

//
// g_player.c
//
void player_die (edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point);
void G_TeamWins(const playerteam_t &team);

//
// g_svcmds.c
//
void ServerCommand ();

//
// p_view.c
//
void ClientEndServerFrame (edict_t &ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t &client);
void G_SetStats (edict_t &ent);
void G_SetSpectatorStats (edict_t &ent);
void G_CheckChaseStats (edict_t &ent);
void DeathmatchScoreboardMessage (edict_t &client, const edict_ref &killer);

//
// m_move.c
//
bool M_CheckBottom (edict_t &ent);
bool M_walkmove (edict_t &ent, vec_t yaw, const vec_t &dist);
void M_MoveToGoal (edict_t &ent, const vec_t &dist);
void M_MoveToController (edict_t &ent, const vec_t &dist, const bool &turn);
bool M_GonnaHitSpecificThing(edict_t &ent, edict_t &other);

//
// g_phys.c
//
void G_RunEntity (edict_t &ent);
void ClipVelocity (const vec3_t &in, const vec3_t &normal, vec3_t &out, const vec_t &overbounce);

//
// g_main.c
//
void SaveClientData ();
void G_CheckPlayerReady();
void EmplaceMonsterInGoodSpot(edict_t &ent);

//
// g_chase.c
//
void UpdateChaseCam(edict_t &ent);
void UpdateTargetCam(edict_t &ent);
void ChaseNext(edict_t &ent);
void ChasePrev(edict_t &ent);
void GetChaseTarget(edict_t &ent);

//
// entities
//
void SP_monster_berserk (edict_t &self);
void SP_monster_gladiator (edict_t &self);
void SP_monster_gunner (edict_t &self);
void SP_monster_infantry (edict_t &self);
void SP_monster_soldier_light (edict_t &self);
void SP_monster_soldier (edict_t &self);
void SP_monster_soldier_ss (edict_t &self);
void SP_monster_tank (edict_t &self);
void SP_monster_medic (edict_t &self);
void SP_monster_chick (edict_t &self);
void SP_monster_parasite (edict_t &self);
void SP_monster_brain (edict_t &self);
void SP_monster_mutant (edict_t &self);

void SP_info_player_start (edict_t &ent);
void SP_info_player_deathmatch (edict_t &ent);
void SP_info_player_coop (edict_t &ent);
void SP_info_player_intermission (edict_t &ent);

void SP_func_plat (edict_t &ent);
void SP_func_rotating (edict_t &ent);
void SP_func_button (edict_t &ent);
void SP_func_door (edict_t &ent);
void SP_func_door_secret (edict_t &ent);
void SP_func_door_rotating (edict_t &ent);
void SP_func_water (edict_t &ent);
void SP_func_train (edict_t &ent);
void SP_func_conveyor (edict_t &self);
void SP_func_wall (edict_t &self);
void SP_func_object (edict_t &self);
void SP_func_timer (edict_t &self);
void SP_func_areaportal (edict_t &ent);
void SP_func_clock (edict_t &ent);
void SP_func_killbox (edict_t &ent);

void SP_trigger_always (edict_t &ent);
void SP_trigger_once (edict_t &ent);
void SP_trigger_multiple (edict_t &ent);
void SP_trigger_relay (edict_t &ent);
void SP_trigger_push (edict_t &ent);
void SP_trigger_hurt (edict_t &ent);
void SP_trigger_counter (edict_t &ent);
void SP_trigger_elevator (edict_t &ent);
void SP_trigger_gravity (edict_t &ent);
void SP_trigger_monsterjump (edict_t &ent);

void SP_target_temp_entity (edict_t &ent);
void SP_target_speaker (edict_t &ent);
void SP_target_explosion (edict_t &ent);
void SP_target_splash (edict_t &ent);
void SP_target_blaster (edict_t &ent);
void SP_target_laser (edict_t &self);
void SP_target_earthquake (edict_t &ent);
void SP_target_character (edict_t &ent);
void SP_target_string (edict_t &ent);

void SP_light_mine1 (edict_t &ent);
void SP_light_mine2 (edict_t &ent);
void SP_info_notnull (edict_t &self);
void SP_path_corner (edict_t &self);

void SP_misc_teleporter (edict_t &self);
void SP_misc_teleporter_dest (edict_t &self);

//============================================================================

// client_t->anim_priority
enum animpriority_t : uint8_t
{
	ANIM_BASIC,		// stand / run
	ANIM_WAVE,
	ANIM_JUMP,
	ANIM_PAIN,
	ANIM_ATTACK,
	ANIM_DEATH,
	ANIM_REVERSE
};

using ammo_data_t = std::array<uint32_t, AMMO_TOTAL>;

// client data that stays across multiple level loads
struct client_persistant_t
{
	userinfo_t		userinfo;
	char			netname[16];
	handedness_t	hand;

	bool			connected;			// a loadgame will leave valid entities that
										// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int32_t			health;
	int32_t			max_health;
	edictflags_t	savedFlags;

	ammo_data_t		ammo;

	gitem_t			*weapon;
	gitem_t			*lastweapon;

	int32_t			score;			// for calculating total unit score in coop games

	bool			spectator;			// client is a spectator
};

// client data that stays across deathmatch respawns
struct client_respawn_t
{
	uint64_t		enterframe;			// level.framenum the client entered the game
	int32_t			score;				// frags, etc
	vec3_t			cmd_angles;			// angles sent over in the last command

	bool			spectator;			// client is a spectator
	bool			ready;
	playerteam_t	team;
};

enum : uint8_t
{
	RADAR_EMPTY,

	RADAR_STAGE_1,
	RADAR_STAGE_2,
	RADAR_STAGE_3,

	RADAR_BAD
};

using radar_status_t = uint8_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_t : gclient_server_t
{
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	bool				showscores;			// set layout stat

	button_t			buttons;
	button_t			latched_buttons;

	bool				weapon_thunk;

	gitem_t				*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int32_t				damage_blood;		// damage taken out of health
	int32_t				damage_knockback;	// impact damage
	vec3_t				damage_from;		// origin for vector calculation

	vec_t				killer_yaw;			// when dead, look at killer

	weaponstate_t		weaponstate;
	vec3_t				kick_angles;	// weapon kicks
	vec3_t				kick_origin;
	vec_t				v_dmg_roll, v_dmg_pitch;
	gtime_t				v_dmg_time;					// damage kicks
	gtime_t				fall_time;
	vec_t				fall_value;		// for view drop on fall
	vec4_t				damage_blend;
	vec3_t				v_angle;			// aiming direction
	gtime_t				bobtime;			// so off-ground doesn't change it
	vec3_t				oldviewangles;
	vec3_t				oldvelocity;

	gtime_t				next_drown_time;
	waterlevel_t		old_waterlevel;

	// animation vars
	int32_t				anim_end;
	animpriority_t		anim_priority;
	bool				anim_duck;
	bool				anim_run;

	soundindex_t		weapon_sound;

	gtime_t				flood_locktill;		// locked from talking
	gtime_t				flood_when[10];		// when messages were said
	int32_t				flood_whenhead;		// head pointer for when said

	gtime_t				respawn_time;		// can respawn when time > this

	edict_ref			chase_target;		// player we are chasing
	bool				update_chase;		// need to update chase info?

	bool				control_waitjump;
	usercmd_t			cmd;
	gtime_t				jump_sound_debounce, regen_debounce;
	struct {
		edict_ref		entity;
		radar_status_t	status, last_status = RADAR_BAD;
		vec_t			factor, last_factor = -1;
	} radar;
	bool				temp_spectator;
	int32_t				num_jumps, last_num_jumps;

	static inline edict_t &ClientToEntity(gclient_t &client);

	template<typename ... Args>
	inline void Print(const printflags_t &printlevel, const char *fmt, Args... args)
	{
		gi.cprintf(ClientToEntity(*this), printlevel, fmt, args...);
	}

	template<typename ... Args>
	inline void Print(const char *fmt, Args... args)
	{
		gi.cprintf(ClientToEntity(*this), PRINT_HIGH, fmt, args...);
	}
	
	template<typename ... Args>
	inline void CenterPrint(const char *fmt, Args... args)
	{
		gi.centerprintf(ClientToEntity(*this), fmt, args...);
	}

	inline void SendSound(const soundindex_t &sound, const vec_t volume = DEFAULT_SOUND_PACKET_VOLUME, const soundchannel_t &channel = CHAN_AUTO, const soundattn_t &attn = DEFAULT_SOUND_PACKET_ATTENUATION, const bool &reliable = false);
};

struct edict_t : edict_server_t
{
private:
	edict_t() = default;

public:
	static inline void initialize(edict_t *e)
	{
		new(e) edict_t;
	}

	inline void Reset()
	{
		*this = edict_t();
		s.number = this - g_edicts;
	}

	movetype_t		movetype;
	edictflags_t	flags;

	const char		*model;
	gtime_t			freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char			*message;
	const char		*classname = "noclass";
	uint32_t		spawnflags;

	gtime_t			timestamp;

	vec_t			angle;			// set in qe3, -1 = up, -2 = down
	char			*target;
	char			*targetname;
	char			*killtarget;
	char			*team;
	char			*pathtarget;
	char			*deathtarget;
	char			*combattarget;
	edict_ref		target_ent;

	vec_t			speed, accel, decel;
	vec3_t			movedir;
	vec3_t			pos1, pos2;

	vec3_t			velocity;
	vec3_t			avelocity;
	int32_t			mass;
	gtime_t			air_finished;
	vec_t			gravity = 1.0f;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_ref		goalentity;
	edict_ref		movetarget;
	vec_t			yaw_speed;
	vec_t			ideal_yaw;

	gtime_t			nextthink;
	void			(*prethink) (edict_t &ent);
	void			(*think)(edict_t &self);
	void			(*blocked)(edict_t &self, edict_t &other);	//move to moveinfo?
	void			(*touch)(edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf);
	void			(*use)(edict_t &self, edict_t &other, edict_t &activator);
	void			(*pain)(edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage);
	void			(*die)(edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point);

	gtime_t			touch_debounce_time;		// are all these legit?  do we need more/less of them?
	gtime_t			pain_debounce_time;
	gtime_t			damage_debounce_time;
	gtime_t			fly_sound_debounce_time;	//move to clientinfo
	gtime_t			last_move_time;

	int32_t			health;
	int32_t			max_health;
	bool			deadflag;

	char			*map;			// target_changelevel

	vec_t			viewheight;		// height above origin where eyesight is determined
	bool			takedamage;
	int32_t			dmg;
	int32_t			radius_dmg;
	vec_t			dmg_radius;
	int32_t			sounds;			//make this a spawntemp var?
	int32_t			count;

	edict_ref		chain;
	edict_ref		enemy;
	edict_ref		oldenemy;
	edict_ref		activator;
	edict_ref		groundentity;
	int32_t			groundentity_linkcount;
	edict_ref		teamchain;
	edict_ref		teammaster;

	soundindex_t	noise_index;
	soundindex_t	noise_index2;
	vec_t			volume;
	vec_t			attenuation;

	// timing variables
	gtime_t			wait;
	gtime_t			delay;			// before firing targets
	gtime_t			random;

	gtime_t			teleport_time;

	brushcontents_t	watertype;
	waterlevel_t	waterlevel;

	vec3_t			move_origin;
	vec3_t			move_angles;

	// move this to clientinfo?
	int32_t			style;			// also used as areaportal number

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	edict_ref		control;
	vec_t			control_dist;
	pmove_state_t	pmove_state, old_pmove_state;

	constexpr bool operator==(const edict_t &other) const
	{
		return this == &other;
	}

	constexpr bool operator!=(const edict_t &other) const
	{
		return this != &other;
	}

	inline void Unicast(const bool &reliable)
	{
		gi.unicast(*this, reliable);
	}

	inline void Link()
	{
		gi.linkentity(*this);
	}

	inline void Unlink()
	{
		gi.unlinkentity(*this);
	}

	inline void PlaySound(const soundindex_t &sound, const soundchannel_t &channel = CHAN_AUTO, const soundattn_t &atten = ATTN_NORM, const vec_t &vol = 1.0f, const vec_t &soundofs = 0.0f)
	{
		gi.sound(*this, channel, sound, vol, atten, soundofs);
	}

	inline void PlayPositionedSound(const soundindex_t &sound, const vec3_t &pos, const soundchannel_t &channel = CHAN_AUTO, const soundattn_t &atten = ATTN_NORM, const vec_t &vol = 1.0f, const vec_t &soundofs = 0.0f)
	{
		gi.positioned_sound(pos, *this, channel, sound, vol, atten, soundofs);
	}
};

/*static*/ inline edict_t &gclient_t::ClientToEntity(gclient_t &client)
{
	return g_edicts[(&client - game.clients.data()) + 1];
}

inline void gclient_t::SendSound(const soundindex_t &sound, const vec_t volume, const soundchannel_t &channel, const soundattn_t &attn, const bool &reliable)
{
	const edict_t &ent = ClientToEntity(*this);
	const vec_t attenuation = min(static_cast<vec_t>(attn), 255.0f / 64);
	const int16_t sendchan = static_cast<int16_t>((ent.s.number << 3) | (static_cast<int16_t>(channel) & 7));

	soundflags_t flags = SND_ENT;
	
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		flags |= SND_VOLUME;
	if (attn != DEFAULT_SOUND_PACKET_ATTENUATION)
		flags |= SND_ATTENUATION;

	// prepare multicast message
	gi.WriteByte(SVC_SOUND);
	gi.WriteByte(flags); // flags
	gi.WriteByte(sound);
	if (flags & SND_VOLUME)
		gi.WriteByte(static_cast<uint8_t>(volume * 255));
	if (flags & SND_ATTENUATION)
		gi.WriteByte(static_cast<uint8_t>(attenuation * 64));
	gi.WriteShort(sendchan);
	gi.unicast(ent, reliable);
}