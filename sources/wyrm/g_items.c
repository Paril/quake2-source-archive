#include "g_local.h"
#include "s_nightmare.h"


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

//Wyrm: New weapons
void Weapon_Disintegrator (edict_t *ent);
void Weapon_PulseRifle (edict_t *ent);
void Weapon_NapalmRockets (edict_t *ent);
void Weapon_ExplosiveMachinegun (edict_t *ent);
void Weapon_Flaregun (edict_t *ent);
void Weapon_Pistol (edict_t *ent);
void Weapon_DGSuperShotgun (edict_t *ent);
void Weapon_FlakCannon (edict_t *ent);
void Weapon_ClusterGrenadeLauncher (edict_t *ent);
void Weapon_Airfist (edict_t *ent);
void Weapon_BloodDrainer (edict_t *ent);
void Weapon_Flamethrower (edict_t *ent);
void Weapon_StreetSweeper (edict_t *ent);
void Weapon_GuidedMisiles (edict_t *ent);
void Weapon_Freezegun (edict_t *ent);
void Weapon_AntimatterCannon (edict_t *ent);
void Weapon_PositronBeam (edict_t *ent);
void Weapon_PlasmaGun (edict_t *ent);
void Weapon_BFGrenadeLauncher (edict_t *ent);
void Weapon_NapalmGrenadeLauncher (edict_t *ent);
void Weapon_RailGrenadeLauncher (edict_t *ent);
void Weapon_StickingGrenadeLauncher (edict_t *ent);
void Weapon_ProxGrenadeLauncher (edict_t *ent);
void Weapon_Tripbomb (edict_t *ent);
void Weapon_LaserTripbomb (edict_t *ent);
void Weapon_VacuumMaker (edict_t *ent);
void Weapon_EnergyVortex (edict_t *ent);
void Weapon_LightningGun (edict_t *ent);
void Weapon_SuperNailgun (edict_t *ent);
void Weapon_Nailgun (edict_t *ent);
void Weapon_Bucky (edict_t *ent);
void Weapon_Nuke (edict_t *ent);
void Weapon_Homing (edict_t *ent);
void Weapon_Perforator (edict_t *ent);

int      jacket_armor_index;
int      combat_armor_index;
int      body_armor_index;
static int	power_screen_index;
static int	power_shield_index;
//Wyrm: tech
int tech_resistance_index;
int tech_strength_index;
int tech_haste_index;
int tech_regeneration_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

//Wyrm: jetpack
void Use_Jet ( edict_t *ent, gitem_t *item );
void Use_Steroids (edict_t *ent, gitem_t *item);

qboolean Jet_Active( edict_t *ent );
void Use_Sentry (edict_t *ent, gitem_t *inv);


//RWS
extern d_RndWeapon             wrndtbl[MAX_ITEMS];

//Wyrm: medipak
qboolean        Pickup_MediPak (edict_t *ent, edict_t *other);
void            Use_MediPak (edict_t *ent, gitem_t *inv);
void            Drop_MediPak (edict_t *ent, gitem_t *item);
//Wyrm: jetpak
qboolean        Pickup_Jetpak (edict_t *ent, edict_t *other);
void            Use_Jetpak (edict_t *ent, gitem_t *inv);
void            Drop_Jetpak (edict_t *ent, gitem_t *item);
//Wyrm: cloaker
void            Use_Cloak (edict_t *ent, gitem_t *inv);
//Wyrm: scanner
qboolean        Pickup_Scanner (edict_t *ent, edict_t *other);
void            Use_Scanner (edict_t *ent, gitem_t *inv);
void            Drop_Scanner (edict_t *ent, gitem_t *item);
//Wyrm: Can of Pork'N'Beans
void Use_Beans (edict_t *ent, gitem_t *item);

//Wyrm: sentry
qboolean        Pickup_Sentry (edict_t *ent, edict_t *other);

