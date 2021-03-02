//===========================================================================
//
// Name:				p_menulib.c
// Function:		menu
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

#include "g_local.h"
#include "bl_spawn.h"
#include "p_observer.h"

#define MAX_DISPLAYEDMENUITEMS	16
#define MENUITEMTEXT_YOFFSET		48
#define MENUITEMTEXT_XOFFSET		66
#define MENUCHANGE_MOVE				1

//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menuitem_t *GetMenuItemWithId(menu_t *menu, int id)
{
	menuitem_t *mi, *found;

	for (mi = menu->firstmenuitem; mi; mi = mi->next)
	{
		if (mi->id == id) return mi;
	} //end for
	for (mi = menu->firstmenuitem; mi; mi = mi->next)
	{
		if (mi->type == MI_SUBMENU)
		{
			if (mi->submenu)
			{
				found = GetMenuItemWithId(mi->submenu, id);
				if (found) return found;
			} //end if
		} //end if
	} //end for
	return NULL;
} //end of the function GetMenuItemWithId
//========================================================================
// returns the menu with the given id
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menu_t *GetMenuWithId(menu_t *mainmenu, int id)
{
	menuitem_t *mi;
	menu_t *found;

	if (mainmenu->id == id) return mainmenu;
	for (mi = mainmenu->firstmenuitem; mi; mi = mi->next)
	{
		if (mi->type == MI_SUBMENU)
		{
			if (mi->submenu)
			{
				found = GetMenuWithId(mi->submenu, id);
				if (found) return found;
			} //end if
		} //end if
	} //end for
	return NULL;
} //end of the function GetCurrentMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menu_t *GetCurrentMenu(menustate_t *menustate)
{
	menu_t *menu;

	menu = GetMenuWithId(menustate->mainmenu, menustate->menuid);
	if (!menu) return menustate->mainmenu;
	return menu;
} //end of the function GetCurrentMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void ChangeMenuItemName(menu_t *menu, int id, char *name)
{
	menuitem_t *mi;

	mi = GetMenuItemWithId(menu, id);
	if (!mi) return;

	strncpy(mi->name, name, MAX_MENU_NAME);
} //end of the function ChangeMenuItemName
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
char *MenuItemName(menu_t *menu, int id)
{
	menuitem_t *mi;

	mi = GetMenuItemWithId(menu, id);
	if (!mi) return NULL;
	return mi->name;
} //end of the function MenuItemName
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menuitem_t *GetMenuItem(menu_t *menu, int item)
{
	menuitem_t *i;

	for (i = menu->firstmenuitem; i; i = i->next)
	{
		if (--item < 0)
		{
			return i;
		} //end if
	} //end for
	return NULL;
} //end of the function GetMenuItem
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menu_t *QuakeCreateMenu(int id, char *title, char *background)
{
	menu_t *m;

	m = gi.TagMalloc(sizeof(menu_t), TAG_GAME);
	memset(m, 0, sizeof(menu_t));
	m->id = id;
	strncpy(m->title, title, MAX_MENU_NAME);
	//make the title green
	for (title = m->title; *title; title++) *title += 128;
	//
	if (background) strncpy(m->background, background, MAX_MENU_NAME);
	else strcpy(m->background, "");
	m->nummenuitems = 0;
	return m;
} //end of the function QuakeCreateMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void QuakeDeleteMenu(menu_t *menu)
{
	menuitem_t *mi;

	for (mi = menu->firstmenuitem; mi; mi = menu->firstmenuitem)
	{
		if (mi->type == MI_SUBMENU) QuakeDeleteMenu(mi->submenu);
		menu->firstmenuitem = mi->next;
		gi.TagFree(mi);
	} //end for
	gi.TagFree(menu);
} //end of the function QuakeDeleteMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void QuakeAppendMenu(menu_t *menu, unsigned char type, int id,
         menu_t *submenu, char *name, char *bitmap)
{
	menuitem_t *mi;

	mi = gi.TagMalloc(sizeof(menuitem_t), TAG_GAME);
	memset(mi, 0, sizeof(menu_t));

	mi->type = type;
	mi->id = id;
	mi->menu = menu;		//the menu the item is in
	mi->submenu = submenu;
	if (submenu) submenu->parent = menu;
	if (name) strncpy(mi->name, name, MAX_MENU_NAME);
	if (bitmap) strncpy(mi->bitmap, bitmap, MAX_MENU_NAME);

	mi->prev = menu->lastmenuitem;
	if (menu->lastmenuitem) menu->lastmenuitem->next = mi;
	else menu->firstmenuitem = mi;
	menu->lastmenuitem = mi;
	mi->next = NULL;

	menu->nummenuitems++;
} //end of the function QuakeAppendMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
menu_t *QuakeCopyMenu(menu_t *menu)
{
	menuitem_t *mi;
	menu_t *newmenu, *submenu;

	newmenu = QuakeCreateMenu(menu->id, menu->title, menu->background);
	for (mi = menu->firstmenuitem; mi; mi = mi->next)
	{
		submenu = NULL;
		if (mi->type == MI_SUBMENU)
		{
			if (mi->submenu) submenu = QuakeCopyMenu(mi->submenu);
		} //end if
		QuakeAppendMenu(newmenu, mi->type, mi->id, submenu, mi->name, mi->bitmap);
	} //end for
	return newmenu;
} //end of the function QuakeCopyMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void QuakeRemoveMenuItem(menu_t *menu, int id)
{
	menuitem_t *mi;

	mi = GetMenuItemWithId(menu, id);
	if (!mi) return;

	if (mi->prev) mi->prev->next = mi->next;
	else mi->menu->firstmenuitem = mi->next;
	if (mi->next) mi->next->prev = mi->prev;
	else mi->menu->lastmenuitem = mi->prev;
	//
	mi->menu->nummenuitems--;
} //end of the function QuakeRemoveMenuItem
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void SendStatusBar(edict_t *ent, char *bar)
{
//	gi.WriteByte(0x0D);		//configstring
//	gi.WriteShort(5);			//status bar index
	gi.WriteByte(svc_layout);
	gi.WriteString(bar);
	gi.unicast(ent, false);
} //end of the function SendStatusBar
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void DisplayMenu(edict_t *ent, menu_t *menu, int highlighteditem, int firstdisplayeditem)
{
	int i, yoffset;
	char menustring[1400];
	menuitem_t *menuitem;

	if (!menu) return;
	//background pic
	if (menu->background && strlen(menu->background)) sprintf(menustring, "xv 32 yv 4 picn %s ", menu->background);
	else sprintf(menustring, "");
	//menu title
	sprintf(menustring+strlen(menustring), "xv 0 yv 24 cstring \"%s\" ", menu->title);
	//first menu item
	menuitem = menu->firstmenuitem;
	//go to the first menu item
	for (i = 0; i < firstdisplayeditem; i++) menuitem = menuitem->next;
	//
	yoffset = MENUITEMTEXT_YOFFSET;
	if (firstdisplayeditem)
	{
		sprintf(menustring+strlen(menustring), "xv %d yv %d string \"... more ...\"", MENUITEMTEXT_XOFFSET, MENUITEMTEXT_YOFFSET-8);
	} //end if
	//show the menu items
	for (i = 0; i < MAX_DISPLAYEDMENUITEMS && menuitem; i++)
	{
		if (firstdisplayeditem + i == highlighteditem)
		{
			sprintf(menustring+strlen(menustring), "xv %d yv %d string \"%c\"", MENUITEMTEXT_XOFFSET-16, yoffset, 13 + 128);
		} //end if
		//print the menu option
		sprintf(menustring+strlen(menustring), "xv %d yv %d string \"%-16s\" ", MENUITEMTEXT_XOFFSET, yoffset, menuitem->name);
		//
		if (menuitem->type == MI_SUBMENU)
		{
			sprintf(menustring+strlen(menustring), "xv %d yv %d string \"%c\"", MENUITEMTEXT_XOFFSET+128, yoffset, 13);
		} //end if
		//
		menuitem = menuitem->next;
		yoffset += 8;
	} //end for
	if (menuitem)
	{
		sprintf(menustring+strlen(menustring), "xv %d yv %d string \"... more ...\"", MENUITEMTEXT_XOFFSET, MENUITEMTEXT_YOFFSET + MAX_DISPLAYEDMENUITEMS * 8);
	} //end if
	//keep the show loading image if present
	if (ent->client->showloading)
	{
		sprintf(menustring+strlen(menustring), "xv 104 yv 128 picn loading");
	} //end if
	SendStatusBar(ent, menustring);
} //end of the function DisplayMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void BoundHighlightedItem(menustate_t *menustate, menu_t *menu)
{
	menuitem_t *mi;

	if (menustate->highlighteditem >= menu->nummenuitems)
	{
		menustate->highlighteditem = 0;
	} //end if
	if (menustate->highlighteditem < 0)
	{
		menustate->highlighteditem = menu->nummenuitems-1;
	} //end if
	while((mi = GetMenuItem(menu, menustate->highlighteditem)) != NULL)
	{
		if (mi->type == MI_SEPERATOR) menustate->highlighteditem++;
		else break;
		if (menustate->highlighteditem >= menu->nummenuitems)
		{
			menustate->highlighteditem = 0;
			break;
		} //end if
	} //end while
	if (menustate->firstdisplayeditem >= menu->nummenuitems)
	{
		menustate->firstdisplayeditem = 0;
	} //end if
	if (menustate->firstdisplayeditem < 0)
	{
		menustate->firstdisplayeditem = menu->nummenuitems-1;
	} //end if
} //end of the function BoundHighlightedItem
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int MenuForward(edict_t *ent)
{
	menuitem_t *mi;
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;
	if (!menustate->showmenu) return false;
	menu = GetCurrentMenu(menustate);
	if (!menu) return false;
	BoundHighlightedItem(menustate, menu);
	mi = GetMenuItem(menu, menustate->highlighteditem);
	if (mi)
	{
		menustate->menuproc(ent, mi->id);
		//if the menu items is a submenu
		if (mi->type == MI_SUBMENU && mi->submenu)
		{
			menustate->menuid = mi->submenu->id;
			menustate->highlighteditem = 0;
			menustate->firstdisplayeditem = 0;
			menu = mi->submenu;
		} //end if
	} //end if
	BoundHighlightedItem(menustate, menu);
	menustate->redrawmenu = true;
	return true;
} //end of the function MenuForward
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int MenuBack(edict_t *ent)
{
	int i;
	menuitem_t *mi;
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;
	if (!menustate->showmenu) return false;
	menu = GetCurrentMenu(menustate);
	if (!menu) return false;
	if (menu->parent)
	{
		for (i = 0, mi = menu->parent->firstmenuitem; mi; mi = mi->next, i++)
		{
			if (mi->submenu == menu)
			{
				menustate->highlighteditem = i;
				menustate->firstdisplayeditem = 0;
				if (menu->parent->nummenuitems >= MAX_DISPLAYEDMENUITEMS)
				{
					menustate->firstdisplayeditem = i;
				} //end if
				break;
			} //end if
		} //end for
		menustate->menuid = menu->parent->id;
		menu = menu->parent;
	} //end if
	BoundHighlightedItem(menustate, menu);
	menustate->redrawmenu = true;
	return true;
} //end of the function MenuBack
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int MenuDown(edict_t *ent)
{
	menuitem_t *mi;
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;
	if (!menustate->showmenu) return false;
	menu = GetCurrentMenu(menustate);
	if (!menu) return false;
	menustate->highlighteditem++;
	if (menustate->highlighteditem >= menu->nummenuitems)
	{
		menustate->highlighteditem = 0;
		menustate->firstdisplayeditem = 0;
	} //end if
	while((mi = GetMenuItem(menu, menustate->highlighteditem)) != NULL)
	{
		if (mi->type == MI_SEPERATOR) menustate->highlighteditem++;
		else break;
		if (menustate->highlighteditem >= menu->nummenuitems)
		{
			menustate->highlighteditem = 0;
			menustate->firstdisplayeditem = 0;
			break;
		} //end if
	} //end while
	if (menustate->highlighteditem - menustate->firstdisplayeditem >= MAX_DISPLAYEDMENUITEMS)
	{
		menustate->firstdisplayeditem = menustate->highlighteditem - MAX_DISPLAYEDMENUITEMS + 1;
	} //end if
	BoundHighlightedItem(menustate, menu);
	menustate->redrawmenu = true;
	return true;
} //end of the function MenuDown
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int MenuUp(edict_t *ent)
{
	menuitem_t *mi;
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;
	if (!menustate->showmenu) return false;
	menu = GetCurrentMenu(menustate);
	if (!menu) return false;
	menustate->highlighteditem--;
	if (menustate->highlighteditem < 0)
	{
		menustate->highlighteditem = menu->nummenuitems-1;
		menustate->firstdisplayeditem = menustate->highlighteditem - MAX_DISPLAYEDMENUITEMS + 1;
		if (menustate->firstdisplayeditem < 0) menustate->firstdisplayeditem = 0;
	} //end if
	while((mi = GetMenuItem(menu, menustate->highlighteditem)) != NULL)
	{
		if (mi->type == MI_SEPERATOR) menustate->highlighteditem--;
		else break;
		if (menustate->highlighteditem < 0)
		{
			menustate->highlighteditem = menu->nummenuitems-1;
			menustate->firstdisplayeditem = menustate->highlighteditem - MAX_DISPLAYEDMENUITEMS + 1;
			if (menustate->firstdisplayeditem < 0) menustate->firstdisplayeditem = 0;
			break;
		} //end if
	} //end while
	if (menustate->highlighteditem - menustate->firstdisplayeditem < 0)
	{
		menustate->firstdisplayeditem = menustate->highlighteditem;
	} //end if
	BoundHighlightedItem(menustate, menu);
	menustate->redrawmenu = true;
	return true;
} //end of the function MenuUp
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void DoMenu(edict_t *ent, usercmd_t *ucmd)
{
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;

	if (!menustate->showmenu) return;
	menu = GetCurrentMenu(menustate);
	if (!menu) return;
	//
	if ((ucmd->forwardmove || ucmd->sidemove)
			&& (menustate->lastchange_time + 0.21 < level.time))
	{
		menustate->lastchange_time = level.time;
		//if activate menu item
		if (ucmd->sidemove > MENUCHANGE_MOVE) MenuForward(ent);
		//if go back to parent menu
		else if (ucmd->sidemove < -MENUCHANGE_MOVE) MenuBack(ent);
		//if go to prev menu item
		else if (ucmd->forwardmove > MENUCHANGE_MOVE) MenuUp(ent);
		//if go to next menu item
		else if (ucmd->forwardmove < -MENUCHANGE_MOVE) MenuDown(ent);
	} //end if
	else
	{
		if (!(level.framenum & 15))
		{
			menustate->redrawmenu = true;
		} //end if
	} //end else
	menu = GetCurrentMenu(menustate);
	BoundHighlightedItem(menustate, menu);
	//clear the velocity so the menu option pointer won't get jumpy
	VectorClear(ent->velocity);
	ent->client->ps.pmove.pm_type = PM_DEAD;
	//clear the use and fire buttons
	ucmd->buttons &= ~(BUTTON_ATTACK | BUTTON_USE);
	ucmd->forwardmove = 0;
	ucmd->sidemove = 0;
	ucmd->upmove = 0;
	//NOTE: this is necessary for the chasecam!!
	//      otherwise the camera keeps sinking
	ent->client->ps.pmove.gravity = 0;
} //end of the function DoMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void ShowMenu(edict_t *ent)
{
	menustate_t *menustate;
	menu_t *menu;

	menustate = &ent->client->menustate;
	if (menustate->redrawmenu)
	{
		menu = GetCurrentMenu(menustate);
		DisplayMenu(ent, menu, menustate->highlighteditem, menustate->firstdisplayeditem);
		menustate->redrawmenu = false;
	} //end if
	/*else if (cl_ent->client->menustate.removemenu)
	{
extern char *dm_statusbar;			//g_spawn.c
extern char *single_statusbar;	//g_spawn.c
		//clear the menu by drawing the status bar
		if (deathmatch->value) SendStatusBar(cl_ent, dm_statusbar);
		else SendStatusBar(cl_ent, single_statusbar);
		menustate->removemenu = false;
	} //end else*/
} //end of the function ShowMenus
