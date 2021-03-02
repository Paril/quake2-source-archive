///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LicenseToKill/src/acesrc/acebot.h 14    26/11/99 0:58 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/acebot.h $
 * 
 * 14    26/11/99 0:58 Riever
 * Max nodes up to 1200
 * 
 * 13    21/10/99 8:19 Riever
 * ltk_showpath cvar enables display of bot paths.
 * 
 * 12    16/10/99 9:11 Riever
 * Declared new config functions.
 * 
 * 11    16/10/99 8:50 Riever
 * Added ltk_chat CVAR
 * 
 * 10    6/10/99 17:40 Riever
 * Added TeamPlay state STATE_POSITION to enable bots to seperate and
 * avoid centipede formations.
 * 
 * 9     29/09/99 13:32 Riever
 * Chnaged DrawPath declaration to accept edict_t *self so it can use the
 * AntPath SLL code.
 * 
 * 8     27/09/99 16:01 Riever
 * Added "self" to ACEND_UpdateNodeEdge calls
 * 
 * 7     27/09/99 14:51 Riever
 * Increased node density by lowering node_gap to 96.
 * 
 * 6     21/09/99 13:10 Riever
 * defined ITEMLIST entries for AQ2 ammo
 * 
 * 5     17/09/99 17:11 Riever
 * New Node structure
 * Link structure added
 * MAXLINKS defined
 * NODE_DOOR definition added
 * 
 * 4     14/09/99 8:05 Riever
 * Added ltk_skill cvar
 * 
 * 3     13/09/99 19:50 Riever
 * Added header
 * 
 * 2     13/09/99 19:49 Riever
 * Initial check in at 0.02alpha
 *
 */

///////////////////////////////////////////////////////////////////////	
//
//  acebot.h - Main header file for ACEBOT
// 
// 
///////////////////////////////////////////////////////////////////////

#ifndef _ACEBOT_H
#define _ACEBOT_H

// Only 100 allowed for now (probably never be enough edicts for 'em
#define MAX_BOTS 100

// Platform states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

// Maximum nodes
#define MAX_NODES 1200

// Link types
#define INVALID -1

// Node types
#define NODE_MOVE 0
#define NODE_LADDER 1
#define NODE_PLATFORM 2
#define NODE_TELEPORTER 3
#define NODE_ITEM 4
#define NODE_WATER 5
#define NODE_GRAPPLE 6
#define NODE_JUMP 7
#define NODE_DOOR 8	// - RiEvEr
#define NODE_ALL 99 // For selecting all nodes

// Density setting for nodes
#define NODE_DENSITY 96

// Maximum links per node
#define MAXLINKS 12

//AQ2 ADD
extern cvar_t	*ltk_skill;	// Skill setting for bots, range 0-10	
extern cvar_t	*ltk_showpath;	// Toggles display of bot paths in debug mode
extern cvar_t	*ltk_chat;	// Chat setting for bots, off or on (0,1)

//AQ2 END

// Bot state types
#define STATE_STAND 0
#define STATE_MOVE 1
#define STATE_ATTACK 2
#define STATE_WANDER 3
#define STATE_FLEE 4
#define STATE_POSITION 5

#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_FORWARD 2
#define MOVE_BACK 3

// Item defines (got this list from somewhere??....so thanks to whoever created it)
#define ITEMLIST_NULLINDEX			0
#define ITEMLIST_BODYARMOR			1
#define ITEMLIST_COMBATARMOR		2
#define ITEMLIST_JACKETARMOR		3
#define ITEMLIST_ARMORSHARD			4
#define ITEMLIST_POWERSCREEN		5
#define ITEMLIST_POWERSHIELD		6

#define ITEMLIST_GRAPPLE            7

#define ITEMLIST_BLASTER			8
#define ITEMLIST_SHOTGUN			9
#define ITEMLIST_SUPERSHOTGUN		10
#define ITEMLIST_MACHINEGUN			11
#define ITEMLIST_CHAINGUN			12
#define ITEMLIST_GRENADES			13
#define ITEMLIST_GRENADELAUNCHER	14
#define ITEMLIST_ROCKETLAUNCHER		15
#define ITEMLIST_HYPERBLASTER		16
#define ITEMLIST_RAILGUN			17
#define ITEMLIST_BFG10K				18

