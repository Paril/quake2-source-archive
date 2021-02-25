
//gameplay
#define SOG_REDBLUE_TEAMS	1	//gen_team has to be on
#define SOG_USE_KEY		2	
#define SOG_FULLYLOADED       4
#define SOG_IDG3_ONLY		8	
#define SOG_IDG2_ONLY		16	
#define SOG_CTF_BASE_RESPAWN	32

// map selection
#define SOG_NO_MAP_VOTING     64
#define SOG_DM_ONLY		128 //maplist
#define SOG_CTF_ONLY		256 //maplist

#define SOG_IDG1_ONLY         512

// regular CTF options
#define DF_FORCEJOIN			131072	
#define DF_ARMOR_PROTECT		262144
#define DF_CTF_NO_TECH			524288

// Game Cvars			
extern cvar_t *sog_team;		
extern cvar_t *sog_ctf;
extern cvar_t *limit;
extern cvar_t *sogflags;

//DOOMGUYS SPEED
extern cvar_t *lan;

extern char *dm_statusbar;

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
	int team1, team2,		// Team/CTF		- Current Frags/Points Total
		team3, team4;		// Pickup		- Rounds Won		
							// Team - 
	int total1, total2,
		total3, total4;		// CTF		-	Total Captures
							// Pickup	-	Total Matches Won
	// Misc	CTF
	float last_flag_capture;	
	int last_capture_team;
}generations_game;

generations_game	gengame;

//Game functions
void GenInit(void);
void GenTryCTF(void);
void SOGCalculateTeamScores(void);
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
void GenTeamScoreboard (edict_t *ent, edict_t *killer);

//Class Changing
void Gen_Class_f(edict_t *ent);
char *GenClassName(int player_class);
void GenClassPicked(edict_t *ent, int desired_class);


//CTF Grapple addons
//Offhand Grapple
void Offhand_Grapple_Fire (edict_t *ent);
void Offhand_Grapple_Drop (edict_t *ent);

void stuffcmd(edict_t *ent, char *s);

// Chasecam Functions
extern void CheckChasecam_Viewent(edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void ChasecamStart (edict_t *ent);

//Skid
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void SendStatusBar(edict_t *ent, char *bar);
void OldDudesUseState(edict_t *ent);

