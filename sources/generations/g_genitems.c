#include "g_local.h"
#include "g_genutil.h"


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
		ent->client->invis_framenum += 300;
	else
		ent->client->invis_framenum = level.framenum + 300;

	if(ent->client->resp.player_class == CLASS_Q1)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("q1items/inv1.wav"), 1, ATTN_NORM, 0);
	else if(ent->client->resp.player_class == CLASS_DOOM)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("ditems/getpow.wav"), 1, ATTN_NORM, 0);
	else 
	{
		float r =random();

		if(r < 0.3)
			gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/yeah.wav"), 1, ATTN_NORM, 0);
		else if(r < 0.6)
			gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/allright.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/yeah1.wav"), 1, ATTN_NORM, 0);
	}
}

//======================================================================

void Use_MegaSphere(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)] --;
	ValidateSelectedItem(ent);

	switch(ent->client->resp.player_class)
	{
		case CLASS_WOLF:
		{
			float r =random();

			if(ent->health < 200)
				ent->health=200;
			ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
			ent->client->pers.inventory[W_BLUE] = 200;
			if(r < 0.3)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/yeah.wav"), 1, ATTN_NORM, 0);
			else if(r < 0.6)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/allright.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/yeah1.wav"), 1, ATTN_NORM, 0);

			break;
		}
		case CLASS_DOOM:
		{
			ent->health=200;
			ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
			ent->client->pers.inventory[D_BLUE] = 200;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("ditems/getpow.wav"), 1, ATTN_NORM, 0);
			break;
		}
		case CLASS_Q1:
		{
			if(ent->health < 200)
				ent->health=200;
			ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
			ent->client->pers.inventory[Q1_RED] = 200;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("q1items/healthM.wav"), 1, ATTN_NORM, 0);
			break;
		}
		case CLASS_Q2:
		{
			if(ent->health < 200)
				ent->health=200;
			ent->client->pers.inventory[(ArmorIndex (ent))] = 0;
			ent->client->pers.inventory[1] = 200;
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
	
	if(ent->health < 100)
		ent->health=100;

	ent->flags |= FL_D_BERSERK;

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += 300;
	else
		ent->client->quad_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("ditems/getpow.wav"), 1, ATTN_NORM, 0);
	if(ITEM_INDEX(ent->client->pers.weapon)!= D_FISTS)
		ent->client->newweapon = GetItemByIndex(D_FISTS);
	return;
}

//======================================================================
//======================================================================
//======================================================================

qboolean Pickup_Rune (edict_t *ent, edict_t *other)
{
	if(gen_ctf->value)
	{
		if(CTFPickup_Tech(ent,other))
			return true;
		return false;
	}
	else
		other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}