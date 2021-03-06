//*************************************************************************************
//*************************************************************************************
// File: kotscpp.h
// has defines for cpp only files
//*************************************************************************************
//*************************************************************************************

class  CUser;
struct edict_s;

//*************************************************************************************
//*************************************************************************************
// Defines:
//*************************************************************************************
//*************************************************************************************

#define STAT_KOTS_IDVIEW   18
#define STAT_KOTS_IDLEVEL  19
#define STAT_KOTS_LEVEL    20
#define STAT_KOTS_RANK     21
#define STAT_KOTS_ITEM     23
#define STAT_KOTS_IDHEALTH 24
#define STAT_KOTS_STREAK   25

//*************************************************************************************
//*************************************************************************************
// Functions: 
//*************************************************************************************
//*************************************************************************************

void KOTSSendAll( bool bSound, char *str );

void KOTSMessage ( const char *msg );

CUser *KOTSGetUser( struct edict_s *ent );

void KOTSHelp     ( CUser *user, int blevel   );
void KOTSPlayerDie( CUser *user );

int KOTSClientCanEnter( struct edict_s *ent );

void KOTSChangeScore( CUser *user, int amount, int type );

bool KOTSTeleport( struct edict_s *ent );

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


