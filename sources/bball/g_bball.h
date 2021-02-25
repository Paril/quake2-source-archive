#define THREE_DISTANCE		350
#define PICKUP_RANGE		50.0
#define IDLE_TIME			7

#define BBALL_TIMER			3.0
#define BBALL_MINSPEED		400.0
#define BBALL_MAXSPEED		800.0
#define STEAL_RANGE 		50.0
#define OPEN_FOR_DUNK		100.0
#define DUNK_RANGE			75.0

enum {NOT_GOOD, STILL_GOOD};

vec_t FlatLength(vec3_t v);
void fire_bball (edict_t *self, vec3_t start, vec3_t aimdir, float speed, qboolean autoshot);
void weapon_bball_fire(edict_t *ent);
void Weapon_Bball (edict_t *ent);
void Inbound(int team);
void Jumpball();
qboolean BallStillGood();
void BallShowCheck(edict_t *ent);
void SP_rim_guard (edict_t *rim);
void Cmd_Ballmove_f (edict_t *ent);
void Cmd_Passball_f(edict_t *ent);
void Cmd_Autoshoot_f(edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void Cmd_Chasecam_Zoom(edict_t *ent, char *opt);
void Cmd_Chasecam_Viewlock(edict_t *ent);
void Cmd_Chasecam_Reset(edict_t *ent);

//bunch of frame crap for model
#define BBALL_ACTIVATE_LAST		4
#define BBALL_FIRE_LAST			10
#define BBALL_IDLE_LAST			20
#define BBALL_DEACTIVATE_LAST	25
#define BBALL_FIRE_FIRST		(BBALL_ACTIVATE_LAST + 1)
#define BBALL_IDLE_FIRST		(BBALL_FIRE_LAST + 1)
#define BBALL_DEACTIVATE_FIRST	(BBALL_IDLE_LAST + 1)

#define BBALL_DRIBBLE_FRAME		16
#define HAND_ACTIVATE_FIRST		(BBALL_DEACTIVATE_LAST + 1)

#define HAND_ACTIVATE_LAST		30
#define HAND_FIRE_LAST			40
#define HAND_IDLE_LAST			41
#define HAND_DEACTIVATE_LAST	46
#define HAND_FIRE_FIRST			(HAND_ACTIVATE_LAST + 1)
#define HAND_IDLE_FIRST			(HAND_FIRE_LAST + 1)
#define HAND_DEACTIVATE_FIRST	(HAND_IDLE_LAST + 1)