#define ITEMLIST_SHELLS				19
#define ITEMLIST_BULLETS			20
#define ITEMLIST_CELLS				21
#define ITEMLIST_ROCKETS			22
#define ITEMLIST_SLUGS				23
#define ITEMLIST_QUADDAMAGE			24
#define ITEMLIST_INVULNERABILITY	25
#define ITEMLIST_SILENCER			26
#define ITEMLIST_REBREATHER			27
#define ITEMLIST_ENVIRONMENTSUIT	28
#define ITEMLIST_ANCIENTHEAD		29
#define ITEMLIST_ADRENALINE			30
#define ITEMLIST_BANDOLIER			31
#define ITEMLIST_AMMOPACK			32
#define ITEMLIST_DATACD				33
#define ITEMLIST_POWERCUBE			34
#define ITEMLIST_PYRAMIDKEY			35
#define ITEMLIST_DATASPINNER		36
#define ITEMLIST_SECURITYPASS		37
#define ITEMLIST_BLUEKEY			38
#define ITEMLIST_REDKEY				39
#define ITEMLIST_COMMANDERSHEAD		40
#define ITEMLIST_AIRSTRIKEMARKER	41
#define ITEMLIST_HEALTH				42

// new for ctf
#define ITEMLIST_FLAG1              43
#define ITEMLIST_FLAG2              44
#define ITEMLIST_RESISTANCETECH     45
#define ITEMLIST_STRENGTHTECH       46
#define ITEMLIST_HASTETECH          47
#define ITEMLIST_REGENERATIONTECH   48

// my additions
#define ITEMLIST_HEALTH_SMALL		49
#define ITEMLIST_HEALTH_MEDIUM		50
#define ITEMLIST_HEALTH_LARGE		51
#define ITEMLIST_BOT				52
#define ITEMLIST_PLAYER				53
#define ITEMLIST_HEALTH_MEGA        54

//AQ2 ADD
#define ITEMLIST_MK23					55	//"MK23 Pistol"
#define ITEMLIST_MP5					56	//"MP5/10 Submachinegun"
#define ITEMLIST_M4						57	//"M4 Assault Rifle"
#define ITEMLIST_M3						58	//"M3 Super 90 Assault Shotgun"
#define ITEMLIST_HC						59	// "Handcannon"
#define ITEMLIST_SNIPER					60	// "Sniper Rifle"
#define ITEMLIST_DUAL					61	// "Dual MK23 Pistols"
#define ITEMLIST_KNIFE					62	// "Combat Knife"
#define ITEMLIST_GRENADE				63	// "M26 Fragmentation Grenade"

#define ITEMLIST_SIL					64	// "Silencer"
#define ITEMLIST_SLIP					65	// "Stealth Slippers"
#define ITEMLIST_BAND					66	// "Bandolier"
#define ITEMLIST_KEV					67	// "Kevlar Vest"
#define ITEMLIST_LASER					68	// "Lasersight"

#define ITEMLIST_AMMO_CLIP			69
#define ITEMLIST_AMMO_M3			70
#define ITEMLIST_AMMO_M4			71
#define ITEMLIST_AMMO_MAG			72
#define ITEMLIST_AMMO_SNIPER		73

//AQ2 END

typedef struct nodelink_s
{
	short int		targetNode;
	float	cost; // updated for pathsearch algorithm

}nodelink_t; // RiEvEr


// Node structure
typedef struct node_s
{
	vec3_t origin; // Using Id's representation
	int type;   // type of node
	short int nodenum;	// node number - RiEvEr
//	short int lightlevel;	// obvious... - RiEvEr
	nodelink_t	links[MAXLINKS];	// store all links. - RiEvEr

} node_t;

typedef struct item_table_s
{
	int item;
	float weight;
	edict_t *ent;
	int node;

} item_table_t;

extern int num_players;
extern edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

