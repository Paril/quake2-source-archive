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

// game.h -- game dll information visible to server
#pragma once

// edict->svflags

enum svflags_t
{
	SVF_NONE			= 0,
	SVF_NOCLIENT		= bit(0),		// don't send entity to clients, even if it has effects
	SVF_DEADMONSTER		= bit(1),	// treat as CONTENTS_DEADMONSTER for collision
	SVF_MONSTER			= bit(2)	// treat as CONTENTS_MONSTER for collision
};

MAKE_BITFLAGS(svflags_t);

// edict->solid values

enum solid_t
{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,		// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
};

//===============================================================

// link_t is only used for entity area links now
struct link_t
{
	link_t *prev, *next;
};

constexpr int32_t	MAX_ENT_CLUSTERS	= 16;

struct edict_t;
struct gclient_t;

struct gclient_server_t
{
	player_state_t	ps;		// communicated by server to clients
	int32_t			ping;
	// the game dll can add anything it wants after
	// this point in the structure
};


struct edict_server_t
{
	entity_state_t	s;
	gclient_t		*client;
	qboolean		inuse;
	int32_t			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t			area;				// linked to a division node or leaf
	
	int32_t			num_clusters;		// if -1, use headnode instead
	int32_t			clusternums[MAX_ENT_CLUSTERS];
	int32_t			headnode;			// unused if num_clusters != -1
	int32_t			areanum, areanum2;

	//================================

	svflags_t		svflags;			// SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
	vec3_t			mins, maxs;
	vec3_t			absmin, absmax, size;
	solid_t			solid;
	brushcontents_t	clipmask;
	edict_ref		owner;
};

//===============================================================

//
// functions provided by the main engine
//
struct game_import_t
{
	// special messages
	void	(*bprintf) (printflags_t printlevel, const char *fmt, ...);
	void	(*dprintf) (const char *fmt, ...);
	void	(*cprintf) (const edict_t &ent, printflags_t printlevel, const char *fmt, ...);
	void	(*centerprintf) (const edict_t &ent, const char *fmt, ...);
	void	(*sound) (const edict_t &ent, soundchannel_t channel, soundindex_t soundindex, vec_t volume, vec_t attenuation, vec_t timeofs);
	void	(*positioned_sound) (const vec3_t &origin, const edict_t &ent, soundchannel_t channel, soundindex_t soundindex, vec_t volume, vec_t attenuation, vec_t timeofs);

	// config strings hold all the index strings, the lightstyles,
	// and misc data like the sky definition and cdtrack.
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	void	(*configstring) (int32_t num, const char *string);

	void	(*error) (const char *fmt, ...);

	// the *index functions create configstrings and some internal server state
	modelindex_t (*modelindex) (const char *name);
	soundindex_t (*soundindex) (const char *name);
	imageindex_t (*imageindex) (const char *name);

	void	(*setmodel) (edict_t &ent, const char *name);

