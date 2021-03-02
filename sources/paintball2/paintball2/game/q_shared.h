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
	
// q_shared.h -- included first by ALL program modules
#pragma once

#ifndef Q_SHARED_H
#define Q_SHARED_H

#ifdef _WIN32
// unknown pragmas are SUPPOSED to be ignored, but....
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)		// truncation from const double to float
#pragma warning(disable : 4996)		// I want to use POSIX, not C++, shut up, Microsoft.
#endif

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <xmmintrin.h> // jitsimd

#ifdef _WIN32
#else
#include <signal.h>

#undef assert
#define assert(expression)  \
  ((void) ((expression) ? 0 : raise(SIGTRAP)))
#endif

#if (defined _M_IX86 || defined __i386__) && !defined C_ONLY && !defined __sun__
#define id386	1
#else
#define id386	0
#endif

#if defined _M_ALPHA && !defined C_ONLY
#define idaxp	1
#else
#define idaxp	0
#endif

typedef unsigned char 		byte;
typedef enum {false, true}	qboolean;


#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef _DEBUG
#undef assert
#define assert(x) 0
#endif


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	512		// max tokens resulting from Cmd_TokenizeString - jit, increased from 80
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token - jit, increased from 128

#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			128		// max length of a filesystem pathname

//
// per-level limits
//
#define	MAX_CLIENTS			256		// absolute limit
#define	MAX_EDICTS			1024	// must change protocol to increase more
#define	MAX_LIGHTSTYLES		256
#define	MAX_MODELS			256		// these are sent over the net as bytes
#define	MAX_SOUNDS			256		// so they cannot be blindly increased
#define	MAX_IMAGES			256
#define	MAX_ITEMS			256
#define MAX_GENERAL			(MAX_CLIENTS*2)	// general config strings


// game print flags
#define	PRINT_LOW			0		// pickup messages
#define	PRINT_MEDIUM		1		// death messages
#define	PRINT_HIGH			2		// critical messages
#define	PRINT_CHAT			3		// chat messages

// paintball2 stuff:
#define PRINT_POPUP		11		// pops up a dailog on the client
#define PRINT_ITEM			12		// for item pickup notifications.
#define PRINT_EVENT			13		// for events (such as flag captures)
#define PRINT_SCOREDATA		14		// for server sending ping, kills, deaths, etc of a client.
#define PRINT_PINGDATA		15		// for client-side scoreboard
#define PRINT_MAPLISTDATA	16		// for client-side vote menu
#define PRINT_POPUP_NOCON	17		// same as PRINT_POPUP, but always uses a popup and never prints to the console.

#define PRINT_CHATN			20		// chat with name index encoded into first character(s)
#define PRINT_CHATN_TEAM	21
#define PRINT_CHATN_PRIVATE	22
#define	PRINT_CHATN_ACTION	23
#define PRINT_CHATN_RESERVED	24
#define PRINT_CHATN_RESERVED2	25


#define	ERR_FATAL			0		// exit the entire game with a popup window
#define	ERR_DROP			1		// print to console and disconnect from game
#define	ERR_DISCONNECT		2		// don't kill server
#define ERR_BENIGN			3		// print error to console and move on

#define	PRINT_ALL			0
#define PRINT_DEVELOPER		1		// only print when "developer 1"
#define PRINT_ALERT			2		


// destination class for gi.multicast()
typedef enum
{
	MULTICAST_ALL,
	MULTICAST_PHS,
	MULTICAST_PVS,
	MULTICAST_ALL_R,
	MULTICAST_PHS_R,
	MULTICAST_PVS_R
} multicast_t;


/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

typedef vec_t quat_t[4]; // jitskm
extern quat_t quat_identity; // jitskm

typedef vec3_t mat3_t[3];


typedef union {
	float			f;
	unsigned int	i;
} float_int_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#define DEG2RAD(a) (((float)a * (float)M_PI) / 180.0f)
#define RAD2DEG(a) ((a * 180.0f) / (float)M_PI)

struct cplane_s;

extern vec3_t vec3_origin;

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

// microsoft's fabs seems to be ungodly slow...
// jitopt -- it's faster with intrinsic function optimizations
//float Q_fabs (float f);
//#define	fabs(f) Q_fabs(f)
#if !defined C_ONLY && !defined __unix__ && !defined __sgi
extern long Q_ftol( float f );
#else
#define Q_ftol( f ) ( long ) (f)
#endif

//#define DotProduct(x,y)			(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
//#define VectorSubtract(a,b,c)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
//#define VectorAdd(a,b,c)		(c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2])
//#define VectorCopy(a,b)			(b[0]=a[0],b[1]=a[1],b[2]=a[2])
//#define VectorClear(a)			(a[0]=a[1]=a[2]=0)
//#define VectorNegate(a,b)		(b[0]=-a[0],b[1]=-a[1],b[2]=-a[2])
//#define VectorSet(v, x, y, z)	(v[0]=(x), v[1]=(y), v[2]=(z))
// Taken from Quake2 Evolved (qfusion?)
#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define DotProduct2(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1]) // jit
#define CrossProduct(x,y,o)		((o)[0]=(x)[1]*(y)[2]-(x)[2]*(y)[1],(o)[1]=(x)[2]*(y)[0]-(x)[0]*(y)[2],(o)[2]=(x)[0]*(y)[1]-(x)[1]*(y)[0])
#define Distance(x,y)			(sqrt((((x)[0]-(y)[0])*((x)[0]-(y)[0])+((x)[1]-(y)[1])*((x)[1]-(y)[1])+((x)[2]-(y)[2])*((x)[2]-(y)[2]))))
#define DistanceSquared(x,y)	(((x)[0]-(y)[0])*((x)[0]-(y)[0])+((x)[1]-(y)[1])*((x)[1]-(y)[1])+((x)[2]-(y)[2])*((x)[2]-(y)[2]))
#define	SnapVector(v)			((v)[0]=((int)((v)[0])),(v)[1]=((int)((v)[1])),(v)[2]=((int)((v)[2])))
#define VectorCopy(i,o)			((o)[0]=(i)[0],(o)[1]=(i)[1],(o)[2]=(i)[2])
#define VectorCopyAddZ(i,o,z)	((o)[0]=(i)[0],(o)[1]=(i)[1],(o)[2]=(i)[2] + z)
#define VectorSet(v,x,y,z)		((v)[0]=(x),(v)[1]=(y),(v)[2]=(z))
#define VectorClear(v)			((v)[0]=(v)[1]=(v)[2]=0.0f)
#define VectorCompare(a,b)		((a)[0]==(b)[0]&&(a)[1]==(b)[1]&&(a)[2]==(b)[2])
#define VectorAdd(a,b,o)		((o)[0]=(a)[0]+(b)[0],(o)[1]=(a)[1]+(b)[1],(o)[2]=(a)[2]+(b)[2])
#define VectorSubtract(a,b,o)	((o)[0]=(a)[0]-(b)[0],(o)[1]=(a)[1]-(b)[1],(o)[2]=(a)[2]-(b)[2])
#define	VectorScale(i,s,o)		((o)[0]=(i)[0]*(s),(o)[1]=(i)[1]*(s),(o)[2]=(i)[2]*(s))
#define VectorMultiply(a,b,o)	((o)[0]=(a)[0]*(b)[0],(o)[1]=(a)[1]*(b)[1],(o)[2]=(a)[2]*(b)[2])
#define	VectorMA(a,s,b,o)		((o)[0]=(a)[0]+(b)[0]*(s),(o)[1]=(a)[1]+(b)[1]*(s),(o)[2]=(a)[2]+(b)[2]*(s))
#define VectorAverage(a,b,o)	((o)[0]=((a)[0]+(b)[0])*0.5,(o)[1]=((a)[1]+(b)[1])*0.5,(o)[2]=((a)[2]+(b)[2])*0.5)
#define VectorNegate(i,o)		((o)[0]=-(i)[0],(o)[1]=-(i)[1],(o)[2]=-(i)[2])
#define VectorInverse(v)		((v)[0]=-(v)[0],(v)[1]=-(v)[1],(v)[2]=-(v)[2])
//#define VectorLength(v)			(sqrt((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2]))
#define VectorLengthSquared(v)	((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2])

