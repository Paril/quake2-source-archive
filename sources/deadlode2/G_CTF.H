extern cvar_t *ctf;
extern char *ctf_statusbar;

void CTFInit(void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);
void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);
void SP_trigger_teleport (edict_t *ent);
void SP_info_teleport_destination (edict_t *ent);

void CTFCalcScores(void);
void CTFScoreboardMessage (edict_t *ent, edict_t *killer);

qboolean CTFCheckRules(void);

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);

