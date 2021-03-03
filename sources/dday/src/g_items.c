/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_items.c,v $
 *   $Revision: 1.21 $
 *   $Date: 2002/07/23 22:48:27 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Grenade (edict_t *ent);
//bcass start - TNT
void Weapon_TNT (edict_t *ent);
//bcass end
void Weapon_Mine (edict_t *ent);
void Weapon_Knife (edict_t *ent);
void Weapon_Binoculars(edict_t *ent);
void Weapon_Antidote(edict_t *ent);
void Weapon_Morphine(edict_t *ent);
void Weapon_Bandage(edict_t *ent);
void Weapon_Flamethrower(edict_t *ent);

//gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
//gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

//static int	jacket_armor_index;
static int	combat_armor_index;
//static int	body_armor_index;
//static int	power_screen_index;
//static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

//void Use_Quad (edict_t *ent, gitem_t *item);
//static int	quad_drop_timeout_hack;

//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}


/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

gitem_t *FindTeamItem (char *dllname, int position)  //faf:  added for team dll support.  Finds item by dll name and 'position'.  Not 100% sure if it works yet...
{
        int             i;
        gitem_t *it;

        it = itemlist;
        for (i=0 ; i<game.num_items ; i++, it++)
        {
                if (!it->position)
                        continue;
                if (it->position != position)
                        continue;
                if (!Q_stricmp(it->dllname, dllname))
                        return it;
        }

        return NULL;
}
//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}


//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		//if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		//{
		//	if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
		//		quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
		//	ent->item->use (other, ent->item);
		//}
	}
	WeighPlayer(other);
	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	if (!item)
		return; // out of ammo, switched before frame?

	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	WeighPlayer(ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	other->burnout = 0;
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	WeighPlayer(other);
	return true;
}


qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_30cal < 250)
		other->client->pers.max_30cal = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	WeighPlayer(other);
	return true;
}



qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 2)
		other->client->pers.max_grenades = 2;
//bcass start - TNT
	if (other->client->pers.max_tnt < 1)
		other->client->pers.max_tnt = 1;
//bcass end
	if (other->client->pers.max_30cal < 300)
		other->client->pers.max_30cal = 300;
	if (other->client->pers.max_slugs < 100)
		other->client->pers.max_slugs = 100;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_30cal)
			other->client->pers.inventory[index] = other->client->pers.max_30cal;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("USA Grenade");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;

	}

	item = FindItem("Potato Masher");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}


	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}



//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
//			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
//				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
//			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	WeighPlayer(other);
	return true;
}

//======================================================================



qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (!item)
	{
		gi.dprintf("WARNING: in Add_Ammo %s has NULL ammo\n", ent->client->pers.netname);
		return false;
	}


	if (item->tag == AMMO_PISTOL)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SUBMG)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
//bcass start - TNT
	else if (item->tag == AMMO_TNT)
		max = ent->client->pers.max_tnt;
//bcass end
	else if (item->tag == AMMO_HMG)
		max = ent->client->pers.max_30cal;
	else if (item->tag == AMMO_LMG)
		max = ent->client->pers.max_30cal;
	else if (item->tag == AMMO_RIFLE)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_SNIPER)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_FLAME)
		max = ent->client->pers.max_flame;
	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	WeighPlayer(ent);

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("colt45") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);

	WeighPlayer(other);
	
	return true;
	
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;
	int		grennum; //Wheaty: Temp variable

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];

	if (item->tag == AMMO_GRENADES)
		grennum = ent->client->pers.inventory[index];

	//Wheaty: Only drop ONE grenade
	if (item->tag == AMMO_GRENADES)
	{
		dropped->count = 1;
	}

	//Wheaty: Clear inventory of any grenades (even though you only drop 1)
	if (!item->tag == AMMO_GRENADES)
		ent->client->pers.inventory[index] -= dropped->count;
	else
		ent->client->pers.inventory[index] = 0;


	ValidateSelectedItem (ent);
	WeighPlayer(ent);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			if (other->burnout < level.time)
				return false;

	if (other->burnout)
	{
		if ((rand() % 100) < ent->count)
			other->burnout = 0;
        else
			other->burnout -= ent->count;
	} 
	other->health += ent->count;
//	if(heal_wounds->value) 
//	{
//		other->wound_location=0;
//		other->die_time=0;
//	}
	
	
	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
			ent->item->pickup_sound = "items/m_health.wav";
	
	
	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	if (!ent->client)
		return 0;

