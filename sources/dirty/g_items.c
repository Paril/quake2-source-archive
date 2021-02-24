#include "g_local.h"

// GRIM - HOLY FUCKING SHIT, ITS THE HIGHLY EDITED G_ITEMS.C!
// NOOOOOOOOOO!!!

// GRIM - All references to IT_STAY_COOP must be delt with

// GRIM
void            RPI_Think (edict_t *ent);
qboolean        Add_Ammo (edict_t *ent, gitem_t *item, int count);
void            Use_Ammo (edict_t *ent, gitem_t *item);
qboolean        Pickup_Health (edict_t *ent, edict_t *other);

void            ExpAmmoDie (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void            ObjectDie (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void            Weapon_Pistol (edict_t *ent);
void            Weapon_Rifle (edict_t *ent);
void            Weapon_Hands (edict_t *ent);

// Dirty
void            Weapon_CanPist (edict_t *ent);
void            Weapon_TwinShotties (edict_t *ent);
void            Weapon_Knife (edict_t *ent);
void            Weapon_Sniper_Rifle (edict_t *ent);
// Dirty

// Two weapons...
void            Weapon_DualPistols (edict_t *ent);
void            Weapon_DualM4s (edict_t *ent);
void            Weapon_DualMachineguns (edict_t *ent);
void            Weapon_DualShotguns (edict_t *ent);
void            Weapon_DualKnives (edict_t *ent);
void            Weapon_DualGrenades (edict_t *ent);
// GRIM

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Grenade (edict_t *ent);

// GRIM
// Dirty
gitem_armor_t jacketarmor_info  = { 10, 100, HIT_CHEST|HIT_BACK|HIT_MID, 0.30};
// Dirty
gitem_armor_t combatarmor_info  = { 25, 100, HIT_HEAD|HIT_CHEST|HIT_MID|HIT_UPPER_LEGS|HIT_UPPER_ARMS, 0.55};
gitem_armor_t bodyarmor_info    = { 40, 100, HIT_HEAD|HIT_CHEST|HIT_MID|HIT_RIGHT_LEG|HIT_LEFT_LEG|HIT_RIGHT_ARM|HIT_LEFT_ARM, 0.80};
// GRIM

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

// GRIM
void RemoveItemWeight (edict_t *ent, gitem_t *item)
{
        if (item->weight > 0)
        {
                ent->weight -= item->weight;
                SetSpeed (ent);
        }
}
// GRIM

//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;
                for (count = 0, ent = master; ent; ent = ent->chain, count++);
		choice = rand() % count;
                for (count = 0, ent = master; count < choice; ent = ent->chain, count++);
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;

        // GRIM
        // Copy corpse_pos as its origin, just in case its moved...
        // DTEMP
        if ((ent->item->flags & IT_AMMO) && (ent->item->flags & IT_SPECIAL))
        {
                //gi.dprintf ("DoRespawn - ammo should be back on ground\n");
                VectorCopy(ent->corpse_pos, ent->s.origin);
        }
        // DTEMP
        ent->flags &= ~FL_RESPAWN;
        ent->kills_in_a_row = 0;

        if (ent->item->flags & IT_RAND)
        {
                RPI_Think (ent);
                return;
        }
        // GRIM

        SetupItems (ent);

	gi.linkentity (ent);

	// send an effect
        // Dirty - Less is more
        //ent->s.event = EV_ITEM_RESPAWN;
        // Dirty
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
        // Dirty - custom respawn time
        if (ent->wait)
                ent->nextthink = level.time + ent->wait;
        else
                ent->nextthink = level.time + delay;
        // Dirty
	ent->think = DoRespawn;
	gi.linkentity (ent);
}

// Paranoid
void SpawnThinker (gitem_t *item, float delay)
{
	edict_t	*ent;
	ent = G_Spawn();

        ent->classname = "thinker";
	ent->item = item;
        ent->deadflag = DEAD_DEAD;
        ent->nextthink = level.time + delay;
        ent->think = RPI_Think;
	gi.linkentity (ent);
}
// Paranoid


//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

// GRIM - Co-op test
//        if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
//                return false;
// GRIM - Co-op test

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
                if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) )
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

// GRIM
void Drop_General (edict_t *ent, gitem_t *item)
{
        Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}
// GRIM

// GRIM
qboolean Pickup_General (edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}
// GRIM


//======================================================================
// Start GRIM Mods
// GRIM MODS FOR ALL HEALTH THINGIES - They no longer just give you health
// They improve your healing rate as well or instead

// GRIM
void DoAdrenaline (edict_t *ent)
{
        float chance, rem;

        // Not being wounded could spell trouble....
        if (!Wounded(ent))
        {
                gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
                
                if (ent->client->adrenaline_framenum > level.framenum)
                {
                        ent->client->adrenaline_framenum += 150;
                }
                else
                {
                        //stuffcmd(ent, "cl_sidespeed 200\n cl_forwardspeed 200\n");
                        ent->client->adrenaline_framenum = level.framenum + 150;
                }
                rem = ent->client->adrenaline_framenum - level.framenum;
                ent->client->pers.heal_rate += 6;

                chance = ((rem + 150) / 750);
                if (random() < chance)
                {
                        ent->health = 0;
                        meansOfDeath = MOD_ADRENALINE;
                        player_die (ent, ent, ent, 100000, vec3_origin);
                }                                        
        }
        else
        {
                BindWounds (ent, 10);
                if (ent->health < (ent->max_health * 0.95))
                        ent->health += 5;
                ent->client->pers.heal_rate += 6;
        }
}

void Use_Adrenaline (edict_t *ent, gitem_t *item)
{
        if (ent->delay_time > level.time)
                return;

        if ((ent->velocity[0]) || (ent->velocity[1]))
        {
                gi.cprintf (ent, PRINT_HIGH, "Hold still to apply\n");
                return;
        }

        ent->delay_time = level.time + 0.8;

        DoAdrenaline(ent);
        ent->client->pers.inventory[ITEM_INDEX(item)]--;
        RemoveAmmoWeight (ent, item, 1);
        ValidateSelectedItem (ent);
}

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
        if ((int)realflags->value & RF_USE_HANDS)
        {
                if (!Add_Ammo (other, ent->item, 1))
                        return false;
        }
        else
                DoAdrenaline(other);

        if (!deathmatch->value)
                other->max_health += 1;
        else if (other->max_health < 100) // Wound damage
                other->max_health += 1;

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
                SetRespawn (ent, 60);

	return true;
}
// GRIM

// GRIM
qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
        other->max_health += 10;

        if (other->health < other->max_health)
                other->health = other->max_health;

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}
// End GRIM Mods

// GRIM
/*
================
Drop_Bandolier
================
*/
void Drop_Bandolier (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
        gitem_t *check;

        check = FindItem("Back Pack");

        if ((!ent->deadflag) && (ent->ammo_carried > 25) && (!(ent->client->pers.inventory[ITEM_INDEX(check)])))
        {
                gi.cprintf (ent, PRINT_HIGH, "Cannot drop : too much ammo!.\n");
                return;
        }
        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        ent->client->pers.inventory[ITEM_INDEX(item)]--;

        if (ent->client->pers.inventory[ITEM_INDEX(check)])
                ent->client->pers.max_ammo = 100;
        else
        // Dirty
                ent->client->pers.max_ammo = 18;
        // Dirty

        // Dirty
        if (!ent->deadflag)
                gi.cprintf (ent, PRINT_HIGH, "Ammo capacity decreased\n");
        // Dirty

        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
        // Dirty
        if (other->client->pers.inventory[ITEM_INDEX(FindItem("Kevlar Vest"))])
        {
                if (level.time - other->last_taken_time > 1)
                        gi.cprintf (other, PRINT_HIGH, "Already wearing a Kevlar Vest!\n");
                return false;
        }
        // Dirty

        if (other->client->pers.max_ammo < 45)
                other->client->pers.max_ammo = 45;
        else
                return false;

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

        if (level.time - other->last_taken_time > 1)
                gi.cprintf (other, PRINT_HIGH, "Ammo capacity increased\n");

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

/*
================
Drop_Pack
================
*/

// GRIM
void Drop_Pack (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
        gitem_t *check;

        check = FindItem("Bandolier");
        if (!ent->deadflag)
        {
                if ((ent->client->pers.inventory[ITEM_INDEX(check)]) && (ent->ammo_carried > 45))
                {
                        gi.cprintf (ent, PRINT_HIGH, "Cannot drop, too much ammo!.\n");
                        return;
                }
                else if (ent->ammo_carried > 25)
                {
                        gi.cprintf (ent, PRINT_HIGH, "Cannot drop, too much ammo!.\n");
                        return;
                }
        }
        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        ent->client->pers.inventory[ITEM_INDEX(item)]--;

        if (ent->client->pers.inventory[ITEM_INDEX(check)])
                ent->client->pers.max_ammo = 45;
        else
        // Dirty
                ent->client->pers.max_ammo = 18;
        // Dirty

        ValidateSelectedItem (ent);
        RemoveItemWeight (ent, item);
        ShowItems(ent);
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
        if (other->client->pers.max_ammo < 100)
                other->client->pers.max_ammo = 100;
        else
                return false;

        if (level.time - other->last_taken_time > 1)
                gi.cprintf (other, PRINT_HIGH, "Ammo increased to max\n");

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        ShowItems(other); // GRIM
	return true;
}
// GRIM

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;

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

// GRIM
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
// GRIM
}

