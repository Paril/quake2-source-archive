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

#pragma once

// q_shared.h -- stuff that Quake client will need. Do not touch this stuff.

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cfloat>

#include "q_math.h"
#include "q_bitflags.h"
#include "q_random.h"

#include <string_view>
#include <string>
#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>

// type used for interop, but shouldn't be used by game. Use bool.
using qboolean = int32_t;

// these exist elsewhere
struct edict_t;

// a safe wrapper to edict_t* for interop and passing nullable entities around
class edict_ref
{
	edict_t *e;

public:
	edict_ref() :
		edict_ref(nullptr)
	{
	}

	edict_ref(const edict_t &p) :
		e(const_cast<edict_t *>(&p))
	{
	}

	edict_ref(nullptr_t) :
		e(nullptr)
	{
	}

	inline const edict_t *operator->() const { return &const_cast<edict_t &>(this->operator const edict_t &()); }
	inline edict_t *operator->() { return &this->operator edict_t &(); }
	inline bool operator==(const edict_ref &b) const { return e == b.e; }
	inline bool operator!=(const edict_ref &b) const { return e == b.e; }
	inline bool operator==(const edict_t &b) const { return e == &b; }
	inline bool operator!=(const edict_t &b) const { return e != &b; }
	inline bool operator==(const nullptr_t &) const { return e == nullptr; }
	inline bool operator!=(const nullptr_t &) const { return e != nullptr; }
	inline bool operator==(const edict_t *b) const { return e == b; }
	inline bool operator!=(const edict_t *b) const { return e != b; }
	inline const edict_t *ptr() const { return e; }
	inline edict_t *ptr() { return e; }

	operator bool() const;
	operator const edict_t &() const;
	operator edict_t &() { return const_cast<edict_t &>(this->operator const edict_t &()); }
};

template<>
struct std::hash<edict_ref>
{
	size_t operator()(const edict_ref& _Keyval) const
	{
		return std::hash<const edict_t*>()(_Keyval.ptr());
	}
};

constexpr size_t	MAX_QPATH			= 64;		// max length of a quake game pathname
constexpr size_t	MAX_OSPATH			= 128;		// max length of a filesystem pathname

//
// per-level limits
//
constexpr size_t	MAX_CLIENTS			= 256;		// absolute limit
constexpr size_t	MAX_EDICTS			= 1024;	    // must change protocol to increase more
constexpr size_t	MAX_LIGHTSTYLES		= 256;
constexpr size_t	MAX_MODELS			= 256;		// these are sent over the net as bytes
constexpr size_t	MAX_SOUNDS			= 256;		// so they cannot be blindly increased
constexpr size_t	MAX_IMAGES			= 256;
constexpr size_t	MAX_ITEMS			= 256;
constexpr size_t	MAX_GENERAL			= (MAX_CLIENTS * 2);	// general config strings

// game print flags
enum printflags_t
{
	PRINT_LOW,		// pickup messages
	PRINT_MEDIUM,	// death messages
	PRINT_HIGH,		// critical messages
	PRINT_CHAT		// chat messages
};

// destination class for gi.multicast()
enum multicast_t
{
	MULTICAST_ALL,
	MULTICAST_PHS,
	MULTICAST_PVS,
	MULTICAST_ALL_R,
	MULTICAST_PHS_R,
	MULTICAST_PVS_R
};

//=============================================

constexpr bool iequals(const std::string_view &a, const std::string_view &b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const char &ac, const char &bc) { return tolower(ac) == tolower(bc); });
}

char	*va(const char *format, ...);

struct com_parse_t
{
	const char			*start;
	std::string_view	token;
};

bool COM_Parse(com_parse_t &parse);

//=============================================

//
// key / value info strings
//
constexpr size_t	MAX_INFO_KEY		= 64;
constexpr size_t	MAX_INFO_VALUE		= 64;
constexpr size_t	MAX_INFO_STRING		= 512;

constexpr const char *DEFAULT_USERINFO = "\\name\\badinfo\\skin\\male/grunt";

struct userinfo_value_t
{
	std::string		string;
	vec_t			number;
};

class userinfo_t
{
	std::unordered_map<std::string, userinfo_value_t> _values;

public:
	bool Parse(const char *s);
	bool Encode(char *output, size_t output_size = MAX_INFO_STRING);

	inline bool Get(const char *key, const char *&string) const
	{
		if (!_values.contains(key))
			return false;

		string = _values.at(key).string.c_str();
		return true;
	}

	template<typename T, typename = typename std::enable_if<std::is_integral_v<T> || std::is_floating_point_v<T>>::type>
	inline bool Get(const char *key, T &number) const
	{
		if (!_values.contains(key))
			return false;

		number = _values.at(key).number;
		return true;
	}

	inline bool Has(const char *key) const
	{
		return _values.contains(key);
	}

	inline bool Set(const char *key, const char *value)
	{
		if (!value || !*value)
		{
			Remove(key);
			return true;
		}

		if (strstr(key, "\\") || strstr(value, "\\") ||
			strstr(key, ";") || strstr(key, "\"") || strstr(value, "\"") ||
			strlen(key) > MAX_INFO_KEY - 1 || strlen(value) > MAX_INFO_VALUE - 1)
			return false;

		_values[key] = { value, strtof(value, nullptr) };
		return true;
	}

	inline void Remove(const char *key)
	{
		_values.erase(key);
	}
};

/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define	CVAR

enum cvarflags_t
{
	CVAR_NONE		= 0,
	CVAR_ARCHIVE	= bit(0),	// set to cause it to be saved to vars.rc
	CVAR_USERINFO	= bit(1),	// added to userinfo  when changed
	CVAR_SERVERINFO	= bit(2),	// added to serverinfo when changed
	CVAR_NOSET		= bit(3),	// don't allow change from console at all,
								// but can be set from the command line
	CVAR_LATCH		= bit(4)	// save changes until server restart
};

MAKE_BITFLAGS(cvarflags_t);

// nothing outside the Cvar_*() functions should modify these fields!
struct cvar_t
{
	const char			*name;
	const char			*string;
	const char			*latched_string;	// for CVAR_LATCH vars
	const cvarflags_t	flags;
	qboolean			modified;	// set each time the cvar is changed
	const vec_t			value;
	const cvar_t		*next;
};
#endif		// CVAR

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// lower bits are stronger, and will eat weaker brushes completely
enum brushcontents_t
{
	CONTENTS_NONE			= 0,
	CONTENTS_SOLID			= bit(0),		// an eye is never valid in a solid
	CONTENTS_WINDOW			= bit(1),		// translucent, but not watery
	CONTENTS_AUX			= bit(2),
	CONTENTS_LAVA			= bit(3),
	CONTENTS_SLIME			= bit(4),
	CONTENTS_WATER			= bit(5),
	CONTENTS_MIST			= bit(6),
	LAST_VISIBLE_CONTENTS	= CONTENTS_MIST,

	// remaining contents are non-visible, and don't eat brushes
	CONTENTS_AREAPORTAL		= bit(15),

	CONTENTS_PLAYERCLIP		= bit(16),
	CONTENTS_MONSTERCLIP	= bit(17),

	// currents can be added to any other contents, and may be mixed
	CONTENTS_CURRENT_0		= bit(18),
	CONTENTS_CURRENT_90		= bit(19),
	CONTENTS_CURRENT_180	= bit(20),
	CONTENTS_CURRENT_270	= bit(21),
	CONTENTS_CURRENT_UP		= bit(22),
	CONTENTS_CURRENT_DOWN	= bit(23),

	CONTENTS_ORIGIN			= bit(24),	// removed before bsping an entity

	CONTENTS_MONSTER		= bit(25),	// should never be on a brush, only in game
	CONTENTS_DEADMONSTER	= bit(26),
	CONTENTS_DETAIL			= bit(27),	// brushes to be added after vis leafs
	CONTENTS_TRANSLUCENT	= bit(28),	// auto set if any surface has trans
	CONTENTS_LADDER			= bit(29),

