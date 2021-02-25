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

void ShotgunStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Shell Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 3;
        ent->client->resp.shot_type = i;
        
        if ((ent->client->resp.upgrades & UPGRADE_EQPT_KIT) && ent->client->resp.inventory[ITEM_INDEX(FindAmmo(FindItem("Shell Clip [EX]")))] > 0) {
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip"))]++;
                stuffcmd(ent, "use shell clip");
        }
        else if (!(ent->client->resp.upgrades & UPGRADE_EQPT_KIT)) {
            ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;
        }

        ent->client->resp.selected_item = i;

        item = FindItem("Shell Clip [EX]");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Smoke Grenade");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2;

        item = FindItem("Pistol");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Health");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("9mm Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1; // 2 1.1e
        if (!(ent->client->resp.upgrades & UPGRADE_EQPT_KIT))
            ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;

        item = FindItem("Shotgun");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        if (!ent->client->weapon) ent->client->weapon = item;
        ent->upgrade_time = 0;
}

int ShotgunHeal (edict_t *ent, edict_t *healer)
{
        const gitem_t *item;
        int i;
//      gitem_armor_t   *newinfo;
        float wait=0;
        int armor = 0;
        char *msg = NULL;
        int ex = 0;

        if(healer && healer->client)
        {
                if (healer->client->resp.class_type==CLASS_ENGINEER) {
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

        item = FindItem("Shell Clip [EX]");
        i = ITEM_INDEX(item);

        if (SETTING_BF(inven_refill_flags) & irf_st_exshell && !ent->client->resp.inventory[ITEM_AMMO_EXSHELLS] && level.time > ent->client->resp.special_refill_time+90) ex = 1;

        // FIXME: Shell Clip [EX] beeps from depot
        if (((ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip"))] == 3 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] != 0) || (
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip"))] == 4 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] == 0)) &&
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Smoke Grenade"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == 1 &&
                //(ent->client->resp.inventory[ITEM_AMMO_EXSHELLS] || ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP]) &&
                (!(healer && healer->client)) &&
                ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP] >= ex
                )
                return 2;

        if (ent->client->healwait > level.time && !(healer && healer->client))
                return 0;

        if ((healer && healer->client) && 
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip"))] == 3 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] != 0) || (
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip"))] == 4 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] == 0)) && 
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) //&&
                //ent->client->resp.inventory[ITEM_INDEX(FindItem("Smoke Grenade"))] == 2 &&
                //((ent->client->resp.inventory[ent->client->ammo_index] > 0 && ent->client->resp.shot_type == i) || ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip [EX]"))] == 1)) {
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
                item = FindItem("Health");
                i = ITEM_INDEX(item);
                wait=(1-ent->client->resp.inventory[i])*10;
                ent->client->resp.inventory[i] = 1;
        }

        item = FindItem("Shell Clip");
        i = ITEM_INDEX(item);

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] != 0) {
                wait+=(3-ent->client->resp.inventory[i])*7; //was 10, then 8
                ent->client->resp.inventory[i] = 3;
        } else {
                wait+=(4-ent->client->resp.inventory[i])*7; //was 10, then 8
                ent->client->resp.inventory[i] = 4;
        }

        item = FindItem("9mm Clip");
        i = ITEM_INDEX(item);

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0) {
                wait+=(1-ent->client->resp.inventory[i])*5;
                ent->client->resp.inventory[i] = 1;
        } else {
                wait+=(2-ent->client->resp.inventory[i])*5;
                ent->client->resp.inventory[i] = 2;
        }

        /*if (!healer || !healer->client) {
                if (ent->client->resp.inventory[ITEM_AMMO_EXSHELLS] == 0 && ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP] == 0) {
                        ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP]++;
                        wait += 45;
                }
        }*/

        //item = GetItemByIndex(ent->client->resp.shot_type_shotgun);
        //if(Q_stricmp(item->pickup_name, "Shell Clip [EX]")!=0) {
        //item = FindItem("Shell Clip [EX]");
        //i = ITEM_INDEX(item);
        //gi.dprintf ("has %d of shell clip ex.",ent->client->resp.inventory[i]);
        //gi.dprintf ("has %d of inv ammo.",ent->client->resp.inventory[ent->client->ammo_index]);
        //item = GetItemByIndex(16);
//      gi.dprintf ("has %d of shot ammo.",ent->client->resp.inventory[ent->client->pers.]);
        //gi.dprintf ("has %d shell type (ex is %d).",ent->client->resp.shot_type_shotgun,i);
        //gi.dprintf ("has %d weap ID ",ITEM_INDEX(ent->client->weapon));
        
        
        //ex shells are dangerous enough as is, they don't need refills... perhaps once this
        //is fixed to not allow spare clips to be carried around it can be uncommented.

        /*if (ent->client->resp.inventory[i] == 0) {
//              if ((ent->client->resp.shot_type == i && ent->client->resp.inventory[ITEM_INDEX(FindItem("Shells"))] == 0) || (ent->client->resp.shot_type != i)) {
                        wait+=10;
                        ent->client->resp.inventory[i] = 1;
//              }
        }*/

        //}
        if (!healer || !healer->client) {
          item = FindItem("Smoke Grenade");
          i = ITEM_INDEX(item);
          wait+=(2-ent->client->resp.inventory[i])*11; //was 15, then 13
          ent->client->resp.inventory[i] = 2;
          
          if (ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP] < ex) {
            ent->client->resp.inventory[ITEM_AMMO_EXSHELLCLIP]++;
            wait += 5;
            ent->client->resp.special_refill_time = level.time;
          }

        }

        if (!(healer && healer->client))
                ent->client->healwait = wait + level.time;

        if (healer && healer->client)
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

        return 1;
}
