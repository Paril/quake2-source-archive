//
// g_local.h, g_main.c were modified to add team variables
//

//
// Add #include "\quake2\gsmod\teamplay.h" to the top of
//
// g_save.c
// p_client.c
// g_combat.c
// g_items.c
// p_view.c
// g_spawn.c
//

#define TEAMHELP1	"Kamikazee needs 11+ grens for quad suicide and 31+ for apocalypse suicide.\n"
#define TEAMHELP2	"Captains and Snipers get the tracker. Type 'tracker' in the console.\n"
#define TEAMHELP3	"To join a different team type 'rejoin' in the console.\n"
#define TEAMHELP4	"To change player classes type the class name in the console.\n"
#define TEAMHELP5	"The color of the grappling hook is the same color as the team you're on.\n"
#define TEAMHELP6	"Soldiers and Berzerkers do not get the grappling hook.\n"
#define TEAMHELP7	"Remember if your getting killed to go for the enemy promotion key.\n"
#define TEAMHELP8	"Snipers can set alarms using the 'alarm' command in the console.\n"
#define TEAMHELP9	"Bodyguard's get a laser weapon using the 'laser' command in the console.\n"
#define TEAMHELP10	""

#define MAX_CLASSES	8
#define MAX_KEYS	3
#define MAX_LEVEL	3

#define STAT_TEAM1_PIC			17
#define STAT_TEAM1_CAPS			18
#define STAT_TEAM2_PIC			19
#define STAT_TEAM2_CAPS			20
#define STAT_FLAG_PIC			21
#define STAT_JOINED_TEAM1_PIC	22
#define STAT_JOINED_TEAM2_PIC	23
#define STAT_TEAM1_HEADER		24
#define STAT_TEAM2_HEADER		25
//#define STAT_TECH				26
#define STAT_ID_VIEW			27

#define STAT_CELLS_ICON			28
#define STAT_CELLS				29
#define STAT_GRENADES_ICON		30
#define STAT_GRENADES			31

#define TEAM_HEALTH_PROTECT			1		// No health damage from friendly fire
#define TEAM_ARMOR_PROTECT			2		// No armor damage from friendly fire
#define TEAM_ATTACKER_DAMAGE		4		// Attacker takes damage from hitting teammates
#define TEAM_FRAG_PENALTY			8		// One frag penalty for killing teammate
#define TEAM_DEATH_PENALTY          16		// Die when you kill a teammate.
#define TEAM_KEEPTEAMS_LEVELCHANGE	32		// Keep same teams after level change
#define TEAM_NOSPLASH_DAMAGE		64		// Will not inflict rocket splash damage on

#define BOTS_QUAD_RALLY				1		// Quad Rally is ON
#define BOTS_CLASSIC_VOTE			2		// Classic voting is used
#define BOTS_FAIR_TEAMS				4		// Fair teams code is ON
#define BOTS_AUTO_TEAM				8		// Auto select a team for a player is ON
#define BOTS_TEAM_SKINS				16		// Use class/team skins

#define BOTS_BODYGUARD				1		// 
#define BOTS_SNIPER					2		// 
#define BOTS_SOLDIER				4		// 
#define BOTS_BERZERKER				8		// 
#define BOTS_INFILTRATOR			16		// 
#define BOTS_KAMIKAZEE				32		// 
#define BOTS_NURSE					64		// 
#define BOTS_CAPTAIN				128		// 

#define	CLASS1	"Captain"
#define	CLASS2	"Bodyguard"
#define	CLASS3	"Sniper"
#define	CLASS4	"Soldier"
#define	CLASS5	"Berzerker"
#define	CLASS6	"Infiltrator"
#define	CLASS7	"Kamikazee"
#define	CLASS8	"Nurse"

//string lengths
#define LAYOUT_CHARS 1400
#define LAYOUT_SAFE 1398
#define STRING_CHARS 1024 //nice round number

void teamplay_InitKeys(void);
void teamplay_ClientBeginDeathmatch (edict_t *ent);
void teamplay_InitClientPersistant (gclient_t *client);
void teamplay_InitGame (void);
void teamplay_Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void teamplay_ClientPickTeam (edict_t *ent, int tteam, qboolean isauto);
void teamplay_ClientPickClass (edict_t *ent, int player_class, qboolean isauto);
void teamplay_ClientCommand (edict_t *ent, char *cmd, char *parms);
void teamplay_InitClass (edict_t *ent, qboolean spawned);
void teamplay_SpawnKey (edict_t *self, int keyindex, qboolean newspawn, qboolean keydropped);
void teamplay_ClientKill(edict_t *ent, int mod);
void teamplay_DropGeneral (edict_t *ent, gitem_t *item, edict_t *dropped);
void teamplay_DisplayStatus (edict_t *ent, edict_t *display);
void teamplay_WeaponToggle (edict_t *ent, gitem_t *item);
void teamplay_DropPromote (edict_t *ent);
void teamplay_LocateKey (edict_t *ent, qboolean isauto);
void teamplay_LocatePromoKey (edict_t *ent, qboolean isauto);
void teamplay_ResetSkin(edict_t *ent);
void teamplay_ScoreboardMessage (edict_t *ent, edict_t *killer);
void teamplay_SetStats (edict_t *ent);
void teamplay_ResetPlayer (edict_t *ent, qboolean override);
void teamplay_ClearMute (edict_t *ent);
void teamplay_CaptainMsg (int team, char *msg);
void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);
void teamplay_Effects(edict_t *player);
void teamplay_ResetImpeach(int team);
void teamplay_FragBoard (edict_t *ent, edict_t *killer);
void teamplay_CaptureBoard (edict_t *ent, edict_t *killer);
qboolean teamplay_AddAmmo (edict_t *ent, int ammotag);
int teamplay_KeyAction (edict_t *ent, edict_t *key);
char *teamplay_NewEntities(char *mapname, char *entities);
qboolean teamplay_HasLeader (int team);
qboolean teamplay_ClientThink (edict_t *ent, usercmd_t *ucmd);
qboolean teamplay_CheckTeamDamage (edict_t *targ, edict_t *attacker);
qboolean teamplay_Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean teamplay_MapCheck(edict_t *mapent, edict_t *player);
qboolean teamplay_IsOffense(edict_t *ent);

extern qboolean tp_keys_spawned;
extern qboolean newmapentities;

typedef struct
{
	int			team_score[3];
	int			team_frags[3];
	qboolean	hasleader[3];
	int			members[3];
	int			team_capsinarow[3];
	int			team_impeachvotes[3];
	int			team_promos[3];
	qboolean	hasprotect[3];
} team_t;

extern team_t	teams;

typedef struct
{
	edict_t		*spawnspot;
	edict_t		*keyent;
	edict_t		*capkey;
	edict_t		*nextrespawn;
	edict_t		*lastcapturer;
	qboolean	isdropped;
	qboolean	capkeydropped;
	qboolean	istaken;
	char		keyname[20];
	char		flagstart[20];
	char		flaggoal[20];
	char		playerstart[30];
	char		flagstart2[20];
	char		playerstart2[30];
	int			keyeffect;
} teamkey_t;

extern teamkey_t teamkeys[MAX_KEYS];

extern float	keychecktime;
extern float	fairwarning;
extern float	startgame;
extern int		mapmode;
extern int		goalmode;
extern int		lastcapteam;
extern float	lastcaptime;

extern gitem_t *flag1_item;
extern gitem_t *flag2_item;

extern edict_t	*intermission_spot;

extern float	newbots_gravity;