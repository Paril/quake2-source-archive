#include "g_local.h"

//************************************************************************************************
//		Sorry about the "hacker" approach to indexing through these menus,
//		it's just quicker this way. :) (doomie)
//************************************************************************************************

//************************************************************************************************
//		**ITEM DELETE MENU**
//************************************************************************************************

void DeleteMenu_handler(edict_t *ent, int option)
{
	if (option - 777 > 0)
	{
		int i;

		//Delete item
		memset(&ent->myskills.items[option - 778], 0, sizeof(item_t));

		//Re-apply equipment
		V_ResetAllStats(ent);
		for (i = 0; i < 4; ++i)
			V_ApplyRune(ent, &ent->myskills.items[i]);

		safe_cprintf(ent, PRINT_HIGH, "Item deleted.\n");
	}
	else if (option - 666 > 0)
	{
		//Back to main
		ShowInventoryMenu(ent, option - 666, false);
		return;
	}
	else
	{
		//Closing menu
		closemenu(ent);
		return;
	}
}

//************************************************************************************************

void ItemDeleteMenu(edict_t *ent, int itemindex)
{
	item_t *item = &ent->myskills.items[itemindex];

	//Process the header
	StartShowInventoryMenu(ent, item);

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "  Delete this item?", 0);
	addlinetomenu(ent, "No, I changed my mind!", 667 + itemindex);
	addlinetomenu(ent, "Yes, delete this item.", 778 + itemindex);

	//Set handler
	setmenuhandler(ent, DeleteMenu_handler);

	ent->client->menustorage.currentline += 4;

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**GENERAL ITEM DISPLAY MENU**
//************************************************************************************************

void StartShowInventoryMenu(edict_t *ent, item_t *item)
{
	int type = item->itemtype;
	int linecount = 1;
	int i;

	if (!ShowMenu(ent))
		return;
	clearmenu(ent);

	//If item has a name, use that instead
	if (strlen(item->name) > 0)
	{
		if (item->setCode == 0)
			addlinetomenu(ent, va(" %s", item->name), MENU_GREEN_LEFT);
		else addlinetomenu(ent, va(" %s (set item)", item->name), MENU_GREEN_LEFT);
	}
	else
	{
		//Print header, depending on the item type
		addlinetomenu(ent, va("%s", V_MenuItemString(item, ' ')), MENU_GREEN_LEFT); 
	}

	//Unique runes need to display stats too
	if (type & ITEM_UNIQUE)
		type ^= ITEM_UNIQUE;

	//Item's stats
	switch(type)
	{
	case ITEM_WEAPON:
		{
			int wIndex = (item->modifiers[0].index / 100) - 10;

			addlinetomenu(ent, " ", 0);
			addlinetomenu(ent, va(" %s", GetWeaponString(wIndex)), 0);
			addlinetomenu(ent, " ", 0);
			addlinetomenu(ent, " Stats:", MENU_GREEN_LEFT);
			linecount += 4;
			for (i = 0; i < MAX_VRXITEMMODS; ++i)
			{
				int mIndex;
				char buf[30];

				if ((item->modifiers[i].type == TYPE_NONE) || (V_HiddenMod(ent, item, &item->modifiers[i])))
					continue;

				mIndex = item->modifiers[i].index % 100;
				strcpy(buf, GetModString(wIndex, mIndex));
				padRight(buf, 20);
				addlinetomenu(ent, va("  %s [%d]", buf, item->modifiers[i].value ), 0);
				++linecount;
			}
			addlinetomenu(ent, " ", 0);
			++linecount;
		}
		break;
	case ITEM_ABILITY:
		{
			addlinetomenu(ent, " ", 0);
			addlinetomenu(ent, " Stats:", MENU_GREEN_LEFT);
			linecount += 2;
			for (i = 0; i < MAX_VRXITEMMODS; ++i)
			{
				int aIndex;
				char buf[30];

				aIndex = item->modifiers[i].index;
				if ((item->modifiers[i].type == TYPE_NONE) || (V_HiddenMod(ent, item, &item->modifiers[i])))
					continue;

				strcpy(buf, GetAbilityString(aIndex));
				padRight(buf, 20);
				addlinetomenu(ent, va("  %s [%d]", buf, item->modifiers[i].value ), 0);
				linecount++;
			}
		}
		break;
	case ITEM_COMBO:
		{
			addlinetomenu(ent, " ", 0);
			addlinetomenu(ent, " Stats:", MENU_GREEN_LEFT);
			linecount += 2;
			for (i = 0; i < MAX_VRXITEMMODS; ++i)
			{
				char buf[30];
				if ((item->modifiers[i].type == TYPE_NONE) || (V_HiddenMod(ent, item, &item->modifiers[i])))
					continue;

				switch(item->modifiers[i].type)
				{
				case TYPE_ABILITY:
					{
						int aIndex;
						aIndex = item->modifiers[i].index;

						strcpy(buf, GetAbilityString(aIndex));
						padRight(buf, 20);
						addlinetomenu(ent, va("  %s [%d]", buf, item->modifiers[i].value ), 0);
						linecount++;
					}
					break;
				case TYPE_WEAPON:
					{
						int wIndex = (item->modifiers[i].index / 100) - 10;
						int mIndex = item->modifiers[i].index % 100;

						strcpy(buf, GetShortWeaponString(wIndex));
						strcat(buf, va(" %s", GetModString(wIndex, mIndex)));
						padRight(buf, 20);
						addlinetomenu(ent, va("  %s [%d]", buf, item->modifiers[i].value ), 0);
						linecount++;
					}
					break;
				}
			}
		}
		break;
	case ITEM_CLASSRUNE:
		{
			addlinetomenu(ent, " ", 0);
			addlinetomenu(ent, " Stats:", MENU_GREEN_LEFT);
			linecount += 2;
			for (i = 0; i < MAX_VRXITEMMODS; ++i)
			{
				int aIndex;
				char buf[30];

				aIndex = item->modifiers[i].index;

				if (item->modifiers[i].type == TYPE_NONE)
					continue;

				strcpy(buf, GetAbilityString(aIndex));
				padRight(buf, 20);
				addlinetomenu(ent, va("  %s [%d]", buf, item->modifiers[i].value ), 0);
				linecount++;
			}
		}
		break;
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);

	//Items such as Gravity boots need to show the number of charges left
	if ((item->itemtype & ITEM_GRAVBOOTS) || (item->itemtype & ITEM_FIRE_RESIST) || (item->itemtype & ITEM_AUTO_TBALL))
	{
		addlinetomenu(ent, va(" Charges left: %d", item->quantity), 0);
		addlinetomenu(ent, " ", 0);
		linecount += 2;
	}

	ent->client->menustorage.currentline = linecount;
}