	// content masks
	MASK_ALL				= (-1),
	MASK_SOLID				= (CONTENTS_SOLID|CONTENTS_WINDOW),
	MASK_DEADSOLID			= (MASK_SOLID|CONTENTS_PLAYERCLIP),
	MASK_PLAYERSOLID		= (MASK_DEADSOLID|CONTENTS_MONSTER),
	MASK_MONSTERSOLID		= (MASK_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_MONSTER),
	MASK_WATER				= (CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME),
	MASK_OPAQUE				= (CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA),
	MASK_SHOT				= (MASK_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER),
	MASK_CURRENT			= (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
};

MAKE_BITFLAGS(brushcontents_t);

enum surfflags_t
{
	SURF_LIGHT		= bit(0),	// value will hold the light strength
	
	SURF_SLICK		= bit(1),	// effects game physics
	
	SURF_SKY		= bit(2),	// don't draw, but add to skybox
	SURF_WARP		= bit(3),	// turbulent water warp
	SURF_TRANS33	= bit(4),
	SURF_TRANS66	= bit(5),
	SURF_FLOWING	= bit(6),	// scroll towards angle
	SURF_NODRAW		= bit(7)	// don't bother referencing the texture
};

MAKE_BITFLAGS(surfflags_t);

// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
enum areatype_t
{
	AREA_SOLID		= 1,
	AREA_TRIGGERS	= 2
};

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
struct cplane_t
{
	vec3_t	normal;
	vec_t	dist;
	uint8_t	type;			// for fast side tests
	uint8_t	signbits;		// signx + (signy<<1) + (signz<<1)
	uint8_t	pad[2];
};

struct csurface_t
{
	char		name[16];
	surfflags_t	flags;
	int32_t		value;
};

// a trace is returned when a box is swept through the world
struct trace_t
{
	qboolean		allsolid;	// if true, plane is not valid
	qboolean		startsolid;	// if true, the initial point was in a solid area
	vec_t			fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t			endpos;		// final position
	cplane_t		plane;		// surface normal at impact
	const csurface_t*surface;	// surface hit
	brushcontents_t	contents;	// contents on other side of surface hit
	edict_ref		ent;		// not set by CM_*() functions
};



// pmove_state_t is the information necessary for client side movement
// prediction
enum pmtype_t
{
	// can accelerate and turn
	PM_NORMAL,
	PM_SPECTATOR,
	// no acceleration or turning
	PM_DEAD,
	PM_GIB,		// different bounding box
	PM_FREEZE
};

// pmove->pm_flags
enum pmflags_t : uint8_t
{
	PMF_DUCKED			= bit(0),
	PMF_JUMP_HELD		= bit(1),
	PMF_ON_GROUND		= bit(2),
	PMF_TIME_WATERJUMP	= bit(3),	// pm_time is waterjump
	PMF_TIME_LAND		= bit(4),	// pm_time is time before rejump
	PMF_TIME_TELEPORT	= bit(5),	// pm_time is non-moving time
	PMF_NO_PREDICTION	= bit(6),	// temporarily disables prediction (used for grappling hook)
};

MAKE_BITFLAGS(pmflags_t);

// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
struct pmove_state_t
{
	pmtype_t	pm_type;

