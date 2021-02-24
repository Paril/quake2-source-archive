#define GOAL_GROUP      -2 // Does what its group does...leave it alone
#define GOAL_DISABLED   -1
#define GOAL_OFF        1
#define GOAL_ON         2

#define CRITS_FAIL      -1
#define CRITS_NA        0
#define CRITS_PASS      1

void SP_target_endgame (edict_t *ent);
void SP_kill_goal (edict_t *ent);
void SP_target_goal (edict_t *ent);
void SP_trigger_goal (edict_t *ent);
void SP_item_goal (edict_t *ent);

// goal_type's for non-item_goal's
#define G_KILL                  1
#define G_ENDGAME               2
#define G_ITEM                  4
#define G_TOUCH                 8
#define G_SOLID                 16
#define G_DROP_TO_FLOOR         32
#define G_FLOATING              64
#define G_ITEM_ROD              128
#define G_ITEM_RETAIN           256
#define G_ITEM_IS_BIG           512
#define G_ITEM_INHAND           1024
#define G_ITEM_DWD              2048
#define G_ROF                   4096
#define G_DOF                   8192

#define GOAL_DROPPED_TIMEOUT 60

qboolean IsOurTeam (edict_t *ent, int teamindex);
void g_droptofloor (edict_t *ent);
qboolean GameOn (void);
qboolean TheyCan (edict_t *ent, edict_t *activator);
qboolean CheckGroupState (char *gname, int state);
edict_t *FindEntBy (char *cname, edict_t *owner, gitem_t *item, gitem_t *item2);
void Set_Forteam_n_Ally (edict_t *ent);
void FireGroup (edict_t *ent);
void FireGoalTargets (edict_t *ent);
void DoGoalStuff (edict_t *ent, int newstate);
void KillGoalCheck (edict_t *ent);
int goalCheckCrits (edict_t *ent, edict_t *other);
void goalFailed (edict_t *ent, edict_t *other);
void generic_goal (edict_t *ent);
void SP_misc_teleporter_dest (edict_t *ent);
void drop_goalitem (edict_t *ent, gitem_t *item);
void GoalAwardPoints (edict_t *ent);
void GoalCommunicates (edict_t *ent);


