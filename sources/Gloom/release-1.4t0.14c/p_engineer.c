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
#include "m_engineer.h"

void EngineerStart(edict_t *ent)
{               
        const gitem_t *item;
        int i;
                        
        ent->client->resp.inventory[ITEM_AMMO_CELLS] = 50;

        item = FindItem("Power Shield");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        ent->flags &= ~FL_POWER_ARMOR;
        item->use(ent, item);

        ent->client->resp.selected_item = i;

        ent->client->anim_priority = ANIM_PAIN;
        ent->client->anim_end = ENG_TELEPORT_E;
        ent->s.frame = ENG_TELEPORT_S;
}

int EngineerHeal(edict_t *ent, edict_t *healer)
{               

        if(healer && healer->client)
        {
                if (healer->client->resp.class_type == CLASS_ENGINEER) {
                        RepairArmor (ent, FindItem("Cells"), 30, 0);
                }else if (healer->client->resp.class_type==CLASS_BIO){
                        HealbyBiotech(ent,healer);
                        return 2;
                }       
        }

        //depot
        if (!healer && ent->health < ent->max_health) {
                ent->health += 10;
                if (ent->health > ent->max_health)
                        ent->health = ent->max_health;
        }

        if (healer && healer->client)
                ent->client->healwait = level.time;
        return 2;
}

void EngineerDie(edict_t *ent)
{
        if(ent->s.frame == ENG_EXPLODE_E)
        {
                ent->svflags = SVF_NOCLIENT;
                ent->solid = SOLID_NOT;
                ent->takedamage = DAMAGE_NO;
                gi.linkentity (ent);

                if (ent->client && ent->dmg)
                        T_RadiusDamage (ent, ent, ent->dmg, ent, ent->dmg_radius, MOD_R_SPLASH, 0);

                ent->dmg = 0;

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_EXPLOSION2);
                gi.WritePosition (ent->s.origin);
                gi.multicast (ent->s.origin, MULTICAST_PVS);
        }
}
