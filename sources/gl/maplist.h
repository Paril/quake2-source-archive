  // 
  // maplist.h 
  // 
  // 
  // 4/98 - L. Allan Campbell (Geist) 
  // 
  // DEFINES ////////////////////////////////////////////////// 
  #define MAX_MAPS           16 
  #define MAX_MAPNAME_LEN    16 
  #define ML_ROTATE_SEQ          0 
  #define ML_ROTATE_RANDOM       1 
  #define ML_ROTATE_NUM_CHOICES  2 
    
  // STRUCTURES /////////////////////////////////////////////// 
  typedef struct 
  { 
     char filename[21];     // filename on server (20-char max length) 
     int  nummaps;          // number of maps in list 
     char mapnames[MAX_MAPS][MAX_MAPNAME_LEN]; 
     char rotationflag;     // set to ML_ROTATE_* 
     int  currentmap;       // index to current map 
  } maplist_t; 
    
  // GLOBALS ////////////////////////////////////////////////// 
  maplist_t maplist; 
    
  // PROTOTYPES /////////////////////////////////////////////// 
  int  LoadMapList         (char *filename); 
  void ClearMapList        (); 
  void Cmd_Maplist_f       (edict_t *ent); 
  void Svcmd_Maplist_f     (); 
  void DisplayMaplistUsage (edict_t *ent); 
  void ShowCurrentMaplist  (edict_t *ent);
