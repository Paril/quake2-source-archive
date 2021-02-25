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

void BiotechStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        ent->client->resp.inventory[ITEM_AMMO_CELLS]=BIOTECH_MAX_CELLS;

        item = FindItem("Pistol");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("9mm Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;

        item = FindItem("Flash Grenade");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Flare");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 4;
        ent->client->resp.selected_item = i;

        item = FindItem("Health");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("Scattergun");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        ent->client->weapon = item;

        item = FindItem("Scattershot Clips");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;
        ent->client->ammo_index = i;

        ent->random = 0;

        // give bio ir ability
        ent->client->ps.rdflags |= RDF_IRGOGGLES;
}

int BiotechHeal (edict_t *ent, edict_t *healer)
{
        const gitem_t *item;
        int i;
        //gitem_armor_t *newinfo;
        float wait = 0;
        int armor = 0;
        char *msg = NULL;

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

        if (((ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] != 0 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == 1) ||
		(ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] == 0 &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == 2)) &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))] == 4 &&
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) &&
                ent->client->resp.inventory[ITEM_AMMO_CELLS] == BIOTECH_MAX_CELLS &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == 1 &&
                ( !(healer && healer->client)))
                return 2;

        if (ent->client->healwait > level.time && !(healer && healer->client))
                 return 0;

        if ((healer && healer->client) && 
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] != 0 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == 1) ||
		(ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] == 0 &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == 2)) &&
                //ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))] == 1 &&
                //ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))] == 4 &&
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) &&
                ent->client->resp.inventory[ITEM_AMMO_CELLS] == BIOTECH_MAX_CELLS) {
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

        if (!healer || !healer->client) {
                wait+=(1-ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))])*10;
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))] = 1;

                wait+=(4-ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))])*4;
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))] = 4;
        }

        item = FindItem("Scattershot Clips");
        i = ITEM_INDEX(item);

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] != 0) {
                wait+=(8-ent->client->resp.inventory[i]*8);
                ent->client->resp.inventory[i] = 1;
        } else {
                wait+=(16-ent->client->resp.inventory[i]*8);
                ent->client->resp.inventory[i] = 2;
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

        wait += (BIOTECH_MAX_CELLS-ent->client->resp.inventory[ITEM_AMMO_CELLS])/15;
        ent->client->resp.inventory[ITEM_AMMO_CELLS] = BIOTECH_MAX_CELLS;

        if (!healer || !healer->client)
                ent->client->healwait=wait+level.time;

        if (healer && healer->client)
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

        return 1;
}

// FIXME: implement this in GenericVoice
void BiotechVoice(edict_t *self, int n)
{
        switch(n)
        {
        case VOICE_SPAWN:
#ifdef CACHE_CLIENTSOUNDS
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(biotech_spawn), 0.7);
#else
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex("voice/biotech/spawn1.wav"), 0.7);
#endif
                break;
        case VOICE_HELP:
#ifdef CACHE_CLIENTSOUNDS
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(biotech_voice1+(randomMT()%3)), 1.0);
#else
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex(va("voice/biotech/help%i.wav", (randomMT()%3)+1)), 1.0);
#endif
                break;
        case VOICE_ORDER:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(biotech_voice4), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex("voice/biotech/order1.wav"), 1, ATTN_NORM);
#endif
                break;
        case VOICE_AFFIRM:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(biotech_voice5), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex("voice/biotech/affirm1.wav"), 1, ATTN_NORM);
#endif
                break;
        case VOICE_DEFEND:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(biotech_voice6), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex("voice/biotech/defend1.wav"), 1, ATTN_NORM);
#endif
                break;
        case VOICE_NOAMMO:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(biotech_voice7), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex("voice/biotech/noammo1.wav"), 1, ATTN_NORM);
#endif
                break;
        case VOICE_TAUNT:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(biotech_voice8+(randomMT()&1)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/biotech/taunt%i.wav", (randomMT()&1)+1)), 1, ATTN_NORM);
#endif
                break;
        }
}
