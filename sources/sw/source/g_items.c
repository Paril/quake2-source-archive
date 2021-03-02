#include "g_local.h"

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

static int	jacket_armor_index;
static int	combat_armor_index;
static int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

static int	quad_drop_timeout_hack;
int num_items = 0;
item_table_t item_table[MAX_EDICTS];

//======================================================================

/*
===============
SpawnEmptyClip
===============
*/
void SpawnEmptyClip (edict_t *self, char *filename)
{
	edict_t *clip;

	clip = G_Spawn();
	clip->s.modelindex = gi.modelindex(filename);
	clip->solid = SOLID_NOT;
	clip->movetype = MOVETYPE_TOSS;
	clip->mass = 10;
	VectorCopy(self->s.origin, clip->s.origin);
	clip->avelocity[0] = 30;
	clip->avelocity[1] = 30;
	clip->avelocity[2] = 30;
	clip->velocity[0] = (random() * 200)-100;
	clip->velocity[1] = (random() * 200)-100;
	clip->velocity[2] = 300;
	clip->think = G_FreeEdict;
	clip->nextthink = level.time + 3;

	gi.linkentity(clip);
}

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

int	FindIndexByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return i;
	}

	return 0;
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

//ZOID
//in ctf, when we are weapons stay, only the master of a team of weapons
//is spawned
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
		{
			ent = master;
		}
		else
		{
//ZOID

			for (count = 0, ent = master; ent; ent = ent->chain, count++);

			choice = rand() % count;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++);
		}
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
		if (((int)dmflags->value & DF_INSTANT_ITEMS) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
		{
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

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

void Use_SaberCrystal (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	ent->saber_colour = item->tag;
}

void Use_MediKit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->medikit_framenum > level.framenum)
		ent->client->medikit_framenum += 50;
	else
		ent->client->medikit_framenum = level.framenum + 50;
}

void Use_GlowLamp (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->glowlamp_framenum > level.framenum)
		ent->client->glowlamp_framenum += 300;
	else
		ent->client->glowlamp_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

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

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
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
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index, clipindex;
	int			max;
	int			clipammo;

	if (!ent->client)
		return false;

//SWTC AMMO
	if (item->tag == AMMO_E11)
	{
		max = 4;
		clipammo = 20;
		clipindex = WEAP_RIFLE;
	}
	else if (item->tag == AMMO_DL44)
	{
		max = 5;
		clipammo = 25;
		clipindex = WEAP_PISTOL;
	}
	else if (item->tag == AMMO_T21)
	{
		max = 5;
		clipammo = 10;
		clipindex = WEAP_REPEATER;
	}
	else if (item->tag == AMMO_DISRUPTOR)
	{
		max = 4;
		clipammo = 3;
		clipindex = WEAP_DISRUPTOR;
	}
	else if (item->tag == AMMO_BOLTS)
	{
		max = 10;
		clipammo = 6;
		clipindex = WEAP_BOWCASTER;
	}
	else if (item->tag == AMMO_CMISS)
	{
		max = 4;
		clipammo = 3;
		clipindex = WEAP_MISSILETUBE;
	}
	else if (item->tag == AMMO_BMISS)
	{
		max = 4;
		clipammo = 6;
		clipindex = WEAP_WRISTROCKET;
	}
	else if (item->tag == AMMO_THERMALS)
	{
		max = 10;
		clipammo = 1;
		clipindex = 15;
	}
	else if (item->tag == AMMO_BACKPACK)
	{
		max = 4;
		clipammo = 70;
		clipindex = WEAP_BEAMTUBE;
	}
	else if (item->tag == AMMO_SNIPER)
	{
		max = 10;
		clipammo = 5;
		clipindex = WEAP_NIGHTSTINGER;
	}
	else
		return false;

	index = ITEM_INDEX(item);

	if(ent->client->pers.clipammo[clipindex] == 0)
	{
		ent->client->pers.clipammo[clipindex] = clipammo;
		if(count != 1000)
			count--;
	}

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
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
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

	if (ent->client->pers.weapon && 
		ent->client->pers.weapon->tag == AMMO_THERMALS &&
		item->tag == AMMO_THERMALS &&
		ent->client->pers.inventory[index] - dropped->count <= 0) {
		safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		G_FreeEdict(dropped);
		return;
	}

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

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

	other->health += ent->count;

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

//ZOID
	if ((ent->style & HEALTH_TIMED)
		&& !CTFHasRegeneration(other))
