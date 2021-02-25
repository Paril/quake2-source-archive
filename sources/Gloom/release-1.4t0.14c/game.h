/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// game.h -- game dll information visible to server

#ifdef R1Q2_BUILD
#define GAME_API_VERSION        4
#else
#define GAME_API_VERSION        3
#endif

// define q2 imports and exports call type
// q_shared.h has a set too for pm.trace and pm.pointcontents
#ifdef WIN32
#define EXPORT __cdecl
#define IMPORT __cdecl
#else
#define IMPORT
#define EXPORT
#endif

//#define EXPORT _fastcall
//#define IMPORT _fastcall

// edict->svflags

#define SVF_NOCLIENT                    0x00000001      // don't send entity to clients, even if it has effects
#define SVF_DEADMONSTER                 0x00000002      // treat as CONTENTS_DEADMONSTER for collision
#define SVF_MONSTER                             0x00000004      // treat as CONTENTS_MONSTER for collision

//R1Q2 SPECIFC
#define SVF_NOPREDICTION                0x00000008
#define SVF_SEND_OWNER                  0x00000010      // send entity to owner (extended r1q2ded) /moo
#define SVF_SKIP_OWNER                  0x00000020      // skip entity from owner (extended r1q2ded) /moo
#define SVF_SEND_OBS                    0x00000040      // send entity to observers (extended r1q2ded) /moo

//R1Q2 SPECIFC

// edict->solid values

typedef enum
{
        SOLID_NOT,                      // no interaction with other objects
        SOLID_TRIGGER,          // only touch when inside, after moving
        SOLID_BBOX,                     // touch on edge
        SOLID_BSP                       // bsp clip, touch on edge
} solid_t;

// link_t is only used for entity area links now
typedef struct link_s
{
        struct link_s   *prev, *next;
} link_t;

#define MAX_ENT_CLUSTERS        16


typedef struct edict_s edict_t;
typedef struct gclient_s gclient_t;

//r1: this breaks VC++ autocomplete
//    so i commented it
/*#ifndef GAME_INCLUDE

struct gclient_s
{
        player_state_t  ps;             // communicated by server to clients
        int                             ping;
        // the game dll can add anything it wants after
        // this point in the structure
};


struct edict_s
{
        entity_state_t  s;
        struct gclient_s        *client;
        qboolean        inuse;
        int                     linkcount;

        // FIXME: move these fields to a server private sv_entity_t
        link_t          area;                           // linked to a division node or leaf
        
        int                     num_clusters;           // if -1, use headnode instead
        int                     clusternums[MAX_ENT_CLUSTERS];
        int                     headnode;                       // unused if num_clusters != -1
        int                     areanum, areanum2;

        int                     svflags;                        // SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
        vec3_t          mins, maxs;
        vec3_t          absmin, absmax, size;
        solid_t         solid;
        int                     clipmask;
        edict_t         *owner;

        // the game dll can add anything it wants after
        // this point in the structure
};

#endif          // GAME_INCLUDE*/

//
// functions provided by the main engine
//


