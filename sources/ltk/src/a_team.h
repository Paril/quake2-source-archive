/*
 * Include for Action team-related things
 */

#define NOTEAM          0
#define TEAM1           1
#define TEAM2           2

#define MAX_TEAMS       2
#define TEAM_TOP        (MAX_TEAMS+1)

#define WINNER_NONE     0
#define WINNER_TEAM1    1
#define WINNER_TEAM2    2
#define WINNER_TIE      3

// Pre- and post-trace code for our teamplay anti-stick stuff.  If there are
// still "transparent" (SOLID_TRIGGER) players, they need to be set to
// SOLID_BBOX before a trace is performed, then changed back again
// afterwards.  PRETRACE() and POSTTRACE() should be called before and after
// traces in all places where combat is taking place (ie "transparent" players
// should be detected), ie shots being traced etc.  
// FB 6/1/99: Now crouching players will have their bounding box adjusted here
// too, for better shot areas. (there has to be a better way to do this?)

#define PRETRACE() \
        if (transparent_list && (int)teamplay->value && !lights_camera_action) \
                TransparentListSet(SOLID_BBOX)

#define POSTTRACE() \
        if (transparent_list && (int)teamplay->value && !lights_camera_action) \
                TransparentListSet(SOLID_TRIGGER)

edict_t *SelectTeamplaySpawnPoint(edict_t *); 
qboolean FallingDamageAmnesty(edict_t *targ);
void OpenJoinMenu(edict_t *);
void OpenWeaponMenu(edict_t *);
void OpenItemMenu(edict_t *ent);
void JoinTeam(edict_t *ent, int desired_team, int skip_menuclose);
edict_t *FindOverlap(edict_t *ent, edict_t *last_overlap);
void CheckTeamRules(void);
void A_Scoreboard(edict_t *ent);
void Team_f(edict_t *ent);
qboolean StartClient(edict_t *ent);
void AssignSkin(edict_t *, char *);
void TallyEndOfLevelTeamScores(void);
void CheckForUnevenTeams(void);
void SetupTeamSpawnPoints();
void GetSpawnPoints();
void CleanBodies(); // from p_client.c, removes all current dead bodies from map
void LeaveTeam(edict_t *);
int newrand(int top);
void InitTransparentList();
void AddToTransparentList(edict_t *);
void RemoveFromTransparentList(edict_t *);
void PrintTransparentList();

typedef struct spawn_distances_s
{
        float distance;
        edict_t *s;
} spawn_distances_t;

typedef struct transparent_list_s
{
        edict_t *ent;
        struct transparent_list_s *next;
} transparent_list_t;


extern qboolean team_game_going;
extern qboolean team_round_going;
extern int team1_score;
extern int team2_score;
extern int team1_total;
extern int team2_total;
extern int lights_camera_action;
extern int holding_on_tie_check;
extern int team_round_countdown;
extern transparent_list_t *transparent_list;
extern trace_t trace_t_temp;
