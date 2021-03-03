#define		RUNE_FIRST		1
#define		RUNE_STRENGTH	1
#define		RUNE_RESIST		2
#define		RUNE_HASTE		3
#define		RUNE_REGEN		4
#define		RUNE_LAST		4

#define		RUNE_REGEN_PER_SEC		10

#define		RUNE_STRENGTH_ICON		"p_quad"
#define		RUNE_RESIST_ICON		"p_invulnerability"
#define		RUNE_HASTE_ICON			"p_adrenaline"
#define		RUNE_REGEN_ICON			"i_health"

#define		RUNE_STRENGTH_NAME		"Power Amplifier"
#define		RUNE_RESIST_NAME		"Disruptor Shield"
#define		RUNE_HASTE_NAME			"Time Accel"
#define		RUNE_REGEN_NAME			"AutoDoc"

extern char *rune_namefornum[];
extern char *rune_iconfornum[];
extern int	rune_renderfx[];

qboolean rune_has_rune(edict_t *ent, int type);
int rune_has_a_rune(edict_t *ent);
qboolean rune_pickup(edict_t *self, edict_t *other);
void rune_move (edict_t *self);
void rune_make_touchable (edict_t *ent);
void rune_use (edict_t *ent, gitem_t *item);
void rune_drop (edict_t *ent, gitem_t *item);
void runes_drop (edict_t *ent);
void rune_drop_dying (edict_t *ent, gitem_t *item);
void runes_drop_dying (edict_t *ent);
void rune_select_spawn_point(vec3_t origin);
void rune_spawn(edict_t *rune, gitem_t *item);
void runes_spawn(edict_t *self);
void runes_spawn_start();
void rune_set_effects(edict_t *ent);
void rune_apply_regen(edict_t *ent);
