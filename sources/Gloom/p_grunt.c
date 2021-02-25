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

void GruntStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Pistol");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        item = FindItem("9mm Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;

        item = FindItem("Health");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        ent->client->resp.selected_item = i;

        item = FindItem("Autogun Clip");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 3;
        ent->client->resp.shot_type = i;

        ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity; // 40, was 30 in 1.1e

        item = FindItem("Autogun");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;

        ent->client->weapon = item;
}

int GruntHeal (edict_t *ent, edict_t *healer)
{
        const gitem_t *item;
        int i;
        //gitem_armor_t *newinfo;
        float wait = 0;
        int armor = 0;
        char *msg = NULL;

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

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Autogun Clip"))] == 3 &&
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == 1 &&
                (!(healer && healer->client)))
                return 2;

        if (ent->client->healwait > level.time && !(healer && healer->client))
                 return 0;

        if ((healer && healer->client) &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Autogun Clip"))] == 3 &&
                ((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
                ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
                ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0))) {
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

        item = FindItem("Autogun Clip");
        i = ITEM_INDEX(item);
        wait+=(3-ent->client->resp.inventory[i])*1;
        ent->client->resp.inventory[i] = 3;

        item = FindItem("9mm Clip");
        i = ITEM_INDEX(item);

        if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0) {
                wait+=(1-ent->client->resp.inventory[i])*5;
                ent->client->resp.inventory[i] = 1;
        } else {
                wait+=(2-ent->client->resp.inventory[i])*5;
                ent->client->resp.inventory[i] = 2;
        }
        
        if (!healer || !healer->client)
                ent->client->healwait=wait+level.time;

        if (healer && healer->client)
                gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

        return 1;
}

// FIXME: implement this in GenericVoice
void GruntVoice(edict_t *self, int n)
{
        switch(n)
        {
        case VOICE_SPAWN:
#ifdef CACHE_CLIENTSOUNDS
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(grunt_voice1+(randomMT()%3)), 0.7);
#else
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex(va("voice/grunt/spawn%i.wav", (randomMT()%3)+1)), 0.7);
#endif
                break;
        case VOICE_HELP:
#ifdef CACHE_CLIENTSOUNDS
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(grunt_voice4+(randomMT()&1)), 1.0);
#else
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex(va("voice/grunt/help%i.wav", (randomMT()&1)+1)), 1.0);
#endif
                break;
        case VOICE_ORDER:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(grunt_voice6+(randomMT()%4)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/grunt/order%i.wav", (randomMT()%4)+1)), 1, ATTN_NORM);
#endif
                break;
        case VOICE_AFFIRM:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(grunt_voice10+(randomMT()%3)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/grunt/affirm%i.wav", (randomMT()%3)+1)), 1, ATTN_NORM);
#endif
                break;
        case VOICE_DEFEND:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(grunt_voice13+(randomMT()%3)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/grunt/defend%i.wav", (randomMT()%3)+1)), 1, ATTN_NORM);
#endif
                break;
        case VOICE_NOAMMO:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(grunt_voice16+(randomMT()%3)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/grunt/noammo%i.wav", (randomMT()%3)+1)), 1, ATTN_NORM);
#endif
                break;
        case VOICE_TAUNT:
#ifdef CACHE_CLIENTSOUNDS
                broadcastUSound(self, SoundIndex(grunt_voice19+(randomMT()%3)), 1, ATTN_NORM);
#else
                broadcastUSound(self, gi.soundindex(va("voice/grunt/taunt%i.wav", (randomMT()%3)+1)), 1, ATTN_NORM);
#endif
                break;
        }
}
