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
// IWM
void Weapon_Devastator (edict_t *ent);
void Weapon_RocketPods (edict_t *ent);
void Weapon_BallLauncher (edict_t *ent);
void Weapon_ShardCannon (edict_t *ent);
void Weapon_RotLasLauncher (edict_t *ent);
void Weapon_ChainGren (edict_t *ent);
void Weapon_Needler (edict_t *ent);
void Weapon_CO2 (edict_t *ent);
void Weapon_FireFly (edict_t *ent);
void Weapon_Fooby (edict_t *ent);
void Weapon_GravRocketLauncher (edict_t *ent);
void Weapon_BalloonLauncher (edict_t *ent);
void Weapon_ScrewDriver (edict_t *ent);
void Weapon_GravityGrenade (edict_t *ent);
void Weapon_InfestLauncher (edict_t *ent);
void Weapon_FlashGrenade (edict_t *ent);
void Weapon_ConcussionGrenade (edict_t *ent);
void Weapon_AccelleRocketLauncher (edict_t *ent);
void Weapon_GibMineLauncher (edict_t *ent);
void Weapon_Bee (edict_t *ent);
void Weapon_Flare (edict_t *ent);
void Weapon_GibMine (edict_t *ent);
void Weapon_InfestGrenade (edict_t *ent);
void Weapon_RotLas (edict_t *ent);
void Weapon_ForceGrenade (edict_t *ent);
void Weapon_JackGrenade (edict_t *ent);
void Weapon_AcidCloud (edict_t *ent);
void Weapon_Mace (edict_t *ent);
void Weapon_Drill (edict_t *ent);
void Weapon_LightningCloud (edict_t *ent);
void Weapon_PacMan (edict_t *ent);
void Weapon_Plasma (edict_t *ent);
void Weapon_Thunderbolt (edict_t *ent);
void Weapon_DiskLauncher (edict_t *ent);
void Weapon_NapalmGrenadeLauncher (edict_t *ent);
void Weapon_Bean (edict_t *ent);
void Weapon_Hose (edict_t *ent);
void Weapon_Volcano (edict_t *ent);
void Weapon_GasGun (edict_t *ent);
void Weapon_SpikeBomb (edict_t *ent);
void Weapon_Soundwave (edict_t *ent);
void Weapon_DKHammer_Temp (edict_t *ent);
void Weapon_EQGrenade (edict_t *ent);
void Weapon_Blade (edict_t *ent);
void Weapon_Det (edict_t *ent);
void Weapon_NeutronStar (edict_t *ent);
void Weapon_ChainFist (edict_t *ent);
void Weapon_ETF_Rifle (edict_t *ent);
void Weapon_ProxLauncher (edict_t *ent);
void Weapon_Disintegrator (edict_t *ent);
void Weapon_Heatbeam (edict_t *ent);
void Weapon_Trap (edict_t *ent);
void Weapon_Phalanx (edict_t *ent);
void Weapon_Ionripper (edict_t *ent);
void Weapon_Tesla (edict_t *ent);
void Weapon_Meteor (edict_t *ent);
void Weapon_HeatLaser (edict_t *ent);
void Weapon_LinkGrenade (edict_t *ent);
void Weapon_PIG (edict_t *ent);
void Weapon_Stapler (edict_t *ent);
void Weapon_BeamCannon (edict_t *ent);
void Weapon_TeleGrenade (edict_t *ent);
void Weapon_AGM(edict_t *self);
void Weapon_ChainLightning (edict_t *ent);
void Weapon_Magnet (edict_t *ent);
void Weapon_BlockCannon (edict_t *ent);
void Weapon_Drunk (edict_t *ent);
void Weapon_Portal (edict_t *ent);
void Weapon_LaserTrap (edict_t *ent);
void Weapon_PingGun (edict_t *ent);

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};


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