gitem_armor_t jacketarmor_info  = { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info  = { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info    = {100, 200, .80, .60, ARMOR_BODY};

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
	else
	{
                //RWS
		int			rt;
		gitem_t		* p = FindRandomWeapon(ent,&rt);

		// any valid item ?
		if (p)
                {
			gi.setmodel (ent, (ent->item=p)->world_model);
                        ent->s.effects = ent->item->world_model_flags;
                }
	}

        //Wyrm: respawning ents: old origin
        VectorCopy(ent->monsterinfo.startorigin, ent->s.origin);

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
/*-----/ PM /-----/ NEW:  Kill fire immediately! /-----*/
	other->burnout = 0;
/*-----------------------------------------------------*/

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
/*-----/ PM /-----/ NEW:  Kill fire immediately! /-----*/
	other->burnout = 0;
/*-----------------------------------------------------*/
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
        else //medipak
                max = 200;

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
        //Wyrm: if called from the RWS function...
        if (ent->count == 100)
                ent->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;

	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
/*-----/ PM /-----/ MODIFIED:  Pick up health if burning. /-----*/
			if (other->burnout < level.time)
				return false;
/*--------------------------------------------------------------*/


/*-----/ PM /-----/ NEW:  Kill fire percentage. /-----*/
        if (other->burnout)
        {       if ((rand() % 100) < ent->count)
                        other->burnout = 0;
                else
                        other->burnout -= ent->count;
        }
/*----------------------------------------------------*/
/*-----/ PM /-----/ MODIFIED:  Add any health, but never subtract. /-----*/
//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

        if (ent->style & HEALTH_IGNORE_MAX)
                other->health += ent->count;
        else
        {       if (other->health < other->max_health)
                {       other->health += ent->count;
                        if (other->health > other->max_health)
                                other->health = other->max_health;
                }
        }
/*-----------------------------------------------------------------------*/
//ZOID
        if (other->health > (other->max_health*2.5) && ent->count > 25)
                other->health = other->max_health * 2.5;
//ZOID

	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

/*-----/ PM /-----/ REMOVED:  Health capping done above. /-----*/

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

extern d_Class             classtbl[];

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;
        //Wyrm: class
        int class;

        if ((enableclass->value)&&(ent->client))
                class = ent->client->resp.class;
        else
                class = 0;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;
        if (ent->item->info == &bodyarmor_info)
                newinfo = &classtbl[class].bodyarmor_info;
        else if (ent->item->info == &combatarmor_info)
                newinfo = &classtbl[class].combatarmor_info;
        else if (ent->item->info == &jacketarmor_info)
                newinfo = &classtbl[class].jacketarmor_info;
        else
                newinfo = NULL;

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
                        oldinfo = &classtbl[class].jacketarmor_info;
                else if (old_armor_index == combat_armor_index)
                        oldinfo = &classtbl[class].combatarmor_info;
		else // (old_armor_index == body_armor_index)
                        oldinfo = &classtbl[class].bodyarmor_info;

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

//Wyrm: modified
        if (ent->flags & FL_POWER_ARMOR)
                return POWER_ARMOR_SHIELD;

        if (ent->flags & FL_POWER_SCREEN)
                return POWER_ARMOR_SCREEN;
/*
	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;
*/
	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

        if ((ent->flags & FL_POWER_ARMOR)||(ent->flags & FL_POWER_SCREEN))
	{
		ent->flags &= ~FL_POWER_ARMOR;
                ent->flags &= ~FL_POWER_SCREEN;
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

//Wyrm: power screen functions
void Use_PowerScreen (edict_t *ent, gitem_t *item)
{
	int		index;

        if ((ent->flags & FL_POWER_ARMOR)||(ent->flags & FL_POWER_SCREEN))
	{
		ent->flags &= ~FL_POWER_ARMOR;
                ent->flags &= ~FL_POWER_SCREEN;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
                        gi.cprintf (ent, PRINT_HIGH, "No cells for power screen.\n");
			return;
		}
                ent->flags |= FL_POWER_SCREEN;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerScreen (edict_t *ent, edict_t *other)
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

void Drop_PowerScreen (edict_t *ent, gitem_t *item)
{
        if ((ent->flags & FL_POWER_SCREEN) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
                Use_PowerScreen (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
int FindInTable(char * name);

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;
        int             i;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?


        if
        (ent->spawnflags & (DROPPED_ITEM|DROPPED_PLAYER_ITEM))
                i = -1;
        else
                i = ITEM_INDEX(ent->item);
        //RWS pickup stuff
        if (i >= 0)
        {
                if (wrndtbl[i].mode == ITEM_UPGRADE)
                {
                        int c=0;
                        gitem_t *minorweapon=NULL;

                        int ammount=0;
                        int newammount;

                        while ( c < wrndtbl[i].num)
                        {
                                if (
                                ((newammount = other->client->pers.inventory[ITEM_INDEX(wrndtbl[i].rnd_item[c])]) < ammount)
                                || (!minorweapon)
                                )
                                {
                                        minorweapon = wrndtbl[i].rnd_item[c];
                                        ammount = newammount;
                                }

                                c++;
                        }
                        ent->item = minorweapon;
                }
                else if (wrndtbl[i].mode == ITEM_GIVE)
                {
                        int c=1;

                        while ( c < wrndtbl[i].num)
                        {
                                ent->item = wrndtbl[i].rnd_item[c];
                                if (ent->item->pickup)
                                        ent->item->pickup(ent, other);
                                c++;
                        }

                        ent->item = wrndtbl[i].rnd_item[0];
                }
                else if (wrndtbl[i].mode == ITEM_CLASS)
                {
                        if ((!enableclass->value) || (other->client->resp.class >= wrndtbl[i].num))
                        {
                                ent->item = wrndtbl[i].rnd_item[0];
                        }
                        else
                                ent->item = wrndtbl[i].rnd_item[other->client->resp.class];
                }
        }

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

                //Wyrm: check before selecting it
		// change selected item
                if (ent->item->use && (other->client->pers.inventory[ITEM_INDEX(ent->item)]))
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

		if (ent->item->pickup == Pickup_Health)
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

//RWS: Return upgrading to the original item value
        if (i >= 0)
        if (wrndtbl[i].mode == ITEM_UPGRADE || wrndtbl[i].mode == ITEM_GIVE || wrndtbl[i].mode == ITEM_CLASS)
                ent->item = ent->org_item;

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

// ================
// PGM
void Use_IR (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->ir_framenum > level.framenum)
		ent->client->ir_framenum += 600;
	else
		ent->client->ir_framenum = level.framenum + 600;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ir_start.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item);
void TechThink(edict_t *tech);

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
//Wyrm: tech think...
        if (ent->item && ent->item->drop == CTFDrop_Tech)
        {
                ent->nextthink = level.time + CTF_TECH_TIMEOUT;
                ent->think = TechThink;
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
        //Wyrm: ir goggles
	dropped->s.renderfx = RF_GLOW | RF_IR_VISIBLE;		// PGM
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
//RWS
	gitem_t		* p;
	
	int			rt;

	// save the original item
	ent -> org_item = item;

//	gi.dprintf("Saving ... %s\n",ent->org_item->classname);

	if (ent->spawnflags & SPAWNFLAG_IGNORE_RWS)
		ent->spawnflags &= ~SPAWNFLAG_IGNORE_RWS;
	else
	{
		// find the random weapon spawn ...?
		if (p = FindRandomWeapon(ent,&rt))
			item = p;
		else
		{
			// if in table and FindRandomWeapon returned NULL, then must be 
			// be ITEM_REMOVE (if not, then it's something we don't know about
			// so leave it alone)
			if (rt == ITEM_REMOVE)
			{
				G_FreeEdict(ent);
				return;
			}
		}
	}

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
                        //Wyrm: medipak
                        if ( ((item->flags == IT_AMMO)&&(strcmp(ent->classname, "item_medipak"))) || (strcmp(ent->classname, "weapon_bfg") == 0) )
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

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);

        //Wyrm: copy start origin for respawning...
        VectorCopy(ent->s.origin, ent->monsterinfo.startorigin);
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


//Item nul! it is used in classes to define an item that a class can't pickup
	{
                "item_null", 
                NULL,
		NULL,
		NULL,
		NULL,
                "",
                "", 0,
		NULL,
/* icon */              "i_fixme",
/* pickup */    "NO item",
/* width */		3,
		0,
		NULL,
                0,
		0,
                NULL,
                0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen", 
                Pickup_PowerScreen,
                Use_PowerScreen,
                Drop_PowerScreen,
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},


	//
	// WEAPONS 
	//

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Blaster,
		"misc/w_pkup.wav",
                "models/weapons/g_blast/tris.md2", EF_ROTATE,
                "models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
                1,
                "Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/* weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
                "weapon_pistol", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Pistol,
		"misc/w_pkup.wav",
                "models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */    "Pistol",
		0,
                1,
                "Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/* weapon_flaregun (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
                "weapon_flaregun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Flaregun,
		"misc/w_pkup.wav",
                "models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */    "Flaregun",
		0,
                1,
                "Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/* weapon_bucky (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
                "weapon_bucky", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Bucky,
		"misc/w_pkup.wav",
                "models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */    "BuckyBall Gun",
		0,
                3,
                "Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "bucky.wav weapons/blastf1a.wav misc/lasfly.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "models/shell/tris.md2 weapons/shotgf1b.wav weapons/shotgr1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_airfist (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_airfist", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Airfist,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */    "Airfist",
		0,
                0,
                NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_blooddrainer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_blooddrainer", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_BloodDrainer,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */    "Blood Drainer",
		0,
                1,
                "Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav drainer/drain.wav",
/* 2nd usage */ 1,
/* 2nd ammo  */ "Slugs"
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
/* precache */ "models/shell/tris.md2 weapons/sshotf1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_doubleimpact (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_doubleimpact", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_DGSuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */    "Double Impact",
		0,
		2,
                "Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_flakcannon (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_flakcannon", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_FlakCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */    "Flak Cannon",
		0,
                1,
                "Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav",
/* 2nd usage */ 5,
/* 2nd ammo  */ "Bullets"
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
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
//Wyrm: new weapon
/*QUAKED weapon_explosive_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_explosive_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_ExplosiveMachinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */    "Explosive Machinegun",
		0,
                1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
/* 2nd usage */ 1,
/* 2nd ammo  */ "Rockets"
	},
//Wyrm: new weapon from rogue
/*QUAKED weapon_pulserifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_pulserifle", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_PulseRifle,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */    "Pulse Rifle",
		0,
                3,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "models/nail/tris.md2 weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav weapons/pulse1.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_nailgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Nailgun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */    "Nailgun",
		0,
                2,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "models/nail/tris.md2 weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_flamethrower", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Flamethrower,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */    "FlameThrower",
		0,
                1,
                "Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_streetsweeper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_streetsweeper", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_StreetSweeper,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */    "Street Sweeper",
		0,
                1,
                "Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "models/shell/tris.md2 weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_supernailgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_supernailgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_SuperNailgun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */    "Super Nailgun",
		0,
                2,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "models/nail/tris.md2 weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_tripbomb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_tripbomb",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Tripbomb,
		"misc/w_pkup.wav",
                "models/items/ammo/grenades/medium/tris.md2", 0,
                "models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */    "Tripbombs",
		0,
                2,
                "Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "lsrbmb.wav lsrbmbpt.wav ",
/* 2nd usage */ 2,
/* 2nd ammo  */ "Cells"
	},
/*QUAKED weapon_lasertripbomb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_lasertripbomb",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_LaserTripbomb,
		"misc/w_pkup.wav",
                "models/items/ammo/grenades/medium/tris.md2", 0,
                "models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */    "Laser Tripbombs",
		0,
                1,
                "Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "lsrbmb.wav lsrbmbpt.wav ",
/* 2nd usage */ 5,
/* 2nd ammo  */ "Cells"
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_clustergrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_clustergrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_ClusterGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "Cluster Grenades",
		0,
                3,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_bfgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_bfgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_BFGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "BFG Grenades",
		0,
                1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 20,
/* 2nd ammo  */ "Cells"
	},
/*QUAKED weapon_proxgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_proxgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_ProxGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "Proximity Grenades",
		0,
                1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 2,
/* 2nd ammo  */ "Cells"
	},
/*QUAKED weapon_railgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_railgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_RailGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "Rail Grenades",
		0,
                1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 2,
/* 2nd ammo  */ "Slugs"
	},
/*QUAKED weapon_stickinggrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_stickinggrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_StickingGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "Sticking Grenades",
		0,
                1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_napalmgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_napalmgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_NapalmGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */    "Napalm Grenades",
		0,
                2,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_napalmrockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_napalmrockets",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_NapalmRockets,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */    "Napalm Rockets",
		0,
                2,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_guided (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_guided",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_GuidedMisiles,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */    "Guided Misiles",
		0,
                1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
/* 2nd usage */ 3,
/* 2nd ammo  */ "Cells"
	},
/*QUAKED weapon_homing (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_homing",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Homing,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */    "Homing Rockets",
		0,
                1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2",
/* 2nd usage */ 3,
/* 2nd ammo  */ "Cells"
	},

