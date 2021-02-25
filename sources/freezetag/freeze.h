struct {
	int	score;
	int	thawed;
	float	win_time;
	qboolean	update;
	float	last_update;
	int	frozen;
	int	alive;
	float	break_time;
	qboolean	ready;
} freeze[5];

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

struct {
	char*	name;
	char*	gravity;
	int	light;
	qboolean	did;
	qboolean	hook;
} maplist[64];

enum team_enum {
	red, blue, green, yellow, none };
static char*	freeze_team[] = {"Red", "Blue", "Green", "Yellow", "None"};
int	endMapIndex;

void gibThink(edict_t* ent);
