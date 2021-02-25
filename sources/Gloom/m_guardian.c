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

void GuardianStart(edict_t *ent)
{
        if (spiketest->value)
                ent->client->resp.inventory[ITEM_SPIKESPORE] = 1;
        else
                ent->client->resp.inventory[ITEM_SPIKESPORE] = 2;

        ent->client->resp.selected_item = ITEM_SPIKESPORE;
        ent->client->last_move_time = level.time + 3;

        if (spiketest->value)
                ent->client->healwait = 120;

        ent->client->teammodelindex = gi.modelindex("players/guardian/tris.md2");
        ent->client->teamrf = RF_IR_VISIBLE | RF_TRANSLUCENT;
}

int GuardianHeal(edict_t *ent, edict_t *healer)
{
        float wait = 0;

        ent->client->last_move_time = level.time + 1;

        if (ent->client->healwait > level.time) {
                if (spiketest->value) {
                        if (ent->client->resp.inventory[ITEM_SPIKESPORE])
                                return 2;
                } else {
                        if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 2)
                                return 2;
                }

                return 0;
        }

        if (spiketest->value) {
                if (ent->client->resp.inventory[ITEM_SPIKESPORE])
                        return 2;
        } else {
                if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 2)
                        return 2;
        }

        if (spiketest->value) {
                wait = 120;
                ent->client->resp.inventory[ITEM_SPIKESPORE] = 1;
        } else {
                wait = (2-(float)ent->client->resp.inventory[ITEM_SPIKESPORE])*30;
                ent->client->resp.inventory[ITEM_SPIKESPORE] = 2;
        }

        ent->client->resp.selected_item = ITEM_SPIKESPORE;
        ent->client->healwait = wait;

        return 1;
}

void GuardianRegen(edict_t *player) {
    if (!level.suddendeath) {
        if (player->s.modelindex == 0 && !(player->client->resp.upgrades & UPGRADE_CELL_WALL)) {
        //invisible guard gets a little bit of armor
            if (player->client->resp.inventory[ITEM_ARMOR_COMBAT] < 70 && player->client->last_reload_time < level.time) {
                player->client->resp.inventory[ITEM_ARMOR_COMBAT]++;
                player->client->last_reload_time = level.time + g_guardian_regen_speed->value;
            }
        }
    }
}

void GuardianMirrorThink(edict_t *ent) {
    ent->s.frame = ent->owner->s.frame;
    VectorCopy(ent->owner->s.origin, ent->s.origin);
    VectorCopy(ent->owner->pos1, ent->pos1);
    VectorCopy(ent->owner->pos2, ent->pos2);
    VectorCopy(ent->owner->pos3, ent->pos3);
    VectorCopy (ent->owner->s.angles, ent->s.angles);

    if (ent->owner->s.modelindex == 255 || ent->owner->client->resp.class_type != CLASS_GUARDIAN) {
        ent->owner->client->mirror_ent = NULL;
        G_FreeEdict(ent);
    }
   
    ent->nextthink = level.time+FRAMETIME*0.5f;
}

void GuardianMirrorSpawn(edict_t *player) {
    edict_t* mirrorguard;
    player->client->mirror_ent = mirrorguard = G_Spawn();
    mirrorguard->classname = "mirror";
    mirrorguard->s.modelindex = gi.modelindex("players/guardian/tris.md2");
    mirrorguard->s.effects |= EF_SPHERETRANS;
    mirrorguard->s.renderfx |= RF_IR_VISIBLE;
    mirrorguard->svflags |= SVF_SKIP_OWNER;
    mirrorguard->svflags |= SVF_SEND_OBS;
    mirrorguard->think = GuardianMirrorThink;
    mirrorguard->owner = player;
    ext.SetEntityMask(mirrorguard->s.number, 16 << player->client->resp.team);
    GuardianMirrorThink(mirrorguard);
    gi.linkentity(mirrorguard);
}
