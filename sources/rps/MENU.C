#include "g_local.h"
/************* MENU.C
Menu Diagram (client->menulist) 
    (prev)        (prev)                      (prev)              (next)
null ---- menulist ----  menulink1  ---------------  menulink2  -------------  null
           |it    (next)  |(it)     (next)            |(it)     (next)           
          null           menu1  -----  null          menu2  -----  null           
                          |(next) (it)                |(next) (it)                
                    (prev)|                           |                           
                         itemlink1 --  menuitem1     itemlink1 --  menuitem1      
                          |(next) (it)  |-itemtext    |(next) (it)  |-itemtext    
                          |             |-ItemSelect  |             |-ItemSelect  
                    (prev)|                           |                           
          		       itemlink2 --  menuitem2       itemlink2 --  menuitem2       
                          |             |-itemtext    |             |-itemtext    
          		       null             |-ItemSelect null           |-ItemSelect   
                                                    

******************/

#ifndef ARENA

/*******
add_to_queue
*******/
void add_to_queue(arena_link_t *t, arena_link_t *que)
{
        while(que->next)
                que=que->next;
        que->next = t;
        t->prev = que;
        t->next = NULL;
}

/*******
remove_from_queue

if NULL is given as first parameter, top list item is popped off

item that is removed is returned, or NULL if not found
*******/
arena_link_t *remove_from_queue(arena_link_t *t, arena_link_t *que)
{
        arena_link_t *got = NULL;

        if(!t) t = que->next;
        if(!t) return(NULL);
        t->prev->next = t->next;
        if(t->next)
                t->next->prev = t->prev;

        return(t);
}

/*******
count_queue
*******/
int count_queue(arena_link_t *que)
{
        int i = 0;

        while(que->next) {
                que=que->next;
                i++;
        }

        return(i);
}

#endif 

arena_link_t *CreateMenu(edict_t *ent, char *name)
{
	arena_link_t	*menu;
	arena_link_t	*menulink;

	menu = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	menulink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	menulink->it = menu;
	menu->it = name;
	menu->next = menu->prev = NULL;
	return menulink;
}

void AddMenuItem(arena_link_t *menulink, char *itemtext, char *valuetext, int value, void *Callback)
{
	arena_link_t	*itemlink;
	menu_item_t	*iteminfo;

	itemlink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	iteminfo = gi.TagMalloc(sizeof(menu_item_t), TAG_LEVEL);

	iteminfo->itemtext = itemtext;
	iteminfo->valuetext = valuetext;
	iteminfo->itemvalue = value;
	iteminfo->ItemSelect = Callback; //ItemSelect;
	itemlink->it = iteminfo;
	add_to_queue(itemlink, menulink->it); //it is the actualy menu

}

void FinishMenu(edict_t *ent, arena_link_t *menulink)
{
	ent->client->curmenulink = menulink;

	ent->client->selected = ((arena_link_t *)menulink->it)->next; //it is menu, it->next is first item
	ent->client->showmenu = true;
	add_to_queue(menulink, &ent->client->menulist);

	switch(ent->client->arenaID)
	{
	case 0: if(ent->client->resp.team==ROCK)
			{
				MenuNext(ent);
				return;
			}
			break;
	case 1: if(ent->client->resp.team==PAPER)
			{
				MenuNext(ent);
				return;
			}
			break;
	case 2: if(ent->client->resp.team==SCISSORS)
			{
				MenuNext(ent);
				return;
			}
			break;
	default: 
			break;
	}

	DisplayMenu(ent);
}

void PrintMenuItem(menu_item_t *menuitem)
{
	gi.bprintf(PRINT_HIGH, "  %s %s %d\n", menuitem->itemtext, menuitem->valuetext, menuitem->itemvalue);
}

void PrintMenu(arena_link_t *menulink)
{
	arena_link_t	*que;

	que=menulink->it;
	gi.bprintf(PRINT_HIGH, "%s\n", ((arena_link_t *)menulink->it)->it);
	while(que->next) {
                que=que->next;
                PrintMenuItem(que->it);
        }

 
}

void PrintMenuQueue(edict_t *ent)
{
	arena_link_t	*que;

	que=&ent->client->menulist;
    while(que->next) {
                que=que->next;
                PrintMenu(que);
        }
}

char *LoPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] > 127)
			text[i]=(byte)text[i]-128;

	return text;
}

char *HiPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] <= 127)
			text[i]=(byte)text[i]+128;
	return text;
}

#define FANCY_MENUS

void SendStatusBar(edict_t *ent, char *bar)
{
	gi.WriteByte (0x0D); //configstring
	gi.WriteShort(5); //status bar index
	gi.WriteString (bar);
	gi.unicast (ent, false);
}

extern char *dm_statusbar; //in g_spawn.c
extern char *single_statusbar;

void DisplayFancyMenu(edict_t *ent)
{
	arena_link_t *cur, *menu, *selected;
	char my_statusbar[1400];
	char tempitem[MAXITEMSIZE];
	char *pos;
	int i,y;

	if (!ent->client->showmenu) //clear the old one
	{
		SendStatusBar(ent, rps_statusbar);

		return;
	}

	menu=ent->client->curmenulink->it;
	selected=ent->client->selected;

	sprintf (my_statusbar, 
		"xv 32 yv 8 picn inventory ");			// background

	pos = my_statusbar+strlen(my_statusbar);

	sprintf(pos,
		"xv 202 yv 12 string2 \"%s\" ",		// menu 
		"Menu");

	pos = my_statusbar+strlen(my_statusbar);

	sprintf(pos,
		"xv 0 yv 24 cstring2 \"%s\" ",		// menu title
		menu->it);

	pos = my_statusbar+strlen(my_statusbar);

//if we have too many to display
	i=count_queue(menu) - count_queue(selected);

	if (i>18)
	{
		cur=selected;
		do
		{
			cur=cur->prev; 
			i--;
		} while (cur != menu && i % 18 != 0);
		
		sprintf(pos, "xv 50 yv 32 string2 \"(More)\" ");
		pos = my_statusbar+strlen(my_statusbar);
	} else
		cur = menu;

	y = 32;
	i=0;
	while(cur->next && i<18)  //only display 18 items
		{ //add items
				cur=cur->next;
				y+=8;
				i++;
				tempitem[0]='\0';

                if (cur == selected) 
				{
					strcat(tempitem, "\15");
					strcat(tempitem, LoPrint(((menu_item_t *)cur->it)->itemtext));
					if (((menu_item_t *)cur->it)->valuetext)
						strcat(tempitem, ((menu_item_t *)cur->it)->valuetext);
				}
				else
				{
					strcat(tempitem, " ");

					/* MIKE */
					
					// This used to be a HiPrint.  For some reason, that caused crashes on Linux.  Will look into it.
					
					strcat(tempitem, LoPrint(((menu_item_t *)cur->it)->itemtext));
					/* MIKE */

					if (((menu_item_t *)cur->it)->valuetext)
						strcat(tempitem,((menu_item_t *)cur->it)->valuetext);
				}

				LoPrint(((menu_item_t *)cur->it)->itemtext); //reset it back to the normal state
				if (((menu_item_t *)cur->it)->itemvalue >= 0)
					sprintf(tempitem + strlen(tempitem), "%d", ((menu_item_t *)cur->it)->itemvalue);
				sprintf(pos,"xv 50 yv %d string2 \"%s\" ",		// each menu item
					y, tempitem);
				pos = my_statusbar+strlen(my_statusbar);
        }

	if (i==18 && cur->next) //if there are more
		sprintf(pos, "xv 50 yv %d string2 \"(More)\" ", y+8);
	SendStatusBar(ent, my_statusbar);
}

void DisplayMenu(edict_t *ent)
{
	char	string[1400];
	int		total;
	arena_link_t *cur, *menu, *selected;

#ifdef FANCY_MENUS
	DisplayFancyMenu(ent);
	return;
#endif

	if (!ent->client->showmenu) //clear the old one
	{
		gi.centerprintf(ent, "");
		return;
	}
	menu=ent->client->curmenulink->it;
	selected=ent->client->selected;
	
	total = count_queue(menu); // get the menu count
	string[0] = 0;
	strcat(string, HiPrint(menu->it)); //print title
	LoPrint(menu->it); //reset it back to the normal state
	strcat(string, "\n"); //space b/t title and items

	cur = menu;
	while(cur->next) 
		{ //add items
				cur=cur->next;
				strcat(string, "\n");
                if (cur == selected)
					strcat(string, "*");
				strcat(string, ((menu_item_t *)cur->it)->itemtext);
				if (((menu_item_t *)cur->it)->valuetext)
					strcat(string, ((menu_item_t *)cur->it)->valuetext);
				if (((menu_item_t *)cur->it)->itemvalue >= 0)
					sprintf(string + strlen(string), "%d", ((menu_item_t *)cur->it)->itemvalue);

				
        }


	gi.centerprintf(ent, "%s", string);
}



