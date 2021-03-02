// FIXME: Change because this is an Expert version?
#define CTF_VERSION			1.02
#define CTF_VSTRING2(x) #x
#define CTF_VSTRING(x) CTF_VSTRING2(x)
#define CTF_STRING_VERSION  CTF_VSTRING(CTF_VERSION)

// for highlighting messages
#define GREENTEXT_ASSIST "ассист"
#define GREENTEXT_CAPTURED "цаптуред"
#define GREENTEXT_RETURNED "ретурнед"
#define GREENTEXT_LOST "лост"
#define GREENTEXT_GRABBED "граббед"

#define STAT_CTF_TEAM1_PIC			17
#define STAT_CTF_TEAM1_CAPS			18
#define STAT_CTF_TEAM2_PIC			19
#define STAT_CTF_TEAM2_CAPS			20
#define STAT_CTF_FLAG_PIC			21
#define STAT_CTF_JOINED_TEAM1_PIC	22
#define STAT_CTF_JOINED_TEAM2_PIC	23
#define STAT_CTF_TEAM1_HEADER		24
#define STAT_CTF_TEAM2_HEADER		25
#define STAT_CTF_TECH				26

// 27 is player ID - in q_shared.h

// Expert: Flag carrier name strings
#define STAT_CTF_CARRIER1			28
#define STAT_CTF_CARRIER2			29

// Expert: Team Distribution
#define STAT_HOME_BASE				30
#define STAT_NEAR_HOME				31
#define STAT_NEAR_ENEMY				26 // replaces tech
#define STAT_ENEMY_BASE				16 // previously unused

#define STAT_TD_TOP_ICON			7  // replaces health icon 
#define STAT_TD_BOTTOM_ICON			8  // replaces pickup string

typedef enum {
	CTF_STATE_START,
	CTF_STATE_PLAYING
} ctfstate_t;

// minimum range from enemy flag a player is allowed to spawn
#define CTF_SPAWN_RANGE					1500
// expressed as a ratio of the distance between flags
#define CTF_SPAWN_RANGE_RATIO			(1.0/3)

// Note that based on the method of awarding bonuses,
// IMMEDIATE_CARRIER_KILL should be at least as much as FLAG_DEFENSE
// CARRIER_SAVE should be at least as much as CARRIER_DEFENSE

#define CTF_CAPTURE_BONUS				9	// capture bonus
#define CTF_RECOVERY_BONUS				1	// flag return bonus
#define CTF_FLAG_BONUS					0	// enemy flag pickup bonus
#define CTF_CARRIER_KILL_BONUS_RATE		0.3	// amount of points killing a carrier is
											// worth per second that the carrier has 
											// lived
#define CTF_CARRIER_KILL_BONUS_MAX		12	// maximum carrier kill bonus
#define CTF_FLAG_RETURN_TIME			30	// seconds until auto return

// bonus for fraggin an enemy who has 
// recently hurt your flag carrier
#define CTF_CARRIER_DANGER_PROTECT_BONUS	3	

// bonus for fraggin an enemy while either 
// you or your target are near your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			2	

// bonus for carriers only given in addition to
// a base defense bonus if the carrier's flag is
// not at base
#define CTF_FLAG_HOLDING_BONUS				1	

// bonus for fraggin an enemy while either you 
// or your target are near your flag, and
// near your base 
#define CTF_BASE_DEFENSE_BONUS				1	

// bonus for fraggin an enemy while either you 
// or your target are near your flag, and not
// near your base
#define CTF_FLAG_DEFENSE_BONUS				2	

// bonus for fraggin an enemy while either you
// or your target are near the enemy's flag
#define CTF_DEFENDER_KILL_BONUS				1	

// awarded for returning a flag that causes 
// a capture to happen almost immediately
#define CTF_RETURN_FLAG_ASSIST_BONUS		1	

// award for fragging a flag carrier 
// if a capture happens almost immediately
#define CTF_CARRIER_KILL_ASSIST_BONUS		2	

// the radius around an object being defended 
// where a target will be worth extra frags
#define CTF_TARGET_PROTECT_RADIUS			600

// the radius around an object being defended where 
// an attacker will get extra frags when making kills
#define CTF_ATTACKER_PROTECT_RADIUS			600

// timeouts
#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		13	
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10	
#define CTF_IMMEDIATE_CARRIER_KILL_TIMEOUT	4

#define CTF_AUTO_FLAG_RETURN_TIMEOUT		30 // number of seconds before dropped flag auto-returns

void CTFInit(void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);

char *CTFTeamName(int team);
char *CTFOtherTeamName(int team);
void CTFAssignSkin(edict_t *ent, char *s);
void CTFAssignTeam(gclient_t *who);
edict_t *SelectCTFSpawnPoint (edict_t *ent);
qboolean CTFTouchFlag(edict_t *ent, edict_t *other);
qboolean CTFDrop_Flag(edict_t *ent, gitem_t *item);
void CTFEffects(edict_t *player);
void CTFCalcScores(void);
void SetCTFStats(edict_t *ent);
void CTFDeadDropFlag(edict_t *self);
void CTFScoreboardMessage (edict_t *ent, edict_t *killer);
void CTFTeam_f (edict_t *ent);
void CTFID_f (edict_t *ent);
void CTFSay_Team(edict_t *who, char *msg);
void CTFFlagSetup (edict_t *ent);
void CTFResetFlag(int ctf_team);
void ExpertCTFScoring(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker);

void CTFOpenJoinMenu(edict_t *ent);
qboolean CTFStartClient(edict_t *ent);

qboolean CTFCheckRules(void);

void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);

extern char *ctf_statusbar;

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);

void SP_trigger_teleport (edict_t *ent);
void SP_info_teleport_destination (edict_t *ent);

int CarrierName(int teamnum);
