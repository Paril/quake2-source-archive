typedef struct {
	int	score;
	int	thawed;
	float	win_time;
	qboolean	update;
	float	last_update;
	int	frozen;
	int	alive;
	float	break_time;
	qboolean	ready;
} freeze_t;

extern freeze_t freeze[5];

typedef struct hnode {
	struct pnode*	entries;
	int	cur;
	int	num;
} hndnode;

typedef struct pnode {
	char*	text;
	int	align;
	void*	arg;
	void	(*selectfunc)(edict_t* ent, struct pnode* entry);
} pmenunode;

typedef struct {
	char*	name;
	char*	gravity;
	int	light;
	qboolean	did;
	qboolean	hook;
} maplist_t;
extern maplist_t maplist[64];

enum team_enum {
	red, blue, green, yellow, none };
static char*	freeze_team[] = {"Red", "Blue", "Green", "Yellow", "None"};
extern int	endMapIndex;

void gibThink(edict_t* ent);
void freezeRespawn(edict_t* ent, float delay);
void pmenu_update(edict_t* ent);
void freezeEffects(edict_t* ent);
void pmenu_close(edict_t* ent);
void pmenu_select(edict_t* ent);
qboolean playerDamage(edict_t* targ, edict_t* attacker, int damage);
void playerShell(edict_t* ent, int team);
qboolean freezeCheck(edict_t* ent);
void freezeAnim(edict_t* ent);
void pmenuBegin(edict_t* ent);
void playerWeapon(edict_t* ent);
void freezeSkin(edict_t* ent, char* s);
void freezeMain(edict_t* ent);
void cmdMoan(edict_t* ent);
void freezeIntermission(void);
void freezeScore(edict_t* ent, edict_t* killer);
void cvarFreeze(void);
void playerStat(edict_t* ent);
qboolean freezeMap(void);
qboolean endCheck(void);
void pmenu_next(edict_t* ent);
void pmenu_prev(edict_t* ent);
void sayTeam(edict_t* who);
void cmdHook(edict_t* ent);
void cmdFlashlight(edict_t* ent);
void cmdChange(edict_t* ent);
void cmdPlay(edict_t* ent);
void cmdVote(edict_t* ent);
void cmdReady(edict_t* ent);
qboolean gibCheck(void);
void freezeSpawn(void);