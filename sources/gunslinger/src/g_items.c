#include "g_local.h"
#include "z_gq.h"

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Bowie_Knife (edict_t *ent);
void Weapon_Revolver (edict_t *ent);
void Weapon_DoubleRevolver (edict_t *ent);
void Weapon_Coachgun (edict_t *ent);
void Weapon_Pump_action_shotgun (edict_t *ent);
void Weapon_Winchester_Rifle (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Hatchet (edict_t *ent);
void Weapon_Molotov_Cocktail (edict_t *ent);
void Weapon_Sharps_Rifle (edict_t *ent);
void Weapon_Cannon (edict_t *ent);

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

//======================================================================

void CalcWeight (edict_t *ent) {
	int		index, i;
	float	totalmass=0;
	if (!item_weight->value || !deathmatch->value) 
		return;

	for (i=0 ; i<MAX_ITEMS ; i++) {
		if (ent->client->pers.inventory[i]) {
			totalmass+=(itemlist[i].weight * ent->client->pers.inventory[i]);
		}
		if (ent->client->pers.carried[i]) {
			index=ITEM_INDEX(FindItem(itemlist[i].ammo));
			totalmass+=(itemlist[index].weight * ent->client->pers.carried[i]);
		}
	}
//	gi_bprintf(PRINT_HIGH, "Carried: %.2f\n", totalmass);
	if (GQ_MatchItem(ent->client->artifact,"Strength of the Bear"))
		totalmass=totalmass/3;

	ent->client->pers.weight=totalmass;
}

// Eraser
/*
===============
AddToItemList

  Adds an item to the respective linked list, returns the head of that list
===============
*/
/*
edict_t	*AddToItemList(edict_t *ent, edict_t	*head)
{
	edict_t *trav;

	// make sure this item isn't already in the list
	trav = head;
	while (trav)
	{
		if (trav == ent)		// already in there!
			return head;

		trav = trav->node_target;
	}

	if (head)
		head->last_goal = ent;

	ent->node_target = head;
	ent->last_goal = NULL;

	return ent;
}

void	RemoveFromItemList(edict_t *ent)
{
	if (ent->node_target)
		ent->node_target->last_goal = ent->last_goal;

	if (ent->last_goal)
	{
		ent->last_goal->node_target = ent->node_target;
//		ent->last_goal->last_goal = NULL;
	}
	else if (ent->node_target)	// ent must be head
	{
		if (ent->item->pickup == Pickup_Weapon)
			weapons_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Health)
			health_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Ammo)
			ammo_head = ent->node_target;
		else
			bonus_head = ent->node_target;
	}

	ent->node_target = NULL;
	ent->last_goal = NULL;
}

void	RemoveDroppedItem(edict_t	*ent)
{
	RemoveFromItemList(ent);
	G_FreeEdict(ent);
}
// Eraser end*/

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
		if (((int)playmode->value == PM_BADGE_WARS) && (ent->item->use == Use_Quad)) {
			other->s.modelindex3 = gi.modelindex("models/items/badge/tris.md2");
			other->client->team = 2;
			ent->spawnflags |= DROPPED_ITEM;
			other->client->quad_framenum = level.framenum + 1000;
			other->client->badge_counter=0;
			return true;
		}
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

qboolean Pickup_Lantern (edict_t *ent, edict_t *other) {
	if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
		return false;
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
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

	if (other->client->pers.max_bullets45LC < 100)
		other->client->pers.max_bullets45LC = 100;
	if (other->client->pers.max_shells < 50)
		other->client->pers.max_shells = 50;
	if (other->client->pers.max_bullets3030 < 50)
		other->client->pers.max_bullets3030 = 50;
	if (other->client->pers.max_bullets4570 < 35)
		other->client->pers.max_bullets4570 = 35;

	item = FindItem(".45 Colt Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets45LC)
			other->client->pers.inventory[index] = other->client->pers.max_bullets45LC;
	}

	item = FindItem(".30-30 Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets3030)
			other->client->pers.inventory[index] = other->client->pers.max_bullets3030;
	}

	item = FindItem(".45-70 Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets4570)
			other->client->pers.inventory[index] = other->client->pers.max_bullets4570;
	}

	item = FindItem("12 Gauge Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	CalcWeight(other);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;
	
	if (other->client->pers.max_bullets45LC < 200)
		other->client->pers.max_bullets45LC = 200;
	if (other->client->pers.max_shells < 100)
		other->client->pers.max_shells = 100;
	if (other->client->pers.max_hatchets < 10)
		other->client->pers.max_hatchets = 10;
	if (other->client->pers.max_cocktails < 10)
		other->client->pers.max_cocktails = 10;
	if (other->client->pers.max_bullets3030 < 100)
		other->client->pers.max_bullets3030 = 100;
	if (other->client->pers.max_bullets4570 < 50)
		other->client->pers.max_bullets4570 = 50;
	if (other->client->pers.max_cannonballs < 24)
		other->client->pers.max_cannonballs = 24;

	item = FindItem(".45 Colt Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets45LC)
			other->client->pers.inventory[index] = other->client->pers.max_bullets45LC;
	}

	item = FindItem(".30-30 Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets3030)
			other->client->pers.inventory[index] = other->client->pers.max_bullets3030;
	}

	item = FindItem(".45-70 Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets4570)
			other->client->pers.inventory[index] = other->client->pers.max_bullets4570;
	}

	item = FindItem("12 Gauge Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Molotov Cocktail");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cocktails)
			other->client->pers.inventory[index] = other->client->pers.max_cocktails;
	}

	item = FindItem("Hatchet");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_hatchets)
			other->client->pers.inventory[index] = other->client->pers.max_hatchets;
	}
