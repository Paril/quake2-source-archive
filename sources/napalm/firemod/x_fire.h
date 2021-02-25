/*==========================================================================
//  x_fire.h -- by Patrick Martin               Last updated:  2-27-1999
//--------------------------------------------------------------------------
//  This is the header file for the x_fire*.c files.
//
//  The Napalm2 code uses my custom flame model.  It is similar to
//  (but NOT the same as) the flame2.mdl in Quake 1.
//========================================================================*/

#include "x_firemd.h"


/***************/
/*  CONSTANTS  */
/***************/

/* Time in seconds the entity on fire will burn. */
#define BURN_TIME       30
#define BURN_DM_TIME    20

/* Default radius for fire splash damage. */
#define RADIUS_FIRE_SPLASH     80

/* Default dropped flame duration. */
#define FLAME_TIME              2
#define FLAME_EXTRA_TIME        3

/* Default firestorm attributes. */
#define FIRESTORM_DELAY         0.7
#define FIRESTORM_DURATION      4
#define FIRESTORM_RADIUS       64

/* Maximum number of firestorms a player can have at a time. */
#define MAX_FIRESTORMS          2

/* Fireflags.
        SHIFT_Z      =  Vertical flame shift.
        DOWN         =  Shift flame down, not up.
        DELTA_ALIVE  =  Adjust only if target is alive.
        DELTA_BASE   =  Base adjustments (+4 if alive, -18 if dead).
        DELTA_VIEW   =  Adjust w/ viewheight (clients only).
        IGNITE       =  Target can ignite.
*/
#define FIREFLAG_SHIFT_Z        0x000000ff
#define FIREFLAG_DOWN           0x00000100
#define FIREFLAG_DELTA_ALIVE    0x00000200
#define FIREFLAG_DELTA_BASE     0x00000400
#define FIREFLAG_DELTA_VIEW     0x00000800
#define FIREFLAG_IGNITE         0x00001000


/****************/
/*  PROTOTYPES  */
/****************/

/*
// g_combat.c
*/
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);


/*
// x_fire1.c
// Core burn code
*/
void     PBM_CacheFire (void);
void     PBM_KillAllFires (void);
qboolean PBM_NeedSpeed (void);

void     PBM_MakeSmoke (vec3_t spot);
void     PBM_BecomeSmoke (edict_t *self);
void     PBM_BecomeSteam (edict_t *self);
void     PBM_FastSmallExplodeThink (edict_t *self);
void     PBM_SmallExplodeThink (edict_t *self);
void     PBM_StartSmallExplosion (edict_t *self);
void     PBM_BecomeSmallExplosion (edict_t *self);

qboolean PBM_InWater (edict_t *ent);
qboolean PBM_Inflammable (edict_t *ent);
qboolean PBM_ActivePowerArmor (edict_t *ent, vec3_t point);
qboolean PBM_FireResistant (edict_t *ent, vec3_t point);
qboolean PBM_NoFriendlyFire (edict_t *victim, edict_t *attacker);

void     PBM_BurnDamage
 (edict_t *victim, edict_t *fire, vec3_t point, vec3_t normal,
  vec3_t damage, int dflags, int mod);
void     PBM_BurnRadius
 (edict_t *fire, float radius, vec3_t damage, edict_t *ignore, int mod);

void     PBM_CheckMaster (edict_t *fire);
void     PBM_FireSpot (vec3_t spot, edict_t *ent);
qboolean PBM_FlameOut (edict_t *self);
void     PBM_Burn (edict_t *self);
void     PBM_Ignite (edict_t *victim, edict_t *attacker, vec3_t point);


/*
// x_fire2.c
// Pyrotechnics
*/
void     PBM_CheckFire (edict_t *self);
void     PBM_FireDropTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FireDrop
 (edict_t *attacker, vec3_t spot, vec3_t damage, vec3_t radius_damage,
  float duration_base, int blast_chance, int mod_strike, int mod_splash);
void     PBM_EasyFireDrop (edict_t *self);

void     PBM_CloudBurst (edict_t *self);
void     PBM_CloudBurstDamage (edict_t *self);
void     PBM_FlameCloud
 (edict_t *attacker, vec3_t start, vec3_t cloud, vec3_t timer,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance);

void     PBM_RemoveFireStorm (edict_t *self);
void     PBM_FireStormThink (edict_t *self);
void     PBM_LinkFireStorm (edict_t *self);
void     PBM_CreateFireStorm
 (edict_t *attacker, vec3_t start, vec3_t damage, vec3_t radius_damage,
  vec3_t dim);

qboolean PBM_CanDamageFromPoint (edict_t *targ, vec3_t point);
qboolean PBM_NegateMissile (edict_t *ent);
void     PBM_FlashRadius
 (edict_t *attacker, vec3_t origin, float damage, vec3_t radii,
  edict_t *ignore, int mod);
void     PBM_FlashEffects (vec3_t p1, vec3_t p2);
void     PBM_FlashExplosion
 (edict_t *attacker, vec3_t p1, vec3_t p2, float damage, vec3_t radii,
  edict_t *ignore, int mod);


/*
// x_fire3.c
// Generic fire-based attacks.
*/
void     PBM_CheckFireDodge
 (edict_t *self, vec3_t start, vec3_t dir, int speed);

void     PBM_FireAngleSpread (vec3_t spread, vec3_t dir);
void     PBM_SpreadConverter (vec3_t spread, float hspread, float vspread);

void     PBM_FireballTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FireFlamer
 (edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance,
  int mod_strike, int mod_splash);
void     PBM_FireInstantFlamer
 (edict_t *attacker, vec3_t start, vec3_t spread, vec3_t aimdir,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance,
  int mod_strike, int mod_splash);
void     PBM_FireExplosiveBurst
 (edict_t *self, vec3_t start, vec3_t spread, vec3_t aimdir, vec3_t damage,
  vec3_t radius_damage, int rain_chance, int blast_chance, int count,
  int mod_strike, int mod_splash);

void     PBM_FlameThrowerTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_FlameThrowerThink (edict_t *self);
void     PBM_FireFlameThrower
 (edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed,
  vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance,
  int mod_strike, int mod_splash);

static void PBM_FireIncendiaryRound
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick,
  int burn_chance, int te_impact, int hspread, int vspread, int mod);
void PBM_FireFlameBullet (edict_t *self, vec3_t start, vec3_t aimdir,
 int damage, int kick, int burn_chance, int hspread, int vspread, int mod);
void PBM_FireFlameShotgun (edict_t *self, vec3_t start, vec3_t aimdir,
 int damage, int kick, int burn_chance, int hspread, int vspread,
 int count, int mod);


/*===========================/  END OF FILE  /===========================*/
