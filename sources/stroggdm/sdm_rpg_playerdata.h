void WRITE_PLAYER_STATS (edict_t *ent);
int READ_PLAYER_STATS (edict_t *ent); 
void PlayerData_Login (edict_t *ent, pmenuhnd_t *p);
void PlayerData_Create (edict_t *ent, pmenuhnd_t *p);
void Step3 (edict_t *ent, pmenuhnd_t *p);
void Step2 (edict_t *ent);
void OpenLoginMenu (edict_t *ent, pmenuhnd_t *p);
void SetFilename(char * filename,int len, edict_t *ent);