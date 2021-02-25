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
#include "m_breeder.h"

void BreedStart(edict_t *ent)
{

        //memset(ent->client->resp.inventory, 0, sizeof(ent->client->resp.inventory));

/*
        int i;
        const gitem_t *item;

        item = FindItem("Larvae");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 1;
        ent->client->resp.selected_item = i;
        ent->client->ammo_index=0;
*/
}

//void Spike_Explode (edict_t *ent);

void BreederDie(edict_t *ent)
{
//      edict_t *sg;
        //vec3_t        up;
        if(ent->s.frame == BREED_EXPLODE_E)
        {
                //int i;
                // Do blood explosion too
                ThrowGib (ent, ent->gib2, 100, GIB_ORGANIC);

                //for (i = 0; i < 10; i++)
                        //ThrowExplodeGib (ent, sm_meat_index, 50+random() * 50, GIB_ORGANIC); 
        

                if (bandwidth_mode->value >= 2)
                        ThrowGib (ent, ent->gib2, 100, GIB_ORGANIC);
        }

        //ThrowExplodeGib (ent, sm_meat_index, 50+random() * 50, GIB_ORGANIC); 
        //fire_spike_grenade (ent, ent->s.origin, vec3_origin, 10, 0, 0, 50);
        /*up[0] = crandom() * 100;
        up[1] = crandom() * 100;
        up[2] = 400;
        VectorNormalize (up);
        fire_spore (ent, ent->s.origin, up, 600, 5);*/
}
