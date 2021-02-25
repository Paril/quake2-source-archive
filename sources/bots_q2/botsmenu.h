
#define MAXITEMSIZE 100

#ifndef ARENA
typedef struct arena_link_s {
        void *it;
        struct arena_link_s *next;
        struct arena_link_s *prev;
} arena_link_t;
#endif

typedef struct menu_item_s {
	char	*itemtext; //text displayed
	char	*valuetext; //text next to value
	int		itemvalue; //a value to display
	int	(*ItemSelect) (edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key); 
	/*
	callback function, return 1 to leave the menu on stack
		ent=calling ent
		menulink=link to calling menu (menulink->it)
		(menulink->prev is previous menu link, menulink->next is next menu link)
		selected=link to calling item (selected->it)
		(selected->next is link to next menu item (selected->next->it) selected->prev is link to previous)
		key=1 for invuse (enter) or 0 for invdrop (')
	*/
	  
} menu_item_t;



void DisplayMenu(edict_t *ent);
void UseMenu(edict_t *ent, int key, qboolean isauto);
void MenuThink(edict_t *ent);
void MenuNext(edict_t *ent);
void MenuPrev(edict_t *ent);
arena_link_t *CreateMenu(edict_t *ent, char *name);
void AddMenuItem(arena_link_t *menulink, char *itemtext, char *valuetext, int value, void *Callback);
void FinishMenu(edict_t *ent, arena_link_t *menu);
void PrintMenuQueue(edict_t *ent);
void KillFancyMenu(edict_t *ent);