	int16_t		origin[3];		// 12.3
	int16_t		velocity[3];	// 12.3
	pmflags_t	pm_flags;		// ducked, jump_held, etc
	uint8_t		pm_time;		// each unit = 8 ms
	int16_t		gravity;
	int16_t		delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters
};

//
// button bits
//
enum button_t : uint8_t
{
	BUTTON_NONE		= 0,
	BUTTON_ATTACK	= bit(0),
	BUTTON_USE		= bit(1),
	BUTTON_ANY		= 0x80			// any key whatsoever
};

MAKE_BITFLAGS(button_t);

// usercmd_t is sent to the server each client frame
struct usercmd_t
{
	uint8_t		msec;
	button_t	buttons;
	int16_t		angles[3];
	int16_t		forwardmove, sidemove, upmove;
	uint8_t		impulse;		// remove?
	uint8_t		lightlevel;		// light level the player is standing on
};

enum waterlevel_t
{
	WATER_NONE,
	WATER_FEET,
	WATER_WAIST,
	WATER_UNDER
};

constexpr size_t	MAXTOUCH	= 32;

struct pmove_t
{
	// state (in / out)
	pmove_state_t	s;

	// command (in)
	usercmd_t		cmd;
	qboolean		snapinitial;	// if s has been changed outside pmove

	// results (out)
	int32_t			numtouch;
	edict_ref		touchents[MAXTOUCH];

	vec3_t			viewangles;			// clamped
	vec_t			viewheight;

	vec3_t			mins, maxs;			// bounding box size

	edict_ref		groundentity;
	brushcontents_t	watertype;
	waterlevel_t	waterlevel;

	// callbacks to test the world
	trace_t			(*trace) (const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end);
	brushcontents_t	(*pointcontents) (const vec3_t &point);
};

// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
enum entity_effects_t : uint32_t
{
	EF_NONE				= 0,
	EF_ROTATE			= bit(0),		// rotate (bonus items)
	EF_GIB				= bit(1),		// leave a trail
	EF_BLASTER			= bit(3),		// redlight + trail
	EF_ROCKET			= bit(4),		// redlight + trail
	EF_GRENADE			= bit(5),
	EF_HYPERBLASTER		= bit(6),
	EF_BFG				= bit(7),
	EF_COLOR_SHELL		= bit(8),
	EF_POWERSCREEN		= bit(9),
	EF_ANIM01			= bit(10),		// automatically cycle between frames 0 and 1 at 2 hz
	EF_ANIM23			= bit(11),		// automatically cycle between frames 2 and 3 at 2 hz
	EF_ANIM_ALL			= bit(12),		// automatically cycle through all frames at 2hz
	EF_ANIM_ALLFAST		= bit(13),		// automatically cycle through all frames at 10hz
	EF_FLIES			= bit(14),
	EF_QUAD				= bit(15),
	EF_PENT				= bit(16),
	EF_TELEPORTER		= bit(17),		// particle fountain
	EF_FLAG1			= bit(18),
	EF_FLAG2			= bit(19),
	// RAFAEL
	EF_IONRIPPER		= bit(20),
	EF_GREENGIB			= bit(21),
	EF_BLUEHYPERBLASTER = bit(22),
	EF_SPINNINGLIGHTS	= bit(23),
	EF_PLASMA			= bit(24),
	EF_TRAP				= bit(25),

	//ROGUE
	EF_TRACKER			= bit(26),
	EF_DOUBLE			= bit(27),
	EF_SPHERETRANS		= bit(28),
	EF_TAGTRAIL			= bit(29),
	EF_HALF_DAMAGE		= bit(30),
	EF_TRACKERTRAIL		= bit(31u)
	//ROGUE
};

MAKE_BITFLAGS(entity_effects_t);

// entity_state_t->renderfx flags
enum entity_renderfx_t : int32_t
{
	RF_NONE				= 0,
	RF_MINLIGHT			= bit(0),		// allways have some light (viewmodel)
	RF_VIEWERMODEL		= bit(1),		// don't draw through eyes, only mirrors
	RF_WEAPONMODEL		= bit(2),		// only draw through eyes
	RF_FULLBRIGHT		= bit(3),		// allways draw full intensity
	RF_DEPTHHACK		= bit(4),		// for view weapon Z crunching
	RF_TRANSLUCENT		= bit(5),
	RF_FRAMELERP		= bit(6),
	RF_BEAM				= bit(7),
	RF_CUSTOMSKIN		= bit(8),		// skin is an index in image_precache
	RF_GLOW				= bit(9),		// pulse lighting for bonus items
	RF_SHELL_RED		= bit(10),
	RF_SHELL_GREEN		= bit(11),
	RF_SHELL_BLUE		= bit(12),