/*
	item = FindItem("Cannonballs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cannonballs)
			other->client->pers.inventory[index] = other->client->pers.max_cannonballs;
	}
*/
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	CalcWeight(other);

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


void Use_Lantern (edict_t *ent, gitem_t *item)
{
//	debugmsg("Using lantern.\n");
	if (ent->client->pers.inventory[ITEM_INDEX(item)]) {
		if (ent->flags & FL_LANTERN) {
//			debugmsg("Turning off.\n");
			ent->flags &= ~FL_LANTERN;
		} else {
//			debugmsg("Turning on.\n");
			ent->flags |= FL_LANTERN;
		}
	} else {
//		debugmsg("None carried.\n");
	}
//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}


//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	if (!ent->client)
		return;

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
	if (!ent->client)
		return;

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
	if (!ent->client)
		return;

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
	if (!ent->client)
		return;

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

	if (item->tag == AMMO_BULLETS45LC)
		max = ent->client->pers.max_bullets45LC;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_HATCHETS)
		max = ent->client->pers.max_hatchets;
	else if (item->tag == AMMO_COCKTAILS)
		max = ent->client->pers.max_cocktails;
	else if (item->tag == AMMO_BULLETS3030)
		max = ent->client->pers.max_bullets3030;
	else if (item->tag == AMMO_BULLETS4570)
		max = ent->client->pers.max_bullets4570;
	else if (item->tag == AMMO_CANNONBALLS)
		max = ent->client->pers.max_cannonballs;
	else
		return false;

	index = ITEM_INDEX(item);
	
	if (ent->client->pers.inventory[index] >= max)
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

/*	debugmsg("Clipmask: %i\n", ent->clipmask);
	debugmsg("Solid: %i\n", ent->solid);
	debugmsg("Movetype: %i\n", ent->movetype);
	debugmsg("Flags: %i\n", ent->flags);
*/
	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count)) {
		return false;
	}

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || (other->client->pers.weapon->weapmodel==WEAP_BOWIE_KNIFE) ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);

	CalcWeight(other);

	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	if (!ent->client)
		return;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (item->tag==AMMO_CANNONBALLS) {
		dropped->velocity[0]=0;
		dropped->velocity[1]=0;
		dropped->nextthink = level.time + 5;
	}
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];

	if (ent->client->pers.weapon && 
		ent->client->pers.weapon->tag == AMMO_COCKTAILS &&
		item->tag == AMMO_COCKTAILS &&
		ent->client->pers.inventory[index] - dropped->count <= 0) {
		gi_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		G_FreeEdict(dropped);
		return;
	}

	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);

	CalcWeight(ent);
}

