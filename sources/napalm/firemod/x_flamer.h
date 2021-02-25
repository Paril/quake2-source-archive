/*==========================================================================
//  x_flamer.h -- by Patrick Martin             Last updated:  2-27-1999
//--------------------------------------------------------------------------
//  This is the header file for x_flamer.c.
//========================================================================*/

/****************/
/*  PROTOTYPES  */
/****************/

/*
// x_flamer.c
// Specialty fire-based attacks.
*/
static void PBM_FireGrenadeExplode (edict_t *ent);
static void PBM_FireGrenadeTouch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void PBM_FireFlameGrenade
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius, vec3_t fire_damage,
  vec3_t splash_damage);
void PBM_FireFlameHandGrenade
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius, qboolean held, vec3_t fire_damage,
  vec3_t splash_damage);

void PBM_FireRocketTouch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void PBM_FireFlameRocket
 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
  float damage_radius, int radius_damage, vec3_t fire_damage,
  vec3_t splash_damage);

void     PBM_MeteorRadius
 (edict_t *fire, edict_t *attacker, float damage, edict_t *ignore,
  float radius, int mod);
void     PBM_MeteorTouch
 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void     PBM_CheckMeteor (edict_t *self);
void     PBM_FireMeteor
 (edict_t *self, vec3_t start, vec3_t dir, int speed, float damage_radius,
  vec3_t direct_damage, vec3_t rain_damage, vec3_t splash_damage,
  vec3_t storm);

void     PBM_FireBuster
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int radius_damage,
  vec3_t radii, vec3_t fire_damage);


/*===========================/  END OF FILE  /===========================*/