//ZOID
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
//	if (!ent->client)
//		return 0;
//
//	if (ent->client->pers.inventory[jacket_armor_index] > 0)
//		return jacket_armor_index;
//
//	if (ent->client->pers.inventory[combat_armor_index] > 0)
//		return combat_armor_index;
//
//	if (ent->client->pers.inventory[body_armor_index] > 0)
//		return body_armor_index;
//
	return 0;
}

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

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
//	if (!ent->client)
//		return POWER_ARMOR_NONE;
//
//	if (!(ent->flags & FL_POWER_ARMOR))
//		return POWER_ARMOR_NONE;
//
//	if (ent->client->pers.inventory[power_shield_index] > 0)
//		return POWER_ARMOR_SHIELD;
//
//	if (ent->client->pers.inventory[power_screen_index] > 0)
//		return POWER_ARMOR_SCREEN;
//
	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
//	int		index;
//
//	if (ent->flags & FL_POWER_ARMOR)
//	{
//		ent->flags &= ~FL_POWER_ARMOR;
//		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
//	}
//	else
//	{
//		index = ITEM_INDEX(FindItem("cells"));
//		if (!ent->client->pers.inventory[index])
//		{
//			safe_cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
//			return;
//		}
//		ent->flags |= FL_POWER_ARMOR;
//		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
//	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
//	int		quantity;
//
//	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
//
//	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
//
//	if (deathmatch->value)
//	{
//		if (!(ent->spawnflags & DROPPED_ITEM) )
//			SetRespawn (ent, ent->item->quantity);
//		// auto-use for DM only if we didn't already have one
//		if (!quantity)
//			ent->item->use (other, ent->item);
//	}
//
	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
//	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
//		Use_PowerArmor (ent, item);
//	Drop_General (ent, item);
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
//		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
//		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
//		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

/*		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->count == 2)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 10)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 25)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
			else // (ent->count == 100)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
		}
		else*/if (ent->item->pickup_sound)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
		}
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
	dropped->s.renderfx = RF_GLOW;
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
//		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
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
void setup_item_table (void)
{
	int i;

	if(!deathmatch->value)
		return;

	for(i=0; i<num_items; i++)
	{
//		item_table[i].node = Bot_Find_Nearest_Node (item_table[i].ent, NO_NODES);
		item_table[i].node = node_count;
		add_node_to_list (item_table[i].ent, node_count);
		node_count++;
		item_table[i].item_index = FindIndexByClassname (item_table[i].ent->classname);
	}
}

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
		if(item->classname)
		{
			item_table[num_items].ent = ent;
			num_items++;
		}
		if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
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
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}


// Saberonly will prevent weapons & ammo from spawning


	if ((int)saberonly->value)
	{
		G_FreeEdict (ent);
		return;
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

//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0))
	{
		G_FreeEdict(ent);
		return;
	}
//ZOID

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0) {
		ent->think = CTFFlagSetup;
	}
//ZOID
}

//======================================================================

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

//*******************************
//
//NEW WEAPONS FOR SWTC
//
//*******************************
/* weapon_blasterpistol (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blasterpistol", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		"models/weapons/g_pistol/tris.md2", 0,
		"models/weapons/v_pistol/tris.md2",
/* icon */		"w_pist",
/* pickup */	"Blaster",
		0,
		1,
		"DL44 clip",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_PISTOL,
		NULL,
		0,
/* precache */ "weapons/pistol/fire.wav"
	},

/*QUAKED weapon_blasterrifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_blasterrifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Rifle,
		"misc/w_pkup.wav",
		"models/weapons/g_trpr/tris.md2", 0,
		"models/weapons/v_trpr/tris.md2",
/* icon */		"w_trpr",
/* pickup */	"Trooper_Rifle",
		0,
		1,
		"E11 clip",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RIFLE,
		NULL,
		0,
/* precache */ "weapons/rifle/fire.wav"
	},

/*QUAKED weapon_repeater (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_repeater",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Repeater,
		"misc/w_pkup.wav",
		"models/weapons/g_rapid/tris.md2", 0,
		"models/weapons/v_rapid/tris.md2",
/* icon */		"w_rapid",
/* pickup */	"Repeater",
		0,
		1,
		"T21 clip",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_REPEATER,
		NULL,
		0,
/* precache */ "weapons/rapid/fire.wav"
	},