void Drop_Lantern (edict_t *self, gitem_t *item) {
	if (self->flags & FL_LANTERN) {
		self->flags &= ~FL_LANTERN;
	}
	Drop_General(self, item);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	debugmsg("MegaHealth think\n");
	if (self->owner->health > self->owner->max_health)
	{
		debugmsg("Health over max\n");
		self->nextthink = level.time + 1;
		self->think = MegaHealth_think;
		if ((!GQ_MatchItem(self->owner->client->artifact, "Tongue of the Leech") && !GQ_MatchItem(self->owner->client->artifact, "Flesh of the Salamander")) || (self->owner->health > self->owner->max_health*2)) {
			self->owner->health -= 1;
			debugmsg("Not carrying the artifacts\n");
			return;
		} else {
			debugmsg("Are carrying the special artifacts!\n");
		}
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value)) {
		SetRespawn (self, 20);
	} else {
		G_FreeEdict (self);
		debugmsg("Freeing MegaHealth\n");
	}
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX)) {
		if (other->health >= other->max_health) {
			return false;
		}
	}

	other->health += ent->count;

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

	if (!ent->client)
		return false;

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
			if (other->client->pers.inventory[old_armor_index] >= newcount) {
				return false;
			}

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	CalcWeight(other);

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
/*	int		index;

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
			// Eraser
			if (!ent->bot_client)
				gi_cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			// Eraser end
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
*/
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

//	gi_bprintf(PRINT_HIGH,"Touched item\n");
	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?
	// Antibot
	if (ent->flags & FL_ANTIBOT)
		return;

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		if (ent->item->drop)
			other->item=ent->item;
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if ((ent->item->use || ent->item->drop) && (other->client->pers.inventory[ITEM_INDEX(ent->item)]>0))
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

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else {
			G_FreeEdict (ent);
		}
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
	// FIXME: Should the following go in Drop_Weapon?? -- Stone
	if (deathmatch->value)
	{
		if (!weapons_unique->value || !(ent->item->flags & IT_WEAPON)) {
			ent->nextthink = level.time + 29;
			ent->think = G_FreeEdict;
		} else {
			// After 5 minutes, spawn the weapon at its original spawn point!
			ent->nextthink = level.time + 300;
			ent->think = GQ_DelayedRespawn;
		}
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
	dropped->clipmask = MASK_PLAYERSOLID;
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

void Make_Dropped (edict_t *ent, gitem_t *item) {
//	debugmsg("Make_Dropped\n");
	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.renderfx = RF_GLOW;
//	VectorSet (ent->mins, -15, -15, -15);
//	VectorSet (ent->maxs, 15, 15, 15);
	ent->solid = SOLID_TRIGGER;
	ent->touch = drop_temp_touch;
	ent->owner = ent;
	ent->think = drop_make_touchable;
	ent->nextthink = level.time + .1;
//	gi_bprintf(PRINT_HIGH, "Item made dropped.\n");
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

	CalcWeight(other);

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
	// Stone
	int			mask;

	if (ent->clipmask)
		mask=ent->clipmask;
	else
		mask=MASK_SOLID;
	// Stone

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
	ent->s.renderfx |= RF_IR_VISIBLE;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, mask);
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
	if (it->reload_sound)
		gi.soundindex (it->reload_sound);
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
	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING)) {
		G_FreeEdict(ent);
		return;
	}

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
		if (!allow_cannon->value && GQ_MatchItem(item,"Cannon")) {
			G_FreeEdict (ent);
			return;
		}
		if (!allow_cannon->value && GQ_MatchItem(item,"Cannonballs")) {
			G_FreeEdict (ent);
			return;
		}
		if (GQ_MatchItem(item,"Sheriff Badge") && (!allow_sheriff_badge->value || ((int)playmode->value == PM_BADGE_WARS))) {
			G_FreeEdict (ent);
			return;
		}
		if (GQ_MatchItem(item,"Marshal Badge") && !allow_marshal_badge->value) {
			G_FreeEdict (ent);
			return;
		}
		if (GQ_MatchItem(item,"Bandolier") && !allow_bandolier->value) {
			G_FreeEdict (ent);
			return;
		}
		if (GQ_MatchItem(item,"Ammo Pack") && !allow_pack->value) {
			G_FreeEdict (ent);
			return;
		}
		if (GQ_MatchItem(item,"Lantern") && !allow_lantern->value) {
			G_FreeEdict (ent);
//			debugmsg("Killing lantern object.\n");
			return;
		}
		if (GQ_MatchItem(item,"Money Bag") && ((int)number_of_money_bags->value<1)) {
			G_FreeEdict (ent);
			return;
		}
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ((int)dmflags->value & DF_NO_ITEMS)
		{
			if (item->pickup == Pickup_Powerup)
			{
				if (!(item->use == Use_Quad) || !((int)playmode->value==PM_BADGE_WARS)) {
					G_FreeEdict (ent);
					return;
				}
			}
		}