//	if (ent->client->pers.inventory[jacket_armor_index] > 0)
//		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

//	if (ent->client->pers.inventory[body_armor_index] > 0)
//		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
			return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{

		return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?
	if (other->client->resp.autopickup == false && strcmp(ent->item->classname, "hgrenade"))
		return;

	//pbowens: you can only have 1 helmet
	if (ent->item->pickup_name && !Q_stricmp(ent->item->pickup_name, "Helmet") )
	{
		if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
		{
//			gi.cprintf(other, PRINT_HIGH, "You already have a helmet!\n");
			return;
		}
	}

	// you cannot pickup more than 1 team weapon
	if (ent->item->guninfo && other->client->pers.inventory[ITEM_INDEX(ent->item)])
		return;

	if (ent->item->position != LOC_GRENADES && ent->count > 1 &&
		other->client->pers.inventory[ITEM_INDEX(ent->item)] ) // dont pick up if ammo already
		return;

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		// flash the screen
		//other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);

		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		//if (ent->item->use)
		//	other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

		gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}

}

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
//faf	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
		dropped->s.frame = 0;
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int		len;
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		else if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t *ent, gitem_t *item)
{
	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_ITEMS )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			if ( (item->flags == IT_AMMO) /*|| (strcmp(ent->classname, "weapon_flamethrower") == 0) */)
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
//faf	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);
}

//======================================================================

gitem_t	itemlist[MAX_ITEMS] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// ARMOR
	//


/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&combatarmor_info,
		ARMOR_COMBAT,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},


	//
	// WEAPONS 
	//


//bcass start - TNT
/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_tnt",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_TNT,
		"misc/am_pkup.wav",
		"models/weapons/g_tnt/tris.md2", 0,
		"models/weapons/v_tnt/tris.md2",
/* icon */		"w_tnt",
/* pickup */	"TNT",
/* width */		3,
		5,
		"tnt",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_TNT,
		LOC_TNT,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},
//bcass end

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
#if 0
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_mine (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mine",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Mine,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Mine",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		1,
		0,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},
#endif

	//
	// AMMO ITEMS
	//



/* weapon_knife
 
*/
{
       "weapon_knife", 
       Pickup_Weapon,
       Use_Weapon,                             //How to use
       Drop_Weapon,
       Weapon_Knife,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/g_knife/tris.md2", 0, 
       "models/weapons/v_knife/tris.md2",      //The models stuff
       "w_knife",                                    //Icon to be used
       "Knife",                                        //Pickup name
       0,
       1,
       "Knife",
       IT_WEAPON,
       NULL,
       0,
	   LOC_KNIFE,
	   0,
	   0,
	   0,
		0,
       "misc/fhit3.wav knife/fire.wav knife/hit.wav knife/pullout.wav"

		},

{ 
       "weapon_fists", 
       NULL,
       Use_Weapon,                             //How to use
       NULL,
       Weapon_Knife,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/g_helmet/tris.md2", 0, 
       "models/weapons/v_fists/tris.md2",      //The models stuff
       "w_fists",                                    //Icon to be used
       "Fists",                                        //Pickup name
       0,
       1,
       "fists",
       IT_WEAPON,
       NULL,
       0,
	   LOC_KNIFE,
	   0,
	   0,
	   0,
		0,
       "misc/fhit3.wav fists/fire.wav fists/hit.wav"

	   },
	   
/*QUAKED item_helmet (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_helmet",
		Pickup_Weapon,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/weapons/g_helmet/tris.md2", 0,
		NULL,
/* icon */		"w_helmet",
/* pickup */	"Helmet",
/* width */		2,
		0,
		NULL,
		IT_WEAPON,
		NULL,
		0,
		LOC_HELMET,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

/*QUAKED weapon_Binoculars (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_binoculars",
		Pickup_Weapon,
		Use_Weapon,
		NULL,
		Weapon_Binoculars,
		"items/pkup.wav",
		"models/items/g_binoculars/tris.md2", 0,
		"models/weapons/v_binoculars/tris.md2",
/* icon */		"w_binoc",
/* pickup */	"Binoculars",
/* width */		1,
		1,
		"Binoculars",
		IT_WEAPON,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		0.5,
		0,
		0,
/* precache */ ""
	},