//ZOID
//in ctf, when we are weapons stay, only the master of a team of weapons
//is spawned
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
			ent = master;
		else {
//ZOID

			for (count = 0, ent = master; ent; ent = ent->chain, count++)
				;

			choice = rand() % count;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
				;
		}
	}

	ent->item = RandomizeItem(ent->item);
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
			if (ent->item->use)
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
void Use_Regen (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	if (ent->client->reg_framenum > level.framenum)
		ent->client->reg_framenum += 300;
	else
		ent->client->reg_framenum = level.framenum + 300;
}

qboolean Pickup_Regen (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity > 0)
		return false;

	if (ent->tempent)
		G_FreeEdict (ent->tempent);

	//other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}

void Use_DKHammer (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	ent->client->dkhammer_framenum = level.framenum + 85;

	ent->client->temp_oldweapon = ent->client->pers.weapon;

	ent->client->old_gunframe = ent->client->ps.gunframe;
	ent->client->old_weaponstate = ent->client->weaponstate;
	ent->client->weaponstate = WEAPON_FIRING;
	ent->client->ps.gunframe = 3;
	ent->client->pers.weapon = FindItem("Donkey Kong Hammer Temp Weapon");
}

qboolean Pickup_DKHammer (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (other->client->star_framenum > level.framenum)
		return false;

	if (other->client->dkhammer_framenum > level.framenum)
		return false;

	//other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}

void Use_Star (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	ent->client->star_framenum = level.framenum + 200;
}

qboolean Pickup_Star (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (other->client->star_framenum > level.framenum)
		return false;

	//other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}
//======================================================================

void Use_Boots (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	if (ent->client->boots_framenum > level.framenum)
		ent->client->boots_framenum += 300;
	else
		ent->client->boots_framenum = level.framenum + 300;
}

qboolean Pickup_Boots (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

//	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}

//======================================================================
void Use_DefShield (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	if (ent->client->deflector_framenum > level.framenum)
		ent->client->deflector_framenum += 300;
	else
		ent->client->deflector_framenum = level.framenum + 300;
}

qboolean Pickup_DefShield (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

//	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}
//======================================================================
void Use_Cloak (edict_t *ent, gitem_t *item)
{
//	int		timeout;

	ValidateSelectedItem (ent);

	if (ent->client->cloak_framenum > level.framenum)
		ent->client->cloak_framenum += 300;
	else
		ent->client->cloak_framenum = level.framenum + 300;

	ent->client->cloak_turninvis_framenum = level.framenum + 20;
}

qboolean Pickup_Cloak (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

//	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	ent->item->use (other, ent->item);

	return true;
}


/*ATTILA begin*/
void Use_Jet ( edict_t *ent, gitem_t *item )
{
    ValidateSelectedItem ( ent );
	
    /*jetpack in inventory but no fuel time? must be one of the
	give all/give jetpack cheats, so put fuel in*/
    if ( ent->client->Jet_remaining == 0 )
		ent->client->Jet_remaining = 300;
	
    if ( Jet_Active(ent) ) 
		ent->client->Jet_framenum = 0; 
    else
		ent->client->Jet_framenum = level.framenum + ent->client->Jet_remaining;
	
    /*The On/Off Sound taken from the invulnerability*/
    gi.sound( ent, CHAN_ITEM, SoundIndex("items/protect.wav"), 0.8, ATTN_NORM, 0 );

	ent->client->pers.inventory[ITEM_INDEX(item)] --;
	ValidateSelectedItem ( ent );

    /*this is the sound played when flying. To here this sound 
	immediately we play it here the first time*/
    //gi.sound ( ent, CHAN_AUTO, SoundIndex("hover/hovidle1.wav"), 0.8, ATTN_NORM, 0 );
}
/*ATTILA end*/
qboolean Pickup_Jet (edict_t *ent, edict_t *other)
{	
	other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
    other->client->Jet_remaining = 300;
	
    /*if deathmatch-flag instant use is set, switch off the jetpack, 
	the item->use function will turn it on again immediately*/
    if ( (int)dmflags->value & DF_INSTANT_ITEMS )
		other->client->Jet_framenum = 0;
	
    /*otherwise update the burn out time if jetpack is activated*/
    else 
		if ( Jet_Active(other) )
			other->client->Jet_framenum = level.framenum + other->client->Jet_remaining;

		Use_Jet (other, ent->item);
		
		return true;
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
//==================================================================
void Use_IR (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->ir_framenum > level.framenum)
		ent->client->ir_framenum += 300;
	else
		ent->client->ir_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, SoundIndex("misc/ir_start.wav"), 1, ATTN_NORM, 0);
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

	gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
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

//	gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
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

//	gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
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

	gi.sound(ent, CHAN_ITEM, SoundIndex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
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
qboolean PickupLinkedWeapons(edict_t *item, edict_t *picker);

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
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") || other->client->pers.weapon == FindItem("screwdriver launcher") || other->client->pers.weapon == FindItem("saw-toothed hammer") || other->client->pers.weapon == FindItem("Battery-Operated Drill") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);

	if (!PickupLinkedWeapons(ent, other))
	{
		gi.dprintf ("PickupLinkedWeapons: Weapon has no links, or returned false somehow!\n");
	}

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
	if (self->owner->health > self->owner->max_health
//ZOID
		&& !CTFHasRegeneration(self->owner)
//ZOID
		)
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
	int cured = 0;

	if (ent->count >= 5) {
		edict_t *yuck;
		for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) {
			if (!yuck->inuse)
				continue;
			if (yuck->infester && yuck->target_ent == other) {
				G_FreeEdict (yuck);
				cured++;
			}
		}
		if (cured) {
			safe_cprintf(other,PRINT_HIGH,"Your infest was cured!\n");
			other->infest_debounce_time = level.time + 3;
		}
	}

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
		&& !CTFHasRegeneration(other)
//ZOID
	)
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
		gi.sound(ent, CHAN_AUTO, SoundIndex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			safe_cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, SoundIndex("misc/power1.wav"), 1, ATTN_NORM, 0);
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
	if (!ent->item || (!ent->item && ent->item->pickup))
		return;		// not a grabbable item?
// CYBERSLASH: don't pick up anything if pigged
	if (other->client->pigged)
		return;


	if (iwm_editor->value)
		taken = false;
	else
		taken = ent->item->pickup(ent, other);

	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = ImageIndex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->count == 2)
				gi.sound(other, CHAN_ITEM, SoundIndex("items/s_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 10)
				gi.sound(other, CHAN_ITEM, SoundIndex("items/n_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 25)
				gi.sound(other, CHAN_ITEM, SoundIndex("items/l_health.wav"), 1, ATTN_NORM, 0);
			else // (ent->count == 100)
				gi.sound(other, CHAN_ITEM, SoundIndex("items/m_health.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->item->pickup_sound)
		{
			gi.sound(other, CHAN_ITEM, SoundIndex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
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

void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
	if (ent->tempfloat == 3)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}


qboolean findspawnpoint (edict_t *ent);

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
	dropped->s.renderfx |= RF_IR_VISIBLE;
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
	else if (Q_stricmp(item->classname, "spawnit") == 0)
	{
		findspawnpoint(ent);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	if (Q_stricmp(item->classname, "spawnit") == 0)
	{
		dropped->velocity[2] = rand()%360;
		dropped->velocity[0] = rand()%360;
		dropped->velocity[1] = rand()%360;
	}
	else
		dropped->velocity[2] = rand()%360;
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
	if (iwm_editor->value)
	{
		ent->solid = SOLID_BBOX;
		ent->svflags = SVF_DEADMONSTER;
	}
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	if (ent->spawnflags & SPAWNFLAG_NO_DROPTOFLOOR)
	{
	ent->movetype = MOVETYPE_NONE;  
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
		return;
	}

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
		SoundIndex (it->pickup_sound);
	if (it->world_model)
		ModelIndex (it->world_model);
	if (it->view_model)
		ModelIndex (it->view_model);
	if (it->icon)
		ImageIndex (it->icon);

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
			ModelIndex (data);
		else if (!strcmp(data+len-3, "sp2"))
			ModelIndex (data);
		else if (!strcmp(data+len-3, "wav"))
			SoundIndex (data);
		if (!strcmp(data+len-3, "pcx"))
			ImageIndex (data);
	}
}

gitem_t *RandomizeItem (gitem_t *item)
{
	gitem_t *ra;

	if (!((int)dmflags->value & DF_RANDOM_ITEMS))
		return item;

retry:
	ra = &itemlist[rand()%game.num_items];

	if (!ra->flags & IT_WEAPON && !ra->flags & IT_AMMO && !ra->flags & IT_POWERUP && !ra->flags & IT_ARMOR)
		goto retry;
	if (ra->flags & IT_KEY)
		goto retry;
	if (!ra->world_model)
		goto retry;

	return ra;
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void HoldSecondPart (edict_t *owner);
void SpawnItem (edict_t *ent, gitem_t *item)
{
	PrecacheItem (item);

	/*if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = DROPPED_ITEM;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}*/

	// some items will be prevented in deathmatch
	if (deathmatch->value && !ent->spawned_item)
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
			if ( (item->flags == IT_AMMO) )
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


//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)) {
		G_FreeEdict(ent);
		return;
	}
//ZOID

	// Paril, hack.
	if (strcmp(ent->classname, "weapon_chainfist") == 0)
	{
		G_FreeEdict (ent);
		return;
	}

	if (!item->world_model)
	{
		G_FreeEdict (ent);
		return;
	}

	item = RandomizeItem (item);
	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		ModelIndex (ent->model);
	if (ent->spawnflags & SPAWNFLAG_NO_SPINNING)
		ent->s.effects &= ~EF_ROTATE;

	if (Q_stricmp(ent->classname, "regeneration") == 0 || Q_stricmp(ent->classname, "item_cloaking") == 0)
	{
		ent->think2 = HoldSecondPart;
		ent->nextthink2 = level.time + .1;
	}

	//gi.dprintf ("%i\n", ent->spawnflags);

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0) {
		ent->think = CTFFlagSetup;
	}
//ZOID

	ent->s.renderfx |= RF_IR_VISIBLE;
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
/* precache */ "",
		NULL
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
/* precache */ "",
		NULL
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
/* precache */ "",
		NULL
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
/* precache */ "",
		NULL
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
/* precache */ "",
		NULL
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav",
		NULL
	},


	//
	// WEAPONS 
	//

/* Donkey Kong Hammer. Weapon is placeholder, never in inventory.*/
	{
		"dk", 
		NULL,
		NULL,
		NULL,
		Weapon_DKHammer_Temp,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/items/dkh/tris.md2",
/* icon */		"i_dkham",
/* pickup */	"Donkey Kong Hammer Temp Weapon",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "",
		NULL
	},

/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_grapple", 
		NULL,
		Use_Weapon,
		NULL,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
/* icon */		"w_grapple",
/* pickup */	"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,
/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},

/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_cloaking", 
		Pickup_Cloak,
		Use_Cloak,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/invis/inner.md2", EF_ROTATE|EF_SPHERETRANS,
		NULL,
/* icon */		"i_invis",
/* pickup */	"Transparency Device",
/* width */		0,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_TRANSPARENCY_DEVICE
	},

/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_defsh", 
		Pickup_DefShield,
		Use_DefShield,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Deflector Shield",
/* width */		0,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_DEFLECTOR_SHIELD
	},

/*QUAKED weapon_chainfist (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_chainfist",									// classname
		NULL,										// pickup function
		Use_Weapon,											// use function
		NULL,										// drop function
		Weapon_ChainFist,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		NULL, 0,		// world model, world model flags
		"models/weapons/v_chainf/tris.md2",					// view model
		"w_chainfist",										// icon
		"Chainfist",										// name printed when picked up 
		0,													// number of digits for statusbar
		0,													// amount used / contained
		NULL,												// ammo type used 
		IT_WEAPON,								// inventory flags
		WEAP_BLASTER,										// visible weapon
		NULL,												// info (void *)
		1,													// tag
		"weapons/sawidle.wav weapons/sawhit.wav",			// precaches
	},

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
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
		NULL
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_ScrewDriver,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"ScrewDriver Launcher",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "",
		NULL
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Mace,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_thor/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Saw-toothed Hammer",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
		NULL
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Drill,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_drill/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Battery-Operated Drill",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "",
		NULL
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Det,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_det/tris_new.md2",
/* icon */		"w_blaster",
/* pickup */	"Radio Detonator",
		0,
		5,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "",
		NULL
	},
	
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
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Neutron Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
		NULL
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Portal,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_puddle/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Porta-Puddle",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
		NULL
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav",
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ShardCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shard Cannon",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "",
		"Shotgun",
		SHOTGUNBAN_SHARDCANNON
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_CO2,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"CO2 Cannon",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav",
		"Shotgun",
		SHOTGUNBAN_CO2CANNON
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Magnet,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Magnets",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav",
		"Shotgun",
		SHOTGUNBAN_MAGNET
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav",
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_DiskLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Disk Launcher",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "",
		"Super Shotgun",
		SSGBAN_DISKLAUNCHER
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GasGun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Toxic Gas Gun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "",
		"Super Shotgun",
		SSGBAN_TOXICGASGUN
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Blade,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"BladeRunner",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "",
		"Super Shotgun",
		SSGBAN_BLADERUNNER
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	/*{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Meteor,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
				"w_sshotgun",
				"Meteor Cannon",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
		"weapons/sshotf1b.wav",
		"Super Shotgun"
	},*/

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Needler,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_needler/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Needler",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "",
		"Machinegun",
		MACHINEGUNBAN_NEEDLER
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Ricochet Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "",
		"Machinegun",
		MACHINEGUNBAN_RICOCHET
	},

/*QUAKED weapon_etf_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_etf_rifle",									// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_ETF_Rifle,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_machn/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_etf_rifle/tris.md2",					// view model
		"w_etf_rifle",										// icon
		"ETF Rifle",										// name printed when picked up 
		0,													// number of digits for statusbar
		1,													// amount used / contained
		"Bullets",										// ammo type used 
		IT_WEAPON,											// inventory flags
		WEAP_MACHINEGUN,										// visible weapon
		NULL,												// info (void *)
		0,													// tag
		"weapons/nail1.wav models/proj/flechette/tris.md2",	// precaches
		"Machinegun",
		MACHINEGUNBAN_ETFRIFLE
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Stapler,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_stapler/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Stapler",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
		"Machinegun",
		MACHINEGUNBAN_STAPLER
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav weapons/chngnd1a.wav",
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ChainGren,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chain-Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "",
		"Chaingun",
		CHAINGUNBAN_CHAINGRENADE
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FireFly,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Homing Firefly Launcher",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "",
		"Chaingun",
		CHAINGUNBAN_FIREFLY
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Drunk,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"The Confuzzler",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "",
		"Chaingun",
		CHAINGUNBAN_CONFUZZLER
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
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
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav ",
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_GravityGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Gravity Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_GRAVITY
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_FlashGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Flash Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_FLASH
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_ConcussionGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Concussion Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_CONCUSSION
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Flare,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Glowsticks",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_GLOWSTICKS
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_GibMine,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Gib Mines",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_GIBMINES
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_InfestGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Infest Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_INFEST
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_RotLas,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Rotating Laser Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_ROTATINGLASER
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_ForceGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Area Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_AREA
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_JackGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Exploding Jack Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		"Grenades",
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_EXPLODINGJACK
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_SpikeBomb,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Spikebombs",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_SPIKEGRENADE
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_EQGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Earthquake Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_EARTHQUAKE
	},

/*QUAKED ammo_trap (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_trap",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", EF_ROTATE,
		"models/weapons/v_trap/tris.md2",
/* icon */		"a_trap",
/* pickup */	"Trap",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "weapons/trapcock.wav weapons/traploop.wav weapons/trapsuck.wav weapons/trapdown.wav",
// "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
		"Grenades",
		GRENADEBAN_TRAP
	},

/*QUAKED ammo_tesla (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_tesla",
		Pickup_Weapon,
		Use_Weapon,						// PGM
		Drop_Ammo,
		Weapon_Tesla,					// PGM
		"misc/am_pkup.wav",
//		"models/weapons/g_tesla/tris.md2", 0,
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_tesla/tris.md2",
		"a_tesla",
		"Tesla",
		3,
		1,
		"grenades",												// PGM
		IT_WEAPON,						// inventory flags
		WEAP_GRENADES,
		NULL,										// info (void *)
		0,									// tag
		"models/weapons/v_tesla2/tris.md2 weapons/teslaopen.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav models/weapons/g_tesla/tris.md2",			// precache
		"Grenades",
		GRENADEBAN_TESLA
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LinkGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Laser Link Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_LASERLINK
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_TeleGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Teleport Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_TELEPORT
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_ChainLightning,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Chain Lightning Grenades",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_CHAINLIGHTNING
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LaserTrap,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Laser Traps",
/* width */		3,
		1,
		"grenades",
		IT_WEAPON,
		WEAP_GRENADES,
		"Grenades",
		0,
/* precache */ "",
		"Grenades",
		GRENADEBAN_LASERTRAP
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BallLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Rubber Ball Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_RUBBERBALLS
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)

	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RotLasLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
		"w_glauncher",
	"Rotating Laser Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
 ""
	},*/