/*QUAKED weapon_perforator (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_perforator",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Perforator,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */    "Perforating Rockets",
		0,
                1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2" "smack.wav",
/* 2nd usage */ 1,
/* 2nd ammo  */ "Slugs"
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_freezegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_freezegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Freezegun,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */    "Freezegun",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav weapons/frozen.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
//Wyrm: new weapon from Rogue
/*QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_disintegrator", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Disintegrator,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */    "Disruptor",
		0,
		1,
                "Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_plasmagun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_plasmagun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_PlasmaGun,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */    "Plasma Gun",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav weapons/plasexpl.wav weapons/plasma.wav models/objects/pbullet/tris.md2",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_asha (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_asha", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_AntimatterCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */    "Antimatter Cannon",
		0,
                3,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED weapon_positron (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_positron", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_PositronBeam,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */    "Positron Beam",
		0,
                2,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav",
/* 2nd usage */ 10,
/* 2nd ammo  */ "Cells"
	},
/*QUAKED weapon_lightninggun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_lightninggun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_LightningGun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */    "Lightning Gun",
                0,                  
                1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
                WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "light/lhit.wav light/lstart.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_nuke (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_nuke",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Nuke,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */    "Nuke",
		0,
                3,
                "Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav",
/* 2nd usage */ 3,
/* 2nd ammo  */ "Slugs"
	},