// extern decs
extern node_t nodes[MAX_NODES]; 
extern item_table_t item_table[MAX_EDICTS];
extern qboolean debug_mode;
extern int numnodes;
extern int num_items;

// id Function Protos I need
void     LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     TossClientWeapon (edict_t *self);
void     ClientThink (edict_t *ent, usercmd_t *ucmd);
void     SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void     ClientUserinfoChanged (edict_t *ent, char *userinfo);
void     CopyToBodyQue (edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void     Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);

// acebot_ai.c protos
void     ACEAI_Think (edict_t *self);
void     ACEAI_PickLongRangeGoal(edict_t *self);
void     ACEAI_PickShortRangeGoal(edict_t *self);
qboolean ACEAI_FindEnemy(edict_t *self);
void     ACEAI_ChooseWeapon(edict_t *self);

// acebot_cmds.c protos
qboolean ACECM_Commands(edict_t *ent);
void     ACECM_Store();

// acebot_items.c protos
void     ACEIT_PlayerAdded(edict_t *ent);
void     ACEIT_PlayerRemoved(edict_t *ent);
qboolean ACEIT_IsVisible(edict_t *self, vec3_t goal);
qboolean ACEIT_IsReachable(edict_t *self,vec3_t goal);
qboolean ACEIT_ChangeWeapon (edict_t *ent, gitem_t *item);
//AQ2 ADD
qboolean ACEIT_ChangeMK23SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeHCSpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeSniperSpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeM4SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeM3SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeMP5SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeDualSpecialWeapon (edict_t *ent, gitem_t *item);
//AQ2 END
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other);
float	 ACEIT_ItemNeed(edict_t *self, int item);
int		 ACEIT_ClassnameToIndex(char *classname);
void     ACEIT_BuildItemNodeTable (qboolean rebuild);

// acebot_movement.c protos
qboolean ACEMV_SpecialMove(edict_t *self,usercmd_t *ucmd);
void     ACEMV_Move(edict_t *self, usercmd_t *ucmd);
void     ACEMV_Attack (edict_t *self, usercmd_t *ucmd);
void     ACEMV_Wander (edict_t *self, usercmd_t *ucmd);

// acebot_nodes.c protos
int      ACEND_FindCost(int from, int to);
int      ACEND_FindCloseReachableNode(edict_t *self, int dist, int type);
int      ACEND_FindClosestReachableNode(edict_t *self, int range, int type);
void     ACEND_SetGoal(edict_t *self, int goal_node);
qboolean ACEND_FollowPath(edict_t *self);
void     ACEND_GrapFired(edict_t *self);
qboolean ACEND_CheckForLadder(edict_t *self);
void     ACEND_PathMap(edict_t *self);
void     ACEND_InitNodes(void);
void     ACEND_ShowNode(int node);
void     ACEND_DrawPath(edict_t *self);
void     ACEND_ShowPath(edict_t *self, int goal_node);
int      ACEND_AddNode(edict_t *self, int type);
void     ACEND_UpdateNodeEdge(edict_t *self, int from, int to);
void     ACEND_RemoveNodeEdge(edict_t *self, int from, int to);
void     ACEND_ResolveAllPaths();
void     ACEND_SaveNodes();
void     ACEND_LoadNodes();

// acebot_spawn.c protos
void	 ACESP_SaveBots();
void	 ACESP_LoadBots();
void	 ACESP_LoadBotConfig();
void	 ACESP_SpawnBotFromConfig( char *inString );
void     ACESP_HoldSpawn(edict_t *self);
void     ACESP_PutClientInServer (edict_t *bot, qboolean respawn, int team);
void     ACESP_Respawn (edict_t *self);
edict_t *ACESP_FindFreeClient (void);
void     ACESP_SetName(edict_t *bot, char *name, char *skin, char *team);
void     ACESP_SpawnBot (char *team, char *name, char *skin, char *userinfo);
void     ACESP_ReAddBots();
void     ACESP_RemoveBot(char *name);
void	 safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void     safe_centerprintf (edict_t *ent, char *fmt, ...);
void     safe_bprintf (int printlevel, char *fmt, ...);
void     debug_printf (char *fmt, ...);

#endif