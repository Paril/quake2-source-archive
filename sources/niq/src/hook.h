#ifdef OHOOK

void hook_laser_think (edict_t *self);
edict_t *hook_laser_start (edict_t *ent, edict_t *owner);
void hook_reset (edict_t *rhook);
qboolean hook_cond_reset(edict_t *self);
void hook_service (edict_t *self);
void hook_track (edict_t *self);
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_hook (edict_t *owner, vec3_t start, vec3_t forward);
void hook_fire (edict_t *ent);
//void Weapon_Hook (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void Weapon_Hook (edict_t *ent);
#endif//OHOOK

