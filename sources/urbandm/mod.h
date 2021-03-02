void CopyHead (edict_t *ent);
void CapturedHeadThink (edict_t *ent, edict_t *head);
void DropPlayerHeads (edict_t *ent);
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin );
void ApplyProtect (edict_t *ent);
extern void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void Weapon_Winder (edict_t *ent);
void LaserDefenceGrenade (edict_t *ent);
void PlaceTurret (edict_t *ent);
void FeignDeath (edict_t *self);
int ApplyLocationalSystem (edict_t *attacker, edict_t *targ, vec3_t point, int mod, int d_damage);

#define NEXT_JUMP 0
#define JUMP_POWER_FINISHED 1
#define PROTECT_PERIOD 2
#define NEXT_PROTECT 3
#define MSG_TIME	4
#define LASER_TIME	5
#define ATTRACKT_BALL_TIME 6
#define FEIGN_GUN_TIME 7