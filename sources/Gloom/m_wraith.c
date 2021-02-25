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
#include "m_wraith.h"

/*  Brian added this  */
void WraithStart(edict_t *ent)
{
        const gitem_t *item;
        int i;

        item = FindItem("Spit");
        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] = 5;

        ent->client->ammo_index=i;
        ent->client->resp.selected_item = 0;
        ent->flags |= FL_FLY;
}

int WraithHeal(edict_t *ent, edict_t *healer)
{
        float wait = 0;

        if (ent->client->healwait > level.time) {
                if (ent->client->resp.inventory[ent->client->ammo_index] == 5)
                        return 2;
                else
                        return 0;
        }

        if (ent->client->resp.inventory[ent->client->ammo_index] == 5)
                return 2;

        wait= (5-ent->client->resp.inventory[ent->client->ammo_index]) * 5;
        ent->client->resp.inventory[ent->client->ammo_index] = 5;
        ent->client->healwait=wait;
        return 1;
}

//r1: fixed wraith death
//    note that this routine should NOT directly alter the frame numbers
//    by ++ or anything, G_SetClientFrame handles that. This should only
//    set the current frame and end frame. That's why the other one was
//    messed up because both this and G_SetClientFrame were editing the
//    frame numbers (and the groundentity/= problems :))
void wraith_dead_think (edict_t *ent)
{
        //r1: must be on ground and not moving (or might 'bounce' off which could
        //    ass things up)
        if (ent->s.frame == WRAITH_FDIE_E && ent->velocity[2] == 0) {
                ent->s.frame = WRAITH_FDIE_ES;
                ent->client->anim_end = WRAITH_FDIE_EE;
                ent->think = NULL;
        } else
                ent->nextthink = level.time + FRAMETIME;
}
