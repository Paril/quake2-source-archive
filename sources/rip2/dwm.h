// 	dwm.h

/*-------------------------------------------------
 	include this file as the last line of local.h
	and change the original function names so that
	the replacement ones are used instead
--------------------------------------------------*/

// g_weapon.c replacements
void Grenade_Explode (edict_t *ent);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
// g_misc.c replacementsvoid barrel_explode (edict_t *self);
// g_target.c replacementsvoid target_explosion_explode (edict_t *self);
// new functions
void T_ShockWave (edict_t *inflictor, float damage, float radius);
void T_ShockItems (edict_t *inflictor);
void ThrowShrapnel (edict_t *self, char *modelname, float speed, vec3_t origin);
void ThrowShrapnel2 (edict_t *self, char *modelname, float speed, vec3_t origin);
void ThrowShrapnel3 (edict_t *self, char *modelname, float speed, vec3_t origin);
void ThrowShrapnel4 (edict_t *self, char *modelname, float speed, vec3_t origin);
void make_debris (edict_t *ent);void BigBang (edict_t *ent);
qboolean isvisible (edict_t *self, edict_t *other);