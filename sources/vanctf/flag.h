//#include "g_local.h"

#define FLAG_ATBASE		0
#define FLAG_CARRIED	1
#define FLAG_ABOUT		2

//#define IS_FLAG_RED(ent)		(ent->s.renderfx & RF_SHELL_RED)
//#define IS_FLAG_BLUE(ent)		(ent->s.renderfx & RF_SHELL_BLUE)

void flag_init();
int	flag_has_flag(edict_t *ent);
int flag_team(edict_t *flag);
void flag_reset(int team);
void flag_respawn(edict_t *self);
void flags_drop(edict_t *owner);
qboolean flag_pickup(edict_t *self, edict_t *other);
void flag_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void flag_update_lasers(edict_t *l);
void flag_lasers_link(edict_t *l);
void flag_lasers_spawn(edict_t *flag);
void flag_place (edict_t *flag);
void flag_spawn (edict_t *flag, int team);
void SP_item_flag_team1 (edict_t *ent);
void SP_item_flag_team2 (edict_t *ent);
int flag_state(int team, edict_t **carrier);
void Cmd_FlagStat_f (edict_t *ent);
void flag_set_effects (edict_t *player);

extern int flag_renderfx[];
extern gitem_t	*flag_item[];
extern char *flag_classnameforteam[];
