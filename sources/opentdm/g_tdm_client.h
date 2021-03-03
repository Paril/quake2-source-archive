/*===============
Client Stuff
===============*/
edict_t *TDM_FindPlayerForTeam (unsigned team);
int TDM_GetPlayerIdView (edict_t *ent);
void TDM_SendStatusBarCS (edict_t *ent);
void TDM_DownloadPlayerConfig (edict_t *ent);

#define SPAWN_RANDOM_ON_SMALL_MAPS	4
