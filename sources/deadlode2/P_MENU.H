#define MENU_STACK_SIZE		8

enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

#define MENU_POS_CENTER		0x00
#define MENU_POS_TOP		0x01
#define MENU_POS_LEFT		0x10
#define MENU_POS_RIGHT		0x20
#define MENU_POS_VERTICAL	0x0F
#define MENU_POS_HORIZONTAL	0xF0

typedef struct pmenuhnd_s {
	struct pmenu_s *entries;
	int cur;
	int num;
	//DEADLODE
	int oldstart;
	int id;
	int param[5];
	void *data;
	void (*UpdateFunc)(edict_t *ent, struct pmenuhnd_s *entry);	// use to refresh menu, if on-screen
	//DEADLODE
//	struct pmenuhnd_s *parent;
} pmenuhnd_t;

typedef struct pmenu_s {
	char *text;
	//DEADLODE
	char *pic_s;	// 8x8 icon to display to left of name
	char *pic_b;	// 24x24 icon displayed on status bar
	int param[5];	// misc params field, usually for identifying
					// previous menu types, or menu item types
	int	xofs;		// amount to indent item
	int flags;		// Misc params for this entry.  (HEADER, FOOTER, etc?)
	//DEADLODE
	int align;
	char *target;	// whats another 4 bytes?  - for map targets.
	void *arg;
	void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry);
	void (*AltSelectFunc)(edict_t *ent, struct pmenu_s *entry);
} pmenu_t;

//DEADLODE
extern int FindMenu (edict_t *ent, int id);
extern void PMenu_Open(edict_t *ent, int id, int *param, void *data);
//DEADLODE
extern void PMenu_Close(edict_t *ent, qboolean show_lower);
extern void PMenu_CloseAll(edict_t *ent);
extern void PMenu_Update(edict_t *ent);
extern void PMenu_Next(edict_t *ent);
extern void PMenu_Prev(edict_t *ent);
extern void PMenu_Select(edict_t *ent);
extern void PMenu_AltSelect(edict_t *ent);
