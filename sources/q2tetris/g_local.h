// g_local.h -- local definitions for game module

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"Q2Tetris"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5

#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level
#define TAG_TET_ARENA	767		// clear when unloading the arenas (@LEVEL?)


// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;
} game_locals_t;


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

	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;
	int		arena;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;



extern	game_locals_t	game;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*maxclients;

// QTET cvars

extern cvar_t *highscore;

#define world	(&g_edicts[0])

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1

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

//
// g_utils.c
//
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);


//
// g_client.c
//
void respawn (edict_t *ent);
void PutClientInServer (edict_t *ent);
void ClientBeginServerFrame (edict_t *ent);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void showScores (edict_t *ent);

//
// g_main.c
//
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);


//============================================================================

// ----------------------------------------
// What used to be tetris.h, but was simpler to roll in to here
//
// BEGIN tetris.h
/*
#define STATE_NONE 0
#define STATE_FALLING 1
#define STATE_DROPPING 2
#define STATE_FALLEN 3
#define STATE_WAITING 4
#define STATE_GAMEOVER 5
*/
typedef struct T_arena_s {
	edict_t *player_start;
	edict_t *block_start;
	edict_t *block_end;
	vec3_t xoff, yoff;
	// bounds go here
	int height;
	int width;
	int blocksize;
	struct gclient_s* player;
	edict_t** map;
	edict_t* current_piece;
	// this is all necessary because tetris pieces do
	// not really rotate in the rational manner you might
	// expect (the 'z' shaped pieces are a prime example)
	int cp_shape, cp_rot, cp_x, cp_y;
	int dwtime, bstime, basedwtime, wftime;
} T_arena_t;


// max # of arenas
#define MAX_ARENAS 32

// QTET: index of all game arenas in this structure
//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int		framenum;
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

	int			players;		// FIXME: count when needed, don't store

	int			arenacount;
	struct T_arena_s	arenas[MAX_ARENAS];
} level_locals_t;

extern	level_locals_t	level;

typedef enum {
	NO_GAME,
	WAIT_BLOCKSPAWN,
	BLOCK_FALLING,
	BLOCK_DROPPING,
	SCAN_FALLEN,
	WAIT_FALLEN,
	GAME_OVER
} T_client_state_t;

typedef enum {
	BL_LEFT,
	BL_RIGHT,
	BL_ROT,
	BL_DROP
} T_command_t;

// END tetris.h


// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;
	
// QTETRIS: gclient info here

	char *name;	//name communicated to other players
	int score;	// score at tetris
	int level; // level
	int rows; // rows this level
	T_arena_t *arena;  // current arena we're playing in	, NULL if no game
	int timer;
	T_client_state_t state;
};

struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
	qboolean	inuse;
	int			linkcount;
	link_t		area;				// linked to a division node or leaf
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;
	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;
	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	// Good, wholesome id code.  mmmmmm.
	char		*message;
	char		*classname;
	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*map;			// target_changelevel

	// Fresh, unwholesome phooky code
	struct edict_s *qt_next_block;  // pieces are linked lists of blocks
};

// More qtet #defines
#define HIGH4(x) (x / 10000)
#define LOW4(x) (x % 10000)

// More qtet globals.  Blecccch.
extern vec3_t block_size;
