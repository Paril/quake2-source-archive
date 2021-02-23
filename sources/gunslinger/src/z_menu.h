/*
Rebel Programmers Society Quake2 Menu System
Copyright July 1999
Developed by Quintin Stone
*/

#define MAXSTRLEN	2048

#define ALIGN_LEFT		1
#define ALIGN_RIGHT		2
#define ALIGN_CENTER	4
#define ALIGN_TITLE		8
#define	HALF_SPACE		16
#define	FULL_SPACE		32
#define NO_SPACE		64

typedef struct menu_s {
	char			*string;
	int				align;
	int				value;
	qboolean		choosable;
	struct	menu_s	*next;
} menu_t;

typedef struct player_menu_s {
	qboolean	showmenu;
	qboolean	update;
	void		(*create_menu)(edict_t *ent);
	menu_t		*start;
	menu_t		*end;
	int			selection;
	int			total;
	qboolean	transmitted;
	void		(*usr_menu_sel)(edict_t *ent, int choice);
	int			displayframe;
} player_menu_t;

void RPS_MenuOpen(edict_t *ent, void (*create_menu)(edict_t *ent), void (*usr_menu_sel)(edict_t *ent, int choice), qboolean update, int delay);
void RPS_MenuAdd(edict_t *ent, char *string, int align, int value, qboolean choice);
void RPS_MenuTransmit(edict_t *ent);
void RPS_MenuUpdate(edict_t *ent);
void RPS_MenuNext(edict_t *ent);
void RPS_MenuPrev(edict_t *ent);
void RPS_MenuSelect(edict_t *ent);
void RPS_MenuClose(edict_t *ent);
void RPS_MenuFree(edict_t *ent);
void RPS_MenuHelp(edict_t *ent);
void RPS_ColorText(char *text, qboolean symbols);
char *RPS_AddToStringPtr(char *base, char *add, qboolean del);
void RPS_AddToString(char *base, char *add, int max);
