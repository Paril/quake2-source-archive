#define BBALL_VERSION			1.00
#define BBALL_VSTRING2(x) #x
#define BBALL_VSTRING(x) BBALL_VSTRING2(x)
#define BBALL_STRING_VERSION  BBALL_VSTRING(BBALL_VERSION)

typedef enum {
	NOTEAM,
	TEAM1,
	TEAM2
} team_t;

typedef enum {
	STATE_START,
	STATE_PLAYING
} state_t;


#define TEAM1_SKIN "red"
#define TEAM2_SKIN "blue"

void BBallScoreboardMessage (edict_t *ent, edict_t *killer);
void BballEffects(edict_t *ent);
void Team_f (edict_t *ent);
void OpenJoinMenu(edict_t *ent);
void AssignSkin(edict_t *ent, char *s);
void AssignTeam(gclient_t *who);
qboolean StartClient(edict_t *ent);
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
