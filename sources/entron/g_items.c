#include "g_local.h"


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean Pickup_PKF_Pack (edict_t *ent, edict_t *other);

void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

// CCH
void Weapon_Hammer (edict_t *ent);
void Weapon_u23k (edict_t *ent);
void Weapon_u23k2 (edict_t *ent);
void Weapon_PlasmaGun (edict_t *ent);
void Weapon_Gattling (edict_t *ent);
void Weapon_Ripper (edict_t *ent);
void Weapon_Staff (edict_t *ent);
void Weapon_Assim (edict_t *ent);
void Weapon_CECCannon (edict_t *ent);
void Weapon_Sniper (edict_t *ent);
void Weapon_Wasp (edict_t *ent);

// Pasted from from IRC. Ghent says so.
// hammer, u23k, plasma, ripper, gattling laser cannon, 
// priest's staff of casting, assimilator, sniper, CEC, WASP

          
gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;
qboolean Pickup_Egg (edict_t *ent, edict_t *other);
qboolean Pickup_Bean (edict_t *ent, edict_t *other);
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
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
   gclient_t *client = other->client;
	int		index;


	if (client->pers.max_srounds < 20)
      client->pers.max_srounds = 20;

   if (client->pers.max_cells < 100)
      client->pers.max_cells = 100;

   if (client->pers.max_liquidfire < 100)
      client->pers.max_liquidfire = 100;

   if (client->pers.max_plasma < 100)
      client->pers.max_plasma = 100;

   if (client->pers.max_discs < 50)
      client->pers.max_discs = 50;

   if (client->pers.max_rockets < 50)
      client->pers.max_rockets = 50;

   if (client->pers.max_clip < 100)
      client->pers.max_clip = 100;

	item = FindItem("Clip");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_clip)
			other->client->pers.inventory[index] = other->client->pers.max_clip;
	   }

	item = FindItem("Sniper Rounds");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_srounds)
			other->client->pers.inventory[index] = other->client->pers.max_srounds;
	   }

   item = FindItem("CGW Cells");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	   }
/*
   item = FindItem("Detarium Spikes");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_detarium)
			other->client->pers.inventory[index] = other->client->pers.max_detarium;
	   }
*/

   item = FindItem("Manna");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_liquidfire)
			other->client->pers.inventory[index] = other->client->pers.max_liquidfire;
	   }

/*
   item = FindItem("WASP Energy Cells");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	   }
*/

   item = FindItem("Rockets");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	   }

   // CCH
	item = FindItem("Plasma Slugs");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_plasma)
			other->client->pers.inventory[index] = other->client->pers.max_plasma;
	   }

	// CCH
	item = FindItem("Ripper Discs");
	if (item)
	   {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_discs)
			other->client->pers.inventory[index] = other->client->pers.max_discs;
	   }

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

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
      if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
			return false;
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		return true;
	   }
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_CLIP)
		max = ent->client->pers.max_clip;
	else if (item->tag == AMMO_SROUNDS)
		max = ent->client->pers.max_srounds;
	else if (item->tag == AMMO_PLASMA)
		max = ent->client->pers.max_plasma;
	else if (item->tag == AMMO_DISCS)
		max = ent->client->pers.max_discs;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_LIQUIDFIRE)
		max = ent->client->pers.max_liquidfire;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;

	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

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
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("hammer") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];
	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
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
			return false;

   if (!ent->count)
      ent->count = 25;

	other->health += ent->count;

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
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

void ShowGun(edict_t *ent);
qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

   ShowGun(other);

	return true;
}

void Use_Goggles (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->IR_framenum > level.framenum)
		ent->client->IR_framenum += 300;
	else
      {
		ent->client->IR_framenum = level.framenum + 300;
      Cmd_FlashLight(ent, 1);
      }
}


//======================================================================

int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

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
		index = ITEM_INDEX(FindItem("Plasma Slugs"));
		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No plasma slugs for power armor.\n");
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

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

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
	//dropped->s.effects = item->world_model_flags;
	//dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, 0); //-15
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

	v = tv(-15,-15,0); //-15
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
		if (!strcmp(data+len-3, "pcx"))
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
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	//ent->s.effects = item->world_model_flags;
	//ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);
}

//======================================================================

gitem_t	itemlist[] = 
{
	{
		NULL
	},	

	{
		"item_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
      "i_bodyarmor",
      "Body Armor",
      3,
		0,
		NULL,
		IT_ARMOR,
		&bodyarmor_info,
		ARMOR_BODY,
      NULL
	},

	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
      "i_combatarmor",
      "Combat Armor",
      3,
		0,
		NULL,
		IT_ARMOR,
		&combatarmor_info,
		ARMOR_COMBAT,
      NULL
	},

	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
      "i_jacketarmor",
      "Jacket Armor",
      3,
		0,
		NULL,
		IT_ARMOR,
		&jacketarmor_info,
		ARMOR_JACKET,
      ""
	},

	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
      "i_jacketarmor",
      "Armor Shard",
      3,
		0,
		NULL,
		IT_ARMOR,
		NULL,
		ARMOR_SHARD,
      NULL
	},
