#include "g_local.h"

int CountUniqueItems(edict_t *ent) {
item_t	*curitem;
int		count=0;

	for ( curitem = ent->inventory; curitem; curitem = curitem->next, count++);
	return count;
}

int CountTotalItems(edict_t *ent) {
item_t	*curitem;
int		count=0;

	for ( curitem = ent->inventory; curitem; curitem = curitem->next)
		count += curitem->quantity;
	return count;
}
		
char *GetItemName(item_t *item) {
	if (!item)
		return NULL;

	switch (item->itemtype) {
	case	ITEM_WEAPON:	return ((weap_t *)(item->item))->name;
	case	ITEM_AMMO:		return ((ammo_t *)(item->item))->name;
	case	ITEM_SUPPLY:	return ((supply_t *)(item->item))->name;
	}
	return NULL;
}

float GetItemWeight(item_t *item) {
	if (!item)
		return 0.0;

	switch (item->itemtype) {
	case	ITEM_WEAPON:	return ((weap_t *)(item->item))->weight;
	case	ITEM_AMMO:		return ((ammo_t *)(item->item))->weight;
	case	ITEM_SUPPLY:	return ((supply_t *)(item->item))->weight;
	}
	return 0.0;
}

item_t *FindInventoryItem(edict_t *ent, void *item) {
item_t *curitem;

	
	for ( curitem = ent->inventory; curitem && curitem->item != item; curitem = curitem->next);
	return curitem;
}

item_t *FindItemByName(edict_t *ent, const char *name, int type) {
item_t	*curitem;
char	*itemname;

	if (!name || !ent)
		return NULL;

	curitem = ent->inventory;
	for ( curitem = ent->inventory; curitem; curitem = curitem->next) {
		if (type) {
			if (type == curitem->itemtype) {
				itemname = GetItemName(curitem);
				if (!DL_strcmp(itemname, name, -1, false))
					break;
			}
		} else {
			itemname = GetItemName(curitem);
			if (itemname && !DL_strcmp(itemname, name, -1, false))
				break;
		}
	}

	return curitem;
}

item_t *FindItemByAltCmd(edict_t *ent, const char *cmd, int type) {
item_t	*curitem;

	if (!cmd || !ent)
		return NULL;

	curitem = ent->inventory;
	for ( curitem = ent->inventory; curitem; curitem = curitem->next) {
		if (curitem->altcmd) {
			if (type && (type == curitem->itemtype) && !DL_strcmp(curitem->altcmd, cmd, -1, false))
				break;
			else if (!type && !DL_strcmp(curitem->altcmd, cmd, -1, false))
				break;
		}
	}

	return curitem;
}

static const char *ac_weap[10] = {"Blaster", "Shotgun", "Super Shotgun", "Machinegun", "Chaingun", "Grenade Launcher","Rocket Launcher", "Hyperblaster", "Railgun", "BFG"};
static const char *ac_gren[3] = {"gren1", "gren2", "gren3"};
static const char *ac_supl[10] = {"light", "jammer", "timer", "wire", "proxy", "video", "tools", "scope", "medikit", "disguise"};

char *GetDefaultAltCmd(edict_t *ent, item_t *item) {
supply_t *s;
char *out;
int i;


	if (item->itemtype == ITEM_WEAPON) {
		for (i = 0; i < 10; i++) {
			if (!FindItemByAltCmd(ent, ac_weap[i], ITEM_WEAPON))
				return DL_strcpy(NULL, ac_weap[i], -1);
		}
		return NULL;
	}
	if (item->itemtype == ITEM_SUPPLY) {
		s = ITEMSUPPLY(item);

		if (s->defaultcmd)
			return s->defaultcmd;

		switch(s->type) {
		case SUP_GRENADE:
			out = NULL;
			for (i = 0; i < 3; i++) {
				if (!FindItemByAltCmd(ent, ac_gren[i], ITEM_SUPPLY))
					out = DL_strcpy(NULL, ac_gren[i], -1);
			}
		break;
		case SUP_FLASHLIGHT: out = DL_strcpy(NULL, ac_supl[0], -1); break;
		case SUP_MSGJAMMER: out = DL_strcpy(NULL, ac_supl[1], -1); break;
		case SUP_TIMER: out = DL_strcpy(NULL, ac_supl[2], -1); break;
		case SUP_TRIPWIRE: out = DL_strcpy(NULL, ac_supl[3], -1); break;
		case SUP_PROXY: out = DL_strcpy(NULL, ac_supl[4], -1); break;
		case SUP_VIDSENSOR: out = DL_strcpy(NULL, ac_supl[5], -1); break;
		case SUP_TOOL: out = DL_strcpy(NULL, ac_supl[6], -1); break;
		case SUP_SIGHT: out = DL_strcpy(NULL, ac_supl[7], -1);	break;
		case SUP_MEDIKIT: out = DL_strcpy(NULL, ac_supl[8], -1); break;
		case SUP_UNIFORM: out = DL_strcpy(NULL, ac_supl[9], -1); break;
		default: out = NULL;
		}
		if (out) DL_DebugLog(va("FUNC: GetDefaultAltCmd: (%s) %s\n", GetItemName(item), out));
		return out;
	}
	return NULL;
}

