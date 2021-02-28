#include "g_local.h"

void ShowTradeMenu(edict_t *ent);
void TradeInventoryMenu(edict_t *ent, int lastline);
void TradeFinalMenu(edict_t *ent);
void ShowTradeAskMenu(edict_t *tradee, edict_t *trader);

//************************************************************************************************
//		**TRADE CODE**
//************************************************************************************************

void TradeItems(edict_t *player1, edict_t *player2)
{
	item_t *pSlots1[3];
	item_t *pSlots2[3];
	int i, j;
	int x = 1, y = 1;
	qboolean cantrade = true;

	j = 0;
	//Loop through entire queue
	for (i = 0; i < 3; ++i)
	{
		//Both players have an item to trade?
		if ((player1->trade_item[i] != NULL) && (player2->trade_item[i] != NULL))
		{
            pSlots1[j] = player1->trade_item[i];
			pSlots2[j++] = player2->trade_item[i];
		}
		//Is someone giving away an item instead of trading it?
		else if ((player1->trade_item[i] != NULL) || (player2->trade_item[i] != NULL))
		{
			//Player 1 giving an item?
			if (player1->trade_item[i] != NULL)
			{
				item_t *freeslot = V_FindFreeTradeSlot(player2, y++);
				if (freeslot == NULL)
				{
					cantrade = false;
					break;
				}
                pSlots1[j] = player1->trade_item[i];
				pSlots2[j++] = freeslot;
			}
			//Player 2 giving an item?
			else if (player2->trade_item[i] != NULL)
			{
				item_t *freeslot = V_FindFreeTradeSlot(player1, x++);
				if (freeslot == NULL)
				{
					cantrade = false;
					break;
				}
                pSlots2[j] = player2->trade_item[i];
				pSlots1[j++] = freeslot;
			}
		}
		//No trading needed for this slot. NEXT!
	}

	//Is the trade ready to go?
	if (!cantrade)
	{
		safe_cprintf(player1, PRINT_HIGH, "Not enough room to make a full trade. Re-arrange your items and try again.\n");
		safe_cprintf(player2, PRINT_HIGH, "Not enough room to make a full trade. Re-arrange your items and try again.\n");
		return;
	}
	
	//Swap the items (3 items max)
    for (i = 0; i < j; ++i)
	{
		char id1[16];
		char id2[16];

		//Do the actual trading
		V_ItemSwap(pSlots1[i], pSlots2[i]);

		if (pSlots1[i] == NULL) strcpy(id1, va("<EMPTY>",pSlots1[i]->itemtype));
		else if (strlen(pSlots1[i]->id) < 1) strcpy(id1, va("<ITEMTYPE: %d>",pSlots1[i]->itemtype));
		else strcpy(id1, pSlots1[i]->id);

		if (pSlots2[i] == NULL) strcpy(id2, va("<EMPTY>",pSlots2[i]->itemtype));
		else if (strlen(pSlots2[i]->id) < 1) strcpy(id2, va("<ITEMTYPE: %d>",pSlots2[i]->itemtype));
		else strcpy(id2, pSlots2[i]->id);

		WriteToLogfile(player1, va("Traded rune %s with %s for rune %s\n", id1, player2->myskills.player_name, id2));
		WriteToLogfile(player2, va("Traded rune %s with %s for rune %s\n", id2, player1->myskills.player_name, id1));
	}

	//Alert players involved of a successful trade
    safe_cprintf(player1, PRINT_HIGH, "Trade completed. Check your items.\n");
	safe_cprintf(player2, PRINT_HIGH, "Trade completed. Check your items.\n");

	//Close menus
	closemenu(player1);
	closemenu(player2);

	//Clear trade info
	for (i = 0; i < 3; ++i)
	{
		player1->trade_item[i] = NULL;
		player2->trade_item[i] = NULL;
	}
	player1->trade_with = NULL;
	player2->trade_with = NULL;
	player1->client->trade_final = false;
	player2->client->trade_final = false;
	player1->client->trade_accepted = false;
	player2->client->trade_accepted = false;

	//Save players
	if (savemethod->value == 1)
	{
		SaveCharacter(player1);
		SaveCharacter(player2);
	}else if (savemethod->value == 0)
	{
		char path[MAX_QPATH];

		memset(path, 0, strlen(path));
		VRXGetPath(path, player1);
		VSF_SaveRunes(player1, path);

		memset(path, 0, strlen(path));
		VRXGetPath(path, player2);
		VSF_SaveRunes(player2, path);

	}else if (savemethod->value == 3)
	{
		VSFU_SaveRunes(player1);
		VSFU_SaveRunes(player2);
	}
#ifndef NO_GDS
	else if (savemethod->value == 2)
	{
		V_GDS_Queue_Add(player1, GDS_SAVERUNES);
		V_GDS_Queue_Add(player2, GDS_SAVERUNES);
	}
#endif
}

