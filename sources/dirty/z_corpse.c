#include "g_local.h"

/*
================
CorpseDrop_Weapon
================
*/
void CorpseDrop_Weapon (edict_t *searcher, edict_t *ent, gitem_t *weapon)
{
        edict_t         *dropped;
        weapons_t       *wep = NULL;
        int             index, ammo_index;

        if (searcher->client->weaponstate != WEAPON_READY) // Must be idle
                return;         

        index = ITEM_INDEX(weapon);

        wep = FindClientWeapon (ent, weapon->pickup_name, 1);

        // Stupid any crash check...
        if (wep == NULL)
                return;

        ammo_index = ITEM_INDEX(FindItem(wep->ammo));

        dropped = Drop_Item (searcher, weapon, ITEM_UNDEF);
        
        dropped->count = 0;

        if (weapon->flags & IT_RELOAD)
        {
                if (wep->quantity > 0)
                {
                        dropped->count = wep->quantity;
                        wep->quantity = 0;
                }
        }
        else if (ent->inventory[index] == 1)
        {
                if (ent->inventory[ammo_index] >= weapon->quantity)
                        dropped->count = weapon->quantity;
                else
                        dropped->count = ent->inventory[ammo_index];
                ent->inventory[ammo_index] -= dropped->count;
        }

        ent->inventory[index] --;

        if ((dropped->count > 0) && (Q_stricmp(wep->ammo, weapon->ammo)))
                dropped->alternate_ammo = wep->ammo;

        // If secondary weapon (eg Rifle's Grenade Launcher),
        // add second count if it has ammo
        if (weapon->scomp && wep->s_ammo)
        {
                if (wep->s_quantity > 0)
                        dropped->modtype = wep->s_quantity;
                /* FIX ME
                if (Q_stricmp(wep->ammo, item->ammo))
                        dropped->alternate_ammo = wep->ammo;
                */
        }

        wep->pickup_name = NULL;
        wep->ammo = NULL;
        wep->quantity = -1;

        if (dropped->flags & W_EMPTY)
                dropped->s.frame = 1;

        if (wep->damage)
                dropped->dmg = wep->damage;

        ValidateSelectedCorpseItem (searcher);
        Touch_Item (searcher, dropped, NULL, NULL); // Dirty
}

// GRIM
void Drop_Health (edict_t *ent, gitem_t *item);
void CorpseDrop_Health (edict_t *searcher, edict_t *ent, gitem_t *item)
{
        edict_t         *dropped;
        dropped = Drop_Item (searcher, item, ITEM_UNDEF);

        ent->inventory[ITEM_INDEX(item)]--;
        ValidateSelectedCorpseItem (searcher);
        Touch_Item (searcher, dropped, NULL, NULL); // Dirty
}

/*
================
CorpseDrop_Ammo
================
*/
void CorpseDrop_Ammo (edict_t *searcher, edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
        clips_t *clip;
        int     index, count, calc, sub, clipindex;

	index = ITEM_INDEX(item);
        dropped = Drop_Item (searcher, item, ITEM_UNDEF);

        if (item->flags & IT_CLIP)
        {
                ent->inventory[index]--;
                if (item->quantity > 0)
                {
                        sub = ent->inventory[index];

                        clip = FindClip(item->pickup_name);
                        clipindex = CLIP_INDEX(clip);

                        calc = ent->clips[clipindex];

                        count = calc - (item->quantity * sub);

                        ent->clips[clipindex] -= count;

                        dropped->count = count;
                }
                ValidateSelectedCorpseItem (searcher);
                return;
        }
        else if (ent->inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
                dropped->count = ent->inventory[index];
        ent->inventory[index] -= dropped->count;

        /*
        if ((dropped->item->flags & IT_AMMO) && (dropped->item->flags & IT_SPECIAL)
         && (dropped->count > 0) && (dropped->item->quantity > 1))
                dropped->s.frame = (5 - dropped->count);
        */

        ValidateSelectedCorpseItem (searcher);
        Touch_Item (searcher, dropped, NULL, NULL); // Dirty
}

/*
================
CorpseDrop_Armor
================
*/
void CorpseDrop_Armor (edict_t *searcher, edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

        dropped = Drop_Item (searcher, item, ITEM_UNDEF);

        if ((ent->armor) && (item == ent->armor))
        {
                dropped->count = ent->armor_index;
                ent->armor = NULL;
                ent->armor_index = 0;
        }
        else if ((ent->carried_armor) && (item == ent->carried_armor))
        {
                dropped->count = ent->carried_armor_index;
                ent->carried_armor = NULL;
                ent->carried_armor_index = 0;
        }

        if (ent->inventory[ITEM_INDEX(item)] == 1)
                ent->inventory[ITEM_INDEX(item)] = 0;
        else
                ent->inventory[ITEM_INDEX(item)] = 1;

        ValidateSelectedCorpseItem (searcher);
        Touch_Item (searcher, dropped, NULL, NULL); // Dirty
}


void SelectNextCorpseItem (edict_t *ent, int itflags)
{
        int             i, index;
	gitem_t		*it;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
                index = (ent->client->pers.selected_item + i)%MAX_ITEMS;
                
                if (ent->corpse->inventory[index] < 1)
			continue;
		it = &itemlist[index];

		if (!(it->flags & itflags))
			continue;

                ent->client->pers.selected_item = index;
		return;
	}
     
        ent->client->pers.selected_item = -1;
}

void SelectPrevCorpseItem (edict_t *ent, int itflags)
{
        int             i, index;
	gitem_t		*it;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
                index = (ent->client->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
                
                if (ent->corpse->inventory[index] < 1)
			continue;
		it = &itemlist[index];

		if (!(it->flags & itflags))
			continue;

                ent->client->pers.selected_item = index;
		return;
	}
     
        ent->client->pers.selected_item = -1;
}

void ValidateSelectedCorpseItem (edict_t *ent)
{
        if (ent->corpse->inventory[ent->client->pers.selected_item] > 0)
		return;		// valid

        SelectNextCorpseItem (ent, -1);
}


void CorpseDropItem (edict_t *ent)
{
        edict_t *dropped; // Dirty
        gitem_t *it;
        int     index, i; // Dirty

        ValidateSelectedCorpseItem (ent);

        if (ent->client->pers.selected_item == -1)
        {
                gi.cprintf (ent, PRINT_HIGH, "Nothing left.\n");
		return;
	}

        it = &itemlist[ent->client->pers.selected_item];

        index = ITEM_INDEX(it);

        // GRIM - 17/01/99
        if (it->drop == Drop_Health)
                CorpseDrop_Health (ent, ent->corpse, it);
        else if (it->flags & IT_AMMO)
        // GRIM - 17/01/99
                CorpseDrop_Ammo (ent, ent->corpse, it);
        else if (it->flags & IT_WEAPON)
                CorpseDrop_Weapon (ent, ent->corpse, it);
        else if (it->flags & IT_ARMOR)
                CorpseDrop_Armor (ent, ent->corpse, it);
        else
        {
                // Dirty
                dropped = Drop_Item (ent, it, ITEM_UNDEF);
                Touch_Item (ent, dropped, NULL, NULL); // Dirty
                // Dirty

                ent->corpse->inventory[index]--;
        }

        ent->client->showinventory = true;

        gi.WriteByte (svc_inventory);
        for (i=0 ; i<MAX_ITEMS ; i++)
                gi.WriteShort (ent->corpse->inventory[i]);
        gi.unicast (ent, false);
}