item_t *AddToInventory(edict_t *ent, int type, void *item, int priority, int count, int max, item_t *depend, char *altcmd) {
item_t *newItem;

	if (!item || !ent)
		return NULL;

	if (count <= 0)
		count = 1;

	newItem = FindInventoryItem(ent,item);
	if (newItem) {
		newItem->quantity += count;
		newItem->maximum = max;
		if ((max > 0) && (newItem->quantity > max))
			newItem->quantity = max;
		if (depend)
			newItem->depend = (void *)depend;
		if (altcmd) {
			if (newItem->altcmd)
				gi.TagFree(newItem->altcmd);
			newItem->altcmd = DL_strcpy(NULL, altcmd, -1);
		}
		return newItem;
	}

	if ((max > 0) && (count > max))
		count = max;

	// Initialize item
	newItem = gi.TagMalloc(sizeof(item_t), TAG_LEVEL);
	newItem->itemtype = type;
	newItem->item = item;
	newItem->priority = priority;
	newItem->quantity = count;
	newItem->maximum = max;
	newItem->status = 0;
	newItem->heat = 0.0;
	newItem->depend = (void *)depend;

	if (altcmd) {
		newItem->altcmd = DL_strcpy(NULL, altcmd, -1);
	} else {
		newItem->altcmd = GetDefaultAltCmd(ent, newItem);
	}
	DL_DebugLog(va("AltCmd: (%s) %s [%s]\n", ent->classname, GetItemName(newItem), newItem->altcmd));

	newItem->prev = NULL;
	newItem->next = ent->inventory;

	// place in inventory
	if (ent->inventory)
		ent->inventory->prev = newItem;
	ent->inventory = newItem;

	return newItem;
}

item_t *RemoveInventoryItem(edict_t *ent, item_t *item, int num) {
item_t *pItem, *nItem;

	if (!item || !ent)
		return NULL;

	if ((num > 0) && (num < item->quantity)) {
		item->quantity-= num;
		return item;
	}

	// Check for other inventory items that are dependant
	// on this item and free t1ory; nItem; nItem = nItem->next)
	if (item->depend == item)
		item->depend = NULL;

	if (ent->client) {
		if (ent->client->curweap == item)
			ent->client->curweap = NULL;
		if (ent->client->nextweap == item)
			ent->client->nextweap = NULL;
	}

	if (ent->last_armour == item)
		ent->last_armour = NULL;

	if (ent->inventory == item)
		ent->inventory = item->next;

	// Now that everything is cleaned up, remove the item
	pItem = item->prev;
	nItem = item->next;

	DL_DebugLog(va("FUNC: RemoveInventoryItem: (%s) %s [%s]\n", ent->classname, GetItemName(item), item->altcmd));

	if (item->altcmd)
		gi.TagFree(item->altcmd);
	gi.TagFree(item);	

	if (pItem)
		pItem->next = nItem;
	if (nItem)
		nItem->prev = pItem;

	if (!nItem)
		return pItem;

	return nItem;
}

item_t *MoveToInventory(edict_t *from, item_t *item, int num, edict_t *to) {
item_t *dup;

	// remove from current inventory
	dup = AddToInventory(to, item->itemtype, item->item, 0, num, 0, NULL, NULL);
	RemoveInventoryItem(from, item, num);

	gi.cprintf(to, PRINT_HIGH, "You have: %d %s\n", dup->quantity, GetItemName(dup));
	return dup;
}

void ClearInventory(edict_t *ent) {
	if (!ent)
		return;

	while(RemoveInventoryItem(ent, ent->inventory, -1));
}

void CleanInventory(edict_t *ent) {
item_t	*curitem;

	if (!ent)
		return;

	for ( curitem = ent->inventory; curitem != NULL; curitem = curitem->next) {
		if (curitem->quantity <= 0)
			curitem = RemoveInventoryItem(ent, curitem, -1);

		if (!curitem) // was that the last item in the inventory?
			break;
	}
}

item_t *FindPrevWeapon(edict_t *ent, item_t *curweap){
	if (!ent)
		return NULL;
	if (!ent->inventory)
		return NULL;
	if (!curweap)
		curweap = ent->inventory;
	else
		curweap = curweap->prev;

	while (curweap && (curweap->itemtype != ITEM_WEAPON))
		curweap = curweap->prev;
	return curweap;
}