//************************************************************************************************
//		**TRADE FINISH MENU**
//************************************************************************************************

void OpenTradeViewOtherMenu_handler(edict_t *ent, int option)
{
	if (option == 777)
	{
		TradeFinalMenu(ent);
		return;
	}
	else //ent closed the menu (reject)
	{
		closemenu(ent);
		return;
	}
}

//************************************************************************************************

void OpenTradeViewOtherMenu(edict_t *ent, int option)
{
	//Menu stuff
	if (!ShowMenu(ent))
		return;
	clearmenu(ent);

	//Load the item
	StartShowInventoryMenu(ent, ent->trade_with->trade_item[option]);

	//Append a footer to the menu
	addlinetomenu(ent, "Previous menu", 777);
	addlinetomenu(ent, "Exit", 666);

	//set currentline
	ent->client->menustorage.currentline += 2;

	//Set handler
	setmenuhandler(ent, OpenTradeViewOtherMenu_handler);

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**TRADE FINISH MENU**
//************************************************************************************************

void TradeFinalMenu_handler(edict_t *ent, int option)
{
	if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}
	else if (option == 777)	//ent selected previous menu
	{
		ent->client->trade_final = false;
		TradeInventoryMenu(ent, 0);
		return;
	}
	else if (option == 888) //ent toggled accept/reject
	{
		// trade no longer valid
		if (!ent->trade_with || !ent->trade_with->inuse || !ent->trade_with->client)
			return;

		//toggle accepted
		ent->client->trade_accepted = !ent->client->trade_accepted;

		//If both people accept, start trading! 
		if (ent->client->trade_accepted && ent->trade_with->client->trade_accepted)
		{
			TradeItems(ent, ent->trade_with);
			return;
		}
	}
	//Viewing an item the other player has?
	else if(option % 10 == 0)
	{
		int type;
		int itemnumber = (option / 10) - 1;
// GHz START
		// 3.7 make sure this item is still valid
		// it is possible the other player somehow unselected it
		if (!ent->trade_with->trade_item[itemnumber])
		{
			WriteServerMsg("TradeFinalMenu_handler() couldn't find item.", "ERROR", true, false);
			return;
		}
		type = ent->trade_with->trade_item[itemnumber]->itemtype;
// GHz END
		//int type = ent->trade_with->trade_item[itemnumber]->itemtype;

		if ((type != ITEM_POTION) && (type != ITEM_ANTIDOTE))
		{
			ent->client->trade_final = false;
			OpenTradeViewOtherMenu(ent, itemnumber);
			return;
		}
	}

	//refresh the menu
	TradeFinalMenu(ent);
}

//************************************************************************************************