void MenuNext(edict_t *ent)
{
	if (ent->client->selected->next)
	{
		ent->client->selected=ent->client->selected->next;
		ent->client->arenaID++;
	}
	else
	{
		ent->client->selected=((arena_link_t *)ent->client->curmenulink->it)->next; //first item
		ent->client->arenaID=0;
	}

	/* MIKE */ // Skip the team you're on.
	switch(ent->client->arenaID)
	{
	case 0: if(ent->client->resp.team==ROCK)
			{
				MenuNext(ent);
				return;
			}
			break;
	case 1: if(ent->client->resp.team==PAPER)
			{
				MenuNext(ent);
				return;
			}
			break;
	case 2: if(ent->client->resp.team==SCISSORS)
			{
				MenuNext(ent);
				return;
			}
			break;
	default: break;
	}
	/* MIKE */

	DisplayMenu(ent);
}

void MenuPrev(edict_t *ent)
{
	if (ent->client->selected->prev->prev)
	{
		ent->client->selected=ent->client->selected->prev;
		ent->client->arenaID--;
	}
	else
		while (ent->client->selected->next)
		{
			ent->client->selected=ent->client->selected->next; //go to the last
			ent->client->arenaID++;
		}

	/* MIKE */ // Skip the team you're on.
	switch(ent->client->arenaID)
	{
	case 0: if(ent->client->resp.team==ROCK)
			{
				MenuPrev(ent);
				return;
			}
			break;
	case 1: if(ent->client->resp.team==PAPER)
			{
				MenuPrev(ent);
				return;
			}
			break;
	case 2: if(ent->client->resp.team==SCISSORS)
			{
				MenuPrev(ent);
				return;
			}
			break;
	default: break;
	}
	/* MIKE */

	DisplayMenu(ent);
}
	

void UseMenu(edict_t *ent, int key) //key=1 invuse, key=0 invdrop
{
	arena_link_t	*que;

	que=ent->client->curmenulink;
	
	if (((menu_item_t *) ent->client->selected->it)->ItemSelect(ent,que, ent->client->selected, key)) //if the callback didnt return 0, leave
	{
		DisplayMenu(ent);
		return;
	}
	
	remove_from_queue(que, &ent->client->menulist);

	que=&ent->client->menulist; //go to front of list

	while (que->next)
		que=que->next; //find last item
	
	if (que->it) //if there  menu
	{
		
		ent->client->curmenulink = que;
		ent->client->selected=((arena_link_t *)ent->client->curmenulink->it)->next; //first menu item link
		
	}
	else  
	{//modified for arena, this way we can turn the menu off and not have it turn back on
		ent->client->curmenulink = NULL;
		ent->client->showmenu = 0;
	}


	//we SHOULD free the ram!!
//modified for arena
//	ent->client->showmenu = ent->client->curmenulink ? true : false;
	DisplayMenu(ent);
}


void MenuThink(edict_t *ent)
{
#ifndef FANCY_MENUS
	if (level.framenum % 15 == 0 && ent->client->showmenu)
		DisplayMenu(ent);
#endif

}

/**************
Example Stuff
***************/

int MySelect(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{

	gi.bprintf(PRINT_HIGH, "menu item %s selected by %s\n", ((menu_item_t *)selected->it)->itemtext, ent->client->pers.netname);
	return 0;
}

int MySelect2(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (key) //inc for enter, dec for '
		((menu_item_t *)selected->it)->itemvalue++;
	else
		((menu_item_t *)selected->it)->itemvalue--;

	if (((menu_item_t *)selected->it)->itemvalue==-1) //if we went past zero
		((menu_item_t *)selected->it)->itemvalue=0;
	return 1;

}

int MySelect3(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	char fl[8], tl[8];
	arena_link_t *itemlink;

	itemlink=((arena_link_t *)(menulink->it) )->next; //first menuitemlink on the current menu
	sprintf(fl, "%d", ((menu_item_t*) (itemlink->it))->itemvalue);
	itemlink=itemlink->next; //next menuitemlink
	sprintf(tl, "%d", ((menu_item_t*) (itemlink->it))->itemvalue);

	gi.bprintf(PRINT_HIGH, "Fraglimit is now %s. Timelimit is now %s\n", fl, tl);
	gi.cvar_set("fraglimit",fl);
	gi.cvar_set("timelimit",tl);
	return 0;
}
