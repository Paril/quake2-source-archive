/* *** JETPACK *** */
void Use_Jet (edict_t *ent, gitem_t *item);
qboolean Jet_AvoidGround( edict_t *ent );
qboolean Jet_Active( edict_t *ent );
void Jet_BecomeExplosion( edict_t *ent, int damage );
void Jet_ApplyLifting( edict_t *ent );
void Jet_ApplySparks ( edict_t *ent );
void Jet_ApplyRolling( edict_t *ent, vec3_t right );
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );
