/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "g_local.h"

void Use_Weapon (edict_t *ent, const gitem_t *inv);

void Weapon_Pistol (edict_t *ent);
void Weapon_Magnum (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_Autogun (edict_t *ent);
void Weapon_Subgun (edict_t *ent);
void Weapon_Rocketgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_PulseLaser (edict_t *ent);

void Toss_Grenade (edict_t *ent, const gitem_t *item);
void Toss_Ggrenade (edict_t *ent, const gitem_t *item);
void Toss_Fgrenade (edict_t *ent, const gitem_t *item);
void Toss_Flare (edict_t *ent, const gitem_t *item);
void Toss_Spore (edict_t *ent, const gitem_t *item);
void Toss_Spike_Spore (edict_t *ent, const gitem_t *item);
void Toss_C4 (edict_t *ent, const gitem_t *item);
edict_t *C4_Arm (edict_t* self);

gitem_armor_t jacketarmor_info  = { 25,  25, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info  = { 50,  50, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info    = {100, 100, .80, .60, ARMOR_BODY};
//gitem_armor_t exterminarmor_info= {100, 100, .80, .60, ARMOR_BODY};
gitem_armor_t mecharmor_info    = {600, 600, .80, .60, ARMOR_FULL};

gitem_armor_t powerarmor_info = {0, 100, .0, 1, ARMOR_BODY};


//r1: removed static, added extern to g_local, defined here.
//why? because the new CTFID code allows engis to view team
//armor count, and i wanted to show pretty pictures. etc.
//note: hope this doesn't fux up the linux build :E
/*int   jacket_armor_index;
int     combat_armor_index;
int     body_armor_index;
int     mech_armor_index;*/
//int   exterm_armor_index;

//not needed in CTFID code.
//static int    power_screen_index;
static int      power_shield_index;


/*
FindItemByClassname

*/
const gitem_t   *FindItemByClassname (char *classname)
{
        int             i;
        const gitem_t   *it;

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
FindItem

*/
const gitem_t   *FindItem (char *pickup_name)
{
        int             i;
        const gitem_t   *it;

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

// FIXME: remove this? fix this?
const gitem_t   *FindClip (int clip_type)
{
        int             i;
        const gitem_t   *it;

        it = itemlist;
        for (i=0 ; i<game.num_items ; i++, it++)
        {
                if (it->flags & IT_CLIP && it->clip_type == clip_type){
                        //gi.dprintf("findclip: returning %s\n", it->pickup_name);
                        return it;
                }
        }

        return NULL;
}

const gitem_t   *FindAmmo (const gitem_t *item)
{
        int             i;
        const gitem_t   *it;

        it = itemlist;
        for (i=0 ; i<game.num_items ; i++, it++)
        {
                if (it->flags & IT_AMMO && it->tag == item->tag) {
                        //gi.dprintf("findammo: returning %s\n", it->pickup_name);
                        return it;
                }
        }

        return NULL;
}

const gitem_t   *FindArmor (unsigned int type)
{
        int             i;
        const gitem_t   *it;

        it = itemlist;
        for (i=0 ; i<game.num_items ; i++, it++)
        {
                if (!(it->flags & IT_ARMOR))
                        continue;
                if (it->tag == type)
                        return it;
        }

        return NULL;
}

void DoRespawn (edict_t *ent)
{
        if (ent->team)
        {
                edict_t *master;
                int     count;
                int choice;

                master = ent->teammaster;

                for (count = 0, ent = master; ent; ent = ent->chain, count++)
                        ;

                choice = randomMT() % count;

                for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
                        ;
        }

        ent->svflags &= ~SVF_NOCLIENT;
        ent->solid = SOLID_TRIGGER;
        gi.linkentity (ent);

        // send an effect
        ent->s.event = EV_ITEM_RESPAWN;
}

/*qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
        other->client->resp.inventory[ITEM_INDEX(ent->item)]++;
        return true;
}*/










qboolean Add_Ammo (edict_t *ent, const gitem_t *item, int count)
{
        int                     index;
        int                     max;

        if (!ent->client)
                return false;

        max = count;

        index = ITEM_INDEX(item);

        if (ent->client->resp.inventory[index] == max)
                return false;

        ent->client->resp.inventory[index] += count;

        if (ent->client->resp.inventory[index] > max)
                ent->client->resp.inventory[index] = max;

        return true;
}


void Clip_Reload(edict_t *ent, const gitem_t *item)
{
        int clip_index, ammo_index;
        const gitem_t *ammo = NULL;
        const gitem_t *it;
        int i;

        if(!ent->client)
                return;

        if(!ent->client->weapon)
                return;

        if(ent->client->weaponstate == WEAPON_RELOADING)        // already reloading.
                return;

        // clip type has to match current weapon
        if (ent->client->weapon->clip_type != item->clip_type)
                return;

        clip_index = ITEM_INDEX(item);

        if(ent->client->resp.inventory[clip_index] <= 0)                // no clips left
                return;

        // find ammo for this clip
        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;

                // match ammo type?
                if (it->flags & IT_AMMO && item->tag == it->tag) {
                        ammo = it;
                        break;
                }
        }

        // clip found, but no ammo?!
        if (!ammo)
                return;

        // inventory index for the ammo
        ammo_index = ITEM_INDEX(ammo);

        if(ent->client->ammo_index != ammo_index)               // load new type of ammo, ditch old type
        {
                // remove old ammo
                ent->client->resp.inventory[ent->client->ammo_index] = 0;

                ent->client->resp.shot_type = clip_index;
                ent->client->ammo_index = ammo_index;
        }
        
        
        if(Add_Ammo (ent, ammo, item->quantity))        // gun is not full, actually do the reload.
        {
                if (!((int)dmflags->value & DF_INFINITE_AMMO))
                        ent->client->resp.inventory[clip_index]--;
                ValidateSelectedItem (ent);
                ent->client->weaponstate = WEAPON_RELOADING;

                if (item->active_sound)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex(item->active_sound), 0.8, ATTN_IDLE, 0);
        }

}

void NoAmmoWeaponChange (edict_t *ent);

// FIXME: this is POS
// clientcommand calls this
// clientbeginserverframe calls this with +use
// weapon_generic calls this on reload after findammoforgun
void Reload(edict_t *ent)
{
        const gitem_t *it;
        int i;

        if (ent->client->resp.target_hits) {
          if (ent->client->resp.target_shots)
            gi.cprintf (ent, PRINT_HIGH, "%2d of %2d -> %.1f%%.\n", ent->client->resp.target_hits, ent->client->resp.target_shots, ent->client->resp.target_hits*100.0f/ent->client->resp.target_shots);
          ent->client->resp.target_hits = ent->client->resp.target_shots = 0;
        }

        if (!ent->client->weapon || ent->health <= 0)
                return;

        // If current item is right for this gun, use it (provided we have some).
        /*if(ent->client->resp.selected_item != -1)
        if(ent->client->resp.inventory[ent->client->resp.selected_item])
        {
                it = &itemlist[ent->client->resp.selected_item];
                if (it->flags & IT_CLIP)
                if (ent->client->ammo_index == ITEM_INDEX(FindItem(it->ammo)))
                {
                        Clip_Reload(ent, it);
                        return;
                }
        }*/

        // do favor check
        // FIXME: add clip_type to client struct
/*      if (ent->client->resp.inventory[ent->client->ammo_index] > 0) {
                Clip_Reload (ent, &itemlist[ent->client->ammo_index]);
                return;
        }*/
        // find any clips for this weapon
        // FIXME: favor the ammo_index used before
        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;              

                // is a clip
                if (!(it->flags & IT_CLIP))
                        continue;

                // match clip type?
                if (ent->client->weapon->clip_type != it->clip_type)
                        continue;

                // use first type of CLIP we come to
                if(ent->client->resp.inventory[i])      
                {
                        Clip_Reload(ent, it);
                        //ent->client->resp.selected_item = i; // show remaining clips
                        break;
                }
        }

        //if we are here we couldn't reload. no ammo? if so next available weapon/
        if (ent->client->resp.inventory[ent->client->ammo_index] < 1)
                NoAmmoWeaponChange (ent);
}

void Auto_Reload(edict_t *ent)
{
  if (ent->client->pers.uflags & UFLAG_NOAUTORELOAD && ent->client->oldbuttons & BUTTON_ATTACK) return;
  Reload(ent);
}


void Use_Health (edict_t *ent, const gitem_t *item)
{
        int             quant = item->quantity;

        if(!ent->client->resp.inventory[ITEM_HEALTH])
                return;

        if(ent->health <= 0)
                return;

        //if (ent->pain_debounce_time + 3 > level.time)
        //      quant = 50;
        
//        ent->client->resp.parasited = 0;

        if (ent->client->acid_duration)
        {
           if (!((int)dmflags->value & DF_SLOW_HEALTH_PACK)) {
                ent->client->acid_damage = ent->client->acid_duration = 0;
                ent->client->acid_attacker = NULL;
                quant*.5;
           }
        }
        else if (ent->health+ent->client->resp.health_regen+ent->client->resp.health_boost >= ent->max_health && !(ent->client->resp.class_type == CLASS_GRUNT && ent->client->resp.upgrades & UPGRADE_STIMPACKS))
        {
                return;
        }

        //if (ent->client->healthinc + quant > ent->max_health)
        //      return;

        ent->client->resp.inventory[ITEM_HEALTH]--;

        ValidateSelectedItem (ent);

        gi.sound(ent, CHAN_AUTO, SoundIndex (items_l_health), 1, ATTN_NORM, 0);

        //ent->client->healthinc += quant;
        //ent->client->healthinc_wait = level.framenum;

        if (ent->health < ent->max_health) {
          if ((int)dmflags->value & DF_SLOW_HEALTH_PACK) {
            ent->client->resp.health_regen += quant;
            if (ent->client->resp.health_regen_time < level.time+0.09f)
              ent->client->resp.health_regen_time = level.time+0.09f;
          } else {
                ent->health += quant;

                if (ent->health > ent->max_health)
                        ent->health = ent->max_health;

                if (ent->client->resp.class_type == CLASS_GRUNT && (ent->client->resp.upgrades & UPGRADE_STIMPACKS))
                        ent->health += STIMPACK_HEALTH_AMOUNT;
          }
        }
}

/*int ArmorIndex (edict_t *ent)
{
        if (!ent->client)
                return 0;

        //gi.dprintf ("armor type %d, %d, %d, %d, %d\n",jacket_armor_index,combat_armor_index,body_armor_index,mech_armor_index,exterm_armor_index);

        if (ent->client->resp.inventory[jacket_armor_index] > 0)
                return jacket_armor_index;

        if (ent->client->resp.inventory[combat_armor_index] > 0)
                return combat_armor_index;

        if (ent->client->resp.inventory[body_armor_index] > 0)
                return body_armor_index;

        if (ent->client->resp.inventory[mech_armor_index] > 0)
                return mech_armor_index;

//      if (ent->client->resp.inventory[exterm_armor_index] > 0)
//              return exterm_armor_index;

        return 0;
}*/

int PowerArmorType (edict_t *ent)
{
        if (!ent->client)
                return POWER_ARMOR_NONE;

        if (!(ent->flags & FL_POWER_ARMOR))
                return POWER_ARMOR_NONE;

        if (ent->client->resp.inventory[power_shield_index] > 0)
                return POWER_ARMOR_SHIELD;

//      if (ent->client->resp.inventory[power_screen_index] > 0)
//              return POWER_ARMOR_SCREEN;

        return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, const gitem_t *item)
{
        if (ent->delay > level.time)
                return;

        if (ent->flags & FL_POWER_ARMOR)
        {
                ent->flags &= ~FL_POWER_ARMOR;
                gi.sound(ent, CHAN_AUTO, SoundIndex (misc_power2), 1, ATTN_IDLE, 0);
                ent->delay = level.time + .5f;
        }
        else
        {
                if (!ent->client->resp.inventory[ITEM_AMMO_CELLS])
                {
                        gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
                        return;
                }
                if (ent->client->resp.class_type == CLASS_EXTERM && ent->client->last_move_time > level.time) {
                        gi.cprintf (ent,PRINT_HIGH,"You must wait for your cells to cool down.\n");
                        return;
                }

                ent->delay = level.time + .5f;
                ent->flags |= FL_POWER_ARMOR;
                gi.sound(ent, CHAN_AUTO, SoundIndex (misc_power1), 1, ATTN_IDLE, 0);

                if (ent->client->resp.class_type == CLASS_EXTERM) {
                        ent->client->resp.inventory[ITEM_AMMO_CELLS] -= 45;
                        if (ent->client->resp.inventory[ITEM_AMMO_CELLS] < 1) {
                                ent->client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                                gi.cprintf(ent, PRINT_HIGH, "Power cells overheated!\n");
                                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_pulseout), 1, ATTN_NORM, 0);
                                ent->client->last_move_time = level.time + 5;
                                if (ent->client->resp.upgrades & UPGRADE_COOLANT)
                                        ent->client->last_move_time -= 2.5f;
                        }
                }
        }
}

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        qboolean        taken;

        if (!other->client)
                return;

        if (other->health < 1)
                return;         // dead people can't pickup

        if (!ent->item->pickup)
                return;         // not a grabbable item?

        taken = ent->item->pickup(ent, other);

        if (taken)
        {
                // flash the screen
                other->client->bonus_alpha = 0.25;      

                // show icon and name on status bar
                other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
                other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
                other->client->pickup_msg_time = level.time + 3.0f;

                gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
        } else {
                return;
        }

        ent->svflags |= SVF_NOCLIENT;
        gi.unlinkentity (ent);
}


//r1: handle keys on death
void Drop_Key (edict_t *self, const gitem_t *key)
{
        edict_t *ent;

        ent = self->client->resp.key;

        if (!ent) {
                gi.dprintf ("WARNING: LOST A KEY!\n");
                return;
        }

        //move the key to the new origin (note, we can't spawn a new one because we would
        //lose key-specific data such as style)
        VectorCopy (self->s.origin, ent->s.origin);

        //SpawnItem (ent, key);

        //throw them in some dir so multiple items won't land in the same place
        ent->velocity[0] = crandom() * 100;
        ent->velocity[1] = crandom() * 100;
        ent->velocity[2] = 200 + random() * 100;

        ent->svflags &= ~SVF_NOCLIENT;

        gi.linkentity (ent);

        self->client->resp.key = NULL;
}

void follow_owner (edict_t *self)
{
        if (!(self->target_ent->svflags & SVF_NOCLIENT) || !self->owner->inuse ||!self->owner->client->resp.key || self->owner->health < 1) {
                G_FreeEdict (self);
                return;
        }

        //self->s.angles[0] = self->owner->s.angles[0];
        self->s.angles[1] = self->owner->s.angles[1];

        VectorCopy (self->owner->s.origin, self->s.origin);
        self->s.origin[2] += self->owner->maxs[2];
        self->nextthink = level.time + FRAMETIME;
        gi.linkentity (self);
}

