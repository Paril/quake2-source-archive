void Cmd_Observe(edict_t *player);
void ToggleChaseCam(edict_t *player);
void ChaseNext(edict_t *player);
qboolean IsObserver(edict_t *ent);
void spectatorStateChange(edict_t *ent);
void PlayerToObserver(edict_t *ent);
void ObserverToPlayer(edict_t *ent);
void ObserverToTeam(edict_t *ent, int teamToAssign);
void markNotObserver(edict_t *ent);