vec_t VectorSquareDistance (const vec_t *a, const vec_t *b); // jitvec

void _VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

// just in case you don't want to use the macros
vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);

void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);
int _VectorCompare (vec3_t v1, vec3_t v2);
vec_t VectorLength (vec3_t v);
void _CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorNormalizeRetLen (vec3_t v); // returns vector length
void VectorNormalize (vec3_t v);
vec_t VectorNormalize2RetLen (vec3_t v, vec3_t out);
void _VectorInverse (vec3_t v);
void _VectorScale (vec3_t in, vec_t scale, vec3_t out);
float Q_rsqrt (float number); // jit - from qfusion
int Q_log2 (int val);
int Q_Round (float val); // jit

void Matrix3_Transpose (mat3_t in, mat3_t out); // jit - from qfusion

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

float DampIIR (float src, float dest, float factor, float dt);

void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);
float	anglemod(float a);
float LerpAngle (float a1, float a2, float frac);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void PerpendicularVector( vec3_t dst, const vec3_t src );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
qboolean	AxisCompare (const vec3_t axis1[3], const vec3_t axis2[3]); // jit - taken from Quake2Evolved

// === jitskm - skeletal model support from QFusion
void Matrix_Identity (vec3_t m[3]);
void Matrix_Copy (vec3_t m1[3], vec3_t m2[3]);
qboolean Matrix_Compare (vec3_t m1[3], vec3_t m2[3]);
void Matrix_Multiply (vec3_t m1[3], vec3_t m2[3], vec3_t out[3]);
void Matrix_TransformVector (vec3_t m[3], vec3_t v, vec3_t out);
void Matrix_Transpose (vec3_t in[3], vec3_t out[3]);
void Matrix_EulerAngles (vec3_t m[3], vec3_t angles);
void Matrix_EulerAngles2 (vec3_t m[3], vec3_t angles); // jitskm
void Matrix_Rotate (vec3_t m[3], vec_t angle, vec_t x, vec_t y, vec_t z);
void Matrix_FromPoints (const vec3_t v1, const vec3_t v2, const vec3_t v3, vec3_t m[3]);

void Quat_Identity (quat_t q);
void Quat_Copy (const quat_t q1, quat_t q2);
qboolean Quat_Compare (const quat_t q1, const quat_t q2);
void Quat_Conjugate (const quat_t q1, quat_t q2);
vec_t Quat_Normalize (quat_t q);
vec_t Quat_Inverse (const quat_t q1, quat_t q2);
void Quat_Multiply (const quat_t q1, const quat_t q2, quat_t out);
void Quat_Lerp (const quat_t q1, const quat_t q2, vec_t t, quat_t out);
void Quat_Vectors (const quat_t q, vec3_t f, vec3_t r, vec3_t u);
void Quat_Matrix (const quat_t q, vec3_t m[3]);
void Matrix_Quat (vec3_t m[3], quat_t q);
void Quat_TransformVector (const quat_t q, const vec3_t v, vec3_t out);
void Quat_ConcatTransforms (const quat_t q1, const vec3_t v1, const quat_t q2, const vec3_t v2, quat_t q, vec3_t v);
void Quat_FromEulerAngle (const vec3_t angle, quat_t q);
void Quat_ToEulerAngle (const quat_t q, vec3_t angle);
// jitskm ===
qboolean Sys_HaveSSE (void); // jitsimd

//=============================================

char *COM_SkipPath (char *pathname);
const char *COM_FileExtension (const char *in); // jit
void COM_StripExtension (const char *in, char *out, int out_size); // jitsecurity
void COM_FileBase (char *in, char *out);
void COM_FilePath (char *in, char *out);
void COM_DefaultExtension (char *path, char *extension);

char *COM_Parse (char **data_p);
char *COM_ParseExt (char **data_p, qboolean nl); // jitrscript - from qfusion
// data is an in/out parm, returns a parsed out token

// jitodo #define sprintf(a, b, c) BAD_PROGRAMMER_DO_NOT_USE(a, b, c)
void Com_sprintf (char *dest, int size, char *fmt, ...);

void Com_PageInMemory (byte *buffer, int size);

//=============================================

// portable case insensitive compare
//int Q_stricmp (const char *s1, const char *s2);
int Q_strcasecmp (const char *s1, const char *s2);
int Q_strncasecmp (const char *s1, const char *s2, int n);
int Q_streq (const char *s1, const char *s2); // jitopt -- faster than !strcmp
int Q_strcaseeq (const char *s1, const char *s2); // jitstricmp
//#ifdef WIN32 // jitstricmp -- don't use these, _stricmp is like 5x's slower!
//#define Q_strcasecmp(a,b) _stricmp(a,b)
//#define Q_stricmp(a,b) _stricmp(a,b)
//#else
//#define Q_strcasecmp(a,b) strcasecmp(a,b)
//#define Q_stricmp(a,b) strcasecmp(a,b)
//#define _stricmp(a,b) strcasecmp(a,b)
//#define stricmp(a,b) strcasecmp(a,b)
//#endif

