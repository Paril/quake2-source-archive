#include "g_local.h"
#include "g_sogutil.h"

//Knightmare added
#include "g_sogcvar.h"

//======================================================================
//======================================================================
// Skid
// Quake / Doom Powerups
//======================================================================

void Use_Invis(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)] --;
	ValidateSelectedItem(ent);

	if (ent->client->invis_framenum > level.framenum)
		ent->client->invis_framenum += (invis_time->value * 10);
	else
		ent->client->invis_framenum = level.framenum + (invis_time->value * 10);

	if(ent->client->resp.player_class == CLASS_Q1)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("idg3items/powerups/invis.wav"), 1, ATTN_NORM, 0);
	else if(ent->client->resp.player_class == CLASS_DOOM)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("idg2items/powerups/invis.wav"), 1, ATTN_NORM, 0);
	else 
		gi.sound(ent, CHAN_ITEM, gi.soundindex("idg1items/powerups/invis.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_MegaSphere(edict_t *ent, gitem_t *item)
{
	//Knightmare added
	gitem_t		*it;
	edict_t		*it_ent;

	ent->client->pers.inventory[ITEM_INDEX(item)] --;
	ValidateSelectedItem(ent);

	switch(ent->client->resp.player_class)
	{
		case CLASS_WOLF:
		{
			//Knightmare- armor and soulsphere stacking
			//if stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				ent->health += 200;
				//cap at soul_health for class
				if (ent->health > idg1_soul_health->value)
					ent->health = idg1_soul_health->value;
			}
			else //if no stacking
				if (ent->health < 200)
					ent->health = 200;
			//Knightmare- add the armor instead of deleting the current and sticking 200 points in the inventory
			//that is, if armor stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				it = FindItem("Body Armor");
				it_ent = G_Spawn();
				it_ent->classname = it->classname;
				SpawnItem (it_ent, it);
				Touch_Item (it_ent, ent, NULL, NULL);
			}
			else //if no stacking
			{
				ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
				ent->client->pers.inventory[W_BLUE] = 200;
			}
			gi.sound(ent, CHAN_ITEM, gi.soundindex("idg1items/powerups/megah.wav"), 1, ATTN_NORM, 0);
			break;
		}
		case CLASS_DOOM:
		{

			//Knightmare- armor and soulsphere stacking, if stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				ent->health += 200;
				//cap at soul_health for class
				if (ent->health > idg2_soul_health->value)
					ent->health = idg2_soul_health->value;
			}
			else //if no stacking
				if (ent->health < 200)
					ent->health = 200;
			//Knightmare- add the armor instead of deleting the current and sticking 200 points in the inventory
			//that is, if armor stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				it = FindItem("Body Armor");
				it_ent = G_Spawn();
				it_ent->classname = it->classname;
				SpawnItem (it_ent, it);
				Touch_Item (it_ent, ent, NULL, NULL);
			}
			else //if no stacking
			{
				ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
				ent->client->pers.inventory[D_BLUE] = 200;
			}
			gi.sound(ent, CHAN_ITEM, gi.soundindex("idg2items/powerups/megah.wav"), 1, ATTN_NORM, 0);
			break;
		}
		case CLASS_Q1:
		{
			//Knightmare- armor and soulsphere stacking, if stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				ent->health += 200;
				//cap at soul_health for class
				if (ent->health > idg3_soul_health->value)
					ent->health = idg3_soul_health->value;
			}
			else //if no stacking
				if (ent->health < 200)
					ent->health = 200;
			//Knightmare- add the armor instead of deleting the current and sticking 200 points in the inventory
			//that is, if armor stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				it = FindItem("Body Armor");
				it_ent = G_Spawn();
				it_ent->classname = it->classname;
				SpawnItem (it_ent, it);
				Touch_Item (it_ent, ent, NULL, NULL);
			}
			else //if no stacking
			{
				ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
				ent->client->pers.inventory[Q1_RED] = 200;
			}
			gi.sound(ent, CHAN_ITEM, gi.soundindex("idg3items/health/large.wav"), 1, ATTN_NORM, 0);
			break;
		}
		case CLASS_Q2:
		{
			//Knightmare- armor and soulsphere stacking, if stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				ent->health += 200;
				//cap at soul_health for class
				if (ent->health > idg4_soul_health->value)
					ent->health = idg4_soul_health->value;
			}
			else //if no stacking
				if (ent->health < 200)
					ent->health = 200;
			//Knightmare- add the armor instead of deleting the current and sticking 200 points in the inventory
			//that is, if armor stacking is enabled
			if (old_guys_allow_armor_stacking->value)
			{
				it = FindItem("Body Armor");
				it_ent = G_Spawn();
				it_ent->classname = it->classname;
				SpawnItem (it_ent, it);
				Touch_Item (it_ent, ent, NULL, NULL);
			}
			else //if no stacking
			{
				ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
				ent->client->pers.inventory[1] = 200;
			}
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
			break;
		}

	}

}






//======================================================================

void Use_Berserk(edict_t *ent, gitem_t *item)
{
	if(ent->client->resp.player_class != CLASS_DOOM)
		return;

//	if(ent->flags & FL_D_BERSERK)
//		return;
	
	if(ent->health < ent->max_health)
		ent->health = ent->max_health;

	ent->flags |= FL_D_BERSERK;

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += 300;
	else
		ent->client->quad_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("idg2items/powerups/berserk.wav"), 1, ATTN_NORM, 0);
	if(ITEM_INDEX(ent->client->pers.weapon)!= D_FISTS)
		ent->client->newweapon = GetItemByIndex(D_FISTS);
	return;
}