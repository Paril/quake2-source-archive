//*************************************************************************************
//*************************************************************************************
// File: exports.h
//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//*************************************************************************************
// Defines: 
//*************************************************************************************
//*************************************************************************************

#define KOTS_SELFKILL 0
#define KOTS_NORMAL   1

#define KOTS_VERSION "5.3"

#define PACK_PACK 1
#define PACK_BAND 2
#define PACK_ADRE 3

//*************************************************************************************
//*************************************************************************************
// Functions: Server
//*************************************************************************************
//*************************************************************************************

void KOTSEnd    ();
void KOTSTime   ( edict_t *ent );
void KOTSInit   ( edict_t *ent );
void KOTSEnter  ( edict_t *ent );
void KOTSStartup();

int KOTSLeave  ( edict_t *ent );
int KOTSScoring( edict_t *self, edict_t *targ, int type );

void KOTSSVCmd_Status_f();

int  KOTSGetClientData( edict_t *ent, char *userinfo );

void KOTSSpawnKick ( edict_t *attacker );
char *KOTSAssignSkin( edict_t *ent );
void KOTSDeathSound( edict_t *ent );
void KOTSWorldSpawn();

void  KOTSSaveDamage ( edict_t *attacker, edict_t *targ, int take, int high );
float KOTSPowerDamage( edict_t *ent, float damagepercell );

//*************************************************************************************
//*************************************************************************************
// Function: KOTS Cmds
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Fly     ( edict_t *ent );
void KOTSCmd_Kick    ( edict_t *ent );
void KOTSCmd_Help    ( edict_t *ent );
void KOTSCmd_Team    ( edict_t *ent );
void KOTSCmd_Hook    ( edict_t *ent );
void KOTSCmd_Laser   ( edict_t *ent );
void KOTSCmd_Cloak   ( edict_t *ent );
void KOTSCmd_Flash   ( edict_t *ent );
void KOTSCmd_TBall   ( edict_t *ent );
void KOTSCmd_EXPack  ( edict_t *ent );
void KOTSCmd_Helmet  ( edict_t *ent );
void KOTSCmd_NoTBall ( edict_t *ent );
void KOTSCmd_BFGBall ( edict_t *ent );
void KOTSCmd_MakeMega( edict_t *ent );

void KOTSCmd_ChangeTeams( edict_t *ent );

void KOTSCmd_Boomerang( edict_t *ent );
void KOTSCmd_StopTBall( edict_t *ent );

int  KOTSPickup_KOTSExPack( edict_t *ent, edict_t *other );

//*************************************************************************************
//*************************************************************************************
// Function: HUD MENU
//*************************************************************************************
//*************************************************************************************

void KOTSShowHUD   ( edict_t *ent );
void KOTSCmd_Vote_f( edict_t *ent );
void KOTSCmd_Info_f( edict_t *ent );

void KOTSOpenJoinMenu     ( edict_t *ent );
void KOTSStatScoreboard   ( edict_t *ent );
void KOTSScoreboardMessage( edict_t *ent, edict_t *killer, int bEnd );

//*************************************************************************************
//*************************************************************************************
// Function: KOTSItems
//*************************************************************************************
//*************************************************************************************

void KOTSPickup_Pack    ( edict_t *ent, int type );
int  KOTSPickup_Armor   ( edict_t *ent, edict_t *other );
int  KOTSPickup_KOTSPack( edict_t *ent, edict_t *other );

int  KOTSPickup_PowerArmor( edict_t *ent, edict_t *other );

//*************************************************************************************
//*************************************************************************************
// Function: TBalls
//*************************************************************************************
//*************************************************************************************

void KOTS_Use_T_Ball( edict_t *ent, gitem_t *item );

void KOTSRadiusTeleport( edict_t *ent );

//*************************************************************************************
//*************************************************************************************
// Function: KOTS Combat
//*************************************************************************************
//*************************************************************************************

int KOTSHeadShot( edict_t *attacker, edict_t *targ, vec3_t dir, vec3_t point, int damage, 
                  int mod );

int KOTSRunes( edict_t *targ, edict_t *attacker, int damage );

int KOTSWeirdRules( edict_t *targ, edict_t *attacker, int damage );

void KOTSHits( edict_t *attacker, edict_t *inflictor, edict_t *targ, int take, int mod, 
               int damage );

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


