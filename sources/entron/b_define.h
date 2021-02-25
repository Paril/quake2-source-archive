/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_define.h
  Description: All data structures needed by Entron.
               (see beholder.h)

\**********************************************************/


#ifndef B_DEFINE
#define B_DEFINE



// Means of Death
// CCH
#define MOD_HAMMER		   256
#define MOD_U23K           257
#define MOD_PLASMA			258
#define MOD_RIPPER			259
#define MOD_CEC            260
#define MOD_RIFLE          261
#define MOD_RIFLE_HEAD     262
#define MOD_RIFLE_TORSO    263
#define MOD_RIFLE_GROIN    264
#define MOD_RIFLE_LEG      265
#define MOD_TEARGAS        266
#define MOD_ASSIMILATOR    267
#define MOD_STAFF          268
#define MOD_GATTLING       269
#define MOD_WASP           270
#define MOD_RIFLE_AUTO     271





// Monster/Entity type
#define  TYPE_MONSTER_ORGANIC       1
#define  TYPE_MONSTER_SILICON       2
#define  TYPE_MONSTER_METALIC       4
#define  TYPE_MONSTER_TEAMMATE      8
#define  TYPE_MONSTER_FOE          16
#define  TYPE_MONSTER_FLAT         32
#define  TYPE_SLAVE_ACTOR          64
#define  TYPE_SLAVE_WARRIOR       128
#define  TYPE_MONSTER_AMBUSH      256
#define  TYPE_WEAPON_GRENADE      303
#define  TYPE_MONSTER_DROPBLOOD   512


// Client SPECs
#define  SPEC_GASMASK               4

// Monster/Entity status
#define  STATUS_CEC                 1
#define  STATUS_MONSTER_HIDE        2
#define  STATUS_MONSTER_COMMUNICATE 4
#define  STATUS_MONSTER_ROAM        8
#define  STATUS_MONSTER_NO_GOAL    16
#define  STATUS_MONSTER_AMBUSH     32
#define  STATUS_MONSTER_NOSHOOT    64



#define  STATUS_SLAVE_CHASE       128
#define  STATUS_SLAVE_ATTACK      256
#define  STATUS_MONSTER_ENEMYVIEW 512

#define  STATUS_SCR_CRITICAL      8192


#define  STATUS_3RD_VIEW            2
#define  STATUS_SCENE_VIEW          4
#define  STATUS_FREE_CAM            8
#define  STATUS_IR						16

// FLIP status:
#define FLIP_SHELL                  1       
#define FLIP_WATER                  2       
#define FLIP_CAMERA                 4
#define FLIP_FLAG                   8
#define FLIP_DEADBODY              16
#define FLIP_TRAMPOLINE            32
#define FLIP_NODE                  64 
#define FLIP_AMBUSH_NODE          128
#define FLIP_WEAPON               256

// Not in game. Only NNE.
#define FLIP_DOOR                1024
#define FLIP_NODE_WALL           2048
#define FLIP_LIGHT               4096

// Monster movement:
#define	STEPSIZE	                 18
#define	DI_NODIR	                 -1


// Internal Map Navigation system for bots.
#define MAX_ADJACENT_NODES         16 
#define NODE_TYPE_INUSE             1
#define NODE_TYPE_VISITED           2

// Weapon Reload times:

#define RELOAD_HAMMER             100
#define RELOAD_U23K               16
#define RELOAD_U23K2              32
#define RELOAD_PLASMAGUN          100
#define RELOAD_GATTLING           100
#define RELOAD_RIPPER             6
#define RELOAD_STAFF              100
#define RELOAD_ASSIM              6
#define RELOAD_CEC                100
#define RELOAD_RIFLE              100
#define RELOAD_WASP               100


#define	TEXTURE_DIRT				1024
#define	TEXTURE_METAL				2048
#define	TEXTURE_TILES				4096


// Map Node structure. Map nodes will be laid out all over the map and will
// help mosnters navigate and find a player without keeping track of trails.
// The nodes will connect paths which will be used by monsters/bots to reach
// a certain destination (Mainly ent->enemy). 

#define  MAX_NODES 255

struct map_node_s
{
   vec3_t	origin;                                // Node Origin
	int		type;                                  // Node type
   unsigned char number;                           // Node number in array.
   unsigned char nodes;                            // Number of nodes
   unsigned char route[256];                       // Nearest route to node
   struct   map_node_s *adjacent[MAX_ADJACENT_NODES];       // Adjacent nodes
};
typedef struct map_node_s map_node_t;

struct ambush_node_s
{
   vec3_t	origin;                                // Node Origin
	int		type;                                  // Node type
   unsigned char number;                           // Node number in array.
};
typedef struct ambush_node_s ambush_node_t;

// Topaz:
map_node_t     *g_nodes;
ambush_node_t  *g_ambushes;

struct script_s
{
   int command;
   int ParamInt[2];
   float ParamFloat[2];
   char *ParamString[2]; //[256];
   vec3_t ParamVector;
};

typedef struct script_s script_t;

struct script_file_s
{
   char filename[256];
   int numcommands;
   int intercepts;
   int on_see;
   int on_touch;
   int on_pain;
   int on_block;
   int see_cmd;
   int touch_cmd;
   int pain_cmd;
   int block_cmd;
   script_t *commands;
};
struct intercept_s
{
   char activator[256];
   int  index;
};

typedef struct script_file_s script_file_t;
typedef struct intercept_s intercept_t;

script_file_t script_file[25];
int script_index;
#endif