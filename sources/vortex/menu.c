#include "g_local.h"

// v3.12
qboolean InMenu (edict_t *ent,int index, void (*optionselected)(edict_t *ent,int option))
{
	// don't need to be here if there's no menu open!
	if (!ent->client->menustorage.menu_active)
		return false;
	// check for index, only necessary if 2 funcs are exactly the same
	if (index && (ent->client->menustorage.menu_index != index))
		return false;
	// check if we're using the right handler
	return (ent->client->menustorage.optionselected == optionselected);
}

void addlinetomenu (edict_t *ent,char *line,int option)
{
	if (ent->client->menustorage.menu_active) // checks to see if the menu is showing
		return;
	if (ent->client->menustorage.num_of_lines >= MAX_LINES) // checks to see if there is space
		return;
	ent->client->menustorage.num_of_lines++; // adds to the number of lines that can be seen
	ent->client->menustorage.messages[ent->client->menustorage.num_of_lines].msg = V_Malloc (strlen(line)+1, TAG_GAME);
	strcpy(ent->client->menustorage.messages[ent->client->menustorage.num_of_lines].msg, line);
	ent->client->menustorage.messages[ent->client->menustorage.num_of_lines].option = option;
}

void clearmenu(edict_t *ent)
{
	int		i = 0;

	if (ent->client->menustorage.menu_active) // checks to see if the menu is showing
		return;

	for (i = 0; i < MAX_LINES; i++){
		ent->client->menustorage.messages[i].option = 0;
		if (ent->client->menustorage.messages[i].msg != NULL){
			V_Free (ent->client->menustorage.messages[i].msg);
			//GHz START
			ent->client->menustorage.messages[i].msg = NULL;
			//GHz END
		}
	}
	//GHz START
	// keep record of last known menu for multi-purpose menus that have more than one handler
	if (ent->client->menustorage.optionselected)
		ent->client->menustorage.oldmenuhandler = ent->client->menustorage.optionselected;
	if (ent->client->menustorage.currentline)
		ent->client->menustorage.oldline = ent->client->menustorage.currentline;
	//GHz END
	ent->client->menustorage.optionselected = NULL;
	ent->client->menustorage.currentline = 0;
	ent->client->menustorage.num_of_lines = 0;
	ent->client->menustorage.menu_index = 0; // 3.45
}

void tradeconfirmation_handler (edict_t *ent, int option);
void itemmenu_handler (edict_t *ent, int option);
void setmenuhandler(edict_t *ent, void (*optionselected)(edict_t *ent,int option))
{
	ent->client->menustorage.optionselected=optionselected;
}

//GHz START
int topofmenu (edict_t *ent)
{
	int		i, option;

	for (i = 0; i < MAX_LINES; i++){
		option = ent->client->menustorage.messages[i].option;
		if (option != 0 && option != MENU_GREEN_CENTERED && option != MENU_WHITE_CENTERED && option != MENU_GREEN_LEFT)
			break;
	}
	return i;
}

int bottomofmenu (edict_t *ent)
{
	int		i, option;

	for (i = MAX_LINES-1; i > 0; i--){
		option = ent->client->menustorage.messages[i].option;
		if (option != 0 && option != MENU_GREEN_CENTERED && option != MENU_WHITE_CENTERED && option != MENU_GREEN_LEFT)
			break;
	}
	return i;
}
//GHz END

void menudown(edict_t *ent)
{
	int	option;

	// 3.65 avoid menu overflows
	if (ent->client->menu_delay > level.time)
		return;
	ent->client->menu_delay = level.time + MENU_DELAY_TIME;

	do
	{
		if (ent->client->menustorage.currentline < ent->client->menustorage.num_of_lines)
			ent->client->menustorage.currentline++;
		else
			ent->client->menustorage.currentline = topofmenu(ent);
		option = ent->client->menustorage.messages[ent->client->menustorage.currentline].option;
	}
	while (option == 0 || option == MENU_WHITE_CENTERED || option == MENU_GREEN_CENTERED 
		|| option == MENU_GREEN_RIGHT || option == MENU_GREEN_LEFT);
	showmenu(ent);
}

void menuup(edict_t *ent)
{
	int	option;

	// 3.65 avoid menu overflows
	if (ent->client->menu_delay > level.time)
		return;
	ent->client->menu_delay = level.time + MENU_DELAY_TIME;

	do
	{
		if (ent->client->menustorage.currentline > topofmenu(ent))
			ent->client->menustorage.currentline--;
		else
			ent->client->menustorage.currentline = bottomofmenu(ent);
		option = ent->client->menustorage.messages[ent->client->menustorage.currentline].option;
	}
	while (option == 0 || option == MENU_WHITE_CENTERED || option == MENU_GREEN_CENTERED 
		|| option == MENU_GREEN_RIGHT || option == MENU_GREEN_LEFT);
	showmenu(ent);
}

/*
=============
menuselect
calls the menu handler with the currently selected option
=============
*/
void menuselect(edict_t *ent)
{
	int i;
	//GHz START
	if (debuginfo->value)
		gi.dprintf("DEBUG: menuselect()\n");
	//GHz END

	i = ent->client->menustorage.messages[ent->client->menustorage.currentline].option;
	closemenu(ent); // close the menu as a selection has been made
	// call the menu handler with the current option value
	ent->client->menustorage.optionselected(ent, i);
//	gi.dprintf("menuselect() at line %d\n", ent->client->menustorage.currentline);
}

