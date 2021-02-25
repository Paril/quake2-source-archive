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

void ExterStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Power Shield");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;     
        ent->client->resp.selected_item = i;

        item = FindItem("Health");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2;

        item = FindItem("Frag Grenade");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 4;
        
        item = FindItem("Pulse Laser");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        ent->client->weapon = item;

        item=FindItem("Cells");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i]=60;

        ent->client->resp.shot_type = 60; // hax
        ent->random = 0;
}

int ExterHeal (edict_t *ent, edict_t *healer)
{
        const gitem_t *item;
        int i;
        int armor = 0;
        float wait = 0;
        char *msg = NULL;
        //gitem_armor_t *newinfo;

        if(healer && healer->client)
        {
                if (healer->client->resp.class_type == CLASS_ENGINEER) {
                        armor = RepairArmor (ent, NULL, 25, 0);
                        if (armor != -1)
                                msg = va("Report: Armor at %d", armor);
                        else
                                msg = va("Report: Armor full");
                }else if (healer->client->resp.class_type==CLASS_BIO){
                        HealbyBiotech(ent,healer);
                        return 2;
                }       
        }

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Frag Grenade"))] == 4 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == 2 &&
                (!(healer && healer->client)))
                return 2;

        if (ent->client->healwait > level.time && !(healer && healer->client))
                return 0;

        if ((healer && healer->client)) {
                //&& ent->client->resp.inventory[ITEM_INDEX(FindItem("Frag Grenade"))] == 4) {
                if (armor == -1) {
                        strcat(msg,", repairs complete.");
                        gi.sound (ent, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_IDLE, 0);
                }
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);
                return 2;
        }

        if (!healer || !healer->client) {
                item = FindItem("Health");
                i = ITEM_INDEX(item);
                wait=(2-ent->client->resp.inventory[i])*10;
                ent->client->resp.inventory[i] = 2;

                item = FindItem("Frag Grenade");
                i = ITEM_INDEX(item);
                wait+=(4-ent->client->resp.inventory[i])*10;
                ent->client->resp.inventory[i] = 4;
        }

        if(wait>20)
                wait=20;

        if (!healer || !healer->client)
                ent->client->healwait=wait+level.time;

        if (healer && healer->client)
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

        return 1;
}