//=============================================

short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);

void	Swap_Init (void);
char	*va(char *format, ...);

//=============================================

//
// key / value info strings
//
#define	MAX_INFO_KEY		64
#define	MAX_INFO_VALUE		64
#define	MAX_INFO_STRING		1024 // jit - allow for more server info (this must be the same in the game dll!)

char *Info_ValueForKey (char *s, char *key);
void Info_RemoveKey (char *s, const char *key);
void Info_SetValueForKey (char *s, const char *key, const char *value);
qboolean Info_Validate (char *s);

/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/

extern	int	curtime;		// time returned by last Sys_Milliseconds

int		Sys_Milliseconds (void);
void	Sys_Mkdir (char *path);

// large block stack allocation routines
void	*Hunk_Begin (int maxsize);
void	*Hunk_Alloc (int size);
void	Hunk_Free (void *buf);
int		Hunk_End (void);

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

/*
** pass in an attribute mask of things you wish to REJECT
*/
char	*Sys_FindFirst (const char *path, unsigned musthave, unsigned canthave);
char	*Sys_FindNext (unsigned musthave, unsigned canthave);
void	Sys_FindClose (void);


// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...);
void Com_Printf (char *msg, ...);


/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define	CVAR

#define	CVAR_ARCHIVE	1	// set to cause it to be saved to vars.rc
#define	CVAR_USERINFO	2	// added to userinfo  when changed
#define	CVAR_SERVERINFO	4	// added to serverinfo when changed
#define	CVAR_NOSET		8	// don't allow change from console at all,
							// but can be set from the command line
#define	CVAR_LATCH		16	// save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
	char		*name;
	char		*string;
	char		*latched_string;	// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;	// set each time the cvar is changed
	float		value;
	struct cvar_s *next;
} cvar_t;

#endif		// CVAR

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			2		// translucent, but not watery
#define	CONTENTS_AUX			4
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_MIST			64
#define	LAST_VISIBLE_CONTENTS	64

// remaining contents are non-visible, and don't eat brushes

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000



#define	SURF_LIGHT		0x1		// value will hold the light strength

#define	SURF_SLICK		0x2		// effects game physics

#define	SURF_SKY		0x4		// don't draw, but add to skybox
#define	SURF_WARP		0x8		// turbulent water warp
#define	SURF_TRANS33	0x10
#define	SURF_TRANS66	0x20
#define	SURF_FLOWING	0x40	// scroll towards angle
#define	SURF_NODRAW		0x80	// don't bother referencing the texture



// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_WINDOW)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW)
#define	MASK_MONSTERSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER)
#define MASK_CURRENT			(CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)


// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2


// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
} cplane_t;

// structure offset for asm code
#define CPLANE_NORMAL_X			0
#define CPLANE_NORMAL_Y			4
#define CPLANE_NORMAL_Z			8
#define CPLANE_DIST				12
#define CPLANE_TYPE				16
#define CPLANE_SIGNBITS			17
#define CPLANE_PAD0				18
#define CPLANE_PAD1				19

typedef struct cmodel_s
{
	vec3_t		mins, maxs;
	vec3_t		origin;		// for sounds or lights
	int			headnode;
} cmodel_t;

// === jitsound - enum for different surface sound types
typedef enum surface_sound_type_s
{
	SURFACE_SOUND_UNKNOWN = 0,
	SURFACE_SOUND_DEFAULT,
	SURFACE_SOUND_WOOD_SOLID,
	SURFACE_SOUND_WOOD_PLANK,
	SURFACE_SOUND_WOOD_OTHER, // can be filled in with something later
	SURFACE_SOUND_METAL_BARREL,
	SURFACE_SOUND_METAL_THIN,
	SURFACE_SOUND_METAL_THICK,
	SURFACE_SOUND_METAL_OTHER, // can be filled in with something later
	SURFACE_SOUND_GLASS,
	SURFACE_SOUND_SNOW,
	SURFACE_SOUND_CEMENT,
	SURFACE_SOUND_BRICK,
	SURFACE_SOUND_DIRT,
	SURFACE_SOUND_GRASS,
	SURFACE_SOUND_WATER,
	SURFACE_SOUND_TREE,
	SURFACE_SOUND_SAND,
	SURFACE_SOUND_ROCK
} surface_sound_type_t;
// jitsound ===

typedef struct csurface_s
{
	char		name[16];
	int			flags;
	int			value;
	surface_sound_type_t surface_sound; // jitsound
} csurface_t;

typedef struct mapsurface_s  // used internally due to name len probs //ZOID
{
	csurface_t	c;
	char		rname[32];
} mapsurface_t;

// a trace is returned when a box is swept through the world
typedef struct
{
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact
	csurface_t	*surface;	// surface hit
	int			contents;	// contents on other side of surface hit
	struct edict_s	*ent;		// not set by CM_*() functions
} trace_t;



// pmove_state_t is the information necessary for client side movement
// prediction
typedef enum 
{
	// can accelerate and turn
	PM_NORMAL,
	PM_SPECTATOR,
	// no acceleration or turning
	PM_DEAD,
	PM_GIB,		// different bounding box
	PM_FREEZE
} pmtype_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_ON_GROUND		4
#define	PMF_TIME_WATERJUMP	8	// pm_time is waterjump
//jitmove #define	PMF_TIME_LAND		16	// pm_time is time before rejump
#define	PMF_TIME_TELEPORT	32	// pm_time is non-moving time
#define PMF_NO_PREDICTION	64	// temporarily disables prediction (used for grappling hook)

// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
typedef struct
{
	pmtype_t	pm_type;

	short		origin[3];		// 12.3
	short		velocity[3];	// 12.3
	byte		pm_flags;		// ducked, jump_held, etc
	byte		pm_time;		// each unit = 8 ms
	short		gravity;
	short		delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters
#ifdef USE_PMOVE_FLOAT // can't have this here, as it screws up offsets between game and engine...
	// jitmove - floating point versions, for more accuracy
	vec3_t		forigin;	
	vec3_t		fvelocity;
#endif
} pmove_state_t;