/*QUAKED weapon_energyvortex (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_energyvortex",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_EnergyVortex,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */    "Energy Vortex",
                0,                  
                100,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "models/items/keys/pyramid/tris.md2 grav_new.wav ",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED weapon_vacuummaker (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_vacuummaker",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_VacuumMaker,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */    "Vacuum Maker",
		0,
                1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

//Wyrm: turret item
/*QUAKED item_sentry (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_sentry",
                Pickup_Sentry,
                Use_Sentry,
                Drop_General,
		NULL,
		"items/pkup.wav",
                "models/items/lturret/tris.md2", EF_ROTATE,
		NULL,
/* icon */              "p_sentry",
/* pickup */ "Auto Sentry", /*ATTILA now we can use it with the use command*/
/* width */		2,
                60,
		NULL,
                IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "hover/hovidle1.wav items/damage.wav items/damage2.wav items/damage3.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
                //Wyrm: new effect for the quad
                "models/items/quaddama/tris.md2", EF_ROTATE|EF_FLAG2,
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
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
                //Wyrm: new effect for the 666
                "models/items/invulner/tris.md2", EF_ROTATE|EF_FLAG1,
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
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "items/airout.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "items/airout.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
		0,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED item_medipak (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_medipak",
                Pickup_MediPak,
                Use_MediPak,
                Drop_MediPak,
		NULL,
                "items/l_health.wav",
                "models/items/mega_h/tris.md2", EF_ROTATE,
		NULL,
/* icon */              "p_megahealth",
/* pickup */    "MediPak",
/* width */		2,
                100,
		NULL,
                IT_AMMO,
		0,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/* _________ *\
** ¯¯¯¯¯¯¯¯¯ **
** Nightmare **
** _________ **
\* ¯¯¯¯¯¯¯¯¯ */
/* item_flashlight (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
                "item_flashlight",
                Pickup_Key,
                Use_Flashlight,
                Drop_General,
		NULL,
		"misc/w_pkup.wav",
                "models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"a_blaster",
/* pickup */	"Flashlight",
		0,
		0,
		NULL,
                IT_KEY,
                0,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
//Nightmare
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
                "models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */              "p_goggles",
/* pickup */	"IR Goggles",
/* width */		2,
		60,
		NULL,
                IT_POWERUP,
                800,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
//Wyrm: jetpack item
/*QUAKED item_jet (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_jet",
                Pickup_Jetpak,
                Use_Jet,
                Drop_Jetpak,
		NULL,
		"items/pkup.wav",
                "models/items/jet/tris.md2", EF_ROTATE,
		NULL,
/* icon */              "p_jetpack",
/* pickup */ "Jetpack", /*ATTILA now we can use it with the use command*/
/* width */		2,
                60,
		NULL,
                IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "hover/hovidle1.wav items/damage.wav items/damage2.wav items/damage3.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED item_steroids (.3 .3 1) (-16 -16 -16) (16 16 16)
Powerup 2x velocity
*/
	{
                "item_steroids",
		Pickup_Powerup,
                Use_Steroids,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */    "Steroids",
/* width */		2,
		60,
		NULL,
                IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED item_cloaker (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_cloaker", 
		Pickup_Powerup,
                Use_Cloak,
		Drop_General,
		NULL,
                "items/pkup.wav",
                "models/items/cloak/tris.md2", EF_ROTATE,
		NULL,
/* icon */              "p_cloak",
/* pickup */    "Cloaker",
/* width */		2,
		60,
		NULL,
                IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "cloak/activate.wav cloak/off.wav cloak/running.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
/*QUAKED item_scanner (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_scanner", 
                Pickup_Scanner,
                Use_Scanner,
                Drop_Scanner,
		NULL,
		"items/pkup.wav",
                "models/scanner/radar.md2", EF_ROTATE,
		NULL,
/* icon */              "p_scanner",
/* pickup */    "Scanner",
/* width */		2,
		60,
		NULL,
                IT_POWERUP,
		0,
		NULL,
		0,
/* precache */  "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

/*QUAKED item_beans (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_beans", 
                Pickup_Powerup,
                Use_Beans,
                Drop_General,
		NULL,
		"items/pkup.wav",
                "models/copb/beans.md2", EF_ROTATE,
		NULL,
/* icon */              "p_copb",
/* pickup */    "Beans",
/* width */		2,
                100,
		NULL,
                IT_POWERUP,
		0,
		NULL,
		0,
/* precache */  "copb/copb_1.wav copb/copb_2.wav copb/copb_3.wav copb/copb_4.wav copb/copb_5.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},
//Wyrm: added for RWS and name
	{
                "item_health_mega",
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
                "models/items/mega_h/tris.md2", 0,
		NULL,
/* icon */              "p_megahealth",
/* pickup */    "MegaHealth",
/* width */		3,
                100,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
		0,
                0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "ctf/tech1.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "ctf/tech2.wav ctf/tech2x.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "ctf/tech3.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
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
/* precache */ "ctf/tech4.wav",
/* 2nd usage */ 0,
/* 2nd ammo  */ NULL
	},

//ZOID
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

	self->count = 100;
        //Wyrm: modified for name
        SpawnItem (self, FindItem ("MegaHealth"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}

//Wyrm: medipak
/*QUAKED item_medipak (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_medipak (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->count = 100;
        SpawnItem (self, FindItem ("MediPak"));
}

//Wyrm: jetpack
void SP_item_jetpak (edict_t *self)
{
        self->count = 800;
        SpawnItem (self, FindItem ("Jetpak"));
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
        //Wyrm: tech
        tech_resistance_index   = ITEM_INDEX(FindItemByClassname("item_tech1"));
        tech_strength_index     = ITEM_INDEX(FindItemByClassname("item_tech2"));
        tech_haste_index        = ITEM_INDEX(FindItemByClassname("item_tech3"));
        tech_regeneration_index = ITEM_INDEX(FindItemByClassname("item_tech4"));
}

//Wyrm: MediPak item
qboolean        Pickup_MediPak (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

        if (quantity >= 200)
		return false;

        other->client->pers.inventory[ITEM_INDEX(ent->item)] += ent->count;
        if (other->client->pers.inventory[ITEM_INDEX(ent->item)] > 200)
                other->client->pers.inventory[ITEM_INDEX(ent->item)] = 200;

        if (deathmatch->value)
                SetRespawn (ent, 60);

	return true;
};

//Wyrm: JetPak item
qboolean        Pickup_Jetpak (edict_t *ent, edict_t *other)
{
        int             ammount;

        if (other->client->pers.inventory[ITEM_INDEX(ent->item)] >= 800)
		return false;

        ammount = ent->count;
        if (!ammount)
                ammount = 800;

        other->client->pers.inventory[ITEM_INDEX(ent->item)] += ammount;
        if (other->client->pers.inventory[ITEM_INDEX(ent->item)] > 800)
                other->client->pers.inventory[ITEM_INDEX(ent->item)] = 800;

        if (deathmatch->value)
                SetRespawn (ent, 60);

	return true;
};

void Use_MediPak (edict_t *ent, gitem_t *inv)
{
        int heal;
        int medipak=ent->client->pers.inventory[ITEM_INDEX(inv)];

        if (ent->health >= ent->max_health)
                return;

        heal = ent->max_health - ent->health;
        if (heal > medipak)
                heal = medipak;

        if (heal)
        {
                ent->client->pers.inventory[ITEM_INDEX(inv)] -= heal;
                ValidateSelectedItem (ent);
                ent->health += heal;
                gi.sound(ent, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
        }

};

void Drop_MediPak (edict_t *ent, gitem_t *item)
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

/*ATTILA begin*/
void Use_Jet ( edict_t *ent, gitem_t *item )
{
    if(ent->client->onturret)
    {
        //You cannot use the jetpack while on a turret!!
        gi.cprintf (ent, PRINT_HIGH, "You cannot use the Jetpack on a turret!\n");
        return;
    }

    /*The On/Off Sound taken from the invulnerability*/
    gi.sound( ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 0.8, ATTN_NORM, 0 );

    if ( Jet_Active(ent) )
      ent->client->Jet_framenum = 0; 
    else
    {
      ent->client->Jet_framenum = level.framenum + ent->client->pers.inventory[ITEM_INDEX(FindItem("Jetpack"))];


    /*this is the sound played when flying. To here this sound 
      immediately we play it here the first time*/
    gi.sound ( ent, CHAN_AUTO, gi.soundindex("hover/hovidle1.wav"), 0.8, ATTN_NORM, 0 );
    }
}
/*ATTILA end*/

void Drop_Jetpak (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

        if (Jet_Active(ent))
        {
                gi.cprintf (ent, PRINT_HIGH, "You're using it!!\n");
        }
	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
        dropped->count = ent->client->pers.inventory[index];

        ent->client->pers.inventory[index] = 0;
	ValidateSelectedItem (ent);
}

void Use_Steroids (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

        if (ent->client->steroids_framenum > level.framenum)
                ent->client->steroids_framenum += 300;
	else
                ent->client->steroids_framenum = level.framenum + 300;
        gi.sound( ent, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 0.8, ATTN_NORM, 0 );
}
void    Use_Cloak (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

        if (ent->client->cloak_framenum > level.framenum)
                ent->client->cloak_framenum += 600;
	else
                ent->client->cloak_framenum = level.framenum + 600;

        gi.sound (ent, CHAN_ITEM, gi.soundindex("cloak/activate.wav"), 1, ATTN_NORM, 0);
}

//Wyrm: scanner item
qboolean        Pickup_Scanner (edict_t *ent, edict_t *other)
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
	}

	return true;
};

void Use_Scanner (edict_t *ent, gitem_t *inv)
{
        Toggle_Scanner (ent);
};

void Drop_Scanner (edict_t *ent, gitem_t *item)
{
	int		index;

	index = ITEM_INDEX(item);

        Drop_Item (ent, item);
        ent->client->pers.inventory[index]--;

        if (!ent->client->pers.inventory[index])
                ent->client->pers.scanner_active = 0;

	ValidateSelectedItem (ent);
}

//Wyrm:  turret item
qboolean        Pickup_Sentry (edict_t *ent, edict_t *other)
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
	}

	return true;
};

void Launch_Sentry (edict_t *ent);
void Use_Sentry (edict_t *ent, gitem_t *inv)
{
        int index=ITEM_INDEX(inv);

        if (!ent->client->pers.inventory[index])
                return;

        gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/turret/throw.wav"), 1, ATTN_NORM, 0);

        Launch_Sentry(ent);

        ent->client->pers.inventory[index]--;
        ValidateSelectedItem(ent);
};


void Use_Beans (edict_t *ent, gitem_t *item)
{
        int index = ITEM_INDEX(item);
        //Effect
        ent->health += 150;
        if (ent->health > (ent->max_health*2.5))
                ent->health = ent->max_health * 2.5;

        //Secondary effect :)
        if (ent->client->beans_framenum > level.framenum)
                ent->client->beans_framenum += 250;
	else
                ent->client->beans_framenum = level.framenum + 251;

        ent->client->pers.inventory[index]--;
        ValidateSelectedItem(ent);
}

