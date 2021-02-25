//============================================================================
// flashlight.h
//
// Forward declarations for methods in flashlight.c
//
// Originally code by majoon.
// Declarations moved here by DingBat.
//
//============================================================================

//majoon: The q_devels.h
#include "q_devels.h"
//#include "chasecam.h"

void FlashlightThink (edict_t *self);
void SP_Flashlight (edict_t *self);
void ClearFlashlight (edict_t *self);
void initPredator (edict_t *ent);
void LookforPredator (edict_t *ent);
void onPlayerConnect (edict_t *ent);

void EndDMLevel (void);
void InfoComputer (edict_t *ent);
void Cmd_ShowInfo_f (edict_t *ent);

int nextdynamicset;

void KickRadiusDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int knockback, int dflags, int mod);

// Added by DingBat
void StartPredator(edict_t *ent) ;
