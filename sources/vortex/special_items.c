#include "g_local.h"
/*
#define	ITEM_TRADE_MAXCLIENTS		10	// max number of clients for player select menu
#define	ITEM_TRADE_MAX_DISTANCE		128	// max trading distance
#define ITEM_TRADE_QUEUE_SIZE		3	// max number of items that can be traded at a time
#define ITEMS_MAX_STORAGE			6	// inventory array size
#define ITEMS_MAX_STACKSIZE			9	// max items that can be placed in one slot

void ItemMenuClose (edict_t *ent)
{
	//gi.dprintf("%s called ItemMenuClose()\n", ent->client->pers.netname);

	EmptyQueue(ent);
	ent->client->trading = false;
	ent->client->trade_accepted = false;

	if (ent->trade_with && ent->trade_with->inuse)
	{
		if (ent->trade_with->client->menustorage.menu_active 
			&& ent->trade_with->client->trading)
			closemenu(ent->trade_with); // close their menu
		EmptyQueue(ent->trade_with);
		ent->trade_with->client->trading = false;
		ent->trade_with->client->trade_accepted = false;
		ent->trade_with->trade_with = NULL;
	}
	ent->trade_with = NULL;

	if (!ent->client->menustorage.menu_active)
		return; // menu already closed!
	closemenu(ent);
	return;
}

void ItemUpdateTradeStatus (edict_t *ent)
{
	if (!G_EntExists(ent->trade_with))
	{
		ItemMenuClose(ent);
		return;
	}
	ent->client->trade_accepted = false;
	ent->trade_with->client->trade_accepted = false;
	// if we are trading with someone, then refresh their menu
	// since we've changed our trade items
	if (ent->trade_with && ent->trade_with->inuse
		&& (ent->trade_with->client->menustorage.optionselected == &tradeconfirm_handler))
	{
		closemenu(ent->trade_with);
		OpenTradeConfirmMenu(ent->trade_with, ent->trade_with->client->menustorage.currentline);
		ent->client->trading = true;
	}
}

void ItemClear (items_t *item)
{
	int i;

	strcpy(item->itemname, "");
	strcpy(item->item_id, "");
	item->level = 0;
	item->owner = NULL;
	item->quantity = 0;
	item->type = 0;
	
	for (i=0; i<8; i++) {
		item->modifiers[i] = 0;
	}
}

qboolean SameItem (items_t *i1, items_t *i2)
{
	int i;

	for (i=0; i<8; i++) {
		if (i1->modifiers[i] != i2->modifiers[i])
			return false;
	}
	return (!strcmp(i1->item_id, i2->item_id) && !strcmp(i1->itemname, i2->itemname)
		&& (i1->level == i2->level) && (i1->type == i2->type));
}

void AddItemToStack (items_t *item, items_t *stack)
{
	item->quantity--;
	if (item->quantity < 1)
		ItemClear(item);
	stack->quantity++;
}

void MoveItemFromStack (items_t *item, items_t *stack)
{
	ItemCopy(stack, item);
	stack->quantity--;
	if (stack->quantity < 1)
		ItemClear(stack);
	item->quantity = 1;
}

void ItemDebugPrint (items_t *item)
{
	int i;

	gi.dprintf("Item Properties:\n");
	gi.dprintf("Level %d, Qty: %d, Owner %s, Type %d\n", item->level, 
		item->quantity, (item->owner?"true":"false"), item->type);
	gi.dprintf("Id String: (%s)\n", item->item_id);
	gi.dprintf("Modifiers: ");
	for (i=0; i<8; i++) {
		gi.dprintf("%d ", item->modifiers[i]);
	}
	gi.dprintf("\n");
}

char GetRandomChar (void) 
{
	switch (GetRandom(1, 3))
	{
	case 1: return (GetRandom(48, 57));
	case 2: return (GetRandom(65, 90));
	case 3: return (GetRandom(97, 113));
	}
	return ' ';
}

char *GetRandomString (int len)
{
	int i;
	char *s;

	s = (char *) malloc(len*sizeof(char));
	for (i=0; i<len-1; i++) {
		s[i] = GetRandomChar();
	}
	s[i] = '\0'; // terminating char
	return s;
}

qboolean G_CanPickUpItem (edict_t *ent)
{
	int i;

	// only allow clients that are alive and haven't just
	// respawned to pick up a rune
	if (!ent->client || !G_EntIsAlive(ent) 
		|| (ent->client->respawn_time+3 > level.time))
		return false;
	// do we have any space in our inventory?
	for (i=0; i<ITEMS_MAX_STORAGE; i++) {
		if (!ent->myskills.item[i].type)
			return true;
	}
	return false;
}

qboolean G_CanEquipHand (edict_t *ent, items_t *item)
{
	// do they meet the level requirements?
	if (ent->myskills.level < item->level)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be level %d to equip this item.\n", item->level);
		return false;
	}
	// is it the right kind of item?
	if (item->type && (item->type != WEAPON_RUNE))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't equip an %s!\n", GetItemTypeString(item));
		return false;
	}
	return true;
}

qboolean G_CanEquipNeck (edict_t *ent, items_t *item)
{
	// do they meet the level requirements?
	if (ent->myskills.level < item->level)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be level %d to adorn this item.\n", item->level);
		return false;
	}
	// is it the right kind of item?
	if (item->type && (item->type != ABILITY_RUNE))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't adorn a %s!\n", GetItemTypeString(item));
		return false;
	}
	return true;
}

items_t *G_FindFreeItemSlot (edict_t *ent)
{
	int i;

	for (i=0; i<ITEMS_MAX_STORAGE; i++) {
		if (ent->myskills.item[i].type)
			continue;
		return &ent->myskills.item[i];
	}
	return NULL;
}

void ItemCopy (items_t *source, items_t *dest)
{
	int		i;

	strcpy(dest->item_id, source->item_id);
	strcpy(dest->itemname, source->itemname);
	dest->level = source->level;
	dest->owner = source->owner;
	dest->type = source->type;
	dest->quantity = source->quantity;

	for (i=0; i<8; i++) {
		dest->modifiers[i] = source->modifiers[i];
	}
}

int GetItemValue (items_t *item)
{
	int	i, val=0;

	if (item->type == WEAPON_RUNE)
	{
		for (i=1; i<4; i++) {
			val += item->modifiers[i];
		}
	}
	else if (item->type == ABILITY_RUNE)
	{
		for (i=1; i<8; i+=2) {
			val += item->modifiers[i];
		}
	}
	return val;
}

char *GetItemQualityString (items_t *item)
{
	int		val;
	char	*runeclass;

	if (strcmp(item->itemname, ""))
		return item->itemname;

	val = GetItemValue(item);
	switch (val)
	{
	case 0: runeclass = "Worthless"; break;
	case 1: runeclass = "Asstastic"; break;
	case 2: runeclass = "Useless"; break;
	case 3: runeclass = "Broken"; break;
	case 4: runeclass = "Cracked"; break;
	case 5: runeclass = "Normal"; break;
	case 6: runeclass = "Decent"; break;
	case 7: runeclass = "Newbie's"; break;
	case 8: runeclass = "Fine"; break;
	case 9: runeclass = "Sturdy"; break;
	case 10: runeclass = "Worthy"; break;
	case 11: runeclass = "Strong"; break;
	case 12: runeclass = "Glorious"; break;
	case 13: runeclass = "Blessed"; break;
	case 14: runeclass = "Enchanted"; break;
	case 15: runeclass = "Saintly"; break;
	case 16: runeclass = "Holy"; break;
	case 17: runeclass = "Godly"; break;
	case 18: runeclass = "Warrior's"; break;
	case 19: runeclass = "Soldier's"; break;
	case 20: runeclass = "Knight's"; break;
	case 21: runeclass = "Lord's"; break;
	case 22: runeclass = "King's"; break;
	case 23: runeclass = "Grandmaster's"; break;
	case 24: runeclass = "God's"; break;
	case 25: runeclass = "GHz's"; break;
	default: runeclass = "Cheater's"; break;
	}
	return runeclass;
}

char *GetItemTypeString (items_t *item)
{
	if (item->type == WEAPON_RUNE)
		return "Weapon Rune";
	else if (item->type == ABILITY_RUNE)
		return "Ability Rune";
	else if (item->type == HEALTH_POTION)
		return "Health Potion";
	else
		return "Unknown";
}

void PrintItemProperties (edict_t *ent, items_t *item)
{
	int		i;
	char	str1[16], str2[16], str3[16], str4[16];
	char	tmp[50], final[500];

	gi.centerprintf(ent, "%s %s\n", GetItemQualityString(item), GetItemTypeString(item));
	if (item->type == WEAPON_RUNE)
	{
		rune_getweaponstrings(item, str1, str2, str3, str4);
		gi.cprintf(ent, PRINT_HIGH, "%s: %s +%d %s +%d %s +%d", str1, str2, 
			item->modifiers[1], str3, item->modifiers[2], str4, item->modifiers[3]);
	}
	else if (item->type == ABILITY_RUNE)
	{
		for (i=0; i<8; i+=2) {
			strcpy(tmp, ""); // clear temp string
			if (item->modifiers[(i+1)] > 0)
				sprintf(tmp, "%s +%d ", GetAbilityString(item->modifiers[i]), item->modifiers[(i+1)]);
			strcat(final, tmp);
		}
		gi.cprintf(ent, PRINT_HIGH, "%s\n", final);
	}

}

void propertiesmenu_handler (edict_t *ent, int option)
{
	if (option == 1)
	{
		if (ent->client->menustorage.oldmenuhandler == &tradeconfirm_handler)
			OpenTradeConfirmMenu(ent, 0);
		else
			ShowInventoryMenu(ent, ent->client->menustorage.oldline);
	}
	if (option == 3)
	{
		RemoveAbilityRune(ent);
		RemoveWeaponRune(ent);

		ItemClear(ent->select);

		ApplyAbilityRune(ent);
		ApplyWeaponRune(ent);
	}
	if (option == 4)
	{
		if (ent->select->type == HEALTH_POTION)
		{
			ent->health = ent->max_health;
			//RemoveAllCurses(ent);
			CurseRemove(ent, 0);
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/potiondrink.wav"), 1, ATTN_NORM, 0);
			if (ent->select->quantity)
			{
				ent->select->quantity--;
				if (ent->select->quantity < 1)
					ItemClear(ent->select);
				ent->select = NULL;
				return;
			}
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
			OpenItemPropertiesMenu(ent, 0);
			return;
		}
		ItemClear(ent->select);
		
	}
	ent->select = NULL;
}

void OpenItemPropertiesMenu (edict_t *ent, items_t *item)
{
	int		value;
	items_t *it;
	char	str1[16], str2[16], str3[16], str4[16];

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	if (item) // we're using an item outside of our inventory (trading)
		it = item;
	else
		it = ent->select;

	value = GetItemValue(it);

	if (strcmp(it->itemname, ""))
		addlinetomenu(ent, va("%s %s", it->itemname, GetItemTypeString(it)), MENU_GREEN_CENTERED);
	else if (value)
		addlinetomenu(ent, va("%s (+%d)", GetItemTypeString(it), value), MENU_GREEN_CENTERED);
	else 
		addlinetomenu(ent, va("%s", GetItemTypeString(it)), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	if (it->type == WEAPON_RUNE)
	{
		rune_getweaponstrings(it, str1, str2, str3, str4);
		addlinetomenu(ent, va("%s:", str1), 0);
		addlinetomenu(ent, va("%s +%d", str2, it->modifiers[1]), 0);
		addlinetomenu(ent, va("%s +%d", str3, it->modifiers[2]), 0);
		addlinetomenu(ent, va("%s +%d", str4, it->modifiers[3]), 0);
	}
	else if (it->type == ABILITY_RUNE)
	{
		if (it->modifiers[1] > 0)
			addlinetomenu(ent, va("%s +%d", GetAbilityString(it->modifiers[0]), 
			it->modifiers[1]), 0);
		else
			addlinetomenu(ent, " ", 0);
		if (it->modifiers[3] > 0)
			addlinetomenu(ent, va("%s +%d", GetAbilityString(it->modifiers[2]), 
			it->modifiers[3]), 0);
		else
			addlinetomenu(ent, " ", 0);
		if (it->modifiers[5] > 0)
			addlinetomenu(ent, va("%s +%d", GetAbilityString(it->modifiers[4]), 
			it->modifiers[5]), 0);
		else
			addlinetomenu(ent, " ", 0);
		if (it->modifiers[7] > 0)
			addlinetomenu(ent, va("%s +%d", GetAbilityString(it->modifiers[6]), 
			it->modifiers[7]), 0);
		else
			addlinetomenu(ent, " ", 0);
	}
	else if (it->type == HEALTH_POTION)
	{
		//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "Use this potion to regain", 0);
		addlinetomenu(ent, "full health and heal all", 0);
		addlinetomenu(ent, "curses.", 0);
		addlinetomenu(ent, " ", 0);
	}

	addlinetomenu(ent, " ", 0);
	if (!item)
	{
		addlinetomenu(ent, "Use", 4);
		addlinetomenu(ent, "Delete", 3);
	}
	else
	{
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
	}
	addlinetomenu(ent, "Back", 1);
	addlinetomenu(ent, "Exit", 2);
	setmenuhandler(ent, propertiesmenu_handler);
	ent->client->menustorage.currentline = 10;
	showmenu(ent);
}

void itemhelp_handler (edict_t *ent, int option)
{
	ShowInventoryMenu(ent, ent->client->menustorage.oldline);
}

void OpenItemHelpMenu (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenItemHelpMenu()\n");

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Item Menu Help", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "To move an item:", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Select the item you want", 0);
	addlinetomenu(ent, "to move and press ENTER.", 0);
	addlinetomenu(ent, "Then select the slot you", 0);
	addlinetomenu(ent, "want to move it to and hit", 0);
	addlinetomenu(ent, "ENTER again.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "To inspect an item:", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Select the item you want to", 0);
	addlinetomenu(ent, "inspect and press ENTER", 0);
	addlinetomenu(ent, "twice.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Back", 1);

	setmenuhandler(ent, itemhelp_handler);
	ent->client->menustorage.currentline = 17;
	showmenu(ent);
}

qboolean G_CanTrade (edict_t *ent, edict_t *other)
{
	return (G_EntIsAlive(ent) && G_EntIsAlive(other) && (ent != other)
		&& (entdist(ent, other) <= ITEM_TRADE_MAX_DISTANCE));
}

qboolean InTradeQueue (edict_t *ent, items_t *item)
{
	int	i;

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		if (ent->trade[i] != item)
			continue;
		return true;
	}
	return false;
}

void RemoveItemFromQueue (edict_t *ent, items_t *item)
{
	int	i;

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		if (ent->trade[i] != item)
			continue;
		ent->trade[i] = NULL;
	}
}

qboolean AddItemToQueue (edict_t *ent, items_t *item)
{
	int	i;

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		if (ent->trade[i] && ent->trade[i]->type)
			continue;
		ent->trade[i] = item;
		return true;
	}
	gi.cprintf(ent, PRINT_HIGH, "Trade queue is full.\n");
	return false;
}

void EmptyQueue (edict_t *ent)
{
	int	i;

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		ent->trade[i] = NULL;
	}
}

items_t *GetItemFromQueue (edict_t *ent)
{
	int i;

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		if (ent->trade[i] && ent->trade[i]->type)
			return ent->trade[i];
	}
	return NULL;
}

void TradeItems (edict_t *ent, edict_t *other)
{
	int		i;
	items_t *it1, *it2, *slot, temp;

	gi.dprintf("INFO: %s and %s exchanged runes.\n", 
		ent->client->pers.netname, other->client->pers.netname);

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++)
	{
		it1 = GetItemFromQueue(ent);
		it2 = GetItemFromQueue(other);

		if (it1 && it2)
		{
			WriteToLogfile(ent, va("Exchanged rune %s with %s for rune %s\n",
				it1->item_id, other->client->pers.netname, it2->item_id));

			WriteToLogfile(other, va("Exchanged rune %s with %s for rune %s\n",
				it2->item_id, other->client->pers.netname, it1->item_id));

			// swap items
			ItemCopy(it1, &temp);
			ItemCopy(it2, it1);
			ItemCopy(&temp, it2);
			RemoveItemFromQueue(ent, it1);
			RemoveItemFromQueue(other, it2);
		}
		else if (it1)
		{
			if ((slot = G_FindFreeItemSlot(other)) != NULL)
			{
				WriteToLogfile(ent, va("Gave rune %s to %s\n",
					it1->item_id, other->client->pers.netname));

				WriteToLogfile(other, va("Received rune %s from %s\n",
					it1->item_id, ent->client->pers.netname));

				// move it to the empty slot
				ItemCopy(it1, slot);
				ItemClear(it1);
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Not enough room.\n");
				gi.cprintf(other, PRINT_HIGH, "Not enough room.\n");
			}
		}
		else if (it2)
		{
			if ((slot = G_FindFreeItemSlot(ent)) != NULL)
			{
				WriteToLogfile(ent, va("Received rune %s from %s\n",
					it2->item_id, other->client->pers.netname));

				WriteToLogfile(other, va("Gave rune %s to %s\n",
					it2->item_id, ent->client->pers.netname));

				// move it to the empty slot
				ItemCopy(it2, slot);
				ItemClear(it2);	
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Not enough room.\n");
				gi.cprintf(other, PRINT_HIGH, "Not enough room.\n");
			}
		}
	}
}

void tradeconfirm_handler (edict_t *ent, int option)
{
	if (option == 99)
	{
		ItemMenuClose(ent);
		return;
	}
	if (option == 98)
	{
		// toggle 
		if (ent->client->trade_accepted)
		{
			ent->client->trade_accepted = false;
		}
		// don't accept trade if they closed their window
		else if (ent->trade_with->client->menustorage.menu_active)
		{
			ent->client->trade_accepted = true;
			if (ent->trade_with->client->trade_accepted)
			{
				TradeItems(ent, ent->trade_with);
				ItemMenuClose(ent);
			}
		}
		OpenTradeConfirmMenu(ent, ent->client->menustorage.currentline);
		return;
	}
	if (option == 97)
	{
		ItemUpdateTradeStatus(ent);
		OpenTradeItemSelectMenu(ent, 14);
		ent->client->trade_accepted = ent->trade_with->client->trade_accepted = false;
		return;
	}
	OpenItemPropertiesMenu(ent, ent->trade_with->trade[option-1]);
}

void OpenTradeConfirmMenu (edict_t *ent, int lastline)
{
	int			i, value;
	items_t		*it;

	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenTradeConfirmMenu()\n");

	if (!G_EntExists(ent->trade_with))
		return;
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Trade Menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Your offer:", 0);

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		it = ent->trade[i];
		if (it && it->type)
		{
			value = GetItemValue(it);
			if (strcmp(it->itemname, ""))
				addlinetomenu(ent, va("%d %s", it->quantity, it->itemname), 0);
			else if (value)
				addlinetomenu(ent, va("%d %s (+%d)", it->quantity, GetItemTypeString(it), value), 0);
			else
				addlinetomenu(ent, va("%d %s", it->quantity, GetItemTypeString(it)), 0);
		}
		else
		{
			addlinetomenu(ent, " ", 0);
		}
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("%s's items:", ent->trade_with->client->pers.netname), 0);

	for (i=0; i<ITEM_TRADE_QUEUE_SIZE; i++) {
		it = ent->trade_with->trade[i];
		if (it && it->type)
		{
			value = GetItemValue(it);
			if (strcmp(it->itemname, ""))
				addlinetomenu(ent, va("%d %s", it->quantity, it->itemname), i+1);
			else if (value)
				addlinetomenu(ent, va("%d %s (+%d)", it->quantity, GetItemTypeString(it), value), i+1);
			else
				addlinetomenu(ent, va("%d %s", it->quantity, GetItemTypeString(it)), i+1);
		}
		else
		{
			addlinetomenu(ent, " ", 0);
		}
	}

	addlinetomenu(ent, " ", 0);
	if (ent->client->trade_accepted)
		addlinetomenu(ent, "Offer Accepted", 98);
	else
		addlinetomenu(ent, "Offer Declined", 98);
	addlinetomenu(ent, "Back", 97);
	addlinetomenu(ent, "Exit", 99);
	setmenuhandler(ent, tradeconfirm_handler);
	if (lastline)
		ent->client->menustorage.currentline = lastline;
	else
		ent->client->menustorage.currentline = 13;
	showmenu(ent);

}

void itemselect_handler (edict_t *ent, int option)
{
	items_t	*it;

	if (option == 99)
	{
		ItemMenuClose(ent);
		return;
	}
	if (option == 98)
	{
		ItemMenuClose(ent);
		OpenPlayerSelectMenu(ent);	
		return;
	}

	ItemUpdateTradeStatus(ent);
	if (option == 97)
	{
		OpenTradeConfirmMenu(ent, 0);
		return;
	}

	it = &ent->myskills.item[option-1];
	if (it->type)
	{
		if (!InTradeQueue(ent, it))
			AddItemToQueue(ent, it);
		else
			RemoveItemFromQueue(ent, it);
	}
	OpenTradeItemSelectMenu(ent, ent->client->menustorage.currentline);
}

void OpenTradeItemSelectMenu (edict_t *ent, int lastline)
{
	int			i, value;
	qboolean	q=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenTradeItemSelectMenu()\n");

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Trade Menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Please select items that", 0);
	addlinetomenu(ent, "you want to offer for", 0);
	addlinetomenu(ent, "trade", 0);
	addlinetomenu(ent, " ", 0);
	// main inventory items
	for (i=0; i<ITEMS_MAX_STORAGE; i++) {
		if (ent->myskills.item[i].type)
		{
			// print all items in the inventory
			// items in the trade queue get a '*' on each side of the item
			q = InTradeQueue(ent, &ent->myskills.item[i]);
			value = GetItemValue(&ent->myskills.item[i]);
			if (strcmp(ent->myskills.item[i].itemname, ""))
				addlinetomenu(ent, va("%c%d %s%c", (q?'*':' '), ent->myskills.item[i].quantity, 
					ent->myskills.item[i].itemname, (q?'*':' ')), i+1);
			else if (value)
				addlinetomenu(ent, va("%c%d %s (+%d)%c", (q?'*':' '), ent->myskills.item[i].quantity, 
					GetItemTypeString(&ent->myskills.item[i]), value, (q?'*':' ')), i+1);
			else
				addlinetomenu(ent, va("%c%d %s%c", (q?'*':' '), ent->myskills.item[i].quantity, 
					GetItemTypeString(&ent->myskills.item[i]), (q?'*':' ')), i+1);
		}
		else
		{
			addlinetomenu(ent, "-Empty-", i+1);
		}
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Done", 97);
	addlinetomenu(ent, "Back", 98);
	addlinetomenu(ent, "Exit", 99);
	setmenuhandler(ent, itemselect_handler);
	if (lastline)
		ent->client->menustorage.currentline = lastline;
	else
		ent->client->menustorage.currentline = 7;
	showmenu(ent);
}

void tradeoffer_handler (edict_t *ent, int option)
{
	if (option == 99)
	{
		ent->client->trading = false;
		return;
	}
	OpenTradeConfirmMenu(ent, 0);
}

void OpenTradeOfferMenu (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenTradeOfferMenu()\n");

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	ent->client->trading = true;
	addlinetomenu(ent, "Trade Menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("%s", ent->trade_with->client->pers.netname) , 0);
	addlinetomenu(ent, "wants to trade with you.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Accept", 98);
	addlinetomenu(ent, "Decline", 99);
	setmenuhandler(ent, tradeoffer_handler);
	ent->client->menustorage.currentline = 6;
	showmenu(ent);
}

void playerselect_handler (edict_t *ent, int option)
{
	edict_t *player;

	if (option == 99)
	{
		ent->client->trading = false;
		return;
	}
	if (option == 98)
	{
		ShowInventoryMenu(ent, 16);
		return;
	}
	
	player = g_edicts + option;
	ItemMenuClose(player);
	ent->client->trading = true;
	player->client->trading = true;
	ent->trade_with = player;
	player->trade_with = ent;

	OpenTradeItemSelectMenu(ent, 0);
	OpenTradeOfferMenu(player);
}

void OpenPlayerSelectMenu (edict_t *ent)
{
	int			i;
	edict_t		*cl_ent;
	qboolean	found=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenPlayerSelectMenu()\n");

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Trade Menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Please select a player to", 0);
	addlinetomenu(ent, "trade with:", 0);
	addlinetomenu(ent, " ", 0);

	for (i=0 ; i<game.maxclients ; i++)
	 {
          cl_ent = g_edicts + 1 + i; 
          if (!G_CanTrade(ent, cl_ent)) 
               continue;
		  found = true;
		  addlinetomenu(ent, va("%s", cl_ent->client->pers.netname), i+1);
     }
	if (!found)
		addlinetomenu(ent, "Nobody to trade with!", 0);

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Back", 98);
	addlinetomenu(ent, "Exit", 99);

	setmenuhandler(ent, playerselect_handler);
	if (!ent->client->trading)
		ent->client->menustorage.currentline = 8;
	else
		ent->client->menustorage.currentline = 6;
	showmenu(ent);
//	gi.dprintf("%d\n", ent->client->menustorage.currentline);
}

void inventorymenu_handler (edict_t *ent, int option)
{
	items_t		temp;
	items_t		*slot;
	qboolean	handslot=false, neckslot=false;

	if (option == 99)
		return;
	if (option == 97)
	{
		OpenItemHelpMenu(ent);
		return;
	}
	if (option == 98)
	{
		OpenPlayerSelectMenu(ent);
		return;
	}

	// neck slot
	if (option == ITEMS_MAX_STORAGE+1)
	{
		slot = &ent->myskills.neck;
		if (ent->select && ent->select->type 
			&& !G_CanEquipNeck(ent, ent->select))
		{
			ent->select = NULL;
			ShowInventoryMenu(ent, ent->client->menustorage.currentline);
			return;
		}
	}
	// hand slot
	else if (option == ITEMS_MAX_STORAGE+2)
	{
		slot = &ent->myskills.hand;
		if (ent->select && ent->select->type 
			&& !G_CanEquipHand(ent, ent->select))
		{
			ent->select = NULL;
			ShowInventoryMenu(ent, ent->client->menustorage.currentline);
			return;
		}
	}
	// inventory slot
	else
	{
		slot = &ent->myskills.item[option-1];
		if (ent->select && (ent->select == &ent->myskills.neck) && !G_CanEquipNeck(ent, slot))
		{
			ent->select = NULL;
			ShowInventoryMenu(ent, ent->client->menustorage.currentline);
			return;
		}
	}

	if (slot->type) // slot has something in it
	{
		if (ent->select) // we have an item already selected
		{
		//	RemoveAbilityRune(ent);
		//	RemoveWeaponRune(ent);

			if (ent->select != slot)
			{
				if (slot == &ent->myskills.hand)
					handslot = true;
				if (slot == &ent->myskills.neck)
					neckslot = true;

				// don't swap an invalid inventory item into our hand
				if ((ent->select == &ent->myskills.hand) && !G_CanEquipHand(ent, slot))
				{
					ent->select = NULL;
					ShowInventoryMenu(ent, ent->client->menustorage.currentline);
					return;
				}

				// don't swap an invalid inventory item onto our neck
				if ((ent->select == &ent->myskills.neck) && !G_CanEquipNeck(ent, slot))
				{
					ent->select = NULL;
					ShowInventoryMenu(ent, ent->client->menustorage.currentline);
					return;
				}

				RemoveAbilityRune(ent);
				RemoveWeaponRune(ent);
				
				// if the destination slot is not an equipment slot, but
				// is the same type of item, then stack it up to the
				// maximum allowed stack size
				if (!handslot && !neckslot && SameItem(ent->select, slot) 
					&& (slot->quantity < ITEMS_MAX_STACKSIZE))
				{
					// move item to stack
					AddItemToStack(ent->select, slot);
				}
				else
				{
					// play equip sound
					if (handslot)
						gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/glovesmetal.wav"), 1, ATTN_NORM, 0);
					if (neckslot)
						gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/amulet.wav"), 1, ATTN_NORM, 0);
					// swap items
					ItemCopy(ent->select, &temp);
					ItemCopy(slot, ent->select);
					ItemCopy(&temp, slot);
					
				}

				ent->select = NULL;
				ShowInventoryMenu(ent, ent->client->menustorage.currentline);

				ApplyAbilityRune(ent);
				ApplyWeaponRune(ent);
			}
			else
			{
				// view item properties
				OpenItemPropertiesMenu(ent, NULL);
			}

		//	ApplyAbilityRune(ent);
		//	ApplyWeaponRune(ent);
			return;
		}

		// we've selected an active item slot
		ent->select = slot;
	}
	else if (ent->select) // slot is empty
	{
		// play equip sound
		if (slot == &ent->myskills.hand)
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/glovesmetal.wav"), 1, ATTN_NORM, 0);
		if (slot == &ent->myskills.neck)
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/amulet.wav"), 1, ATTN_NORM, 0);

		RemoveAbilityRune(ent);
		RemoveWeaponRune(ent);

		if (ent->select->quantity) // we're taking something from a stack
		{
			// move 1 item from the stack
			MoveItemFromStack(slot, ent->select);
		}
		else
		{	
			// move it
			ItemCopy(ent->select, slot); // copy to new location
			ItemClear(ent->select); // clear the old slot since we've moved it	
		}

		ent->select = NULL; // clear the pointer

		ApplyAbilityRune(ent);
		ApplyWeaponRune(ent);
	}
	ShowInventoryMenu(ent, ent->client->menustorage.currentline);
}
/*
void ShowInventoryMenu (edict_t *ent, int lastline)
{
	int i, value;

	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, va("%s's items", ent->client->pers.netname), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	// main inventory items
	for (i=0; i<ITEMS_MAX_STORAGE; i++) {
		value = GetItemValue(&ent->myskills.item[i]);
		if (ent->myskills.item[i].type)
		{
			if (strcmp(ent->myskills.item[i].itemname, ""))
				addlinetomenu(ent, va("%d %s", ent->myskills.item[i].quantity, 
					ent->myskills.item[i].itemname), i+1);
			else if (value)
				addlinetomenu(ent, va("%d %s (+%d)", ent->myskills.item[i].quantity, 
					GetItemTypeString(&ent->myskills.item[i]), value), i+1);
			else
				addlinetomenu(ent, va("%d %s", ent->myskills.item[i].quantity,
					GetItemTypeString(&ent->myskills.item[i])), i+1);
		}
		else
		{
			addlinetomenu(ent, "-Empty-", i+1);
		}
	}

	// neck equipment slot
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Neck:", 0);
	if (ent->myskills.neck.type)
	{
		value = GetItemValue(&ent->myskills.neck);
		if (strcmp(ent->myskills.neck.itemname, ""))
			addlinetomenu(ent, va("%d %s", ent->myskills.neck.quantity, 
				ent->myskills.neck.itemname), ITEMS_MAX_STORAGE+1);
		else if (value)
			addlinetomenu(ent, va("%d %s (+%d)", ent->myskills.neck.quantity, 
				GetItemTypeString(&ent->myskills.neck), value), ITEMS_MAX_STORAGE+1);
		else
			addlinetomenu(ent, va("%d %s", ent->myskills.neck.quantity,
				GetItemTypeString(&ent->myskills.neck)), ITEMS_MAX_STORAGE+1);
	}
	else
	{
		addlinetomenu(ent, "-Empty-", ITEMS_MAX_STORAGE+1);
	}

	// hand equipment slot
	addlinetomenu(ent, "Hand:", 0);
	if (ent->myskills.hand.type)
	{
		value = GetItemValue(&ent->myskills.hand);
		if (strcmp(ent->myskills.hand.itemname, ""))
			addlinetomenu(ent, va("%d %s", ent->myskills.hand.quantity, 
				ent->myskills.hand.itemname), ITEMS_MAX_STORAGE+2);
		else if (value)
			addlinetomenu(ent, va("%d %s (+%d)", ent->myskills.hand.quantity, 
				GetItemTypeString(&ent->myskills.hand), value), ITEMS_MAX_STORAGE+2);
		else
			addlinetomenu(ent, va("%d %s", ent->myskills.hand.quantity,
				GetItemTypeString(&ent->myskills.hand)), ITEMS_MAX_STORAGE+2);
	}
	else
	{
		addlinetomenu(ent, "-Empty-", ITEMS_MAX_STORAGE+2);
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Help", 97);
	addlinetomenu(ent, "Trade", 98);
	addlinetomenu(ent, "Exit", 99);

	setmenuhandler(ent, inventorymenu_handler);

	if (lastline)
		ent->client->menustorage.currentline = lastline;
	else
		ent->client->menustorage.currentline = 3;
	showmenu(ent);

//	gi.dprintf("%d\n", ent->client->menustorage.currentline);
}
*/