	//ROGUE
	RF_IR_VISIBLE		= bit(15),		// 32768
	RF_SHELL_DOUBLE		= bit(16),		// 65536
	RF_SHELL_HALF_DAM	= bit(17),
	RF_USE_DISGUISE		= bit(18)
	//ROGUE
};

MAKE_BITFLAGS(entity_renderfx_t);

// player_state_t->refdef flags
enum refdef_flags_t
{
	RDF_NONE			= 0,
	RDF_UNDERWATER		= bit(0),		// warp the screen as apropriate
	RDF_NOWORLDMODEL	= bit(1),		// used for player configuration screen

	//ROGUE
	RDF_IRGOGGLES		= bit(2),
	RDF_UVGOGGLES		= bit(3)
	//ROGUE
};

MAKE_BITFLAGS(refdef_flags_t);

//
// muzzle flashes / player effects
//
enum muzzleflash_t : uint8_t
{
	MZ_BLASTER,
	MZ_MACHINEGUN,
	MZ_SHOTGUN,
	MZ_CHAINGUN1,
	MZ_CHAINGUN2,
	MZ_CHAINGUN3,
	MZ_RAILGUN,
	MZ_ROCKET,
	MZ_GRENADE,
	MZ_LOGIN,
	MZ_LOGOUT,
	MZ_RESPAWN,
	MZ_BFG,
	MZ_SSHOTGUN,
	MZ_HYPERBLASTER,
	MZ_ITEMRESPAWN,
	// RAFAEL
	MZ_IONRIPPER,
	MZ_BLUEHYPERBLASTER,
	MZ_PHALANX,

	//ROGUE
	MZ_ETF_RIFLE		= 30,
	MZ_SHOTGUN2			= 32,
	MZ_HEATBEAM,
	MZ_BLASTER2,
	MZ_TRACKER,
	MZ_NUKE1,
	MZ_NUKE2,
	MZ_NUKE4,
	MZ_NUKE8,
	//ROGUE