/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BalloonLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Balloon Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_BALLOONLAUNCHER
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)

	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_InfestLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
		"w_glauncher",
	"Infest Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
 "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},*/

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BallLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Basketball Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_BASKETBALL
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BallLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Beach Ball Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_BEACHBALL
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GibMineLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
		"w_glauncher",
	"Gib Mine Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
"models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},*/

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_NapalmGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Napalm Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_NAPALMLAUNCHER
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Bean,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Napalm Jumping Bean Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Grenade Launcher",
		GLBAN_NAPALMJUMPINGBEANS
	},

	// ROGUE
/*QUAKED weapon_proxlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_proxlauncher",								// classname
		Pickup_Weapon,										// pickup
		Use_Weapon,											// use
		Drop_Weapon,										// drop
		Weapon_ProxLauncher,								// weapon think
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_launch/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_plaunch/tris.md2",				// view model
		"w_proxlaunch",										// icon
		"Prox Launcher",									// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used
		"Grenades",												// ammo type used
		IT_WEAPON,											// inventory flags
		WEAP_GRENADELAUNCHER,									// visible weapon
		NULL,												// info (void *)
		0,											// tag
		"weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav weapons/proxwarn.wav weapons/proxopen.wav",
		"Grenade Launcher",
		GLBAN_PROXLAUNCHER
	},
	// rogue

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Fooby,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Fooby the Kamikaze Watermelon",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_FOOBY
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GravRocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Gravity Missile Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_GRAVITYMISSILE
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_AccelleRocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"AccelleRocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_ACCELLEROCKET
	},

/*QUAKED weapon_dev (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_dev",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Devastator,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_dev/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Devastator",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_DEVASTATOR
	},

/*QUAKED weapon_rpod (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rpod",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketPods,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rpod/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Pods",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_ROCKETPODS
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Freeze Nuke Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_FREEZENUKE
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Weapon of Mass Destruction",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_WOMD
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PacMan,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"PacMan Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "",
		"Rocket Launcher",
		RLBAN_PACMAN
	},


/*QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_disintegrator",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Disintegrator,								// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_dist/tris.md2",					// view model
		"w_disintegrator",									// icon
		"Disruptor",										// name printed when picked up 
		0,													// number of digits for statusbar
		1,													// amount used / contained
		"Rockets",											// ammo type used 
		IT_WEAPON,											// inventory flags
		WEAP_ROCKETLAUNCHER,										// visible weapon
		NULL,												// info (void *)
		1,													// tag
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",	// precaches
		"Rocket Launcher",
		RLBAN_DISRUPTOR
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BlockCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Block Cannon",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
		"Rocket Launcher",
		RLBAN_BLOCKCANNON
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav",
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Bee,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"Kamikaze Bee Launcher",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "",
		"Hyperblaster",
		HYPERBLASTERBAN_BEES
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Plasma,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_plasma/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"Plasma Rifle",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "",
		"Hyperblaster",
		HYPERBLASTERBAN_PLASMA
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
/*
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Hose,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", 0,
		"models/weapons/v_hose/tris.md2",
 icon 		"w_hyperblaster",
 pickup 	"Fire Hose",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
 precache  "",
		"Hyperblaster",
		HYPERBLASTERBAN_FIREHOSE
	},*/

	// ROGUE