//======================================================================

// GRIM
/*
================
Drop_Breather
================
*/
// GRIM
void Drop_Breather (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

	if (ent->client->breather_framenum > level.framenum)
        {
                gi.cprintf (ent, PRINT_HIGH, "Cannot drop item while in use!\n");
                return;
        }

        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))])
        {
                dropped->count = ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))];
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] = 0;
        }

        ent->client->pers.inventory[ITEM_INDEX(item)] --;
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

qboolean Pickup_Breather (edict_t *ent, edict_t *other)
{
        if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

        if (ent->count > 0)
                other->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] += ent->count;
        else if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
                other->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] += 60;

	if (deathmatch->value)
	{
                if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) )
			SetRespawn (ent, ent->item->quantity);
	}

	return true;
}

void Use_Breather (edict_t *ent, gitem_t *item)
{
        gitem_t *ammo_filter;
        int filter_time;

	if (ent->client->breather_framenum > level.framenum)
        {
                gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
                filter_time = (ent->client->breather_framenum > level.framenum)/10;
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] -= filter_time;
                ent->client->breather_framenum = level.framenum;
        }
        else if (ent->waterlevel >= 3)
        {
                gi.cprintf (ent, PRINT_HIGH, "You are already underwater!\n");
                return;
        }
        else
        {
                ammo_filter = FindItem("Filters");
                if (!(ent->client->pers.inventory[ITEM_INDEX(ammo_filter)]))
                {
                        gi.cprintf (ent, PRINT_HIGH, "All filters used.\n");
                        return;
                }
                filter_time = (ent->client->pers.inventory[ITEM_INDEX(ammo_filter)] * 10);
                ent->client->breather_framenum = level.framenum + filter_time;
        }
        ValidateSelectedItem (ent);
//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}
// GRIM

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;
// GRIM
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
// GRIM
//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

// GRIM
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
// GRIM
	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

// GRIM
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
// GRIM
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

	if (deathmatch->value)
	{
                if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) )
                        SetRespawn (ent, 60);
	}

	return true;
}

//======================================================================
// GRIM
void Drop_Ammo_Specific (edict_t *ent, gitem_t *item, int count)
{
	edict_t	*dropped;
        
        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        dropped->count = count;

        if ((dropped->item->flags & IT_AMMO) && (dropped->item->flags & IT_SPECIAL)
         && (dropped->count > 0) && (dropped->item->quantity > 1))
                dropped->s.frame = (5 - dropped->count);
}

// Dirty - For reloading an empty gun, even if you can't pick up the clip
void ForceAddClip (edict_t *ent, gitem_t *item, int count)
{
        ent->client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] += count;
        ent->client->pers.inventory[ITEM_INDEX(item)] += 1;
        ent->ammo_carried += item->weight;
        ent->weight += item->weight;
        SetSpeed (ent);
}
// Dirty


// Dirty - Just for clips. See if we can put it straight into the gun
qboolean PutStraightIn (edict_t *ent, gitem_t *item, int count)
{
        if (!(item->flags & IT_CLIP))
                return false;

        if ((!ent->client->pers.weapon->ammo) || (ent->client->pers.grabbing == 2))
                return false;

        if (!Q_stricmp(ent->client->pers.weapon->ammo, item->pickup_name))
        {
                if (ent->client->pers.goodhand->quantity < 1)
                {
                        ForceAddClip (ent, item, count);
                        ent->client->weaponstate = WEAPON_RELOADING;
                        return true;
                }
        }

        if (ent->client->pers.weapon_handling && (!Q_stricmp(ent->client->pers.weapon2->ammo, item->pickup_name)))
        {
                if (ent->client->pers.offhand->quantity < 1)
                {
                        ForceAddClip (ent, item, count);
                        ent->client->weaponstate = WEAPON_RELOADING2;
                        return true;
                }
        }

        return false;
}
// Dirty

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
        float   calc, ammo_wt, wt, max, i;
        int     final_count, index;

	if (!ent->client)
		return false;

        index = ITEM_INDEX(item);

        if (!strcmp(item->pickup_name, "Chaingun Pack"))
        {
                if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun Pack"))])
                {
                        if (level.time - ent->last_taken_time > 1)
                                gi.cprintf (ent, PRINT_HIGH, "You already have a Chaingun Pack.\n");
                        return false;
                }
                else
                {
                        ent->client->pers.inventory[index] += count;
                        i = (count / item->weight);
                        ent->weight += i;
                        return true;
                }
        }
        else
                max = ent->client->pers.max_ammo;

        wt = item->weight;

        if (ent->ammo_carried < 0)
                ent->ammo_carried = 0;

        if (ent->ammo_carried >= max)
		return false;

        calc = max - ent->ammo_carried;

        if (item->flags & IT_CLIP)
                ammo_wt = wt;
        else if (item->flags & IT_BIG)
                ammo_wt = wt * count;
        else
                ammo_wt = count / wt;

        if (ammo_wt > calc)
        {
                // Maxxed
                if ((item->flags & IT_CLIP) || (calc <= 0) || ((item->flags & IT_BIG) && item->pickup == Pickup_Health))
                {
                        if (PutStraightIn (ent, item, count))
                                return true;

                        if ((level.time - ent->last_taken_time) > 1)
                                gi.cprintf (ent, PRINT_HIGH, "Unable to carry : %s \n", item->pickup_name);
                        return false;
                }
                else
                {
                        if (item->flags & IT_BIG)
                                final_count = (calc / item->weight);
                        else
                                final_count = (calc * item->weight);
                        i = count - final_count;

                        if (final_count < 1)
                                return false;
                        else if (final_count >= 1)
                        {
                                gi.cprintf (ent, PRINT_HIGH, "Unable to carry : %i %s\n", (int)i, item->pickup_name);
                                Drop_Ammo_Specific (ent, item, i);
                        }
                }
        }
        else
                final_count = count;

        if (item->flags & IT_CLIP)
        {
                if (final_count > 0)
                        ent->client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] += final_count;
                ent->client->pers.inventory[index] += 1;
                i = item->weight;
        }
        else
        {
                ent->client->pers.inventory[index] += final_count;
                if (item->flags & IT_BIG)
                        i = (final_count * item->weight);
                else
                        i = (final_count / item->weight);
        }

        if (strcmp(item->pickup_name, "Chaingun Pack"))
                ent->ammo_carried += i;
        ent->weight += i;
        SetSpeed (ent);

	return true;
}
// GRIM

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
        int     count;

        if ( (ent->item->flags & IT_WEAPON) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
        else if (ent->spawnflags & (DROPPED_PLAYER_ITEM))
        {
                if (ent->count > 0)
                        count = ent->count;
                else
                        count = 0;
        }
        else
		count = ent->item->quantity;

        if (!Add_Ammo (other, ent->item, count))
                return false;

        if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && deathmatch->value)
		SetRespawn (ent, 30);

        // Dirty - Switch to this
        if (ent->item->flags & IT_WEAPON)
        {
                if (other->client->pers.weapon == FindItem("Hands"))
                       other->client->newweapon = ent->item;
        }
        SetAmmoHud (other);
        // Dirty

	return true;
}