	MZ_SILENCED			= bit(7)		// bit flag ORed with one of the above numbers
};

MAKE_BITFLAGS(muzzleflash_t);

#include "m_flash.h"

// temp entity events
//
// Temp entity events are for things that happen
// at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed
// and broadcast.
enum temp_event_t : uint8_t
{
	TE_GUNSHOT,
	TE_BLOOD,
	TE_BLASTER,
	TE_RAILTRAIL,
	TE_SHOTGUN,
	TE_EXPLOSION1,
	TE_EXPLOSION2,
	TE_ROCKET_EXPLOSION,
	TE_GRENADE_EXPLOSION,
	TE_SPARKS,
	TE_SPLASH,
	TE_BUBBLETRAIL,
	TE_SCREEN_SPARKS,
	TE_SHIELD_SPARKS,
	TE_BULLET_SPARKS,
	TE_LASER_SPARKS,
	TE_PARASITE_ATTACK,
	TE_ROCKET_EXPLOSION_WATER,
	TE_GRENADE_EXPLOSION_WATER,
	TE_MEDIC_CABLE_ATTACK,
	TE_BFG_EXPLOSION,
	TE_BFG_BIGEXPLOSION,
	TE_BOSSTPORT,			// used as '22' in a map, so DON'T RENUMBER!!!
	TE_BFG_LASER,
	TE_GRAPPLE_CABLE,
	TE_WELDING_SPARKS,
	TE_GREENBLOOD,
	TE_BLUEHYPERBLASTER,
	TE_PLASMA_EXPLOSION,
	TE_TUNNEL_SPARKS,
//ROGUE
	TE_BLASTER2,
	TE_RAILTRAIL2,
	TE_FLAME,
	TE_LIGHTNING,
	TE_DEBUGTRAIL,
	TE_PLAIN_EXPLOSION,
	TE_FLASHLIGHT,
	TE_FORCEWALL,
	TE_HEATBEAM,
	TE_MONSTER_HEATBEAM,
	TE_STEAM,
	TE_BUBBLETRAIL2,
	TE_MOREBLOOD,
	TE_HEATBEAM_SPARKS,
	TE_HEATBEAM_STEAM,
	TE_CHAINFIST_SMOKE,
	TE_ELECTRIC_SPARKS,
	TE_TRACKER_EXPLOSION,
	TE_TELEPORT_EFFECT,
	TE_DBALL_GOAL,
	TE_WIDOWBEAMOUT,
	TE_NUKEBLAST,
	TE_WIDOWSPLASH,
	TE_EXPLOSION1_BIG,
	TE_EXPLOSION1_NP,
	TE_FLECHETTE
//ROGUE
};

enum splashtype_t : uint8_t
{
	SPLASH_UNKNOWN,
	SPLASH_SPARKS,
	SPLASH_BLUE_WATER,
	SPLASH_BROWN_WATER,
	SPLASH_SLIME,
	SPLASH_LAVA,
	SPLASH_BLOOD
};

// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
enum soundchannel_t
{
	CHAN_AUTO,
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	
	// modifier flags
	CHAN_NO_PHS_ADD			= bit(3),	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
	CHAN_RELIABLE			= bit(4)	// send by reliable message, not datagram
};

MAKE_BITFLAGS(soundchannel_t);

// sound attenuation values
enum soundattn_t
{
	ATTN_NONE,	// full volume the entire level
	ATTN_NORM,
	ATTN_IDLE,
	ATTN_STATIC	// diminish very rapidly with distance
};

// player_state->stats[] indexes
enum statindex_t : uint8_t
{
	// For compatibility sake, these 17 stat IDs should not be modified.
	STAT_HEALTH_ICON,
	STAT_HEALTH,
	STAT_AMMO_ICON,
	STAT_AMMO,
	STAT_ARMOR_ICON,
	STAT_ARMOR,
	STAT_SELECTED_ICON,
	STAT_PICKUP_ICON,
	STAT_PICKUP_STRING,
	STAT_TIMER_ICON,
	STAT_TIMER,
	STAT_HELPICON,
	STAT_SELECTED_ITEM,
	STAT_LAYOUTS,
	STAT_FRAGS,
	STAT_FLASHES,		// cleared each frame, 1 = health, 2 = armor
	STAT_CHASE,
	STAT_SPECTATOR,

	// 18-31 reserved for mods
	//STAT_BULLETS,
	STAT_SHELLS,
	STAT_GRENADES,

	STAT_HUNTER,
	STAT_HIDER,
	STAT_CONTROL,
	STAT_RADAR,
	STAT_ROUND_TIMER,

	MAX_STATS				= 32
};

// dmflags->value flags
enum dmflags_bits_t : uint32_t
{
	DF_NO_FALLING		= bit(3),	// 8
	DF_SAME_LEVEL		= bit(5),	// 32
	DF_NO_FRIENDLY_FIRE	= bit(8),	// 256
	DF_INFINITE_AMMO	= bit(13)	// 8192
};

#ifdef _WIN32
#pragma warning(disable : 4201)
#endif
union dmflags_t
{
	dmflags_bits_t	bits;
	struct {
		bool		_dummy0 : 1;
		bool		_dummy1 : 1;
		bool		_dummy2 : 1;
		bool		no_falling_damage : 1;
		bool		_dummy4 : 1;
		bool		same_level : 1;
		bool		_dummy6 : 1;
		bool		_dummy7 : 1;
		bool		no_friendly_fire : 1;
		bool		_dummy9 : 1;
		bool		_dummy10 : 1;
		bool		_dummy11 : 1;
		bool		_dummy12 : 1;
		bool		infinite_ammo : 1;
	};
};
#ifdef _WIN32
#pragma warning(default : 4201)
#endif

// ROGUE
/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

constexpr int16_t ANGLE2SHORT(const vec_t &x) { return (static_cast<int16_t>(x * 65536 / 360) & static_cast<int16_t>(65535)); }
constexpr vec_t SHORT2ANGLE(const int16_t &x) { return x * (360.0 / 65536); }

//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
enum
{
	CS_NAME,
	CS_CDTRACK,
	CS_SKY,
	CS_SKYAXIS,		// %f %f %f format
	CS_SKYROTATE,
	CS_STATUSBAR,	// display program string