// button bits
#define	BUTTON_ATTACK		1
#define	BUTTON_USE			2
#define	BUTTON_TOSSGREN		4	// === jitbutton
#define	BUTTON_4			8
#define	BUTTON_5			16
#define	BUTTON_6			32
#define	BUTTON_CHAT			64	// jitbutton ===
#define	BUTTON_ANY			128	// any key whatsoever


// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
	byte	msec;
	byte	buttons;
	short	angles[3];
	short	forwardmove, sidemove, upmove;
	byte	impulse;
	byte	lightlevel;		// light level the player is standing on (pointless, but needed for compatibility - might reuse this byte)
} usercmd_t;


#define	MAXTOUCH	32

// WARNING! THIS STRUCTURE MUST REMAIN THE SAME TO BE COMPATIBLE WITH THE GAME DLL!
typedef struct
{
	// state (in / out)
	pmove_state_t	s;

	// command (in)
	usercmd_t		cmd;
	qboolean		snapinitial;	// if s has been changed outside pmove

	// results (out)
	int			numtouch;
	struct edict_s	*touchents[MAXTOUCH];

	vec3_t		viewangles;			// clamped
	float		viewheight;

	vec3_t		mins, maxs;			// bounding box size

	struct edict_s	*groundentity;
	int			watertype;
	int			waterlevel;

	// callbacks to test the world
	trace_t		(*trace) (const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end);
	int			(*pointcontents) (vec3_t point);
} pmove_t;


// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
#define	EF_ROTATE			0x00000001		// rotate (bonus items)
#define	EF_GIB				0x00000002		// leave a trail
#define	EF_BLASTER			0x00000008		// redlight + trail
#define	EF_ROCKET			0x00000010		// redlight + trail
#define	EF_GRENADE			0x00000020
#define	EF_HYPERBLASTER		0x00000040
#define	EF_BFG				0x00000080
#define EF_COLOR_SHELL		0x00000100
#define EF_POWERSCREEN		0x00000200
#define	EF_ANIM01			0x00000400		// automatically cycle between frames 0 and 1 at 2 hz
#define	EF_ANIM23			0x00000800		// automatically cycle between frames 2 and 3 at 2 hz
#define EF_ANIM_ALL			0x00001000		// automatically cycle through all frames at 2hz
#define EF_ANIM_ALLFAST		0x00002000		// automatically cycle through all frames at 10hz
#define	EF_FLIES			0x00004000
#define	EF_QUAD				0x00008000
#define	EF_PENT				0x00010000
#define	EF_TELEPORTER		0x00020000		// particle fountain
#define EF_FLAG1			0x00040000
#define EF_FLAG2			0x00080000
// RAFAEL
#define EF_IONRIPPER		0x00100000
#define EF_GREENGIB			0x00200000
#define	EF_BLUEHYPERBLASTER 0x00400000
#define EF_SPINNINGLIGHTS	0x00800000
#define EF_PLASMA			0x01000000
#define EF_TRAP				0x02000000

//ROGUE
#define EF_TRACKER			0x04000000
#define	EF_DOUBLE			0x08000000
#define	EF_SPHERETRANS		0x10000000
#define EF_TAGTRAIL			0x20000000
#define EF_HALF_DAMAGE		0x40000000
#define EF_TRACKERTRAIL		0x80000000
//ROGUE

// entity_state_t->renderfx flags
#define	RF_MINLIGHT			1		// allways have some light (viewmodel)
#define	RF_VIEWERMODEL		2		// don't draw through eyes, only mirrors
#define	RF_WEAPONMODEL		4		// only draw through eyes
#define	RF_FULLBRIGHT		8		// allways draw full intensity
#define	RF_DEPTHHACK		16		// for view weapon Z crunching
#define	RF_TRANSLUCENT		32
#define	RF_FRAMELERP		64
#define RF_BEAM				128
#define	RF_CUSTOMSKIN		256		// skin is an index in image_precache
#define	RF_GLOW				512		// pulse lighting for bonus items
#define RF_SHELL_RED		1024
#define	RF_SHELL_GREEN		2048
#define RF_SHELL_BLUE		4096

//ROGUE
#define RF_IR_VISIBLE		0x00008000		// 32768
#define	RF_SHELL_DOUBLE		0x00010000		// 65536
#define	RF_SHELL_HALF_DAM	0x00020000
#define RF_USE_DISGUISE		0x00040000
//ROGUE

// player_state_t->refdef flags
#define	RDF_UNDERWATER		1		// warp the screen as apropriate
#define RDF_NOWORLDMODEL	2		// used for player configuration screen

//ROGUE
#define	RDF_IRGOGGLES		4
#define RDF_UVGOGGLES		8
//ROGUE

//
// muzzle flashes / player effects
//
#define	MZ_BLASTER			0
#define MZ_MACHINEGUN		1
#define	MZ_SHOTGUN			2
#define	MZ_CHAINGUN1		3
#define	MZ_CHAINGUN2		4
#define	MZ_CHAINGUN3		5
#define	MZ_RAILGUN			6
#define	MZ_ROCKET			7
#define	MZ_GRENADE			8
#define	MZ_LOGIN			9
#define	MZ_LOGOUT			10
#define	MZ_RESPAWN			11
#define	MZ_BFG				12
#define	MZ_SSHOTGUN			13
#define	MZ_HYPERBLASTER		14
#define	MZ_ITEMRESPAWN		15
// RAFAEL
#define MZ_IONRIPPER		16
#define MZ_BLUEHYPERBLASTER 17
#define MZ_PHALANX			18
#define MZ_SILENCED			128		// bit flag ORed with one of the above numbers

//ROGUE
#define MZ_ETF_RIFLE		30
#define MZ_UNUSED			31
#define MZ_SHOTGUN2			32
#define MZ_HEATBEAM			33
#define MZ_BLASTER2			34
#define	MZ_TRACKER			35
#define	MZ_NUKE1			36
#define	MZ_NUKE2			37
#define	MZ_NUKE4			38
#define	MZ_NUKE8			39
//ROGUE

