
#define TEAMDM_VERSION			0.80
#define TEAMDM_VSTRING2(x) #x
#define TEAMDM_VSTRING(x) TEAMDM_VSTRING2(x)
#define TEAMDM_STRING_VERSION  TEAMDM_VSTRING(TEAMDM_VERSION)

#define STAT_TEAMDM_TEAM1_PIC			19
#define STAT_TEAMDM_TEAM1_CAPS			20
#define STAT_TEAMDM_TEAM2_PIC			21
#define STAT_TEAMDM_TEAM2_CAPS			22
#define STAT_TEAMDM_JOINED_TEAM1_PIC	24
#define STAT_TEAMDM_JOINED_TEAM2_PIC	25
#define STAT_TEAMDM_TEAM1_HEADER		26
#define STAT_TEAMDM_TEAM2_HEADER		27
#define STAT_TEAMDM_ID_VIEW			29

typedef enum {
	TEAMDM_NOTEAM,
	TEAMDM_TEAM1,
	TEAMDM_TEAM2
} teamdmteam_t;

typedef enum {
	TEAMDM_STATE_START,
	TEAMDM_STATE_PLAYING
} teamdmstate_t;

extern cvar_t *teamdm;

#define TEAMDM_TEAM1_SKIN "teamred"
#define TEAMDM_TEAM2_SKIN "teamblue"

#define DF_TEAMDM_FORCEJOIN	131072	

void TeamDMInit(void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);

char *TEAMDMTeamName(int team);
char *TEAMDMOtherTeamName(int team);
void TEAMDMAssignSkin(edict_t *ent, char *s);
void TEAMDMAssignTeam(gclient_t *who);
void TEAMDMAssignBotTeam(gclient_t *who);
edict_t *SelectTEAMDMSpawnPoint (edict_t *ent);
void TEAMDMCalcScores(void);
void SetTEAMDMStats(edict_t *ent);
void TEAMDMScoreboardMessage (edict_t *ent, edict_t *killer);
void TEAMDMTeam_f (edict_t *ent);
void TEAMDMID_f (edict_t *ent);
void TEAMDMSay_Team(edict_t *who, char *msg);

void TEAMDMOpenJoinMenu(edict_t *ent);
qboolean TEAMDMStartClient(edict_t *ent);

qboolean TEAMDMCheckRules(void);

extern char *teamdm_statusbar;

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);