void RemoveAmmoWeight (edict_t *ent, gitem_t *item, float count)
{
        float i;

        if ((item->flags & IT_BIG) || (item->flags & IT_CLIP))
                i = (count * item->weight);
        else
                i = (count / item->weight);

        if (strcmp(item->pickup_name, "Chaingun Pack"))
                ent->ammo_carried -= i;

        ent->weight -= i;

        SetSpeed (ent);
        //gi.cprintf(ent, PRINT_HIGH, "RemoveAmmo = %f\n", i);
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
        clips_t *clip;
        int     index, count, calc, sub, clipindex;

	index = ITEM_INDEX(item);

        // Dirty - Fixes unlimited knives/grenades bug?
        if (ent->client->pers.inventory[index] <= 1)
        {
                if ((item->flags & IT_WEAPON) && (ent->client->pers.weapon == item))
                {
                        if (LiveGrenadeCheck (ent))
                                return;
                }
        }
        else if (ent->client->pers.inventory[index] <= 2)
        {
                if ((item->flags & IT_WEAPON) && ent->client->pers.weapon2 && (ent->client->pers.weapon2 == item))
                {
                        if (LiveGrenadeCheck (ent))
                                return;
                }
        }
        // Dirty

        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        if (item->flags & IT_CLIP)
        {
                ent->client->pers.inventory[index]--;
                if (item->quantity > 0)
                {
                        sub = ent->client->pers.inventory[index];

                        clip = FindClip(item->pickup_name);
                        clipindex = CLIP_INDEX(clip);

                        calc = ent->client->pers.clips[clipindex];

                        count = calc - (item->quantity * sub);

                        ent->client->pers.clips[clipindex] -= count;
                }
                else
                        count = 0;

                dropped->count = count;
                RemoveAmmoWeight (ent, item, 1);
                ValidateSelectedItem (ent);
                return;
        }
        else if (ent->client->pers.inventory[index] >= item->quantity)
        {       // Always drop least amount of ammo first
                sub = (ent->client->pers.inventory[index] / item->quantity);
                calc = ent->client->pers.inventory[index];
                count = calc - (sub * item->quantity);
                if (count == 0)
                        count = item->quantity;
                dropped->count = count;
        }
        else
		dropped->count = ent->client->pers.inventory[index];

        /* Dirty
        if ((dropped->item->flags & IT_AMMO) && (dropped->item->flags & IT_SPECIAL)
         && (dropped->count > 0) && (dropped->item->quantity > 1))
                dropped->s.frame = (5 - dropped->count);
        */

	ent->client->pers.inventory[index] -= dropped->count;
        RemoveAmmoWeight (ent, item, dropped->count);
        ValidateSelectedItem (ent);

        // Dirty - Fixes unlimited knives/grenades bug?
        if (ent->client->pers.inventory[index] < 1)
        {
                if ((item->flags & IT_WEAPON) && (ent->client->pers.weapon == item))
                        SetHands (ent);
        }
        else if (ent->client->pers.inventory[index] < 2)
        {
                if ((item->flags & IT_WEAPON) && ent->client->pers.weapon2 && (ent->client->pers.weapon2 == item))
                        ent->client->newweapon = ent->client->pers.weapon;
        }
        // Dirty

        SetAmmoHud (ent);
        // Dirty
}
// GRIM


//======================================================================

// GRIM
void ApplyHealthItem (edict_t *ent, gitem_t *item, gitem_t *itemagain)
{
        edict_t *guy = NULL;

        // On the fly healing on yourself, so less effecient then if
        // you picked it up.  Prevents god like behavior and promotes 
        // teamwork (Here, have this and heal me).
        if (ent->corpse && (ent->corpse->client))
        {
                if (ApplyHealth (ent->corpse, item->quantity + 1))
                { // Not wounded, how did you get this far?
                }
                guy = ent->corpse;
        }
        else
        {
                if (ApplyHealth (ent, item->quantity))
                { // Not wounded, how did you get this far?
                }
                guy = ent;
        }

        ent->client->pers.inventory[ITEM_INDEX(item)]--;
        RemoveAmmoWeight (ent, item, 1);

        if ((ent->client->pers.inventory[ITEM_INDEX(item)] > 0) && (Wounded(guy)))
                ent->delay_time = level.time + (0.2 * item->quantity);
        else
        {
                if (ent->client->pers.inventory[ITEM_INDEX(item)] > 0)
                        gi.centerprintf (ent, "Finished using %s...\n", item->pickup_name);
                else
                        gi.centerprintf (ent, "Out of %s...\n", item->pickup_name);

                ValidateSelectedItem (ent);
                ent->corpse = NULL;
                Cmd_WeapLast_f (ent);
                ValidateSelectedItem (ent);
        }
        //gi.sound(ent, CHAN_ITEM, gi.soundindex(item->pickup_sound), 1, ATTN_NORM, 0);
}

void Use_Health (edict_t *ent, gitem_t *item)
{
        if (ent->deadflag)
                return;

        if (ent->delay_time > level.time)
                return;

        // Dirty
        /*
        if ((!((int)realflags->value & RF_USE_HANDS)) && (!ent->corpse))
        {
                if (ApplyHealth(ent, item->quantity))
                {
                        ent->delay_time = level.time + (0.2 * item->quantity);
                        ent->client->pers.inventory[ITEM_INDEX(item)]--;
                        gi.sound(ent, CHAN_ITEM, gi.soundindex(item->pickup_sound), 1, ATTN_NORM, 0);
                        RemoveAmmoWeight (ent, item, 1);
                        ValidateSelectedItem (ent);
                }
                return;
        }
        */
        // Dirty

        if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
        {
                gi.centerprintf (ent, "Put away your weapon first.\n");
                return;                      
        }

        if (ent->corpse && (ent->corpse->client))
        {
                if (!Wounded(ent->corpse))
                        return;
                gi.centerprintf (ent->corpse, "%s is binding your wounds\n", ent->client->pers.netname);
        }
        else if (!Wounded(ent))
                return;

        gi.centerprintf (ent, "Applying %s...\n", item->pickup_name);

        ent->delay_time = level.time + (0.5 * item->quantity);
        ent->combfunc = ApplyHealthItem;
        ent->combine = item;
        ent->combinetwo = item;
}

// ZOID
void MegaHealth_think (edict_t *self)
{
        if (self->owner->health > self->owner->max_health)
        {
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

        if (!(self->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}
// ZOID

// GRIM
void Drop_Health (edict_t *ent, gitem_t *item)
{
        Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
        RemoveAmmoWeight (ent, item, 1);
        ValidateSelectedItem (ent);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
        qboolean pickmeup = false;

        // Megahealth can't be picked up...
        if (ent->item->flags & IT_SPECIAL)
        {
                if (other->health >= 250)
                        return false;

                if (!ApplyHealth(other, ent->item->quantity))
                        return false;

                ent->think = MegaHealth_think;
                ent->nextthink = level.time + 5;
                ent->owner = other;
                ent->flags |= FL_RESPAWN;
                ent->svflags |= SVF_NOCLIENT;
                ent->solid = SOLID_NOT;

                return true;
        }

        // Dirty
        //if ((int)realflags->value & RF_USE_HANDS)
        //{
                if (!Add_Ammo (other, ent->item, 1))
                        return false;
        /*
        }
        else if (!ApplyHealth(other, ent->item->quantity))
        {
                if (!Add_Ammo (other, ent->item, 1))
                        return false;
        }
        */
        // Dirty

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
                SetRespawn (ent, 30);

	return true;
}
// GRIM

//======================================================================

// GRIM - Armor has been changed a great deal.
void SetArmorHUD (edict_t *ent)
{
        gitem_armor_t   *armor_info;
        char            icon[64];
        int             i;

        if (!ent->client)
                return;

        if (!ent->client->pers.armor)
        {
                ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
                return;
        }
 
        armor_info = (gitem_armor_t *)ent->client->pers.armor->info;

        strcpy(icon, ent->client->pers.armor->icon);
        i = armor_info->max_count - armor_info->min_count;

        if (ent->client->pers.armor_index > (armor_info->max_count - (i*0.25)))
                strcat(icon, "_f");
        else if (ent->client->pers.armor_index > (armor_info->max_count - (i*0.5)))
                strcat(icon, "_g");
        else if (ent->client->pers.armor_index > (armor_info->min_count + (i*0.25)))
                strcat(icon, "_y");
        else
                strcat(icon, "_r");
        ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (icon);
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	gitem_armor_t	*newinfo;
        int             armor_count = 0;

        // Dirty
        if (other->client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))])
        {
                gi.cprintf (other, PRINT_HIGH, "Already wearing a Bandolier!\n");
                return false;
        }
        // Dirty

	newinfo = (gitem_armor_t *)ent->item->info;

        if ((other->client->pers.carried_armor) && ((int)realflags->value & RF_USE_HANDS))
        {
                gi.centerprintf (other, "You are already carrying %s\n", other->client->pers.carried_armor->pickup_name);
                return false;
        }

        if (ent->spawnflags & (DROPPED_PLAYER_ITEM))
                armor_count = ent->count;
        else        
                armor_count = newinfo->max_count;

        if ((int)realflags->value & RF_USE_HANDS)
        {
                other->client->pers.carried_armor_index = armor_count;
                other->client->pers.carried_armor = ent->item;
        }
        else if (other->client->pers.armor)
                return false;
        else
        {
                other->client->pers.armor_index = armor_count;
                other->client->pers.armor = ent->item;
                SetArmorHUD (other);
        }

        if ((!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) && (deathmatch->value))
                SetRespawn (ent, 30);

        other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

        // Dirty
        gi.sound(other, CHAN_ITEM, gi.soundindex("misc/veston.wav"), 1, ATTN_NORM, 0);
        // Dirty

	return true;
}

// handle armor shards specially
// GRIM - Yeah, you can't use em without armor!
// They ownly repair old armor.
qboolean Pickup_Shard (edict_t *ent, edict_t *other)
{
        if (!other->client->pers.armor)
                return false;

        other->client->pers.armor_index += 2;

        if (other->client->pers.armor_index > other->client->pers.armor->quantity)
                other->client->pers.armor_index = other->client->pers.armor->quantity;

        if  ( (!(ent->spawnflags & DROPPED_PLAYER_ITEM)) && (deathmatch->value) )
                SetRespawn (ent, 15);

        SetArmorHUD (other);

	return true;
}