/*QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/ 
	{
		"weapon_plasmabeam",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Heatbeam,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_beamer/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_beamer/tris.md2",					// view model
		"w_heatbeam",											// icon
		"Plasma Beam",											// name printed when picked up 
		0,													// number of digits for statusbar
		// FIXME - if this changes, change it in NoAmmoWeaponChange as well
		2,													// amount used / contained
		"Cells",											// ammo type used 
		IT_WEAPON,											// inventory flags
		WEAP_HYPERBLASTER,										// visible weapon
		NULL,												// info (void *)
		0,													// tag
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav",		// precaches
		"Hyperblaster",
		HYPERBLASTERBAN_PLASMABEAM
	},
	//rogue

/*QUAKED weapon_boomer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_boomer",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
		"models/weapons/g_boom/tris.md2", EF_ROTATE,
		"models/weapons/v_boomer/tris.md2",
/* icon */	"w_ripper",
/* pickup */ "Ionripper",
		0,
		2,
		"Cells",
		IT_WEAPON,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav weapons/rippfire.wav",
		"Hyperblaster",
		HYPERBLASTERBAN_IONRIPPER
	},
// END 14-APR-98

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HeatLaser,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"Heat Laser",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav",
		"Hyperblaster",
		HYPERBLASTERBAN_HEATLASER
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PingGun,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_ping/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"Ping Gun",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "",
		"Hyperblaster",
		HYPERBLASTERBAN_PINGGUN
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav",
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Ricochet Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "",
		"Railgun",
		RAILGUNBAN_RICOCHET
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Triple Slug Railgun",
		0,
		3,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "",
		"Railgun",
		RAILGUNBAN_TRIPLESLUG
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Thunderbolt,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_lightn/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Thunderbolt",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "",
		"Railgun",
		RAILGUNBAN_THUNDERBOLT
	},

// RAFAEL 14-APR-98
/*QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
/* icon */	"w_phallanx",
/* pickup */ "Phalanx",
		0,
		1,
		"Slugs",
		IT_WEAPON,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/plasshot.wav",
		"Railgun",
		RAILGUNBAN_PHALANX
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BeamCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Beam Cannon",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav",
		"Railgun",
		RAILGUNBAN_BEAMCANNON
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav",
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_AcidCloud,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Acid Cloud",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "",
		"BFG10K",
		BFGBAN_ACIDCLOUD
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_LightningCloud,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Lightning Cloud",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "",
		"BFG10K",
		BFGBAN_LIGHTNINGCLOUD
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Volcano,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Volcano Launcher",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav",
		"BFG10K",
		BFGBAN_VOLCANO
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Soundwave,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Soundwave Emitter",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav",
		"BFG10K",
		BFGBAN_SOUNDWAVE
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_NeutronStar,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Neutron Star",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav",
		"BFG10K",
		BFGBAN_NEUTRON
	},

// CYBERSLASH
	{
		"weapon_pig", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PIG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
        "models/weapons/v_pig2k/tris.md2",
		"w_pig",
		"PIG2K",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
		"weapons/rg_hum.wav models/weapons/v_pig2k/tris.md2",
		"BFG10k",
		BFGBAN_PIG2K
	},
