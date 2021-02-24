// p_weapon.c
#include "g_local.h"
#include "m_player.h"

// GRIM
/*
============================================================
Welcome to the even more edited then g_items.c p_weapon.c
Some ppl might find some things a bit hard to follow...
If so, bad fucking luck...
Look on the bright side...it's get's harder from here.
The weapon system I looking too next expands this and includes
everything else too!

Fun, a'?

// FuNBoY - DAMN MOTHERFUCKERS PULLIN SHIT DAMN!
============================================================
*/
// GRIM

static qboolean	is_quad;
static byte is_silenced;

//GRIM - Removed static
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (deathmatch->value)
		return;

        if (!who->client)
                return;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (who->flags & FL_NOTARGET)
		return;

	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}

void ZoomOff (edict_t *ent)
{
        ent->client->ps.fov = 90;
        ent->client->ps.stats[STAT_CENTRAL] = 0;
        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        stuffcmd(ent, "set crosshair $old_crosshair_dude; set sensitivity $b4_zoom_dude\n");
}

// FIX ME? WTF I do with this now?
/*
int ModAdjust (int type)
{
        int i;
        i = 0;

        if (type & TOD_EXPLOSIVE)
                i += 2;

        if (type & TOD_AP)
                i++;

        if (type & TOD_HV)
                i++;

        if (!(type & TOD_LEAD))
                i--;

        return i;
}
*/

/*
===============
MuzzleFlash - Hmm, I wanted to eventually use this to place actual
        flashes, instead of just a sound and shitty burst of light...
        Oh well, you work with what you got I suppose...
===============
*/
void MuzzleFlash (edict_t *ent, char *soundname, int left)
{
        int chan;
        if (left)
                chan = CHAN_OFFHAND;
        else
                chan = CHAN_GOODHAND;

        gi.sound(ent, chan, gi.soundindex(soundname), 1, ATTN_NORM, 0);
                                   
	// get start / end positions
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_BLASTER | is_silenced); // blaster fire sound NULL'd
        gi.multicast (ent->s.origin, MULTICAST_PVS);
}

//===========================================================================


/*
======================

What follows is simply a bunch of helper functions...
I used to just right all this shit out for some reason...
But we all some simplification, so I simplified the fuckers =]

======================
*/

/*
===============
SetAmmoIndexes - Save's doing it all the time in weapon_generic
===============
*/
void SetAmmoIndexes (edict_t *ent)
{
        ent->client->ammo_index = -1;
        ent->client->ammo_index2 = -1;

        // Dirty - TEMP
        // IT_AMMO check? Try to stop the weird shit with dual grenades
        if (ent->client->pers.weapon->flags & IT_AMMO)
                ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
        else if (ent->client->pers.goodhand->ammo)
                ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo));

        if (ent->client->pers.weapon2 && (ent->client->pers.weapon2->flags & IT_AMMO))
                ent->client->ammo_index2 = ITEM_INDEX(FindItem(ent->client->pers.weapon2->ammo));
        else if (ent->client->pers.weapon2 && ent->client->pers.offhand->ammo)
                ent->client->ammo_index2 = ITEM_INDEX(FindItem(ent->client->pers.offhand->ammo));
        // Dirty

        SetAmmoHud (ent);
}

/*
===============
SetOffHand - If the weapon has a second part, then this will set that up.
        That's all it's for, really.
        Used to do grenades in BraZen/Paranoid
===============
*/
void SetOffHand (edict_t *ent)
{
        if ((ent->client->pers.weapon_handling == NULL) && (ent->client->pers.weapon2 == NULL))
        {
                /*
                if ((ent->client->pers.weapon->weight < 30) && (!(ent->client->pers.weapon->flags & (IT_AMMO | IT_MELEE))))
                        ent->client->pers.weapon2 = FindItem("Hand Grenades");
                else */
                if (ent->client->pers.weapon->scomp)
                        ent->client->pers.weapon2 = FindItem(ent->client->pers.weapon->scomp);
        }

        if ((ent->client->pers.weapon_handling == NULL) && ent->client->pers.weapon2)
        {
                ent->client->pers.offhand->pickup_name = ent->client->pers.weapon2->pickup_name;
                ent->client->pers.offhand->ammo = ent->client->pers.weapon2->ammo;
                ent->client->pers.offhand->quantity = ent->client->pers.goodhand->s_quantity;
        }
}


/*
===============
SetLargeWeaponHUD - Instead of calling it every frame, just do it when needed
===============
*/
void SetLargeWeaponHUD (edict_t *ent)
{
        weapons_t       *slot;
        gitem_t         *item;

        slot = ent->client->pers.largeweapon;
        if (slot->pickup_name)
        {
                item = FindItem(slot->pickup_name);

                if ((!item) || (item == FindItem("Hands")))
                {
                        ent->client->ps.stats[STAT_LWEAPON] = 0;
                        SetSlot (ent, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
                        return;
                }

                if (item)
                        ent->client->ps.stats[STAT_LWEAPON] = gi.imageindex (item->icon);
        }
        else
                ent->client->ps.stats[STAT_LWEAPON] = 0;
}


/*
===============
SetHands - Bare hands. Zip, nada, nothing.
        For weapons that have no view model actually, which is only
        really hands... Cept in Paranoid, which includes claws.
===============
*/
void SetHands (edict_t *ent)
{
        ent->client->pers.weapon = FindItem("Hands");
        ent->client->pers.weapon2 = NULL;
        ent->client->pers.weapon_handling = NULL;
        ent->client->pers.goodhand->pickup_name = ent->client->pers.weapon->pickup_name;
        ent->client->pers.goodhand->ammo = NULL;
        ent->client->pers.offhand->pickup_name = ent->client->pers.weapon->pickup_name;
        ent->client->pers.offhand->ammo = NULL;
        ent->client->weaponstate = WEAPON_READY;
        ent->client->ps.gunindex = 0;
        ent->client->ps.gunframe = 0;
        ent->s.modelindex2 = 0;
        ent->client->pers.specials &= ~(SI_LASER_SIGHT | SI_LASER_SIGHT_HELP);
        SetLargeWeaponHUD (ent);
        SetAmmoIndexes (ent);
}


/*
===============
SetWepModels - Sets up weapon view models (hud side) and world models (vwep)
===============
*/
void SetWepModels (edict_t *ent)
{
        int     i;

        ent->client->pers.specials &= ~(SI_LASER_SIGHT | SI_LASER_SIGHT_HELP);
        if (!ent->client->pers.weapon->view_model)
        {
                ent->s.modelindex2 = 0;
                return;
        }

        ent->s.modelindex2 = 255;

        if (ent->client->pers.weapon_handling)
        {
                i = ((ent->client->pers.weapon_handling->weapmodel & 0xff) << 8);
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon_handling->view_model);
        }
        else if (ent->client->pers.weapon)
        {
                i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        }
        else
                i = 0;

        ent->s.skinnum = (ent - g_edicts - 1) | i;

        SetLargeWeaponHUD (ent);
}


/*
===============
ShowWhipOut - "Just wait a second while I whip this out!"
        What it says... plays generic "whip out" animation,
        sets gunframe to zero, sets weapons state to WEAPON_ACTIVATING
        and then calls the above functions...
===============
*/

void ShowWhipOut (edict_t *ent)
{
        //ShowItems(ent); // Dirty - Doesn't have VItems

	// set visible model
        if (!ent->client->pers.weapon->view_model)
        {
                SetHands (ent);
                return;
        }

        ent->client->pers.specials &= ~(SI_LASER_SIGHT | SI_LASER_SIGHT_HELP);

        SetWepModels (ent);
	ent->client->ps.gunframe = 0;
	ent->client->weaponstate = WEAPON_ACTIVATING;

	ent->client->anim_priority = ANIM_PAIN;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
                ent->s.frame = FRAME_crpain1;
                ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
                ent->s.frame = FRAME_pain301;
                ent->client->anim_end = FRAME_pain304;
	}

        SetOffHand (ent);
        SetAmmoIndexes (ent);
}


/*
===============
ChangeQuick - Doesn't do shit with any weapon slots.
        Just sets the weapon items and calls ShowWhipOut.
===============
*/
void ChangeQuick (edict_t *ent, gitem_t *item, qboolean left)
{
        gitem_t *comb = NULL;

        if (left)
        {
                ent->client->pers.weapon2 = item;
                comb = FindCombo (ent->client->pers.weapon->pickup_name, ent->client->pers.weapon2->pickup_name, false);
                if (comb == NULL)
                {
                        //gi.cprintf (ent, PRINT_HIGH, "Unknown combination.\n");
                        return; // err, what now?
                }
        }
        else
                ent->client->pers.weapon = item;

        ent->client->continuous_shots = 0;
	ent->client->machinegun_shots = 0;
        ent->client->weapon_sound = 0;
        ent->client->grenade_time = 0;

        if (comb)
                ent->client->pers.weapon_handling = comb;

        ShowWhipOut (ent);
}


/*
===============
SwitchGuns - Swaps weapon info from your "offhand" to your "goodhand"
        and then resets "offhand". Used for shooting guns out of hands,
        dropped "goodhand" weapon etc.
===============
*/
void SwitchGuns(edict_t *ent)
{
        gitem_t *weapon2;
        char    *pickup_name = NULL;
        char    *ammo = NULL;
        int     quantity = 0;
        char    *s_ammo = NULL;
        int     s_quantity = 0;
        int     damage = 0;
        int     flags = 0;

        pickup_name = ent->client->pers.offhand->pickup_name;
        ammo = ent->client->pers.offhand->ammo;
        quantity = ent->client->pers.offhand->quantity;
        s_ammo = ent->client->pers.offhand->s_ammo;
        s_quantity = ent->client->pers.offhand->s_quantity;
        damage = ent->client->pers.offhand->damage;
        flags = ent->client->pers.offhand->flags;

        weapon2 = ent->client->pers.weapon2;
        Drop_CurrentWeapon (ent, 1);
        ent->client->pers.weapon = weapon2;

        ent->client->pers.goodhand->pickup_name = pickup_name;
        ent->client->pers.goodhand->ammo = ammo;
        ent->client->pers.goodhand->quantity = quantity;
        ent->client->pers.goodhand->s_ammo = s_ammo;
        ent->client->pers.goodhand->s_quantity = s_quantity;
        ent->client->pers.goodhand->damage = damage;
        ent->client->pers.goodhand->flags = flags;

        // FIX ME - Order?
        ChangeQuick (ent, ent->client->pers.weapon, false);
}


//===========================================================================


/*
===============
Pickup_Weapon - Fuck... grew from it's original version, heh.
        FIX ME - Needs to include support for two different weapons
===============
*/
qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
        gitem_t         *ammo_clip;
        gitem_t         *item;
        gitem_t         *item2;
        int             index, count, scount, flags, damage;
        weapons_t       *slot;

        count = 0;
        scount = 0;
        flags = 0;
        damage = 0;
        slot = NULL;
        item = NULL;
        item2 = NULL;

        // If you are using your hands, then just "grab" it (auto-select)
        if ((!Q_stricmp(other->client->pers.weapon->pickup_name, "Hands")) && (other->client->pers.grabbing != 2))
                slot = other->client->pers.goodhand;
        // Weapon that requires normally requires two hands...
        else if (ent->item->flags & IT_BIG)
        {
                slot = other->client->pers.largeweapon;
                if (slot->pickup_name) // Large weapon slot full 
                {
                        // Dirty - FIX ME! No support for weapon combos
                        if ((other->client->pers.weapon_handling) || (other->client->pers.weapon->flags & (IT_AMMO | IT_MELEE)))
                                return false;
                        else if ((other->client->pers.weapon == ent->item) && (ent->item->flags & IT_TWIN))
                                slot = other->client->pers.offhand;
                        else
                                return false;
                        // Dirty - FIX ME! No support for weapon combos
                }
        }
        else
        {
                slot = FindBlank(other, 1);
                if (slot == NULL)
                {
                        // Dirty - FIX ME! No support for weapon combos
                        if ((other->client->pers.weapon_handling) || (other->client->pers.weapon->flags & (IT_AMMO | IT_MELEE)))
                                return false;
                        else if (other->client->pers.weapon == ent->item)
                                slot = other->client->pers.offhand;
                        else
                                return false;
                        // Dirty - FIX ME! No support for weapon combos
                }
        }

        if (slot == NULL)
                return false;

        // give them some ammo with it
        ammo_clip = FindItem(ent->item->ammo);
        if (ent->alternate_ammo)
                ammo_clip = FindItem(ent->alternate_ammo);

        if (ent->count > 0)
        {
                count = ent->count;

                // Just check that count aint too large...
                if (count > ent->item->quantity)
                        count = ent->item->quantity;
        }
        else if (!(ent->spawnflags & (DROPPED_PLAYER_ITEM)))
                count = ent->item->quantity;

        if (ent->flags)
                flags = ent->flags;
        
        if (ent->dmg)
                damage = ent->dmg;

        item = ent->item;
        index = ITEM_INDEX(item);

        if (item->flags & IT_RELOAD)
                SetSlot (other, slot->slot_name, item->pickup_name, flags, damage, ammo_clip->pickup_name, count, NULL, 0);
        else
        {       // give a quantity of 1 for now... FIX ME - Weapon in gun?
                SetSlot (other, slot->slot_name, item->pickup_name, flags, damage, ammo_clip->pickup_name, 1, NULL, 0);
                if (!Add_Ammo (other, ammo_clip, count))
                        Drop_Ammo_Specific (other, ammo_clip, count);
        }

        // Check for second part of weapon (ie Grenade Launcher on a Rifle)
        if (ent->item->scomp)
                item2 = FindItem(ent->item->scomp);

        if (item2 && (!(item2->flags & IT_AMMO)))
        {
                if (ent->modtype > 0)
                        scount = ent->modtype;
                else if (!(ent->spawnflags & (DROPPED_PLAYER_ITEM)))
                        scount = item2->quantity;

                if (item2->flags & IT_RELOAD)
                        SetSlot (other, slot->slot_name, item->pickup_name, flags, damage, ammo_clip->pickup_name, count, item2->ammo, scount);
                else
                {
                        SetSlot (other, slot->slot_name, item->pickup_name, flags, damage, ammo_clip->pickup_name, 0, item2->ammo, 0);
                        if (!Add_Ammo (other, FindItem(item2->ammo), scount))
                                Drop_Ammo_Specific (other, FindItem(item2->ammo), scount);
                }
        }

        other->client->pers.inventory[index]++;

        // ShowItems(other); // Dirty - Doesn't have VItems

        if ((!(ent->spawnflags & DROPPED_PLAYER_ITEM)) && (!(ent->spawnflags & DROPPED_ITEM)))
        {
                if (deathmatch->value)
                {
                        // Dirty - FIX ME? Make unique flag instead??
                        if ((int)bflags->value & BF_CHOOSE_STUFF)
                                SetRespawn (ent, 30);
                        else if (((int)dmflags->value & DF_WEAPONS_STAY) && (ent->item->flags & IT_BIG))
                        {
                                ent->flags |= FL_RESPAWN;
                                ent->svflags |= SVF_NOCLIENT;
                                ent->solid = SOLID_NOT;
                                gi.linkentity (ent);
                        }
                        else
                                SetRespawn (ent, 30);
                        // Dirty
                }
        }

        if (slot == other->client->pers.goodhand)
                ChangeQuick (other, item, false);
        else if (slot == other->client->pers.offhand)
                ChangeQuick (other, item, true);

        SetLargeWeaponHUD (other);

        //gi.dprintf ("Pickup_Weapon - slot_name %s, pickup_name %s\n", slot->slot_name, slot->pickup_name);
	return true;
}


//===========================================================================


/*
===============
PutAwayWeapon - Takes weapon info from good/off hand, and moves it to
        another slot. If one isn't available, returns false (which is BAD)
===============
*/
qboolean PutAwayWeapon (edict_t *ent, qboolean left)
{
        weapons_t       *hand;
        weapons_t       *slot = NULL;
        gitem_t         *weapon;

        if (left)
        {
                weapon = ent->client->pers.weapon2;
                hand = ent->client->pers.offhand;
        }
        else
        {
                weapon = ent->client->pers.weapon;
                hand = ent->client->pers.goodhand;
        }

        // It's ammo... goes anywhere...
        if (weapon->flags & IT_AMMO)
                return true;

        // Check large weapon slot
        if (weapon->flags & IT_BIG)
        {
                slot = ent->client->pers.largeweapon;
                // Large weapon slot full
                if (slot->pickup_name)
                        return false;
        }
        else
        {
                slot = FindBlank(ent, 1);
                if (slot == NULL)
                        return false;
        }

        // Copy info on weapon back to a slot...
        SetSlot (ent, slot->slot_name, hand->pickup_name, hand->flags, hand->damage, hand->ammo, hand->quantity, hand->s_ammo, hand->s_quantity);

        // Then blank out the hand (Which is classed as a slot)
        hand->pickup_name = NULL;
        hand->ammo = NULL;
        hand->quantity = -1;
        return true;
}

/*
===============
TakeOutWeapon
===============
*/
qboolean TakeOutWeapon (edict_t *ent, gitem_t *item, qboolean left)
{
        weapons_t       *hand;
        weapons_t       *slot = NULL;

        slot = FindClientWeapon (ent, item->pickup_name, 1);

        // Shit, we didn't find any info on this weapon!
        // We could fix it nicely, and give them brand new info.
        // Instead, let's just waste the fucker...
        if (!slot)
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--; // wasted
                return false;
        }

        if (left)
                hand = ent->client->pers.offhand;
        else
                hand = ent->client->pers.goodhand;

        // Copy info on weapon back to the hand...
        SetSlot (ent, hand->slot_name, slot->pickup_name, slot->flags, slot->damage, slot->ammo, slot->quantity, slot->s_ammo, slot->s_quantity);
        //gi.dprintf ("TakeOutWeapon - slot %s, quantity %i\n", hand->slot_name, hand->quantity);

        // Then blank out the slot.
        slot->pickup_name = NULL;
        slot->ammo = NULL;
        slot->quantity = -1;
        return true;
}

/*
===============
ChangeWeapon - We've fucked off the old weapon, so set up the new one
===============
*/
void ChangeWeapon2 (edict_t *ent)
{
        gitem_t         *comb;

        // This shouldn't happen...
        if (ent->client->pers.weapon_handling)
        {
                if (!PutAwayWeapon (ent, true))
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->newweapon = NULL;
                        ent->client->newweapon2 = NULL;
                        SetAmmoIndexes (ent);
                        SetAmmoHud (ent);
                        return; // err, something fucked up...
                }               // client probably locked on one gunframe
        }

        comb = FindCombo (ent->client->pers.weapon->pickup_name, ent->client->newweapon2->pickup_name, false);

        if (comb == NULL)
        {
                //gi.cprintf (ent, PRINT_HIGH, "Unknown combination.\n");
                ent->client->weaponstate = WEAPON_READY;
                ent->client->pers.prev_weapon2 = NULL;
                ent->client->pers.lastweapon2 = NULL;
                ent->client->newweapon = NULL;
                ent->client->newweapon2 = NULL;
                return;
        }

        ent->client->weapon_sound = 0;
	ent->client->machinegun_shots = 0;

        ent->client->pers.lastweapon2 = NULL;
        ent->client->pers.weapon2 = ent->client->newweapon2;
        ent->client->newweapon2 = NULL;
        ent->client->pers.weapon_handling = comb;

        // Get info...
        if (ent->client->pers.weapon2->flags & IT_AMMO)
                SetSlot (ent, "Off Hand", ent->client->pers.weapon2->pickup_name, 0, 0, ent->client->pers.weapon2->ammo, 0, NULL, 0);
        else if (!TakeOutWeapon (ent, ent->client->pers.weapon2, true))
        {
                ent->client->pers.weapon_handling = NULL;
                ent->client->pers.weapon2 = NULL;
        }

        ShowWhipOut (ent);
}

void ChangeWeapon (edict_t *ent)
{
        weapons_t       *slot = NULL;

        // seperate off hand weapon
        if (ent->client->pers.weapon_handling)
        {
                // Remove info...
                if (!PutAwayWeapon (ent, true))
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->newweapon = NULL;
                        ent->client->newweapon2 = NULL;
                        SetAmmoIndexes (ent);
                        SetAmmoHud (ent);
                        return; // err, something fucked up...
                                // client probably locked on one gunframe
                }               // bad :(

                ent->client->pers.weapon_handling = NULL;
                ent->client->pers.lastweapon2 = ent->client->pers.weapon2;
                ent->client->pers.weapon2 = NULL;
                ent->client->pers.use_second = false;

                // That's just the offhand weapon...
                // newweapon still exists, so we will be called again anyway.
                if (ent->health > 0)
                {
                        SetWepModels (ent);
                        ent->client->weaponstate = WEAPON_READY;

                        if (ent->client->pers.weapon->flags & IT_AMMO)
                                slot = NULL;
                        else if (ent->client->pers.weapon->flags & IT_BIG)
                        {
                                slot = ent->client->pers.largeweapon;
                                // Large weapon slot full
                                if (slot->pickup_name)
                                        slot = NULL;
                        }
                        else
                                slot = FindBlank(ent, 1);

                        // This checks if we can put the goodhand weapon
                        // away too... if not, stops us here
                        if ((!slot) && (!(ent->client->pers.weapon->flags & IT_AMMO)))
                        {
                                // If trying to bandage, just throw it
                                if (ent->client->newweapon == FindItem("Hands"))
                                {
                                        ent->client->newweapon = NULL;
                                        Drop_CurrentWeapon (ent, 1);
                                        return;
                                }
                                ent->client->newweapon = NULL;
                        }

                        SetOffHand (ent);
                        SetAmmoIndexes (ent);
                        return;
                }
        } // second part to weapon, offhand grenades (??)
        else if (ent->client->pers.weapon2)
        {
                ent->client->pers.weapon2 = NULL;
                slot = ent->client->pers.offhand;
                ent->client->pers.goodhand->s_ammo = slot->ammo;
                ent->client->pers.goodhand->s_quantity = slot->quantity;
                slot->pickup_name = NULL;
                slot->ammo = NULL;
                slot->quantity = -1;
        }

        ent->client->machinegun_shots = 0;
        ent->client->weapon_sound = 0;
        ent->client->grenade_time = 0;

        if (ent->deadflag)
        {
		ent->client->ps.gunindex = 0;
                return;
        }

        // Remove info...
        if (ent->client->pers.weapon->flags & (IT_AMMO | IT_MELEE))
        {
                slot = ent->client->pers.goodhand;
                slot->pickup_name = NULL;
                slot->ammo = NULL;
                slot->quantity = -1;
        }
        else if (!PutAwayWeapon (ent, false))
        {
                ent->client->weaponstate = WEAPON_READY;
                return; // Bad :(
        }

        ent->client->pers.lastweapon = ent->client->pers.weapon;
        ent->client->pers.weapon = ent->client->newweapon;
        ent->client->newweapon = NULL;

        // Get info...
        if (ent->client->pers.weapon->flags & (IT_AMMO | IT_MELEE))
        {
                slot = ent->client->pers.goodhand;
                slot->pickup_name = ent->client->pers.weapon->pickup_name;
                slot->ammo = ent->client->pers.weapon->ammo;
                slot->quantity = 0;
        }
        else if (!TakeOutWeapon (ent, ent->client->pers.weapon, false))
                ent->client->pers.weapon = FindItem("Hands"); // we fuck'd up

        ShowWhipOut (ent);
}
// GRIM

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	// if just died, put the weapon away
        if (ent->deadflag)
                return;

        // Dirty
        if (ent->client->hanging)
        {
                if (ent->client->old_gunindex == 0)
                {
                        ent->client->old_gunindex = ent->client->ps.gunindex;
                        ent->client->ps.gunindex = 0;
                }
                return;
        }
        // Dirty

        // Dirty
        if (ent->is_a_lame_mofo)
                return; // Can't even attack as a lame mofo
        // Dirty

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;

                // Check for two weapon handler and run that...
                if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
                        ent->client->pers.weapon_handling->weaponthink (ent);
                else
                        ent->client->pers.weapon->weaponthink (ent);
	}
}


/*
================
Use_Weapon - Check if we have info for this weapon...
        If so, everything's peachy... if not, waste the fucker
================
*/
void Use_Weapon2 (edict_t *ent, gitem_t *item)
{
        weapons_t       *slot = NULL;
        gitem_t         *comb;
        int             index;

        ent->client->pers.use_second = false;

	// see if we're already using it
        if (ent->client->pers.weapon2 && (item == ent->client->pers.weapon2))
		return;

        comb = FindCombo (ent->client->pers.weapon->pickup_name, item->pickup_name, false);
        if (comb == NULL)
                return;

        index = ITEM_INDEX(item);

        if (ent->client->pers.inventory[index] < 1)
                return;

        if ((comb->flags & IT_TWIN) && (ent->client->pers.inventory[index] < 2))
                return;

        // Dirty - the selection of ammo (knives)
        if (!(item->flags & IT_AMMO))
        {
                slot = FindClientWeapon (ent, item->pickup_name, 1);

                // Didn't find info for this weapon... waste the fucker
                if (!slot)
                {
                        ent->client->pers.inventory[index]--; // wasted
                        return;
                }
        }
        // Dirty

        if (ArmWounds(ent) > 6)
        {
                gi.cprintf (ent, PRINT_HIGH, "Arm too damaged!\n");
                return;
        }

        ent->client->pers.use_second = true;
        ent->client->newweapon2 = item;

        // Dirty - This will stop auto-reload...
        ent->client->continuous_shots = 0;
        ent->client->machinegun_shots = 0;
        // Dirty
}

void Use_Weapon (edict_t *ent, gitem_t *item)
{
        weapons_t       *slot = NULL;
        gitem_t         *weapon;
        int             count = 0;

        // Dirty
        // Already putting away our weapon...
        if (ent->client->weaponstate == WEAPON_DROPPING)
                return;

        // Can't do things while hanging off something!
        if (ent->client->hanging)
                return;
        // Dirty

	// see if we're already using it
        if (ent->client->pers.weapon_handling)
                ent->client->pers.use_second = false;
        else if ((item == ent->client->pers.weapon) || ent->client->pers.use_second)
        {
                Use_Weapon2 (ent, item);
                return;
        }

        // FIX ME - Add flag to define this?
        if (!(item->flags & (IT_MELEE | IT_AMMO)))
        {
                if (ent->client->pers.weapon == item)
                        count = 1;

                if (ent->client->pers.weapon_handling && ent->client->pers.weapon2 && (ent->client->pers.weapon2 == item))
                        count = 2;
                
                // We first check if there is another weapon greater then
                // the amount we're already using
                if (count > 0)
                {
                        if (ent->client->pers.inventory[ITEM_INDEX(item)] > count)
                        {
                                slot = FindClientWeapon (ent, item->pickup_name, 1);
                                if (!slot) // Didn't find info for this weapon... waste the fucker
                                {
                                        ent->client->pers.inventory[ITEM_INDEX(item)]--; // wasted
                                        return;
                                }
                        }
                        else // switching back to one
                                count = -1;
                }
        }

        // Free to go...
        // these are classed as ammo etc, and don't use weapon slots
        if ((ent->client->pers.weapon->flags & IT_MELEE) || (ent->client->pers.weapon->flags & IT_AMMO))
        {
                ent->client->newweapon = item;
                return;
        }

        // Check for a place to put one weapon, at least...
        if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
                weapon = ent->client->pers.weapon2;
        else
                weapon = ent->client->pers.weapon;

        if (weapon->flags & IT_BIG)
        {
                slot = ent->client->pers.largeweapon;
                // No free spot to put large weapon...
                if (slot->pickup_name)
                        slot = NULL;
        }
        else
                slot = FindBlank(ent, 1);

        // Heh, no space so we just drop em'
        if (!slot)
        {       // I hope you know that weapon_handling denotes a SEPERATE
                // second weapon and NOT weapon2. Weapon2 could be a weapon
                // second component... like a rifles grenade launcher
                if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
                        Drop_CurrentWeapon (ent, 2);

                if (ent->client->pers.weapon && (count > -1))
                        Drop_CurrentWeapon (ent, 1);

                if (count == -1) // not just switching back to one weapon
                        return;
        }

        // check done, change to this weapon when down
        ent->client->newweapon = item;

        // Dirty - This will stop auto-reload...
        ent->client->continuous_shots = 0;
        ent->client->machinegun_shots = 0;
        // Dirty
}


/*
================
Drop_CurrentWeapon - Drop a weapon we have in our hands
        Is called if they players asks or if the player dies.
        This unfortunately means that some nasty looking hacks are
        added for certain weapons, but who gives a fuck in the end, right? =]
================
*/
void weapon_grenade_fire (edict_t *ent, qboolean left, int speed);

qboolean LiveGrenadeCheck (edict_t *ent)
{
        if (!ent->client->pulled_pin)
                return false;

        if (ent->client->pers.weapon2)
        {
                weapon_grenade_fire (ent, false, 1);
                weapon_grenade_fire (ent, true, 1);
        }
        else
                weapon_grenade_fire (ent, false, 3);

        if (!ent->deadflag)
                SetHands (ent);

        return true;
}

void Drop_CurrentWeapon (edict_t *ent, int which)
{
        edict_t         *dropped;
        gitem_t         *weapon;
        weapons_t       *wep = NULL;
        int             index, ammo_index;

        if (ent->client->pers.weapon == FindItem("Hands"))
                return;

        // Dirty - Unless your firing or activating, it doesn't give a fuck...
        if ((ent->client->weaponstate == WEAPON_FIRING) || (ent->client->weaponstate == WEAPON_FIRING2)
          || (ent->client->weaponstate == WEAPON_FIRING3) || (ent->client->weaponstate == WEAPON_ACTIVATING))
		return;
        // Dirty

        // FIX ME - dual melee weapon or ammos? 
        if ((ent->client->pers.weapon->flags & IT_MELEE) || (ent->client->pers.weapon->flags & IT_AMMO))
        {
                // Grenade death... FIX ME - Check for wrong frames?
                // Dirty - Changed frame for Dirty's grenade...
                if (LiveGrenadeCheck (ent))
                        return;
                // Dirty

                if (ent->client->pers.weapon != FindItem("Hands"))
                {
                        // Dirty
                        if (ent->client->pers.weapon2 && ent->client->pers.weapon2->drop)
                        {
                                ent->client->v_angle[YAW] -= 7;
                                ent->client->pers.weapon2->drop (ent, ent->client->pers.weapon2);
                                ent->client->v_angle[YAW] += 7;
                        }

                        if (ent->client->pers.weapon->drop)
                        {
                                ent->client->v_angle[YAW] += 7;
                                ent->client->pers.weapon->drop (ent, ent->client->pers.weapon);
                                ent->client->v_angle[YAW] -= 7;
                        }
                        // Dirty
                        if (!ent->deadflag)
                                SetHands (ent);
                }
                return;
        }

        if ((which == 2) && ent->client->pers.weapon2 && ent->client->pers.weapon_handling)
        {
                weapon = ent->client->pers.weapon2;
                wep = ent->client->pers.offhand;
                ent->client->v_angle[YAW] += 7;
                dropped = Drop_Item (ent, weapon, ITEM_UNDEF);
                ent->client->v_angle[YAW] -= 7;
                dropped->s.frame = 0; // Dirty
        }
        else if ((which == 1) && ent->client->pers.weapon)
        {
                weapon = ent->client->pers.weapon;
                wep = ent->client->pers.goodhand;
                ent->client->v_angle[YAW] -= 7;
                dropped = Drop_Item (ent, weapon, ITEM_UNDEF);
                ent->client->v_angle[YAW] += 7;
                dropped->s.frame = 1; // Dirty
        }
        else
                return;

        ammo_index = ITEM_INDEX(FindItem(wep->ammo));

        index = ITEM_INDEX(weapon);

        dropped->count = 0;
        dropped->flags = wep->flags;

        if (weapon->flags & IT_RELOAD)
        {
                if (wep->quantity > 0)
                {
                        dropped->count = wep->quantity;
                        wep->quantity = 0;
                }
        }
        else if (ent->client->pers.inventory[index] == 1)
        {
                if (ent->client->pers.inventory[ammo_index] >= weapon->quantity)
                        dropped->count = weapon->quantity;
                else
                        dropped->count = ent->client->pers.inventory[ammo_index];
                ent->client->pers.inventory[ammo_index] -= dropped->count;
                RemoveAmmoWeight (ent, FindItem(wep->ammo), dropped->count);
        }

        ent->client->pers.inventory[index]--;

        if ((dropped->count > 0) && (Q_stricmp(wep->ammo, weapon->ammo)))
                dropped->alternate_ammo = wep->ammo;

        // If secondary weapon (eg Rifle's Grenade Launcher),
        // add second count if it has ammo (& isn't ammo itself!)
        if (weapon->scomp && ent->client->pers.weapon2 && (!(ent->client->pers.weapon2->flags & IT_AMMO)))
        {
                if (ent->client->pers.weapon_handling == NULL)
                        ent->client->pers.goodhand->s_quantity = ent->client->pers.offhand->quantity;

                if (ent->client->pers.goodhand->s_quantity >= 0)
                        dropped->modtype = ent->client->pers.goodhand->s_quantity;
                /* FIX ME - Support second kind of ammo in secondary weapon?
                if (Q_stricmp(wep->ammo, weapon->ammo))
                        drop->alternate_ammo = wep->ammo;
                */
        }

        ent->client->weaponstate = WEAPON_READY;
        ent->client->pers.use_second = false;

        wep->pickup_name = NULL;
        wep->ammo = NULL;
        wep->quantity = -1;
        wep->flags = 0;

        if (ent->deadflag)
                ent->client->pers.weapon_handling = NULL;
        else if (which == 2)
        {
                ent->client->pers.prev_weapon2 = ent->client->pers.weapon2;
                ent->client->pers.weapon_handling = NULL;
                ent->client->pers.use_second = false;
                ent->client->pers.offhand->ammo = NULL;
                SetWepModels (ent);
                SetAmmoIndexes (ent);
                SetOffHand (ent);
        }
        else
        {
                ent->client->pers.prev_weapon = ent->client->pers.weapon;
                ent->client->ps.gunindex = 0;
                ent->client->ps.gunframe = 0;
                ent->s.modelindex2 = 0;
                SetHands (ent);
        }

        ent->client->pers.weapon2 = NULL;

        if ((ent->client->pers.inventory[ammo_index] > 0) && (weapon != ent->client->pers.weapon))
                ent->client->pers.selected_item = ammo_index;

        if (dropped->flags & W_EMPTY)
               dropped->s.frame += 2;

        if (wep->damage)
                dropped->dmg = wep->damage;

        if (ent->client->ps.fov != 90)
                ZoomOff(ent);

        //ShowItems(ent); // Dirty - Doesn't support em'

        RemoveItemWeight (ent, weapon);
        ValidateSelectedItem (ent);

        // Heh... this groovy little hack is for grabbing someones gun =P
        if (ent->corpse && ent->corpse->client)
        {
                ent->corpse->client->pers.grabbing = 1;
                ent->corpse->client->weaponstate = WEAPON_READY;
                Touch_Item (dropped, ent->corpse, NULL, NULL);
                ent->corpse->client->pers.grabbing = 0;
                ent->corpse = NULL;
        }
}

// Drop all weapons you're using or bring one out if not using any.
void Cmd_DropWepQuick (edict_t *ent)
{
        // You're a spectator! Fuck off already!
        if (ent->movetype == MOVETYPE_NOCLIP)
                return;

        if (ent->deadflag || (ent->health < 1))
                return;

        if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
        {
                Cmd_WeapLast_f (ent);
                return;
        }

        // Dirty - This will stop auto-reload...
        ent->client->continuous_shots = 0;
        ent->client->machinegun_shots = 0;
        // Dirty

        if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
        {
                qboolean good = false;
                qboolean off = false;

                if ((ent->client->pers.offhand->quantity <= ent->client->pers.goodhand->quantity)
                 || (ent->client->pers.offhand->quantity == 0))
                        off = true;

                if ((ent->client->pers.goodhand->quantity <= ent->client->pers.offhand->quantity)
                 || (ent->client->pers.goodhand->quantity == 0))
                        good = true;

                if (off && good)
                {
                        Drop_CurrentWeapon (ent, 2);
                        Drop_CurrentWeapon (ent, 1);
                }
                else if (off)
                        Drop_CurrentWeapon (ent, 2);
                else
                        SwitchGuns (ent);
        }
        else if (ent->client->pers.weapon)
                Drop_CurrentWeapon (ent, 1);
}

/*
================
Drop_Weapon - Drop weapons you aint using
        If in fact, you are using them, then drop them anyway, heh
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
        edict_t         *dropped;
        weapons_t       *wep;
        int             index, ammo_index;

        // Must be idle....doesn't get called when dead, so there!
        if (ent->client->weaponstate != WEAPON_READY)
                return;         
 
        if (item->flags & IT_AMMO) // err, wtf?
                return;

        if (item == FindItem("Hands"))
                return;

        index = ITEM_INDEX(item);

        if (ent->client->pers.inventory[index] == 1)
        {
                if ((ent->client->pers.weapon_handling) && (item == ent->client->pers.weapon2))
                {
                        Drop_CurrentWeapon(ent, 2);
                        return;
                }
                else if (item == ent->client->pers.weapon)
                {
                        Drop_CurrentWeapon(ent, 1);
                        return;
                }
        }
        else if (ent->client->pers.inventory[index] == 2)
        {
                if ((ent->client->pers.weapon_handling) && (item == ent->client->pers.weapon2) && (item == ent->client->pers.weapon))
                {
                        Drop_CurrentWeapon(ent, 2);
                        return;
                }
        }

        wep = FindClientWeapon (ent, item->pickup_name, 1);

        // No info on the weapon? Then waste it!
        if (wep == NULL)
        {
                ent->client->pers.inventory[index]--; // wasted
                return;
        }

        ammo_index = ITEM_INDEX(FindItem(wep->ammo));

        dropped = Drop_Item (ent, item, ITEM_UNDEF);
        dropped->count = 0;
        dropped->flags = wep->flags;

        if (item->flags & IT_RELOAD)
        {
                if (wep->quantity > 0)
                {
                        // This is neat...
                        // Strip the ammo if hands free. Simple, no?
                        if ((ent->client->pers.weapon == FindItem("Hands"))
                         && (ent->health > 0) && (Add_Ammo (ent, FindItem(wep->ammo), wep->quantity)))
                                dropped->flags |= W_EMPTY;
                        else
                                dropped->count = wep->quantity;

                        wep->quantity = 0;
                }
        }
        else if (ent->client->pers.inventory[index] == 1)
        {
                if (ent->client->pers.inventory[ammo_index] >= item->quantity)
                        dropped->count = item->quantity;
                else
                        dropped->count = ent->client->pers.inventory[ammo_index];
                ent->client->pers.inventory[ammo_index] -= dropped->count;
                RemoveAmmoWeight (ent, FindItem(wep->ammo), dropped->count);
        }

        ent->client->pers.inventory[index] --;

        if ((dropped->count > 0) && (Q_stricmp(wep->ammo, item->ammo)))
                dropped->alternate_ammo = wep->ammo;

        // If secondary weapon (eg Rifle's Grenade Launcher),
        // add second count if it has ammo
        if (item->scomp && wep->s_ammo)
        {
                if (wep->s_quantity > 0)
                        dropped->modtype = wep->s_quantity;
                /* FIX ME
                if (Q_stricmp(wep->ammo, item->ammo))
                        dropped->alternate_ammo = wep->ammo;
                */
        }

        wep->pickup_name = NULL;
        wep->ammo = NULL;
        wep->quantity = -1;
        wep->flags = 0;

        if (ent->client->pers.inventory[ammo_index] > 0)
                ent->client->pers.selected_item = ammo_index;

        // Dirty - Paranoid doesn't have the models yet
        if (dropped->flags & W_EMPTY)
                dropped->s.frame += 2;
        // Dirty

        if (wep->damage)
                dropped->dmg = wep->damage;

        SetLargeWeaponHUD (ent);
        // ShowItems(ent); // Dirty - Doesn't support it

        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}


// GRIM - THIS NEXT WHOLE SECTION IS EDITED TO SHIT
/*
=================
NoAmmoCheckAlternate's
Called when out of ammo being used,
to check if there is alternate ammo to use.
=================
*/
qboolean NoAmmoCheckAlternate2 (edict_t *ent)
{
        ent->client->pers.offhand->ammo = ent->client->pers.weapon2->ammo;
        ent->alternate_ammo = NULL;

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon2->ammo))])
                return true;

        return false;
}

qboolean NoAmmoCheckAlternate (edict_t *ent)
{
        ent->client->pers.goodhand->ammo = ent->client->pers.weapon->ammo;
        ent->alternate_ammo = NULL;

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo))])
                return true;

        return false;
}
// GRIM


/*
================
Weapon_Generic

A generic function to handle the basics of MOST weapon thinking
================
*/
#define FRAME_FIRE_FIRST        (FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST        (FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST  (FRAME_IDLE_LAST + 1)
#define FRAME_RELOAD_FIRST      (FRAME_DEACTIVATE_LAST + 1)

// GRIM
#define FRAME_FIRE_FIRST2       (FRAME_RELOAD_LAST + 1)
#define FRAME_RELOAD_FIRST2     (FRAME_FIRE_LAST2 + 1)
// GRIM

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST,
 int FRAME_DEACTIVATE_LAST, int FRAME_RELOAD_LAST, int FRAME_AMMO_OUT, int FRAME_AMMO_IN,
  int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent), int FRAME_FIRE_LAST2,
   int FRAME_RELOAD_LAST2, int FRAME_AMMO_OUT2, int FRAME_AMMO_IN2, int *fire_frames2,
    void (*fire2)(edict_t *ent), void (*fire3)(edict_t *ent))
{
        weapons_t *wep;
        weapons_t *wep2 = NULL;
        int     old_ammo, old_ammo2, n;
        qboolean no_ammo = false;

        // VWep animations screw up corpses
        if (ent->deadflag) // || ent->s.modelindex != 255)
		return;

        // Dirty
        if ((ent->client->ps.gunindex == 0) && (ent->client->ps.fov == 90))
        {
                ent->client->ps.gunindex = ent->client->old_gunindex;
                ent->client->old_gunindex = 0;
        }
        // Dirty

        wep = ent->client->pers.goodhand;

        if (ent->client->ammo_index)
        {
                if (ent->client->pers.weapon->flags & IT_RELOAD)
                        old_ammo = wep->quantity;
                else
                        old_ammo = ent->client->pers.inventory[ent->client->ammo_index];
        }
        else
                old_ammo = -1;

        if (ent->client->pers.weapon2 && ent->client->ammo_index2)
        {
                wep2 = ent->client->pers.offhand;
                if (ent->client->pers.weapon2->flags & IT_RELOAD)
                        old_ammo2 = wep2->quantity;
                else
                        old_ammo2 = ent->client->pers.inventory[ent->client->ammo_index2];
        }
        else
                old_ammo2 = old_ammo;

        //
        // Reloading section
        //
        // GRIM - When in reload2 state, a check is needed for both
        //      two weapons and clip. For a single weapon which can
        //      be reloaded here, s_quantity is use instead of quantity.
        //      FIX ME - Add support for a single weapon's secondary weapon
        //              using a clip! Right now, only non-clippers will work
        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (!wep2)
                {
			ent->client->weaponstate = WEAPON_READY;
                        return;
                }
                        
                if ((ent->client->ps.gunframe < FRAME_RELOAD_FIRST2)
                 && (old_ammo2 >= ent->client->pers.weapon2->quantity)
                  && (ent->alternate_ammo == NULL))
                {
			ent->client->weaponstate = WEAPON_READY;
                        return;
                }

                if ((old_ammo2 < 1) && (ent->client->pers.inventory[ent->client->ammo_index2] <= 0)
                 && ent->client->ps.gunframe <= FRAME_RELOAD_FIRST2 && (ent->alternate_ammo == NULL))
                {
                        if (!NoAmmoCheckAlternate2(ent))
                        {
                                ent->client->weaponstate = WEAPON_READY;
                                return;
                        }
                }

                if (ent->client->ps.gunframe < FRAME_RELOAD_FIRST2)
                        ent->client->ps.gunframe = FRAME_RELOAD_FIRST2;

                if (ent->client->ps.gunframe == FRAME_AMMO_OUT2)
                {
                        if (ent->client->pers.weapon2->flags & IT_CLIP)
                        {
                                if (!(wep2->flags & W_EMPTY))
                                        Drop_Clip (ent, &itemlist[ent->client->ammo_index2], old_ammo2);
                                wep2->flags |= W_EMPTY;
                                wep2->flags |= W_UNLOADED; // Dirty
                                wep2->quantity = -1;
                                A2HudOff (ent);
                        }

                        if (ent->alternate_ammo)
                        {
                                if (ent->client->pers.weapon_handling)
                                        wep2->ammo = ent->alternate_ammo;
                                else
                                        wep2->s_ammo = ent->alternate_ammo;
                        }
                }
        
                if (ent->client->ps.gunframe == FRAME_AMMO_IN2)
                {
                        if (ent->client->pers.weapon2->flags & IT_CLIP)
                        {
                                int     clip_index2;
                                gitem_t *ammo_in2;

                                ammo_in2 = FindItem(wep2->ammo);
                                clip_index2 = CLIP_INDEX(FindClip(ammo_in2->pickup_name));
                                
                                if (ent->client->pers.clips[clip_index2] <= ammo_in2->quantity)
                                {
                                        wep2->quantity = ent->client->pers.clips[clip_index2];
                                        ent->client->pers.clips[clip_index2] = 0;
                                        if (ent->client->pers.inventory[ent->client->ammo_index2] > 1)
                                        {
                                                while (ent->client->pers.inventory[ent->client->ammo_index2] > 1)
                                                {
                                                        ent->client->pers.inventory[ent->client->ammo_index2] -= 1;
                                                        RemoveAmmoWeight (ent, ammo_in2, 1);
                                                        Add_Ammo (ent, FindItem(ammo_in2->scomp), 0);
                                                }
                                        }
                                        ent->client->pers.inventory[ent->client->ammo_index2] = 0;
                                }
                                else if (ent->client->pers.clips[clip_index2])
                                {
                                        wep2->quantity = ammo_in2->quantity;
                                        ent->client->pers.clips[clip_index2] -= ammo_in2->quantity;
                                        ent->client->pers.inventory[ent->client->ammo_index2] -= 1;
                                }
                                RemoveAmmoWeight (ent, ammo_in2, 1);
                        }
                        else if (ent->client->pers.inventory[ent->client->ammo_index2] > 0)
                        {
                                ent->client->pers.inventory[ent->client->ammo_index2]--;
                                wep2->quantity++;
                                RemoveAmmoWeight (ent, &itemlist[ent->client->ammo_index2], 1);
                        }

                        if (ent->alternate_ammo)
                                ent->alternate_ammo = NULL;

                        if ((wep2->flags & W_EMPTY) && (wep2->quantity > 0))
                                wep2->flags &= ~W_EMPTY;

                        if (ent->client->pers.inventory[ent->client->ammo_index2] < 1)
                                ValidateSelectedItem (ent);

                        SetAmmoHud (ent);
                }

                if ((ent->client->machinegun_shots > 0) && (ent->client->ps.gunframe >= ent->client->machinegun_shots))
                {
                        if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
                         || ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE)
                          || ent->client->pers.grabbing)
                        {
                                ent->client->continuous_shots = 0;
                                ent->client->machinegun_shots = 0;
                        }
                        else if ((!(ent->client->pers.weapon2->flags & IT_CLIP)) && (ent->client->pers.inventory[ent->client->ammo_index2] > 0)
                         && (wep2->quantity < ent->client->pers.weapon2->quantity))
                                ent->client->ps.gunframe = ent->client->continuous_shots;
                }


                if (ent->client->ps.gunframe >= FRAME_RELOAD_LAST2)
                {
                        ent->alternate_ammo = NULL;
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                        ent->client->weaponstate = WEAPON_READY;
                        if (ent->client->pers.weapon_handling && (ent->client->pers.weapon_handling->flags & IT_TWIN))
                        {
                                // Must be using identical ammo...
                                if ((old_ammo < 1) && (!strcmp(wep->ammo, wep2->ammo)))
                                {
                                        if (ent->client->pers.inventory[ent->client->ammo_index])
                                                ent->client->weaponstate = WEAPON_RELOADING;
                                        else if (NoAmmoCheckAlternate(ent))
                                                ent->client->weaponstate = WEAPON_RELOADING;
                                        else // Change to the gun in our left hand...
                                                SwitchGuns (ent);
                                }
                        }
                        return;
                }

                ent->client->ps.gunframe++;
                return;
        }

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                // Dirty
                if (ent->client->ps.fov != 90)
                        ZoomOff(ent);
                // Dirty

                 if ((ent->client->ps.gunframe < FRAME_RELOAD_FIRST)
                   && (old_ammo >= ent->client->pers.weapon->quantity)
                    && (ent->alternate_ammo == NULL))
                {
			ent->client->weaponstate = WEAPON_READY;
                        return;
                }

                if ((old_ammo < 1) && (ent->client->pers.inventory[ent->client->ammo_index] == 0)
                 && ent->client->ps.gunframe <= FRAME_RELOAD_FIRST && (ent->alternate_ammo == NULL))
                {
                        if (!NoAmmoCheckAlternate(ent))
                        {
                                ent->client->weaponstate = WEAPON_READY;
                                return;
                        }
                }

                if (ent->client->ps.gunframe < FRAME_RELOAD_FIRST)
                        ent->client->ps.gunframe = FRAME_RELOAD_FIRST;

                if (ent->client->ps.gunframe == FRAME_AMMO_OUT)
                {
                        if ((ent->client->pers.weapon->flags & IT_CLIP))
                        {
                                if (!(wep->flags & W_EMPTY))
                                        Drop_Clip (ent, &itemlist[ent->client->ammo_index], old_ammo);
                                wep->flags |= W_EMPTY;
                                wep->flags |= W_UNLOADED; // Dirty
                                wep->quantity = -1;
                                A1HudOff (ent);
                        }
                        if (ent->alternate_ammo)
                                wep->ammo = ent->alternate_ammo;
                }
        
                if (ent->client->ps.gunframe == FRAME_AMMO_IN)
                {
                        if (ent->client->pers.weapon->flags & IT_CLIP)
                        {
                                int     clip_index;
                                gitem_t *ammo_in;

                                ammo_in = FindItem(wep->ammo);
                                clip_index = CLIP_INDEX(FindClip(ammo_in->pickup_name));

                                if (ent->client->pers.clips[clip_index] <= ammo_in->quantity)
                                {
                                        wep->quantity = ent->client->pers.clips[clip_index];
                                        ent->client->pers.clips[clip_index] = 0;
                                        if (ent->client->pers.inventory[ent->client->ammo_index] > 1)
                                        {
                                                while (ent->client->pers.inventory[ent->client->ammo_index] > 1)
                                                {
                                                        ent->client->pers.inventory[ent->client->ammo_index] -= 1;
                                                        RemoveAmmoWeight (ent, ammo_in, 1);
                                                        Add_Ammo (ent, FindItem(ammo_in->scomp), 0);
                                                }
                                        }
                                        ent->client->pers.inventory[ent->client->ammo_index] = 0;
                                }
                                else if (ent->client->pers.clips[clip_index])
                                {
                                        wep->quantity = ammo_in->quantity;
                                        ent->client->pers.clips[clip_index] -= ammo_in->quantity;
                                        ent->client->pers.inventory[ent->client->ammo_index] -= 1;
                                }
                                RemoveAmmoWeight (ent, ammo_in, 1);
                        }
                        else if (ent->client->pers.inventory[ent->client->ammo_index] > 0)
                        {
                                ent->client->pers.inventory[ent->client->ammo_index]--;
                                wep->quantity++;
                                RemoveAmmoWeight (ent, &itemlist[ent->client->ammo_index], 1);
                        }
                        if (ent->alternate_ammo)
                                ent->alternate_ammo = NULL;

                        if ((wep->flags & W_EMPTY) && (wep->quantity > 0))
                                wep->flags &= ~W_EMPTY;
                        if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                                ValidateSelectedItem (ent);

                        SetAmmoHud (ent);
                }

                if ((ent->client->machinegun_shots > 0) && (ent->client->ps.gunframe >= ent->client->machinegun_shots))
                {
                        if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
                         || ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE)
                          || ent->client->pers.grabbing)
                        {
                                ent->client->continuous_shots = 0;
                                ent->client->machinegun_shots = 0;
                        }
                        else if ((!(ent->client->pers.weapon->flags & IT_CLIP)) && (ent->client->pers.inventory[ent->client->ammo_index] > 0)
                         && (wep->quantity < ent->client->pers.weapon->quantity))
                                ent->client->ps.gunframe = ent->client->continuous_shots;
                }

                if (ent->client->ps.gunframe >= FRAME_RELOAD_LAST)
                {
                        ent->alternate_ammo = NULL;
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                        ent->client->weaponstate = WEAPON_READY;
                        if ( (ent->client->pers.weapon_handling) && (ent->client->pers.weapon_handling->flags & IT_TWIN) )
                        {
                                // Must be using identical ammo...
                                if ((old_ammo2 < 1) && (!strcmp(wep->ammo, wep2->ammo)))
                                {
                                        if (ent->client->pers.inventory[ent->client->ammo_index2])
                                                ent->client->weaponstate = WEAPON_RELOADING2;
                                        else if (NoAmmoCheckAlternate2(ent))
                                                ent->client->weaponstate = WEAPON_RELOADING2;
                                }
                        }
                        return;
                }

                ent->client->ps.gunframe++;
                return;
        }

        //
        // Selection section
        //
        // Seems that I finally fixed this...
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
                if ((ent->client->ps.gunframe > FRAME_DEACTIVATE_LAST) || (ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST))
                        ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

                if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
                {
                        if (ent->client->newweapon2)
                                ChangeWeapon2(ent);
                        else
                                ChangeWeapon(ent);
                }
                else
                        ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                // Dirty
                if (ent->client->ps.fov != 90)
                        ZoomOff(ent);
                // Dirty

                if (ent->client->pers.grabbing)
                        ent->client->pers.grabbing = 0;

                if (ent->client->ps.gunframe >= FRAME_ACTIVATE_LAST)
		{
                        ent->client->continuous_shots = 0;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

        //
        // Idle section - FIX ME??
        //
	if (ent->client->weaponstate == WEAPON_READY)
	{
                qboolean        attack, attack2;

                ent->client->machinegun_shots = 0;
                ent->client->weapon_forward_set = 0;
                ent->client->weapon_side_set = 0;

                // Dirty - try this out. Improves first shot
                if ((!ent->velocity[0]) && (!ent->velocity[1]))
                {
                        ent->client->held_shots += 0.1;
                        
                        if (ent->client->held_shots > 5)
                                ent->client->held_shots = 5;
                }        
                // Dirty

                ent->client->continuous_shots = 0;

                attack = false;
                attack2 = false;

                if (ent->client->newweapon || ent->client->newweapon2)
                {
                        // Switching from two weapons back to one...
                        if (ent->client->newweapon && (!ent->client->pers.weapon_handling)
                         && (ent->client->pers.weapon == ent->client->newweapon))
                        {
                                ent->client->newweapon = NULL;
                                //SetAmmoIndexes (ent);
                                return;
                        }

                        // Hentai's original code! Woohoo! =]
                        if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
                        {
                                ent->client->anim_priority = ANIM_REVERSE;
                                if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                                {
                                        ent->s.frame = FRAME_crpain4+1;
                                        ent->client->anim_end = FRAME_crpain1;
                                }
                                else
                                {
                                        ent->s.frame = FRAME_pain304+1;
                                        ent->client->anim_end = FRAME_pain301;
				
                                }
                        }
                }

                if (ent->client->newweapon)
                {
                        // Switching from two weapons back to one...again
                        if ((ent->client->pers.weapon_handling == NULL) && (ent->client->pers.weapon == ent->client->newweapon))
                        {
                                ent->client->newweapon = NULL;
                                //SetAmmoIndexes (ent);
                        }
                        else
                        {
                                ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
                                ent->client->weaponstate = WEAPON_DROPPING;
                                if (ent->client->pers.weapon_handling)
                                {
                                        ent->client->ammo_index2 = -1;
                                        A2HudOff (ent);
                                }
                                else
                                {
                                        ent->client->ammo_index = -1;
                                        A1HudOff (ent);
                                }
                        }
                        return;
                }
                else if (ent->client->newweapon2)
                {
                        if (ent->client->pers.weapon_handling == NULL)
                        {
                                ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                                ChangeWeapon2 (ent);
                        }
                        else
                        {
                                ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
                                ent->client->weaponstate = WEAPON_DROPPING;
                                if (ent->client->pers.weapon_handling)
                                {
                                        ent->client->ammo_index2 = -1;
                                        A2HudOff (ent);
                                }
                                else
                                {
                                        ent->client->ammo_index = -1;
                                        A1HudOff (ent);
                                }
                        }
                        return;
                }

                if (ent->client->ps.gunframe > FRAME_IDLE_LAST || (ent->client->ps.gunframe < FRAME_ACTIVATE_LAST))
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;

                if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        attack = true;
                }

                if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE)
                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        attack2 = true;
                }

                // May not fire until match has actually begun...
                if ((level.match_state == MATCH_START) || (ent->client->ps.gunframe < (FRAME_IDLE_FIRST + 1)))
                {
                        attack = false;
                        attack2 = false;
                }

                if (attack || attack2)
		{
                        // Dirty - Off with the extra info
                        if (ent->client->showinventory || ent->client->showhelp || ent->client->showscores)
                        {
                                ent->client->showinventory = false;
                                ent->client->showhelp = false;
                                ent->client->showscores = false;
                                ent->client->pers.grabbing = 0;
                                ent->corpse = NULL;
                        }
                        // Dirty

                        if (attack2 && (FRAME_FIRE_LAST2 == -1))
                        {
                                if (fire2)
                                        fire2 (ent);
                        }
                        else if (attack && attack2 && (old_ammo > 0) && (old_ammo2 > 0))
                        {
                                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                if (fire2)
                                {
                                        if (ent->client->pers.weapon_handling && fire3)
                                                ent->client->weaponstate = WEAPON_FIRING3;
                                        else
                                        {
                                                if (FRAME_FIRE_LAST2)
                                                        ent->client->ps.gunframe = FRAME_FIRE_FIRST2;
                                                ent->client->weaponstate = WEAPON_FIRING2;
                                        }
                                }
                                else
                                        ent->client->weaponstate = WEAPON_FIRING;
                        }
                        else if (attack2 && (old_ammo2 > 0))
                        {
                                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                if (fire2)
                                {
                                        if (FRAME_FIRE_LAST2)
                                                ent->client->ps.gunframe = FRAME_FIRE_FIRST2;
                                        ent->client->weaponstate = WEAPON_FIRING2;
                                }
                                else
                                        ent->client->weaponstate = WEAPON_FIRING;
                        }
                        else if (attack && (old_ammo > 0))
                        {
                                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                ent->client->weaponstate = WEAPON_FIRING;
                        }
                        else
                                no_ammo = true;

                        // Special check for dual weapons.
                        if (no_ammo && ent->client->pers.weapon_handling && (ent->client->pers.weapon == ent->client->pers.weapon2) && (ent->client->pers.inventory[ent->client->ammo_index] < 1))
                        {
                                // If both out, continue...
                                if ((old_ammo < 1) && (old_ammo2 < 1))
                                        no_ammo = true;
                                else if (attack && (old_ammo2 > 0))
                                {
                                        no_ammo = false;
                                        ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                        ent->client->weaponstate = WEAPON_FIRING;
                                }
                                else if (attack2 && (old_ammo > 0))
                                {
                                        no_ammo = false;
                                        if (fire2)
                                        {
                                                ent->client->ps.gunframe = FRAME_FIRE_FIRST2;
                                                ent->client->weaponstate = WEAPON_FIRING2;
                                        }
                                        else
                                        {
                                                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                                ent->client->weaponstate = WEAPON_FIRING;
                                        }
                                }
                        }

                        if (no_ammo == true)
			{
				if (level.time >= ent->pain_debounce_time)
				{
					ent->pain_debounce_time = level.time + 1;
                                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				}

                                if (attack && (old_ammo < 1))
                                {
                                        if ((ent->client->pers.inventory[ent->client->ammo_index] >= 0) || (NoAmmoCheckAlternate(ent)))
                                                ent->client->weaponstate = WEAPON_RELOADING;
                                        else if (ent->client->pers.weapon_handling && (ent->client->pers.offhand->quantity > 0))
                                                SwitchGuns(ent);
                                        return;
                                }
                                else if (attack2 && (old_ammo2 < 1))
                                {
                                        if (ent->client->pers.weapon2)
                                        {
                                                if ((ent->client->pers.inventory[ent->client->ammo_index2] >= 0) || (NoAmmoCheckAlternate2(ent)))
                                                        ent->client->weaponstate = WEAPON_RELOADING2;
                                                else if (ent->client->pers.weapon_handling)
                                                        Drop_CurrentWeapon (ent, 2);
                                                return;
                                        }
                                        else if ((ent->client->pers.inventory[ent->client->ammo_index] >= 0) || (NoAmmoCheckAlternate(ent)))
                                        {
                                                ent->client->weaponstate = WEAPON_RELOADING;
                                                return;
                                        }
                                }
                        }
                        else
                        {
                                // start the animation
                                ent->client->anim_priority = ANIM_ATTACK;
                                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                                {
                                        ent->s.frame = FRAME_crattak1-1;
                                        ent->client->anim_end = FRAME_crattak9;
                                }
                                else
                                {
                                        ent->s.frame = FRAME_attack1-1;
                                        ent->client->anim_end = FRAME_attack8;
                                }
                        }
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}
        
        if (ent->client->weaponstate == WEAPON_FIRING)
	{
                if (ent->client->pers.weapon_handling)
                        fire(ent);
                else
                {
                        for (n = 0; fire_frames[n]; n++)
                        {
                                if (ent->client->ps.gunframe == fire_frames[n])
                                {
                                        fire (ent);
                                        break;
                                }
                        }

                        if (!fire_frames[n])
                                ent->client->ps.gunframe++;

                        if ((ent->client->ps.gunframe == FRAME_RELOAD_FIRST2) || (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1))
                        {
                                ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                                ent->client->weaponstate = WEAPON_READY;
                        }
                }
                SetAmmoHud (ent);
                return;
	}

        if (ent->client->weaponstate == WEAPON_FIRING2)
	{
                if (ent->client->pers.weapon_handling)
                        fire2(ent);
                else
                {
                        for (n = 0; fire_frames2[n]; n++)
                        {
                                if (ent->client->ps.gunframe == fire_frames2[n])
                                {
                                        fire2 (ent);
                                        break;
                                }
                        }

                        if (!fire_frames2[n])
                                ent->client->ps.gunframe++;

                        if ((ent->client->ps.gunframe == FRAME_RELOAD_FIRST2) || (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1))
                        {
                                ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                                ent->client->weaponstate = WEAPON_READY;
                        }
                }
                SetAmmoHud (ent);
                return;
	}

        if (ent->client->weaponstate == WEAPON_FIRING3)
        {
                fire3 (ent);
                SetAmmoHud (ent);
        }
}
// GRIM - Told ya it was messy and hacky and shit...heh heh

// GRIM
/*
================
Cmd_Weapon_OffHand - Object/Weapon in off (usually left) hand.
================
*/
void Cmd_Weapon_OffHand (edict_t *ent)
{
        gitem_t         *ammo;
        weapons_t       *wep;
        int             i;

        // Must be idle or already reloading...
        if ((ent->client->weaponstate != WEAPON_RELOADING2)
         && (ent->client->weaponstate != WEAPON_READY))
                return;         

        if (ent->client->pers.weapon_handling == NULL)
        {
                if (ent->client->pers.use_second)
                {
                        ent->client->pers.use_second = false;
                        gi.cprintf (ent, PRINT_HIGH, "Don't use a second weapon.\n");
                }
                else
                {
                        i = ArmWounds(ent);

                        if (i > 6)
                        {
                                gi.cprintf (ent, PRINT_HIGH, "Arm too damaged!\n");
                                return;
                        }
                        ent->client->pers.use_second = true;
                        gi.cprintf (ent, PRINT_HIGH, "Use a second weapon.\n");
                }
                return;
        }

        if (ent->client->pers.weapon2 == NULL)
                return;

        // Err... this weapon doesn't reload fuckhead =]
        if (!(ent->client->pers.weapon2->flags & IT_RELOAD))
                return;

        wep = ent->client->pers.offhand;
        ammo = FindItem(wep->ammo);
        if (wep->quantity >= ent->client->pers.weapon2->quantity)
                return;

        if ((ent->client->pers.inventory[ITEM_INDEX(ammo)] <= 0) && (!(ammo->flags & IT_CLIP))) 
                return;

        // This may still be needed...
        // Dirty - yeah, especially if I add a manual reloading flag
        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ammo->flags & IT_CLIP) 
                        return;
                if (ent->client->ps.gunframe >= ent->client->machinegun_shots)
                        ent->client->ps.gunframe = ent->client->continuous_shots;
                return;
        }
        else
                ent->client->weaponstate = WEAPON_RELOADING2;
}

/*
================
Cmd_Weapon_GoodHand - Object/Weapon in good (usually right) hand.
================
*/
void Cmd_Weapon_GoodHand (edict_t *ent)
{
        gitem_t         *ammo;
        weapons_t       *wep;

        // Must be idle or already reloading...
        if ((ent->client->weaponstate != WEAPON_RELOADING)
         && (ent->client->weaponstate != WEAPON_READY))
                return;         

        if (ent->client->pers.weapon == NULL)
                return; // Dead?

        // Err... this weapon doesn't reload fuckhead =]
        if (!(ent->client->pers.weapon->flags & IT_RELOAD))
                return;

        wep = ent->client->pers.goodhand;
        ammo = FindItem(wep->ammo);

        if (wep->quantity >= ent->client->pers.weapon->quantity)
                return;
        if ((ent->client->pers.inventory[ITEM_INDEX(ammo)] <= 0) && (!(ammo->flags & IT_CLIP))) 
                return;

        // This may still be needed...
        // Dirty - yeah, especially if I add a manual reloading flag
        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ammo->flags & IT_CLIP) 
                        return;

                if (ent->client->ps.gunframe >= ent->client->machinegun_shots)
                        ent->client->ps.gunframe = ent->client->continuous_shots;
                return;
        }
        else
                ent->client->weaponstate = WEAPON_RELOADING;
}

/*
================
Cmd_Weapon_Reload - Generic reload command
================
*/
void Cmd_Weapon_Reload (edict_t *ent)
{
        if (!ent->client->pers.weapon)
                return;

        if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
        {
                if (ent->client->pers.goodhand->quantity <= ent->client->pers.offhand->quantity)
                        Cmd_Weapon_GoodHand (ent);
                else if (ent->client->pers.offhand->quantity < ent->client->pers.goodhand->quantity)
                        Cmd_Weapon_OffHand (ent);
        }
        else
                Cmd_Weapon_GoodHand (ent);
}
// GRIM

// GRIM
/*
================
UnderWaterCheck
================
*/
qboolean UnderWaterCheck (vec3_t point)
{
        if (gi.pointcontents (point) & MASK_WATER)
                return true;
        return false;
}
// GRIM

/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

// GRIM
// Dirty
void weapon_grenade_fire (edict_t *ent, qboolean left, int speed)
// Dirty
{
        vec3_t  forward, right, start, offset;
        float   radius;
        int     type, timer;
        int     damage = 240;
        gitem_t *ammo;

        radius = 440;

	if (is_quad)
		damage *= 4;

        if (left && ent->client->pers.weapon2)
        {
                ammo = FindItem (ent->client->pers.weapon2->ammo);
                VectorSet (offset, 0, -8, ent->viewheight-8);
        }
        else
        {
                ammo = FindItem (ent->client->pers.weapon->ammo);
                VectorSet (offset, 0, 8, ent->viewheight-8);
        }

        // Dirty - HACK! Why is the grenade bug happening?
        if (ent->client->pers.inventory[ITEM_INDEX(ammo)] < 1)
        {
                ent->client->pulled_pin = false;
                return;
        }
        // Dirty

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        type = GREN_HAND;
        if (speed == -1)
                type += GREN_HELD;

        if (!Q_stricmp(ammo->pickup_name, "Cluster Grenades"))
                type += GREN_CLUSTER;
        else if (!Q_stricmp(ammo->pickup_name, "Flash Grenades"))
        {
                type += GREN_FLASH;
                damage = 0;
        }

        // Grenade death
        if (ent->deadflag)
                speed = 1;

        if (speed > 0)
                timer = 3;
        else
                timer = -1;

        fire_grenade2 (ent, start, forward, damage, speed, timer, radius, type);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
        {
                ent->client->pers.inventory[ITEM_INDEX(ammo)]--;
                ent->weight -= ammo->weight;
                ent->ammo_carried -= ammo->weight;
                ValidateSelectedItem (ent);
        }

        ent->client->weapon_sound = 0;
        ent->client->grenade_time = level.time + 1.0;

        // VWep animations screw up corpses
        if (ent->deadflag || ent->s.modelindex != 255)
		return;

        if (ent->health <= 0)
                return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}

        SetAmmoHud (ent);
        ent->client->pulled_pin = false;
}
// GRIM


// GRIM
// Dirty
void Weapon_Grenade (edict_t *ent)
{
        edict_t         *cantrap = NULL;
        qboolean        unfinished;

        unfinished = false;

        if (ent->health <= 0)
                return;

        // Dirty
        if (ent->client->ps.gunindex == 0)
        {
                ent->client->ps.gunindex = ent->client->old_gunindex;
                ent->client->old_gunindex = 0;
        }
        // Dirty

        if (ent->client->weaponstate == WEAPON_DROPPING)
        {
                if (ent->client->ps.gunframe < 40)
                        ChangeWeapon (ent);
                else
                {
                        if (ent->client->ps.gunframe != 73)
                                weapon_grenade_fire (ent, false, 3);
                        ent->client->ps.gunframe = 72;
                        ChangeWeapon (ent);
                        /*
                        if (ent->client->ps.gunframe < 71)

                                ent->client->ps.gunframe = 81;
                        else
                        {
                                ent->client->ps.gunframe--;
                                if (ent->client->ps.gunframe == 73)
                                        ChangeWeapon (ent);
                        }
                        */
                }
		return;
        }

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                ent->client->ps.gunframe++;
                if (ent->client->ps.gunframe == 3)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 10;
                }
                return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
                // DTEMP
                if (ent->client->newweapon2)
                {
                        if (ent->client->newweapon2 == ent->client->pers.weapon)
                        {
                                ent->client->ps.gunframe = 0;
                                ChangeWeapon2 (ent);
                                return;
                        }
                        else
                        {
                                ent->client->newweapon = ent->client->newweapon2;
                                ent->client->newweapon2 = NULL;
                        }
                }
                // DTEMP

                if (ent->client->newweapon)
                {
                        if ((ent->client->pers.weapon_handling == NULL) && (ent->client->pers.weapon == ent->client->newweapon))
                                ent->client->newweapon = NULL;
                        else
                        {
                                ent->client->weaponstate = WEAPON_DROPPING;
                                return;
                        }
                }

                // May not fire until match has actually begun...
                if (level.match_state == MATCH_START)
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE))
                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        {
                                while ((cantrap = findradius(cantrap, ent->s.origin, 36)) != NULL)
                                {
                                        if ((cantrap->item == NULL) && (Q_stricmp(cantrap->classname, "bodyque")))
                                                continue;
                                        
                                        ent->trapping = cantrap;
                                }

                                if (ent->trapping)
                                {
                                        if (ent->trapping->item)
                                        {
                                                if (ent->trapping->item->weight <= 15)
                                                {
                                                        gi.centerprintf (ent, "That item is too light!\n");
                                                        ent->trapping = NULL;
                                                }
                                                else if (ent->trapping->item->pickup == CTFPickup_Flag)
                                                {
                                                        gi.centerprintf (ent, "That item can't be trapped!\n");
                                                        ent->trapping = NULL;
                                                }
                                                else if (ent->trapping->item->pickup == Pickup_Powerup)
                                                {
                                                        gi.centerprintf (ent, "That item floats!\n");
                                                        ent->trapping = NULL;
                                                }
                                        }

                                        if (ent->trapping)
                                        {
                                               //gi.dprintf ("G & FOUND.\n");
                                                ent->client->ps.gunframe = 70;
                                                ent->delay_time = level.time + 3;
                                                ent->client->weaponstate = WEAPON_FIRING2;
                                                gi.centerprintf (ent, "Setting trap...hold still!\n");
                                        }
                                }
                        }
                        else if (ent->client->ps.gunframe < 40)
                        {
                                ent->client->weaponstate = WEAPON_FIRING2;
                                ent->client->ps.gunframe = 74;
                        }
                        else
			{
                                ent->client->weaponstate = WEAPON_FIRING2;
                                ent->client->ps.gunframe = 4;
			}
			return;
                }
                else if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ent->client->ps.gunframe < 40)
			{
				ent->client->weaponstate = WEAPON_FIRING;
                                ent->client->ps.gunframe = 74;
			}
                        else
			{
				ent->client->weaponstate = WEAPON_FIRING;
                                ent->client->ps.gunframe = 4;
			}
			return;
		}

                if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 39)
                 || (ent->client->ps.gunframe == 49) || (ent->client->ps.gunframe == 69))
		{
			if (rand()&15)
				return;
		}

                if (ent->client->ps.gunframe >= 40)
                {
                        if (ent->client->ps.gunframe >= 69)
                                ent->client->ps.gunframe = 40;
                }
                else if (ent->client->ps.gunframe >= 39)
                        ent->client->ps.gunframe = 10;
                ent->client->ps.gunframe++;
		return;
	}

        // Setting a grenade trap.
        // The more traps set on an object, the more risky it is...
        if ((ent->client->weaponstate == WEAPON_FIRING2) && (ent->client->ps.gunframe >= 70) && (ent->client->ps.gunframe < 74))
        {
                ent->client->pulled_pin = true;

                if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                        unfinished = true;

                if ((ent->client->newweapon) || (ent->velocity[0]) || (ent->velocity[1]))
                        unfinished = true;

                if ((ent->delay_time == (level.time + 2)) || (ent->delay_time == (level.time + 1))
                   || (ent->delay_time == (level.time + 1.5)))
                        ent->client->ps.gunframe = 71;

                if (ent->client->ps.gunframe == 71)
                {
                        if (random() < (0.99 - (ent->trapping->kills_in_a_row*0.06)))
                                ent->client->ps.gunframe = 70;
                        else
                                ent->client->ps.gunframe = 72;
                }

                if ((ent->delay_time > level.time) && unfinished)
                {
                        // If less then a moment left, bad news...
                        ent->delay_time = level.time;
                        ent->client->ps.gunframe = 73;
                        if (ent->delay_time < (level.time + 0.4))
                                ent->client->ps.gunframe = 72;
                        else
                                gi.centerprintf (ent, "Action unfinised\n");
                }

                if (ent->client->ps.gunframe == 72)
                {
                        gi.centerprintf (ent, "Oops...\n");
                        weapon_grenade_fire (ent, false, -1);
                        ent->client->ps.gunframe = 73;
                        ent->delay_time = level.time;
                }

                if ((ent->delay_time < level.time) && ent->trapping)
                {
                       //gi.dprintf ("G & FIN.\n");
                        GrenadeTrap(ent, FindItem(ent->client->pers.goodhand->ammo));
                        gi.centerprintf (ent, "Trap set...don't pick it up!\n");
                        ent->client->pulled_pin = false;
                        SetAmmoHud (ent);
                        ent->client->ps.gunframe = 73;
                }

                if (ent->client->ps.gunframe == 73)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->trapping = NULL;
                        if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                                SetHands (ent);
                                //ent->client->newweapon = FindItem("Hands");
                        else
                                ent->client->ps.gunframe = 10;
                }
        }
        else if ((ent->client->weaponstate == WEAPON_FIRING) || (ent->client->weaponstate == WEAPON_FIRING2))
	{
                if (ent->client->ps.gunframe >= 74)
                {
                        ent->client->ps.gunframe++;
                        if (ent->client->ps.gunframe == 77)
                        {
                                gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/grenade.wav"), 1, ATTN_NORM, 0);
                                ent->client->pulled_pin = true;
                        }
                        else if (ent->client->ps.gunframe == 81)
                        {
                                ent->client->weaponstate = WEAPON_READY;
                                ent->client->ps.gunframe = 40;
                        }
                        return;
                }

                if (ent->client->ps.gunframe == 7)
		{
                        if ((ent->client->buttons & BUTTON_ATTACK)
                         || (ent->client->buttons & BUTTON_USE))
				return;
		}

                if (ent->client->ps.gunframe == 8)
		{
                        if (ent->client->weaponstate == WEAPON_FIRING2)
                                weapon_grenade_fire (ent, false, 300);
                        else
                                weapon_grenade_fire (ent, false, 700);
		}

		ent->client->ps.gunframe++;

                if (ent->client->ps.gunframe == 9)
		{
			ent->client->grenade_time = 0;
                        ent->client->weaponstate = WEAPON_ACTIVATING;

                        if (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon->pickup_name))] < 1)
                        {
                                SetHands (ent);
                                //ent->client->newweapon = FindItem("Hands");
                                //ent->client->weaponstate = WEAPON_DROPPING;
                        }
                        else
                                ent->client->ps.gunframe = 0;
		}
	}
}

void Weapon_DualGrenades (edict_t *ent)
{
        if (ent->health <= 0)
                return;

        // Dirty
        if (ent->client->ps.gunindex == 0)
        {
                ent->client->ps.gunindex = ent->client->old_gunindex;
                ent->client->old_gunindex = 0;
        }
        // Dirty

        if (ent->client->ps.gunframe == 18)
        {
                ent->client->grenade_time = 0;
                ent->client->weaponstate = WEAPON_ACTIVATING;

                if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                {
                        ent->client->pers.inventory[ent->client->ammo_index] = 0;
                        SetHands (ent);
                }
                else if (ent->client->pers.inventory[ent->client->ammo_index] < 2)
                {
                        ent->client->newweapon = ent->client->pers.weapon;
                        ent->client->weaponstate = WEAPON_DROPPING;
                }
                else
                        ent->client->ps.gunframe = 0;
        }

        if (ent->client->weaponstate == WEAPON_DROPPING)
        {
                if (ent->client->ps.gunframe == 15)
                {
                        if (ent->client->newweapon == ent->client->pers.weapon)
                                ChangeWeapon (ent);
                        else
                                SetHands (ent);
                }
                else
                {
                        if (ent->client->ps.gunframe == 16)
                        {
                                weapon_grenade_fire (ent, true, 1);
                                weapon_grenade_fire (ent, false, 1);
                        }
                        ent->client->ps.gunframe = 17;
                        if (ent->client->newweapon == ent->client->pers.weapon)
                                ChangeWeapon (ent);
                        else
                                SetHands (ent);
                }
		return;
        }

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                ent->client->ammo_index2 = -1;
                // Dirty - HACK! Stop extra grenades
                if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                {
                        ent->client->pers.weapon_handling = NULL;
                        ent->client->pers.weapon2 = NULL;
                        ent->client->weaponstate = WEAPON_READY;
                        return;
                }
                // GRIM

                ent->client->ps.gunframe++;
                if (ent->client->ps.gunframe >= 3)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 15;
                }
                return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
                if (ent->client->newweapon)
                {
                        ent->client->weaponstate = WEAPON_DROPPING;
                        return;
                }

                // May not fire until match has actually begun...
                if (level.match_state == MATCH_START)
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE))
                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        if (ent->client->ps.gunframe == 15)
                        {
                                ent->client->weaponstate = WEAPON_FIRING2;
                                ent->client->ps.gunframe = 21;
                        }
                        else
			{
                                ent->client->weaponstate = WEAPON_FIRING2;
                                ent->client->ps.gunframe = 10;
			}
			return;
                }
                else if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ent->client->ps.gunframe == 15)
                        {
                                ent->client->weaponstate = WEAPON_FIRING;
                                ent->client->ps.gunframe = 21;
                        }
                        else
			{
                                ent->client->weaponstate = WEAPON_FIRING;
                                ent->client->ps.gunframe = 4;
			}
			return;
		}
                return;
	}

        if (ent->client->ps.gunframe >= 21)
        {
                ent->client->ps.gunframe++;
                if (ent->client->ps.gunframe == 25)
                {
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/grenade.wav"), 1, ATTN_NORM, 0);
                        ent->client->pulled_pin = true;
                }
                else if (ent->client->ps.gunframe == 33)
                {
                        gi.sound(ent, CHAN_OFFHAND, gi.soundindex("misc/grenade.wav"), 1, ATTN_NORM, 0);
                        ent->client->pulled_pin = true;
                }
                else if (ent->client->ps.gunframe == 38)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 16;
                }
                return;
        }

        if (ent->client->weaponstate == WEAPON_FIRING2)
	{
                if (ent->client->ps.gunframe == 13)
                {
                        weapon_grenade_fire (ent, false, 1);
                        weapon_grenade_fire (ent, true, 1);
                }

                if (ent->client->ps.gunframe == 14)
                        ent->client->ps.gunframe = 18;
                else
                        ent->client->ps.gunframe++;
                return;
	}

        if (ent->client->weaponstate == WEAPON_FIRING)
	{
                if (ent->client->ps.gunframe == 7)
		{
                        if ((ent->client->buttons & BUTTON_ATTACK)
                         || (ent->client->buttons & BUTTON_USE))
				return;
		}

                if (ent->client->ps.gunframe == 8)
                {
                        weapon_grenade_fire (ent, false, 400);
                        weapon_grenade_fire (ent, true, 400);
                }

                if (ent->client->ps.gunframe == 9)
                        ent->client->ps.gunframe = 18;
                else
                        ent->client->ps.gunframe++;
	}
}
// Dirty
// GRIM

// GRIM - All weapons from now on are heavily edited or completely changed
/*
======================================================================

RIFLE / PISTOL / MACHINEGUN / CHAINGUN

======================================================================
*/

int ContEffect (edict_t *ent)
{
        if (ent->client->continuous_shots)
                return ent->client->continuous_shots;
        else
                return -1;
}

// Dirty
void SwitchMode (edict_t *ent)
{
        ent->client->weaponstate = WEAPON_READY;
        ent->client->ps.gunframe = 13;

        if (ent->attack_time > level.time)
                return;

        ent->attack_time = level.time + 0.3;

        gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/click.wav"), 0.4, ATTN_NORM, 0);

        if (ent->client->pers.goodhand->flags & W_ALTERNATE1)
        {
                ent->client->pers.goodhand->flags &= ~W_ALTERNATE1;
                gi.cprintf (ent, PRINT_HIGH, "%s switched to full auto\n", ent->client->pers.goodhand->pickup_name);
        }
        else
        {
                ent->client->pers.goodhand->flags |= W_ALTERNATE1;
                gi.cprintf (ent, PRINT_HIGH, "%s switched to 3 shot burst\n", ent->client->pers.goodhand->pickup_name);
        }
}

        // Dirty temp
        //VectorCopy (ent->f_offset, offset);
        // Dirty temp

void Rifle_Fire (edict_t *ent, qboolean left, int f_frame, int s_frame, int i_frame)
{
        vec3_t          start, forward, right, offset;
        int             i, effect;
        int             type = (TOD_LEAD | TOD_HV);
        qboolean        no_ammo = false;
        int             buttonMask = 0;
        weapons_t       *wep;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

        if (left)
        {
                if (!buttonMask)
                        buttonMask = BUTTON_USE;
                VectorSet(offset, 0, -2,  ent->viewheight-1);
                wep = ent->client->pers.offhand;
        }
        else
        {
                if (!buttonMask)
                        buttonMask = BUTTON_ATTACK;
                VectorSet(offset, 0, 2,  ent->viewheight-1);
                wep = ent->client->pers.goodhand;
        }

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        //if (UnderWaterCheck(start) && (wep->flags & W_WATER_JAMMED))
        if ((ent->waterlevel == 3) && (wep->flags & W_WATER_JAMMED))
        {
                ent->client->ps.gunframe = i_frame;
                ent->client->weaponstate = WEAPON_READY;
                return;
        }
        else if (wep->flags & W_WATER_JAMMED)
                wep->flags &= ~W_WATER_JAMMED;

        if (!(ent->client->buttons & BUTTON_ATTACK))
        {
                if ((wep->flags & W_ALTERNATE1) && (ent->client->continuous_shots < 9) && (wep->quantity > 0))
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else
                {
                        //if (UnderWaterCheck(start) && ent->client->continuous_shots > 0)
                        if ((ent->waterlevel == 3) && ent->client->continuous_shots > 0)
                                wep->flags |= W_WATER_JAMMED;
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = i_frame;
                        return;
                }
        }
        else if ((wep->flags & W_ALTERNATE1) && ((ent->client->continuous_shots >= 9) || (wep->quantity < 1)))
        {
                ent->client->weaponstate = WEAPON_FIRING;
                ent->client->ps.gunframe = s_frame;
                return;
        }

        if (wep->quantity <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = i_frame;
                return;
	}

        if (s_frame)
        {
                if (ent->client->ps.gunframe == s_frame)
                        ent->client->ps.gunframe = f_frame;
                else            
                        ent->client->ps.gunframe = s_frame;
        }

        fire_baselead (ent, start, forward, 384, 1536, 3072, 80, 70, type, 80, 120, MOD_RIFLE, 5);
        MuzzleFlash (ent, "weapons/m4a1fire.wav", false);

        effect = ContEffect(ent);

        if (effect >= 9)
        {
                if (ent->groundentity && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                {
                        if (forward[2] > -45 && forward[2] < 45)
                        {
                                ent->velocity[2] += 1;
                                forward[2] = 0;
                                VectorScale (forward, -50, ent->velocity);
                        }
                }
                ent->client->weapon_forward_set = 100;
        }

        if (ent->client->continuous_shots < 12)
                ent->client->continuous_shots += 3;
                                        
        for (i=1 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = (crandom() * 0.30) * effect;
                ent->client->kick_angles[i] = (crandom() * 0.15) * effect;
        }

        ent->client->anim_priority = ANIM_ATTACK;       
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FRAME_crattak1 - 1;
                ent->client->anim_end = FRAME_crattak9;
        }
        else
        {
                ent->s.frame = FRAME_attack1 - 1;
                ent->client->anim_end = FRAME_attack8;
        }

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                wep->quantity--;
}

void SingleRifle_Fire (edict_t *ent)
{
        Rifle_Fire (ent, false, 11, 12, 13);
}

void Weapon_Rifle (edict_t *ent)
{
        static int      pause_frames[]  = {41, 0};
        static int      fire_frames[]   = {11, 12, 0};

        if (ent->client->ps.gunframe == 4)
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1slide.wav"), 1, ATTN_NORM, 0);

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 53)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 58)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1in.wav"), 1, ATTN_NORM, 0);
        }
        if ((ent->client->ps.gunframe < 5) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Sight"))] > 0))
                ent->client->pers.specials |= SI_LASER_SIGHT;

        if ((ent->client->weaponstate == WEAPON_READY) || (ent->client->weaponstate == WEAPON_FIRING))
                TracenPlace (ent, 2, ent->viewheight-1);

        Weapon_Generic (ent, 10, 12, 39, 44, 63, 53, 58, pause_frames, fire_frames, SingleRifle_Fire,
        -1, 0, 0, 0, NULL, SwitchMode, NULL);
}


void Dual_M4s_Fire (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        if ((wep->quantity < 1) || ((wep->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 68) || (ent->client->ps.gunframe > 69))
                        ent->client->ps.gunframe = 68;
                Rifle_Fire (ent, true, 68, 69, 15);
        }
        else if ((wep2->quantity < 1) || ((wep2->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 13) || (ent->client->ps.gunframe > 14))
                        ent->client->ps.gunframe = 13;
                Rifle_Fire (ent, false, 13, 14, 15);
        }
        else if (wep->quantity && wep2->quantity)
        {
                if (ent->client->ps.gunframe < 101)
                        ent->client->ps.gunframe = 101;

                if (ent->client->weaponstate == WEAPON_FIRING2)
                {
                        Rifle_Fire (ent, false, 0, 0, 15);
                        Rifle_Fire (ent, true, 101, 102, 15);
                }
                else
                {
                        Rifle_Fire (ent, false, 101, 102, 15);
                        Rifle_Fire (ent, true, 0, 0, 15);
                }

                // crawl along
                if (ent->client->continuous_shots > 4)
                {
                        ent->client->weapon_forward_set = 50;
                        ent->client->weapon_side_set = 100;
                }
        }
        else
        {
                ent->client->ps.gunframe = 15;
                ent->client->weaponstate = WEAPON_READY;
        }
}


void Weapon_DualM4s (edict_t *ent)
{
        static int      pause_frames[]  = {28, 0};

        // only full auto with dual weapons
        if (ent->client->weaponstate == WEAPON_ACTIVATING)
        {
                ent->client->pers.goodhand->flags &= ~W_ALTERNATE1;
                ent->client->pers.offhand->flags &= ~W_ALTERNATE1;
        }

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe == 83)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 88)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1in.wav"), 1, ATTN_NORM, 0);
        }

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 50)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 55)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1in.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 12, 14, 23, 36, 67, 50, 55, pause_frames, NULL, Dual_M4s_Fire,
        69, 100, 83, 88, NULL, Dual_M4s_Fire, NULL);
}



// Dirty


// Dirty
void FireMachinegun (edict_t *ent, qboolean left, int f_frame, int s_frame, int i_frame)
{
        vec3_t          start, forward, right;
        vec3_t          v, angles, offset;
        qboolean        no_ammo = false;
        int             i = 0;
        int             buttonMask = 0;
        int             type = TOD_LEAD;
        weapons_t       *wep;
        qboolean        twin_alt = false;

        VectorCopy (ent->client->v_angle, angles);
        AngleVectors (angles, forward, right, NULL);

        // Dual machineguns hack =P
        if (ent->client->pers.weapon_handling && (ent->client->pers.weapon == ent->client->pers.weapon2))
        {
                if (ent->client->weaponstate == WEAPON_FIRING2)
                {
                        buttonMask = BUTTON_USE;
                        twin_alt = true;
                }
                else
                        buttonMask = BUTTON_ATTACK;
        }

        if (left)
        {
                if (!buttonMask)
                        buttonMask = BUTTON_USE;
                if (twin_alt)
                        VectorSet(offset, 0, -1, ent->viewheight-5);
                else
                        VectorSet(offset, 0, -3, ent->viewheight-3);
                wep = ent->client->pers.offhand;
        }
        else
        {
                if (!buttonMask)
                        buttonMask = BUTTON_ATTACK;
                if (twin_alt)
                        VectorSet(offset, 0, 1, ent->viewheight-5);
                else
                        VectorSet(offset, 0, 3, ent->viewheight-3);
                wep = ent->client->pers.goodhand;
        }

        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (!(ent->client->buttons & buttonMask))
        {
                if ((wep->flags & W_ALTERNATE1) && (ent->client->continuous_shots < 3) && (wep->quantity > 0))
                        ent->client->ps.gunframe = (f_frame - 1);
                else
                {
                        //if (UnderWaterCheck(start) && ent->client->continuous_shots > 0)
                        if ((ent->waterlevel == 3) && ent->client->continuous_shots > 0)
                                wep->flags |= W_WATER_JAMMED;
                        ent->client->ps.gunframe = i_frame;
                        ent->client->weaponstate = WEAPON_READY;
                        return;
                }
        }
        else if ((wep->flags & W_ALTERNATE1) && ((ent->client->continuous_shots >= 3) || (wep->quantity < 1)))
        {
                ent->client->weaponstate = WEAPON_FIRING;
                ent->client->ps.gunframe = i_frame;
                return;
        }

        if (wep->quantity <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = i_frame;
                return;
	}

        //if (UnderWaterCheck(start) && (wep->flags & W_WATER_JAMMED))
        if ((ent->waterlevel == 3) && (wep->flags & W_WATER_JAMMED))
        {
                ent->client->ps.gunframe = i_frame;
                ent->client->weaponstate = WEAPON_READY;
                return;
        }
        else if (wep->flags & W_WATER_JAMMED)
                wep->flags &= ~W_WATER_JAMMED;

        if (s_frame)
        {
                if (ent->client->ps.gunframe == s_frame)
                        ent->client->ps.gunframe = f_frame;
                else            
                        ent->client->ps.gunframe = s_frame;
        }

        for (i=1 ; i<3 ; i++)
        {
                ent->client->kick_angles[i] = (crandom() * 0.25);
                ent->client->kick_origin[i] = (crandom() * 0.5);
        }

        //PlayerNoise(ent, start, PNOISE_WEAPON);
        if (twin_alt)
        {
                v[PITCH] = ent->client->v_angle[PITCH];
                v[ROLL]  = ent->client->v_angle[ROLL];
                if (left)
                        v[YAW] = ent->client->v_angle[YAW] - 5;
                else
                        v[YAW] = ent->client->v_angle[YAW] + 5;
                AngleVectors (v, forward, NULL, NULL);
                fire_baselead (ent, start, forward, 128, 640, 1536, 95, 90, type, 48, 20, MOD_MACHINEGUN, 2);
        }
        else
                fire_baselead (ent, start, forward, 128, 640, 1536, 95, 90, type, 48, 20, MOD_MACHINEGUN, 2);

        //if (ent->client->pers.specials & SI_SILENCER)
        if ((ent->client->pers.specials & SI_SILENCER) && (!ent->client->pers.weapon2))
                gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/silencer.wav"), 1, ATTN_NORM, 0);
        else
                MuzzleFlash (ent, "weapons/mp5fire1.wav", left);

        if (ent->client->continuous_shots < 10)
                ent->client->continuous_shots++;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

        if (!((int)dmflags->value & DF_INFINITE_AMMO))
                wep->quantity--;
}

void MachinegunFireRight (edict_t *ent)
{
        FireMachinegun (ent, false, 11, 12, 13);
}

void Weapon_Machinegun (edict_t *ent)
{
        static int      pause_frames[]  = {49, 0};
        static int      fire_frames[]   = {11, 12, 0};

        if (ent->client->ps.gunframe == 5)
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5slide.wav"), 1, ATTN_NORM, 0);

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 56)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 59)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 64)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5slide.wav"), 1, ATTN_NORM, 0);
        }

        if ((ent->client->ps.gunframe < 5) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Sight"))] > 0))
                ent->client->pers.specials |= SI_LASER_SIGHT;

        if ((ent->client->weaponstate == WEAPON_READY) || (ent->client->weaponstate == WEAPON_FIRING))
                TracenPlace (ent, 3, ent->viewheight-3);

        Weapon_Generic (ent, 10, 12, 47, 51, 69, 56, 59, pause_frames, fire_frames, MachinegunFireRight,
        -1, 0, 0, 0, NULL, SwitchMode, NULL);
}
// Dirty


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

// Dirty
void FireShotgun (edict_t *ent, qboolean left, int i_first)
{
        vec3_t          start, forward, right, offset;
        int             type = (TOD_LEAD | TOD_PELLET);
        weapons_t       *wep;

        if (left)
        {
                VectorSet(offset, 0, -7, ent->viewheight-2);
                wep = ent->client->pers.offhand;
        }
        else
        {
                VectorSet(offset, 0, 7, ent->viewheight-2);
                wep = ent->client->pers.goodhand;
        }

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        //if (UnderWaterCheck(start) && (wep->flags & W_WATER_JAMMED))
        if ((ent->waterlevel == 3) && (wep->flags & W_WATER_JAMMED))
        {
                ent->client->ps.gunframe = i_first;
                ent->client->weaponstate = WEAPON_READY;
                return;
        }
        else if (wep->flags & W_WATER_JAMMED)
                wep->flags &= ~W_WATER_JAMMED;

        //if (!Q_stricmp(wep->ammo, "Exp Shotgun Clip"))
        //        type |= TOD_RAD_EXPLODE;

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

        //PlayerNoise(ent, start, PNOISE_WEAPON);
        fire_baselead (ent, start, forward, 64, 192, 768, 750, 550, type, 12, 18, MOD_SHOTGUN, 13);
        MuzzleFlash (ent, "weapons/shotgf1b.wav", left);

        if (ent->groundentity)
        {
                ent->velocity[2] += 1;
                forward[2] = 0;
                VectorScale (forward, -50, ent->velocity);
        }

        //if (UnderWaterCheck(start))
        if (ent->waterlevel == 3)
                wep->flags |= W_WATER_JAMMED;

        PlayerNoise(ent, start, PNOISE_WEAPON);
        if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                wep->quantity--;

        if (ent->client->weaponstate != WEAPON_FIRING3)
                ent->client->ps.gunframe++;
}

void ShotgunFireRight (edict_t *ent)
{
        FireShotgun (ent, false, 16);
}

void Weapon_Shotgun (edict_t *ent)
{
        static int      pause_frames[]  = {18, 0};
        static int      fire_frames[]   = {8, 0};

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                //if (ent->client->pers.goodhand->quantity == 0)
                //        ent->client->pers.goodhand->flags |= W_ALTERNATE1;

                if (ent->client->ps.gunframe < 45)
                {
                        ent->client->continuous_shots = 45;
                        ent->client->machinegun_shots = 50;
                }

                if (ent->client->ps.gunframe == 47)
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/m3in.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 52)
                {
                        //if (ent->client->pers.goodhand->flags & W_ALTERNATE1)
                        //        ent->client->ps.gunframe = 10;
                        //else
                                ent->client->pers.goodhand->flags &= ~W_UNLOADED;
                }
        }

        if ((ent->client->weaponstate == WEAPON_READY)
          && (ent->client->pers.goodhand->flags & W_UNLOADED)
           && (ent->client->ps.gunframe > 15))
                ent->client->ps.gunframe = 10;

        // This is for shoot and drop situations... no reload next round etc
        if (ent->client->ps.gunframe == 10)
        {
                ent->client->pers.goodhand->flags |= W_UNLOADED;
                ent->client->weaponstate = WEAPON_READY;
        }

        if (ent->client->ps.gunframe == 13)
        {
                ent->client->pers.goodhand->flags &= ~W_UNLOADED;
                gi.sound(ent, CHAN_OFFHAND, gi.soundindex("weapons/shotgcock.wav"), 1, ATTN_NORM, 0);
        }

        if ((ent->client->weaponstate == WEAPON_READY) || ((ent->client->weaponstate == WEAPON_FIRING) && (ent->client->ps.gunframe < 10)))
                TracenPlace (ent, 7, ent->viewheight-2);

        Weapon_Generic (ent, 7, 15, 35, 41, 52, 45, 48, pause_frames, fire_frames, ShotgunFireRight,
        0, 0, 0, 0, NULL, NULL, NULL);
}

void DualShotgunsFireRight (edict_t *ent)
{
        if (ent->client->ps.gunframe == 15)
        {
                if (ent->client->buttons & BUTTON_ATTACK)
                        ent->client->weaponstate = WEAPON_FIRING3;
                else
                        ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe++;
        }
        else if (ent->client->ps.gunframe == 7)
                FireShotgun (ent, false, 35);
        else
                ent->client->ps.gunframe++;
}

void DualShotgunsFireLeft (edict_t *ent)
{
        if (ent->client->ps.gunframe == 75)
        {
                if (ent->client->buttons & BUTTON_USE)
                        ent->client->weaponstate = WEAPON_FIRING3;
                else
                        ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe++;
        }
        else if (ent->client->ps.gunframe == 67)
                FireShotgun (ent, true, 35);
        else
                ent->client->ps.gunframe++;
}

void DualShotgunsFireBoth (edict_t *ent)
{
        if ((ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 94))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 35;
        }
        else if (ent->client->ps.gunframe == 124)
        {
                if (ent->client->buttons & BUTTON_USE)
                {
                        ent->client->weaponstate = WEAPON_FIRING3;
                        ent->client->ps.gunframe = 76;
                }
                else if (ent->client->buttons & BUTTON_ATTACK)
                {
                        ent->client->weaponstate = WEAPON_FIRING3;
                        ent->client->ps.gunframe = 16;
                }
                else
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 35;
                }
        }
        else if (ent->client->ps.gunframe == 116)
        {
                if ((ent->client->pers.goodhand->quantity > 0) && (ent->client->pers.offhand->quantity > 0))
                {
                        FireShotgun (ent, false, 35);
                        FireShotgun (ent, true, 35);
                        ent->client->ps.gunframe++;
                }
                else if (ent->client->pers.goodhand->quantity > 0)
                {
                        FireShotgun (ent, false, 35);
                        ent->client->ps.gunframe++;
                }
                else if (ent->client->pers.offhand->quantity > 0)
                {
                        FireShotgun (ent, true, 35);
                        ent->client->ps.gunframe++;
                }
                else
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 35;
                }
        }
        else
                ent->client->ps.gunframe++;
}

void Weapon_DualShotguns (edict_t *ent)
{
        static int      pause_frames[]  = {0};
        static int      fire_frames[]   = {7, 15, 0};
        static int      fire_frames2[]   = {67, 75, 0};

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe < 58)
                {
                        ent->client->continuous_shots = 58;
                        ent->client->machinegun_shots = 63;
                }

                if (ent->client->ps.gunframe == 62)
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/m3in.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 66)
                        ent->client->pers.goodhand->flags &= ~W_UNLOADED;
        }

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe < 107)
                {
                        ent->client->continuous_shots = 107;
                        ent->client->machinegun_shots = 112;
                }

                if (ent->client->ps.gunframe == 112)
                        gi.sound(ent, CHAN_OFFHAND, gi.soundindex("weapons/m3in.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 115)
                        ent->client->pers.goodhand->flags &= ~W_UNLOADED;
        }


        Weapon_Generic (ent, 6, 34, 38, 45, 66, 59, 62, pause_frames, fire_frames, DualShotgunsFireRight,
        94, 115, 108, 112, fire_frames2, DualShotgunsFireLeft, DualShotgunsFireBoth);
}

// Dirty


// GRIM
void DualShotgunsFire (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        if (ent->client->ps.gunframe == 7)
        {
                if ((wep->quantity < 1) || ((wep->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
                {
                        ent->client->ps.gunframe = 61;
                        FireShotgun (ent, true, 13);
                }
                else if ((wep2->quantity < 1) || ((wep2->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
                        FireShotgun (ent, false, 13);
                else if (wep->quantity && wep2->quantity)
                {
                        ent->client->ps.gunframe = 91;
                        FireShotgun (ent, false, 13);
                        FireShotgun (ent, true, 0);
                }
                else
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 13;
                }
        }
        ent->client->ps.gunframe++;

        if ((ent->client->ps.gunframe == 13) || (ent->client->ps.gunframe == 67)
         || (ent->client->ps.gunframe == 98))
        {
                ent->client->ps.gunframe = 13;
                ent->client->weaponstate = WEAPON_READY;
        }
}

void DualShotgunsFireAlternate (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        if (ent->client->ps.gunframe == 61)
        {
                if ((wep->quantity < 1) || ((wep->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
                        FireShotgun (ent, true, 13);
                else if ((wep2->quantity < 1) || ((wep2->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
                {
                        ent->client->ps.gunframe = 7;
                        FireShotgun (ent, false, 13);
                }
                else if (wep->quantity >= wep2->quantity)
                {
                        ent->client->ps.gunframe = 7;
                        FireShotgun (ent, false, 13);
                }
                else if (wep->quantity < wep2->quantity)
                        FireShotgun (ent, true, 13);
        }

        ent->client->ps.gunframe++;

        if ((ent->client->ps.gunframe == 13) || (ent->client->ps.gunframe == 67)
         || (ent->client->ps.gunframe == 98))
        {
                ent->client->ps.gunframe = 13;
                ent->client->weaponstate = WEAPON_READY;
        }
}

// Dirty
void FireDoubleBarrelShotgun (edict_t *ent, qboolean left, int barrel, int s_frame)
{
        vec3_t          start, forward, right, offset, v;
        int             yaw_mod;
        int             type = (TOD_LEAD | TOD_PELLET);
        weapons_t       *wep;

        yaw_mod = 3;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	if (left)
	{
	        wep = ent->client->pers.offhand;
                VectorSet(offset, 0, -6, ent->viewheight-4);
	}
	else
	{
	        wep = ent->client->pers.goodhand;
                VectorSet(offset, 0, 6, ent->viewheight-4);
	}

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;
        
	v[PITCH] = ent->client->v_angle[PITCH];
	v[ROLL]  = ent->client->v_angle[ROLL];

        if (barrel == 1)
        {
                v[YAW]   = ent->client->v_angle[YAW] + yaw_mod;
                AngleVectors (v, forward, NULL, NULL);
                //MuzzleFlash (ent, "weapons/shotf.wav", left);
                MuzzleFlash (ent, "weapons/cannon_fire.wav", left);
                fire_baselead (ent, start, forward, 64, 192, 768, 1200, 900, type, 12, 18, MOD_ASHOTGUN, 13);
        }
        else if (barrel == 2)
        {
                v[YAW]   = ent->client->v_angle[YAW] - yaw_mod;
                AngleVectors (v, forward, NULL, NULL);
                //MuzzleFlash (ent, "weapons/shotf.wav", left);
                MuzzleFlash (ent, "weapons/cannon_fire.wav", left);
                fire_baselead (ent, start, forward, 64, 192, 768, 1200, 900, type, 12, 18, MOD_ASHOTGUN, 13);
        }
        else
        {
                v[YAW]   = ent->client->v_angle[YAW] + yaw_mod;
                AngleVectors (v, forward, NULL, NULL);
                fire_baselead (ent, start, forward, 64, 192, 768, 1400, 1100, type, 12, 18, MOD_ASHOTGUN, 13);

                v[YAW]   = ent->client->v_angle[YAW] - yaw_mod;
                AngleVectors (v, forward, NULL, NULL);
                fire_baselead (ent, start, forward, 64, 192, 768, 1400, 1100, type, 12, 18, MOD_ASHOTGUN, 13);

                MuzzleFlash (ent, "weapons/cannon_fire.wav", left);
        }

        if (ent->groundentity)
        {
                ent->velocity[2] += 1;
                forward[2] = 0;
                if (barrel)
                        VectorScale (forward, -125, ent->velocity);
                else
                        VectorScale (forward, -200, ent->velocity);
        }

        if (s_frame && barrel)
                ent->client->ps.gunframe = s_frame;

        if (!((int)dmflags->value & DF_INFINITE_AMMO))
        {
                if (barrel == 0)
                        wep->quantity -= 2;
                else        
                        wep->quantity -= 1;
        }
}

void OneBarrel (edict_t *ent);
void BothBarrels (edict_t *ent)
{
        if (ent->client->pers.goodhand->quantity < 2)
        {
                ent->client->weaponstate = WEAPON_FIRING2;
                OneBarrel (ent);
                return;
        }
        else
                FireDoubleBarrelShotgun (ent, false, 0, 12);
        ent->client->ps.gunframe++;
}

void OneBarrel (edict_t *ent)
{
        if (ent->client->ps.gunframe == 85)
        {
                if ((ent->client->buttons & BUTTON_USE) && (ent->client->pers.goodhand->quantity == 1))
                {
                        ent->client->ps.gunframe = 86;
                        FireDoubleBarrelShotgun (ent, false, 2, 0);
                }
        }
        else if (ent->client->pers.goodhand->quantity == 2)
                FireDoubleBarrelShotgun (ent, false, 1, 0);
        else
                FireDoubleBarrelShotgun (ent, false, 2, 0);
        ent->client->ps.gunframe++;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};
        static int      fire_frames2[]   = {83, 85, 0};

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 64)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/copen.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 68)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cout.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 76)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cin.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 77)
                {
                        if ((ent->client->pers.goodhand->quantity == 1)
                         && (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]))
                        {
                                ent->client->pers.goodhand->quantity = 2;
                                ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]--;
                                RemoveAmmoWeight (ent, FindItem(ent->client->pers.goodhand->ammo), 1);
                                SetAmmoHud (ent);
                        }
                }

                if (ent->client->ps.gunframe == 82)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cclose.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 6, 17, 57, 61, 82, 68, 76, pause_frames, fire_frames, BothBarrels,
        88, 0, 0, 0, fire_frames2, OneBarrel, NULL);
}

/*
================================================
Dual Sawn Offs - John Woo style.
================================================
*/
void FireTwoSawnsAlt (edict_t *ent)
{
        if ((ent->client->ps.gunframe == 9) || (ent->client->ps.gunframe == 57))
        {
                if ((ent->client->buttons & BUTTON_USE) && ((ent->client->pers.goodhand->quantity > 0) || (ent->client->pers.offhand->quantity > 0)))
                        ent->client->ps.gunframe = 7;
                else
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 10;
                        return;
                }
        }
        ent->client->machinegun_shots = -1;

        if ((ent->client->ps.gunframe == 55) || (ent->client->ps.gunframe == 7))
        {
                ent->client->weaponstate = WEAPON_FIRING3;
                if ((ent->client->pers.offhand->quantity >= ent->client->pers.goodhand->quantity) && (ent->client->pers.offhand->quantity > 0))
                {
                        ent->client->ps.gunframe = 55;
                        if (ent->client->pers.offhand->quantity == 1)
                                FireDoubleBarrelShotgun (ent, true, 2, 0);
                        else
                                FireDoubleBarrelShotgun (ent, true, 1, 0);
                }
                else if (ent->client->pers.goodhand->quantity > 0)
                {
                        ent->client->ps.gunframe = 7;
                        if (ent->client->pers.goodhand->quantity == 1)
                                FireDoubleBarrelShotgun (ent, false, 2, 0);
                        else
                                FireDoubleBarrelShotgun (ent, false, 1, 0);
                }
        }
        ent->client->ps.gunframe++;
}

void FireTwoSawns (edict_t *ent)
{
        if (ent->client->machinegun_shots < 0)
        {
                FireTwoSawnsAlt (ent);
                return;
        }

        if (ent->client->ps.gunframe == 7)
        {
                ent->client->weaponstate = WEAPON_FIRING3;
                if ((ent->client->pers.goodhand->quantity == 2) && (ent->client->pers.offhand->quantity == 2))
                {
                        ent->client->ps.gunframe = 86;
                        FireDoubleBarrelShotgun (ent, false, 0, 0);
                        FireDoubleBarrelShotgun (ent, true, 0, 0);
                }
                else if ((ent->client->pers.goodhand->quantity == 2) && (ent->client->pers.offhand->quantity == 1))
                {
                        ent->client->ps.gunframe = 108;
                        FireDoubleBarrelShotgun (ent, false, 0, 0);
                        FireDoubleBarrelShotgun (ent, true, 1, 0);
                }
                else if ((ent->client->pers.goodhand->quantity == 1) && (ent->client->pers.offhand->quantity == 2))
                {
                        ent->client->ps.gunframe = 97;
                        FireDoubleBarrelShotgun (ent, false, 1, 0);
                        FireDoubleBarrelShotgun (ent, true, 0, 0);
                }
                else if ((ent->client->pers.goodhand->quantity == 1) && (ent->client->pers.offhand->quantity == 1))
                {
                        ent->client->ps.gunframe = 86;
                        FireDoubleBarrelShotgun (ent, false, 1, 91);
                        FireDoubleBarrelShotgun (ent, true, 1, 91);
                }
                else if ((ent->client->pers.goodhand->quantity == 2) && (ent->client->pers.offhand->quantity == 0))
                {
                        ent->client->ps.gunframe = 126;
                        ent->client->machinegun_shots = 3;
                        FireDoubleBarrelShotgun (ent, false, 0, 0);
                }
                else if ((ent->client->pers.goodhand->quantity == 0) && (ent->client->pers.offhand->quantity == 2))
                {
                        ent->client->ps.gunframe = 119;
                        ent->client->machinegun_shots = 4;
                        FireDoubleBarrelShotgun (ent, true, 0, 0);
                }
                else
                        FireTwoSawnsAlt (ent);
                return;
        }
        else
                ent->client->ps.gunframe++;

        if ((ent->client->ps.gunframe == 96) || (ent->client->ps.gunframe == 107) || (ent->client->ps.gunframe == 118)
         || (ent->client->ps.gunframe == 125) || (ent->client->ps.gunframe == 132))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 10;
        }

}

void TwinShotties (edict_t *ent)
{
        if (ent->client->machinegun_shots < 0)
                FireTwoSawnsAlt (ent);
        else
                FireTwoSawns (ent);
}

void Weapon_TwinShotties (edict_t *ent)
{
        static int      pause_frames[]  = {0};

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe == 65)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/copen.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 67)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cout.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 76)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cin.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 77)
                {
                        if ((ent->client->pers.offhand->quantity == 1)
                         && (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]))
                        {
                                ent->client->pers.offhand->quantity = 2;
                                ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]--;
                                RemoveAmmoWeight (ent, FindItem(ent->client->pers.offhand->ammo), 1);
                                SetAmmoHud (ent);
                        }
                }

                if (ent->client->ps.gunframe == 82)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cclose.wav"), 1, ATTN_NORM, 0);
        }

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 34)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/copen.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 36)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cout.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 45)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cin.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 46)
                {
                        if ((ent->client->pers.goodhand->quantity == 1)
                         && (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]))
                        {
                                ent->client->pers.goodhand->quantity = 2;
                                ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]--;
                                RemoveAmmoWeight (ent, FindItem(ent->client->pers.goodhand->ammo), 1);
                                SetAmmoHud (ent);
                        }
                }

                if (ent->client->ps.gunframe == 51)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cclose.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 6, 9, 19, 26, 54, 36, 45, pause_frames, NULL, FireTwoSawns,
        57, 85, 67, 76, NULL, FireTwoSawnsAlt, TwinShotties);
}
// Dirty

/*
======================================================================

2 MACHINEGUN'S

======================================================================
*/
// GRIM
void Dual_Machineguns_Fire_Alt (edict_t *ent);

void Dual_Machineguns_Fire (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        // This is to stop ppl changing modes real quick...
        if (ent->client->ps.gunframe > 103)
        {
                Dual_Machineguns_Fire_Alt (ent);
                ent->client->weaponstate = WEAPON_FIRING2;
                return;
        }

        if ((wep->quantity < 1) || ((wep->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 74) || (ent->client->ps.gunframe > 75))
                        ent->client->ps.gunframe = 74;
                FireMachinegun (ent, true, 74, 75, 8);
        }
        else if ((wep2->quantity < 1) || ((wep2->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 6) || (ent->client->ps.gunframe > 7))
                        ent->client->ps.gunframe = 6;
                FireMachinegun (ent, false, 6, 7, 8);
        }
        else if (wep->quantity && wep2->quantity)
        {
                if (ent->client->ps.gunframe < 102)
                        ent->client->ps.gunframe = 102;

                if (ent->client->weaponstate == WEAPON_FIRING2)
                {
                        FireMachinegun (ent, false, 0, 0, 8);
                        FireMachinegun (ent, true, 102, 103, 8);
                }
                else
                {
                        FireMachinegun (ent, false, 102, 103, 8);
                        FireMachinegun (ent, true, 0, 0, 8);
                }
        }
        else
        {
                ent->client->ps.gunframe = 8;
                ent->client->weaponstate = WEAPON_READY;
        }
}

void Dual_Machineguns_Fire_Alt (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;
        int             i;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        if (ent->client->ps.gunframe < 104)
                ent->client->ps.gunframe = 104;
        else if ((ent->client->ps.gunframe < 108) || ((ent->client->ps.gunframe >= 110) && (ent->client->ps.gunframe <= 113)))
                ent->client->ps.gunframe++;
        else if ((wep->quantity < 1) || ((wep->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 116) || (ent->client->ps.gunframe > 117))
                        ent->client->ps.gunframe = 116;
                FireMachinegun (ent, true, 116, 117, 110);
        }
        else if ((wep2->quantity < 1) || ((wep2->flags & W_WATER_JAMMED) && ent->waterlevel > 1))
        {
                if ((ent->client->ps.gunframe < 114) || (ent->client->ps.gunframe > 115))
                        ent->client->ps.gunframe = 114;
                FireMachinegun (ent, false, 114, 115, 110);
        }
        else if (wep->quantity && wep2->quantity)
        {
                if ((ent->client->ps.gunframe < 108) || (ent->client->ps.gunframe > 109))
                        ent->client->ps.gunframe = 108;

                if ((ent->client->ps.gunframe == 108) || (ent->client->ps.gunframe == 109))
                {
                        FireMachinegun (ent, false, 0, 0, 110);
                        FireMachinegun (ent, true, 108, 109, 110);
                }
        }

        if ((ent->client->ps.gunframe >= 110) && (ent->client->ps.gunframe <= 113))
        {
                ent->client->weaponstate = WEAPON_FIRING2;
                if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE) && (wep->quantity || wep2->quantity))

                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        i = ent->client->ps.gunframe - 110;
                        ent->client->ps.gunframe = (i + 104);
                }
                else if (ent->client->ps.gunframe == 113)
                {
                        ent->client->ps.gunframe = 8;
                        ent->client->weaponstate = WEAPON_READY;
                }
        }
}

void Weapon_DualMachineguns (edict_t *ent)
{
        static int      pause_frames[]  = {13, 0};

        // only full auto with dual weapons
        if (ent->client->weaponstate == WEAPON_ACTIVATING)
        {
                ent->client->pers.goodhand->flags &= ~W_ALTERNATE1;
                ent->client->pers.offhand->flags &= ~W_ALTERNATE1;
        }

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 57)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 60)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 65)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5slide.wav"), 1, ATTN_NORM, 0);
        }

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe == 85)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 88)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 93)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5slide.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 5, 7, 42, 48, 73, 57, 60, pause_frames, NULL, Dual_Machineguns_Fire,
        75, 101, 85, 88, NULL, Dual_Machineguns_Fire_Alt, NULL);
}
// GRIM - Not sure why I am stopping, the next bit is also edited

//================================================================

// Dirty
void FirePistol (edict_t *ent, qboolean left, int f_frame, int s_frame, int i_frame)
{
        vec3_t          forward, right, offset, start;
        int             buttonMask; // mod, 
        int             type = TOD_LEAD;
        weapons_t       *wep;

        if (left)
        {
                buttonMask = BUTTON_USE;
                VectorSet(offset, 0, -2, ent->viewheight-2);
                wep = ent->client->pers.offhand;
        }
        else
        {
                buttonMask = BUTTON_ATTACK;
                VectorSet(offset, 0, 2, ent->viewheight-2);
                wep = ent->client->pers.goodhand;
        }

        if (ent->client->ps.gunframe == s_frame)
        {
                if (!(ent->client->buttons & buttonMask))
                {
                        ent->client->ps.gunframe++;
                        return;
                }
                else if (wep->quantity > 0)
                        ent->client->ps.gunframe = f_frame;
                else
                        return;
        }

        if (wep->quantity <= 0)
        {
                ent->client->ps.gunframe = i_frame;
                ent->client->weaponstate = WEAPON_READY;
                return;
	}

        if (ent->client->continuous_shots < 9)
                ent->client->continuous_shots++;

        /* FIX ME
        if (ent->client->pers.weapon_handling && (ent->client->pers.weapon == ent->client->pers.weapon2))
                mod = MOD_PISTOL_JW;
        else
                mod = MOD_PISTOL;
        */

        // get start / end positions
        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_baselead (ent, start, forward, 96, 768, 1536, 100, 100, type, 57, 30, MOD_PISTOL, 1); // mod instead of MOD_PISTOL?

        if ((ent->client->pers.specials & SI_SILENCER) && (!ent->client->pers.weapon2))
                gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/silencer.wav"), 1, ATTN_NORM, 0);
        else
                MuzzleFlash (ent, "weapons/mk23fire.wav", left);

        //PlayerNoise(ent, start, PNOISE_WEAPON);

        if (!((int)dmflags->value & DF_INFINITE_AMMO))
                wep->quantity--;
}

void PistolFireRight (edict_t *ent)
{
        FirePistol (ent, false, 10, 0, 13);
        ent->client->ps.gunframe++;
}

void PistolAltFire (edict_t *ent)
{
        int     i;

        if (ent->client->ps.gunframe < 69)
                ent->client->ps.gunframe = 69;
        else if (ent->client->ps.gunframe < 73)
        {
                if (!(ent->client->buttons & BUTTON_USE))
                {
                        i = ent->client->ps.gunframe - 68;
                        ent->client->ps.gunframe = (i + 75);
                }
        }
        else if (ent->client->ps.gunframe == 73)
                FirePistol (ent, false, 73, 0, 13);
        else if (ent->client->ps.gunframe == 75)
        {
                if ((ent->client->buttons & BUTTON_USE) && (ent->client->pers.goodhand->quantity > 0))
                {
                        ent->client->ps.gunframe = 73;
                        FirePistol (ent, false, 73, 0, 13);
                }
        }
        else if (ent->client->ps.gunframe >= 79)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 13;
        }
        ent->client->ps.gunframe++;
}

void Weapon_Pistol (edict_t *ent)
{
        static int      pause_frames[]  = {0};
        static int      fire_frames[]   = {10, 0};
        static int      fire_frames2[]   = {65, 69, 70, 71, 72, 73, 75, 79, 0};

        if (ent->client->ps.gunframe == 4)
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23slide.wav"), 1, ATTN_NORM, 0);

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 49)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 56)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 61)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23slap.wav"), 1, ATTN_NORM, 0);
        }

        if ((ent->client->ps.gunframe < 5) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Sight"))] > 0))
                ent->client->pers.specials |= SI_LASER_SIGHT;

        if ((ent->client->weaponstate == WEAPON_READY) || (ent->client->ps.gunframe == 10)  || (ent->client->ps.gunframe == 75))
                TracenPlace (ent, 2, ent->viewheight-2);

        Weapon_Generic (ent, 9, 12, 40, 43, 64, 49, 61, pause_frames, fire_frames, PistolFireRight,
        79, 0, 0, 0, fire_frames2, PistolAltFire, NULL);
}
// Dirty

/*
================================================
Dual Pistols - John Woo style.
================================================
*/
// Dirty
void Dual_Pistols_Fire (edict_t *ent)
{
        weapons_t       *wep;
        weapons_t       *wep2;

        wep = ent->client->pers.goodhand;
        wep2 = ent->client->pers.offhand;

        if ((ent->client->ps.gunframe == 10) || (ent->client->ps.gunframe == 11))
        {
                if ((ent->client->buttons & BUTTON_ATTACK) && (wep2->quantity > 0))
                {
                        ent->client->ps.gunframe = 61;
                        FirePistol (ent, true, 0, 0, 0);
                }
                else if (ent->client->ps.gunframe == 11)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 12;
                }
                ent->client->ps.gunframe++;
                return;
        }
        else if ((ent->client->ps.gunframe == 62) || (ent->client->ps.gunframe == 63))
        {
                if ((ent->client->buttons & BUTTON_ATTACK) && (wep->quantity > 0))
                {
                        ent->client->ps.gunframe = 9;
                        FirePistol (ent, false, 0, 0, 0);
                }
                else if (ent->client->ps.gunframe == 63)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 12;
                }
                ent->client->ps.gunframe++;
                return;
        }

        if (wep->quantity >= wep2->quantity)
        {
                ent->client->ps.gunframe = 9;
                FirePistol (ent, false, 0, 0, 0);
                ent->client->ps.gunframe++;
        }
        else if (wep2->quantity > 0)
        {
                ent->client->ps.gunframe = 61;
                FirePistol (ent, true, 0, 0, 0);
                ent->client->ps.gunframe++;
        }
        else
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 12;
        }
}

void Weapon_DualPistols (edict_t *ent)
{
        static int      pause_frames[]  = {19, 0};

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 45)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 52)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 57)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23slap.wav"), 1, ATTN_NORM, 0);
        }

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe == 69)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 76)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 81)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23slap.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 8, 11, 36, 39, 60, 45, 57, pause_frames, NULL, Dual_Pistols_Fire,
        63, 84, 69, 81, NULL, NULL, NULL);
}
// Dirty
// GRIM
//================================================================

/*
================
PUNCH
================
*/
void Punch (edict_t *ent, qboolean grab) 
{
        vec3_t  start, forward, right; 
        vec3_t  point, offset;
        int     damage, hl;
        trace_t tr;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

        if (rand() & 1)
                VectorSet(offset, 0, 5,  ent->viewheight);
        else
                VectorSet(offset, 0, -5,  ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        VectorMA (start, 36, forward, point);

        tr = gi.trace (start, NULL, NULL, point, ent, MASK_SHOT);

        if (tr.fraction < 1)
        {
                damage = 6 + (rand() %5);

                if (tr.ent->takedamage)
                {
                        // Dirty
                        if (tr.ent->client && grab && (!OnSameTeam(tr.ent, ent)))
                        {
                                if (infront(tr.ent, ent))
                                {
                                        if ((tr.ent->client->weaponstate != WEAPON_FIRING)
                                         && (tr.ent->client->weaponstate != WEAPON_FIRING2)
                                          && (tr.ent->client->weaponstate != WEAPON_FIRING3))
                                        {
                                                tr.ent->client->weaponstate = WEAPON_READY;
                                                tr.ent->corpse = ent;
                                                if (tr.ent->client->pers.weapon_handling)
                                                        SwitchGuns(tr.ent);
                                                else
                                                        Drop_CurrentWeapon (tr.ent, 1);
                                
                                        }
                                        ObjectAndFunction (ent);
                                        return;
                                }

                                hl = GetHitLocation (tr.ent, forward, tr.endpos, TOD_NO_WOUND, MOD_HIT);
                                if ((hl & HIT_BACK) && (hl & (HIT_CHEST | HIT_HEAD)))
                                {
                                        if ((level.time > (tr.ent->last_combat + 1)) && ((tr.ent->spotted != ent) || (tr.ent->spotted_time < level.time)))
                                        {
                                                // This is from Paranoid limb loss...
                                                // we use it to make a silent kill...
                                                T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, 120, 0, (HIT_HEAD | HIT_THROAT), (TOD_NO_ARMOR | TOD_NO_KNOCKBACK | TOD_PELLET), MOD_NECKSNAP);
                                                gi.sound (tr.ent, CHAN_AUTO, gi.soundindex ("misc/bhit_head1.wav"), 0.5, ATTN_NORM, 0);
                                                return;
                                        }
                                }
                        }

                        T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage, 50, 0, TOD_NO_WOUND, MOD_HIT);

                        if (random() > 0.5)
                                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/phitf1.wav"), 0.1, ATTN_NORM, 0);
                        else 
                                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/phitf2.wav"), 0.1, ATTN_NORM, 0);
                        // Dirty
                }
                else
                {       // hit wall
                        if (random() > 0.5)
                                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/phitw1.wav"), 0.1, ATTN_NORM, 0);
                        else 
                                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/phitw2.wav"), 0.1, ATTN_NORM, 0);

                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_SHOTGUN);
                        gi.WritePosition (tr.endpos);
                        gi.WriteDir (tr.plane.normal);
                        gi.multicast (tr.endpos, MULTICAST_PVS);
                }
        }
}

void Weapon_Hands (edict_t *ent)
{
        gitem_t *item = NULL;
        int     i = 0;

        // Dirty
        if (ent->client->newweapon || (ent->combine && (ent->client->pers.grabbing > 0)))
         // || ent->velocity[0] || ent->velocity[1])))
        // Dirty
        {
                if (ent->delay_time > level.time)
                {
                        gi.centerprintf (ent, "Action unfinished!\n");
                        ent->delay_time = level.time;
                }
                ent->combfunc = NULL;
                ent->combine = NULL;
                ent->combinetwo = NULL;

                if (ent->client->newweapon)
                        ChangeWeapon (ent);
                return;
	}

        if (ent->combfunc)
        {
                if (ent->delay_time < level.time)
                {
                        ent->combfunc (ent, ent->combine, ent->combinetwo);

                        // Re-check, coz some re-start...
                        if (ent->delay_time < level.time)
                        {
                                ent->combfunc = NULL;
                                ent->combine = NULL;
                                ent->combinetwo = NULL;
                                ent->client->weaponstate = WEAPON_READY;
                        }
                }
        }
        else if ((ent->delay_time < level.time) && (level.time - ent->last_damage > 1)
         && (!ent->velocity[0]) && (!ent->velocity[1]) && (ent->client->pers.grabbing == 0))
        {
                i = Wounded(ent);

                if (i > 0)
                {
                        // Dirty        
                        /*
                        if (i > 3)
                        {
                                if (ent->client->pers.inventory[ITEM_INDEX(FindItem("MediKit"))] > 0)
                                        item = FindItem("MediKit");
                                else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BindKit"))] > 0)
                                        item = FindItem("BindKit");
                        }
                        else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BindKit"))] > 0)
                                item = FindItem("BindKit");
                        else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("MediKit"))] > 0)
                                item = FindItem("MediKit");
                        */
                        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bandages"))] > 0)
                                item = FindItem("Bandages");
                        // Dirty
                        if (item)
                                Use_Health (ent, item);
                }
        }

	if (ent->client->weaponstate == WEAPON_READY)
	{
                // May not fire until match has actually begun...
                if (level.match_state == MATCH_START)
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
                 || ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE))
		{
                        if (ent->attack_time > level.time)
                                return;

                        if (ent->delay_time > level.time)
                        {
                                gi.centerprintf (ent, "Action unfinished!\n");
                                ent->combfunc = NULL;
                                ent->combine = NULL;
                                ent->combinetwo = NULL;
                                ent->delay_time = level.time;
                        }

                        ent->client->weaponstate = WEAPON_FIRING;
                        if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE)
                                Punch (ent, true);
                        else if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
                                Punch (ent, false);
                        ent->attack_time = level.time + 0.3;
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        ent->client->latched_buttons &= ~BUTTON_USE;
		}
	}

        if (ent->client->weaponstate == WEAPON_FIRING)
	{
                if (ent->attack_time < level.time)
                        ent->client->weaponstate = WEAPON_READY;
        }
}
// GRIM

/*
==============
Cmd_Hands_f - Put away you weapon and change to you hands
                or change back to you weapon..
==============
*/
void Cmd_Hands_f (edict_t *ent)
{
        if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
                Cmd_WeapLast_f(ent);
        else
                Use_Weapon (ent, FindItem("Hands"));
}


// Dirty
void Sniper_Rifle_Fire (edict_t *ent)
{
        vec3_t          start, forward, right, offset;
        qboolean        no_ammo = false;
        int             sh = 60;
        int             sv = 50;
        int             type = (TOD_LEAD | TOD_HV | TOD_AP);
        weapons_t       *wep = ent->client->pers.goodhand;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

        if (ent->client->pers.goodhand->flags & (W_ALTERNATE1 | W_ALTERNATE2 | W_ALTERNATE3))
                VectorSet(offset, 0, 0, ent->viewheight);
        else
                VectorSet(offset, 0, 1, ent->viewheight-1);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        //if (UnderWaterCheck(start) && (wep->flags & W_WATER_JAMMED))
        if ((ent->waterlevel == 3) && (wep->flags & W_WATER_JAMMED))
        {
                ent->client->ps.gunframe = 22;
                ent->client->weaponstate = WEAPON_READY;
                return;
        }
        else if (wep->flags & W_WATER_JAMMED)
                wep->flags &= ~W_WATER_JAMMED;

        if (ent->client->ps.fov != 90)
        {
                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                {
                        sh = 0;
                        sv = 0;
                }
                else
                {
                        sh = 20;
                        sv = 20;
                }
        }

        fire_baselead (ent, start, forward, 2048, 6144, 8192, sh, sv, type, 160, 220, MOD_SNIPER_RIFLE, 10);

        if (ent->client->pers.specials & SI_SILENCER)
                gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/silencer.wav"), 1, ATTN_NORM, 0);
        else
                MuzzleFlash (ent, "weapons/ssgfire.wav", false);

        if (!((int)dmflags->value & DF_INFINITE_AMMO))
                wep->quantity--;

        ent->client->ps.gunframe++;
}

void SetZoom (edict_t *ent)
{
        if (ent->client->pers.goodhand->flags & W_ALTERNATE1)
        {
                ent->client->ps.gunindex = 0;
                ent->client->ps.fov = 40;
                ent->client->ps.stats[STAT_CENTRAL] = gi.imageindex ("scope2x");
                stuffcmd(ent, "set old_crosshair_dude $crosshair; set sensitivity 6\n");
        }
        else if (ent->client->pers.goodhand->flags & W_ALTERNATE2)
        {
                ent->client->ps.gunindex = 0;
                ent->client->ps.fov = 20;
                ent->client->ps.stats[STAT_CENTRAL] = gi.imageindex ("scope4x");
                stuffcmd(ent, "set old_crosshair_dude $crosshair; set sensitivity 4\n");
        }
        else if (ent->client->pers.goodhand->flags & W_ALTERNATE3)
        {
                ent->client->ps.gunindex = 0;
                ent->client->ps.fov = 10;
                ent->client->ps.stats[STAT_CENTRAL] = gi.imageindex ("scope6x");
                stuffcmd(ent, "set old_crosshair_dude $crosshair; set sensitivity 2\n");
        }
        else
                ZoomOff(ent);
}

void ResetZoom (edict_t *ent)
{
        ent->radius_dmg = 0;
        if (ent->client->ps.fov != 90)
        {
                ent->client->ps.fov = 90;
                stuffcmd(ent, "set crosshair $old_crosshair_dude; set sensitivity $b4_zoom_dude\n");
                ent->client->ps.stats[STAT_CENTRAL] = 0;
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        }

        ent->client->pers.goodhand->flags &= ~W_ALTERNATE1;
        ent->client->pers.goodhand->flags &= ~W_ALTERNATE2;
        ent->client->pers.goodhand->flags &= ~W_ALTERNATE3;
}

void Sniper_Rifle_Zoom (edict_t *ent)
{
        weapons_t       *wep;

        ent->client->weaponstate = WEAPON_READY;
        ent->client->ps.gunframe = 22;

        if (ent->attack_time > level.time)
                return;

        ent->attack_time = level.time + 0.3;

        if (ent->client->ps.fov != 90)
                ZoomOff(ent);

        wep = ent->client->pers.goodhand;

        //stuffcmd(ent, "play misc/lensflik.wav\n");
        //unicastSound (ent, gi.soundindex("misc/lensflik.wav"), 1);
        //gi.sound(ent, CHAN_GOODHAND, gi.soundindex("misc/lensflik.wav"), 1, ATTN_NORM, 0);

        if (wep->flags & W_ALTERNATE1)
        {
                wep->flags &= ~W_ALTERNATE1;
                wep->flags |= W_ALTERNATE2;
        }
        else if (wep->flags & W_ALTERNATE2)
        {
                wep->flags &= ~W_ALTERNATE2;
                wep->flags |= W_ALTERNATE3;
        }
        else if (wep->flags & W_ALTERNATE3)
                wep->flags &= ~W_ALTERNATE3;
        else
                wep->flags |= W_ALTERNATE1;

        if (ent->velocity[0] || ent->velocity[1])
        {
                // makes it zoom in when you stop
                if (ent->radius_dmg < 2)
                        ent->radius_dmg = 2;
                return;
        }

        SetZoom (ent);
}

void Weapon_Sniper_Rifle (edict_t *ent)
{
        static int      pause_frames[]  = {41, 0};
        static int      fire_frames[]   = {9, 0};

        if ((ent->client->weaponstate == WEAPON_DROPPING) || (ent->client->weaponstate == WEAPON_ACTIVATING))
                ResetZoom (ent);

        if (ent->client->ps.gunframe == 13)
                gi.sound(ent, CHAN_OFFHAND, gi.soundindex("weapons/ssgbolt.wav"), 1, ATTN_NORM, 0);

        //if (ent->client->ps.gunframe == 16)
        //        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/ssgbolt.wav"), 1, ATTN_NORM, 0);

        if (ent->client->ps.gunframe == 21)
        {
                ent->client->pers.goodhand->flags &= ~W_UNLOADED;
                SetZoom (ent);
        }

        if (ent->client->ps.gunframe == 42)
                ZoomOff(ent);

        if (ent->client->ps.gunframe == 12)
                ZoomOff(ent);

        // Simple, the guns been dropped before it's had the next round
        // put in, so it needs to be chambered.
        if ((ent->client->ps.gunframe > 21)
         && (ent->client->weaponstate == WEAPON_READY)
          && (ent->client->pers.goodhand->flags & W_UNLOADED))
                ent->client->ps.gunframe = 10;

        // This is for shoot and drop situations... no reload next round etc
        if ((ent->client->ps.gunframe == 10) && (ent->client->weaponstate == WEAPON_FIRING))
        {
                ent->client->pers.goodhand->flags |= W_UNLOADED;
                ent->client->weaponstate = WEAPON_READY;
        }

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                ent->radius_dmg = 2;
                if (ent->client->ps.gunframe < 66)
                {
                        ent->client->continuous_shots = 66;
                        ent->client->machinegun_shots = 73;
                }

                if (ent->client->ps.gunframe == 58)
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/ssgbolt.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 71)
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/ssgin.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 76)
                {
                        ent->client->pers.goodhand->flags &= ~W_UNLOADED;
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/ssgbolt.wav"), 1, ATTN_NORM, 0);
                }
        }
        else if (ent->velocity[0] || ent->velocity[1] || (!ent->groundentity))
        {
                if (ent->client->ps.fov < 90)
                        ent->radius_dmg++;

                if (ent->radius_dmg >= 20)
                {
                        if (ent->client->ps.fov < 90)
                                ZoomOff(ent);
                        ent->radius_dmg = 20;
                }
        }
        else if (ent->radius_dmg > 0)
        {
                ent->radius_dmg -= 2;

                if (ent->radius_dmg < 2)
                {
                        ent->radius_dmg = 0;
                        SetZoom (ent);
                }
        }

        Weapon_Generic (ent, 8, 21, 41, 50, 81, 58, 71, pause_frames, fire_frames, Sniper_Rifle_Fire,
        -1, 0, 0, 0, NULL, Sniper_Rifle_Zoom, NULL);
}

/*
====================================
The knife
====================================
*/
static void KnifeTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        edict_t *knife = NULL;
        vec3_t  forward;
        int     damage;
        float   dot;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

        if (ent->watertype & CONTENTS_LAVA)
	{
		G_FreeEdict (ent);
		return;
	}

        ent->prethink = NULL;

        if (other->takedamage)
	{
                if (ent->movetype == MOVETYPE_FLYMISSILE)
                        damage = 120;
                else
                        damage = 60;
                T_Damage (other, ent->master, ent->master, ent->velocity, ent->s.origin, plane->normal, damage, 50, 0, TOD_AP, MOD_THROWN_KNIFE);
                gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/stab.wav"), 0.3, ATTN_NORM, 0);
                G_FreeEdict (ent);
                return;
	}

        // Check limit here so we can't have shitloads in the air as well...
        if ((int)max_knives->value > 0)
        {
                knife_number++;
                if (knife_number > (int)max_knives->value)
                        knife_number = 1;
                knife = FindEdictByClassnum ("stuck_knife", knife_number);
                if (knife)
                        knife->nextthink = level.time + 0.1;
        }

        ent->classname = "stuck_knife";
        ent->points = knife_number;

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_SHOTGUN);
        gi.WritePosition (ent->s.origin);
        gi.WriteDir (plane->normal);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/clank.wav"), 0.4, ATTN_NORM, 0);

        /*
        if (other->movetype == MOVETYPE_PUSH)
	{
                G_FreeEdict (ent);
                return;
	}
        */

        ent->movetype = MOVETYPE_NONE;

        // Make it grabbable
        VectorSet (ent->mins, -15, -15, -15);
        VectorSet (ent->maxs, 15, 15, 15);
        ent->solid = SOLID_TRIGGER;
        ent->touch = Touch_Item;
        ent->nextthink = level.time + 30;
        ent->think = G_FreeEdict;
        ent->s.sound = 0; // Oops... forgot to turn it off last time        
        AngleVectors (ent->s.angles, forward, NULL, NULL);
        dot = DotProduct (plane->normal, forward);
	
        if (dot > 0)
                ent->s.angles[0] += 180;

        gi.linkentity (ent);
}

void KnifeArc (edict_t *ent)
{
        ent->nextthink = 0;
        ent->owner = NULL;
        ent->think = NULL;
        ent->movetype = MOVETYPE_BOUNCE;
        ent->prethink = Calc_Arc;
        gi.linkentity (ent);
}


void ThrownKnife (edict_t *ent, vec3_t start, vec3_t aimdir)
{
        edict_t *knife = NULL;

        knife = G_Spawn();
        knife->classname = "thrown_knife";

        VectorCopy (start, knife->s.origin);
        VectorScale (aimdir, 1200, knife->velocity);
        vectoangles (aimdir, knife->s.angles);

        knife->movetype = MOVETYPE_FLYMISSILE;
        knife->clipmask = MASK_SHOT;
        knife->solid = SOLID_BBOX;
        knife->s.modelindex = gi.modelindex ("models/objects/knife/tris.md2");
        knife->owner = ent;
        knife->master = ent;
        knife->touch = KnifeTouch;
        knife->s.sound = gi.soundindex ("misc/flyloop.wav");

        //VectorSet (knife->avelocity, 2000, 0, 0);
	VectorSet (knife->avelocity, 1200, 0, 0);
        knife->nextthink = level.time + 0.4;
        knife->think = KnifeArc;

        // Add stuff so it can be picked up...
        knife->item = FindItem("Combat Knife");
        knife->spawnflags = DROPPED_PLAYER_ITEM;
        knife->count = 1;

        gi.linkentity (knife);
}

void Throw (edict_t *ent, qboolean left)
{
        vec3_t  start, forward, right; 
        vec3_t  offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
        if (left)
                VectorSet(offset, 0, -8, ent->viewheight-6);
        else
                VectorSet(offset, 0, 8, ent->viewheight-6);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        ThrownKnife (ent, start, forward);
        gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/throw.wav"), 0.2, ATTN_NORM, 0);
        ent->client->pers.inventory[ITEM_INDEX(FindItem("Combat Knife"))]--;
        RemoveAmmoWeight (ent, FindItem("Combat Knife"), 1);
        ValidateSelectedItem (ent);
        SetAmmoHud (ent);
}

void Swish (edict_t *ent)
{
        vec3_t  start, forward, right; 
        vec3_t  point, offset;
        trace_t tr;
        int     hl = 0;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
        VectorSet(offset, 0, 0, ent->viewheight - 6);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        VectorMA (start, 45, forward, point);

        tr = gi.trace (start, NULL, NULL, point, ent, MASK_SHOT);

        if (tr.fraction < 1)
        {
                if (tr.ent->takedamage)
                {
                        if (tr.ent->client)
                        {
                                hl = GetHitLocation (tr.ent, forward, tr.endpos, TOD_AP, MOD_KNIFETOTHEBACK);
                                if ((hl & HIT_BACK) && (hl & (HIT_CHEST | HIT_HEAD | HIT_MID)))
                                {
                                        if ((level.time > (tr.ent->last_combat + 1)) && ((tr.ent->spotted != ent) || (tr.ent->spotted_time < level.time)))
                                        {
                                                // This is from Paranoid limb loss...
                                                // we use it to make a silent kill...
                                                tr.ent->count = HIT_HEAD;
                                                T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, 120, 0, hl, (TOD_AP | TOD_NO_KNOCKBACK), MOD_KNIFETOTHEBACK);
                                                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/stab.wav"), 0.2, ATTN_NORM, 0);
                                                return;
                                        }
                                }
                        }

                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/stab.wav"), 0.2, ATTN_NORM, 0);
                        T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, 60, 15, hl, TOD_AP, MOD_KNIFE);
                } // hit wall
                //else if (ent->client->ps.gunframe == 7)
                //        gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/clank.wav"), 1, ATTN_NORM, 0);
        }
}

void Weapon_Knife (edict_t *ent)
{
        // Dirty
        if (ent->client->ps.gunindex == 0)
        {
                ent->client->ps.gunindex = ent->client->old_gunindex;
                ent->client->old_gunindex = 0;
        }
        // Dirty

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                ent->client->pers.specials &= ~(SI_LASER_SIGHT | SI_LASER_SIGHT_HELP);
                if (ent->client->newweapon && (ent->client->newweapon == ent->client->pers.weapon))
                {
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                                ent->client->ps.gunframe = 63;
                        else
                                ent->client->ps.gunframe = 5;
                        ent->client->newweapon = NULL;
                }

                if ((ent->client->ps.gunframe == 0) && (ent->client->resp.alternates & A_KNIFE_THROW))
                        ent->client->ps.gunframe = 60;
                else if (ent->client->ps.gunframe == 5)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 13;
                }
                else if ((ent->client->ps.gunframe == 63) || (ent->client->ps.gunframe == 117))
                        ent->client->weaponstate = WEAPON_READY;
                else
                        ent->client->ps.gunframe++;
		return;
	}

        if (ent->client->weaponstate == WEAPON_DROPPING)
	{
                if (ent->client->ps.gunframe == 59)
                        ChangeWeapon (ent);
                else if (ent->client->ps.gunframe == 110)
                {
                        if (ent->client->newweapon)
                                ChangeWeapon (ent);
                        else
                                ent->client->weaponstate = WEAPON_READY;
                }
                else
                        ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
                // May not fire until match has actually begun...
                if (level.match_state == MATCH_START)
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE))
                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                ent->client->resp.alternates &= ~A_KNIFE_THROW;
                                ent->client->weaponstate = WEAPON_DROPPING;
                                ent->client->ps.gunframe = 106;
                        }
                        else
                        {
                                ent->client->resp.alternates |= A_KNIFE_THROW;
                                ent->client->weaponstate = WEAPON_ACTIVATING;
                                ent->client->ps.gunframe = 60;
                        }
                        return;
                }
                else if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                ent->client->weaponstate = WEAPON_FIRING2;
                                ent->client->ps.gunframe = 103;
                        }
                        else
                        {
                                ent->client->weaponstate = WEAPON_FIRING;
                                ent->client->ps.gunframe = 6;
                        }
                }
                else
                {
                        if (ent->client->newweapon2)
                        {
                                if (ent->client->newweapon2 == ent->client->pers.weapon)
                                {
                                        ent->client->ps.gunframe = 0;
                                        ChangeWeapon2 (ent);
                                        return;
                                }
                                else
                                {
                                        ent->client->newweapon = ent->client->newweapon2;
                                        ent->client->newweapon2 = NULL;
                                }
                        }

                        if (ent->client->newweapon)
                        {
                                if ((ent->client->pers.weapon_handling == NULL) && (ent->client->pers.weapon == ent->client->newweapon))
                                        ent->client->newweapon = NULL;
                        }

                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                if (ent->client->newweapon)
                                {
                                        ent->client->weaponstate = WEAPON_DROPPING;
                                        ent->client->ps.gunframe = 106;
                                        return;
                                }

                                if (ent->client->ps.gunframe >= 102)
                                        ent->client->ps.gunframe = 64;
                        }
                        else
                        {
                                if (ent->client->newweapon)
                                {
                                        ent->client->weaponstate = WEAPON_DROPPING;
                                        ent->client->ps.gunframe = 53;
                                        return;
                                }
                                if (ent->client->ps.gunframe >= 52)
                                        ent->client->ps.gunframe = 13;
                        }
                        ent->client->ps.gunframe++;
                }
	}

        if (ent->client->weaponstate == WEAPON_FIRING)
	{
                if (ent->client->ps.gunframe >= 12)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        return;
                }
                else
                        ent->client->ps.gunframe++;

                if (ent->client->ps.gunframe == 7)
                {
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/swish.wav"), 0.1, ATTN_NORM, 0);
                        Swish (ent);
                }
                //else if (ent->client->ps.gunframe == 8)
                //        Swish (ent);
        }

        if (ent->client->weaponstate == WEAPON_FIRING2)
	{
                ent->client->ps.gunframe++;
                if (ent->client->ps.gunframe == 105)
                        Throw (ent, false);
                else if (ent->client->ps.gunframe >= 106)
                {
                        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Combat Knife"))] < 1)
                        {
                                ent->client->newweapon = FindItem("Hands");
                                ChangeWeapon (ent);
                        }
                        else
                        {
                                ent->client->weaponstate = WEAPON_ACTIVATING;
                                ent->client->ps.gunframe = 111;
                        }
                        return;
                }
        }
}


void Weapon_DualKnives (edict_t *ent)
{
        // Dirty
        if (ent->client->ps.gunindex == 0)
        {
                ent->client->ps.gunindex = ent->client->old_gunindex;
                ent->client->old_gunindex = 0;
        }
        // Dirty

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                ent->client->ammo_index2 = -1;
                ent->client->pers.specials &= ~(SI_LASER_SIGHT | SI_LASER_SIGHT_HELP);

                if ((ent->client->ps.gunframe == 0) && (ent->client->resp.alternates & A_KNIFE_THROW))
                        ent->client->ps.gunframe = 28;
                else if (ent->client->ps.gunframe == 5)
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 20;
                }
                else if ((ent->client->ps.gunframe == 31) || (ent->client->ps.gunframe == 44) || (ent->client->ps.gunframe == 55))
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = 32;
                }
                else
                        ent->client->ps.gunframe++;
		return;
	}

        if (ent->client->weaponstate == WEAPON_DROPPING)
	{
                if (ent->client->ps.gunframe == 27)
                {
                        if (ent->client->newweapon == ent->client->pers.weapon)
                                ChangeWeapon (ent);
                        else
                                SetHands (ent);
                }
                else if (ent->client->ps.gunframe == 37)
                {
                        if (ent->client->newweapon)
                        {
                                if (ent->client->newweapon == ent->client->pers.weapon)
                                        ChangeWeapon (ent);
                                else
                                        SetHands (ent);
                        }
                        else
                                ent->client->weaponstate = WEAPON_READY;
                }
                else
                        ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
                // May not fire until match has actually begun...
                if (level.match_state == MATCH_START)
                        ent->client->ps.gunframe = ent->client->ps.gunframe;
                else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE))
                {
                        ent->client->latched_buttons &= ~BUTTON_USE;
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                ent->client->resp.alternates &= ~A_KNIFE_THROW;
                                ent->client->weaponstate = WEAPON_DROPPING;
                                ent->client->ps.gunframe = 33;
                        }
                        else
                        {
                                ent->client->resp.alternates |= A_KNIFE_THROW;
                                ent->client->weaponstate = WEAPON_ACTIVATING;
                                ent->client->ps.gunframe = 28;
                        }
                        return;
                }
                else if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                ent->client->weaponstate = WEAPON_FIRING2;
                                if (ent->radius_dmg > 0)
                                        ent->client->ps.gunframe = 56;
                                else
                                        ent->client->ps.gunframe = 45;
                        }
                        else
                        {
                                ent->client->weaponstate = WEAPON_FIRING;
                                if (ent->radius_dmg > 0)
                                        ent->client->ps.gunframe = 13;
                                else
                                        ent->client->ps.gunframe = 6;
                        }

                        if (ent->radius_dmg > 0)
                                ent->radius_dmg = 0;
                        else
                                ent->radius_dmg = 1;
                }
                else
                {
                        if (ent->client->resp.alternates & A_KNIFE_THROW)
                        {
                                if (ent->client->newweapon)
                                {
                                        ent->client->weaponstate = WEAPON_DROPPING;
                                        if ((ent->client->ps.gunframe == 63) || (ent->client->ps.gunframe == 67))
                                                ent->client->ps.gunframe = 37;
                                        else
                                                ent->client->ps.gunframe = 33;
                                        return;
                                }

                                ent->client->ps.gunframe = 32;
                        }
                        else
                        {
                                if (ent->client->newweapon)
                                {
                                        if (ent->client->newweapon == ent->client->pers.weapon)
                                                ChangeWeapon (ent);
                                        else
                                        {
                                                ent->client->weaponstate = WEAPON_DROPPING;
                                                ent->client->ps.gunframe = 21;
                                        }
                                        return;
                                }
                                ent->client->ps.gunframe = 20;
                        }
                }
	}

        if (ent->client->weaponstate == WEAPON_FIRING)
	{
                if ((ent->client->ps.gunframe == 11) || (ent->client->ps.gunframe == 12))
                {
                        if (ent->client->buttons & BUTTON_ATTACK)
                                ent->client->ps.gunframe = 13;
                        else
                        {
                                ent->client->weaponstate = WEAPON_READY;
                                return;
                        }
                }
                else if ((ent->client->ps.gunframe == 18) || (ent->client->ps.gunframe == 19))
                {
                        if (ent->client->buttons & BUTTON_ATTACK)
                                ent->client->ps.gunframe = 6;
                        else
                        {
                                ent->client->weaponstate = WEAPON_READY;
                                return;
                        }
                }

                if ((ent->client->ps.gunframe == 7) || (ent->client->ps.gunframe == 14))
                {
                        gi.sound(ent, CHAN_GOODHAND, gi.soundindex("weapons/swish.wav"), 0.1, ATTN_NORM, 0);
                        Swish (ent);
                }

                ent->client->ps.gunframe++;
        }

        if (ent->client->weaponstate == WEAPON_FIRING2)
	{
                ent->client->ps.gunframe++;
                if ((ent->client->ps.gunframe == 48) || (ent->client->ps.gunframe == 59))
                {
                        if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                        {
                                ent->client->ps.gunframe = 27;
                                ent->client->newweapon = FindItem("Hands");
                                ChangeWeapon (ent);
                        }
                        else if (ent->client->pers.inventory[ent->client->ammo_index] < 2)
                        {
                                ent->client->ps.gunframe = 27;
                                ent->client->newweapon = FindItem("Combat Knife");
                                ChangeWeapon (ent);
                                ent->client->ps.gunframe = 64;
                        }
                        else if (ent->client->ps.gunframe == 48)
                        {
                                if (ent->client->buttons & BUTTON_ATTACK)
                                        ent->client->ps.gunframe = 64;
                                else
                                {
                                        ent->client->ps.gunframe = 38;
                                        ent->client->weaponstate = WEAPON_ACTIVATING;
                                }
                        }
                        else if (ent->client->ps.gunframe == 59)
                        {
                                if (ent->client->buttons & BUTTON_ATTACK)
                                        ent->client->ps.gunframe = 60;
                                else
                                {
                                        ent->client->ps.gunframe = 49;
                                        ent->client->weaponstate = WEAPON_ACTIVATING;
                                }
                        }
                }
                else if ((ent->client->ps.gunframe == 47) || (ent->client->ps.gunframe == 62))
                        Throw (ent, false);
                else if ((ent->client->ps.gunframe == 58) || (ent->client->ps.gunframe == 66))
                        Throw (ent, true);
                else if ((ent->client->ps.gunframe == 63) || (ent->client->ps.gunframe == 67))
                {
                        if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
                        {
                                ent->client->ps.gunframe = 27;
                                ent->client->newweapon = FindItem("Hands");
                                ChangeWeapon (ent);
                        }
                        else if (ent->client->pers.inventory[ent->client->ammo_index] < 2)
                        {
                                ent->client->ps.gunframe = 27;
                                ent->client->newweapon = FindItem("Combat Knife");
                                ChangeWeapon (ent);
                                ent->client->ps.gunframe = 64;
                        }
                        else
                        {
                                ent->client->ps.gunframe = 28;
                                ent->client->weaponstate = WEAPON_ACTIVATING;
                        }
                }
        }
}


//================================================================================

void CanPistolFireCan (edict_t *ent)
{
        if (ent->client->ps.gunframe == 10)
        {
                if (ent->client->pers.goodhand->quantity == 2)
                {
                        if (ent->client->buttons & BUTTON_ATTACK)
                                FireDoubleBarrelShotgun (ent, false, 0, 15);
                        else
                                FireDoubleBarrelShotgun (ent, false, 1, 15);
                }
                else if (ent->client->pers.goodhand->quantity < 2)
                        FireDoubleBarrelShotgun (ent, false, 2, 15);
        }
        ent->client->ps.gunframe++;
        if (ent->client->ps.gunframe == 20)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 21;
        }

}

void CanPistolFireLeft (edict_t *ent)
{
        if (ent->client->ps.gunframe == 73)
                FirePistol (ent, true, 73, 74, 75);
        ent->client->ps.gunframe++;
        if (ent->client->ps.gunframe == 75)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 21;
        }
}

void CanPistolFireBoth (edict_t *ent)
{
        if (ent->client->ps.gunframe == 10)
        {
                if ((ent->client->pers.offhand->quantity > 0) && (ent->client->pers.goodhand->quantity > 0))
                {
                        ent->client->ps.gunframe = 97;
                        FirePistol (ent, true, 0, 0, 0);
                        if (ent->client->pers.goodhand->quantity < 2)
                                FireDoubleBarrelShotgun (ent, false, 2, 102);
                        else
                                FireDoubleBarrelShotgun (ent, false, 0, 102);
                }
                else if (ent->client->pers.goodhand->quantity > 0)
                {
                        ent->client->ps.gunframe = 10;
                        CanPistolFireCan (ent);
                        return;
                }
                else if (ent->client->pers.offhand->quantity > 0)
                {
                        ent->client->ps.gunframe = 73;
                        CanPistolFireLeft (ent);
                        return;
                }
        }

        ent->client->ps.gunframe++;
        if ((ent->client->ps.gunframe == 20) || (ent->client->ps.gunframe == 75) || (ent->client->ps.gunframe == 107))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe = 21;
        }
}


void Weapon_CanPist (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if (ent->client->ps.gunframe == 54)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/copen.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 58)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cout.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 66)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cin.wav"), 1, ATTN_NORM, 0);

                if (ent->client->ps.gunframe == 67)
                {
                        if ((ent->client->pers.goodhand->quantity == 1)
                         && (ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]))
                        {
                                ent->client->pers.goodhand->quantity = 2;
                                ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.goodhand->ammo))]--;
                                RemoveAmmoWeight (ent, FindItem(ent->client->pers.goodhand->ammo), 1);
                        }
                }

                if (ent->client->ps.gunframe == 72)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/cclose.wav"), 1, ATTN_NORM, 0);
        }

        if (ent->client->weaponstate == WEAPON_RELOADING2)
        {
                if (ent->client->ps.gunframe == 81)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23out.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 88)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23in.wav"), 1, ATTN_NORM, 0);
                if (ent->client->ps.gunframe == 93)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23slap.wav"), 1, ATTN_NORM, 0);
        }

        Weapon_Generic (ent, 9, 20, 48, 51, 72, 58, 66, pause_frames, NULL, CanPistolFireCan,
        75, 96, 81, 93, NULL, CanPistolFireLeft, CanPistolFireBoth);
}
// Dirty