	// collision detection
private:
	trace_t	(*_trace) (const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, edict_ref passent, brushcontents_t contentmask);

public:
	trace_t trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const edict_ref &passent, const brushcontents_t &contentmask);

	inline trace_t trace(const vec3_t start, const vec3_t &mins, const vec3_t &maxs, const vec3_t end, const brushcontents_t &contentmask)
	{
		return trace(start, vec3_origin, vec3_origin, end, nullptr, contentmask);
	}

	inline trace_t trace(const vec3_t start, const vec3_t end, const edict_ref &passent, const brushcontents_t &contentmask)
	{
		return trace(start, vec3_origin, vec3_origin, end, passent, contentmask);
	}

	inline trace_t trace(const vec3_t start, const vec3_t end, const brushcontents_t &contentmask)
	{
		return trace(start, vec3_origin, vec3_origin, end, nullptr, contentmask);
	}

	brushcontents_t	(*pointcontents) (const vec3_t &point);
	qboolean	(*inPVS) (const vec3_t &p1, const vec3_t &p2);
	qboolean	(*inPHS) (const vec3_t &p1, const vec3_t &p2);
	void		(*SetAreaPortalState) (int32_t portalnum, qboolean open);
	qboolean	(*AreasConnected) (int32_t area1, int32_t area2);

	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.
	void	(*linkentity) (edict_t &ent);
	void	(*unlinkentity) (edict_t &ent);		// call before removing an interactive edict
	int32_t	(*BoxEdicts) (const vec3_t &mins, const vec3_t &maxs, edict_ref *list, int32_t maxcount, areatype_t areatype);
	void	(*Pmove) (pmove_t &pmove);		// player movement code common with client prediction

	// network messaging
	void	(*multicast) (const vec3_t &origin, multicast_t to);
	void	(*unicast) (const edict_t &ent, qboolean reliable);
	void	(*WriteChar) (int32_t c);
	void	(*WriteByte) (int32_t c);
	void	(*WriteShort) (int32_t c);
	void	(*WriteLong) (int32_t c);
	void	(*WriteFloat) (vec_t f);
	void	(*WriteString) (const char *s);
	void	(*WritePosition) (const vec3_t &pos);	// some fractional bits
	void	(*WriteDir) (const vec3_t &pos);		// single uint8_t encoded, very coarse
	void	(*WriteAngle) (vec_t f);

	void WriteEntity(const edict_t &ent);

	// managed memory allocation
private:
	void	*(*_TagMalloc) (int32_t size, int32_t tag);

public:
	template<typename T>
	inline T *TagMalloc(size_t count, int32_t tag)
	{
		return reinterpret_cast<T*>(_TagMalloc(static_cast<int32_t>(sizeof(T) * count), tag));
	}

	void	(*TagFree) (void *block);
	void	(*FreeTags) (int32_t tag);

	// console variable interaction
	cvar_t	*(*cvar) (const char *var_name, const char *value, cvarflags_t flags);
	cvar_t	*(*cvar_set) (const char *var_name, const char *value);
	cvar_t	*(*cvar_forceset) (const char *var_name, const char *value);

	// ClientCommand and ServerCommand parameter access
	int32_t		(*argc) ();
	const char	*(*argv) (int32_t n);
	const char	*(*args) ();	// concatenation of all argv >= 1

	// add commands to the server console as if they were typed in
	// for map changing, etc
	void	(*AddCommandString) (const char *text);

	void	(*DebugGraph) (vec_t value, int32_t color);
};

struct edict_pool_t
{
	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// The size will be fixed when ge->Init() is called
	edict_t		*pool;
	uint32_t	size;
	uint32_t	num;		// current number, <= max_edicts
	uint32_t	max = MAX_EDICTS;
};

//
// functions exported by the game subsystem
//
struct game_export_t
{
	const int32_t	apiversion = 3;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		(*Init) ();
	void		(*Shutdown) ();

	// each new level entered will cause a call to SpawnEntities
	void		(*SpawnEntities) (const char *mapname, const char *entstring, const char *spawnpoint);

	// Read/Write Game is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called on a loadgame.
	void		(*WriteGame) (const char *filename, qboolean autosave);
	void		(*ReadGame) (const char *filename);

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities
	void		(*WriteLevel) (const char *filename);
	void		(*ReadLevel) (const char *filename);

	qboolean	(*ClientConnect) (edict_t &ent, char *userinfo);
	void		(*ClientBegin) (edict_t &ent);
	void		(*ClientUserinfoChanged) (edict_t &ent, const char *userinfo);
	void		(*ClientDisconnect) (edict_t &ent);
	void		(*ClientCommand) (edict_t &ent);
	void		(*ClientThink) (edict_t &ent, const usercmd_t &cmd);

	void		(*RunFrame) ();

	// ServerCommand will be called when an "sv <command>" command is issued on the
	// server console.
	// The game can issue gi.argc() / gi.argv() commands to get the rest
	// of the parameters
	void		(*ServerCommand) ();

	//
	// global variables shared between game and server
	//
	edict_pool_t	entities;
};
