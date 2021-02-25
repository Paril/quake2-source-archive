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
#include "m_stalker.h"

/*static int    sound_pain;
static int      sound_pain2;
static int      sound_death;
static int      sound_idle;
static int      sound_search;
static int      sound_sight;
static int      sound_die;                      */

void StalkStart(edict_t *ent)
{
        ent->client->resp.inventory[ITEM_AMMO_SPIKES] = 20;
        ent->client->ammo_index = ITEM_AMMO_SPIKES;

        ent->client->resp.inventory[ITEM_SPIKESPORE] = 3;
        ent->client->resp.selected_item = ITEM_SPIKESPORE;
}

int StalkHeal(edict_t *ent, edict_t *healer)
{
        float wait = 0;
        int zero = 0;
        int two = 2;

        float clamp = (ent->client->resp.upgrades & UPGRADE_SPIKE_SAC)?1.4f:1.9f;
       if (ent->random > level.time+clamp) //grim style hijacking, "lets random!"
         ent->random = level.time+clamp;   //clamp spike regen delay to a lower value when touching healer

        if (ent->client->healwait > level.time) {
                if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 3)
                        return two;
                else
                        return zero;
        }

        if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 3)
                return two;

        /*wait=(20.0-(float)ent->client->resp.inventory[i]);
        ent->client->resp.inventory[i] = 20;

        item = FindItem("Spike Spore");
        i = ITEM_INDEX(item);*/

        wait+=(3-(float)ent->client->resp.inventory[ITEM_SPIKESPORE])*13;

        ent->client->resp.inventory[ITEM_SPIKESPORE] = 3;
        ent->client->resp.selected_item = ITEM_SPIKESPORE;
        ent->client->healwait = wait;

        return 1;
}