item_t *FindNextWeapon(edict_t *ent, item_t *curweap){
	if (!ent)
		return NULL;
	if (!ent->inventory)
		return NULL;
	if (!curweap)
		curweap = ent->inventory;
	else
		curweap = curweap->next;

	while (curweap && (curweap->itemtype != ITEM_WEAPON))
		curweap = curweap->next;
	return curweap;
}

item_t *FindPreferedWeapon(edict_t *ent){
item_t *curWeap, *bestWeap=NULL;

	for (curWeap = ent->inventory; curWeap; curWeap = curWeap->next)
		if (curWeap->itemtype == ITEM_WEAPON)
			if (!bestWeap || (curWeap->priority > bestWeap->priority))
				bestWeap = curWeap;

	return bestWeap;
}

qboolean IsAmmoCompatible(weap_t *weap, ammo_t *ammo) {

	if (!weap || !ammo)
		return false;

	if (!DL_strcmp(weap->base_ammo, ammo->base_name, -1, false))
		return true;
	if (!DL_strcmp(weap->base_ammo, ammo->name, -1, false))
		return true;
	return false;
}

ammo_t *FindPrevCompatibleAmmo(edict_t *ent, ammo_t *curammo){
item_t *nextAmmo;

	if (!curammo || !ent)
		return NULL;

	nextAmmo = FindItemByName(ent, curammo->name, ITEM_AMMO);
	if (nextAmmo)
		nextAmmo = nextAmmo->prev;
	else
		return NULL;

	if (!nextAmmo)
		nextAmmo = ent->inventory;

	while (nextAmmo && (ITEMAMMO(nextAmmo) != curammo)) {
		if (nextAmmo->itemtype == ITEM_AMMO) {
			if (!DL_strcmp(ITEMAMMO(nextAmmo)->base_name, curammo->base_name, -1, false) && nextAmmo->quantity > 0)
				return ITEMAMMO(nextAmmo);
			if (ITEMAMMO(nextAmmo) == curammo)
				return curammo;
		}
		// At end of inventory, jump to start and search again
		if (!nextAmmo->prev)
			while(nextAmmo->next)
				nextAmmo = nextAmmo->next;
		else
			nextAmmo = nextAmmo->prev;
	}

	return NULL;
}

ammo_t *FindNextCompatibleAmmo(edict_t *ent, ammo_t *curammo){
item_t *nextAmmo;

	if (!curammo || !ent)
		return NULL;

	nextAmmo = FindItemByName(ent, curammo->name, ITEM_AMMO);
	if (nextAmmo)
		nextAmmo = nextAmmo->next;
	else
		return NULL;

	if (!nextAmmo)
		nextAmmo = ent->inventory;

	while (nextAmmo && (ITEMAMMO(nextAmmo) != curammo)) {
		if (nextAmmo->itemtype == ITEM_AMMO) {
			if (!DL_strcmp(ITEMAMMO(nextAmmo)->base_name, curammo->base_name, -1, false) && nextAmmo->quantity > 0)
				return ITEMAMMO(nextAmmo);
			if (ITEMAMMO(nextAmmo) == curammo)
				return curammo;
		}
		// At end of inventory, jump to start and search again
		if (!nextAmmo->next)
			nextAmmo = ent->inventory;
		else
			nextAmmo = nextAmmo->next;
	}

	return NULL;
}

item_t *FindPreferedCompatibleAmmo(edict_t *ent, char *ammoType){
item_t *curAmmo, *bestAmmo=NULL;

	if (!ent || !ammoType)
		return NULL;

	for (curAmmo = ent->inventory; curAmmo; curAmmo = curAmmo->next)
		if ((curAmmo->itemtype == ITEM_AMMO) && (curAmmo->quantity > 0))
			if (!DL_strcmp(((ammo_t *)(curAmmo->item))->base_name, ammoType, -1, false))
				if (!bestAmmo || (curAmmo->priority > bestAmmo->priority))
				bestAmmo = curAmmo;

	return bestAmmo;
}

float	GetInventoryWeight(edict_t *ent) {
item_t	*curitem;
float	weight = 0.0;

	if (!ent)
		return 0.0;
	if (!ent->inventory)
		return 0.0;

	// Player weight is always 0 when playing in "no encumbrance" mode.
	if (dlsys.playmode & PM_NOENCUMBRANCE)
		return 0.0;

	for ( curitem = ent->inventory; curitem; curitem = curitem->next)
		if (curitem->quantity > 0)
			weight += GetItemWeight(curitem) * curitem->quantity;

	ent->mass = 200 + weight;	// recalc players mass
	return weight;
}