// GRIM - New Drop armor bit
/*
================
Drop_Armor
================
*/
void Drop_Armor (edict_t *ent, gitem_t *item);
void RemoveArmor (edict_t *ent, gitem_t *item, gitem_t *itemagain)
{
        gi.centerprintf (ent, "Finished taking off %s.\n", item->pickup_name);
        Drop_Armor (ent, item);
}

void Drop_Armor (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
        gitem_t *armor;
        int     count = 0;

        armor = item;

        if (ent->delay_time > level.time)
                return;
        else if (ent->client->pers.carried_armor && (item == ent->client->pers.carried_armor))
        {
                count = ent->client->pers.carried_armor_index;
                armor = ent->client->pers.carried_armor;
        }
        else if (((int)realflags->value & RF_USE_HANDS) && (!ent->combfunc) && (!ent->deadflag))
        {
                if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
                {
                        gi.centerprintf (ent, "Put away your weapon first.\n");
                        return;                      
                }
                gi.centerprintf (ent, "Taking off %s.\n", item->pickup_name);
                ent->delay_time = level.time + (item->weight/10);
                ent->combfunc = RemoveArmor;
                ent->combine = item;
                ent->combinetwo = item;
                return;
        }

        dropped = Drop_Item (ent, armor, ITEM_UNDEF);

        if (count > 0)
        {
                dropped->count = count;
                ent->client->pers.carried_armor_index = 0;
                ent->client->pers.inventory[ITEM_INDEX(ent->client->pers.carried_armor)]--;
                ent->client->pers.carried_armor = NULL;
        }
        else if (ent->client->pers.armor)
        {
                dropped->count = ent->client->pers.armor_index;
                ent->client->pers.armor_index = 0;
                ent->client->pers.inventory[ITEM_INDEX(ent->client->pers.armor)]--;
                ent->client->pers.armor = NULL;
                ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
                ent->combfunc = NULL;
                ent->combine = NULL;
                ent->combinetwo = NULL;
                SetArmorHUD (ent);
        }
        else
                return;

        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

void Drop_ArmorQuick (edict_t *ent)
{
        if (ent->client->pers.armor)
                Drop_Armor(ent, ent->client->pers.armor);
        else
                return;                        
}

/*
================
Use_Armor - For those who love that realism...
================
*/
void Use_Armor (edict_t *ent, gitem_t *item);
void ApplyArmor (edict_t *ent, gitem_t *item, gitem_t *itemagain)
{
        Use_Armor (ent, item);
}

void Use_Armor (edict_t *ent, gitem_t *item)
{
        if (ent->client->pers.carried_armor == NULL)
                return; // The item chosen is not the armor being carried.

        if (item != ent->client->pers.carried_armor)
                return;

        if (ent->client->pers.armor)
        {
                gi.centerprintf (ent, "Take off your other armor first...\n");
                return;                      
        }

        if (ent->delay_time > level.time)
                return;

        if (((int)realflags->value & RF_USE_HANDS) && (!ent->combfunc))
        {
                if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
                {
                        gi.centerprintf (ent, "Put away your weapon first.\n");
                        return;                      
                }
                gi.centerprintf (ent, "Putting on %s...\n", item->pickup_name);
                ent->delay_time = level.time + (item->weight/15);
                ent->combfunc = ApplyArmor;
                ent->combine = item;
                ent->combinetwo = item;
                return;
        }

        ent->client->pers.armor_index = ent->client->pers.carried_armor_index;
        ent->client->pers.armor = ent->client->pers.carried_armor;
        ent->client->pers.carried_armor_index = 0;
        ent->client->pers.carried_armor = NULL;
        ent->combfunc = NULL;
        ent->combine = NULL;
        ent->combinetwo = NULL;

        SetArmorHUD (ent);

        gi.centerprintf (ent, "Now wearing %s.\n", item->pickup_name);
}
// GRIM

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
                if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) )
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
// GRIM
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;
        qboolean        last_item_set = false;

        if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
        {
                if (gi.pointcontents(ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
                {
                        ent->nextthink = level.time + 0.1;
                        return;
                }
        }
        // GRIM

	if (!other->client)
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

        // GRIM
        if (ent->item && (!ent->item->pickup))
		return;		// not a grabbable item?
        // GRIM 

        // May not pick up items during pre start...
        if (level.match_state == MATCH_START)
                return;

        // Can't pickup while doing something with a weapon, must be idle
        if (other->client->weaponstate != WEAPON_READY)
                return;         

        if (other->client->pers.grabbing < 1)
                return;

        // Dirty
        if (other->is_a_lame_mofo)
        {
                if (level.time - other->last_taken_time > 1)
                {
                        gi.centerprintf (other, "TeamKill limit passed! - No items for you!");
                        other->last_taken_time = level.time;
                }
                return;
        }
        // Dirty

        // No picking up stuff not infront of us - GRIM
        if (other->velocity[0] || other->velocity[1])
        {
                if (!(point_infront (other, ent->s.origin)))
                        return;            
        }

        // Booby trapped items explode instead of being picked up
        if (ent->trapping)
        {
                if (ent->trapping->die)
                {
                        ent->trapping->enemy = ent->trapping->master;
                        ent->trapping->die (ent->trapping, ent->trapping, ent->trapping, 100000, vec3_origin);
                }
                return;
        }

	taken = ent->item->pickup(ent, other);
        other->last_taken_time = level.time; // Not true, but don't matter

	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

                if ( (ent->item->flags & IT_WEAPON) && (!(ent->item->flags & IT_AMMO)) )
                        last_item_set = true;
                else if ((other->last_taken->flags & IT_WEAPON) && (level.time - other->last_taken_time > 2))
                        last_item_set = true;
                else if (level.time - other->last_taken_time > 1)
                        last_item_set = true;

                // Dirty
                if ((last_item_set == true) && (other->client->pers.grabbing != 2))
                // Dirty
                {
                        other->last_taken = ent->item;
                        if (ent->item->use)
                                other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
                }

                // Off permanent?
                //gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);

                if ((!(ent->item->flags & IT_AMMO)) && (ent->item->weight > 0))
                {
                        other->weight += ent->item->weight;
                        SetSpeed (other);
                }
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

        ValidateSelectedItem (other);

        if (ent->item->flags & IT_RAND)
                G_FreeEdict (ent);
        else if (ent->flags & FL_RESPAWN)
                ent->flags &= ~FL_RESPAWN;
        else
                G_FreeEdict (ent);
}
// GRIM

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        // GRIM
        if (gi.pointcontents(ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
        {
                ent->nextthink = level.time + 0.1;
                return;
        }
        // GRIM

        if (other == ent->owner)
		return;

        Touch_Item (ent, other, plane, surf);
}

// Dirty
/*
==================
GetDroppedItem
==================
*/
edict_t *GetDroppedItem (int classnum)
{
        edict_t *ent;
        int     i;

        for (i=0 ; i<globals.num_edicts ; i++)
        {
                ent = &g_edicts[i];

                if (!ent->item)
                        continue;
                if (!(ent->spawnflags & DROPPED_PLAYER_ITEM))
                        continue;
                if (!ent->points)
                        continue;
                if (ent->points == classnum)
                        return ent;
        }

        return NULL;
}
// Dirty


static void drop_make_touchable (edict_t *ent)
{
        edict_t *hippy_smurf = NULL; // Dirty

	ent->touch = Touch_Item;

        ent->owner = NULL;

	if (deathmatch->value)
	{
                // Dirty
                // Check limit here so we can't have shitloads in the air as well...
                if ((int)max_items->value > 0)
                {
                        items_number++;
                        if (items_number > (int)max_items->value)
                                items_number = 1;
                        hippy_smurf = GetDroppedItem (items_number);
                        if (hippy_smurf)
                                hippy_smurf->nextthink = level.time + 0.1;

                        ent->points = items_number; // Smart limit thing
                }
                // Dirty
                
                // GRIM        
                if (ent->item->flags & IT_RAND)
                {
                        ent->nextthink = level.time + (60 - 1);
                        ent->think = RPI_Think;
                }
                else if (ent->item->pickup == Pickup_Weapon)
                {
                        ent->nextthink = level.time + 59;
                        ent->think = WeaponsStayThink;
                }
                else
                {
                        ent->nextthink = level.time + 59;
                        ent->think = G_FreeEdict;
                }
                // GRIM
	}
}


// GRIM
void TrapThink (edict_t *ent)
{
        //gi.dprintf ("GT gone.\n");
        if (ent->die)
                ent->die (ent, ent, ent, 100000, vec3_origin);          
        G_FreeEdict (ent);
}
// GRIM

// GRIM
/*
===============
BOOBY TRAP
===============
*/
void GrenadeTrap (edict_t *ent, gitem_t *item)
{
        edict_t *grentrap;
	vec3_t	forward, right;
        vec3_t  offset;
        trace_t tr;

        grentrap = G_Spawn();

        grentrap->classname = "GrenadeTrap";

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        VectorSet(offset, 24, 0, -16);
        G_ProjectSource (ent->s.origin, offset, forward, right, grentrap->s.origin);
        tr = gi.trace (ent->s.origin, grentrap->mins, grentrap->maxs, grentrap->s.origin, ent, CONTENTS_SOLID);
        VectorCopy (tr.endpos, grentrap->s.origin);
        VectorCopy (ent->s.angles, grentrap->s.angles);
        grentrap->s.angles[0] = 0;

        grentrap->solid = SOLID_BBOX;
        grentrap->movetype = MOVETYPE_NONE;
        grentrap->takedamage = DAMAGE_YES;
        grentrap->svflags |= SVF_DEADMONSTER;

        grentrap->modtype = MOD_BOOBYTRAP;
        grentrap->health = 10;
        grentrap->die = ExpAmmoDie;

        grentrap->master = ent;

        VectorSet (grentrap->mins, -15, -15, -15);
        VectorSet (grentrap->maxs, 15, 15, -7);
        grentrap->s.modelindex = gi.modelindex ("models/items/ammo/grenades/hand/tris.md2");

        grentrap->nextthink = level.time + 149;
        grentrap->think = TrapThink;
        grentrap->spawnflags |= DROPPED_PLAYER_ITEM;

        gi.linkentity (grentrap);

        if (ent->trapping->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                ent->trapping->nextthink = grentrap->nextthink + 0.1;

        if (ent->trapping->trapping)
                ent->trapping->trapping->trapping = NULL;

        ent->trapping->trapping = grentrap;
        grentrap->trapping = ent->trapping;
        ent->trapping->kills_in_a_row += 1;
        
        ent->trapping = NULL;

        ent->client->pers.inventory[ITEM_INDEX(item)]--;
        ent->weight -= item->weight;
        ent->ammo_carried -= item->weight;
	ValidateSelectedItem (ent);
}
// GRIM


// GRIM - Added type to handle different placment styles...
edict_t *Drop_Item (edict_t *ent, gitem_t *item, int type)
{
	edict_t	*dropped;
	vec3_t	forward, right;
        vec3_t  offset, start; // Dirty
        trace_t trace;
        int     act;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;

        if (ent->client)
                dropped->spawnflags = DROPPED_PLAYER_ITEM;
        else                
                dropped->spawnflags = DROPPED_ITEM;

        dropped->s.effects = item->world_model_flags;

        VectorSet (dropped->mins, -15, -15, -15);
        VectorSet (dropped->maxs, 15, 15, 15);

        // Dirty
        act = type;

        if (ent->client)
        {
                // Dirty
                if (ent->client->pers.grabbing != 2)
                        dropped->owner = ent;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
                VectorCopy(ent->s.origin, start);
                start[2] += 16;
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
                trace = gi.trace (start, dropped->mins, dropped->maxs, dropped->s.origin, ent, CONTENTS_SOLID);
                // Dirty
		VectorCopy (trace.endpos, dropped->s.origin);

                VectorCopy (ent->s.angles, dropped->s.angles);

                dropped->s.angles[0] = 0;
                //dropped->s.angles[1] += 90;

                if ((type == ITEM_UNDEF) && (ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                        act = ITEM_PLACED;
	}
	else
	{
                dropped->owner = ent;
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

        if ((act == ITEM_UNDEF) || (act == ITEM_THROWN))
        {
                VectorScale (forward, 150, dropped->velocity);
                dropped->velocity[2] = 300;
        }
        else if (act == ITEM_DROPPED)
        {
                VectorSet (dropped->avelocity, 300, 300, 300);
                VectorScale (forward, 50, dropped->velocity);
                dropped->velocity[2] = 25;
                dropped->movetype = MOVETYPE_BOUNCE;
        }
        else if (act == ITEM_PLACED)
                VectorScale (forward, 5, dropped->velocity);

        // Dirty
        dropped->think = drop_make_touchable;
        if (ent->client->pers.grabbing == 2)
                dropped->nextthink = level.time + 0.1;
        else
                dropped->nextthink = level.time + 1;

        if (ent->health < 1)
        {
                VectorSet (dropped->avelocity, 0, 1000, 0);
                VectorAdd (dropped->velocity, ent->velocity, dropped->velocity);
        }
        // Dirty

        // Make sure we have a model before setting one...
        if (dropped->item->world_model)
        {
                gi.setmodel (dropped, dropped->item->world_model);
                dropped->solid = SOLID_TRIGGER;
                dropped->movetype = MOVETYPE_TOSS;
                dropped->touch = drop_temp_touch;
        }
        else 
        {       // Item not suitable to be dropped...
                dropped->s.modelindex = gi.modelindex ("models/weapons/g_dual/tris.md2");
                dropped->s.frame = 0;
                dropped->svflags |= SVF_NOCLIENT;

                dropped->movetype = MOVETYPE_NONE;
                dropped->solid = SOLID_NOT;
                dropped->touch = NULL;

                dropped->nextthink = level.time + 1;
                dropped->think = G_FreeEdict;
        }
        // Dirty

        SetupItems (dropped);

	gi.linkentity (dropped);

	return dropped;
}
// GRIM

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

        // GRIM
        SetupItems (ent);
        // GRIM

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

        // BraZen temp
        //gi.dprintf ("droptofloor: %s, postition at %s\n", ent->classname, vtos(ent->s.origin));
        // BraZen temp

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

        VectorCopy(ent->s.origin, ent->corpse_pos); // Dirty

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
// GRIM - Altered things...
void SpawnItem (edict_t *ent, gitem_t *item)
{
	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

        // Fuck em' off permantly
        // Dirty
        if ((item->pickup == Pickup_Shard) || (item->pickup == Pickup_Pack)
         || (item->pickup == Pickup_PowerArmor) || (item->pickup == Pickup_Powerup)
          || (item->pickup == Pickup_Adrenaline) || (item->pickup == Pickup_AncientHead)
           || (item->pickup == Pickup_Armor) || (item->pickup == Pickup_Health))
        {
                G_FreeEdict (ent);
                return;
        }

        if ((int)realflags->value & RF_NO_EQUIPMENT)
        {
                if ((item->pickup != CTFPickup_Flag) && (item->pickup != Pickup_Key))
                        G_FreeEdict (ent);
                return;
        }
        // Dirty

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
                if (((int)dmflags->value & DF_INFINITE_AMMO) && ((item->flags == IT_AMMO) || (!strcmp(ent->classname, "weapon_bfg"))))
		{
                        G_FreeEdict (ent);
                        return;
		}
	}

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

        // Don't spawn the flags unless enabled
        if ((item->pickup == CTFPickup_Flag) && (!ctf->value))
        {
		G_FreeEdict(ent);
        	return;
	}

        // Dirty
        if (!strcmp(ent->classname, "weapon_hyperblaster"))
        {
		G_FreeEdict(ent);
        	return;
	}

        if (!strcmp(ent->classname, "weapon_supershotgun"))
        {
		G_FreeEdict(ent);
        	return;
	}

        if (!strcmp(ent->classname, "weapon_railgun"))
        {
                G_FreeEdict (ent);
                return;
        }

        if (!strcmp(ent->classname, "weapon_grenadelauncher"))
        {
                G_FreeEdict (ent);
                return;
        }
        // Dirty

        // Dirty
        if (!strcmp(ent->classname, "ammo_bullets"))
                ent->item = FindItem("Pistol Clip");
        else if (!strcmp(ent->classname, "weapon_machinegun"))
                ent->item = FindItem("MP5/10 Submachinegun");
        else if (!strcmp(ent->classname, "weapon_rocketlauncher"))
                ent->item = FindItem("Handcannon");
        else if (!strcmp(ent->classname, "weapon_chaingun"))
                ent->item = FindItem("Sniper Rifle");
        else if (!strcmp(ent->classname, "ammo_rockets"))
                ent->item = FindItem("Machinegun Magazine");
        else if (!strcmp(ent->classname, "weapon_bfg"))
                ent->item = FindItem("M4 Assault Rifle");
        else if (!strcmp(ent->classname, "ammo_cells"))
                ent->item = FindItem("M4 Clip");
        else if (!strcmp(ent->classname, "ammo_slugs"))
                ent->item = FindItem("Sniper Rifle Ammo");
        // Dirty
        else if (item->pickup == Pickup_PowerArmor) // Deamed too powerful
                ent->item = FindItem("Bandolier");
        else if (!strcmp(ent->classname, "item_silencer"))
                ent->item = FindItem("Flash Light");
        else
                ent->item = item;

        // Dirty
        ent->classname = ent->item->classname;

        if (sv_weaponchoices->string && sv_weaponchoices->string[0] && (!strstr (sv_weaponchoices->string, "all")))
        {
                gitem_t *thing = NULL;

                if ((ent->item->pickup == Pickup_Weapon) || (!strcmp(ent->classname, "ammo_grenades")))
                        thing = ent->item;
                else if (!strcmp(ent->classname, "ammo_shells"))
                {
                        if (!strstr (sv_weaponchoices->string, "weapon_shotgun"))
                        {
                                if (!strstr (sv_weaponchoices->string, "weapon_cannon"))
                                {
                                        G_FreeEdict (ent);
                                        return;
                                }
                        }
                }
                else if ((ent->item->flags & IT_AMMO) && (!(ent->item->flags & IT_WEAPON)))
                {
                        if (ent->item->weapon)
                                thing = FindItem (ent->item->weapon);
                }

                if (thing)
                {
                        if (!strstr (sv_weaponchoices->string, thing->classname))
                        {
                                G_FreeEdict (ent);
                                return;
                        }
                }
        }

        // Moved from top of SpawnItem
        PrecacheItem (ent->item);
        // Dirty

	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;

        // Only add glow if we want it...
        if (!((int)realflags->value & RF_NO_GLOWING))
                ent->s.renderfx = RF_GLOW;

	if (ent->model)
		gi.modelindex (ent->model);

        // flags are server animated and have special handling
        if (item->pickup == CTFPickup_Flag)
		ent->think = CTFFlagSetup;
}
// GRIM

//======================================================================
// GRIM
// This entire area has been so fucked with,
// it will need to be gone through line by line.
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
                Use_Armor,
                Drop_Armor,
		NULL,
		"misc/ar1_pkup.wav",
                "models/items/armor/fullbody/tris.md2", 0,
                // GRIM - OLD "models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */    "Full Body Armor",
/* width */		3,
                100,
		NULL,
		NULL,
		NULL,
		NULL,
		IT_ARMOR,
                130,
		0,
		&bodyarmor_info,
/* precache */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
                Use_Armor,
                Drop_Armor,
		NULL,
		"misc/ar1_pkup.wav",
                "models/items/armor/body/tris.md2", 0,
                // GRIM - OLD "models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
                100,
		NULL,
		NULL,
		NULL,
		NULL,
		IT_ARMOR,
                35,
		0,
		&combatarmor_info,
/* precache */ ""
	},

// Dirty
/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
                Use_Armor,
                Drop_Armor,
		NULL,
		"misc/ar1_pkup.wav",
                "models/items/armor/jacket/tris.md2", 0,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */    "Kevlar Vest",
/* width */		3,
                100,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_ARMOR|IT_SPECIAL,
                15,
		0,
		&jacketarmor_info,
/* precache */ ""
	},
// Dirty


/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard", 
                Pickup_Shard,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
                "models/items/armor/shard/tris.md2", 0,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_ARMOR,
                0,
		0,
		NULL,
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
                "models/items/armor/screen/tris.md2", 0,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
		IT_ARMOR,
                0,
		0,
		NULL,
/* precache */ ""
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
                "models/items/armor/shield/tris.md2", 0,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
		IT_ARMOR,
		0,
                0,
		NULL,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

// GRIM
// GRIM
/*HANDS*/
	{
                "nothing", 
                NULL,
                NULL,
                NULL,
                Weapon_Hands,
                NULL,
                NULL, 0,
                NULL,
/* icon */      "i_fixme",
/* pickup */    "Hands",
                0,
                1,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_WEAPON|IT_MELEE,
                0,
                0,
		NULL,
/* precache */ "weapons/kick1.wav weapons/kick2.wav weapons/phitf1.wav weapons/phitf2.wav weapons/phitw1.wav weapons/phitw2.wav"
	},

// Dirty
	{
                "weapon_knife", 
                Pickup_Ammo,
		Use_Weapon,
                Drop_Ammo,
                Weapon_Knife,
		"misc/w_pkup.wav",
                "models/objects/knife/tris.md2", 0,
                "models/weapons/v_knife/tris.md2",
/* icon */      "w_knife",
/* pickup */    "Combat Knife",
                0,
                1,
                "Combat Knife",
                NULL,
                NULL,
		NULL,
                IT_WEAPON|IT_MELEE|IT_BIG|IT_AMMO|IT_TWIN,
                1,
                WEAP_KNIFE,
		NULL,
/* precache */ ""
	},

/* weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
now this is a pistol, not blaster.
*/
	{
                "weapon_pistol", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Pistol,
		"misc/w_pkup.wav",
                "models/weapons/g_dual/tris.md2", 0,
                "models/weapons/v_pistol/tris.md2",
/* icon */      "w_mk23",
/* pickup */    "MK23 Pistol",
                0,
                12,
                "Pistol Clip",
                NULL,
                NULL,
		NULL,
                IT_WEAPON|IT_TWIN|IT_RELOAD|IT_CLIP,
                6,
                WEAP_MK23,
		NULL,
/* precache */ "weapons/mk23fire.wav weapons/mk23in.wav weapons/mk23out.wav weapons/mk23slap.wav weapons/mk23slide.wav"
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
                "models/weapons/g_shotg/tris.md2", 0,
		"models/weapons/v_shotg/tris.md2",
/* icon */      "w_super90",
/* pickup */    "M3 Super 90 Assault Shotgun",
                0,
                7,
                "12 Gauge Shells",
                NULL,
                NULL,
		NULL,
                IT_WEAPON|IT_RELOAD|IT_BIG,
                35,
                WEAP_SUPER90,
		NULL,
/* precache */ "weapons/shotgf1b.wav weapons/m3in.wav weapons/shotgcock.wav"
	},

/*QUAKED weapon_cannon (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_cannon", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_SuperShotgun,
		"misc/w_pkup.wav",
                "models/weapons/g_cannon/tris.md2", 0,
                "models/weapons/v_cannon/tris.md2",
/* icon */      "w_cannon",
/* pickup */    "Handcannon",
                0,
                2,
                "12 Gauge Shells",
		NULL,
		NULL,
		NULL,
                IT_WEAPON|IT_TWIN|IT_RELOAD|IT_BIG,
                15,
                WEAP_CANNON,
		NULL,
/* precache */ "weapons/cannon_fire.wav weapons/copen.wav weapons/cin.wav weapons/cout.wav weapons/cclose.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_supershotgun", 
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},


/*QUAKED weapon_mp5 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_mp5", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
                "models/weapons/g_machn/tris.md2", 0,
                "models/weapons/v_machn/tris.md2",
/* icon */      "w_mp5",
/* pickup */    "MP5/10 Submachinegun",
                0,
                30,
                "Machinegun Magazine",
                NULL,
                NULL,
		NULL,
                IT_WEAPON|IT_TWIN|IT_RELOAD|IT_CLIP|IT_BIG,
                30,
                WEAP_MP5,
		NULL,
/* precache */ "weapons/mp5fire1.wav weapons/mp5slide.wav weapons/mp5in.wav weapons/mp5out.wav"
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

// Dirty

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},
// Dirty
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
/* icon */      "a_m61frag",
/* pickup */    "M26 Fragmentation Grenade",
/* width */		3,
                1,
                "m26 fragmentation grenade",
		NULL,
		NULL,
		NULL,
                IT_AMMO|IT_WEAPON|IT_BIG|IT_SPECIAL|IT_TWIN,
                3,
                WEAP_M61FRAG,
		NULL,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},