//		if ( (int)dmflags->value & DF_NO_HEALTH )
		if (!allow_bandages->value)
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

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
//	if (!Q_strcasecmp(ent->classname,"item_quad"))
//		ent->s.renderfx = RF_SHELL_RED;
	if (ent->model)
		gi.modelindex (ent->model);
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
	Program crashes if some armors are commented out.*/
	{
		"nullitem_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price, price
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},


/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) */

	{
		"nullitem_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price, price
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)*/

	{
		"nullitem_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price, price
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)*/

	{
		"nullitem_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)*/

	{
		"nullitem_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,

/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */	""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16) */

	{
		"nullitem_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price
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

/* weapon_bowie_knife (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_bowie_knife", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Bowie_Knife,
		"misc/w_pkup.wav",
		"",
		NULL, 0,
		"models/weapons/v_bowie/tris.md2",
/* icon */		"w_bowie",
/* pickup */	"Bowie Knife",
		0,
		0, 0, // quantities
		0, 0, // capacity & reload time
		0.0, 0,	// weight, price
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BOWIE_KNIFE,
		NULL,
		0,
/* precache */ "brain/melee1.wav gladiator/melee3.wav weapons/hgrenb2a.wav"
	},

/*QUAKED weapon_revolver (.3 .3 1) (-16 -16 -16) (16 16 16)  Colt Peacemaker
*/
	{
		"weapon_nullrevolver",	// Basic revolver does not appear in levels
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Revolver,
		"misc/w_pkup.wav",
		"gunslinger/loadrevolver.wav",
		"models/weapons/g_revolv/tris.md2", EF_ROTATE,
		"models/weapons/v_revolv/tris.md2",
/* icon */		"w_revolver",
/* pickup */	"Revolver",
		0,
		1, 1, // quantities
		6, 10, // capacity & reload time
		0.0, 0,	// weight, price
		".45 Colt Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_REVOLVER,
		NULL,
		0,
/* precache */ "gunslinger/rev1-1.wav gunslinger/rev1-2.wav gunslinger/rev1-3.wav world/ric1.wav world/ric2.wav world/ric3.wav weapons/blastf1a.wav"
	},

/*QUAKED weapon_doublerevolver (.3 .3 1) (-16 -16 -16) (16 16 16)  Double Peacemakers
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_DoubleRevolver,
		"misc/w_pkup.wav",
		"gunslinger/loaddrevolver.wav",
		"models/weapons/g_revolv/tris.md2", EF_ROTATE,
		"models/weapons/v_double/tris.md2",
/* icon */		"w_drevolver",
/* pickup */	"Double Revolver",
		0,
		1, 1, // quantities
		6, 20, // capacity & reload time
		2.5, 25,	// weight, price
		".45 Colt Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_DOUBLEREVOLVER,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_coachgun (.3 .3 1) (-16 -16 -16) (16 16 16)  Coach Shotgun
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Coachgun,
		"misc/w_pkup.wav",
		"gunslinger/loadcoach.wav",
		"models/weapons/g_coach/tris.md2", EF_ROTATE,
		"models/weapons/v_coach/tris.md2",
/* icon */		"w_coachgun",
/* pickup */	"Coach gun",
		0,
		1, 1, // quantities
		2, 10, // capacity & reload time
		6.5, 50,	// weight, price
		"12 Gauge Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_COACHGUN,
		NULL,
		0,
/* precache */ "gunslinger/shotgun1a.wav gunslinger/shotgun1b.wav"
	},

/*QUAKED weapon_pump_action_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)  Winchester Model 97 Shotgun
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Pump_action_shotgun,
		"misc/w_pkup.wav",
		"gunslinger/loadpump.wav",
		"models/weapons/g_pump/tris.md2", EF_ROTATE,
		"models/weapons/v_pump/tris.md2",
//		"models/weapons/g_chain/tris.md2", EF_ROTATE,
//		"models/weapons/v_chain/tris.md2",
/* icon */		"w_pump_action",
/* pickup */	"Pump Shotgun",
		0,
		1, 1, // quantities
		5, 7, // capacity & reload time
		7.0, 120,	// weight, price
		"12 Gauge Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_PUMPACTIONSHOTGUN,
		NULL,
		0,
/* precache */ "gunslinger/shotgu2c.wav gunslinger/shotgu2d.wav"
	},