/*QUAKED weapon_bowcaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bowcaster",							//
		Pickup_Weapon,								// pickup
		Use_Weapon,									// use
		Drop_Weapon,								// drop
		Weapon_Bowcaster,							//
		"misc/w_pkup.wav",							// pickup_sound
		"models/weapons/g_arrow/tris.md2", 0,		// world_model
		"models/weapons/v_arrow/tris.md2",			// view_model
/* icon */		"w_arrw",							// icon
/* pickup */	"Bowcaster",						//
		0,											//
		1,											//
		"Bolts",									// ammo
		IT_WEAPON|IT_STAY_COOP,						//
		WEAP_BOWCASTER,								//
		NULL,										//
		0,											//
/* precache */ "weapons/arrow/fire.wav"
	},

/*QUAKED weapon_wristrocket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_wristrocket",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_WristRocket,
		"misc/w_pkup.wav",
		"models/weapons/g_wrstrkt/tris.md2", 0,
		"models/weapons/v_wrstrkt/tris.md2",
/* icon */		"w_wrstrkt",
/* pickup */	"Wrist_Rocket",
		0,
		1,
		"Dumb Fire missiles",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_WRISTROCKET,
		NULL,
		0,
/* precache */ "weapons/wrocket/fire.wav"
	},

/*QUAKED weapon_missiletube (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_missiletube",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_MissileTube,
		"misc/w_pkup.wav",
		"models/weapons/g_lgrckt/tris.md2", 0,
		"models/weapons/v_lgrckt/tris.md2",
/* icon */		"w_mtube",
/* pickup */	"Rocket_Launcher",
		0,
		1,
		"Concussion missiles",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MISSILETUBE,
		NULL,
		0,
/* precache */ "weapons/mtube/fire.wav"
	},

/*QUAKED weapon_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_disruptor",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Disruptor,
		"misc/w_pkup.wav",
		"models/weapons/g_biggun/tris.md2", 0,
		"models/weapons/v_biggun/tris.md2",
/* icon */		"w_dis",
/* pickup */	"Disruptor",
		0,
		1,
		"Disruptor clip",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_DISRUPTOR,
		NULL,
		0,
/* precache */ "weapons/disrupt/fire.wav"
	},

/*QUAKED weapon_sniper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sniper",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sniper,
		"misc/w_pkup.wav",
		"models/weapons/g_sniper/tris.md2", 0,
		"models/weapons/v_sniper/tris.md2",
/* icon */		"w_nstg",
/* pickup */	"Night_Stinger",
		0,
		1,
		"Sniper Ammo",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_NIGHTSTINGER,
		NULL,
		0,
/* precache */ "weapons/sniper/fire.wav"
	},

/*QUAKED weapon_beamtube (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_beamtube",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BeamTube,
		"misc/w_pkup.wav",
		"models/weapons/g_beam/tris.md2", 0,
		"models/weapons/v_beam/tris.md2",
/* icon */		"w_beam",
/* pickup */	"Beam_Tube",
		0,
		1,
		"BeamPack",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BEAMTUBE,
		NULL,
		0,
/* precache */ "weapons/beam/fire.wav"
	},

/*QUAKED ammo_thermal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_thermal",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Thermal,
		"misc/am_pkup.wav",
		"models/items/ammo/dets/tris.md2", 0,
		"models/weapons/v_thdet/tris.md2",
/* icon */		"a_thermal",
/* pickup */	"Thermals",
/* width */		3,
		4,
		"Thermals",
		IT_AMMO|IT_WEAPON,
		WEAP_THERMALS,
		NULL,
		AMMO_THERMALS,
/* precache */ "weapons/td/activate.wav weapons/td/throw.wav weapons/td/tick.wav"
	},

/*QUAKED weapon_hands (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hands",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Hands,
		"misc/w_pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"w_blast",
/* pickup */	"Hands",
		0,
		0,
		NULL,
		0,
		WEAP_HANDS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_saber (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_saber",
		Pickup_Weapon,
		Use_Weapon,
		NULL,
		Weapon_Saber,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_saber/tris.md2",
/* icon */		"w_sabr",
/* pickup */	"Lightsaber",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SABER,
		NULL,
		0,
/* precache */ "weapons/sabre/idle.wav weapons/sabre/on.wav weapons/sabre/off.wav weapons/sabre/strike1.wav weapons/sabre/strike2.wav weapons/sabre/swing1.wav weapons/sabre/swing2.wav weapons/sabre/swing3.wav weapons/sabre/swing4.wav"
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_nothing (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_nothing",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"Nothing",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_NOTHING,
/* precache */ ""
	},

//**************************
//CaRRaC SWTC Ammo --  START
//**************************

/*QUAKED ammo_dl44 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_dl44",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pistclip/tris.md2", 0,
		NULL,
/* icon */		"a_blaster",
/* pickup */	"DL44 clip",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_DL44,
/* precache */ ""
	},