// Dirty

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

// Dirty
/*QUAKED weapon_rifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_rifle", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Rifle,
		"misc/w_pkup.wav",
                "models/weapons/g_m4/tris.md2", 0,
                "models/weapons/v_m4/tris.md2",
/* icon */      "w_m4",
/* pickup */    "M4 Assault Rifle",
                0,
                24,
                "M4 Clip",
                "M4 Assault Rifle",
                NULL,
		NULL,
                IT_WEAPON|IT_TWIN|IT_RELOAD|IT_BIG|IT_CLIP,
                45,
                WEAP_M4,
		NULL,
/* precache */ "weapons/m4a1fire.wav weapons/m4a1in.wav weapons/m4a1out.wav weapons/m4a1slide.wav"
	},

/*QUAKED weapon_sniper_rifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "weapon_sniper", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
                Weapon_Sniper_Rifle,
		"misc/w_pkup.wav",
                "models/weapons/g_sniper/tris.md2", 0,
                "models/weapons/v_sniper/tris.md2",
/* icon */      "w_sniper",
/* pickup */    "Sniper Rifle",
                0,
                6,
                "Sniper Rifle Ammo",
                NULL,
                NULL,
		NULL,
                IT_WEAPON|IT_RELOAD|IT_BIG,
                50,
                WEAP_SNIPER,
		NULL,