//
// monster muzzle flashes
//
#define MZ2_TANK_BLASTER_1				1
#define MZ2_TANK_BLASTER_2				2
#define MZ2_TANK_BLASTER_3				3
#define MZ2_TANK_MACHINEGUN_1			4
#define MZ2_TANK_MACHINEGUN_2			5
#define MZ2_TANK_MACHINEGUN_3			6
#define MZ2_TANK_MACHINEGUN_4			7
#define MZ2_TANK_MACHINEGUN_5			8
#define MZ2_TANK_MACHINEGUN_6			9
#define MZ2_TANK_MACHINEGUN_7			10
#define MZ2_TANK_MACHINEGUN_8			11
#define MZ2_TANK_MACHINEGUN_9			12
#define MZ2_TANK_MACHINEGUN_10			13
#define MZ2_TANK_MACHINEGUN_11			14
#define MZ2_TANK_MACHINEGUN_12			15
#define MZ2_TANK_MACHINEGUN_13			16
#define MZ2_TANK_MACHINEGUN_14			17
#define MZ2_TANK_MACHINEGUN_15			18
#define MZ2_TANK_MACHINEGUN_16			19
#define MZ2_TANK_MACHINEGUN_17			20
#define MZ2_TANK_MACHINEGUN_18			21
#define MZ2_TANK_MACHINEGUN_19			22
#define MZ2_TANK_ROCKET_1				23
#define MZ2_TANK_ROCKET_2				24
#define MZ2_TANK_ROCKET_3				25

#define MZ2_INFANTRY_MACHINEGUN_1		26
#define MZ2_INFANTRY_MACHINEGUN_2		27
#define MZ2_INFANTRY_MACHINEGUN_3		28
#define MZ2_INFANTRY_MACHINEGUN_4		29
#define MZ2_INFANTRY_MACHINEGUN_5		30
#define MZ2_INFANTRY_MACHINEGUN_6		31
#define MZ2_INFANTRY_MACHINEGUN_7		32
#define MZ2_INFANTRY_MACHINEGUN_8		33
#define MZ2_INFANTRY_MACHINEGUN_9		34
#define MZ2_INFANTRY_MACHINEGUN_10		35
#define MZ2_INFANTRY_MACHINEGUN_11		36
#define MZ2_INFANTRY_MACHINEGUN_12		37
#define MZ2_INFANTRY_MACHINEGUN_13		38

#define MZ2_SOLDIER_BLASTER_1			39
#define MZ2_SOLDIER_BLASTER_2			40
#define MZ2_SOLDIER_SHOTGUN_1			41
#define MZ2_SOLDIER_SHOTGUN_2			42
#define MZ2_SOLDIER_MACHINEGUN_1		43
#define MZ2_SOLDIER_MACHINEGUN_2		44

#define MZ2_GUNNER_MACHINEGUN_1			45
#define MZ2_GUNNER_MACHINEGUN_2			46
#define MZ2_GUNNER_MACHINEGUN_3			47
#define MZ2_GUNNER_MACHINEGUN_4			48
#define MZ2_GUNNER_MACHINEGUN_5			49
#define MZ2_GUNNER_MACHINEGUN_6			50
#define MZ2_GUNNER_MACHINEGUN_7			51
#define MZ2_GUNNER_MACHINEGUN_8			52
#define MZ2_GUNNER_GRENADE_1			53
#define MZ2_GUNNER_GRENADE_2			54
#define MZ2_GUNNER_GRENADE_3			55
#define MZ2_GUNNER_GRENADE_4			56

#define MZ2_CHICK_ROCKET_1				57

#define MZ2_FLYER_BLASTER_1				58
#define MZ2_FLYER_BLASTER_2				59

#define MZ2_MEDIC_BLASTER_1				60

#define MZ2_GLADIATOR_RAILGUN_1			61

#define MZ2_HOVER_BLASTER_1				62

#define MZ2_ACTOR_MACHINEGUN_1			63

#define MZ2_SUPERTANK_MACHINEGUN_1		64
#define MZ2_SUPERTANK_MACHINEGUN_2		65
#define MZ2_SUPERTANK_MACHINEGUN_3		66
#define MZ2_SUPERTANK_MACHINEGUN_4		67
#define MZ2_SUPERTANK_MACHINEGUN_5		68
#define MZ2_SUPERTANK_MACHINEGUN_6		69
#define MZ2_SUPERTANK_ROCKET_1			70
#define MZ2_SUPERTANK_ROCKET_2			71
#define MZ2_SUPERTANK_ROCKET_3			72

#define MZ2_BOSS2_MACHINEGUN_L1			73
#define MZ2_BOSS2_MACHINEGUN_L2			74
#define MZ2_BOSS2_MACHINEGUN_L3			75
#define MZ2_BOSS2_MACHINEGUN_L4			76
#define MZ2_BOSS2_MACHINEGUN_L5			77
#define MZ2_BOSS2_ROCKET_1				78
#define MZ2_BOSS2_ROCKET_2				79
#define MZ2_BOSS2_ROCKET_3				80
#define MZ2_BOSS2_ROCKET_4				81

#define MZ2_FLOAT_BLASTER_1				82

#define MZ2_SOLDIER_BLASTER_3			83
#define MZ2_SOLDIER_SHOTGUN_3			84
#define MZ2_SOLDIER_MACHINEGUN_3		85
#define MZ2_SOLDIER_BLASTER_4			86
#define MZ2_SOLDIER_SHOTGUN_4			87
#define MZ2_SOLDIER_MACHINEGUN_4		88
#define MZ2_SOLDIER_BLASTER_5			89
#define MZ2_SOLDIER_SHOTGUN_5			90
#define MZ2_SOLDIER_MACHINEGUN_5		91
#define MZ2_SOLDIER_BLASTER_6			92
#define MZ2_SOLDIER_SHOTGUN_6			93
#define MZ2_SOLDIER_MACHINEGUN_6		94
#define MZ2_SOLDIER_BLASTER_7			95
#define MZ2_SOLDIER_SHOTGUN_7			96
#define MZ2_SOLDIER_MACHINEGUN_7		97
#define MZ2_SOLDIER_BLASTER_8			98
#define MZ2_SOLDIER_SHOTGUN_8			99
#define MZ2_SOLDIER_MACHINEGUN_8		100

