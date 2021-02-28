#define CACODEMON_SKULL_REGEN_FRAMES	250
#define CACODEMON_SKULL_REGEN_DELAY		50

#define MELEE_HIT_NOTHING		0
#define MELEE_HIT_ENT			1
#define MELEE_HIT_WORLDSPAWN	2

void RunFlyerFrames (edict_t *ent, usercmd_t *ucmd);
void Cmd_PlayerToFlyer_f (edict_t *ent);
void RunParasiteFrames (edict_t *ent, usercmd_t *ucmd);
void Cmd_PlayerToParasite_f (edict_t *ent);
void RunCacodemonFrames (edict_t *ent, usercmd_t *ucmd);
void Cmd_PlayerToCacodemon_f (edict_t *ent);
void Cmd_PlayerToMutant_f (edict_t *ent);
void RunMutantFrames (edict_t *ent, usercmd_t *ucmd);
void Cmd_PlayerToBrain_f (edict_t *ent);
qboolean mutant_boost (edict_t *ent);//4.4
void boss_makron_spawn (edict_t *ent);
void MorphRegenerate (edict_t *ent, int regen_delay, int regen_frames);
void Cmd_PlayerToMedic_f (edict_t *ent);
void RunMedicFrames (edict_t *ent, usercmd_t *ucmd);
void RunBerserkFrames (edict_t *ent, usercmd_t *ucmd);
void Cmd_PlayerToBerserk_f (edict_t *ent);

