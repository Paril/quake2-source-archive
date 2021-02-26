// CUSTOM SOURCE FILE

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

	if (!ent->client)
		return 0;

	if (ent->client->pers.fancymenu)
	{
		if (ent->client->ping > 250)
			ent->client->pers.fancymenu = false;
		else
			ent->client->pers.fancymenu = true;
	}

ent->client->pers.fancymenu = false;

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

	if(menulink)
	{
		itemlink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
		iteminfo = gi.TagMalloc(sizeof(menu_item_t), TAG_LEVEL);

		iteminfo->itemtext = itemtext;
		iteminfo->valuetext = valuetext;
		iteminfo->itemvalue = value;
		iteminfo->ItemSelect = Callback; //ItemSelect;
		itemlink->it = iteminfo;
		add_to_queue(itemlink, menulink->it); //it is the actualy menu
	}
}

void FinishMenu(edict_t *ent, arena_link_t *menulink)
{

	if (!ent->client)
		return;

	if(menulink)
	{
		ent->client->curmenulink = menulink;
		ent->client->selected = ((arena_link_t *)menulink->it)->next; //it is menu, it->next is first item
		ent->client->showmenu = true;
		add_to_queue(menulink, &ent->client->menulist);
		DisplayMenu(ent);
	}
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

	if (!ent->client)
		return;

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
	/*
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] <= 127)
			text[i]=(byte)text[i]+128;
	*/
	i = 0;
	while (text[i] != 0) {
	  text[i++] |= 128;
	}
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

//extern char *dm_statusbar; //in g_spawn.c
//extern char *single_statusbar;

/*
char *new_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

//tech
"yb -129 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -102 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
//"xr -62 "
"xr -78 "
"num 3 18 "

//joined overlay
"if 22 "
  "yb -104 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -75 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
//"xr -62 "
"xr -78 "
"num 3 20 "
"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -24 "
  "pic 21 "
"endif "

// id view state
"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "
;
*/


char *new_statusbar =

// health
"yb -25 "
"xl 0 "
"hnum "
"xl 50 "
"pic 0 "

// armor
"yb -50 "
"if 4 "
 "xl 0 "
 "rnum "
 "xl 50 "
 "pic 4 "
"endif "

// cells
"xl 0 "
"yb -100 "
"num 3 29 "
"xl 50 "
"pic 28 "

// grenades
"yb -125 "
"xl 0 "
"num 3 31 "
"xl 50 "
"pic 30 "

// ammo
"yb -150 "
"xl 0 "
"anum "
"xl 50 "
"pic 2 "

// selected item
"yb -24"
"if 6 "
 "xl 100 "
 "pic 6 "
"endif "

"yb	-50 "
// picked up item
"if 7 "
 "xl	100 "
 "pic 7 "
 "xl	126 "
 "yb	-42 "
 "stat_string 8 "
 "yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// red team
"yb -52 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -78 "
"num 3 18 "

//joined overlay
"if 22 "
  "yb -54 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -25 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -78 "
"num 3 20 "
"if 23 "
  "yb -27 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -24 "
  "pic 21 "
"endif "

// id view state
"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "
;


void KillFancyMenu(edict_t *ent)
{
}

void DisplayFancyMenu(edict_t *ent)
{
	arena_link_t *cur, *menu, *selected;
	char my_statusbar[1400];
	char tempitem[MAXITEMSIZE];
	char *pos;
	int i,y;

	if (!ent->client)
		return;

	if (!ent->client->showmenu) //clear the old one
	{
		SendStatusBar(ent, new_statusbar);
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
		(char *) menu->it);

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
					strcat(tempitem, HiPrint(((menu_item_t *)cur->it)->itemtext));
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

	if (!ent->client)
		return;

	if (ent->client->showmenu && !ent->client->pers.fancymenu)
	{
		if (level.time > ent->menudisplay)
			ent->menudisplay = level.time + 30.0;
		else
			return;
	}

	ent->client->showscores = false;

//#ifdef FANCY_MENUS
	if (ent->client->pers.fancymenu)
	{
		DisplayFancyMenu(ent);
		return;
	}
//#endif

	if (!ent->client->showmenu) //clear the old one
	{
		gi.centerprintf(ent, "");
		return;
	}
	menu=ent->client->curmenulink->it;
	selected=ent->client->selected;
	
	total = count_queue(menu); // get the menu count
	//string[0] = 0;
	//strcat(string, HiPrint(menu->it)); //print title
	strcpy(string, menu->it);
	//LoPrint(menu->it); //reset it back to the normal state
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
	if (!ent->client)
		return;

	if (ent->client->selected->next)
		ent->client->selected=ent->client->selected->next;
	else
		ent->client->selected=((arena_link_t *)ent->client->curmenulink->it)->next; //first item

	if (ent->client->showmenu && !ent->client->pers.fancymenu)
	{
		if (level.time < ent->menudisplay)
		{
			ent->menudisplay = level.time + 1.0;
			return;
		}
	}

	DisplayMenu(ent);
}

void MenuPrev(edict_t *ent)
{
	if (!ent->client)
		return;

	if (ent->client->selected->prev->prev)
		ent->client->selected=ent->client->selected->prev;
	else
		while (ent->client->selected->next)
			ent->client->selected=ent->client->selected->next; //go to the last

	if (ent->client->showmenu && !ent->client->pers.fancymenu)
	{
		if (level.time < ent->menudisplay)
		{
			ent->menudisplay = level.time + 1.0;
			return;
		}
	}

	DisplayMenu(ent);
}
	

void UseMenu(edict_t *ent, int key, qboolean isauto) //key=1 invuse, key=0 invdrop
{
	arena_link_t	*que;

	if (!ent->client)
		return;

	que=ent->client->curmenulink;
	
	if (!isauto)
	{
		if (((menu_item_t *) ent->client->selected->it)->ItemSelect(ent,que, ent->client->selected, key)) //if the callback didnt return 0, leave
		{
			DisplayMenu(ent);
			return;
		}
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
	{
		//modified for arena, this way we can turn the menu off and not have it turn back on
		ent->client->curmenulink = NULL;
		ent->client->showmenu = 0;
	}


	//we SHOULD free the ram!!
	//modified for arena
	//	ent->client->showmenu = ent->client->curmenulink ? true : false;
	
	ent->menudisplay = 0.1;

	DisplayMenu(ent);
}


void MenuThink(edict_t *ent)
{
	if (!ent->client)
		return;

	if(!ent->client->pers.fancymenu)
	{
		if (level.framenum % 15 == 0 && ent->client->showmenu)
			DisplayMenu(ent);
	}
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


#ifndef ARENA
void menu_test_f(edict_t *ent)
{
	arena_link_t *menu1l, *menu2l, *menu3l;
	char *menuitem;
	int i;

	menu1l=CreateMenu(ent, "More Options");
	AddMenuItem(menu1l, "menu1item1",NULL, -1,  &MySelect);
	AddMenuItem(menu1l, "menu1item2",NULL, -1, &MySelect);
	AddMenuItem(menu1l, "menu1item3",NULL, -1, &MySelect);
	AddMenuItem(menu1l, "menu1item4",NULL, -1, &MySelect);
	FinishMenu(ent, menu1l);	

	menu2l=CreateMenu(ent, "Options");
	AddMenuItem(menu2l, "Fraglimit - ",NULL, fraglimit->value, &MySelect2);
	AddMenuItem(menu2l, "Timelimit - ",NULL, timelimit->value, &MySelect2);
	AddMenuItem(menu2l, "OK", NULL, -1, &MySelect3);
	FinishMenu(ent, menu2l);

/***********
Demonstrates a Large scrolling Menu
*************/

	menu3l=CreateMenu(ent, "Big List");
	for (i=0; i<30; i++)
	{
		menuitem=malloc(MAXITEMSIZE);
		sprintf(menuitem, "Menuitem %d",i);
		AddMenuItem(menu3l, menuitem, NULL, -1, &MySelect);
	}
	FinishMenu(ent,menu3l);


//	PrintMenuQueue(ent);

	
}
#endif