/* precache */ "weapons/ssgfire.wav weapons/ssgin.wav weapons/ssgbolt.wav"
	},
// Dirty

        {       // Just here so that it can be found and swapped.
                "weapon_hyperblaster", 
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},


	//
	// AMMO ITEMS
	//
// Dirty
/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */    "12 Gauge Shells",
/* width */     3,
                7,
                "12 Gauge Shells",
		NULL,
		NULL,
                NULL,
		IT_AMMO,
                3.5,
                0,
		NULL,
/* precache */ ""
	},
// Dirty

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
                Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
                45,
                "Bullets",
		NULL,
		NULL,
		NULL,
		IT_AMMO,
                18,
                0,
		NULL,
/* precache */ ""
	},

// Dirty
/*QUAKED ammo_sniper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "ammo_sniper",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/sniper/tris.md2", 0,
		NULL,
/* icon */      "a_bullets",
/* pickup */    "Sniper Rifle Ammo",
/* width */		3,
		10,
                "Sniper Rifle Ammo",
		NULL,
		NULL,
		NULL,
                IT_AMMO,
                10,
                0,
		NULL,
/* precache */ ""
	},
// Dirty

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
                NULL,
                NULL,
                "",
                "", 0,
                "",
/* icon */      "",
/* pickup */    "",
		0,
                0,
		NULL,
		NULL,
		NULL,
		NULL,
                0,
                0,
                0,
		NULL,
