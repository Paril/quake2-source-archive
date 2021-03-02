//
// compmod.h
//

// Defines
#define CM_FFA				0
#define CM_MATCHRALLY		1
#define CM_MATCHINPROGRESS	2
#define CM_MATCHCOUNTDOWN	3

#define CM_POWERUPS_ON	0
#define CM_QUAD_OFF		1
#define CM_INVULN_OFF	2
#define CM_POWERUPS_OFF	3

#define CM_SHUTUP_OFF	0
#define CM_SHUTUP_SPECS	1
#define CM_SHUTUP_ALL	2

#define CM_DISABLE		0
#define CM_ENABLE		1
#define CM_DIS_NONE		0
#define CM_DIS_QUAD		1
#define CM_DIS_INV		2
#define CM_DIS_POWERUPS	3
#define CM_DIS_BFG		4
#define CM_DIS_RAIL		8
#define CM_DIS_HYPER	16
#define CM_DIS_RL		32
#define CM_DIS_GL		64
#define CM_DIS_GREN		128
#define CM_DIS_CHAIN	256
#define CM_DIS_MG		512
#define CM_DIS_SSG		1024
#define CM_DIS_SG		2048
#define CM_DIS_ENERGY	20
#define CM_DIS_EXPLOS	224
#define CM_DIS_BULLETS	768
#define CM_DIS_SHELLS	3072
#define CM_DIS_WEAPS	4092
#define CM_DIS_ALL		4095

#define CM_TEAM_SPECTATOR	-1
#define CM_TEAM_NONE		0

#define CM_LOCKDOWN_OFF	0
#define CM_LOCKDOWN_ON	1

#define CM_LOCALCOMMAND	11

#define CM_ADMINFLAG_NOT		0
#define CM_ADMINFLAG_ELECTED	1
#define CM_ADMINFLAG_CODED		2

#define CM_NOTREADY	0
#define CM_READY	1

#define CM_VOTE_NO	0
#define CM_VOTE_YES	1

#define CM_VOTETYPE_NONE		0
#define CM_VOTETYPE_WARP		1
#define CM_VOTETYPE_ADMIN		2
#define CM_VOTETYPE_ENDMATCH	3

#define CM_VOTED_NOT	0
#define CM_VOTED_NO		1
#define CM_VOTED_YES	2

#define CM_VOTETHINKTIME	15

#define ATAP_CLEARREADY			0
#define ATAP_GIB				1
#define ATAP_CLEARSCORES		2
#define ATAP_DUMPSCORES			3
#define ATAP_BROADCASTWINNER	4
#define ATAP_CLEARVOTESTATUS	5

#define ATAC_CLEARSCORES		0
#define ATAC_DESTROYZEROCLANS	1
#define ATAC_KILLUSEDCLANS		2

#define ATAA_REPORTREFUSEDCONNECT 0

#define MS_MATCHUP		0
#define MS_MATCHDOWN	1
#define MS_MATCHSET		2

#define MS_MATCHFRAGUP		0
#define MS_MATCHFRAGDOWN	1
#define MS_MATCHFRAGSET		2

#define PC_ALLCLIENTS		0
#define PC_PLAYERSONLY		1
#define PC_SPECTATORSONLY	2

#define MOD_MATCHSTARTEND	34

// Structures
typedef struct
{
	int		powerups;
	int		serverstatus;
	int		shutup;
	int		lockdown;
	int		matchtime;
	int		matchfrags;
	int		rallytime;
	char	nextmap[256];
} compmod_t;

typedef struct
{
	char	teamname[13];
} team_t;


// Globals
compmod_t	compmod;
team_t		team[17];	// entry 0 is NULL and means team not selected
						// -1 is not actually stored and means observer

// Prototypes
void Cmd_Admin_f (edict_t *ent);
void Cmd_BecomePlayer_f (edict_t *ent);
void Cmd_BecomeSpectator_f (edict_t *ent);
void Cmd_ClanList_f (edict_t *ent, int iListingType);
void Cmd_Contact_f (edict_t *ent);
void Cmd_Disable_f (edict_t *ent);
void Cmd_DisplayModMode_f (edict_t *ent);
void DisplayMOTD (edict_t *ent);
void Cmd_DropQuad_f (edict_t *ent);
void Cmd_Elect_f (edict_t *ent);
void Cmd_Endmatch_f (edict_t *ent);
void Cmd_FriendlyFire_f (edict_t *ent);
void Cmd_Lockdown_f (edict_t *ent);
void Cmd_MatchFragSet_f (edict_t *ent);
void Cmd_MatchTimeSet_f (edict_t *ent);
void Cmd_ModeSet_f (edict_t *ent);
void Cmd_Normal_f (edict_t *ent);
void Cmd_NotReady_f (edict_t *ent);
void Cmd_PlayerList_f (edict_t *ent);
void Cmd_Powerups_f (edict_t *ent);
void Cmd_Ready_f (edict_t *ent);
void Cmd_Restart_f (edict_t *ent);
void Cmd_ShutUp_f (edict_t *ent);
void Cmd_Team_f (edict_t *ent);
void Cmd_TimerSet_f (edict_t *ent);
void Cmd_Vote_f (edict_t *ent, int iVoteType);
void Cmd_Warp_f (edict_t *ent);
//
void Cmd_Commands_f (edict_t *ent);
void Cmd_Credits_f (edict_t *ent);
//
void StuffCommand (edict_t *e, char *s);
int MatchInProgress (edict_t *ent);
int NotAnAdmin (edict_t *ent);
edict_t *MakeClan (int iClanNumber);
edict_t *FindClan (int iClanNumber);
void SpawnTimer (void);
edict_t *FindTimer (void);
void TimerThink (edict_t *ent);
int AllReady (void);
void ApplyToAllPlayers (int iApplyCode, int iTeamNumber, int iFrags);
void RestartServer (void);
void UpdatePlayerStats (edict_t *player, int iFrags, int iEnemyKills, int iFriendlyKills, int iDeaths);
void DumpPlayerScore (edict_t *ent);
void DumpClanScore (edict_t *ent);
void MakeClansUsed (void);
void ApplyToAllClans (int iApplyCode);
void DeclareWinner (void);
int AssignTeam (char *szTeamName);
void ApplyToAllAdmins (int iApplyCode);
void EndMatch (void);
int PlayerCount (int iMode);
int TestPlayerCount (void);
edict_t *FindVote (void);
void VoteThink (edict_t *ent);
void EvaluateVote (edict_t *vote);
void KillVote (edict_t *vote);
int CountClans (void);
void DisableFlagSet (int iEnableDisable, int iDisableFlags);
int IsItDisabled (edict_t *ent);
