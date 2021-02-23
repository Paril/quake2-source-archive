#define		FLASH_RADIUS		400
#define		POISON_RADIUS		400

void pre_target_laser_think (edict_t *self);

//Quake2 Functions:

void NoAmmoWeaponChange (edict_t *ent);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Grenade_Explode (edict_t *ent);
void target_laser_on (edict_t *self);
void target_laser_off (edict_t *self);
void target_laser_think (edict_t *self);

