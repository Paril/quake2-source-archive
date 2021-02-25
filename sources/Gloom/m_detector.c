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

#define TRIGGER_DISTANCE                350

void detector_think(edict_t *self)
{
        edict_t *ent = NULL;

        while ( (ent=findradius_c (ent,self,TRIGGER_DISTANCE) ) != NULL ) {

                if(ent->client->resp.team != TEAM_ALIEN)        // only on aliens
                        continue;

                if (ent->client->resp.class_type == CLASS_GUARDIAN) {
                        ent->s.modelindex = 255;
                        ent->fly_sound_debounce_time = level.framenum + 10;
                }
                
                if (self->pain_debounce_time < level.time) {
                        gi.sound (self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
                        self->pain_debounce_time = level.time + 1;
                }


                if (ent->client->glow_time <= level.time + 0.1f)
                        ent->client->glow_time = level.time + 1.2f;
        }

        self->nextthink = level.time + .2f;
}

void detector_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        if (self->target)
                G_UseTargets (self,attacker);

        if (attacker->client) {
                log_killstructure (attacker, self);
                if (attacker->client->resp.team == TEAM_HUMAN)
                        attacker->client->resp.total_score--;
                else
                        attacker->client->resp.total_score++;
        }

        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_ALIEN)
                        team_info.buildpool[TEAM_HUMAN] += COST_DETECTOR*team_info.refund_fraction_h;
                else
                        team_info.points[TEAM_HUMAN] -= COST_DETECTOR;
        }

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_HUMAN) {
                attacker->client->resp.teamkills += 5;
                CheckTKs (attacker);
        }

        BecomeExplosion1(self);
}

void SP_detector(edict_t *self)
{
        static int soundstyle=0;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_HUMAN] -= COST_DETECTOR;

        self->s.modelindex = gi.modelindex("models/objects/detector/tris.md2");
        VectorSet (self->mins, -8, -8, 0);
        VectorSet (self->maxs, 8, 8, 8);
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;

        CheckSolid (self);

        self->classname = "detector";

        self->flags |= FL_CLIPPING;
        self->health = 100;
        self->mass = 100;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        soundstyle++;
        soundstyle %= 4;

        if(soundstyle == 1)
                self->noise_index = SoundIndex (detector_alarm1);
        else if(soundstyle == 2)
                self->noise_index = SoundIndex (detector_alarm2);
        else if(soundstyle == 3)
                self->noise_index = SoundIndex (world_klaxon2);         
        else
                self->noise_index = SoundIndex (detector_alarm3);

        gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);

        self->takedamage = DAMAGE_YES;
        self->die = detector_die;
        self->enttype = ENT_DETECTOR;
        self->owner  = NULL;
        self->think = detector_think;
        self->nextthink = level.time + 3;       

        gi.linkentity (self);
}