/*QUAKED weapon_Morphine(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Morphine",
		Pickup_Weapon,
		Use_Weapon,
		NULL,
		Weapon_Morphine,
		"items/pkup.wav",
		"models/items/band/tris.md2", 0,
		"models/weapons/v_morphine/tris.md2",
/* icon */		"w_morphine",
/* pickup */	"Morphine",
/* width */		0,
		5,
		"Morphine",
		IT_WEAPON|IT_AMMO,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

	
/*QUAKED weapon_Flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flamethrower",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Flamethrower,
		"misc/w_pkup.wav",
		"models/weapons/g_flamethrower/tris.md2", 0,//0,
		"models/weapons/v_flamethrower/tris.md2",
/* icon */		"w_flame",
/* pickup */	"Flamethrower",
		0,
		1,
		"flame_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_FLAME,
		3,
		70,
		0,
		0,
/* precache */ "weapons/flamer/fire.wav"
	},

/*QUAKED ammo_map dday (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_napalm",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_flame",
/* pickup */	"flame_mag",
/* width */		3,
		FLAME_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_FLAME,
		0,
		0,
		0.25,
		0,
		0,
/* precache */ ""
	},


/*QUAKED weapon_Antidote (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Antidote",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Antidote,
		"items/pkup.wav",
		"models/items/band/tris.md2", 0,
		"models/weapons/v_binoculars/tris.md2",
/* icon */		"w_morphine",
/* pickup */	"Antidote",
/* width */		0,
		5,
		"Antidote",
		IT_WEAPON|IT_AMMO,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

#if 0

/*QUAKED weapon_Bandage (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Bandage",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Bandage,
		"items/pkup.wav",
		"models/items/band/tris.md2", 0,
		"models/weapons/v_knife/tris.md2",
/* icon */		"w_morphine",
/* pickup */	"Bandage",
/* width */		2,
		5,
		"Bandage",
		IT_WEAPON|IT_AMMO,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		0,
		0,
		0,
/* precache */ ""
	},
	
/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", 0,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		15,
		0,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ "items/airout.wav"
	},


/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		0,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},



/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		NULL,
		0,
		0,
		0,
		4,
		0,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		0,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

	//
	// KEYS
	//


/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
/* precache */ ""
	},



	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		NULL,
		0,
		LOC_SPECIAL,
		0,
		0,
		0,
		0,
/* precache */ ""
		},
#endif
	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
//	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
//	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
//	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
//	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}


void InitItems (void)
{
//	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
	gitem_t *it;
	int i,count=0;
	it=itemlist;

	for(i=0;i<=MAX_ITEMS;i++,it++)
		if(it && it->pickup_name) count++;
	game.num_items = count;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}

//	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
//	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	
}

//added by vjj
//These are the functions needed to spawn the builtin Quake items.
//This problem is compounded by the fact that we are trying to build
//two things at once - one is the general solution to allow us to make
//things in the general way we want and two, fixes to the code so that 
//it'll work properly. Of course, we wish to minimize the changes.
//To simplify our effort, we will place these in the spawns[] list.
//A developer would have to create these functions for the new items
//that would be developed and then add tha appropriate entries into
//both the spawns[] and itemlist[]. We will create an interface to 
//the spawns[] list and that will be the low level interface since 
//everything ends up in the spawns[] list from items to monsters
//to triggers, etc.

//These are simply the old calls to SpawnItem with the entity filled in.

void SP_item_armor_body(edict_t *self)

{
//    SpawnItem(self,FindItemByClassname("item_armor_body"));
}

void SP_item_armor_combat(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_armor_combat"));
}

void SP_item_armor_jacket(edict_t *self)
{
//    SpawnItem(self,FindItemByClassname("item_armor_jacket"));
}

void SP_item_armor_shard(edict_t *self)
{
//    SpawnItem(self,FindItemByClassname("item_armor_shard"));
}

//weapons





void SP_item_ammo_grenades(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades"));
}

void SP_item_weapon_mine(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mine"));
}
		

void SP_item_powerup_silencer(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_silencer"));
}

void SP_item_powerup_breather(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_breather"));
}

void SP_item_powerup_enviro(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_enviro"));
}

void SP_item_powerup_adrenaline(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_adrenaline"));
}

void SP_item_powerup_bandolier(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_bandolier"));
}

void SP_item_powerup_pack(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("item_pack"));
}

void SP_item_key_blue_key(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("key_blue_key"));
}

void SP_item_key_red_key(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("key_red_key"));
}