// CYBERSLASH

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_AGM,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Gravity Manipulator",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav",
		"BFG10k",
		BFGBAN_GRAVITY
	},

	// New items for IWM
/*QUAKED item_ir_goggles (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
gives +1 to maximum health
*/
	{
		"item_ir_goggles",
		Pickup_Powerup,
		Use_IR,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/goggles/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_ir",
/* pickup */	"IR Goggles",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "misc/ir_start.wav",
		NULL,
		ITEMBAN_IR_GOGGLES
	},

/*QUAKED jumpboots (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"jumpboots",
		Pickup_Boots,
		Use_Boots,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/boots/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_boots",
/* pickup */	"Jump Boots",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_JUMP_BOOTS
	},

/*QUAKED regen (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
gives +1 to maximum health
*/
	{
		"regeneration",
		Pickup_Regen,
		Use_Regen,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/regen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_regen",
/* pickup */	"Regeneration",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_REGENERATION
	},

/*QUAKED donkey kong hammer (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"dkhammer",
		Pickup_DKHammer,
		Use_DKHammer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/dkh/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_dkham",
/* pickup */	"Donkey Kong Hammer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_DONKEY_KONG_HAMMER
	},

/*QUAKED star (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_star",
		Pickup_Star,
		Use_Star,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/star/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Starman",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
		NULL,
		ITEMBAN_STARMAN
	},

	/*ATTILA begin*/
	/*QUAKED item_jet (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
		{
			"item_carpet",    
				Pickup_Jet,
				Use_Jet,        /*ATTILA the Use_Jet function from above*/
				NULL,           /*ATTILA No dropping function for jetpack*/
				NULL,
				"items/pkup.wav",
				
				/*ATTILA this will show the monster icarus instead of quad damage in 
				the game*/
				"models/items/carpet/tris.md2", EF_ROTATE, 
				
				NULL,
				/* icon */   "i_carpet",  /*ATTILA Ok, its the quad icon on screen but who cares*/
				/* pickup */ "Magic Carpet", /*ATTILA now we can use it with the use command*/
				/* width */  2,
				60,
				NULL,
				IT_POWERUP,
				0,
				NULL,
				0,
				/* precache */ "",
				NULL,
				ITEMBAN_MAGIC_CARPET
		},
		/*Attila end*/


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
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"Shells",
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
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
		50,
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
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"Cells",
/* width */		3,
		50,
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
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_rockets",
/* pickup */	"Rockets",
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
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */		"a_slugs",
/* pickup */	"Slugs",
/* width */		3,
		10,
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
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
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
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
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
/* icon */		"p_silencer",
/* pickup */	"Silencer",
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
		IT_HEALTH,	//JABot
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
		IT_HEALTH,	//JABot
		0,
		NULL,
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
		0,
		NULL,
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
		IT_HEALTH,	//JABot
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
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
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
/* icon */		"i_ctf1",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
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
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
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
#ifdef _WIN32
extern __declspec(dllexport) void SP_item_health (edict_t *self);
#endif
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
	SoundIndex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
#ifdef _WIN32
extern __declspec(dllexport) void SP_item_health_small (edict_t *self);
#endif
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
	SoundIndex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
#ifdef _WIN32
extern __declspec(dllexport) void SP_item_health_large (edict_t *self);
#endif
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
	SoundIndex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
#ifdef _WIN32
extern __declspec(dllexport) void SP_item_health_mega (edict_t *self);
#endif
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
	SoundIndex ("items/m_health.wav");
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