/*QUAKED ammo_cocktail (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Molotov_Cocktail,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/bottle/tris.md2", EF_ROTATE,
		"models/weapons/v_bottle/tris.md2",
/* icon */		"a_cocktail",
/* pickup */	"Molotov Cocktail",
/* width */		3,
		1, 0, // quantities
		0, 0, // capacity & reload time
		1.0, 10,	// weight, price
		"Molotov Cocktail",
		IT_WEAPON|IT_AMMO,
		WEAP_COCKTAIL,
		NULL,
		AMMO_COCKTAILS,
/* precache */ "gunslinger/cocktail.wav models/objects/explode/tris.md2 models/objects/debris2/tris.md2 player/burn1.wav player/burn2.wav sprites/s_explo2.sp2 weapons/bfg__l1a.wav world/brkglas.wav models/objects/bottle/tris.md2"
	},

/*QUAKED ammo_hatchets (.3 .3 1) (-16 -16 -16) (16 16 16)  Hatchet
*/
	{
		"weapon_rocketlauncher",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Hatchet,
		"misc/w_pkup.wav",
		"",
		"models/items/ammo/hatchet/tris.md2", EF_ROTATE,
		"models/weapons/v_hatchet/tris.md2",
//		"models/weapons/v_rocket/tris.md2",
/* icon */		"a_hatchet",
/* pickup */	"Hatchet",
		0,
		1, 0, // quantities
		0, 0, // capacity & reload time
		1.5, 7,	// weight, price
		"Hatchet",
		IT_WEAPON|IT_AMMO,
		WEAP_HATCHET,
		NULL,
		AMMO_HATCHETS,
/* precache */ "gladiator/melee3.wav weapons/hgrenb2a.wav models/objects/hatchet/tris.md2"
	},

/*QUAKED weapon_winchester_rifle (.3 .3 1) (-16 -16 -16) (16 16 16)  Winchester Model 94 Rifle
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Winchester_Rifle,
		"misc/w_pkup.wav",
		"gunslinger/loadlever.wav",
		"models/weapons/g_lever/tris.md2", EF_ROTATE,
		"models/weapons/v_lever/tris.md2",
/* icon */		"w_winchester",
/* pickup */	"Winchester Rifle",
		0,
		1, 1, // quantities
		7, 8, // capacity & reload time
		6.0, 90,	// weight, price
		".30-30 Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_WINCHESTER_RIFLE,
		NULL,
		0,
/* precache */ "gunslinger/winch1.wav gunslinger/winch2.wav"
	},

/*QUAKED weapon_sharps_rifle (.3 .3 1) (-16 -16 -16) (16 16 16)  Sharps No. 1 Rifle
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sharps_Rifle,
		"misc/w_pkup.wav",
		"gunslinger/loadsharps.wav",
		"models/weapons/g_sharps/tris.md2", EF_ROTATE,
		"models/weapons/v_sharps/tris.md2",
/* icon */		"w_sharps",
/* pickup */	"Sharps Rifle",
		0,
		1, 0, // quantities
		1, 15, // capacity & reload time
		11.0, 120,	// weight, price
		".45-70 Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHARPS_RIFLE,
		NULL,
		0,
/* precache */ "gunslinger/sharps.wav"
	},

/*QUAKED weapon_cannon (.3 .3 1) (-16 -16 -16) (16 16 16)  Cannon
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Cannon,
		"misc/w_pkup.wav",
		"gunslinger/loadcannon.wav",
		"models/weapons/g_cannon/tris.md2", EF_ROTATE,
		"models/weapons/v_cannon/tris.md2",
/* icon */		"w_cannon",
/* pickup */	"Cannon",
		0,
		1, 1, // quantities
		1, 20, // capacity & reload time
		300.0, 150,	// weight, price
		"Cannonballs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CANNON,
		NULL,
		0,
/* precache */ "gunslinger/light3.wav gunslinger/cannon3.wav"
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",		// Put shells where bullets are supposed to be?
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"12 Gauge Shells",
/* width */		3,
		5, 0, // quantities
		0, 0,
		0.12, 12,	// weight, price
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets45LC (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",		// Put bullets where shells are supposed to be?
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	".45 Colt Bullets",
/* width */		3,
		10, 0, // quantities
		0, 0,
		0.06, 8,	// weight, price
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS45LC,
/* precache */ ""
	},

/*QUAKED ammo_bullets3030 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	".30-30 Bullets",
/* width */		3,
		10, 0, // quantities
		0, 0,
		0.05, 7,	// weight, price
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS3030,
/* precache */ ""
	},

