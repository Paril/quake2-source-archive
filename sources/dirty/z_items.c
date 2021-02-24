#include "g_local.h"

// z_items.c - Extra stuff concerning items and weight.
void RPI_Think (edict_t *ent);

/*
==============

CheckWeightAndSetSpeed - Changes speed based on what your carrying
        This will be scrapped if and when I find a better way of doing it.

==============
*/
/*
==============
SetWeight
==============
*/
void SetWeight (edict_t *ent)
{
        gitem_t *it;
        float   wt;
        int     i;

        ent->client->weapon_side_set;
        ent->client->weapon_forward_set;

        if (ent->flags & FL_GODMODE)
        {
                ent->weight = 0;
                return;
        }
        wt = 0;

        ent->ammo_carried = 0;

        // Weapon, PowerUp and Tech calc area
        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;

                if (!it->pickup)
                        continue;

                if (!ent->client->pers.inventory[i])
                        continue;

                if ((it->flags & IT_AMMO) && (it->weight > 0))
                {
                        if ((it->flags & IT_BIG) || (it->flags & IT_CLIP))
                        {
                                wt += (ent->client->pers.inventory[i] * it->weight);
                                if (strcmp(it->pickup_name, "Chaingun Pack"))
                                         ent->ammo_carried = ent->ammo_carried + (ent->client->pers.inventory[i] * it->weight);
                        }
                        else
                        {
                                wt += (ent->client->pers.inventory[i]/it->weight);
                                if (strcmp(it->pickup_name, "Chaingun Pack"))
                                         ent->ammo_carried = ent->ammo_carried + (ent->client->pers.inventory[i]/it->weight);
                        }
                }
                else if (it->weight > 0)
                        wt += (it->weight * ent->client->pers.inventory[i]);
                else
                        continue;
        }

        // Finally, give us the weight.
        ent->weight = wt;
}

void SetSpeed (edict_t *ent)
{
        if (ent->weight < 0)
                ent->weight = 0;
        else if (ent->weight > 360)
                ent->weight = 360;
        if (ent->ammo_carried < 0)
                ent->ammo_carried = 0;

        if (ent->deadflag || (!ent->client))
                return;

        if (ent->weight > 290)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_full");
        else if (ent->weight > 280)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_290");
        else if (ent->weight > 270)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_280");
        else if (ent->weight > 260)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_270");
        else if (ent->weight > 250)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_260");
        else if (ent->weight > 240)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_250");
        else if (ent->weight > 230)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_240");
        else if (ent->weight > 220)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_230");
        else if (ent->weight > 210)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_220");
        else if (ent->weight > 200)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_210");
        else if (ent->weight > 190)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_200");
        else if (ent->weight > 180)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_190");
        else if (ent->weight > 170)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_180");
        else if (ent->weight > 160)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_170");
        else if (ent->weight > 150)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_160");
        else if (ent->weight > 140)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_150");
        else if (ent->weight > 130)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_140");
        else if (ent->weight > 120)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_130");
        else if (ent->weight > 110)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_120");
        else if (ent->weight > 100)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_110");
        else if (ent->weight > 90)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_100");
        else if (ent->weight > 80)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_90");
        else if (ent->weight > 70)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_80");
        else if (ent->weight > 60)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_70");
        else if (ent->weight > 50)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_60");
        else if (ent->weight > 40)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_50");
        else if (ent->weight > 30)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_40");
        else if (ent->weight > 20)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_30");
        else if (ent->weight > 10)
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_20");
        else
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_10");
}


/*
==================
Visuals - Back Packs, Chainguns etc
(At moment, just chaingun or back pack!)
==================
*/
void ShowItems(edict_t *ent)
{
        return; // Dirty

        if (ent->deadflag)
	{
                ent->s.modelindex4 = 0;
		return;
	}

        if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))]) && ent->client->pers.weapon != FindItem("Chaingun") )
                ent->s.modelindex4 = gi.modelindex("models/visuals/chnpack/chaingun.md2");
        else if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("Back Pack"))]) || ent->client->pers.weapon == FindItem("Chaingun"))
                ent->s.modelindex4 = gi.modelindex("models/visuals/chnpack/backpack.md2");
        else                     
                ent->s.modelindex4 = 0;
}

//===========================================================================
/*
===========
BadCombo - Universal function to say that thing is no good
============
*/
void BadCombo (edict_t *ent)
{
        gi.centerprintf (ent, "Invalid choice. Combination canceled.\n");
        ent->combfunc = NULL;
        ent->combine = NULL;
        ent->combinetwo = NULL;
}

/*
===========
Drop_Clip -
============
*/
void debris_free (edict_t *self);
void Drop_Clip (edict_t *ent, gitem_t *item, int count)
{
	edict_t	*dropped;
        int     index;

        // Dirty - Enforce limit
        if (count == 0)
        {
                if (!ShitAvailable())
                        return;
                if (total_debris > (int)debris->value)
                        return;
                total_debris++;
        }
        // Dirty - Enforce limit

	index = ITEM_INDEX(item);
        dropped = Drop_Item (ent, item, ITEM_DROPPED);

        dropped->count = count;
//        RemoveAmmoWeight (ent, item, 1);

        if (count == 0)
        {
                dropped->item = FindItem(item->scomp);
                gi.setmodel (dropped, dropped->item->world_model);
                // Dirty - Less time. No one needs an empty clip
                dropped->think = debris_free;
                dropped->nextthink = level.time + 5 + random()*5;
                // Dirty
                gi.linkentity (dropped);
        }
}

/*
===========
Use_Clip2 - Switch between clips for secondary weapon.
============
*/

void Use_Clip2 (edict_t *ent, gitem_t *item)
{
        gitem_t         *wep;
        gitem_t         *comb;
        gitem_t         *ammo;
        weapons_t       *wep_info;
        char            *in;
        qboolean        check = false;

        wep = ent->client->pers.weapon2;
        wep_info = ent->client->pers.offhand;

        in = item->pickup_name;
        ammo = FindItem(wep_info->ammo);
        //gi.cprintf (ent, PRINT_HIGH, "Use clip2.\n");
        
        // Already using that type

        // Already using that type
        if ( (!strcmp(in, wep_info->ammo)) || (!(ammo->flags & IT_CLIP)))
                return;
        else
        {
                comb = FindComboByName (in);
                if (!strcmp(comb->weapon, wep->pickup_name))
                        ent->alternate_ammo = in;
        }

        if (ent->alternate_ammo == NULL)
        {
                gi.centerprintf (ent, "Can't use %s in %s or %s.\n", in, ent->client->pers.weapon->pickup_name, wep->pickup_name);
                return;
        }

        ent->client->weaponstate = WEAPON_RELOADING2;
}


/*
===========
Use_Clip - Switch between clips for primary weapon.
============
*/

void Use_Clip (edict_t *ent, gitem_t *item)
{
        gitem_t         *wep;
        gitem_t         *comb;
        gitem_t         *ammo;
        char            *in;
        weapons_t       *wep_info;
        qboolean        check = false;

        if (item->quantity < 1)
        {
                gi.centerprintf (ent, "That clip is empty.\n");
                return;
        }

        wep = ent->client->pers.weapon;
        wep_info = ent->client->pers.goodhand;

        in = item->pickup_name;
        ammo = FindItem(wep_info->ammo);
        //gi.cprintf (ent, PRINT_HIGH, "Use clip.\n");
        
        // Already using that type
        if ((!strcmp(in, wep_info->ammo)) || (!(ammo->flags & IT_CLIP)))
        {
                if (ent->client->pers.weapon_handling)
                        Use_Clip2 (ent, item);
                else
                        gi.cprintf (ent, PRINT_HIGH, "Can't use.\n");
                return;
        }
        else
        {
                comb = FindComboByName (in);
                if (!strcmp(comb->weapon, wep->pickup_name))
                        ent->alternate_ammo = in;
        }

        if (ent->alternate_ammo == NULL)
        {
                if (ent->client->pers.weapon_handling)
                        Use_Clip2 (ent, item);
                else
                        gi.cprintf (ent, PRINT_HIGH, "Can't use %s in %s.\n", in, wep->pickup_name);
                return;
        }

        ent->client->weaponstate = WEAPON_RELOADING;
}

/*
===========
CombineToMakeClip - Combine ammo type with this clip (Empty or otherwise).
============
*/
void CombineToMakeClip (edict_t *ent, gitem_t *combine, gitem_t *combinetwo)
{
        gitem_t         *newclip;
        gitem_t         *clip;
        gitem_t         *ammo;
        int             index, ammoindex;
        qboolean        bad = false;

        if (combine->flags & IT_CLIP)
        {
                clip = combine;
                ammo = combinetwo;
        }
        else
        {
                clip = combinetwo;
                ammo = combine;
        }

        newclip = FindCombo (ammo->pickup_name, clip->ammo, true);

        index = ITEM_INDEX(clip);
        ammoindex = ITEM_INDEX(ammo);

        ent->client->pers.inventory[index]--;
        RemoveAmmoWeight (ent, clip, 1);

        if (ent->client->pers.inventory[ammoindex] < newclip->quantity)
        {
                if (!Add_Ammo (ent, newclip, ent->client->pers.inventory[ammoindex]))
                {
                        Drop_Ammo_Specific (ent, newclip, ent->client->pers.inventory[ammoindex]);
                        bad = true;
                }
                ent->client->pers.inventory[ammoindex] = 0;
        }
        else
        {
                if (!Add_Ammo (ent, newclip, newclip->quantity))
                {
                        Drop_Ammo_Specific (ent, newclip, newclip->quantity);
                        bad = true;
                }
                ent->client->pers.inventory[ammoindex] -= newclip->quantity;
        }

        ent->combfunc = NULL;
        ent->combine = NULL;
        ent->combinetwo = NULL;
        if (!bad)
                gi.centerprintf (ent, "Clip made\n");
        else
                gi.centerprintf (ent, "Clip made, couldn't stash it\n");
	ValidateSelectedItem (ent);
}

void CheckClipCombine (edict_t *ent, gitem_t *ammo, gitem_t *clip)
{
        float   i;
        gitem_t *comb;
        qboolean bad = false;

        if (strcmp(clip->ammo, ammo->pickup_name) == 0)
                bad = true;

        // Really, I should make this automatic...but I'm lazy :)
        if (clip->quantity > 0)
        {
                gi.centerprintf (ent, "That clip is not empty, choose another\n");
                return;                
        }

        comb = FindCombo (ammo->pickup_name, clip->scomp, true);

        if (comb == NULL)
                bad = true;

        if (bad)
                BadCombo (ent);
        else
        {
                i = comb->quantity/15;
                if (i < 1)
                        i += 1;
                ent->delay_time = level.time + i;
                gi.centerprintf (ent, "Making new clip...\n");
                ent->combfunc = CombineToMakeClip;
        }
}

/*
===========
Use_Ammo - Switch between ammo types.
============
*/
void EmptyClip (edict_t *ent, gitem_t *item, gitem_t *itemagain)
{
        int     clipindex;

        clipindex = ent->client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))];
        if (clipindex < item->quantity)
        {
                Drop_Ammo_Specific (ent, FindItem(item->fcomp), clipindex);
                ent->client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] = 0;
        }
        else            
        {
                Drop_Ammo_Specific (ent, FindItem(item->fcomp), item->quantity);
                ent->client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] -= item->quantity;
        }

        ent->client->pers.inventory[ITEM_INDEX(item)]--;
        RemoveAmmoWeight (ent, item, 1);

        if (!Add_Ammo (ent, FindItem(item->scomp), 0))
        {
                gi.centerprintf (ent, "Clip emptied, but had no space\n");
                Drop_Ammo_Specific (ent, FindItem(item->scomp), 0);
                // return false;
        }
        else
                gi.centerprintf (ent, "Clip emptied\n");
	ValidateSelectedItem (ent);
}

void Use_Ammo2 (edict_t *ent, gitem_t *item)
{
        float i;

        if (ent->delay_time > level.time)
                return;

        if ((item->flags & IT_CLIP) && (item->quantity > 0))
                ent->combine = item;
        else if (ent->combine == NULL)
        {
                if (item->quantity < 1)
                        gi.centerprintf (ent, "Select ammo\n");
                else
                        gi.centerprintf (ent, "Select empty clip...\n");
                ent->combine = item;
                return;
        }
        ent->combinetwo = item;

        if ((item->flags & IT_CLIP) && (ent->combine == item))
        {
                if (item->quantity < 1)
                        BadCombo (ent);
                else
                {
                        gi.centerprintf (ent, "Emptying clip...\n");
                        i = item->quantity/15;
                        if (i < 1)
                                i += 1;
                        ent->delay_time = level.time + i;
                        ent->combfunc = EmptyClip;
                }
        }
        else if (item->flags & IT_CLIP)
                CheckClipCombine(ent, ent->combine, item);
        else
                CheckClipCombine(ent, item, ent->combine);
}

void Use_Ammo (edict_t *ent, gitem_t *item)
{
        if (ent->client->weaponstate != WEAPON_READY) // Must be idle
                return;         

        if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
                Use_Ammo2 (ent, item);
        else if (item->flags & IT_CLIP)
                Use_Clip(ent, item);
        ValidateSelectedItem (ent);
}

//===========================================================================

/*
================
SetupItems - In non-game mode, items will be lie on ground, not glow etc.
================
*/
// GRIM
void ExpAmmoExplode (edict_t *ent)
{
        vec3_t  origin;
        int     mod, radius;

        radius = ent->dmg_radius;

        mod = ent->modtype;

        T_RadiusDamage(ent, ent->enemy, ent->dmg, ent, radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
        if (ent->waterlevel)
	{
        	if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}

	gi.WritePosition (origin);
        gi.multicast (ent->s.origin, MULTICAST_PHS);

        VectorClear (ent->velocity); // Dirty

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM) && (deathmatch->value))
                SetRespawn (ent, 25);
        else
                G_FreeEdict (ent);
}

void ExpAmmoDie (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        ent->dmg = 150;
        ent->dmg_radius = 220;
        ent->takedamage = DAMAGE_NO;
        ent->think = ExpAmmoExplode;
        ent->nextthink = level.time + 0.1;
        if (ent->trapping && ent->trapping->item)
        {
                if (ent->trapping->spawnflags & DROPPED_PLAYER_ITEM)
                        ent->trapping->nextthink = level.time + 0.1;
                else
                {
                        SetRespawn (ent->trapping, 30);
                        ent->trapping->flags &= ~FL_RESPAWN;
                }
                ent->trapping->trapping = NULL;
        }
        ent->trapping = NULL;

        gi.linkentity (ent);
}
// GRIM

void ObjectDieThink (edict_t *ent)
{
        ThrowDebris (ent, "models/objects/debris3/tris.md2", 1, ent->s.origin);
        ThrowDebris (ent, "models/objects/debris2/tris.md2", 3, ent->s.origin);

        if ( (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) && deathmatch->value)
                SetRespawn (ent, 30);
        else
                G_FreeEdict (ent);
}

void ObjectDie (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        ent->dmg = damage;
        ent->takedamage = DAMAGE_NO;
        ent->think = ObjectDieThink;
        ent->nextthink = level.time + 0.1;
}

void SetupItems (edict_t *ent)
{
	float		*v;

        if ((!(ent->s.effects & EF_ROTATE)) && (!(ent->spawnflags & DROPPED_PLAYER_ITEM)))
                ent->s.angles[1] = random()*360;

        if ((ent->item->flags & IT_AMMO) && (ent->item->flags & IT_SPECIAL))
        {
                //VectorCopy(ent->s.origin, ent->corpse_pos);
                ent->solid = SOLID_BBOX;
                ent->mass = ent->item->weight;
                ent->health = 10;
                ent->die = ExpAmmoDie;
                ent->modtype = MOD_EXPLODING_AMMO;
                ent->takedamage = DAMAGE_YES;
                ent->trapping = NULL;

                v = tv(-15, -15, -15);
                VectorCopy (v, ent->mins);

                v = tv(15, 15, -7);
                VectorCopy (v, ent->maxs);
        }
}

/*
===========================================================================

New arrays and associated functions

===========================================================================
*/

/*
===============
FindCombo
===============
*/
gitem_t *FindCombo (char *first, char *second, qboolean anything)
{
        int     i;
        gitem_t *it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
                if (!it->fcomp)
			continue;

                if (!it->scomp)
			continue;

                if ((!Q_stricmp(it->fcomp, first)) && (!Q_stricmp(it->scomp, second)))
			return it;
                else if ((!Q_stricmp(it->fcomp, second)) && (!Q_stricmp(it->scomp, first)) && (anything == true))
			return it;
	}

	return NULL;
}

/*
===============
FindComboByName
===============
*/
gitem_t *FindComboByName (char *name)
{
	int		i;
        gitem_t *it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
                if (!it->fcomp)
			continue;

                if (!it->scomp)
			continue;

                if (!Q_stricmp(it->pickup_name, name))
			return it;
	}

	return NULL;
}

//==================================================================

/*
===============
FindClip
===============
*/
clips_t *FindClip (char *name)
{
	int		i;
        clips_t *it;

        it = cliplist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
                if (!it->name)
			continue;

                if (!Q_stricmp(it->name, name))
			return it;
	}

	return NULL;
}

/*
===============
Clip List - Keeps track of the amount of ammo in the clips.
===============
*/

// Dirty
clips_t cliplist[] = 
{
	{
		NULL
	},	// leave index 0 alone

        {
                "Pistol Clip",
        },

        {
                "Machinegun Magazine",
        },

        {
                "M4 Clip",
        },

	// end of list marker
	{NULL}
};
// Dirty


/*
=============== 
FindClientWeapon - find weapon info on a weapon NOT being used
===============
*/
weapons_t *FindClientWeapon (edict_t *ent, char *pickup_name, int occ)
{
        int             i, o;
        weapons_t       *it;

        if (ent->client == NULL)
                it = ent->weaponlist;
        else
                it = ent->client->pers.weaponlist;

        o = 0;
        for (i=0 ; i<9 ; i++, it++)
	{
                if (i < 3)
			continue;
                if (!it->slot_name)
			continue;
		if (!it->pickup_name)
			continue;
                if (!Q_stricmp(it->pickup_name, pickup_name))
                {
                        o++;
                        if (o >= occ)
                        {
                                //gi.dprintf ("FindClientWeapon - slot_name %s, pickup_name %s\n", it->slot_name, it->pickup_name);
                                return it;
                        }
                        else
                                continue;
                }
	}
        return NULL;
}

/*
===============
FindSlot
===============
*/
weapons_t *FindSlot (edict_t *ent, char *slot_name)
{
        weapons_t       *it;
        int             i;

        if (ent->client == NULL)
                it = ent->weaponlist;
        else
                it = ent->client->pers.weaponlist;

        for (i=0 ; i<9 ; i++, it++)
	{
                if (!it->slot_name)
			continue;
                if (!Q_stricmp(it->slot_name, slot_name))
                {
                        //gi.dprintf ("FindSlot - slot_name %s, pickup_name %s\n", it->slot_name, it->pickup_name);
                        return it;
                }
	}
        return NULL;
}

/*
===============
FindBlank
===============
*/
weapons_t *FindBlank (edict_t *ent, int occ)
{
        int             i, o;
        weapons_t       *it;

        o = 0;
        if (ent->client == NULL)
                it = ent->weaponlist;
        else
                it = ent->client->pers.weaponlist;

        for (i=0; i < 9; i++, it++)
	{
                if (i < 4)
			continue;
                if (!it->slot_name)
			continue;
                if (!it->pickup_name)
                {
                        o++;
                        if (o >= occ)
                        {
                                //gi.dprintf ("FindBlank - Found : %s, PN = %s\n", it->slot_name, it->pickup_name);
                                return it;
                        }
                }
	}
        return NULL;
}

/*
===============
SetSlot
===============
*/
void SetSlot (edict_t *ent, char *slot_name, char *pickup_name, int flags, int damage, char *ammo, int quantity, char *s_ammo, int s_quantity)
{
        weapons_t       *wep;

        wep = FindSlot (ent, slot_name);

        wep->pickup_name = pickup_name;
        wep->flags = flags;
        wep->damage = damage;
        wep->ammo = ammo;
        wep->quantity = quantity;
        wep->s_ammo = s_ammo;
        wep->s_quantity = s_quantity;

        //gi.dprintf ("SetSlot - slot_name %s, pickup_name %s, flags %i, damage %i, ammo %s, quantity %i, s_ammo %s, s_quantity %i\n", wep->slot_name, wep->pickup_name, wep->flags, wep->damage, wep->ammo, wep->quantity, wep->s_ammo, wep->s_quantity);
}

weapons_t weaponlist[] =
{
	{
		NULL
	},	// leave index 0 alone

	{
/* Slot Name */         "Good Hand",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

	{
/* Slot Name */         "Off Hand",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

	{
/* Slot Name */         "Large Weapon Slot",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

        // Slot1 is on straight away...
        // place default weapon here unless it is large...
        {
/* Slot Name */         "Slot1",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

        {
/* Slot Name */         "Slot2",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

        {
/* Slot Name */         "Slot3",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

        {
/* Slot Name */         "Slot4",
/* Weapon Name */       NULL,
                        0,
                        0,
/* Ammo */              NULL,
                        0,
/* Ammo */              NULL,
                        0,
	},

	// end of list marker
	{NULL}
};

// GRIM - 12/01/99
edict_t *ScanForWeapon (char *classname)
{
        edict_t *spot = NULL;
        int     count = 0;

        while ((spot = G_Find (spot, FOFS(classname), classname)) != NULL)
		count++;
        
	if (!count)
		return NULL;

	spot = NULL;
	do
	{
                spot = G_Find (spot, FOFS(classname), classname);

                if ((spot->svflags & SVF_NOCLIENT) && (!(spot->flags & FL_RESPAWN)))
                        break;
        } while (count--);

        if (!spot)
                return NULL;

        // Just check again anyway...
        if (spot->flags & FL_RESPAWN)
                return NULL;

        if (!(spot->svflags & SVF_NOCLIENT))
                return NULL;

        return spot;
}

void WeaponsStayThink (edict_t *ent)
{
        edict_t *weapon = NULL;

        if ((int)bflags->value & BF_CHOOSE_STUFF)
        {
                G_FreeEdict (ent);
                return;
        }

        if (((int)dmflags->value & DF_WEAPONS_STAY) && (ent->item->flags & IT_BIG))
        {
                weapon = ScanForWeapon (ent->item->classname);

                if (weapon)
                {
                        SetRespawn (weapon, 5);
                        //gi.dprintf ("%s found\n", ent->item->classname);
                }
                else
                        gi.dprintf ("weapon not found\n");
        }

        G_FreeEdict (ent);
}

void RespawnThisWeapon (char *classname)
{
        edict_t *weapon = NULL;

        weapon = ScanForWeapon (classname);

        if (weapon)
        {
                SetRespawn (weapon, 30);
                //gi.dprintf ("%s found\n", classname);
        }
        else
                gi.dprintf ("weapon not found\n");
}

void CheckForWeapons(edict_t *ent)
{
        gitem_t         *item = NULL;

        if (ent->client->pers.largeweapon->pickup_name)
        {
                item = FindItem(ent->client->pers.largeweapon->pickup_name);
                if (item)
                        RespawnThisWeapon (item->classname);
        }

        if ((ent->client->pers.weapon_handling) && (ent->client->pers.weapon2->flags & IT_BIG))
                RespawnThisWeapon (ent->client->pers.weapon2->classname);

        if (ent->client->pers.weapon->flags & IT_BIG)
                RespawnThisWeapon (ent->client->pers.weapon->classname);
}
// GRIM

// Dirty
/*
==============================
Laser Sight
==============================
*/
qboolean Pickup_LaserSight (edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        gi.sound(other, CHAN_ITEM, gi.soundindex("misc/lasersight.wav"), 1, ATTN_NORM, 0);

        if (!other->client->pers.weapon_handling)
        {
                if ((other->client->pers.weapon == FindItem("MK23 Pistol"))
                 || (other->client->pers.weapon == FindItem("MP5/10 Submachinegun"))
                  || (other->client->pers.weapon == FindItem("M4 Assault Rifle")))
                        other->client->pers.specials |= SI_LASER_SIGHT;
        }

	return true;
}

void Drop_LaserSight (edict_t *ent, gitem_t *item)
{
        Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;

        ent->client->pers.specials &= ~SI_LASER_SIGHT;
        ent->client->pers.specials &= ~SI_LASER_SIGHT_HELP;

        RemoveItemWeight (ent, item);
	ValidateSelectedItem (ent);
}

/*
==============================
Flash Light
==============================
*/
void Use_Light (edict_t *ent, gitem_t *item)
{
        if (ent->client->pers.specials & SI_FLASH_LIGHT)
                ent->client->pers.specials &= ~SI_FLASH_LIGHT;
        else
                ent->client->pers.specials |= SI_FLASH_LIGHT;
}

void Drop_Flashlight (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

        dropped = Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;

        if (ent->client->pers.specials & SI_FLASH_LIGHT)
                Use_Light (ent, NULL);

        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

qboolean Pickup_Flashlight (edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

        return true;
}

/*
==============================
Stealth Slippers
==============================
*/
qboolean Pickup_Slippers (edict_t *ent, edict_t *other)
{
        other->client->pers.specials |= SI_STEALTH_SLIPPERS;
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        return true;
}

void Drop_Slippers (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

        dropped = Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
        ent->client->pers.specials &= ~SI_STEALTH_SLIPPERS;
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

/*
==============================
Stealth Slippers
==============================
*/
qboolean Pickup_Silencer (edict_t *ent, edict_t *other)
{
        other->client->pers.specials |= SI_SILENCER;
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        gi.sound(other, CHAN_ITEM, gi.soundindex("misc/screw.wav"), 1, ATTN_NORM, 0);
        return true;
}

void Drop_Silencer (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

        dropped = Drop_Item (ent, item, ITEM_UNDEF);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
        ent->client->pers.specials &= ~SI_SILENCER;
        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

void CheckForSpecials(edict_t *ent)
{
        if (ent->client->pers.armor)
                Drop_ArmorQuick(ent);

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Sight"))])
        {
                ent->client->v_angle[YAW] -= 30;
                Drop_LaserSight (ent, FindItem("Laser Sight"));
                ent->client->v_angle[YAW] += 30;
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Flash Light"))])
        {
                ent->client->v_angle[YAW] -= 15;
                Drop_Flashlight (ent, FindItem("Flash Light"));
                ent->client->v_angle[YAW] += 15;
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Slippers"))])
        {
                ent->client->v_angle[YAW] += 15;
                Drop_Slippers (ent, FindItem("Stealth Slippers"));
                ent->client->v_angle[YAW] -= 15;
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))])
        {
                ent->client->v_angle[YAW] += 20;
                Drop_Bandolier (ent, FindItem("Bandolier"));
                ent->client->v_angle[YAW] -= 20;
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Silencer"))])
        {
                ent->client->v_angle[YAW] += 40;
                Drop_Silencer (ent, FindItem("Silencer"));
                ent->client->v_angle[YAW] -= 40;
        }
}

/*
====================
SpawnRPI (Random Placement Item)
====================
*/
void SpawnRPI (gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
        vec3_t  forward, right, angles;

        if (spot == NULL)
                return;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
        ent->item->flags |= IT_RAND;
	ent->s.effects = item->world_model_flags;
	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);

	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

        if (!((int)realflags->value & RF_NO_GLOWING))
                ent->s.renderfx = RF_GLOW;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

        ent->nextthink = level.time + 60;
        ent->think = RPI_Think;

	gi.linkentity (ent);
}

void RPI_Think (edict_t *ent)
{
        edict_t *spot = NULL;

        if (!ent->item)
                return;

        spot = SelectRandomDMSpot();

        if (spot)
        {
                SpawnRPI (ent->item, spot);
                G_FreeEdict (ent);
	}
        else
        {
                ent->nextthink = level.time + 60;
                ent->think = RPI_Think;
	}
}

// Dirty
/*
====================
SpecialItems - Use random spawn points
====================
*/
void SpawnSetupItem (edict_t *ent, gitem_t *item)
{
        ent->classname = item->classname;
        gi.linkentity (ent);

        ED_CallSpawn (ent);
}

void SpawnSpecialItems(edict_t *ent)
{
	edict_t *spot;
        gitem_t *item;
	int i;

        if (((int)bflags->value & BF_CHOOSE_STUFF) || ((int)bflags->value & BF_JOHN_WOO))
                return;

        spot = NULL;
        i = 0;
        while (i < 6)
        {
                if (i == 0)
                        item = FindItem("Bandolier");
                else if (i == 1)
                        item = FindItem("Laser Sight");
                else if (i == 2)
                        item = FindItem("Silencer");
                else if (i == 3)
                        item = FindItem("Kevlar Vest");
                else if (i == 4)
                        item = FindItem("Stealth Slippers");
                else if (i == 5)
                        item = FindItem("Flash Light");

                if (sv_itemchoices->string && sv_itemchoices->string[0] && (!strstr (sv_itemchoices->string, "all")))
                {
                        if (!strstr (sv_itemchoices->string, item->classname))
                        {
                                i++;
                                continue;
                        }
                }

                if ((spot = SelectRandomDMSpot()) != NULL)
                        SpawnRPI (item, spot); // GRIM SpawnTech
                i++;
	}
}

void SpawnSpecials(edict_t *ent)
{
        SpawnSpecialItems(ent);
	G_FreeEdict(ent);
}

void SetupSpecialsSpawn(void)
{
	edict_t *ent;

        if (((int)bflags->value & BF_CHOOSE_STUFF) || ((int)bflags->value & BF_JOHN_WOO))
                return;

	ent = G_Spawn();
        ent->nextthink = level.time + 3;
        ent->think = SpawnSpecials;
}
// Dirty

