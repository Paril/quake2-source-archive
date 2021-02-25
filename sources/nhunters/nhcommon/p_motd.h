//===========================================================================
// p_motd.h
//
// MOTD functions.
//
// Originally Coded by: BatMax and DingBat 
//
//===========================================================================

#define MOTD_TIME_DEFAULT "15"

void ShowMOTD(edict_t *ent) ;
qboolean ShowingMOTD(edict_t *ent) ;
void ClearMOTD(edict_t *ent) ;

void validateMotdTime(); 
int getMotdTime();
