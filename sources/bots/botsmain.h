//
// Add #include "\quake2\gsmod\gsmod.h" to the top of
//
// gsmod.c
// g_cmds.c	
// g_spawn.c	
//

#define MAX_MAPS   52

#define HELP1	"Type 'help' in the console for a list of aliases.\n"
#define HELP2	"Type 'use hook' to use the grappling hook or just switch weapons to hook.\n"
#define HELP3	"Women can jump higher than men but carry less ammo.\n"
#define HELP4	"Women get a faster blaster.\n"
#define HELP5	"Battle of the Sexes Quake 2 Mod written by Jesse Herrera aka Spinoza\n"
#define HELP6	"Check 'spinoza.warzone.com' for the latest information.\n"
#define HELP7	"Remember to type 'vote' in the console to vote on a new map.\n"
#define HELP8	"Women can run faster than Men.\n"
#define HELP9	"Remember to read the Strategy Guide and FAQ written by Pain on the website.\n"
#define HELP10	""

#define MAX_STATUSSTRING 1398

void ShowGun(edict_t *ent);
void gsmod_ClientUserinfoChanged (edict_t *ent, char *userinfo, qboolean spawned, qboolean newskin);
void gsmod_ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void gsmod_InitClientMod (edict_t *ent, qboolean spawned);
void gsmod_Gravity (edict_t *ent, usercmd_t *ucmd);
void gsmod_Drop_item (edict_t *ent, char *itemname);
void gsmod_DisplayMsg (edict_t *ent, char *filename);
void gsmod_ClientCommand (edict_t *ent, char *cmd, char *parms);
void gsmod_Weapon_RocketLauncher (edict_t *ent);
void gsmod_Weapon_Blaster (edict_t *ent);
void gsmod_Taunt (edict_t *ent);
void gsmod_InitClientPersistant (gclient_t *client);
void gsmod_DropArmor (edict_t *ent, char *armorname);
void gsmod_Vote (edict_t *ent, qboolean redisplay, char *parms);
void gsmod_ReadMaps (char *filename);
void gsmod_CastVote (edict_t *ent, char *votemap);
void gsmod_ShowPlayerID (edict_t *ent);
void gsmod_LevelReset (void);
void gsmod_InitVoting(void);
void gsmod_Rebind (edict_t *ent);
void gsmod_Suicide (edict_t *ent);
void gsmod_AutoRewire (edict_t *ent);
void gsmod_QueueCommand(edict_t *ent, char *cmdstring);
void gsmod_LightsToggle(qboolean ab_toggle);
void gsmod_ClearVotes(void);
void gsmod_SetStats (edict_t *ent);
int gsmod_NumPlayers(void);
char *gsmod_TeamColor (int team);
void gsmod_Killmenu (edict_t *ent);
qboolean gsmod_BannedIP (char *ip, char *filename);
qboolean gsmod_PlayerReset (edict_t *ent, qboolean respawn);
qboolean gsmod_Pickup_Armor(edict_t *ent, char *pickup_name, int armorindex);
qboolean gsmod_Pickup_Weapon (edict_t *ent, edict_t *other);

typedef struct
{
	int		numvotes;
	char	levelname[20];
	char	levelmap[12];
	char	grouplist[20];
} vote_t;

extern vote_t   votes[MAX_MAPS];
extern int		 nextmap;
extern int		 votetotal;
extern float	 startvote;
extern int		 votemode;		// 1-classic, 2-voting w/ini, 3-sequential, 4-groups
extern int		 rotatemode;	// 1-sequential, 2-eliminzation, 3-random
extern qboolean logvotes;
extern qboolean mapsread;

extern int		 currentmap;
extern int		 nextinseq;
extern int		 groupx;
extern int		 groupx_count;
extern int		 elimx;
extern int		 elimx_count;
extern int		 randomx;
extern int		 randomx_count;

extern float	 nextwarn;
extern float	 startimpeach;
extern edict_t	 *earthquake;
extern qboolean lights;

// Laser stuff

// controlling parameters	
#define	LASER_TIME					15
#define	CELLS_FOR_LASER				50	
#define	LASER_DAMAGE				150
#define	LASER_MOUNT_DAMAGE			150	
#define	LASER_MOUNT_DAMAGE_RADIUS	150

// In-built Quake2 routines
void target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
void target_laser_think (edict_t *self);
void target_laser_on (edict_t *self);	
void target_laser_off (edict_t *self);
