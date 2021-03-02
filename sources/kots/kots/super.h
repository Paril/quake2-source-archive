//*************************************************************************************
//*************************************************************************************
// File: super.h
// All levels of super powers can be changed here
//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//*************************************************************************************
// Functions: Weapons
//*************************************************************************************
//*************************************************************************************

int KOTSSpeed  ( edict_t *ent, int mod );
int KOTSHaste  ( edict_t *ent );
int KOTSDamage ( edict_t *ent, int mod );	
int KOTSSilent ( edict_t *ent, int mod );
int KOTSSpecial( edict_t *ent, int mod, int type );

//*************************************************************************************
//*************************************************************************************
// Function: Players
//*************************************************************************************
//*************************************************************************************

int KOTSCloak( edict_t *ent, usercmd_t *ucmd );

void KOTSStopCloak( edict_t *ent );

void KOTSRegen     ( edict_t *ent );
int  KOTSSwitch    ( edict_t *ent );
int  KOTSModKarma  ( edict_t *ent );
int	 KOTSResist    ( edict_t *targ    , int take );
int  KOTSFly       ( edict_t *ent );
void KOTSKnock     ( edict_t *attacker, edict_t *targ, int take );
void KOTSVampire   ( edict_t *attacker, edict_t *targ, int take );
void KOTSMakeMega  ( edict_t *ent );
int  KOTSHighJump  ( edict_t *ent );
int  KOTSStrength  ( edict_t *attacker, int take );
int  KOTSModDamage ( edict_t *attacker, int damage, int mod );
int  KOTSSilentJump( edict_t *ent );
int  KOTSSilentWalk( edict_t *ent );

int  KOTSSilentPickup   ( edict_t *ent );
int  KOTSArmorProtection( edict_t *ent, int *damage );

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