/* precache */ ""
	},

// Dirty
/*
==============
Pistol Clips
==============
*/
	{
                "clip_pistol_empty",
                NULL, // Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/clip/tris.md2", 0,
		NULL,
/* icon */      "a_clip",
/* Pickup */    "Empty Pistol Clip",
/* width */     3,
                0,
                "Empty Pistol Clip",
		NULL,
                "Empty Pistol Clip",
                "MK23 Pistol",
                IT_AMMO|IT_CLIP,
                2,
                0,
		NULL,
/* precache */ ""
	},

	{
                "ammo_clip",
                Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/clip/tris.md2", 0,
		NULL,
/* icon */      "a_clip",
/* Pickup */    "Pistol Clip",
/* width */     3,
                12,
                "Bullets",
                "Bullets",
                "Empty Pistol Clip",
                "MK23 Pistol",
                IT_AMMO|IT_CLIP,
                2,
                0,
		NULL,
/* precache */ ""
	},

/*
==============
Machinegun Clips
==============
*/
	{
                "clip_machinegun_empty",
                NULL, // Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/mag/tris.md2", 0,
		NULL,
/* icon */      "a_mag",
/* Pickup */    "Empty Machinegun Magazine",
/* width */     3,
                0,
                "Empty Machinegun Magazine",
                NULL,
                "Empty Machinegun Magazine",
                "MP5/10 Submachinegun",
                IT_AMMO|IT_CLIP,
                4,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED ammo_mag (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "ammo_mag",
		Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/mag/tris.md2", 0,
		NULL,
/* icon */      "a_mag",
/* Pickup */    "Machinegun Magazine",
/* width */     3,
                30,
                "Bullets",
                "Bullets",
                "Empty Machinegun Magazine",
                "MP5/10 Submachinegun",
                IT_AMMO|IT_CLIP,
                4,
                0,
		NULL,
/* precache */ ""
	},


/*
==============
Rifle Clip
==============
*/
	{
                "clip_rifle_empty",
                NULL, // Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/m4/tris.md2", 0,
		NULL,
/* pickup */    "M4 Clip",
/* pickup */    "Empty M4 Clip",
/* width */     3,
                0,
                "Empty M4 Clip",
		NULL,
                "Empty M4 Clip",
		NULL,
                IT_AMMO|IT_CLIP,
                5.7,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED clip_m4rifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "clip_rifle",
		Pickup_Ammo,
                NULL, // Dirty Use_Ammo,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/m4/tris.md2", 0,
		NULL,
/* icon */      "a_m4",
/* pickup */    "M4 Clip",
/* width */		3,
                24,
                "Bullets",
                "Bullets",
                "Empty M4 Clip",
                "M4 Assault Rifle",
                IT_AMMO|IT_CLIP,
                6,
                0,
		NULL,
/* precache */ ""
	},


// GRIM - DUAL WEAPONS OR BUST

/*QUAKED weapon_dualpistols_which_can_be_put_on_maps (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		NULL,
                NULL,
		NULL,
                NULL,
                Weapon_DualPistols,
		NULL,
                NULL, 0,
                "models/weapons/v_2pistols/tris.md2",
/* icon */              "w_blaster",
/* pickup */    "Dual Pistols",
		0,
		1,
                "Pistol Clip",
                "MK23 Pistol",
                "MK23 Pistol",
                "MK23 Pistol",
                IT_WEAPON|IT_TWIN,
                0,
                WEAP_AKIMBO,
		NULL,
/* precache */ ""
	},

/*
DUAL HANDCANNONS - Holds the model.  That's about it.
*/
	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_TwinShotties,
		NULL,
                NULL, 0,
                "models/weapons/v_2cannons/tris.md2",
/* icon */      "w_machinegun",
/* pickup */    "Dual Handcannons",
		0,
		1,
                "12 Gauge Shells",
                "Handcannon",
                "Handcannon",
                "Handcannon",
                IT_WEAPON|IT_TWIN,
                0,
                WEAP_CANNON,
		NULL,
/* precache */ ""
	},

/*
DUAL MP5's - Holds the model.  That's about it.
*/
	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_DualMachineguns,
		NULL,
                NULL, 0,
                "models/weapons/v_2machn/tris.md2",
/* icon */      "w_mp5",
/* pickup */    "MP5/10 Submachinegun",
                0,
                1,
                "Machinegun Magazine",
                "MP5/10 Submachinegun",
                "MP5/10 Submachinegun",
                "MP5/10 Submachinegun",
                IT_WEAPON|IT_TWIN,
                0,
                WEAP_MP5,
		NULL,
/* precache */ ""
	},

/*QUAKED twin m4's (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_DualM4s,
		NULL,
                NULL, 0,
                "models/weapons/v_2m4s/tris.md2",
/* icon */      "w_mp5",
/* pickup */    "M4 Assault Rifle",
                0,
                1,
                "M4 Clip",
                "M4 Assault Rifle",
                "M4 Assault Rifle",
                "M4 Assault Rifle",
                IT_WEAPON|IT_TWIN,
                0,
                WEAP_M4,
		NULL,
/* precache */ ""
	},


/*
SHOTTIE PISTOL - Holds the model.  That's about it.
*/
	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_CanPist,
		NULL,
                NULL, 0,
                "models/weapons/v_canpist/tris.md2",
/* icon */      "w_machinegun",
/* pickup */    "Handcannon Pistol Combo",
		0,
		1,
                "12 Gauge Shells",
                NULL, // Temp disable....
                NULL,
                NULL,
                //"Handcannon",
                //"MK23 Pistol",
                //"Handcannon",
                IT_WEAPON,
                WEAP_CANNON,
                0,
		NULL,
/* precache */ ""
	},

	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_DualShotguns,
		NULL,
                NULL, 0,
                "models/weapons/v_2shotg/tris.md2",
/* icon */      "w_super90",
/* pickup */    "Dual M3 Super 90 Assault Shotgun",
                0,
                7,
                "12 Gauge Shells",
                NULL, // PERMANENT disable! They sucked ass!
                NULL,
                NULL,
                //"M3 Super 90 Assault Shotgun",
                //"M3 Super 90 Assault Shotgun",
                //"M3 Super 90 Assault Shotgun",
                IT_WEAPON|IT_TWIN,
                0,
                WEAP_SUPER90,
		NULL,
/* precache */ ""
	},

// Dirty
	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_DualKnives,
		NULL,
                NULL, 0,
                "models/weapons/v_2knife/tris.md2",
/* icon */      "w_knife",
/* pickup */    "Duals Combat Knives",
                0,
                1,
                "Combat Knife",
                "Combat Knife",
                "Combat Knife",
                "Combat Knife",
                IT_WEAPON|IT_MELEE|IT_AMMO|IT_TWIN,
                0,
                WEAP_KNIFE,
		NULL,
/* precache */ ""
	},

	{
                NULL, 
		NULL,
                NULL,
                NULL,
                Weapon_DualGrenades,
		NULL,
                NULL, 0,
                "models/weapons/v_2handgr/tris.md2",
/* icon */      "a_m61frag",
/* pickup */    "Dual M26 Fragmentation Grenades",
/* width */     0,
                1,
                "M26 Fragmentation Grenade",
                "M26 Fragmentation Grenade",
                "M26 Fragmentation Grenade",
                "M26 Fragmentation Grenade",
                IT_AMMO|IT_WEAPON|IT_BIG|IT_TWIN|IT_SPECIAL,
                0,
                WEAP_M61FRAG,
		NULL,
/* precache */ ""
	},

// Dirty

// GRIM


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
		NULL,
		NULL,
		NULL,
		IT_POWERUP,
                10,
		0,
		NULL,
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
		NULL,
		NULL,
		NULL,
		IT_POWERUP,
                30,
                0,
		NULL,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},