qboolean Pickup_Key (edict_t *key, edict_t *other)
{
        edict_t *icon;
        //int i;
        //i = ITEM_INDEX(key->item);

        //check class/team
        if (!(classtypebit[other->client->resp.class_type] & key->style))
                return false;

        //only allowed to carry one key at once
        if (other->client->resp.key)
                return false;

        SendToTeam (other->client->resp.team, "%s picked up the %s!\n", other->client->pers.netname, key->item->pickup_name);

        other->client->resp.key = key;

        icon = G_Spawn();
        icon->classname = "keyindicator";
        icon->solid = SOLID_NOT;
        icon->think = follow_owner;
        icon->nextthink = level.time + FRAMETIME;
        icon->owner = other;
        icon->target_ent = key;
        icon->s.renderfx |= RF_GLOW;
        icon->s.modelindex = key->s.modelindex;
        VectorCopy (other->s.origin, icon->s.origin);
        icon->s.origin[2] += icon->owner->maxs[2];
        gi.linkentity (icon);

        return true;
}

/*
droptofloor
*/
void droptofloor (edict_t *ent)
{
        trace_t         tr;
        vec3_t          dest;
        float           *v;

        v = tv(-15,-15,-15);
        VectorCopy (v, ent->mins);
        v = tv(15,15,15);
        VectorCopy (v, ent->maxs);

        if (ent->model)
                gi.setmodel (ent, ent->model);
        else if (ent->item->world_model)
                gi.setmodel (ent, ent->item->world_model);
        else {
                gi.dprintf ("REMOVED: ILLEGAL ENTITY %s at %s\n", ent->classname, vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        ent->solid = SOLID_TRIGGER;
        ent->movetype = MOVETYPE_TOSS;  
        ent->touch = Touch_Item;

        v = tv(0,0,-128);
        VectorAdd (ent->s.origin, v, dest);

        tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
        if (tr.startsolid)
        {
                gi.dprintf ("REMOVED: %s spawned in solid at %s\n", ent->classname, vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        VectorCopy (tr.endpos, ent->s.origin);

        gi.linkentity (ent);
}


/*
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
*/
void PrecacheItem (const gitem_t *it)
{
        char    *s, *start;
        char    data[MAX_QPATH];
        int             len;
        //gitem_t       *ammo;

        if (!it)
                return;

        if (it->pickup_sound)
                gi.soundindex (it->pickup_sound);
    if (it->active_sound)
        gi.soundindex (it->active_sound);
    if (it->world_model)
                gi.modelindex (it->world_model);
        if (it->view_model)
                gi.modelindex (it->view_model);
        if (it->icon)
                gi.imageindex (it->icon);

        // parse everything for its ammo
/*      not much use anymore -ank
        if (it->ammo && it->ammo[0])
        {
                ammo = FindItem (it->ammo);
                if (ammo != it)
                        PrecacheItem (ammo);
        }*/

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
                if (!strcmp(data+len-3, "md2") || !strcmp(data+len-3, "lmp"))
                        gi.modelindex (data);
                else if (!strcmp(data+len-3, "sp2"))
                        gi.modelindex (data);
                else if (!strcmp(data+len-3, "wav"))
                        gi.soundindex (data);
                if (!strcmp(data+len-3, "pcx") || !strcmp(data+len-3, "png"))
                        gi.imageindex (data);
        }
}



        //qboolean      (*pickup)(struct edict_s *ent, struct edict_s *other);
        //void          (*use)(struct edict_s *ent, struct gitem_s *item);
        //void          (*drop)(struct edict_s *ent, struct gitem_s *item);

/*
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
*/

void SpawnItem (edict_t *ent, const gitem_t *item)
{
        ent->item = item;
        ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
        ent->think = droptofloor;
        ent->s.effects = item->world_model_flags;
        ent->s.renderfx = RF_GLOW;

        if (st.classes)
                ent->style = st.classes;

        if (ent->model)
                gi.modelindex (ent->model);
}

/*
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
!!!!!!!!!!!!!! IF CHANGING ITEM LIST BE SURE TO UPDATE INDEXES IN G_LOCAL ITEMCACHE !!!!!!!!!!
*/

const gitem_t   itemlist[] = 
{
        {
                NULL
        },      // leave index 0 alone

        //
        // ARMOR
        //

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)*/
        {
                "item_armor_body", 
                NULL, //Pickup_Armor,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/armor/body/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_bodyarmor",
/* pickup */    "Heavy Armor",
/* width */             3,
                0,
                CLIP_NONE,
                IT_ARMOR,
                &bodyarmor_info,
                ARMOR_BODY,
//              100, 
/* precache */ "",
                i_bodyarmor
        },

        {
                "item_armor_mech", 
                NULL, //Pickup_Armor,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/armor/body/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_bodyarmor",
/* pickup */    "Mech Armor",
/* width */             3,
                0,
                CLIP_NONE,
                IT_ARMOR,
                &mecharmor_info,
                ARMOR_FULL,
//              600, 
/* precache */ "weapons/mechbeep.wav", //wrong place but who cares
                i_bodyarmor
        },

        {
                "item_armor_combat", 
                NULL, //Pickup_Armor,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/armor/combat/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_combatarmor",
/* pickup */    "Medium Armor",
/* width */             3,
                0,
                CLIP_NONE,
                IT_ARMOR,
                &combatarmor_info,
                ARMOR_COMBAT,
//              75, 
/* precache */ "",
                i_combatarmor
        },

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "item_armor_jacket", 
                NULL, //Pickup_Armor,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/armor/jacket/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_jacketarmor",
/* pickup */    "Light Armor",
/* width */             3,
                0,
                CLIP_NONE,
                IT_ARMOR,
                &jacketarmor_info,
                ARMOR_JACKET,
//              40, 
/* precache */ "",
                i_jacketarmor
        },

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "item_power_shield",
                NULL,//Pickup_PowerArmor,
                Use_PowerArmor,
                NULL,//Drop_PowerArmor,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/armor/shield/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_powershield",
/* pickup */    "Power Shield",
/* width */             0,
                60,
                CLIP_NONE,
                IT_ARMOR,
                &powerarmor_info,
                AMMO_NONE,
//              80, 
/* precache */ "misc/power2.wav misc/power1.wav",
                i_powershield
        },

        //
        // WEAPONS 
        //
/* weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
        "weapon_pistol",
        NULL,//
        Use_Weapon,
        NULL,             
        Weapon_Pistol,
                NULL,
                "weapons/pistact.wav",
                NULL,0,//"models/weapons/g_shotg/tris.md2", 0,
                "models/weapons/v_pist/tris.md2",
/* icon */              "i_pistol",
/* pickup */    "Pistol",     
                0,
                0,
                CLIP_BULLETS,
                IT_WEAPON|IT_STAY_COOP,
                NULL,
                AMMO_NONE,
//              50, 
/* precache */ "weapons/pistgf1.wav weapons/pistgf2.wav weapons/pistact.wav weapons/pistload.wav",
                i_pistol
        },
/* weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
        "weapon_magnum",
        NULL,//NULL,//Pickup_Weapon,
        Use_Weapon,
        NULL,
        Weapon_Magnum,
                NULL,
                "weapons/actmag.wav",
                NULL,0,//"models/weapons/g_shotg/tris.md2", 0,
                "models/weapons/v_mag/tris.md2",
/* icon */              "i_magnum",
/* pickup */    "Magnum BD",    
                0,
                0,
                CLIP_MAGNUM,
                IT_WEAPON|IT_STAY_COOP,
                NULL,
                AMMO_NONE,
//              50, 
/* precache */ "weapons/pistgf1.wav weapons/pistgf2.wav weapons/pistact.wav weapons/pistload.wav",
                i_magnum
        },
/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "weapon_shotgun", 
        NULL,//NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_Shotgun,
                NULL,
                "weapons/spasact.wav",
                NULL,0,//"models/weapons/g_shotg/tris.md2", 0,
                "models/weapons/v_shot/tris.md2",
/* icon */              "i_autoshotgun",
/* pickup */    "Shotgun",
                0,
                0,
                CLIP_SHELLS,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              100, 
/* precache */ "weapons/shotgr1b.wav weapons/shotgf1b.wav",
                i_autoshotgun
        },

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "weapon_supershotgun", 
                NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_SuperShotgun,
                NULL,
                "weapons/spasact.wav",
                NULL,0,//"models/weapons/g_shotg2/tris.md2", 0,
                "models/weapons/v_spas/tris.md2",
/* icon */              "i_spas",
/* pickup */    "Scattergun",
                0,
                2,
                CLIP_SCATTER,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              150, 
/* precache */ "weapons/spasact.wav weapons/spasload.wav weapons/spasshot.wav",
                i_spas
        },

/*QUAKED weapon_autogun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/                              
        {
                "weapon_autogun", 
                NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_Autogun,
                NULL,
                "weapons/autoact.wav",
                NULL,0,//"models/weapons/g_machn/tris.md2", 0,
                "models/weapons/v_auto/tris.md2",
/* icon */              "i_autogun",
/* pickup */    "Autogun",
                0,
                1,
                CLIP_AUTO,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              170, 
        /* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav",
                i_autogun
        },

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "weapon_subgun", 
                NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_Subgun,
                NULL,
                "weapons/subact.wav",
                NULL,0,//"models/weapons/g_chain/tris.md2", 0,
                "models/weapons/v_sub/tris.md2",
/* icon */              "i_submachinegun",
/* pickup */    "Submachinegun",
                0,
                1,
                CLIP_SUB,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              120, 
/* precache */ "weapons/subfire.wav",
                i_submachinegun
        },

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                "weapon_rocketlauncher",
                NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_RocketLauncher,
                NULL,
                "weapons/rgact.wav",
                NULL,0,//"models/weapons/g_rocket/tris.md2", 0,
                "models/weapons/v_launch/tris.md2",
/* icon */              "i_rl",
/* pickup */    "Rocket Launcher",
                1,
                1,
                CLIP_ROCKETS,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              300, 
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav",
                i_rl
        },

        {
                "weapon_pulselaser",
                NULL,//NULL,//Pickup_Weapon,
                Use_Weapon,
                NULL,
                Weapon_PulseLaser,
                NULL,
                NULL,
                NULL,0,//"models/weapons/g_rocket/tris.md2", 0,
                "models/weapons/v_plas/tris.md2",
/* icon */              "i_plasmagun",
/* pickup */    "Pulse Laser",
                0,
                1,
                CLIP_CELLS,
                IT_WEAPON|IT_STAY_COOP|IT_PRIMARY,
                NULL,
                AMMO_NONE,
//              170, 
/* precache */ "weapons/pulsfire.wav weapons/up.wav weapons/up_end.wav",
                i_plasmagun
        },


        //
        // AMMO ITEMS - ammo is not found in game, only in inventory.
        // Firing lowers ammo
        // Clips are found and can be used to replace ammo.
        // Clips - quantity is number of bullets per clip (when used)
        // Clips - ammo is the type of ammo (pistol, shotgun, ect..)
        // When ammo is used, it also sets the max_ and shot_type_ fields
        //              in client->pers

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,           
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,  
/* icon */              "i_shellclip",
/* pickup */    "Shells",
/* width */             3,
/* quantity*/   0,
                CLIP_SHELLS,
                IT_AMMO,
                NULL,
                AMMO_SHELLS,
//              5, 
/* precache */ "",
                i_shellclip
        },

        {
                NULL,
                NULL,           
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,  
/* icon */              "i_shellex",
/* pickup */    "EX Shells",
/* width */             3,
/* quantity*/   0,
                CLIP_SHELLS,
                IT_AMMO,
                NULL,
                AMMO_EXSHELLS,
//              5, 
/* precache */ "",
                i_shellex
        },

        {
                NULL,
                NULL,           
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,  
/* icon */              "a_shells",
/* pickup */    "Scattershot",
/* width */             3,
/* quantity*/   0,
                CLIP_SCATTER,
                IT_AMMO,
                NULL,
                AMMO_SCATTER,
//              5, 
/* precache */ "",
                a_shells
        },

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_bullets",
/* pickup */    "Bullets",
/* width */             3,
                0,
                CLIP_BULLETS,
                IT_AMMO,
                NULL,
                AMMO_BULLETS,
//              5, 
/* precache */ "",
                a_bullets
        },
/*QUAKED ammo_magnum (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "i_magclip",
/* pickup */    "Magnum",
/* width */             3,
                0,
                CLIP_MAGNUM,
                IT_AMMO,
                NULL,
                AMMO_MAGNUM,
//              5, 
/* precache */ "",
                i_magclip
        },
/*QUAKED ammo_autoshot (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_bullets",
/* pickup */    "Autoshot",
/* width */             3,
                0,
                CLIP_AUTO,
                IT_AMMO,
                NULL,
                AMMO_AUTO,
//              5, 
/* precache */ "",
                a_bullets
        },
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_bullets",
/* pickup */    "Subshot",
/* width */             3,
                0,
                CLIP_SUB,
                IT_AMMO,
                NULL,
                AMMO_SUB,
//              5, 
/* precache */ "",
                a_bullets
        },
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_acid",
/* pickup */    "Spit",
/* width */             3,
                0,
                CLIP_FUEL,
                IT_AMMO,
                NULL,
                AMMO_FUEL,
//              5, 
/* precache */ "",
                a_acid
        },
        /*  Mech autocannon  */
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
                "i_acammo",
                "Cannon Rounds",
                3,
                0,
                CLIP_CANNON,
                IT_AMMO,
                NULL,
                AMMO_CANNON,
//              5, 
                "",
                i_acammo
        },

        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_bullets",
/* pickup */    "Spikes",
/* width */             3,
                0,
                CLIP_SPIKES,
                IT_AMMO,
                NULL,
                AMMO_SPIKES,
//              5, 
/* precache */ "",
                a_bullets
        },
/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "a_cells",
/* pickup */    "Cells",
/* width */             3,
                0,
                CLIP_CELLS,
                IT_AMMO,
                NULL,
                AMMO_CELLS,
//              5, 
/* precache */ "",
                a_cells
        },

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "i_missile",
/* pickup */    "Rockets",
/* width */             3,
                0,
                CLIP_ROCKETS,
                IT_AMMO,
                NULL,
                AMMO_ROCKETS,
//              5, 
/* precache */ "",
                i_missile
        },

        /*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
        {
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL, 0,
                NULL,
/* icon */              "i_missile",
/* pickup */    "Mini Missiles",
/* width */             3,
                0,
                CLIP_ROCKETS,
                IT_AMMO,
                NULL,
                AMMO_ROCKETS,
//              5, 
/* precache */ "",
                i_missile
        },


/*QUAKED clip_9mm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/                              
        {
                "clip_9mm",
                NULL, //NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/pistload.wav",
                NULL,0,//"models/weapons/c_pist/tris.md2", 0,
                NULL,
/* icon */              "i_9mmclip",
/* pickup */    "9mm Clip",
/* width */             3,
/* Quantity */  13,
                CLIP_BULLETS,
                IT_CLIP,
                NULL,
                AMMO_BULLETS,
