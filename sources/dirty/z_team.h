#define TEAM_INDEX(x) ((x)-teamgame.teamlist)

typedef struct team_handling_s
{
        teams_t teamlist[10];
        char    message[512];
        float   last_score;
        teams_t *last_score_team;
        int     flag_items;
        char    goal_description[10][1024]; // Paranoid
} team_handling_t;

extern qboolean first_team; // For giving more random DM spots at start of match
extern team_handling_t teamgame;
extern teams_t teamlist[10];

void TeamInit(void);

void MakeObserver(edict_t *ent);

void Open_Join_Team_Menu(edict_t *ent);
teams_t *GetTeamByIndex (int index);
int GetTeamBySize (qboolean want_most, qboolean ignore_zero, qboolean ppl_ready);
void ChosenTeam(edict_t *ent, pmenu_t *p);
teams_t *FindTeam (char *name);
void AssignSkin(edict_t *ent, char *s);
void AssignDisc(edict_t *ent);
char *FlagClassname(teams_t *team);

char *TeamSpawnPoint(teams_t *team);
edict_t *SelectTeamSpawnPoint (edict_t *ent);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);
void SP_info_player_team3(edict_t *self);
void SP_info_player_team4(edict_t *self);
void SP_info_player_team5(edict_t *self);
void SP_info_player_team6(edict_t *self);
void SP_info_player_team7(edict_t *self);
void SP_info_player_team8(edict_t *self);

#define TEAM1_GOAL              1
#define TEAM2_GOAL              2
#define TEAM3_GOAL              4
#define TEAM4_GOAL              8
#define TEAM5_GOAL              16
#define TEAM6_GOAL              32
#define TEAM7_GOAL              64
#define TEAM8_GOAL              128

void TeamInit(void);
teams_t *GetGlobalTeamByIndex (int index);
void GiveTeamASpot(teams_t *team);
void ForceAssignTeam(gclient_t *who);
int TeamGoal(teams_t *team);
void Cmd_Team_f (edict_t *ent);
void JoinTeam (edict_t *ent, teams_t *team);
void SP_info_team (edict_t *ent);
