//Leper_item.c
qboolean LeperPickUpArm (edict_t *ent, edict_t *other);
qboolean LeperPickUpLeg (edict_t *ent, edict_t *other);
qboolean LeperPickUpGoldenLeg (edict_t *ent, edict_t *other);
qboolean LeperPickUpHead (edict_t *ent, edict_t *other);
//leper_think.c
void Leper_Think (edict_t *ent);
//leper_locdam.c
int MatrixApplyLocationDamage (edict_t *targ, vec3_t point, int mod, int damage);
void Cmd_KillLeg_f(edict_t *ent);
void Cmd_KillArm_f(edict_t *ent);
int Leper_RandomShot (edict_t *ent, int offsetangle);
qboolean IsFemale (edict_t *ent);
qboolean IsNeutral (edict_t *ent);
void ThrowUpNow(edict_t *self, int limb);
void weapon_teeth_fire (edict_t *ent);
void Weapon_Teeth (edict_t *ent);
void kick_attack (edict_t *ent, vec3_t start, vec3_t dir, int damage, int kick, int MOD);

#define round(x) ((int)(x+0.5))

extern char *leper_statusbar;
/*
===================
Hit Locations
===================
*/
/*typedef enum
{
	LOC_NULL,
	LOC_LEFTARM,
	LOC_RIGHTARM,
	LOC_LEFTLEG,
	LOC_RIGHTLEG,
	LOC_CHEST,
	LOC_BACK,
	LOC_FACE,
	LOC_HEAD
} location_t;
*/
#define		LOC_NULL		0
#define		LOC_LEFTARM		1
#define		LOC_RIGHTARM	2
#define		LOC_LEFTLEG		3
#define		LOC_RIGHTLEG	4
#define		LOC_CHEST		5
#define		LOC_BACK		6
#define		LOC_FACE		7
#define		LOC_HEAD		8