//              5, 
/* precache */ "",
                i_9mmclip
        },
        {
                "clip_magnum",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/magload.wav",
                NULL,0,//"models/weapons/c_pist/tris.md2", 0,
                NULL,
/* icon */              "i_magclip",
/* pickup */    "Magnum Clip",
/* width */             3,
/* Quantity */  8,
                CLIP_MAGNUM,
                IT_CLIP,
                NULL,
                AMMO_MAGNUM,
//              6, 
/* precache */ "",
                i_magclip
        },

        {
                "clip_shells",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/shotload.wav",
                NULL,0,//"models/items/ammo/shells/medium/tris.md2", 0,
                NULL,
/* icon */              "i_shellclip",
/* pickup */    "Shell Clip",
/* width */             3,
/* Quantity */  5,
                CLIP_SHELLS,
                IT_CLIP,
                NULL,
                AMMO_SHELLS,
//              5, 
/* precache */ "",
                i_shellclip
        },

        {
                "clip_shells_ex",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,//NULL,
                NULL,
                NULL,
                "weapons/shotload.wav",
                NULL,0,//"models/items/ammo/shells/medium/tris.md2", 0,
                NULL,
/* icon */              "i_shellex",
/* pickup */    "Shell Clip [EX]",
/* width */             3,
/* Quantity */  5,
                CLIP_SHELLS,
                IT_CLIP,
                NULL,
                AMMO_EXSHELLS,
//              10, 
/* precache */ "",
                i_shellex
        },

        {
                "clip_scatter",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/spasload.wav",
                NULL,0,//"models/items/ammo/shells/medium/tris.md2", 0,
                NULL,
/* icon */              "a_shells",
/* pickup */    "Scattershot Clips",
/* width */             3,
/* Quantity */  8,
                CLIP_SCATTER,
                IT_CLIP,
                NULL,
                AMMO_SCATTER,
//              1, 
/* precache */ "",
                a_shells
        },

        {
                "clip_autogun",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/autoload.wav",
                NULL,0,//"models/weapons/c_pist/tris.md2", 0,
                NULL,
/* icon */              "i_autoclip",
/* pickup */    "Autogun Clip",
/* width */             3,
/* shots/clip */        40,
                CLIP_AUTO,
                IT_CLIP,
                NULL,
                AMMO_AUTO,
//              4, 
/* precache */ "",
                i_autoclip
        },

        {
                "clip_rocket",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/misload.wav",
                NULL,0,//"models/items/ammo/shells/medium/tris.md2", 0,
                NULL,
/* icon */              "i_missile",
/* pickup */    "Rockets",
/* width */             3,
/* shots/clip */        1,
                CLIP_ROCKETS,
                IT_CLIP,
                NULL,
                AMMO_ROCKETS,
//              1, 
/* precache */ "",
                i_missile
        },


        {
                "clip_subgun",
                NULL, //Pickup_Clip,
                Clip_Reload,
                NULL,
                NULL,
                NULL,
                "weapons/subload.wav",
                NULL,0,//"models/weapons/c_pist/tris.md2", 0,
                NULL,
/* icon */              "i_subclip",
/* pickup */    "SMG Clip",
/* width */             3,
/* shots/clip */        60, // 80 in 1.3, 60 in 1.1d
                CLIP_SUB,
                IT_CLIP,
                NULL,
                AMMO_SUB,
//              4, 
                "",
                i_subclip
        },

        {
                "item_grenade", 
                NULL,
                Toss_Grenade,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/grenade/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "gren_i",
/* pickup */    "Frag Grenade",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_GRENADE,
//              3,
                "",
                gren_i
        },

        {
                "item_ggrenade", 
                NULL,
                Toss_Ggrenade,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/ggrenade/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "ggren_i",
/* pickup */    "Smoke Grenade",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_GRENADE,
//              3,
                "",
                ggren_i
        },

        {
                "item_fgrenade", 
                NULL,
                Toss_Fgrenade,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/ggrenade/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "ggren_i",
/* pickup */    "Flash Grenade",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_GRENADE,
                "",
                ggren_i
        },

        {
                "item_flare", 
                NULL,
                Toss_Flare,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/ggrenade/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "flare_i",
/* pickup */    "Flare",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_GRENADE,
//              3,
                "",
                flare_i
        },

        {
                "item_c4", 
                NULL,
                Toss_C4,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/c4/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "c4_i",
/* pickup */    "C4 Explosive",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_C4,
                "",
                c4_i
        },

        {
                "item_spore", 
                NULL,
                Toss_Spore,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/spore/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_spore",
/* pickup */    "Spore",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_SPORE,
//              3,
/* precache */ "",
                i_spore
        },

        {
                "item_spike_spore", 
                NULL,
                Toss_Spike_Spore,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/objects/sgrenade/tris.md2", EF_ROTATE,
                NULL,
/* icon */              "i_sspore",
/* pickup */    "Spike Spore",
/* width */             2,
                60,
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_SPORE,
//              3,
/* precache */ "",
                i_sspore
        },

        {
                "item_health",
                NULL,
                Use_Health,
                NULL, //Drop_General,
                NULL,
                NULL,
                NULL,
                NULL,0,//"models/items/healing/medium/tris.md2", 0,
                NULL,
/* icon */              "i_health",
/* pickup */    "Health",
/* width */             3,
                80,     // health it gives
                CLIP_NONE,
                IT_POWERUP,
                NULL,
                AMMO_NONE,
/* precache */ "",
                i_health
        },

        //r1: mission key thingies
        {
                "+", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/pass/tris.md2", EF_ROTATE,   //model+flags
                NULL,   //view model
                "k_security",   //icon
                "Security Card",        //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        k_security
        },


        {
                "key_airstrike_target", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/target/tris.md2", EF_ROTATE, //model+flags
                NULL,   //view model
                "i_airstrike",  //icon
                "Beacon",       //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        i_airstrike
        },

        {
                "key_pyramid", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/pyramid/tris.md2", EF_ROTATE,        //model+flags
                NULL,   //view model
                "k_pyramid",    //icon
                "Plutonium",    //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        k_pyramid
        },

        {
                "key_power_cube", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/power/tris.md2", EF_ROTATE,  //model+flags
                NULL,   //view model
                "k_powercube",  //icon
                "Energy Cube",  //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        k_powercube
        },

        {
                "key_data_cd", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/data_cd/tris.md2", EF_ROTATE,        //model+flags
                NULL,   //view model
                "k_datacd",     //icon
                "Data DVD",     //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        k_datacd
        },

        {
                "key_red_key", //classname
                Pickup_Key, //pickup
                NULL,           //use
                Drop_Key,       //drop
                NULL,           //weapon
                "items/pkup.wav",       //sound
                NULL,                           //sound
                "models/items/keys/red_key/tris.md2", EF_ROTATE,        //model+flags
                NULL,   //view model
                "k_redkey",     //icon
                "Red Keycard",  //pickup name
                2,      //pickup width
                0,      //quantity
                0,      //clip type
                IT_STAY_COOP|IT_KEY, //flags
                NULL,   //info
                0,      //tag
/* precache */ "",
        k_redkey
        },

        // end of list marker
        {NULL}
};



void InitItems (void)
{
        game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}

/*
SetItemNames

Called by worldspawn
*/
void SetItemNames (void)
{
        //int           i;
        //gitem_t       *it;

        //don't start from 0 or you'll get very confused.
        /*for (i=1 ; i<game.num_items ; i++)
        {
                it = &itemlist[i];
                if (!Q_stricmp (it->pickup_name, "Health") || !Q_stricmp (it->pickup_name, "Spore") || !Q_stricmp (it->pickup_name, "Spike Spore") || !Q_stricmp (it->pickup_name, "Mini Missiles") || it->flags & IT_KEY)
                        gi.configstring (CS_ITEMS+i, it->pickup_name);
        }*/

        //whee.
        gi.configstring (CS_ITEMS+ITEM_HEALTH, ((const gitem_t *)(itemlist+ITEM_HEALTH))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_SPORE, ((const gitem_t *)(itemlist+ITEM_SPORE))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_SPIKESPORE, ((const gitem_t *)(itemlist+ITEM_SPIKESPORE))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_AMMO_MINIMISSILES, ((const gitem_t *)(itemlist+ITEM_AMMO_MINIMISSILES))->pickup_name);

        gi.configstring (CS_ITEMS+ITEM_KEY_PASS, ((const gitem_t *)(itemlist+ITEM_KEY_PASS))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_KEY_AIRSTRIKE, ((const gitem_t *)(itemlist+ITEM_KEY_AIRSTRIKE))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_KEY_PYRAMID, ((const gitem_t *)(itemlist+ITEM_KEY_PYRAMID))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_KEY_CUBE, ((const gitem_t *)(itemlist+ITEM_KEY_CUBE))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_KEY_DATACD, ((const gitem_t *)(itemlist+ITEM_KEY_DATACD))->pickup_name);
        gi.configstring (CS_ITEMS+ITEM_KEY_REDKEY, ((const gitem_t *)(itemlist+ITEM_KEY_REDKEY))->pickup_name);

/*      jacket_armor_index = ITEM_INDEX(FindItem("Light Armor"));
        combat_armor_index = ITEM_INDEX(FindItem("Medium Armor"));
        body_armor_index   = ITEM_INDEX(FindItem("Heavy Armor"));
        mech_armor_index   = ITEM_INDEX(FindItem("Mech Armor"));
        //exterm_armor_index = ITEM_INDEX(FindItem("Exterminator Armor"));
        //power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
*/
//      power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
        power_shield_index = ITEM_ARMOR_POWERSHIELD;

        //armor items for RepairArmor
        // removed, as we can't change read only structs
        // don't add this back, the usage has changed
/*      classlist[CLASS_ENGINEER].armor = NULL;
        classlist[CLASS_GRUNT].armor = FindItem ("Light Armor");
        classlist[CLASS_BIO].armor = FindItem ("Light Armor");
        classlist[CLASS_SHOCK].armor = FindItem ("Medium Armor");
        classlist[CLASS_HEAVY].armor = FindItem ("Heavy Armor");
        classlist[CLASS_COMMANDO].armor = FindItem ("Medium Armor");
        classlist[CLASS_EXTERM].armor = FindItem ("Medium Armor");
        classlist[CLASS_MECH].armor = FindItem ("Mech Armor");*/

        //note: aliens don't call RepairArmor, but they might use these
        //      in future so I added them anyway.
        // yes they do, atleast drone, guard and stalk
/*      classlist[CLASS_BREEDER].armor = NULL;
        classlist[CLASS_HATCHLING].armor = NULL;
        classlist[CLASS_DRONE].armor = FindItem ("Medium Armor");
        classlist[CLASS_WRAITH].armor = NULL;
        classlist[CLASS_KAMIKAZE].armor = FindItem ("Light Armor");
        classlist[CLASS_STINGER].armor = NULL;
#ifdef GUARDIAN_REGEN_NEAR_NEST
        classlist[CLASS_GUARDIAN].armor = NULL;
#else
        classlist[CLASS_GUARDIAN].armor = FindItem ("Medium Armor");
#endif
        classlist[CLASS_STALKER].armor = FindItem ("Light Armor");
*/

        // GLOOM
        //helmet_armor_index = ITEM_INDEX(FindItem("Helmet"));
        //leg_armor_index = ITEM_INDEX(FindItem("Leg Armor"));
}

// Gloom

void Toss_Grenade (edict_t *ent, const gitem_t *item)
{
        vec3_t          angles, forward, start;
        int speed;

        if(ent->client->throw_time > level.time || ent->client->resp.primed)
                return;

        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->throw_time = level.time + 1.3f;
        ent->client->resp.can_respawn = false;

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;
        AngleVectors (angles, forward, NULL, NULL);

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        if (ent->client->resp.class_type == CLASS_COMMANDO)
        {
                ent->fly_sound_debounce_time = level.framenum + 20;
                if (ent->client->resp.upgrades & UPGRADE_CYBER_LIGAMENTS)
                        speed *= 1.5f;
        }

        fire_grenade (ent, start, forward, 350, speed, 3, 220);
        //fire_grenade (ent, start, forward, 350, speed, 3, 160);
}

void Toss_Ggrenade (edict_t *ent, const gitem_t *item)
{
        vec3_t          angles, forward, start;
        int speed;

        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->resp.can_respawn = false;

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;
        AngleVectors (angles, forward, NULL, NULL);

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        fire_grenade (ent, start, forward, 0, speed, 1.5, 256);
}

void Toss_Fgrenade (edict_t *ent, const gitem_t *item)
{
        edict_t *grenade;
        vec3_t          angles, forward, start;
        int speed;

        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->resp.can_respawn = false;

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;
        AngleVectors (angles, forward, NULL, NULL);

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        grenade = fire_grenade (ent, start, forward, 100, speed, 1.0, 150);
        grenade->s.sound = 0;
        grenade->classname = "fgrenade";
        grenade->enttype = ENT_FLASH_GRENADE;
}  

static void flare_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        BecomeExplosion1 (self);
}

void flare_think (edict_t *ent)
{
        edict_t *tent = NULL;

        if (ent->delay == 1 && ent->groundentity) {
                ent->s.effects = EF_BFG;
                ent->delay = 0;
        }

        while ((tent=findradius_c(tent,ent,350))) {
                if (tent->client->resp.team == TEAM_ALIEN) {
                        if (tent->client->resp.class_type == CLASS_GUARDIAN) {
                                tent->s.modelindex = 255;
                                tent->fly_sound_debounce_time = level.framenum + 10;
                        }
                }
        }

        ent->nextthink = level.time + .2f;

        if (ent->count++ > 225) //(45 sec)
                BecomeExplosion1 (ent);
}

void Toss_Flare (edict_t *ent, const gitem_t *item)
{
        edict_t         *flare;
        vec3_t          angles, start, forward, right, up;
        vec3_t          dir;
        edict_t *e, *first;
        int activeflares, besttime;

        int speed;

        // cmd_use_f and cmd_invuse_f already check for quantity in inventory
        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->resp.can_respawn = false;

        e = first = NULL;
        activeflares = 0;
        besttime = 0;

        while ((e = G_Find3 (e, ENT_FLARE)) != NULL) {
                if (e->owner == ent) {
                        if (e->count >= besttime) {
                                first = e;
                                besttime = e->count;
                        }
                        if (++activeflares == 4)
                                break;
                }
        }

        if (activeflares == 4)
                BecomeExplosion1 (first);

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        AngleVectors (angles, forward, NULL, NULL);
                                                   
        vectoangles (forward, dir);
        AngleVectors (dir, forward, right, up);

        flare = G_Spawn();
        VectorCopy (start, flare->s.origin);
        VectorScale (forward, speed, flare->velocity);
        VectorMA (flare->velocity, 200 + crandom() * 10.0, up, flare->velocity);
        VectorMA (flare->velocity, crandom() * 10.0, right, flare->velocity);
        VectorSet (flare->avelocity, 300, 300, 300);
        flare->movetype = MOVETYPE_BOUNCE;
        flare->clipmask = MASK_SHOT;
        flare->solid = SOLID_BBOX;
        flare->enttype = ENT_FLARE;
        flare->s.effects = EF_ROCKET;
        flare->s.renderfx |= RF_IR_VISIBLE;
        VectorClear (flare->mins);
        VectorClear (flare->maxs);

        flare->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");

        flare->owner = ent;

        flare->takedamage = DAMAGE_YES;
        flare->health = 50;
        flare->die = flare_die;

        flare->delay = 1;
        flare->nextthink = level.time + 2;
        flare->think = flare_think;
        flare->classname = "flare";

        gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_hgrent1a), 1, ATTN_NORM, 0);

        gi.linkentity (flare);
}
                        
void Toss_Spore (edict_t *ent, const gitem_t *item)
{
        int                     speed;
        vec3_t          angles, forward, start;

        if(ent->client->throw_time > level.time)
                return;

        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->resp.can_respawn = false;
        ent->client->throw_time = level.time + 0.5f;

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;
        AngleVectors (angles, forward, NULL, NULL);

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        //fire_grenade (ent, start, forward, 0, 500, 1.5, 256);
        fire_spore (ent, start, forward, speed, 1.5);
}

void Toss_Spike_Spore (edict_t *ent, const gitem_t *item)
{
        int                     speed;
        vec3_t          angles, forward, start;

        if(ent->client->throw_time > level.time)
                return;

        //if(ent->client->resp.inventory[ITEM_INDEX(item)] <= 0)
        //      return;

        ent->client->resp.can_respawn = false;

        ent->client->throw_time = level.time + SPIKE_GRENADE_THROW_DELAY;       
        ent->s.modelindex = 255;
        ent->fly_sound_debounce_time = level.framenum + 20;  //  make guardian visible

        ent->client->resp.inventory[ITEM_INDEX(item)]--;
        //ValidateSelectedItem (ent);

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += 16;
        AngleVectors (angles, forward, NULL, NULL);

        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                speed = 100;
        else
                speed = 500;

        if (ent->client->resp.class_type == CLASS_GUARDIAN) {
                fire_spike_proxy_grenade (ent, start, forward, speed, 30,100,0);
        } else {
                fire_spike_grenade (ent, start, forward, speed, SPIKE_GRENADE_EXPLODE_DELAY, 60, 300);
        }
}

void Toss_C4 (edict_t *ent, const gitem_t *item)
{
        if(ent->client->throw_time > level.time)
                return;

        if(!ent->client->resp.primed)
        {
                /*if (ent->velocity[0] || ent->velocity[1])
                {
                        gi.cprintf (ent, PRINT_HIGH, "You must be standing still to prime C4!\n");
                        return;
                }*/
                ent->client->ps.stats[STAT_PICKUP_ICON] = imagecache[c4_i];
                ent->client->ps.stats[STAT_PICKUP_STRING] = CS_GENERAL+game.maxclients+2;
                ent->client->pickup_msg_time = level.time + 3.5f;

                //gi.cprintf (ent, PRINT_HIGH, "Priming C4...\n");
                gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_keyboard), 1.0, ATTN_NORM, 0);
                ent->client->resp.primed = 1;
                ent->client->throw_time = level.time + 3.5f;
                //ent->client->frozenmode = PMF_NO_PREDICTION;
                //ent->client->frozentime = 3.5;
        } else {
                ent->client->resp.inventory[ITEM_INDEX(item)]--;
                C4_Arm(ent);
                ent->client->resp.primed = 0;
        }
}