// --- Xian shit below ---
#define	MZ2_MAKRON_BFG					101
#define MZ2_MAKRON_BLASTER_1			102
#define MZ2_MAKRON_BLASTER_2			103
#define MZ2_MAKRON_BLASTER_3			104
#define MZ2_MAKRON_BLASTER_4			105
#define MZ2_MAKRON_BLASTER_5			106
#define MZ2_MAKRON_BLASTER_6			107
#define MZ2_MAKRON_BLASTER_7			108
#define MZ2_MAKRON_BLASTER_8			109
#define MZ2_MAKRON_BLASTER_9			110
#define MZ2_MAKRON_BLASTER_10			111
#define MZ2_MAKRON_BLASTER_11			112
#define MZ2_MAKRON_BLASTER_12			113
#define MZ2_MAKRON_BLASTER_13			114
#define MZ2_MAKRON_BLASTER_14			115
#define MZ2_MAKRON_BLASTER_15			116
#define MZ2_MAKRON_BLASTER_16			117
#define MZ2_MAKRON_BLASTER_17			118
#define MZ2_MAKRON_RAILGUN_1			119
#define	MZ2_JORG_MACHINEGUN_L1			120
#define	MZ2_JORG_MACHINEGUN_L2			121
#define	MZ2_JORG_MACHINEGUN_L3			122
#define	MZ2_JORG_MACHINEGUN_L4			123
#define	MZ2_JORG_MACHINEGUN_L5			124
#define	MZ2_JORG_MACHINEGUN_L6			125
#define	MZ2_JORG_MACHINEGUN_R1			126
#define	MZ2_JORG_MACHINEGUN_R2			127
#define	MZ2_JORG_MACHINEGUN_R3			128
#define	MZ2_JORG_MACHINEGUN_R4			129
#define MZ2_JORG_MACHINEGUN_R5			130
#define	MZ2_JORG_MACHINEGUN_R6			131
#define MZ2_JORG_BFG_1					132
#define MZ2_BOSS2_MACHINEGUN_R1			133
#define MZ2_BOSS2_MACHINEGUN_R2			134
#define MZ2_BOSS2_MACHINEGUN_R3			135
#define MZ2_BOSS2_MACHINEGUN_R4			136
#define MZ2_BOSS2_MACHINEGUN_R5			137

//ROGUE
#define	MZ2_CARRIER_MACHINEGUN_L1		138
#define	MZ2_CARRIER_MACHINEGUN_R1		139
#define	MZ2_CARRIER_GRENADE				140
#define MZ2_TURRET_MACHINEGUN			141
#define MZ2_TURRET_ROCKET				142
#define MZ2_TURRET_BLASTER				143
#define MZ2_STALKER_BLASTER				144
#define MZ2_DAEDALUS_BLASTER			145
#define MZ2_MEDIC_BLASTER_2				146
#define	MZ2_CARRIER_RAILGUN				147
#define	MZ2_WIDOW_DISRUPTOR				148
#define	MZ2_WIDOW_BLASTER				149
#define	MZ2_WIDOW_RAIL					150
#define	MZ2_WIDOW_PLASMABEAM			151		// PMM - not used
#define	MZ2_CARRIER_MACHINEGUN_L2		152
#define	MZ2_CARRIER_MACHINEGUN_R2		153
#define	MZ2_WIDOW_RAIL_LEFT				154
#define	MZ2_WIDOW_RAIL_RIGHT			155
#define	MZ2_WIDOW_BLASTER_SWEEP1		156
#define	MZ2_WIDOW_BLASTER_SWEEP2		157
#define	MZ2_WIDOW_BLASTER_SWEEP3		158
#define	MZ2_WIDOW_BLASTER_SWEEP4		159
#define	MZ2_WIDOW_BLASTER_SWEEP5		160
#define	MZ2_WIDOW_BLASTER_SWEEP6		161
#define	MZ2_WIDOW_BLASTER_SWEEP7		162
#define	MZ2_WIDOW_BLASTER_SWEEP8		163
#define	MZ2_WIDOW_BLASTER_SWEEP9		164
#define	MZ2_WIDOW_BLASTER_100			165
#define	MZ2_WIDOW_BLASTER_90			166
#define	MZ2_WIDOW_BLASTER_80			167
#define	MZ2_WIDOW_BLASTER_70			168
#define	MZ2_WIDOW_BLASTER_60			169
#define	MZ2_WIDOW_BLASTER_50			170
#define	MZ2_WIDOW_BLASTER_40			171
#define	MZ2_WIDOW_BLASTER_30			172
#define	MZ2_WIDOW_BLASTER_20			173
#define	MZ2_WIDOW_BLASTER_10			174
#define	MZ2_WIDOW_BLASTER_0				175
#define	MZ2_WIDOW_BLASTER_10L			176
#define	MZ2_WIDOW_BLASTER_20L			177
#define	MZ2_WIDOW_BLASTER_30L			178
#define	MZ2_WIDOW_BLASTER_40L			179
#define	MZ2_WIDOW_BLASTER_50L			180
#define	MZ2_WIDOW_BLASTER_60L			181
#define	MZ2_WIDOW_BLASTER_70L			182
#define	MZ2_WIDOW_RUN_1					183
#define	MZ2_WIDOW_RUN_2					184
#define	MZ2_WIDOW_RUN_3					185
#define	MZ2_WIDOW_RUN_4					186
#define	MZ2_WIDOW_RUN_5					187
#define	MZ2_WIDOW_RUN_6					188
#define	MZ2_WIDOW_RUN_7					189
#define	MZ2_WIDOW_RUN_8					190
#define	MZ2_CARRIER_ROCKET_1			191
#define	MZ2_CARRIER_ROCKET_2			192
#define	MZ2_CARRIER_ROCKET_3			193
#define	MZ2_CARRIER_ROCKET_4			194
#define	MZ2_WIDOW2_BEAMER_1				195
#define	MZ2_WIDOW2_BEAMER_2				196
#define	MZ2_WIDOW2_BEAMER_3				197
#define	MZ2_WIDOW2_BEAMER_4				198
#define	MZ2_WIDOW2_BEAMER_5				199
#define	MZ2_WIDOW2_BEAM_SWEEP_1			200
#define	MZ2_WIDOW2_BEAM_SWEEP_2			201
#define	MZ2_WIDOW2_BEAM_SWEEP_3			202
#define	MZ2_WIDOW2_BEAM_SWEEP_4			203
#define	MZ2_WIDOW2_BEAM_SWEEP_5			204
#define	MZ2_WIDOW2_BEAM_SWEEP_6			205
#define	MZ2_WIDOW2_BEAM_SWEEP_7			206
#define	MZ2_WIDOW2_BEAM_SWEEP_8			207
#define	MZ2_WIDOW2_BEAM_SWEEP_9			208
#define	MZ2_WIDOW2_BEAM_SWEEP_10		209
#define	MZ2_WIDOW2_BEAM_SWEEP_11		210

// ROGUE

extern	vec3_t monster_flash_offset [];


// temp entity events
//
// Temp entity events are for things that happen
// at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed
// and broadcast.
typedef enum
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
} temp_event_t;

