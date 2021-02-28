#define D_PHYSICAL	1
#define D_MAGICAL	2
#define D_BULLET	4
#define D_ENERGY	8
#define D_EXPLOSIVE	16
#define D_PIERCING	32
#define D_SHELL		64
#define D_WORLD		128

int		G_DamageType (int mod, int dflags);
float	G_AddDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, float damage, int dflags, int mod);
float	G_SubDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, float damage, int dflags, int mod);
float	G_ModifyDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, float damage, int dflags, int mod);