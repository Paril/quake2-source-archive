// 
// maplist.h 
// 
// DEFINES ////////////////////////////////////////////////// 
// STRUCTURES /////////////////////////////////////////////// 
// GLOBALS ////////////////////////////////////////////////// 
// PROTOTYPES /////////////////////////////////////////////// 


#define MAX_MAPS           16 
#define MAX_MAPNAME_LEN    16 

#define ML_ROTATE_SEQ          0 
#define ML_ROTATE_RANDOM       1 
#define ML_ROTATE_NUM_CHOICES  2 

int  LoadMapList         (char *filename); 
void ClearMapList        (); 
void Cmd_Maplist_f       (edict_t *ent); 
void Svcmd_Maplist_f     (); 
void DisplayMaplistUsage (edict_t *ent); 
void ShowCurrentMaplist  (edict_t *ent);