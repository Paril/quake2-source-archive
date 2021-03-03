// 
// fileio.h 
// 

// PROTOTYPES /////////////////////////////////////////////// 

FILE *WFOpenFile(edict_t *ent, char *filename); 
void WFCloseFile(edict_t *ent, FILE *fp);
void ReadLine( FILE *fp, char *buffer, int maxlen);
