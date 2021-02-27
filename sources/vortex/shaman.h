#ifndef SHAMAN_H
#define SHAMAN_H

void Cmd_AmpDamage(edict_t *ent);
void Cmd_Weaken(edict_t *ent);
void Cmd_Slave(edict_t *ent);
void Cmd_Amnesia(edict_t *ent);
void Cmd_Curse(edict_t *ent);
void Cmd_Bless(edict_t *ent);
void Cmd_Healing(edict_t *ent);
void MindAbsorb(edict_t *ent);
void CursedPlayer (edict_t *ent);
void CurseEffects (edict_t *self, int num, int color);
void Cmd_LifeDrain(edict_t *ent);
void LifeDrain (edict_t *ent);
void Bleed (edict_t *curse);
void Cmd_LowerResist (edict_t *ent);
#endif