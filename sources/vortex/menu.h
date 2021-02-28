#define MAX_LINES				21
#define LINE_SPACING			8
#define MENU_DELAY_TIME			0.2		// 3.65 delay time when cycling thru menu (in seconds)

#define MENU_WHITE_CENTERED		20001
#define	MENU_GREEN_CENTERED		20002
#define MENU_GREEN_RIGHT		20003
#define MENU_GREEN_LEFT			20004

// 3.45 menu indexes, used to tell which menu we're on
// note that this isn't necessary unless there are 2 menus
// that are exactly the same (because the compiler treats
// them as the same function)
#define MENU_MASTER_PASSWORD	1
#define MENU_SPECIAL_UPGRADES	2
#define MENU_MULTI_UPGRADE		3
#define MENU_COMBAT_PREFERENCES	4

void addlinetomenu (edict_t *ent,char *line,int option);
void clearmenu (edict_t *ent);
void setmenuhandler (edict_t *ent,void (*optionselected)(edict_t *ent,int option));
void clearallmenus (void);

void menuup (edict_t *ent);
void menudown (edict_t *ent);
void menuselect (edict_t *ent);

void initmenu (edict_t *ent);
void showmenu (edict_t *ent);
void closemenu (edict_t *ent);

qboolean ShowMenu (edict_t *ent);
qboolean InMenu (edict_t *ent,int index, void (*optionselected)(edict_t *ent,int option));

typedef struct menumsg_s
{
	char	*msg;
	int		option;
} menumsg_t;

typedef struct menusystem_s
{
	void 		(*optionselected)(edict_t *ent,int option);
	void		(*oldmenuhandler)(edict_t *ent,int option);
	qboolean 	menu_active;
	qboolean	displaymsg;
	int			oldline;
	menumsg_t	messages[MAX_LINES];
	int 		currentline;
	int			num_of_lines;
	int			menu_index;
} menusystem_t;
