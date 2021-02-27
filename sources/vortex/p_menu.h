
enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s {
	struct pmenu_s *entries;
	int cur;
	int num;
	qboolean UseNumberKeys; //If true, number keys will select a menu item
	float MenuTimeout;	//If set, menu will time out and be removed after a set amount of time
	qboolean ShowBackground;	//Set if background should be shown
} pmenuhnd_t;

typedef struct pmenu_s {
	char *text;
	int align;
//	void *arg;
	int arg;
	void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry);
} pmenu_t;

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, qboolean usekeys, qboolean showbackground);
void PMenu_Close(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);
int MenuFromNumberKey(edict_t *ent, int slot);