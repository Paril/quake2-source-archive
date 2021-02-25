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

void StingStart(edict_t *ent)
{
        int i;
        const gitem_t *item;

        item = FindItem("Spit");
        i = ITEM_INDEX(item);

        ent->client->resp.inventory[i] = 10;
        ent->client->ammo_index = i;

        item = FindItem("Spore");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 2;
        ent->client->resp.selected_item = i;
}

#define MAX_STINGER_GAS_SPORES  2

int StingHeal(edict_t *ent, edict_t *healer)
{
        int i;
        const gitem_t *item;
        float wait = 0;

        item = FindItem("Spore");
        i = ITEM_INDEX(item);

        if (ent->client->healwait > level.time) 
        {
                if (ent->client->resp.inventory[ent->client->ammo_index] == 10 && ent->client->resp.inventory[i] == MAX_STINGER_GAS_SPORES)
                        return 2;
                else
                        return 0;
        }

        if (ent->client->resp.inventory[ent->client->ammo_index] == 10 && ent->client->resp.inventory[i] == MAX_STINGER_GAS_SPORES)
                return 2;

        if (ent->client->resp.inventory[ent->client->ammo_index] < 10) 
        {
                ent->client->resp.inventory[ent->client->ammo_index] = 10;
                wait=5;
        }

        wait += (MAX_STINGER_GAS_SPORES - ent->client->resp.inventory[i]) * 10;
        ent->client->resp.inventory[i] = MAX_STINGER_GAS_SPORES;
        ent->client->resp.selected_item = i;
        ent->client->healwait=wait;
        return 1;
}
