//************************************************************************************************
//			String functions
//************************************************************************************************

void padRight(char *String, int numChars);
char *GetWeaponString (int weapon_number);
char *GetModString (int weapon_number, int mod_number);
void V_RestoreMorphed (edict_t *ent, int refund);
void V_ModifyMorphedHealth (edict_t *ent, int type, qboolean morph);
void V_RegenAbilityAmmo (edict_t *ent, int ability_index, int regen_frames, int regen_delay);
void V_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void V_UpdatePlayerAbilities (edict_t *ent);
qboolean V_HealthCache (edict_t *ent, int max_per_second, int update_frequency);
qboolean V_ArmorCache (edict_t *ent, int max_per_second, int update_frequency);
void V_ResetPlayerState (edict_t *ent);
void V_TouchSolids (edict_t *ent);
qboolean V_HasSummons (edict_t *ent);
void V_ValidateCombatPreferences (edict_t *ent);//4.5
void V_SetEffects (edict_t *ent);
void V_ShellNonAbilityEffects (edict_t *ent);
void V_ShellAbilityEffects (edict_t *ent);
int V_GetNumPlayerPrefs (qboolean monsters, qboolean players);//4.5
qboolean V_MatchPlayerPrefs (edict_t *player, int monsters, int players);//4.5
qboolean isMonster (edict_t *ent);
qboolean isMorphingPolt(edict_t *ent);
//************************************************************************************************