/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
                Pickup_Breather,
		Use_Breather,
                Drop_Breather,
		NULL,
		"items/pkup.wav",
                "models/items/breather/tris.md2", 0,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
                "Filters",
		NULL,
		NULL,
		NULL,
                IT_POWERUP,
                30,
                0,
		NULL,
/* precache */ "items/airout.wav"
	},

/*QUAKED filters (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "ammo_filters",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
                "models/items/ammo/filters/tris.md2", 0,
		NULL,
/* icon */      "a_filters",
/* pickup */    "Filters",
/* width */		3,
                60,
		NULL,
		NULL,
		NULL,
                "Rebreather",
		IT_AMMO,
                30,
                0,
		NULL,
/* precache */ ""
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
                "models/items/enviro/tris.md2", 0,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_POWERUP,
                50,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
		0,
		0,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
                Use_Adrenaline, // GRIM
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
                1, // GRIM
		NULL,
		NULL,
		NULL,
		NULL,
                IT_BIG|IT_AMMO, // GRIM
                7, // GRIM
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
                Drop_Bandolier,
		NULL,
		"items/pkup.wav",
                "models/items/band/tris.md2", 0,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_POWERUP,
                7,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
                Drop_Pack,
		NULL,
		"items/pkup.wav",
                "models/items/pack/tris.md2", 0,
		NULL,
/* icon */		"i_pack",
/* pickup */    "Back Pack",
/* width */		2,
		180,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_POWERUP,
                25,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
                IT_KEY,
		0,
                0,
		NULL,
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
		NULL,
		NULL,
		NULL,
		0,
		0,
                0,
		NULL,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	//
        // FLAG ITEMS
	//

/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
                CTFDrop_Flag, // Can drop flag, it just tells everyone - GRIM
		NULL,
                "items/pkup.wav",
                "models/flags/red/tris.md2", 0, // Dirty - EF_FLAG1,
		NULL,
/* icon */      "i_ctf1",
/* pickup */	"Red Flag",
/* width */     2,
                1,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                20,
                0,
                NULL,
/* precache */ "teams/t1_fcp.wav teams/t1_frt.wav teams/t1_ftk.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
                CTFDrop_Flag, // Can drop flag, it just tells everyone - GRIM
		NULL,
                "items/pkup.wav",
                "models/flags/blue/tris.md2", 0, // Dirty - EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
                2,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                20,
                0,
		NULL,
/* precache */ "teams/t2_fcp.wav teams/t2_frt.wav teams/t2_ftk.wav"
	},

// GRIM
/*QUAKED item_flag_team3 (1 0.2 0) (-16 -16 -24) (16 16 32)*/
	{
                "item_flag_team3",
		CTFPickup_Flag,
		NULL,
                CTFDrop_Flag, // Can drop flag, it just tells everyone - GRIM
		NULL,
                "items/pkup.wav",
                "models/flags/green/tris.md2", 0, // Dirty - EF_FLAG2,
		NULL,
/* icon */              "i_ctf3",
/* pickup */    "Green Flag",
/* width */		2,
                3,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                20,
                0,
		NULL,
/* precache */ "teams/t3_fcp.wav teams/t3_frt.wav teams/t3_ftk.wav"
	},

/*QUAKED item_flag_team4 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
                "item_flag_team4",
		CTFPickup_Flag,
		NULL,
                CTFDrop_Flag, // Can drop flag, it just tells everyone - GRIM
		NULL,
                "items/pkup.wav",
                "models/flags/yellow/tris.md2", 0, // Dirty - EF_FLAG2,
		NULL,
/* icon */              "i_ctf4",
/* pickup */    "Yellow Flag",
/* width */		2,
                4,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                20,
                0,
		NULL,
/* precache */ "teams/t4_fcp.wav teams/t4_frt.wav teams/t4_ftk.wav"
	},
// GRIM
// ZOID

	//
        // HEALTH ITEMS
	//

	{
                "health_stimpack",
		Pickup_Health,
                Use_Health,
                Drop_Health,
		NULL,
                "items/s_health.wav",
                "models/items/healing/stimpack/tris.md2", 0,
		NULL,
/* icon */      "hi_stim",
/* pickup */    "StimPack",
/* width */		3,
                -1,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_AMMO,
                5,
                0,
		NULL,
/* precache */ ""
	},

	{
                "health_bindkit",
		Pickup_Health,
                Use_Health,
                Drop_Health,
		NULL,
                "items/n_health.wav",
                "models/items/healing/medium/tris.md2", 0,
		NULL,
/* icon */              "hi_bind",
/* pickup */    "BindKit",
/* width */		3,
                3,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_AMMO|IT_BIG,
                2, // ParanoiD - Half Weight
                0,
		NULL,
/* precache */ ""
	},

	{
                "health_medikit",
		Pickup_Health,
                Use_Health,
                Drop_Health,
		NULL,
                "items/l_health.wav",
                "models/items/healing/large/tris.md2", 0,
		NULL,
/* icon */              "hi_medi",
/* pickup */    "MediKit",
/* width */		3,
                6,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_AMMO|IT_BIG,
                4, // ParanoiD - Half Weight
                0,
		NULL,
/* precache */ ""
	},

        // Dirty
	{
                "health_medikit",
		Pickup_Health,
                Use_Health,
                Drop_Health,
		NULL,
                "items/l_health.wav",
                "models/items/healing/large/tris.md2", 0,
		NULL,
/* icon */              "hi_medi",
/* pickup */    "Bandages",
/* width */		3,
                1,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_AMMO|IT_BIG,
                0.2,
                0,
		NULL,
/* precache */ ""
	},
        // Dirty

	{
                "health_megapack",
		Pickup_Health,
                Use_Health,
                Drop_Health,
		NULL,
                "items/m_health.wav",
                "models/items/mega_h/tris.md2", 0,
		NULL,
/* icon */              "hi_mega",
/* pickup */    "Mega Health Pack",
/* width */     3,
                30,
                NULL,
		NULL,
		NULL,
		NULL,
                IT_AMMO|IT_BIG|IT_SPECIAL,
                20,
                0,
		NULL,
/* precache */ ""
	},

	//
        // MISC
	//
	{
                "gib_skull",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */      "k_comhead",
/* pickup */    "Skull",
/* width */     2,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_KEY,
                0,
                0,
		NULL,
/* precache */ ""
	},
// GRIM

// Dirty
/*QUAKED item_flashlight (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_flashlight",
                Pickup_Flashlight,
                Use_Light,
                Drop_Flashlight,
		NULL,
		"items/pkup.wav",
                "models/items/flash/tris.md2", 0,
		NULL,
/* icon */      "p_flash",
/* pickup */    "Flash Light",
/* width */     2,
                1,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                2,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_slippers (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_slippers",
                Pickup_Slippers,
                NULL,
                Drop_Slippers,
		NULL,
		"items/pkup.wav",
                "models/items/slippers/slippers.md2", 0,
		NULL,
/* icon */      "slippers",
/* pickup */    "Stealth Slippers",
/* width */     2,
                1,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                2,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
                Pickup_Silencer,
                NULL,
                Drop_Silencer,
		NULL,
		"items/pkup.wav",
                "models/items/quiet/tris.md2", 0,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                2,
                0,
		NULL,
/* precache */ ""
	},

/*QUAKED item_laser_sight (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
                "item_laser_sight",
                Pickup_LaserSight,
                NULL,
                Drop_LaserSight,
		NULL,
		"items/pkup.wav",
                "models/items/laser/tris.md2", 0,
		NULL,
/* icon */      "p_laser",
/* pickup */    "Laser Sight",
/* width */		2,
		60,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL,
                2,
                0,
		NULL,
/* precache */ ""
	},
// Dirty

// GRIM - FIX ME? Still needed in Dirty?
/* item_flag_generic
*/
	{
                "item_flag_generic",
		CTFPickup_Flag,
		NULL,
                NULL, // FIX ME URGENT LIKE!!!
                // CTFDrop_Tech, // Can drop flag, it just tells everyone - GRIM
		NULL,
                "items/pkup.wav",
                "models/flags/green/tris.md2", 0, // Dirty - EF_BLASTER,
		NULL,
/* icon */      "i_ctf3",
/* pickup */    "The Flag",
/* width */     2,
                5,
		NULL,
		NULL,
		NULL,
		NULL,
                IT_SPECIAL|IT_RAND,
                20,
                0,
		NULL,
/* precache */ "teams/t5_fcp.wav teams/t5_frt.wav teams/t5_ftk.wav"
	},

// GRIM
	// end of list marker
	{NULL}
};




// GRIM
// GRIM - New health stuff

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
        SpawnItem (self, FindItem ("BindKit"));
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
        SpawnItem (self, FindItem ("StimPack"));
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
        SpawnItem (self, FindItem ("MediKit"));
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
        SpawnItem (self, FindItem ("Mega Health Pack"));
}
// GRIM - New health stuff

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

        /* GRIM
	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
        */

	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}
