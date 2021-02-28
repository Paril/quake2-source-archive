#define	ALLY_WAIT_TIMEOUT	30 // number of seconds before a request times out

qboolean IsAlly (edict_t *ent, edict_t *other);
void AddAlly (edict_t *ent, edict_t *other);
void RemoveAlly (edict_t *ent, edict_t *other);
void AllyCleanup (edict_t *ent);
void RemoveAllies (edict_t *ent);
void AllyValidate (edict_t *ent);
int AddAllyExp (edict_t *ent, int exp);
int numAllies (edict_t *ent);
void ShowAllyMenu (edict_t *ent);
void AbortAllyWait (edict_t *ent);
void ShowAllyWaitMenu_handler (edict_t *ent, int option);
void AllyID (edict_t *ent);
qboolean CanAlly (edict_t *ent, edict_t *other, int range);
qboolean ValidAlly (edict_t *ent);
void NotifyAllies (edict_t *ent, int msgtype, char *s);
void InitializeTeamNumbers (void);
void ResetAlliances (edict_t *ent);//4.5 - reset alliances when changing combat preferences