/*
=============
initmenu
clears all menus for this client
=============
*/
void initmenu (edict_t *ent)
{
	int i;

	for (i = 0;i < MAX_LINES;i++){
		ent->client->menustorage.messages[i].msg = NULL;
		ent->client->menustorage.messages[i].option = 0;
	}
	

	ent->client->menustorage.menu_active = false;
	ent->client->menustorage.displaymsg = false;
	ent->client->menustorage.optionselected = NULL;
	ent->client->menustorage.currentline = 0;
	ent->client->menustorage.num_of_lines = 0;
	ent->client->menustorage.menu_index = 0; // 3.45
}

/*
=============
ShowMenu
used every frame to display a player's menu
=============
*/
void showmenu(edict_t *ent)
{
	int		i, j;  // general purpose integer for temporary use :)
	char	finalmenu[1024]; // where the control strings for the menu are assembled.
	char	tmp[80]; // temporary storage strings
	int		center;

	if (debuginfo->value)
		gi.dprintf("DEBUG: showmenu()\n");
	if ((ent->client->menustorage.num_of_lines < 1) || (ent->client->menustorage.num_of_lines > MAX_LINES))
	{
		gi.dprintf("WARNING: showmenu() called with %d lines\n", ent->client->menustorage.num_of_lines);
		return;
	}

	// copy menu bg control strings to our final menu
	sprintf (finalmenu, "xv 32 yv 8 picn inventory ");
	// get y coord of text based on the number of lines we want to create
	// this keeps the text vertically centered on our screen
	j = 24 + LINE_SPACING*(ceil((float)(20-ent->client->menustorage.num_of_lines) / 2));
	// cycle through all lines and add their control codes to our final menu
	// nothing is actually displayed until the very end
	for (i = 1;i < (ent->client->menustorage.num_of_lines + 1); i++)
	{
		// get x coord of screen based on the length of the string for
		// text that should be centered
		center = 216/2 - strlen(ent->client->menustorage.messages[i].msg)*4 + 52;
		if (ent->client->menustorage.messages[i].option == 0)// print white text
		{
			sprintf(tmp,"xv 52 yv %i string \"%s\" ",j,ent->client->menustorage.messages[i].msg);
		}
		else if (ent->client->menustorage.messages[i].option == MENU_GREEN_LEFT)// print green text
		{
			sprintf(tmp,"xv 52 yv %i string2 \"%s\" ",j,ent->client->menustorage.messages[i].msg);
		}
		else if (ent->client->menustorage.messages[i].option == MENU_GREEN_CENTERED)// print centered green text
		{
			sprintf(tmp,"xv %d yv %i string2 \"%s\" ", center, j, ent->client->menustorage.messages[i].msg);
		}
		else if (ent->client->menustorage.messages[i].option == MENU_WHITE_CENTERED)// print centered white text
		{
			sprintf(tmp,"xv %d yv %i string \"%s\" ", center, j, ent->client->menustorage.messages[i].msg);
		}
		else if (ent->client->menustorage.messages[i].option == MENU_GREEN_RIGHT)// print right-aligned green text
		{
			center = 216 - strlen(ent->client->menustorage.messages[i].msg)*8 + 52;
			sprintf(tmp,"xv %d yv %i string2 \"%s\" ", center, j, ent->client->menustorage.messages[i].msg);
		}
		else if (i == ent->client->menustorage.currentline)
		{
			sprintf(tmp,"xv 52 yv %i string2 \">> %s\" ",j,ent->client->menustorage.messages[i].msg);
		}
		else 
			sprintf(tmp,"xv 52 yv %i string \"   %s\" ",j,ent->client->menustorage.messages[i].msg); 
		// add the control string to our final menu space
		strcat(finalmenu, tmp);
		j += LINE_SPACING;
	}

	// actually display the final menu
	ent->client->menustorage.menu_active = true;
	ent->client->menustorage.displaymsg = false;
	ent->client->showinventory = false;
	ent->client->showscores = true;
	gi.WriteByte (svc_layout);
	gi.WriteString (finalmenu);
	gi.unicast (ent, true);

	//gi.dprintf("showmenu() at line %d\n", ent->client->menustorage.currentline);
}

void closemenu (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: closemenu()\n");

	clearmenu(ent); // reset all menu variables and strings
	ent->client->showscores = false;
	ent->client->menustorage.menu_active = false;
	ent->client->menustorage.displaymsg = false;
	ent->client->showinventory = false;
	ent->client->trading = false; // done trading
}

/*
=============
clearallmenus
cycles thru all clients and resets their menus
=============
*/
void clearallmenus (void)
{
	int i;
	edict_t *ent;

	for (i=0 ; i < game.maxclients ; i++)
	{
		ent = g_edicts + 1 + i;
		closemenu(ent);
	}
}

/*
=============
ShowMenu
returns false if the client has another menu open
=============
*/
qboolean ShowMenu (edict_t *ent) 
{
	if (ent->client->showscores || ent->client->showinventory
		|| ent->client->menustorage.menu_active || ent->client->pers.scanner_active)
		return false;
	return true;
}