#define SPLASH_UNKNOWN		0
#define SPLASH_SPARKS		1
#define SPLASH_BLUE_WATER	2
#define SPLASH_BROWN_WATER	3
#define SPLASH_SLIME		4
#define	SPLASH_LAVA			5
#define SPLASH_BLOOD		6


// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
#define	CHAN_AUTO               0
#define	CHAN_WEAPON             1
#define	CHAN_VOICE              2
#define	CHAN_ITEM               3
#define	CHAN_BODY               4
// modifier flags
#define	CHAN_NO_PHS_ADD			8	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define	CHAN_RELIABLE			16	// send by reliable message, not datagram


// sound attenuation values
#define	ATTN_NONE               0	// full volume the entire level
#define	ATTN_NORM               1
#define	ATTN_IDLE               2
#define	ATTN_STATIC             3	// diminish very rapidly with distance


// player_state->stats[] indexes
//#define STAT_HEALTH_ICON		0
#define	STAT_HEALTH				1
//#define	STAT_AMMO_ICON			2
#define	STAT_AMMO				3
//#define	STAT_ARMOR_ICON			4
//#define	STAT_ARMOR				5
//#define	STAT_SELECTED_ICON		6
//#define	STAT_PICKUP_ICON		7
//#define	STAT_PICKUP_STRING		8
//#define	STAT_TIMER_ICON			9
//#define	STAT_TIMER				10
//#define	STAT_HELPICON			11
//#define	STAT_SELECTED_ITEM		12
//#define	STAT_LAYOUTS			13
//#define	STAT_FRAGS				14
//#define	STAT_FLASHES			15		// cleared each frame, 1 = health, 2 = armor
//#define STAT_CHASE				16
//#define STAT_SPECTATOR			17

// jithud -- Paintball2 HUD stuff.
#define	STAT_CO2_12G			1
#define	STAT_CO2_7OZ			2
#define	STAT_CO2_12OZ			3
#define	STAT_CO2_20OZ			4
#define	STAT_CO2BAR				5
#define	STAT_PB_MASK			6
#define STAT_PB_HOPPER			7
#define STAT_PB_BARREL			8
#define STAT_PB_PAINTCOUNT		9
#define STAT_PB_GRENADE			10
// 11 ***unused***
//#define STAT_PB_PAINTGREN		10
#define STAT_OLD_PB_SMOKEGREN	11
#define STAT_GUNTEMP			12
#define	STAT_LAYOUTS			13
#define	STAT_FRAGS				14
#define	STAT_FLASHES			15		// cleared each frame, 1 = health, 2 = armor
#define STAT_ALIVE_TEAM3		16
#define STAT_ALIVE_TEAM4		17
#define STAT_PB_TEAM1			18
#define STAT_PB_TEAM2			19
#define STAT_PB_TEAM3			20
#define STAT_PB_TEAM4			21
#define STAT_SCORE_TEAM1		22
#define STAT_SCORE_TEAM2		23
#define STAT_SCORE_TEAM3		24
#define STAT_SCORE_TEAM4		25
#define STAT_TIMELEFT			26 // jit - finally found one unused :)
#define STAT_ALIVE_TEAM1		27
#define STAT_ALIVE_TEAM2		28
#define	STAT_STREAK				29
#define	STAT_ID					30 // Player ID
#define STAT_UNUSED				31

#define	MAX_STATS				32


// dmflags->value flags
#define	DF_NO_HEALTH		0x00000001	// 1
#define	DF_NO_ITEMS			0x00000002	// 2
#define	DF_WEAPONS_STAY		0x00000004	// 4
#define	DF_NO_FALLING		0x00000008	// 8
#define	DF_INSTANT_ITEMS	0x00000010	// 16
#define	DF_SAME_LEVEL		0x00000020	// 32
#define DF_SKINTEAMS		0x00000040	// 64
#define DF_MODELTEAMS		0x00000080	// 128
#define DF_NO_FRIENDLY_FIRE	0x00000100	// 256
#define	DF_SPAWN_FARTHEST	0x00000200	// 512
#define DF_FORCE_RESPAWN	0x00000400	// 1024
#define DF_NO_ARMOR			0x00000800	// 2048
#define DF_ALLOW_EXIT		0x00001000	// 4096
#define DF_INFINITE_AMMO	0x00002000	// 8192
#define DF_QUAD_DROP		0x00004000	// 16384
#define DF_FIXED_FOV		0x00008000	// 32768

// RAFAEL
#define	DF_QUADFIRE_DROP	0x00010000	// 65536

//ROGUE
#define DF_NO_MINES			0x00020000
#define DF_NO_STACK_DOUBLE	0x00040000
#define DF_NO_NUKES			0x00080000
#define DF_NO_SPHERES		0x00100000
//ROGUE

/*
ROGUE - VERSIONS
1234	08/13/1998		Activision
1235	08/14/1998		Id Software
1236	08/15/1998		Steve Tietze
1237	08/15/1998		Phil Dobranski
1238	08/15/1998		John Sheley
1239	08/17/1998		Barrett Alexander
1230	08/17/1998		Brandon Fish
1245	08/17/1998		Don MacAskill
1246	08/17/1998		David "Zoid" Kirsch
1247	08/17/1998		Manu Smith
1248	08/17/1998		Geoff Scully
1249	08/17/1998		Andy Van Fossen
1240	08/20/1998		Activision Build 2
1256	08/20/1998		Ranger Clan
1257	08/20/1998		Ensemble Studios
1258	08/21/1998		Robert Duffy
1259	08/21/1998		Stephen Seachord
1250	08/21/1998		Stephen Heaslip
1267	08/21/1998		Samir Sandesara
1268	08/21/1998		Oliver Wyman
1269	08/21/1998		Steven Marchegiano
1260	08/21/1998		Build #2 for Nihilistic
1278	08/21/1998		Build #2 for Ensemble

9999	08/20/1998		Internal Use
*/
#define ROGUE_VERSION_ID		1278

#define ROGUE_VERSION_STRING	"08/21/1998 Beta 2 for Ensemble"

// ROGUE
/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))


//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
#define	CS_NAME				0
#define	CS_CDTRACK			1
#define	CS_SKY				2
#define	CS_SKYAXIS			3		// %f %f %f format
#define	CS_SKYROTATE		4
#define	CS_STATUSBAR		5		// display program string

#define CS_AIRACCEL			29		// air acceleration control
#define	CS_MAXCLIENTS		30
#define	CS_MAPCHECKSUM		31		// for catching cheater maps

