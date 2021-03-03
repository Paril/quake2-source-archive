//===========================================================================
//
// Name:				p_menulib.h
// Function:		menu
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

#define MAX_MENU_NAME		32

#define MI_ITEM				1
#define MI_SUBMENU			2
#define MI_SEPERATOR			3

typedef struct menuitem_s
{
	unsigned char type;								//type of menu item
	char name[MAX_MENU_NAME];						//name of the menu item
	char bitmap[MAX_MENU_NAME];					//name of the menu item bitmap
	int id;												//menu item id
	struct menu_s *menu;								//menu the item is in
	struct menu_s *submenu;							//submenu if it is a submenu
	struct menuitem_s *prev, *next;				//next and prev item in menu
} menuitem_t;

typedef struct menu_s
{
	int id;												//menu id
	char title[MAX_MENU_NAME];						//title of the menu
	char background[MAX_MENU_NAME];				//background picture
   unsigned char type;								//menu type
	struct menu_s *parent;							//menu id of the parent
   short nummenuitems;								//number of menu items
   struct menuitem_s *firstmenuitem, *lastmenuitem;
} menu_t;

typedef struct menustate_s
{
	int showmenu;										//true if the menu is up
	int redraw;											//true if the menu should be redrawn
	struct menu_s *mainmenu;						//the main menu
	int menuid;											//id of the current (sub)menu
	void (*menuproc)(edict_t *ent, int id);	//menu procedure
   int highlighteditem;								//highlighted menu item
	int firstdisplayeditem;							//first item displayed
	float lastchange_time;							//last time the highlight changed
	int redrawmenu;									//true when the menu should be redrawn
	int removemenu;									//true when menu should be removed from screen
} menustate_t;

//returns the menuitem with the given id
menuitem_t *GetMenuItemWithId(menu_t *menu, int id);
//change the name of the menu item with the given id
void ChangeMenuItemName(menu_t *menu, int id, char *name);
//returns the name of the menu item with the given id
char *MenuItemName(menu_t *menu, int id);
//returns the 'item'th menu item from the given menu
menuitem_t *GetMenuItem(menu_t *menu, int item);
//create a new menu with the given title and background
menu_t *QuakeCreateMenu(int id, char *title, char *background);
//delete the given menu
void QuakeDeleteMenu(menu_t *menu);
//copy the given menu
menu_t *QuakeCopyMenu(menu_t *menu);
//append a menu option to the given menu
void QuakeAppendMenu(menu_t *menu, unsigned char type, int id,
							menu_t *submenu, char *name, char *bitmap);
//remove the menu item with the given id
void QuakeRemoveMenuItem(menu_t *menu, int id);
//send a status bar layout to the client
void SendStatusBar(edict_t *ent, char *bar);
//show the menu the client is viewing
void ShowMenu(edict_t *ent);
//go forward in the menu (activate menu item or enter submenu)
int MenuForward(edict_t *ent);
//go backward in the menu (go back to parent menu)
int MenuBack(edict_t *ent);
//go to the previous menu item
int MenuUp(edict_t *ent);
//go the the next menu item
int MenuDown(edict_t *ent);
//do regular menu updates
void DoMenu(edict_t *ent, usercmd_t *ucmd);
