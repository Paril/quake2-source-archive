/* Vanilla CTF Grappling Hook */

qboolean hook_cond_reset(edict_t *self);

edict_t *hook_laser_start (edict_t *ent);

void hook_laser_think (edict_t *self);
void hook_reset (edict_t *rhook);
void hook_service (edict_t *self);
void hook_track (edict_t *self);
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void hook_fire (edict_t *ent);
void fire_hook (edict_t *owner, vec3_t start, vec3_t forward);

#define		HOOK_READY	0
#define		HOOK_OUT	1
#define		HOOK_ON		2