//r1: hax
//haxored death to make VC++ recognise gi. for autocomplete.
#ifdef UNDEFINED_KEEP_IT_THIS_WAY
typedef struct
{
        void    *bprintf (int printlevel, const char *fmt, ...);
        void    *dprintf (const char *fmt, ...);
        void    *cprintf (edict_t /*@null@*/*ent, int printlevel, const char *fmt, ...);
        void    *centerprintf (edict_t *ent, char *fmt, ...);
        void    *sound (edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
        void    *positioned_sound (vec3_t origin, edict_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

        // config strings hold all the index strings, the lightstyles,
        // and misc data like the sky definition and cdtrack.
        // All of the current configstrings are sent to clients when
        // they connect, and changes are sent to all connected clients.
        void    *configstring (int num, char *string);

        void    *error (char *fmt, ...);

        // the *index functions create configstrings and some internal server state
        int             *modelindex (char *name);
        int             *soundindex (char *name);
        int             *imageindex (char *name);

        void    *setmodel (edict_t *ent, char *name);

        // collision detection
        trace_t *trace (vec3_t start, vec3_t /*@null@*/mins, vec3_t /*@null@*/maxs, vec3_t end, edict_t /*@null@*/*passent, int contentmask);
        int             *pointcontents (vec3_t point);
        qboolean        *inPVS (vec3_t p1, vec3_t p2);
        qboolean        *inPHS (vec3_t p1, vec3_t p2);
        void            *SetAreaPortalState (int portalnum, qboolean open);
        qboolean        *AreasConnected (int area1, int area2);

        // an entity will never be sent to a client or used for collision
        // if it is not passed to linkentity.  If the size, position, or
        // solidity changes, it must be relinked.
        void    *linkentity (edict_t *ent);
        void    *unlinkentity (edict_t *ent);           // call before removing an interactive edict
        int             *BoxEdicts (vec3_t mins, vec3_t maxs, edict_t /*@out@*/**list,  int maxcount, int areatype);
        void    *Pmove (pmove_t *pmove);                // player movement code common with client prediction

        // network messaging
        void    *multicast (vec3_t origin, multicast_t to);
        void    *unicast (edict_t *ent, qboolean reliable);
        void    *WriteChar (int c);
        void    *WriteByte (int c);
        void    *WriteShort (int c);
        void    *WriteLong (int c);
        void    *WriteFloat (float f);
        void    *WriteString (char *s);
        void    *WritePosition (vec3_t pos);    // some fractional bits
        void    *WriteDir (vec3_t pos);         // single byte encoded, very coarse
        void    *WriteAngle (float f);

        // managed memory allocation
        void    *TagMalloc (int size, int tag);
        void    *TagFree (void *block);
        void    *FreeTags (int tag);

        // console variable interaction
        cvar_t  *cvar (char *var_name, char *value, int flags);
        cvar_t  *cvar_set (char *var_name, char *value);
        cvar_t  *cvar_forceset (char *var_name, char *value);

        // ClientCommand and ServerCommand parameter access
        int             *argc (void);
        char    *argv (int n);
        char    *args (void);   // concatenation of all argv >= 1

        // add commands to the server console as if they were typed in
        // for map changing, etc
        void    *AddCommandString (char *text);

        void    *DebugGraph (float value, int color);
} game_import_t;
#endif


typedef struct game_import_s
{
        void    (IMPORT *bprintf) (int printlevel, const char *fmt, ...);
        void    (IMPORT *dprintf) (const char *fmt, ...);
        void    (IMPORT *cprintf) (edict_t /*@null@*/*ent, int printlevel, const char *fmt, ...);
        void    (IMPORT *centerprintf) (edict_t *ent, const char *fmt, ...);
        void    (IMPORT *sound) (edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
        void    (IMPORT *positioned_sound) (vec3_t origin, edict_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

        // config strings hold all the index strings, the lightstyles,
        // and misc data like the sky definition and cdtrack.
        // All of the current configstrings are sent to clients when
        // they connect, and changes are sent to all connected clients.
        void    (IMPORT *configstring) (int num, const char *string);

        void    (IMPORT *error) (const char *fmt, ...);

        // the *index functions create configstrings and some internal server state
        int             (IMPORT *modelindex) (const char *name);
        int             (IMPORT *soundindex) (const char *name);
        int             (IMPORT *imageindex) (const char *name);

        void    (IMPORT *setmodel) (edict_t *ent, const char *name);

        // collision detection
        trace_t (IMPORT *trace) (vec3_t start, /*@null@*/vec3_t mins, /*@null@*/vec3_t maxs, vec3_t end, edict_t /*@null@*/*passent, int contentmask);
        int             (IMPORT *pointcontents) (vec3_t point);
        qboolean        (IMPORT *inPVS) (vec3_t p1, vec3_t p2);
        qboolean        (IMPORT *inPHS) (vec3_t p1, vec3_t p2);
        void            (IMPORT *SetAreaPortalState) (int portalnum, qboolean open);
        qboolean        (IMPORT *AreasConnected) (int area1, int area2);

        // an entity will never be sent to a client or used for collision
        // if it is not passed to linkentity.  If the size, position, or
        // solidity changes, it must be relinked.
        void    (IMPORT *linkentity) (edict_t *ent);
        void    (IMPORT *unlinkentity) (edict_t *ent);          // call before removing an interactive edict
        int             (IMPORT *BoxEdicts) (vec3_t /*@null@*/mins, vec3_t /*@null@*/maxs, edict_t /*@out@*/**list,     int maxcount, int areatype);
        void    (IMPORT *Pmove) (pmove_t *pmove);               // player movement code common with client prediction

        // network messaging
        void    (IMPORT *multicast) (vec3_t origin, multicast_t to);
        void    (IMPORT *unicast) (edict_t *ent, qboolean reliable);
        void    (IMPORT *WriteChar) (int c);
        void    (IMPORT *WriteByte) (int c);
        void    (IMPORT *WriteShort) (int c);
        void    (IMPORT *WriteLong) (int c);
        void    (IMPORT *WriteFloat) (float f);
        void    (IMPORT *WriteString) (const char *s);
        void    (IMPORT *WritePosition) (vec3_t pos);   // some fractional bits
        void    (IMPORT *WriteDir) (vec3_t pos);                // single byte encoded, very coarse
        void    (IMPORT *WriteAngle) (float f);

        // managed memory allocation
        void    *(IMPORT *TagMalloc) (int size, int tag);
        void    (IMPORT *TagFree) (void *block);
        void    (IMPORT *FreeTags) (int tag);

        // console variable interaction
        cvar_t  *(IMPORT *cvar) (const char *var_name, const char *value, int flags);
        cvar_t  *(IMPORT *cvar_set) (const char *var_name, const char *value);
        cvar_t  *(IMPORT *cvar_forceset) (const char *var_name, const char *value);

        // ClientCommand and ServerCommand parameter access
        int             (IMPORT *argc) (void);
        char    *(IMPORT *argv) (int n);
        char    *(IMPORT *args) (void); // concatenation of all argv >= 1

        // add commands to the server console as if they were typed in
        // for map changing, etc
        void*    (IMPORT *AddCommandString) (const char *text);

        void    (IMPORT *DebugGraph) (float value, int color);
} game_import_t;

//
// functions exported by the game subsystem
//
typedef struct game_export_s
{
        int                     apiversion;

        // the init function will only be called when a game starts,
        // not each time a level is loaded.  Persistant data for clients
        // and the server can be allocated in init
        void            (EXPORT *Init) (void);
        void            (EXPORT *Shutdown) (void);

        // each new level entered will cause a call to SpawnEntities
        void            (EXPORT *SpawnEntities) (char *mapname, char *entstring, char *spawnpoint);

        // Read/Write Game is for storing persistant cross level information
        // about the world state and the clients.
        // WriteGame is called every time a level is exited.
        // ReadGame is called on a loadgame.
        void            (EXPORT *WriteGame) (char *filename, qboolean autosave);
        void            (EXPORT *ReadGame) (char *filename);

        // ReadLevel is called after the default map information has been
        // loaded with SpawnEntities
        void            (EXPORT *WriteLevel) (char *filename);
        void            (EXPORT *ReadLevel) (char *filename);

        qboolean        (EXPORT *ClientConnect) (edict_t *ent, char *userinfo);
        void            (EXPORT *ClientBegin) (edict_t *ent);
        void            (EXPORT *ClientUserinfoChanged) (edict_t *ent, char *userinfo);
        void            (EXPORT *ClientDisconnect) (edict_t *ent);
        void            (EXPORT *ClientCommand) (edict_t *ent);
        void            (EXPORT *ClientThink) (edict_t *ent, usercmd_t *cmd);

        void            (EXPORT *RunFrame) (void);

        // ServerCommand will be called when an "sv <command>" command is issued on the
        // server console.
        // The game can issue gi.argc() / gi.argv() commands to get the rest
        // of the parameters
        void            (EXPORT *ServerCommand) (void);

        //
        // global variables shared between game and server
        //

        // The edict array is allocated in the game dll so it
        // can vary in size from one game to another.
        // 
        // The size will be fixed when ge->Init() is called
        struct edict_s  *edicts;
        int                     edict_size;
        int                     num_edicts;             // current number, <= max_edicts
        int                     max_edicts;
} game_export_t;

game_export_t * EXPORT GetGameApi (game_import_t *import);
