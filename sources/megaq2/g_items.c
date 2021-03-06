/*
Copyright (C) 1997-2001 Id Software, Inc.

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

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);


//peewee start
//new weapon functions
void Weapon_Cutter (edict_t *ent);
void Weapon_Bomb (edict_t *ent);
void Weapon_Needle (edict_t *ent);
void Weapon_Crashb (edict_t *ent);
void Weapon_Dust (edict_t *ent);
void Weapon_Gemini (edict_t *ent);
void Weapon_Magnet (edict_t *ent);
void Weapon_Skulls (edict_t *ent);
void Weapon_Sabre (edict_t *ent);
//peewee end

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

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

//peewee start
//randomize respawns for powerups
/*void SetEnhRespawn (edict_t *ent, float delay, int type)
{
	int i;
	//i = rand() % 3;
	//i = 1;

	if (i == 0)
	{
		if (type == 1);
			ent->item = FindItem("Wall Kicks");
	}
	else if (i == 1)
	{
		if (type == 1);
			ent->item = FindItem("Air Slide");
	}
	else
	{
		if (type == 1);
			ent->item = FindItem("Silencer");
	}

	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;

	gi.linkentity (ent);
}*/
//peewee end
//======================================================================

//peewee start
//tank functions
void dietank(edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//remove tank enhancment
	ent->client->pers.enhance = ENH_NONE;

	//make explosion
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);

	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	ent->viewheight = 22;
	ent->mass = 200;

	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);

	VectorClear (ent->velocity);
	ent->velocity[2] += 300;


	//restore old model info
	ent->s.modelindex = ent->oldmodelindex;
	ent->s.modelindex2 = ent->oldmodel2index;
	ent->client->ps.gunindex = 	ent->oldgunindex;

	//restor health and armor
	ent->health = ent->oldhealth;
	ent->client->pers.inventory[ArmorIndex(ent)] = 0;
	ent->client->pers.inventory[ent->oldarmortype] = ent->oldarmorcount;

	ent->die = player_die;
}

void starttank (edict_t *ent)
{
	// reset some values
	ent->client->pers.charge = 0;

	ent->viewheight = 56;
	ent->mass = 400;
	ent->solid = SOLID_BBOX;
	ent->movetype = MOVETYPE_WALK;

	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, 64);

	VectorClear (ent->velocity);
	ent->velocity[2] += 300;

	//save old models
	ent->oldmodelindex = ent->s.modelindex;
	ent->oldmodel2index = ent->s.modelindex2;
	ent->oldgunindex = ent->client->ps.gunindex;

	//tank model
	ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	//no weapon model
	ent->s.modelindex2 = 0;
	ent->client->ps.gunindex = 0;

	//save old health and armor
	ent->oldhealth = ent->health;
	ent->oldarmorcount = ent->client->pers.inventory[ArmorIndex(ent)];
	ent->oldarmortype = ArmorIndex(ent);

	//health and armor
	ent->health = 200;
	ent->client->pers.inventory[ArmorIndex(ent)] = 0;
	ent->client->pers.inventory[body_armor_index] = 200;

	ent->die = dietank;
}


//peewee end



qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	//peewee start
	//powerup code is getting completely rewritten
	int respawntime;
	//int ptype = 0;


	//can't pickup a powerup if we already have one
	if (other->client->pers.enhance)
	{
		//display message and reset message time
		if (other->client->pers.msg_time < level.time)
		{
    		gi.cprintf (other, PRINT_HIGH, "You already have an Enhancment\n");
			other->client->pers.msg_time = level.time + 1.5;
		}


		return false;
	}

	if (ent->item == FindItem("Double Buster"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Secondary X-Buster Enhancement - (Use the +use command to fire)\n");
		other->client->pers.enhance = ENH_BLAST;

		//ptype = 1;
		respawntime = 30;
	}
	else if (ent->item == FindItem("Air Slide"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Air Sliding Enhancement - (Use the +use command while jumping)\n");
		other->client->pers.enhance = ENH_SLIDE;

		//ptype = 1;
		respawntime = 30;
	}
	else if (ent->item == FindItem("Wall Kicks"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Wall Kick Enhancement - (Use the +use command to kick off walls)\n");
		other->client->pers.enhance = ENH_WALLK;

		//ptype = 1;
		respawntime = 30;
	}
	else if (ent->item == FindItem("E-Tank"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Energy Tank Enhancement - (Pick up additional health and press +use to heal)\n");
		other->client->pers.enhance = ENH_CAPSL;

		respawntime = 30;
	}
	else if (ent->item == FindItem("Super Armor"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Super Armor Enhancement - (Restore armor by taking damage and press +use to cause an explosion)\n");
		other->client->pers.enhance = ENH_ARMOR;

		//clear existing armor values
		other->client->pers.inventory[ArmorIndex(other)] = 0;
		other->client->pers.inventory[body_armor_index] = 10;

		respawntime = 60;
	}
	else if (ent->item == FindItem("Riding Tank"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Riding Tank - (Press +use to fire rockets)\n");
		other->client->pers.enhance = ENH_RTANK;

		//start tank info
		starttank(other);

		respawntime = 60;
	}
	else if (ent->item == FindItem("Phase Shield"))
	{
   		gi.cprintf (other, PRINT_HIGH, "Phase Shields - (Temporarily phase out of existance when hit)\n");
		other->client->pers.enhance = ENH_PHASE;

		respawntime = 45;
	}


	//respawn the powerup
	//if (ptype == 0)
		SetRespawn (ent, respawntime);
	//else
	//	SetEnhRespawn (ent, respawntime, ptype);

	return true;
	//peewee end
/*	int		quantity;

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

	return true;*/
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

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
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
	if (other->client->pers.max_grenades < 100)
		other->client->pers.max_grenades = 100;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
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
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
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

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
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
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
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
		ent->client->pers.weapon->tag == AMMO_GRENADES &&
		item->tag == AMMO_GRENADES &&
		ent->client->pers.inventory[index] - dropped->count <= 0) {
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
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
	//peewee start
	//tank cannot pickup health
	if (other->client->pers.enhance == ENH_RTANK)
		return false;
	//peewee end

	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
		{
			//peewee start
			//if we have capsule enhancement
			if (other->client->pers.enhance == ENH_CAPSL)
			{
				if (other->client->pers.charge_2 >= 99)
					return false;

				//add count to reserve
				other->client->pers.charge_2 += ent->count;
				//if reserve is greater than 99
				if (other->client->pers.charge_2 > 99)
					other->client->pers.charge_2 = 99; //reset to 99

				//now let the item respawn
				if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
					SetRespawn (ent, 30);

				return true;
			}
			else
				return false;
			//peewee end
		}

	other->health += ent->count;

	//peewee start
	//hearts (stimpacks) can raise maxhealth

	//if count is 2 (stimp pack) and
	//player has 118 or less then increase max_health
	if (ent->count == 2 && other->max_health <= 118)
	{
		other->max_health += 2;
	}
	//peewee end

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

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

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

	int				superarmorcount;

	//peewee start
	//tank cannot pickup health
	if (other->client->pers.enhance == ENH_RTANK)
		return false;
	//peewee end

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	//peewee start
	//super armor adds different ammounts based on type
	//jacket
	if (other->client->pers.enhance == ENH_ARMOR)
	{
		//cannot pick up item shards
		if (ent->item->tag == ARMOR_SHARD)
			return false;

		//otherwise determine different ammounts
		if (newinfo->base_count == 25)
			superarmorcount = 10;
		else if (newinfo->base_count == 50)
			superarmorcount = 20;
		else if (newinfo->base_count == 100)
			superarmorcount = 40;
	}
	//peewee end

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
		//peewee start
		//armor has different count with super armor
		if (other->client->pers.enhance == ENH_ARMOR)
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = superarmorcount;
		else
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
		//armor has different count with super armor
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
			//peewee start
			if (other->client->pers.enhance == ENH_ARMOR)
			{
				//do not decrease old armor
				newcount = superarmorcount + other->client->pers.inventory[old_armor_index];


				//limits for super armor change
				if (newcount > 200)
					newcount = 200;
			}
			else
			{
				// calc new armor values
				salvage = oldinfo->normal_protection / newinfo->normal_protection;
				salvagecount = salvage * other->client->pers.inventory[old_armor_index];
				newcount = newinfo->base_count + salvagecount;

				if (newcount > newinfo->max_count)
					newcount = newinfo->max_count;
			}

			//peewee end

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{

			//peewee start
			//limits for super armor change
			if (other->client->pers.enhance == ENH_ARMOR)
			{
				//do not decrease old armor
				newcount = superarmorcount + other->client->pers.inventory[old_armor_index];

				if (newcount > 200)
					newcount = 200;

				if (other->client->pers.inventory[old_armor_index] >= 200)
					return false;
			}
			else
			{
				// calc new armor values
				salvage = newinfo->normal_protection / oldinfo->normal_protection;
				salvagecount = salvage * newinfo->base_count;
				newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

				if (newcount > oldinfo->max_count)
					newcount = oldinfo->max_count;

				if (other->client->pers.inventory[old_armor_index] >= newcount)
					return false;
			}


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
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

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

		if (ent->item->pickup == Pickup_Health)
		{
			//peewee start
			if (ent->count == 2)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/fill2.wav"), 1, ATTN_NORM, 0);
				//gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 10)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/fill2.wav"), 1, ATTN_NORM, 0);
				//gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 25)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/fill1.wav"), 1, ATTN_NORM, 0);
				//gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
			else // (ent->count == 100)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/item1.wav"), 1, ATTN_NORM, 0);
				//gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
			//peewee end
		}
		//peewee start
		else if (ent->item->pickup == Pickup_Ammo)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex("items/fill2.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->item->pickup == Pickup_Weapon ||
				 ent->item->pickup == Pickup_Powerup)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex("items/weap.wav"), 1, ATTN_NORM, 0);
		}
		//peewee end
		else if (ent->item->pickup_sound)
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


		//peewee start
	//change skinnum for items
	//peewee start
	//change skinnum for items
	if (strcmp(dropped->classname, "ammo_bullets") == 0)
	{
		dropped->s.skinnum = 1;
	}
	else if (strcmp(dropped->classname, "ammo_grenades") == 0)
	{
		dropped->s.skinnum = 2;
	}
	else if (strcmp(dropped->classname, "ammo_rockets") == 0)
	{
		dropped->s.skinnum = 3;
	}
	else if (strcmp(dropped->classname, "ammo_cells") == 0)
	{
		dropped->s.skinnum = 4;
	}
	else if (strcmp(dropped->classname, "ammo_slugs") == 0)
	{
		dropped->s.skinnum = 5;
	}

	//change skinnum for weapons
	if (strcmp(dropped->classname, "weapon_supershotgun") == 0)
	{
		dropped->s.skinnum = 1;
	}
	else if (strcmp(dropped->classname, "weapon_machinegun") == 0)
	{
		dropped->s.skinnum = 2;
	}
	else if (strcmp(dropped->classname, "weapon_chaingun") == 0)
	{
		dropped->s.skinnum = 3;
	}
	else if (strcmp(dropped->classname, "weapon_grenadelauncher") == 0)
	{
		dropped->s.skinnum = 4;
	}
	else if (strcmp(dropped->classname, "weapon_rocketlauncher") == 0)
	{
		dropped->s.skinnum = 5;
	}
	else if (strcmp(dropped->classname, "weapon_hyperblaster") == 0)
	{
		dropped->s.skinnum = 6;
	}
	else if (strcmp(dropped->classname, "weapon_railgun") == 0)
	{
		dropped->s.skinnum = 7;
	}



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
//peewee start
//spawn monster function
void SP_monster_soldier(edict_t *self);
void SP_monster_berserk(edict_t *self);
void SP_monster_soldier_ss(edict_t *self);
void SP_monster_infantry(edict_t *self);
void SP_monster_flyer(edict_t *self);
void SP_monster_gunner(edict_t *self);
void SP_monster_chick(edict_t *self);
void SP_monster_medic(edict_t *self);
void SP_monster_gladiator(edict_t *self);

void ItemSpawnMonster(edict_t *ent, edict_t *monster, gitem_t *item, char *spawnname)
{
	int temp;

	monster=G_Spawn();

	ent->item = item;
	monster->activator = ent;
	monster->item = item;

	monster->classname = spawnname;
	VectorCopy(ent->s.origin, monster->s.origin);
	monster->s.origin[2] += 32;
	gi.linkentity(monster);

	skill->value=3;
	temp = deathmatch->value;
	deathmatch->value = 0;

	if (strcmp(spawnname, "Shotgun") == 0)
	{
		SP_monster_soldier(monster);
	}
	else if (strcmp(spawnname, "Super Shotgun") == 0)
	{
		SP_monster_berserk(monster);
	}
	else if (strcmp(spawnname, "Machinegun") == 0)
	{
		SP_monster_soldier_ss(monster);
	}
	else if (strcmp(spawnname, "Chaingun") == 0)
	{
		SP_monster_infantry(monster);
	}
	else if (strcmp(spawnname, "Grenades") == 0)
	{
		SP_monster_flyer(monster);
	}
	else if (strcmp(spawnname, "Grenade Launcher") == 0)
	{
		SP_monster_gunner(monster);
	}
	else if (strcmp(spawnname, "Rocket Launcher") == 0)
	{
		SP_monster_chick(monster);
	}
	else if (strcmp(spawnname, "Hyperblaster") == 0)
	{
		SP_monster_medic(monster);
	}
	else if (strcmp(spawnname, "Railgun") == 0)
	{
		SP_monster_gladiator(monster);
	}

	deathmatch->value = temp;
}


void ItemSpawnMonsterTimer(edict_t *ent)
{
	SpawnItem(ent->activator, ent->item);
	G_FreeEdict(ent);
}
//peewee end


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

	//peewee start
	//change skinnum for items
	if (strcmp(ent->classname, "ammo_bullets") == 0)
	{
		ent->s.skinnum = 1;
	}
	else if (strcmp(ent->classname, "ammo_grenades") == 0)
	{
		ent->s.skinnum = 2;
	}
	else if (strcmp(ent->classname, "ammo_rockets") == 0)
	{
		ent->s.skinnum = 3;
	}
	else if (strcmp(ent->classname, "ammo_cells") == 0)
	{
		ent->s.skinnum = 4;
	}
	else if (strcmp(ent->classname, "ammo_slugs") == 0)
	{
		ent->s.skinnum = 5;
	}

	//change skinnum for weapons
	if (strcmp(ent->classname, "weapon_supershotgun") == 0)
	{
		ent->s.skinnum = 1;
	}
	else if (strcmp(ent->classname, "weapon_machinegun") == 0)
	{
		ent->s.skinnum = 2;
	}
	else if (strcmp(ent->classname, "weapon_chaingun") == 0)
	{
		ent->s.skinnum = 3;
	}
	else if (strcmp(ent->classname, "weapon_grenadelauncher") == 0)
	{
		ent->s.skinnum = 4;
	}
	else if (strcmp(ent->classname, "weapon_rocketlauncher") == 0)
	{
		ent->s.skinnum = 5;
	}
	else if (strcmp(ent->classname, "weapon_hyperblaster") == 0)
	{
		ent->s.skinnum = 6;
	}
	else if (strcmp(ent->classname, "weapon_railgun") == 0)
	{
		ent->s.skinnum = 7;
	}

	//spawn monsters
	//currently spawning monsters will be removed
	//edict_t *monster;
/*	if (strcmp(ent->classname, "weapon_shotgun") == 0)
	{
		ItemSpawnMonster(ent, monster, item, "Shotgun");
	}
	else if (strcmp(ent->classname, "weapon_supershotgun") == 0)
	{
		ItemSpawnMonster(ent, monster, item, "Super Shotgun");
	}
	else if (strcmp(ent->classname, "weapon_machinegun") == 0)
	{
		ItemSpawnMonster(ent, monster, item, "Machinegun");
	}
	else if (strcmp(ent->classname, "weapon_chaingun") == 0)
	{
		ItemSpawnMonster(ent, monster, item, "Chaingun");
	}
	else if (strcmp(ent->classname, "ammo_grenades") == 0)
	{
		ItemSpawnMonster(ent, monster, item, "Grenades");
	}*/
/*	if (item == FindItem("Grenade Launcher"))
	{
		ItemSpawnMonster(ent, monster, item, "Grenade Launcher");
	}
	else if (item == FindItem("Rocket Launcher"))
	{
		ItemSpawnMonster(ent, monster, item, "Rocket Launcher");
	}
	else if (item == FindItem("Hyperblaster"))
	{
		ItemSpawnMonster(ent, monster, item, "Hyperblaster");
	}
	else if (item == FindItem("Railgun"))
	{
		ItemSpawnMonster(ent, monster, item, "Railgun");
	}*/
	//peewee end
	//else
	//{
		ent->item = item;
		ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
		ent->think = droptofloor;
		ent->s.effects = item->world_model_flags;
		ent->s.renderfx = RF_GLOW;
		if (ent->model)
			gi.modelindex (ent->model);
	//}
}

//======================================================================

gitem_t	itemlist[] =
{
	{
		NULL
	},	// leave index 0 alone

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


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
//		Pickup_PowerArmor,
//peewee start
		Pickup_Powerup,
//peewee end
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
//		"models/items/armor/shield/tris.md2", EF_ROTATE,
		"models/items/phase/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"i_powershield",
/* icon */		"i_phase",
///* pickup */	"Power Shield",
/* pickup */	"Phase Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS
	//

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		//"models/weapons/v_blast/tris.md2",
		"models/weapons/v_buster/tris.md2",
///* icon */		"w_blaster",
/* icon */		"w_buster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
///* precache */ "weapons/blastf1a.wav misc/lasfly.wav models/objects/plasma/tris.md2 models/objects/plasma2/tris.md2 models/objects/plasma3/tris.md2 models/objects/r_explode/tris.md2"
/* precache */ "weapons/blastf1a.wav models/objects/plasma/tris.md2 models/objects/plasma2/tris.md2 models/objects/plasma3/tris.md2 weapons/bust.wav weapons/charge1.wav weapons/charge2.wav weapons/charge3.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
		//Weapon_Shotgun,
		Weapon_Dust,
//peewee end
		"misc/w_pkup.wav",
//peewee start
		//"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
		//"models/weapons/v_shotg/tris.md2",
		"models/weapons/v_buster2/tris.md2",
//peewee end
///* icon */		"w_shotgun",
/* icon */		"w_dust",
///* pickup */	"Shotgun",
		"Dust Crusher",
		0,
		1,
		"White Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav models/objects/dust/tris.md2 models/objects/debris1/tris.md2 models/objects/debris2/tris.md2 models/objects/debris3/tris.md2"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_SuperShotgun,
		Weapon_Skulls,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_shotg2/tris.md2",
		"models/weapons/v_buster3/tris.md2",
//peewee end
///* icon */		"w_sshotgun",
/* icon */		"w_skull",
///* pickup */	"Super Shotgun",
		"Skull Shield",
		0,
		2,
		"White Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
//peewee start
/* precache */ "weapons/sshotf1b.wav models/objects/gibs/skull/tris.md2 models/objects/gibs/sm_meat/tris.md2"
//peewee end
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_Machinegun,
		Weapon_Crashb,
//peewee end
		"misc/w_pkup.wav",

//peewee start
//		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_machn/tris.md2",
		"models/weapons/v_buster4/tris.md2",
//peewee end
///* icon */		"w_machinegun",
/* icon */		"w_crash",
///* pickup */	"Machinegun",
		"Crash Bomb",
		0,
		1,
//peewee start
//		"Bullets",
		"Green Energy",
//peewee end
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
//peewee start
///* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav models/objects/crashb/tris.md2 weapons/grenlb1b.wav"
		"models/objects/crashb/tris.md2 weapons/crash.wav"
//peewee end
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_Chaingun,
		Weapon_Cutter,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_chain/tris.md2",
		"models/weapons/v_buster5/tris.md2",
//peewee end
///* icon */		"w_chaingun",
/* icon */		"w_cutter",
///* pickup */	"Chaingun",
		"Rolling Cutter",
		0,
		1,
		"Gray Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
///* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav weapons/chngnd1a.wav"
		"models/objects/cutter/tris.md2 weapons/cutter.wav"
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,

//		Use_Weapon,
		NULL,

 		Drop_Ammo,

//		Weapon_Grenade,
		NULL,

		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end

		//"models/weapons/v_handgr/tris.md2",
		NULL,

/* icon */		"a_wenergy3",
///* icon */		"a_grenades",
///* pickup */	"Grenades",
/* pickup */	"Green Energy",
/* width */		3,
//		5,
		10,

//		"grenades",
		NULL,

//		IT_AMMO|IT_WEAPON,
		IT_AMMO,

//		WEAP_GRENADES,
		0,

		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_GrenadeLauncher,
		Weapon_Bomb,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_launch/tris.md2",
		"models/weapons/v_buster6/tris.md2",
//peewee end
///* icon */		"w_glauncher",
/* icon */		"w_bomb",
///* pickup */	"Grenade Launcher",
		"Hyper Bomb",
		0,
		1,
		"Green Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav models/objects/bombs/tris.md2"
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_RocketLauncher,
		Weapon_Magnet,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_rocket/tris.md2",
		"models/weapons/v_buster7/tris.md2",
//peewee end
///* icon */		"w_rlauncher",
/* icon */		"w_magnet",
///* pickup */	"Rocket Launcher",
		"Magnet Missile",
		0,
		1,
		"Red Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
///* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
		"models/objects/magnet/tris.md2 weapons/magnet.wav"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_HyperBlaster,
		Weapon_Needle,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
		//"models/weapons/v_hyperb/tris.md2",
		"models/weapons/v_buster8/tris.md2",
//peewee end
///* icon */		"w_hyperblaster",
/* icon */		"w_needle",
///* pickup */	"HyperBlaster",
		"Needle Cannon",
		0,
		1,
		"Yellow Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
///* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav
		"models/objects/needle/tris.md2"
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_Railgun,
		Weapon_Gemini,
//peewee end
		"misc/w_pkup.wav",
//peewee start
//		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/items/special/tris.md2", EF_ROTATE,
//		"models/weapons/v_rail/tris.md2",
		"models/weapons/v_buster9/tris.md2",
//peewee end
///* icon */		"w_railgun",
/* icon */		"w_gemini",
///* pickup */	"Railgun",
		"Gemini Laser",
		0,
		1,
		"Blue Energy",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
///* precache */ "weapons/rg_hum.wav"
		"weapons/gemini.wav weapons/geminihit.wav"
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
//peewee start
//		Weapon_BFG,
		Weapon_Sabre,
//peewee end
		"misc/w_pkup.wav",
		//"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/items/g_sabre/tris.md2", EF_ROTATE,
//peewee start
//		"models/weapons/v_bfg/tris.md2",
		"models/weapons/v_sabre/tris.md2",
//peewee end
///* icon */		"w_bfg",
/* icon */		"w_sabre",
///* pickup */	"BFG10K",
		"Beam Sword",
		0,
//peewee start
//		50,
		0,
//		"Cells",
		"NULL",
//		NULL,
//peewee end
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
///* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	"weapons/saber1.wav models/objects/sblast/tris.md2"
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/shells/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end
		NULL,
/* icon */		"a_wenergy1",
///* icon */		"a_shells",
/* pickup */	"White Energy",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/bullets/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end
		NULL,
/* icon */		"a_wenergy2",
///* icon */		"a_bullets",
///* pickup */	"Bullets",
/* pickup */	"Gray Energy",
/* width */		3,
//peewee start
//		50,
		10,
//peewee end
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/cells/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end
		NULL,
/* icon */		"a_wenergy5",
///* icon */		"a_cells",
/* pickup */	"Yellow Energy",
/* width */		3,
//peewee start
//		50,
		25,
//peewee end
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/rockets/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end
		NULL,
/* icon */		"a_wenergy4",
///* icon */		"a_rockets",
/* pickup */	"Red Energy",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		//peewee start
//		"models/items/ammo/slugs/medium/tris.md2", 0,
		"models/items/wenergy/tris.md2", 0,
		//peewee end
		NULL,
/* icon */		"a_wenergy6",
///* icon */		"a_slugs",
/* pickup */	"Blue Energy",
/* width */		3,
		15,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},


	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad",
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"p_quad",
/* icon */		"i_tank",
/* pickup */	"Riding Tank",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"p_invulnerability",
/* icon */		"i_sarmor",
/* pickup */	"Super Armor",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
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
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"p_silencer",
/* icon */		"i_2buster",
///* pickup */	"Silencer",
/* pickup */	"Double Buster",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
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
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
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
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

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

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
//		Pickup_Adrenaline,
//peewee start
		Pickup_Powerup,
//peewee end
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
//peewee start
		//"models/items/adrenal/tris.md2", EF_ROTATE,
		"models/items/etank/tris.md2", EF_ROTATE,
//peewee end
		NULL,
///* icon */		"p_adrenaline",
/* icon */		"i_etank",
///* pickup */	"Adrenaline",
/* pickup */	"E-Tank",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
//		Pickup_Bandolier,
//peewee start
		Pickup_Powerup,
//peewee end
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		//"models/items/band/tris.md2", EF_ROTATE,
		"models/items/wallk/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"p_bandolier",
		"i_wallk",
///* pickup */	"Bandolier",
/* pickup */	"Wall Kicks",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
//		Pickup_Pack,
//peewee start
		Pickup_Powerup,
//peewee end
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
//		"models/items/pack/tris.md2", EF_ROTATE,
		"models/items/airslide/tris.md2", EF_ROTATE,
		NULL,
///* icon */		"i_pack",
/* icon */		"i_airslide",

///* pickup */	"Ammo Pack",
/* pickup */	"Air Slide",
/* width */		2,
		180,
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
///* icon */		"i_health",
/* icon */		"i_energy",
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

	//peewee start
	//self->model = "models/items/healing/medium/tris.md2";
	self->model = "models/items/energysm/tris.md2";
	//peewee end
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

	//self->model = "models/items/healing/stimpack/tris.md2";
	self->model = "models/items/heart/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;

	self->s.effects |= EF_ROTATE;

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

	//peewee start
	//self->model = "models/items/healing/large/tris.md2";
	self->model = "models/items/energy/tris.md2";
	//peewee end
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

	//self->model = "models/items/mega_h/tris.md2";
	self->model = "models/items/extralife/tris.md2";

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
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}