	CS_AIRACCEL			= 29,		// air acceleration control
	CS_MAXCLIENTS,
	CS_MAPCHECKSUM,		// for catching cheater maps

	CS_MODELS,
	CS_SOUNDS			= (CS_MODELS + MAX_MODELS),
	CS_IMAGES			= (CS_SOUNDS + MAX_SOUNDS),
	CS_LIGHTS			= (CS_IMAGES + MAX_IMAGES),
	CS_ITEMS			= (CS_LIGHTS + MAX_LIGHTSTYLES),
	CS_PLAYERSKINS		= (CS_ITEMS + MAX_ITEMS),
	CS_GENERAL			= (CS_PLAYERSKINS + MAX_CLIENTS),
	MAX_CONFIGSTRINGS	= (CS_GENERAL + MAX_GENERAL),

	CS_GAME_STATUS		= (CS_GENERAL),
	CS_RADAR_STATUS		= (CS_GENERAL + 1),
	CS_ROUND_STATUS		= (CS_GENERAL + 2)
};

//==============================================


// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
enum entity_event_t
{
	EV_NONE,
	EV_ITEM_RESPAWN,
	EV_FOOTSTEP,
	EV_FALLSHORT,
	EV_FALL,
	EV_FALLFAR,
	EV_PLAYER_TELEPORT,
	EV_OTHER_TELEPORT
};

enum modelindex_t
{
	MODEL_NONE		= 0,
	MODEL_WORLD		= 1,
	MODEL_PLAYER	= 255
};

enum soundindex_t
{
	SOUND_NONE		= 0
};

enum imageindex_t
{
	IMAGE_NONE		= 0
};

// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
struct entity_state_t
{
	int32_t				number;				// edict index
	vec3_t				origin;
	vec3_t				angles;
	vec3_t				old_origin;			// for lerping
	modelindex_t		modelindex;
	modelindex_t		modelindex2,
						modelindex3,
						modelindex4;		// weapons, CTF flags, etc
	int32_t				frame;
	int32_t				skinnum;
	entity_effects_t	effects;		// PGM - we're filling it, so it needs to be unsigned
	entity_renderfx_t	renderfx;
	int32_t				solid;			// for client side prediction, 8*(bits 0-4) is x/y radius
										// 8*(bits 5-9) is z down distance, 8(bits10-15) is z up
										// gi.linkentity sets this properly
	soundindex_t		sound;			// for looping sounds, to guarantee shutoff
	entity_event_t		event;			// impulse events -- muzzle flashes, footsteps, etc
										// events only go out for a single frame, they
										// are automatically cleared each frame
};

//==============================================


// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pmove_state_t changes will be reletive to client
// frame rates
struct player_state_t
{
	pmove_state_t					pmove;				// for prediction

	vec3_t							viewangles;			// for fixed views
	vec3_t							viewoffset;			// add to pmovestate->origin
	vec3_t							kick_angles;		// add to view direction to get render angles
														// set by weapon kicks, pain effects, etc

	vec3_t							gunangles;
	vec3_t							gunoffset;
	modelindex_t					gunindex;
	int32_t							gunframe;

	vec4_t							blend;			// rgba full screen effect
	
	vec_t							fov;				// horizontal field of view

	refdef_flags_t					rdflags;			// refdef flags

	std::array<int16_t, MAX_STATS>	stats;	// fast status bar updates
};