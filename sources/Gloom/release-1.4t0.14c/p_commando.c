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

void CommandoStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Magnum BD");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Health");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2; // 1 1.1e

        item = FindItem("Magnum Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 0; // 3 1.1e

        item = FindItem("Magnum");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 8;

        //item = FindItem("Pistol");
        //i = ITEM_INDEX(item);
        //ent->client->resp.inventory[i] = 0;

        item = FindItem("C4 Explosive");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Frag Grenade");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2;
        ent->client->resp.selected_item = i;

        item = FindItem("SMG Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2; // 4 1.1e , 1 in 1.3
        ent->client->resp.shot_type = i;

        ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity; // 80, was 60 in 1.1e

        //item = FindItem("Timed Explosive");
        //i = ITEM_INDEX(item);
        //ent->client->resp.inventory[i] = 3;

        item = FindItem("Submachinegun");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        ent->client->weapon = item;
        ent->upgrade_time = 0;
}

int CommandoHeal (edict_t *ent, edict_t *healer)
{
        int clipz = 0;
        float wait = 0;
        int armor = 0;
        char *msg = NULL;
        int c4 = 0;

        if(healer && healer->client)
        {
                if (healer->client->resp.class_type == CLASS_ENGINEER) {
                        armor = RepairArmor (ent, NULL, 25, 0);
                        if (armor != -1)
                                msg = va("Report: Armor at %d", armor);
                        else
                                msg = va("Report: Armor full");
                } else if (healer->client->resp.class_type==CLASS_BIO) {
                        HealbyBiotech(ent,healer);
                        return 2;
                }       
        }
        
        if (SETTING_BF(inven_refill_flags) & irf_cmd_c4 && level.time > ent->client->resp.special_refill_time+120) c4 = 1;

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("SMG Clip"))] >= 2 &&
                (ent->client->resp.inventory[ITEM_AMMO_MAGNUM] == GetItemByIndex(ITEM_AMMO_MAGCLIP)->quantity || ent->client->resp.inventory[ITEM_AMMO_MAGCLIP] == 1) &&
                //(ent->client->resp.inventory[ITEM_INDEX(FindItem("Magnum Clip"))] == 1) &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Frag Grenade"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == 2 &&
                ent->client->resp.inventory[ITEM_C4] >= c4 &&
                (!(healer && healer->client)))
                return 2;

        if (ent->client->healwait > level.time && !(healer && healer->client))
                return 0;

        if ((healer && healer->client) && 
                ent->client->resp.inventory[ITEM_INDEX(FindItem("SMG Clip"))] >= 2 &&
                (ent->client->resp.inventory[ITEM_AMMO_MAGNUM] == GetItemByIndex(ITEM_AMMO_MAGCLIP)->quantity || ent->client->resp.inventory[ITEM_AMMO_MAGCLIP] == 1)
                //(ent->client->resp.inventory[ITEM_INDEX(FindItem("Magnum Clip"))] == 1)
                //&& ent->client->resp.inventory[ITEM_INDEX(FindItem("Frag Grenade"))] == 2
                )
                {
                if (armor == -1) {
                        strcat(msg,", repairs complete.");
                        gi.sound (ent, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_IDLE, 0);
                }
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);
                return 2;
        }

        if (!healer || !healer->client) {
                wait=(2-ent->client->resp.inventory[ITEM_HEALTH])*10;
                ent->client->resp.inventory[ITEM_HEALTH] = 2;
        }

        if (ent->client->resp.inventory[ITEM_AMMO_SUBSHOT] == 0 && ent->client->resp.inventory[ITEM_AMMO_SUBGUNCLIP] == 0)
                clipz = 3;
        else
                clipz = 2;

        wait += (clipz-ent->client->resp.inventory[ITEM_AMMO_SUBGUNCLIP])*12;
        ent->client->resp.inventory[ITEM_AMMO_SUBGUNCLIP] = clipz;


        if (ent->client->resp.inventory[ITEM_AMMO_MAGCLIP] == 0 && ent->client->resp.inventory[ITEM_AMMO_MAGNUM] == 0)
        {
                wait += (1-ent->client->resp.inventory[ITEM_AMMO_MAGCLIP]) * 16;
                ent->client->resp.inventory[ITEM_AMMO_MAGCLIP] = 1;
        }
        else
        {
                wait += (GetItemByIndex (ITEM_AMMO_MAGCLIP)->quantity - ent->client->resp.inventory[ITEM_AMMO_MAGNUM]) * 2;
                ent->client->resp.inventory[ITEM_AMMO_MAGNUM] = GetItemByIndex(ITEM_AMMO_MAGCLIP)->quantity;
        }

        if (!healer || !healer->client) {
                wait+=(2-ent->client->resp.inventory[ITEM_GRENADE])*10;
                ent->client->resp.inventory[ITEM_GRENADE] = 2;
                
                if (ent->client->resp.inventory[ITEM_C4] < c4) {
                  ent->client->resp.inventory[ITEM_C4]++;
                  wait += 10;
                  ent->client->resp.special_refill_time = level.time;
                }
        }
        
        if(wait>20)
                wait=20;

        if (!healer || !healer->client)
                ent->client->healwait=wait+level.time;

        if (healer && healer->client)
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

        return 1;
}
