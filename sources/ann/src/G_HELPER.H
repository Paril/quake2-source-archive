/* *** HELP FUNCTIONS *** */

// G_..radius doesn't work, so I wrote T_..
//qboolean G_Within_Radius(vec3_t start, vec3_t end, float rad);
qboolean G_EntExists(edict_t *ent);
qboolean G_ClientNotDead(edict_t *ent);
qboolean G_ClientInGame(edict_t *ent);
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin );
void G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin );
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin );
void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin );
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin );
