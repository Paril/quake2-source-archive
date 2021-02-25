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

void DroneStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Spit");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = DRONE_SPIT_MAX;

        ent->client->ammo_index = i;
}  

int DroneHeal(edict_t *ent, edict_t *healer)
{
        //float wait;
        //wait=(DRONE_SPIT_MAX-(float)ent->client->resp.inventory[ent->client->ammo_index]);

        if (ent->client->healwait > level.time) {
                if (ent->client->resp.inventory[ent->client->ammo_index] == DRONE_SPIT_MAX)
                        return 2;
                else
                        return 0;
        }

        if (ent->client->resp.inventory[ent->client->ammo_index] == DRONE_SPIT_MAX)
                return 2;

        ent->client->resp.inventory[ent->client->ammo_index] = DRONE_SPIT_MAX;
        ent->client->healwait = 5;
        return 1;

}
