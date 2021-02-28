// general stuff
#define FRAMES_RUN_FORWARD			1
#define FRAMES_RUN_BACKWARD			2
#define FRAMES_ATTACK				3

#define BOSS_TARGET_RADIUS			1024
#define BOSS_ALLY_BONUS_RANGE		512
#define BOSS_EXPERIENCE				250
#define BOSS_CREDITS				100
#define	BOSS_DAMAGE_BONUSMOD		3
#define BOSS_ALLY_BONUSMOD			3
#define BOSS_STATUS_DELAY			5
#define BOSS_MAXVELOCITY			200
#define BOSS_REGEN_IDLE_FRAMES		900
#define BOSS_REGEN_ACTIVE_FRAMES	1800

dmglist_t *findDmgSlot (edict_t *self, edict_t *other);
dmglist_t *findEmptyDmgSlot (edict_t *self);
dmglist_t *findHighestDmgPlayer (edict_t *self);
void printDmgList (edict_t *self);
void boss_pain (edict_t *self, edict_t *other, float kick, int damage);
qboolean findNearbyBoss (edict_t *self);
char *HiPrint(char *text);
void boss_eyecam (edict_t *player, edict_t *boss);
void boss_position_player (edict_t *player, edict_t *boss);
qboolean boss_findtarget (edict_t *boss);
qboolean boss_checkstatus (edict_t *self);
void boss_update (edict_t *ent, usercmd_t *ucmd);
void G_RunFrames1 (edict_t *ent, int start_frame, int end_frame, int *skip_frames, qboolean reverse);
void boss_regenerate (edict_t *self);
void CreateBoss (edict_t *ent);
int p_tank_getFirePos (edict_t *self, vec3_t start, vec3_t forward);

// tank commander stuff
#define TANK_INITIAL_HEALTH			2000
#define TANK_ADDON_HEALTH			1300
#define TANK_MAXVELOCITY			200
#define TANK_ROCKET_INITIAL_DAMAGE	80
#define TANK_ROCKET_ADDON_DAMAGE	10
#define TANK_ROCKET_SPEED			950
#define TANK_ROCKET_MAXRADIUS		256
#define TANK_PUNCH_INITIAL_DAMAGE	70
#define TANK_PUNCH_ADDON_DAMAGE		30
#define TANK_PUNCH_RADIUS			256
#define TANK_REGEN_IDLE_FRAMES		1200
#define TANK_REGEN_ACTIVE_FRAMES	1800
#define TANK_STATUS_DELAY			5
#define TANK_TARGET_RADIUS			1024

void boss_tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void boss_tank_think (edict_t *self);
