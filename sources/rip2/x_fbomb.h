/*==========================================================================
//  x_fbomb.h -- by Patrick Martin              Last updated:  3-5-1998
//--------------------------------------------------------------------------
//  This is the header file for x_fbomb.c.
//========================================================================*/

static void FireGrenade_Explode (edict_t *ent);
static void FireGrenade_Touch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_flamegrenade
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius);
void fire_flamegrenade2
 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed,
  float timer, float damage_radius, qboolean held);
void firerocket_touch
 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_flamerocket
 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
  float damage_radius, int radius_damage);


/*=============================  END OF FILE =============================*/