/*
	{
		"item_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
      "i_powerscreen",
      "Power Screen",
      0,
		60,
		NULL,
		IT_ARMOR,
		NULL,
		0,
      NULL
	},

	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
      "i_powershield",
      "Power Shield",
      0,
		60,
		NULL,
		IT_ARMOR,
		NULL,
		0,
      "misc/power2.wav misc/power1.wav"
	},
*/
	{
		"weapon_hammer", 
		NULL,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Hammer,
		"misc/w_pkup.wav",
      NULL, 0,
		"models/weapons/v_hammer/tris.md2",
      "w_hammer",
      "Hammer",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

	{
		"weapon_u23k", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_u23k,
		"misc/w_pkup.wav",
		"models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_blast/tris.md2",
      "w_u23k",
      "u23k",
		0,
		RELOAD_U23K,
		"Clip",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

   {
		"weapon_u23k2", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_u23k2,
		"misc/w_pkup.wav",
		"models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_dual/tris.md2",
      "w_u23k2",
      "u23k2",
		0,
		RELOAD_U23K2,
		"Clip",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0, 
      NULL
	},

   {
		"weapon_plasmagun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PlasmaGun,
		"misc/w_pkup.wav",
		"models/weapons/g_plasmagun/tris.md2", EF_ROTATE,
		"models/weapons/v_plasmagun/tris.md2",
      "w_plasmagun",
      "Plasma Gun",
		0,
		RELOAD_PLASMAGUN,
		"Plasma Slugs",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      "weapons/plasmagun/shot1.wav weapons/plasmagun/shot2.wav"
	},

   {
		"weapon_gat",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Gattling,
		"misc/w_pkup.wav",
		"models/weapons/g_trigat/tris.md2", EF_ROTATE,
		"models/weapons/v_trigat/tris.md2",
      "w_gattling",
      "Gattling Gun",
		0,
		RELOAD_GATTLING,
		"CGW Cells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

   {
		"weapon_ripper", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ripper,
		"misc/w_pkup.wav",
		"models/weapons/g_ripper/tris.md2", EF_ROTATE,
		"models/weapons/v_ripper/tris.md2",
      "w_ripper",
      "Ripper",
		0,
		RELOAD_RIPPER,
		"Ripper Discs",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      "weapons/ripper/shot1.wav weapons/ripper/shot2.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

	{
		"weapon_staff",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Staff,
		"misc/w_pkup.wav",
		"models/weapons/g_staff/tris.md2", EF_ROTATE,
		"models/weapons/v_staff/tris.md2",
      "w_staff",
      "Staff",
		0,
		RELOAD_STAFF,
		"Manna",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

	{
		"weapon_assim", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Assim,
		"misc/w_pkup.wav",
		"models/weapons/g_assim/tris.md2", EF_ROTATE,
		"models/weapons/v_assim/tris.md2",
      "w_assimilator",
      "Assimilator",
		0,
		RELOAD_ASSIM,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

	{
		"weapon_cec",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_CECCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_cec/tris.md2", EF_ROTATE,
		"models/weapons/v_cec/tris.md2",
      "w_cec",
      "CEC",
		0,
		RELOAD_CEC,
		"CGW Cells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      "weapons/blastf1a.wav misc/lasfly.wav"
	},

	{
		"weapon_rifle", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sniper,
		"misc/w_pkup.wav",
		"models/weapons/g_rifle/tris.md2", EF_ROTATE,
		"models/weapons/v_rifle/tris.md2",
      "w_rifle",
      "Rifle",
		0,
		RELOAD_RIFLE,
		"Sniper Rounds",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

	{
		"weapon_wasp",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Wasp,
		"misc/w_pkup.wav",
		"models/weapons/g_wasp/tris.md2", EF_ROTATE,
		"models/weapons/v_wasp/tris.md2",
      "w_wasp",
      "WASP",
		0,
		RELOAD_WASP,
		"CGW Cells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
      NULL
	},

	{
		"ammo_clip",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/clip/tris.md2", 0,
		NULL,
      "a_clip",
      "Clip",
      3,
		16,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_CLIP,
      NULL
	},

	{
		"ammo_plasma_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pslugs/tris.md2", 0,
		NULL,
      "a_plasma_slugs",
      "Plasma Slugs",
      3,
		25,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PLASMA,
      NULL
	},
	
	{
		"ammo_discs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/discs/tris.md2", 0,
		NULL,
      "a_discs",
      "Ripper Discs",
      3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_DISCS,
      NULL
	},

	{
		"ammo_manna",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/manna/tris.md2", 0,
		NULL,
      "a_manna",
      "Manna",
      3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LIQUIDFIRE,
      NULL
	},

	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/tris.md2", 0,
		NULL,
      "a_rockets",
      "Rockets",
      3,
		12,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
      NULL
	},
	
	{
		"ammo_cgcells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cgcells/tris.md2", 0,
		NULL,
      "a_cgcells",
      "CGW Cells",
      3,
		25,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_CELLS,
      NULL
	},
 
	{
		"ammo_srounds",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/srounds/tris.md2", 0,
		NULL,
      "a_srounds",
      "Sniper Rounds",
      3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SROUNDS,
      NULL
	},

   /*
   {
      "item_quad",
      Pickup_Powerup,
      Use_Quad,
      Drop_General,
      NULL,
      "items/pkup.wav",
      "models/items/quaddama/tris.md2", EF_ROTATE,
      NULL,
      "p_quad",
      "Quad Damage",
      2,
      60,
      NULL,
      0,
      NULL,
      0,
      "items/damage.wav items/damage2.wav items/damage3.wav"
   },

	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
      "p_invulnerability",
      "Invulnerability",
      2,
		300,
		NULL,
		IT_POWERUP,
		NULL,
		0,
      "items/protect.wav items/protect2.wav items/protect4.wav"
	},

	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
      "p_silencer",
      "Silencer",
      2,
		60,
		NULL,
		IT_POWERUP,
		NULL,
		0,
      ""
	},

   {
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
		"p_envirosuit",
	   "Environment Suit",
		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
      "items/airout.wav"
	},

	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
     	"p_rebreather",
	   "Rebreather",
		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
      "items/airout.wav"
	},

	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
		"i_fixme",
	   "Ancient Head",
		2,
		60,
		NULL,
		0,
		NULL,
		0,
      ""
	},

	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
		"p_adrenaline",
    	"Adrenaline",
		2,
		60,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
		"p_bandolier",
    	"Bandolier",
		2,
		60,
		NULL,
		0,
		NULL,
		0,
      NULL
	},
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
		"i_pack",
    	"Ammo Pack",
		2,
		180,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"item_pkf_pack",
		Pickup_PKF_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
		"i_pack",
    	"PKF Ammo Pack",
		2,
		180,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_skull",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/skull/tris.md2", EF_ROTATE,
		NULL,
		"k_skull",
		"Skull Key",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_skeleton",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/skeleton/tris.md2", EF_ROTATE,
		NULL,
		"k_skeleton",
		"Skeleton Key",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_fuse",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/fuse/tris.md2", EF_ROTATE,
		NULL,
		NULL,
		"Fuse",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_key1",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key1/tris.md2", EF_ROTATE,
		NULL,
		NULL,
		"Security Key",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_key2",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key2/tris.md2", EF_ROTATE,
		NULL,
		NULL,
		"Data Key",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"key_key3",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key3/tris.md2", EF_ROTATE,
		NULL,
		NULL,
		"Key Card",
		2,
		0,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"item_egg",
		Pickup_Egg,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/egg/tris.md2", EF_ROTATE,
		NULL,
		"i_egg",
		"Egg",
		2,
		1,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		"item_bean",
		Pickup_Bean,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/jin_bean/tris.md2", EF_ROTATE,
		NULL,
		"i_bean",
		"Jin Bean",
		2,
		1,
		NULL,
		0,
		NULL,
		0,
      NULL
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/healing/large/tris.md2", 0,
		NULL,
		"i_health",
    	"Health",
		3,
		0,
		NULL,
		0,
		NULL,
		0,
      ""   
	},


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
	SpawnItem (self, FindItem ("Health"));
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
	SpawnItem (self, FindItem ("Health"));
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
	SpawnItem (self, FindItem ("Health"));
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
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
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

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
}


int script_Drop_Item (edict_t *ent)
{
   gitem_t *item = GetItemByIndex(ent->scr.cmd->ParamInt[0]);
   vec3_t forward, right;
   edict_t *dropped;
   if (!item)
      return 0;   	

   dropped = G_Spawn();
	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	//dropped->s.effects = item->world_model_flags;
	//dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->velocity, 20, 0, 7);
   VectorSet (dropped->mins, -15, -15, 0);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	AngleVectors (ent->s.angles, forward, right, NULL);
	VectorCopy (ent->s.origin, dropped->s.origin);
	VectorScale (forward, ent->scr.cmd->ParamInt[1], dropped->velocity);

	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

   return 1;
}

qboolean Pickup_PKF_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
   gclient_t *client = other->client;
	int		index;

	item = FindItem("Plasma Slugs");
	if (item)
	   {
		index = ITEM_INDEX(item);
		client->pers.inventory[index] += item->quantity;
		if (client->pers.inventory[index] > client->pers.max_plasma)
			client->pers.inventory[index] = client->pers.max_plasma;
	   }

	return true;
}

qboolean Pickup_Egg (edict_t *ent, edict_t *other)
{
	other->client->invincible = level.framenum + 300;
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 40);
	return true;
}

qboolean Pickup_Bean (edict_t *ent, edict_t *other)
{
	other->client->beserk = level.framenum + 300;
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 40);
	return true;
}
