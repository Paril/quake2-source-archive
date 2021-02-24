#include "g_local.h"
#include "m_player.h"

/*
=================
PutOnStorageItem - back pack or bandolier
=================
*/
void PutOnStorageItem (edict_t *ent)
{
	edict_t *si = ent->storage_item;
	edict_t *itemEnt;

	//gi.dprintf ("PutOnStorageItem -\n");
	
	if (si == NULL)
	{
		//gi.dprintf ("no storage item\n");
		return;
	}
	
	itemEnt = si->activator;
	while (itemEnt)
	{
		//gi.dprintf ("stashing -> %s\n", itemEnt->classname);
		StashItem (ent, itemEnt->item, itemEnt->last_hitloc, itemEnt->count, itemEnt->viewheight, itemEnt->last_fire);
		itemEnt->think = DroppedThink;
		itemEnt->nextthink = level.time + FRAMETIME;
		itemEnt = itemEnt->activator;
	}
	
	ent->storage_item->think = DroppedThink;
	ent->storage_item->nextthink = level.time + FRAMETIME;
	ent->storage_item = NULL;
}

/*
=================
UseArmour
=================
*/
qboolean UseArmour(edict_t *ent)
{
	int	area = -1;
	int 	i;
	
	qboolean armor = true;

	switch(ent->client->pers.cstats[CSTAT_RIGHTHAND])
	{
	case II_JACKET_ARMOUR:
		area = BA_CHEST_ARMOUR;
		break;

	case II_COMBAT_ARMOUR:
		area = BA_CHEST_ARMOUR;
		break;

	case II_BODY_ARMOUR:
		area = BA_CHEST_ARMOUR;
		break;

	case II_BANDOLIER:
		area = BA_BANDOLIER;
		armor = false;
		break;

	case II_BACK_PACK:
		area = BA_OVER_BACK;
		armor = false;
		break;

	default:
		break;
	}

	if (area == -1)
		return false;

	if (ent->client->pers.item_quantities[area] > 0)
	{
	        //gi.dprintf("UseArmour - item_quantities[%i] > %i\n", area, ent->client->pers.item_quantities[area]);
		return false;
	}
	
	if (armor)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ar1_pkup.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/pkup.wav"), 1, ATTN_NORM, 0);

	StashItem (ent, GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]), area, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO], ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);

	ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
	ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = 0;

	if (((area == BA_BANDOLIER) || (area == BA_OVER_BACK)) && (ent->storage_item != NULL))
		PutOnStorageItem(ent);

	return true;
}

/*
=================
Think_Armour

=================
*/
void Think_Armor(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                else if (ent->client->newLeftWeapon > -1)
                        ChangeLeftWeapon(ent);
		else if (ent->client->latched_buttons & BUTTON_ATTACK)
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (UseArmour(ent))
			{
        	                ChangeRightWeapon(ent);
	                	return;
			}
		}
                break;

        default:
                newState = W_READY;
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        default:
                break;
        }

        if (newFrame != -1)
                ent->client->ps.gunframe = newFrame;

        if (newState != -1)
                ent->client->weaponstate = newState;
}