#define	CS_MODELS			32
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS) // 32 + 256 = 
#define	CS_IMAGES			(CS_SOUNDS+MAX_SOUNDS) // + 256 = 544
#define	CS_LIGHTS			(CS_IMAGES+MAX_IMAGES) // + 256 = 800
#define	CS_ITEMS			(CS_LIGHTS+MAX_LIGHTSTYLES) // + 256 = 1056
#define	CS_PLAYERSKINS		(CS_ITEMS+MAX_ITEMS) // + 256 = 1312
#define CS_GENERAL			(CS_PLAYERSKINS+MAX_CLIENTS) // + 256 = 1568
#define	MAX_CONFIGSTRINGS	(CS_GENERAL+MAX_GENERAL) // + 512 = 2080

#define CS_REQUIREDFILES	(CS_GENERAL+2) // jitdownload = 1568 + 2 = 1570
#define CS_SERVERGVERSION	(CS_GENERAL+7) // jitversion = 1568 + 7 = 1575
#define CS_SERVEREVERSION	(CS_GENERAL+8) // jitversion = 1568 + 8 = 1576
#define CS_TEAMINDEXES		(CS_GENERAL+9) // jitscores
#define CS_GAMETYPE			(CS_GENERAL+10) // jitscores
#define CS_MOVEPHYSICS		(CS_GENERAL+11)
#define CS_WHATEVERSNEXT	(CS_GENERAL+12) // use this for additional stuff

#define CS_EVENTS			(CS_GENERAL+256) // jitevents

//==============================================


// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
typedef enum
{
	EV_NONE,
	EV_ITEM_RESPAWN,
	EV_FOOTSTEP,
	EV_FALLSHORT,
	EV_FALL,
	EV_FALLFAR,
	EV_PLAYER_TELEPORT,
	EV_OTHER_TELEPORT
} entity_event_t;


// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
typedef struct entity_state_s
{
	int		number;			// edict index

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	old_origin;		// for lerping
	int		modelindex;
	int		modelindex2, modelindex3, modelindex4;	// weapons, CTF flags, etc
	int		frame;
	int		skinnum;
	unsigned int		effects;		// PGM - we're filling it, so it needs to be unsigned
	int		renderfx;
	int		solid;			// for client side prediction, 8*(bits 0-4) is x/y radius
							// 8*(bits 5-9) is z down distance, 8(bits10-15) is z up
							// gi.linkentity sets this properly
	int		sound;			// for looping sounds, to guarantee shutoff
	int		event;			// impulse events -- muzzle flashes, footsteps, etc
							// events only go out for a single frame, they
							// are automatically cleared each frame
} entity_state_t;

//==============================================


// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pmove_state_t changes will be reletive to client
// frame rates
typedef struct
{
	pmove_state_t	pmove;		// for prediction

	// these fields do not need to be communicated bit-precise

	vec3_t		viewangles;		// for fixed views
	vec3_t		viewoffset;		// add to pmovestate->origin
	vec3_t		kick_angles;	// add to view direction to get render angles
								// set by weapon kicks, pain effects, etc

	vec3_t		gunangles;
	vec3_t		gunoffset;
	int			gunindex;
	int			gunframe;

	float		blend[4];		// rgba full screen effect
	
	float		fov;			// horizontal field of view

	int			rdflags;		// refdef flags

	short		stats[MAX_STATS];		// fast status bar updates
} player_state_t;


// ==================
// PGM 
#define VIDREF_GL		1
#define VIDREF_SOFT		2
#define VIDREF_OTHER	3

extern int vidref_val;
// PGM
// ==================


// jittext / jitconsole
#define UCHAR_ENDFORMAT	133
#define UCHAR_UNDERLINE	134
#define UCHAR_ITALICS	135
#define UCHAR_COLOR		136
#define SCHAR_ENDFORMAT	((char)133)
#define SCHAR_UNDERLINE	((char)134)
#define SCHAR_ITALICS	((char)135)
#define SCHAR_COLOR		((char)136)

#define COLOR_CHAT		('z'+16)
#define COLOR_MAPNAME	']'

// =======
// jithash
typedef struct hash_node_s {
	void *data;
	char *key;
	struct hash_node_s *next;
} hash_node_t;

typedef struct hash_table_s {
	int mask; // should be size - 1
	int size; // should be a power of 2
	void (*free_func)(void *data); // function to call when freeing elements
	hash_node_t **table;
} hash_table_t;

void hash_table_init (hash_table_t *table, unsigned int size, void *free_func);
void hash_table_free (hash_table_t *table);
void hash_add (hash_table_t *table, const unsigned char *key, void *data);
void *hash_get (hash_table_t *table, const unsigned char *key);
void hash_delete (hash_table_t *table, const unsigned char *key);
// jithash
// ========

// jitevents
typedef enum {
	EVENT_NONE = 0,
	EVENT_ENTER = 1,
	EVENT_JOIN = 2,
	EVENT_KILL = 3,
	EVENT_DEATH = 4,
	EVENT_GRAB = 5,
	EVENT_CAP = 6,
	EVENT_ROUNDSTART = 7,
	EVENT_ROUNDOVER = 8,
	EVENT_SUICIDE = 9,
	EVENT_ADMINKILL = 10,
	EVENT_FFIRE = 11,
	EVENT_QUIT = 12,
	EVENT_RESPAWN = 13,
	EVENT_DROPFLAG = 14,
	EVENT_DISCONNECT = 15,
	EVENT_OVERTIME = 16,
	EVENT_PONG_SCORE = 17, // not actually used
	EVENT_JAILESCAPE = 18
} game_event_t;

qboolean FileExists (const char *filename); // jit

#define NULLTERMINATE(a) a[sizeof(a)-1] = '\0' // jitsecurity
#ifndef WIN32
#define _vsnprintf(a, b, c, d) vsnprintf(a, b, c, d)
#endif

float RadiusFromBounds (const vec3_t mins, const vec3_t maxs); // jitskm
int Q_snprintfz (char *dest, size_t size, const char *fmt, ...); // jitskm
void Q_strncpyz (char *dest, const char *src, size_t size); // jitskm
#ifdef HAVE_STRLCAT
#define Q_strncatz(a, b, c) strlcat(a, b, c)
#else
void Q_strncatz (char *dest, const char *src, size_t size);
#endif
void Q_strncpyzna (char *dest, const char *src, size_t size); // jit
void strip_garbage (char *cout, const char *cin, size_t size_out); // jit

#endif
