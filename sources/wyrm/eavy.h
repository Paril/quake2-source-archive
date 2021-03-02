/* Copyright © EAVY */
char *EAVYLoadEntities(char *mapname, char *entities);
char *EAVYReadTextFile(char *filename);
void EAVYSpawnFlags(void);
edict_t *EAVYFindFarthestFlagPosition(edict_t *flag);
void EAVYSetupFlagSpots(void);
void EAVYSpawnTeamNearFlag(edict_t *flag);
void EAVYSpawnTeamNearFlagCheck(void);
void SV_Lights_f (qboolean cmd);
void ED_CallSpawn (edict_t *ent);