//************************************************************************************************
//		**ITEM DISPLAY MENU**
//************************************************************************************************

void ShowItemMenu_handler(edict_t *ent, int option)
{
	if (option - 9999 > 0)
	{
		//Delete item menu
		ItemDeleteMenu(ent, option - 10000);
	}
	else if (option - 7777 > 0)
	{
		//Previous menu
		ShowInventoryMenu(ent, option - 7777, false);
		return;
	}
	else if (option - 4444 > 0)
	{
		//Equip/Unequip item
		V_EquipItem(ent, option - 4445);
	}
	else
	{
		//Closing menu
		closemenu(ent);
		return;
	}
}

//************************************************************************************************
//************************************************************************************************

void ShowItemMenu(edict_t *ent, int itemindex)
{
	item_t *item = &ent->myskills.items[itemindex];

	//Load the item
	StartShowInventoryMenu(ent, item);

	//Check to see if this item can be equipped or not
	if (!((item->itemtype & ITEM_GRAVBOOTS) || (item->itemtype & ITEM_FIRE_RESIST) || (item->itemtype & ITEM_AUTO_TBALL)))
	{
		if (itemindex < 4)	addlinetomenu(ent, "Stash this item", 4445 + itemindex); //cambiado
		else				addlinetomenu(ent, "Equip this item", 4445 + itemindex);
		++ent->client->menustorage.currentline;
	}
	
	//Append a footer to the menu
	addlinetomenu(ent, "Previous menu", 7778 + itemindex);
	addlinetomenu(ent, "Exit", 666);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Delete this item", 10000 + itemindex);

	//set currentline
	ent->client->menustorage.currentline += 2;

	//Set handler
	setmenuhandler(ent, ShowItemMenu_handler);

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**MAIN MENU**
//************************************************************************************************

void ShowInventoryMenu_handler(edict_t *ent, int option)
{
	if ((option > 0) && (option-1 < MAX_VRXITEMS) && (ent->myskills.items[option-1].itemtype != ITEM_NONE))
	{
		//Use the consumable items (health potions, holy water)
		if ((ent->myskills.items[option-1].itemtype == ITEM_POTION) || (ent->myskills.items[option-1].itemtype == ITEM_ANTIDOTE))
		{
			cmd_Drink(ent, ent->myskills.items[option-1].itemtype, option);
			return;
		}

		//View selected item
		ShowItemMenu(ent, option-1);
	}
	else if (option == 666)
	{
		closemenu(ent);
		return;
	}

	//refresh the menu
	ShowInventoryMenu(ent, option, false);
}

//************************************************************************************************

void ShowInventoryMenu(edict_t *ent, int lastline, qboolean selling)
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
	for (i = 0; i < MAX_VRXITEMS; ++i)
	{
		item_t *item;
		item = &ent->myskills.items[i];

		//Print equip slot (if required)
		switch(i)
		{
		case 0:	addlinetomenu(ent, " Hand", MENU_GREEN_LEFT); break;
		case 1:	addlinetomenu(ent, " Neck", MENU_GREEN_LEFT); break;
		case 2:	addlinetomenu(ent, " Belt", MENU_GREEN_LEFT); break;
		case 3: addlinetomenu(ent, " back", MENU_GREEN_LEFT);break;
		case 4:	addlinetomenu(ent, " Stash", MENU_GREEN_LEFT);break;
		}
		addlinetomenu(ent, va("%s", V_MenuItemString(item, ' ')), i+1);
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowInventoryMenu_handler);

	//Where are we in the menu?
	if (lastline)
	{
		switch(lastline)
		{
		case 1:		ent->client->menustorage.currentline = 4; break;
		case 2:		ent->client->menustorage.currentline = 6; break;
		case 3:     ent->client->menustorage.currentline = 2; break;
		case 4:		ent->client->menustorage.currentline = 8; break;
		default:	ent->client->menustorage.currentline = 6 + lastline; break;	//start at line 10
		}
	}
	else ent->client->menustorage.currentline = 18;

	//Display the menu (don't show it if this menu was loaded from the armory!)
	if (!selling) showmenu(ent);
}