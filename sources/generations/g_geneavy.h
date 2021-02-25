

char *ReadTextFile(char *filename);
char *EAVYLoadEntities(char *mapname, char *entities);
void EAVYCTF_Init(void);
edict_t * EAVYFindFarthestFlagPosition(edict_t * flag);
void EAVYSpawnFlags(void);
void EAVYSpawnTeamNearFlagCheck(void);
void EAVYSpawnTeamNearFlag(edict_t * flag);
void EAVYSetupFlagSpots(void);
