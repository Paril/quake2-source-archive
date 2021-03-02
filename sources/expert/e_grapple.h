/**
 * e_grapple.h
 *
 * The Expert hook.
 */

qboolean Grapple_Is_Pulling (gclient_t *client);
qboolean Ended_Grappling(gclient_t *client);
qboolean Is_Grappling(gclient_t *client);
void Throw_Grapple(edict_t *ent);
void Release_Grapple(edict_t *ent);
void Think_Grapple(edict_t *ent);
void Pull_Grapple(edict_t *ent);
