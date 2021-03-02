//g_faith.h

#define CHRISTIAN 0
#define SATANIST 1

void OpenTeamMenu (edict_t *ent);
void JoinGod (edict_t *ent, pmenu_t *p);
void JoinSatan (edict_t *ent, pmenu_t *p);
void holylevel (edict_t *ent);
void SpellMenu (edict_t *ent);
void SpellOne (edict_t *ent, pmenu_t *p);
void SpellTwo (edict_t *ent, pmenu_t *p);
void SpellThree (edict_t *ent, pmenu_t *p);
void SpellFour (edict_t *ent, pmenu_t *p);
void SpellFive (edict_t *ent, pmenu_t *p);
void SpellSix (edict_t *ent, pmenu_t *p);
void Flash_Explode (edict_t *ent);
void Shockwave (edict_t *ent);
void ShockwaveThink (edict_t *ent);
void ShockwaveExplode (edict_t *self);
void RandomPowerUp (edict_t *ent);
void Lightning (edict_t *ent);
void LightningThink (edict_t *ent);
void LightningExplode (edict_t *self);
void Vampire (edict_t *self);
void Spell_earthquake (edict_t *ent);
void Spell_earthquake_think (edict_t *self);
void Starburst (edict_t *ent);
void fire_starburst (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void starburst_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Hellspawn (edict_t *ent);
void Eyes (edict_t *ent);
void Eyes_think (edict_t *self);
qboolean CheckforIt (void);
void SetIt (void);
void SetItEffects (edict_t *ent);
void Rules (edict_t *killer, edict_t *victim);
void RemoveItEffects(edict_t *ent);
void Check (edict_t *ent);
