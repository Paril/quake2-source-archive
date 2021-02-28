#include "g_local.h"

void ShowHelpMenu(edict_t *ent, int lastpick);

//************************************************************************************************
//		**SECONDARY HELP MENU HANDLER**
//************************************************************************************************

void ShowSecondaryMenu_handler(edict_t *ent, int option)
{
	if (option != 666)
	{
		ShowHelpMenu(ent, option);
		return;
	}
	closemenu(ent);
}

//************************************************************************************************
//		**MODE HELP MENU**
//************************************************************************************************

void ShowModeHelpMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Game modes:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "  Player Vs. Player  (PvP)", 0);
	addlinetomenu(ent, "  Player Vs. Monters (PvM)", 0);
	addlinetomenu(ent, "  Domination         (DOM)", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Domination is a variation", 0);
	addlinetomenu(ent, " of capture the flag.", 0);
	addlinetomenu(ent, " Protect your flag runner!", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 80);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 11;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**VOTE HELP MENU**
//************************************************************************************************

void ShowVoteHelpMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Voting", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " To vote for the next game", 0);
	addlinetomenu(ent, " and map, use the command:", 0);
	addlinetomenu(ent, " vote. You must vote for", 0);
	addlinetomenu(ent, " both a game mode and ", 0);
	addlinetomenu(ent, " a map in order for your", 0);
	addlinetomenu(ent, " vote to be recorded.", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 70);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 10;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**TRADE HELP MENU**
//************************************************************************************************

void ShowTradeHelpMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Trading", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " To start a trade, use the", 0);
	addlinetomenu(ent, " command: trade.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Select up to 3 items", 0);
	addlinetomenu(ent, " for the trade queue,", 0);
	addlinetomenu(ent, " then select 'accept'", 0);
	addlinetomenu(ent, " to complete the trade.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "cmd: trade [on/off]", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 60);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 13;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**ITEMS HELP MENU**
//************************************************************************************************

void ShowItemsHelpMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Item types:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Health potions", MENU_GREEN_LEFT);
	addlinetomenu(ent, "  Heals 50% max health.", 0);
	addlinetomenu(ent, " Holy water", MENU_GREEN_LEFT);
	addlinetomenu(ent, "  Removes all curses.", 0);
	addlinetomenu(ent, " Tballs", MENU_GREEN_LEFT);
	addlinetomenu(ent, "  Teleport self/others.", 0);
	addlinetomenu(ent, " Gravity Boots", MENU_GREEN_LEFT);
	addlinetomenu(ent, "  Jump great distances.", 0);
	addlinetomenu(ent, " Fire Resistant Clothing", MENU_GREEN_LEFT);
	addlinetomenu(ent, "  Resists burns.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " To use: cmd use <item>", 0);
	addlinetomenu(ent, " Some items are automatic.", 0);
	addlinetomenu(ent, " Tballs can not be dropped.", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 50);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 18;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**RUNES HELP MENU**
//************************************************************************************************

void ShowRuneTypes(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Rune types:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Ability runes   (blue)", 0);
	addlinetomenu(ent, " Weapon runes    (red)", 0);
	addlinetomenu(ent, " Combo runes     (purple)", 0);
	addlinetomenu(ent, " Class runes     (cyan)", 0);
	addlinetomenu(ent, " Unique runes    (yellow)", 0);
	addlinetomenu(ent, " Special Uniques (green)", 0);
	
	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Previous", 40);
	addlinetomenu(ent, " Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 10;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**COMMANDS HELP MENU**
//************************************************************************************************

void ShowCommandsMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Standard Vortex Commands:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " set vrx_password <pass> u", 0);
	addlinetomenu(ent, " email <address>", 0);
	addlinetomenu(ent, " upgrade_weapon", 0);
	addlinetomenu(ent, " upgrade_ability", 0);
	addlinetomenu(ent, " vrxarmory", 0);
	addlinetomenu(ent, " vrxrespawn", 0);
	addlinetomenu(ent, " vrxinfo", 0);
	addlinetomenu(ent, " rune", 0);
	addlinetomenu(ent, " vote", 0);
	addlinetomenu(ent, " vrxid", 0);
	addlinetomenu(ent, " use tball [self]", 0);
	addlinetomenu(ent, " togglesecondary", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 20);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 16;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**BASICS HELP MENU**
//************************************************************************************************

void ShowBasicsMenu(edict_t *ent)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " To find out more about the", 0);
	addlinetomenu(ent, " basics of Vortex, go", 0);
	addlinetomenu(ent, " visit our website at:", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " www.project-vortex.com", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " There is too much info", 0);
	addlinetomenu(ent, " to show in this menu.", 0);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous", 10);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowSecondaryMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 11;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**MAIN HELP MENU**
//************************************************************************************************

void ShowHelpMenu_handler(edict_t *ent, int option)
{
	if (option == 666)
	{
		closemenu(ent);
		return;
	}
	else
	{
		switch(option)
		{
		case 10:	ShowBasicsMenu(ent);	return;
		case 20:	ShowCommandsMenu(ent);	return;
		case 30:	PrintCommands(ent);		return;
		case 40:	ShowRuneTypes(ent);		return;
		case 50:	ShowItemsHelpMenu(ent);	return;
		case 60:	ShowTradeHelpMenu(ent);	return;
		case 70:	ShowVoteHelpMenu(ent);	return;
		case 80:	ShowModeHelpMenu(ent);	return;
		}
	}
}

//************************************************************************************************

void ShowHelpMenu(edict_t *ent, int lastpick)
{
	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	addlinetomenu(ent, "Choose a help topic:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);

	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, " Basics", 10);
	addlinetomenu(ent, " Standard commands", 20);
	addlinetomenu(ent, " Print full command list", 30);
	addlinetomenu(ent, " Rune Types", 40);
	addlinetomenu(ent, " Consumable items", 50);
	addlinetomenu(ent, " Trading", 60);
	addlinetomenu(ent, " Voting", 70);
	addlinetomenu(ent, " Game modes", 80);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowHelpMenu_handler);

	//Set current line
	if (lastpick)
		ent->client->menustorage.currentline = (lastpick / 10) + 2;
	else ent->client->menustorage.currentline = 12;

	//Display the menu
	showmenu(ent);
}