/*QUAKED ammo_bullets4570 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */		"a_slugs",
/* pickup */	".45-70 Bullets",
/* width */		3,
		10, 0, // quantities
		0, 0,
		0.08, 6,	// weight, price
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS4570,
/* precache */ ""
	},

/*QUAKED ammo_cannonballs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"",
		"models/items/ammo/cannonball/tris.md2", 0,
		NULL,
/* icon */		"a_cannon",
/* pickup */	"Cannonballs",
/* width */		3,
		4, 0, // quantities
		0, 0,
		10.0, 5,	// weight, price
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CANNONBALLS,
/* precache */ ""
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED item_lantern (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_adrenaline",
		Pickup_Lantern,
		Use_Lantern,
		Drop_Lantern,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/lantern/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_lantern",
/* pickup */	"Lantern",
/* width */		2,
		60, 0, // quantities
		0, 0,
		1.0, 0,	// weight, price
		NULL,
		IT_STAY_COOP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad", 
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/sheriff/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_sheriff",
/* pickup */	"Sheriff Badge",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav models/items/badge/tris.md2"
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
		"",
		"models/items/marshal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_marshal",
/* pickup */	"Marshal Badge",
/* width */		2,
		300, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_money_bag (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_money_bag",
		GQ_PickupMoneyBag,
		NULL,
		GQ_DropMoneyBag,
		NULL,
		"gunslinger/money.wav",
		"",
		"models/items/moneybag/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_money",
/* pickup */	"Money Bag",
/* width */		2,
		RF_GLOW, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_POWERUP,
		0,
		NULL,
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
		"",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.0, 0,	// weight, price
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
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60, 0, // quantities
		0, 0,
		0.5, 0,	// weight, price
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
		"",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		180, 0, // quantities
		0, 0,
		2.0, 0,	// weight, price
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
		"",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
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
		"",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Bandages",
/* width */		3,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Laudanum",
/* width */		3,
		0, 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

/*QUAKED artifact_rapid_reload (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Hands of Lightning
*/
	{
		"artifact_rapid_reload",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_yellow",
/* pickup */	"Hands of Lightning",
/* width */		2,
		(RF_SHELL_RED|RF_SHELL_GREEN), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_strength (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Strength of the Bear
*/
	{
		"artifact_strength",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_cyan",
/* pickup */	"Strength of the Bear",
/* width */		2,
		(RF_SHELL_GREEN|RF_SHELL_BLUE), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_regeneration (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Flesh of the Salamander
*/
	{
		"artifact_regeneration",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_white",
/* pickup */	"Flesh of the Salamander",
/* width */		2,
		(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_pain (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Aura of Pain
*/
	{
		"artifact_pain",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_green",
/* pickup */	"Aura of Pain",
/* width */		2,
		(RF_SHELL_GREEN), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_vampire (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Tongue of the Leech
*/
	{
		"artifact_vampire",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_purple",
/* pickup */	"Tongue of the Leech",
/* width */		2,
		(RF_SHELL_RED|RF_SHELL_BLUE), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_poison (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Tail of the Scorpion
*/
	{
		"artifact_poison",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_red",
/* pickup */	"Tail of the Scorpion",
/* width */		2,
		(RF_SHELL_RED), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED artifact_darkness (0 .5 .8) (-16 -16 -16) (16 16 16)
Artifact - Shroud of Darkness
*/
	{
		"artifact_darkness",
		GQ_PickupArtifact,
		NULL,
		GQ_DropArtifact,
		NULL,
		"items/pkup.wav",
		"",
		"models/items/quaddama/tris.md2", EF_ROTATE | EF_COLOR_SHELL,
		NULL,
/* icon */		"z_blue",
/* pickup */	"Shroud of Darkness",
/* width */		2,
		(RF_SHELL_BLUE), 0,	// quantities
		0, 0,
		0.0, 0,	// weight, price
		NULL,
		IT_ARTIFACT,
		0,
		NULL,
		0,
/* precache */ ""
	},

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
//	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	if ( deathmatch->value && !allow_bandages->value )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Bandages"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
//	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	if ( deathmatch->value && !allow_bandages->value )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Bandages"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
//	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	if ( deathmatch->value && !allow_bandages->value )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Bandages"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
//	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	if (deathmatch->value && !allow_laudanum->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Laudanum"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}


void InitItems (void)
{
	debugmsg("Running InitItems\n");
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