void TradeFinalMenu(edict_t *ent)
{
	int i;
	int linecount = 0;

	//If the player is in the final menu, showmenu returns false, so add an extra check for that
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	if (ent->trade_with == NULL)
	{
		gi.dprintf("ERROR: TradeFinalMenu() found a NULL ent->trade_with!\n");
		ent->client->trading = false;
		closemenu(ent);
		return;
	}

	//Print header
	addlinetomenu(ent, va("%s's items", ent->myskills.player_name), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	//Print each item (this player)
	for (i = 0; i < 3; ++i)
	{
		item_t *item;
		item = ent->trade_item[i];
		if (item != NULL)
		{
			addlinetomenu(ent, va("%s", V_MenuItemString(item, ' ')), 0);
			++linecount;
		}
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("%s's items", ent->trade_with->myskills.player_name), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	//Print each item (trade_with)
	for (i = 0; i < 3; ++i)
	{
		item_t *item;
		item = ent->trade_with->trade_item[i];
		if (item != NULL)
		{
			addlinetomenu(ent, va("%s", V_MenuItemString(item, ' ')), (i+1)*10);	//10, 20, 30
			++linecount;
		}
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);

	//Accepted or denied?
	if (ent->client->trade_accepted)
        addlinetomenu(ent, "<You have accepted>", 888);
	else addlinetomenu(ent, "<You have not accepted>", 888);

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Previous", 777);
	addlinetomenu(ent, " Exit", 666);

	//Set handler
	setmenuhandler(ent, TradeFinalMenu_handler);

	//Where are we in the menu?
	ent->client->menustorage.currentline = 9 + linecount;

	//show that we are in the final menu
	ent->client->trade_final = true;

	//Display the menu
	showmenu(ent);
}


//************************************************************************************************
//		**TRADE SELECT ITEM MENU**
//************************************************************************************************

void TradeInventoryMenu_handler(edict_t *ent, int option)
{
	if (ent->trade_with == NULL)
	{
		gi.dprintf("ERROR: TradeInventoryMenu_handler() found a NULL ent->trade_with!\n");
		ent->client->trading = false;
		closemenu(ent);
		return;
	}

	if ((option > 0) && (option-1 < MAX_VRXITEMS) && (ent->myskills.items[option-1].itemtype != ITEM_NONE))
	{
		int i;
		item_t *item = &ent->myskills.items[option-1];

		//Was this item selected?
		for (i = 0; i < 3; ++i)
		{
            if (item == ent->trade_item[i])
			{
				ent->trade_item[i] = NULL;
				TradeInventoryMenu(ent, option);
				return;
			}
		}

		//Find a free trade slot
		for (i = 0; i < 3; ++i)
		{
            if (ent->trade_item[i] == NULL)
			{
				// don't allow trade of 1-up (it's broken)
				if (item->itemtype == 1032)
				{
					safe_cprintf(ent, PRINT_HIGH, "Can't trade this item.\n");
					return;
				}

				ent->trade_item[i] = item;
				TradeInventoryMenu(ent, option);
				return;
			}
		}
	}
	else if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}
	else if (option == 888)	//Ready to confirm trade
	{
		//We haven't accepted yet
		ent->client->trade_accepted = false;
		ent->trade_with->client->trade_accepted = false;

		//Refresh the other player's menu if they were waiting
		if (ent->trade_with->client->trade_final)
		{			
			closemenu(ent->trade_with);
			TradeFinalMenu(ent->trade_with);
		}

		//Show the next menu
		TradeFinalMenu(ent);
		return;
	}

	//refresh the menu
	TradeInventoryMenu(ent, option);
}

//************************************************************************************************

void TradeInventoryMenu(edict_t *ent, int lastline)
{
	int i;

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	addlinetomenu(ent, va("%s's items", ent->client->pers.netname), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	//Print each item
	for (i = 3; i < MAX_VRXITEMS; ++i)
	{
		char ch;
		item_t *item;
		item = &ent->myskills.items[i];

		//Is this item selected for trading?
		if ((item == ent->trade_item[0]) || (item == ent->trade_item[1]) || (item == ent->trade_item[2]))
			ch = '*';
		else ch = ' ';

		addlinetomenu(ent, va("%s", V_MenuItemString(item, ch)), i+1);
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Done", 888);
	addlinetomenu(ent, " Exit", 666);

	//Set handler
	setmenuhandler(ent, TradeInventoryMenu_handler);

	//Where are we in the menu?
	if (lastline)
		ent->client->menustorage.currentline = lastline-1;
	else ent->client->menustorage.currentline = 11;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**TRADE SELECT ITEM MENU**
//************************************************************************************************

void ShowTradeMenu_handler(edict_t *ent, int option)
{
	if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}
	else	//ent picked someone
	{
		edict_t *trade_with = V_getClientByNumber(option-1);

		//Make sure this player isn't already trading
		if(trade_with->trade_with != NULL)
		{
			safe_cprintf(ent, PRINT_HIGH, "This player is trading with someone else.\n");
			closemenu(ent);
			return;
		}

		//Set the person to trade with (I hope this is REAL confusing :)
		ent->trade_with = trade_with;
		trade_with->trade_with = ent;
		ent->client->trade_accepted = false;
		trade_with->client->trade_accepted = false;

		//Reset the trade items (both players)
		memset(&ent->trade_item, 0, sizeof(item_t *) * 3);
		memset(&ent->trade_with->trade_item, 0, sizeof(item_t *) * 3);

		//Ask them to trade with you
		ShowTradeAskMenu(trade_with, ent);

		//open the next menu
		TradeInventoryMenu(ent, 0);
	}
}

//************************************************************************************************

void ShowTradeMenu(edict_t *ent)
{
	int i;
	int j = 0;
	edict_t *temp;

	//Blocking trades?
	if (ent->client->trade_off)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have disabled trading. Turn it on again with cmd: \"trade on\".\n");
		return;
	}

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	addlinetomenu(ent, "Select player to trade with", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);

	for_each_player(temp, i)
	{
		if ((V_EntDistance(ent, temp) < TRADE_MAX_DISTANCE)  && !G_IsSpectator(temp)
			&& (!temp->client->trading) && (!temp->client->trade_off) && (temp != ent))
		{
			//Add player to the list
			addlinetomenu(ent, va(" %s", temp->myskills.player_name), GetClientNumber(temp));
			++j;
		}
	}

	//If nobody is on the list, don't bother showing the menu
	if (j == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Nobody found close enough to trade with.\n");
		closemenu(ent);
		return;
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowTradeMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 5 + j;

	//Display the menu
	showmenu(ent);

	// try to shortcut to chat-protect mode
	if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
		ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
}

//************************************************************************************************
//		**TRADE ASK MENU**
//************************************************************************************************

void ShowTradeAskMenu_handler(edict_t *ent, int option)
{
	if (ent->trade_with == NULL)
	{
		gi.dprintf("ERROR: ShowTradeAskMenu_handler() found a NULL ent->trade_with!\n");
		ent->client->trading = false;
		closemenu(ent);
		return;
	}

	if (option == 666)	//ent closed the menu (reject)
	{
		closemenu(ent);
		return;
	}
	else	//ent wants to trade
	{
		//open the next menu
		TradeInventoryMenu(ent, 0);

		// try to shortcut to chat-protect mode
		if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
			ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
	}
}

//************************************************************************************************

void ShowTradeAskMenu(edict_t *tradee, edict_t *trader)
{
	//Usual menu stuff
	if (!ShowMenu(tradee))
		return;

	//People already trading are not in the list
	if (tradee->client->trading)
		return;

	clearmenu(tradee);

	//We are now trading
	tradee->client->trading = true;
	trader->client->trading = true;

	//Print header
	addlinetomenu(tradee, " ", 0);
	addlinetomenu(tradee, " ", 0);
	addlinetomenu(tradee, va("%s is trying", trader->myskills.player_name), MENU_GREEN_CENTERED);
	addlinetomenu(tradee, "to trade with you.", MENU_GREEN_CENTERED);
	addlinetomenu(tradee, " ", 0);
	addlinetomenu(tradee, " ", 0);
	addlinetomenu(tradee, " ", 0);

	//Menu footer
	addlinetomenu(tradee, " Accept", 777);
	addlinetomenu(tradee, " Reject", 666);

	//Set handler
	setmenuhandler(tradee, ShowTradeAskMenu_handler);

	//Set current line
	tradee->client->menustorage.currentline = 9;

	//Display the menu
	showmenu(tradee);
}

//************************************************************************************************

void EndTrade (edict_t *ent)
{
	int i;

	if (!ent->trade_with || !ent->trade_with->inuse)
		return;

	//Clear the trade pointers
	for (i = 0; i < 3; ++i)
	{
		ent->trade_item[i] = NULL;
		ent->trade_with->trade_item[i] = NULL;
	}

	//cancel the trade (trade_with)
	closemenu(ent->trade_with);
	ent->trade_with->client->trade_accepted = false;
	ent->trade_with->client->trade_final = false;
	ent->trade_with->client->trading = false;
	ent->trade_with->trade_with = NULL;

	//cancel the trade (ent)
	ent->trade_with = NULL;
	ent->client->trade_accepted = false;
	ent->client->trade_final = false;
	ent->client->trading = false;
}