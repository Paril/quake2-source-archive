#include "g_local.h"

qboolean Pickup_Generic (edict_t *ent, edict_t *other);
qboolean Pickup_Powerup (edict_t *ent, edict_t *other);

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

void Use_Quad (edict_t *ent, gitem_t *item);
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


//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += 300;
	else
		ent->client->quad_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

void Use_Invuln (edict_t *ent, gitem_t *item)
{
	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;
}

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
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
		SetRespawn (self, 5);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

	other->health += ent->dmg;

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
			SetRespawn (ent, 20);
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

		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->dmg == 10)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_IDLE, 0);
			else if (ent->dmg == 25)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_IDLE, 0);
			else if (ent->dmg == 50)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_IDLE, 0);
			else // (ent->count == 100)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_IDLE, 0);
		}
		else if (ent->item->pickup_sound)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_IDLE, 0);
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
	PrecacheItem (item);

	if (item->flags & IT_JUMPSCORE) {
		ent->jumpinfo = gi.TagMalloc (sizeof(jumpinfo_t), TAG_LEVEL);
		ent->count = ++item->count;
		ent->style = level.itemstyle++;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;

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
	//
	// WEAPONS 
	//

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_JUMPSCORE,
		WEAP_BFG,
		NULL,
		0,
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
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_JUMPSCORE,
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
		Use_Invuln,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_JUMPSCORE,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		NULL,
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
		NULL,
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
		Pickup_Generic,
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
		IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"AOL 7.0 Platinum",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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
		Pickup_Generic,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_JUMPSCORE,
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

	// end of list marker
	{NULL}
};

void JumpScore (edict_t *ent, edict_t *other)
{
	unsigned int	total;
	unsigned int	mins;
	float	secs;

	if (other->client->resp.checkpoint) {
		return;
	}

	total = level.framenum - other->client->respawn_time;

	if (total > 63356)
		return;

	mins = floor (total / 600);
	secs = (float)(total - (600 * mins)) / 10;

	if (ent->item->count > 1)
		gi.bprintf (PRINT_HIGH, "%s got the #%d %s in %d mins %.1f secs! [%s%.1f]\n", other->client->pers.netname, ent->count, ent->item->pickup_name, mins, secs, total > ent->jumpinfo->time ? "+" : "", (float)((int)total - (int)ent->jumpinfo->time) / 10.0f);
	else
		gi.bprintf (PRINT_HIGH, "%s got the %s in %d mins %.1f secs [%s%.1f]!\n", other->client->pers.netname, ent->item->pickup_name, mins, secs, total > ent->jumpinfo->time ? "+" : "", (float)((int)total - (int)ent->jumpinfo->time) / 10.0f);

	if (level.disable_scoring)
		return;

	if (!ent->jumpinfo->time || total < ent->jumpinfo->time) {
		if (!ent->jumpinfo->time) {
			if (ent->item->count > 1)
				gi.bprintf (PRINT_HIGH, "%s set the record for the #%d %s!!\n", other->client->pers.netname, ent->count, ent->item->pickup_name);
			else
				gi.bprintf (PRINT_HIGH, "%s set the record for the %s!!\n", other->client->pers.netname, ent->item->pickup_name);

		} else {
			if (ent->item->count > 1)
				gi.bprintf (PRINT_HIGH, "%s beat the record time by %.1f secs for the #%d %s!!\n", other->client->pers.netname, (float)(ent->jumpinfo->time - total) / 10.0f, ent->count, ent->item->pickup_name);
			else
				gi.bprintf (PRINT_HIGH, "%s beat the record time by %.1f secs for the %s!!\n", other->client->pers.netname, (float)(ent->jumpinfo->time - total) / 10.0f, ent->item->pickup_name);
		}
		ent->jumpinfo->date = time(0);
		ent->jumpinfo->time = total;
		strncpy (ent->jumpinfo->name, other->client->pers.netname, sizeof(ent->jumpinfo->name)-1);
	}

	other->client->resp.score += 5;
}


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
	self->dmg = 25;
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
	self->dmg = 10;
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
	self->dmg = 50;
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
	self->dmg = 150;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}

//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	if (other->client->pers.jump[ent->s.number])
		return false;

	if (ent->item->flags & IT_JUMPSCORE)
		JumpScore (ent, other);

	other->client->pers.jump[ent->s.number] = true;

	ent->item->use (other, ent->item);

	ent->flags |= FL_RESPAWN;
	return true;
}

qboolean Pickup_Generic (edict_t *ent, edict_t *other)
{
	if (other->client->pers.jump[ent->s.number])
		return false;

	if (ent->item->flags & IT_JUMPSCORE)
		JumpScore (ent, other);

	other->client->pers.jump[ent->s.number] = true;

	ent->flags |= FL_RESPAWN;

	return true;
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
