
//gameplay
#define GEN_REDBLUE_TEAMS		1	//gen_team has to be on
#define GEN_RANDOM_CLASS		2	//Assigns random class
#define GEN_FULLYLOADED         4
#define GEN_Q1_ONLY				8	
#define GEN_DOOM_ONLY			16	
#define GEN_CTF_BASE_RESPAWN	32

// map selection
#define GEN_NO_MAP_VOTING       64
#define GEN_DM_ONLY				128 //maplist
#define GEN_CTF_ONLY			256 //maplist

// regular CTF options
#define DF_FORCEJOIN			131072	
#define DF_ARMOR_PROTECT		262144
#define DF_CTF_NO_TECH			524288

//Midi Timer
extern float midiendtime;

// Game Cvars
extern cvar_t *gen;
extern cvar_t *gen_sp;				
extern cvar_t *gen_team;		
extern cvar_t *gen_ctf;
extern cvar_t *limit;
extern cvar_t *gen_forcejoin;
extern cvar_t *genflags;

//DOOMGUYS SPEED
extern cvar_t *lan;

extern char *dm_statusbar;
extern char *wolfdm;
extern char *wolfsp;
extern char *doomdm;
extern char *doomsp;

//Player States
typedef enum
{
	START_GEN,
	PLAYING_GEN,
	WAITING_SPAWN
} Genstate_t;

//Player Classes
typedef enum 
{
	NO_CLASS,
	CLASS_WOLF,
	CLASS_DOOM,
	CLASS_Q1,
	CLASS_Q2
} Genclass_t;

typedef struct gen_score
{
	// Scores
	int team1, team2,		// CTF		-	Current Score
		team3, team4;		// Pickup	-	Rounds Won		
							// Team - Current Frags Total
	int total1, total2,
		total3, total4;		// CTF		-	Total Captures / only set when going into intermission!
							// Pickup	-	Total Matches Won
	// Misc	CTF
	float last_flag_capture;	
	int last_capture_team;
}generations_game;


//Game functions
void GenInit(void);
void GenTryCTF(void);
qboolean GenCheckRBCTFRules(void);
qboolean GenCheckTeamRules(void);

//Client Startup
qboolean GenStartClient(edict_t *ent);
void GenPutClientInGame(edict_t *ent);
edict_t *SelectGenSpawnPoint (edict_t *ent);

//Menus
void GenOpenJoinMenu(edict_t *ent);
void GenOpenJoinClassMenu(edict_t *ent);

//Misc
void GenAssignClass(gclient_t *who);
void GenAssignModel(edict_t *ent, const char *s); 
qboolean GenAssignSkin(edict_t *ent, const char *qs);

//Skid added
//Scoreboards
void GenRBScoreboardMessage (edict_t *ent, edict_t *killer);
void Q1CTFScoreboardMessage (edict_t *ent, edict_t *killer);
void Q1DMScoreboardMessage (edict_t *ent, edict_t *killer);
void DoomDMScoreboardMessage (edict_t *ent, edict_t *killer);
void WolfDMScoreboardMessage (edict_t *ent, edict_t *killer);
void GenTeamScoreboard (edict_t *ent, edict_t *killer);
void Q1IntermissionScreen(edict_t *ent);
//

//Class Changing
void Gen_Class_f(edict_t *ent);
char *GenClassName(int player_class);
void GenClassPicked(edict_t *ent, int desired_class);


//CTF Grapple addons
//Offhand Grapple
void Offhand_Grapple_Fire (edict_t *ent);
void Offhand_Grapple_Drop (edict_t *ent);

//Quake1 Grapple
void Q1GrapplePull(edict_t *self);
void Q1GrappleReset(edict_t *self);

void stuffcmd(edict_t *ent, char *s);

// Chasecam Functions
extern void CheckChasecam_Viewent(edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void ChasecamStart (edict_t *ent);

//Skid
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void SendStatusBar(edict_t *ent, char *bar);
void OldDudesUseState(edict_t *ent);