/*QUAKED ammo_e11 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_e11",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/riflclip/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"E11 clip",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_E11,
/* precache */ ""
	},

	/*QUAKED ammo_t21 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_t21",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rapclip/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"T21 clip",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_T21,
/* precache */ ""
	},

/*QUAKED ammo_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_dis",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/discharg/tris.md2", 0,
		NULL,
/* icon */		"a_disruptor",
/* pickup */	"Disruptor clip",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_DISRUPTOR,
/* precache */ ""
	},

/*QUAKED ammo_bolts (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bolts",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/arrows/tris.md2", 0,
		NULL,
/* icon */		"a_arrows",
/* pickup */	"Bolts",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BOLTS,
/* precache */ ""
	},

/*QUAKED ammo_cmissiles (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cmiss",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/tris.md2", 0,
		NULL,
/* icon */		"a_rockets2",
/* pickup */	"concussion missiles",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CMISS,
/* precache */ ""
	},

/*QUAKED ammo_bmissiles (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bmiss",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/wrocket/tris.md2", 0,
		NULL,
/* icon */		"a_rockets1",
/* pickup */	"Dumb Fire missiles",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BMISS,
/* precache */ ""
	},

/*QUAKED ammo_detonators (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_dets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/dets/tris.md2", 0,
		NULL,
/* icon */		"a_thermal",
/* pickup */	"Detonators",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_DETONATORS,
/* precache */ ""
	},

/*QUAKED ammo_backpack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bpack",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/beamclip/tris.md2", 0,
		NULL,
/* icon */		"a_beampack",
/* pickup */	"BeamPack",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BACKPACK,
/* precache */ ""
	},

/*QUAKED ammo_sniper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_sniper",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/snipclip/tris.md2", 0,
		NULL,
/* icon */		"a_nightstinger",
/* pickup */	"Sniper Ammo",
/* width */		3,
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SNIPER,
/* precache */ ""
	},

//CaRRaC SWTC Ammo --  END

//CaRRaC SWTC Items -- START

/*QUAKED item_medikit (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_medikit",
		Pickup_Powerup,
		Use_MediKit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/healing/medium/tris.md2", 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"MediKit",
/* width */		2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_glowlamp (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_glowlamp",
		Pickup_Powerup,
		Use_GlowLamp,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"GlowLamp",
/* width */		2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_gill (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_gill",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Gill",
/* width */		2,
		3,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_bluecrystal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bluecrystal",
		Pickup_Powerup,
		Use_SaberCrystal,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Blue Crystal",
/* width */		2,
		3,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		CRYSTAL_BLUE,
/* precache */ ""
	},

/*QUAKED item_yellowcrystal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_yellowcrystal",
		Pickup_Powerup,
		Use_SaberCrystal,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Yellow Crystal",
/* width */		2,
		3,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		CRYSTAL_YELLOW,
/* precache */ ""
	},

/*QUAKED item_redcrystal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_redcrystal",
		Pickup_Powerup,
		Use_SaberCrystal,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Red Crystal",
/* width */		2,
		3,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		CRYSTAL_RED,
/* precache */ ""
	},

/*QUAKED item_greencrystal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_greencrystal",
		Pickup_Powerup,
		Use_SaberCrystal,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Green Crystal",
/* width */		2,
		3,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		CRYSTAL_GREEN,
/* precache */ ""
	},

//CaRRaC SWTC Items -- END

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	//
	// KEYS
	//
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

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
		0,
		NULL,
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
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
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	//
	// ARMOR
	//

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

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
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},

//ZOID
/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"models/objects/jawa/jawa.md2", EF_FLAG1,
		NULL,
/* icon */		"i_ctf1",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"models/objects/ewok/ewok.md2", EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech1",
/* pickup */	"Disruptor Shield",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
	},

/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech2",
/* pickup */	"Power Amplifier",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech3",
/* pickup */	"Time Accel",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
	},

/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"AutoDoc",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
//ZOID
	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
/*	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");*/
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
/*	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");*/
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
/*	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");*/
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
/*	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;*/
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

//	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
//	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
//	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
//	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
//	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}
