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

static void resetcount(edict_t*depot)
{
  depot->count = 0;
}

static void depot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void depot_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == world && VectorLength(self->velocity) == 0)
        {
                float old = self->s.origin[2];
                ClipBBoxToEights (self);
                self->s.origin[2] = old;
                
                self->spawnflags &= ~SPAWNFLAG_BREEDER_PUSH;
        }       
        
        if(!other->client)
                return;

        if(other->client->resp.team == TEAM_HUMAN) {
           if (self->spawnflags & 4096) {
             if (other->touch_debounce_time >= level.time) return;
             if (other->touch_debounce_time+5 < level.time) {
               if (!(self->spawnflags & 16)) gi.sound(other, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_NORM, 0);
               if (!(self->spawnflags & 8)) gi.cprintf(other, PRINT_MEDIUM, "You must wait %d.99 seconds for another refill!\n", 4+randomMT()%3);
             }
             other->touch_debounce_time = level.time+0.1f;
             if (++self->count == 5) {
               depot_die(self, self, self, 0, self->s.origin);
               return;
             }
             self->nextthink = level.time+0.5f;
             self->think = resetcount;
             return;
           }
                if (other->touch_debounce_time < level.time) {
                        int response;
                        if (!(self->spawnflags & 4) && classlist[other->client->resp.class_type].healfunc) {
                                response = classlist[other->client->resp.class_type].healfunc(other, NULL);
                                if (response == 1) {
                                        if (!(self->spawnflags & 16)) {
                                                gi.sound (other, CHAN_AUTO, SoundIndex (misc_ar1_pkup), 1, ATTN_NORM, 0);
                                        }
                                        if (!(self->spawnflags & 8)) {

                                                //gi.cprintf (other,PRINT_MEDIUM,"Inventory refilled! %.1f seconds to next refill.\n",other->client->healwait - level.time);

                                                // do some effects
                                                other->client->ps.stats[STAT_PICKUP_ICON] = imagecache[i_health];
                                                other->client->ps.stats[STAT_PICKUP_STRING] = CS_GENERAL+game.maxclients+1;
                                                other->client->pickup_msg_time = level.time + 3.0f;

                                                other->client->bonus_alpha = 0.1;
                                        }
                                } else if (response == 0) {
                                        if (!(self->spawnflags & 16)) {
                                                gi.sound (other, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_NORM, 0);
                                        }
                                        if (!(self->spawnflags & 8)) {
                                                gi.cprintf( other, PRINT_MEDIUM, "You must wait %.1f seconds for another refill!\n", other->client->healwait - level.time);
                                        }
                                }
                        }
                        if (self->spawnflags & 2) {
                                if (other->client->armor) {
                                        RepairArmor (other, NULL, self->count, 0);
                                }
                                other->touch_debounce_time = level.time + self->wait;
                        } else {
                                other->touch_debounce_time = level.time + 2;
                        }
                }
        }
}

static void depot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{                                       
        vec3_t start;

        if (self->deadflag == DEAD_DEAD) return;
        self->deadflag = DEAD_DEAD;

        VectorCopy(self->s.origin, start);      

        temp_point (TE_EXPLOSION1, start);
        gi.multicast (start, MULTICAST_PVS);

        if (self->spawnflags & 4096) {
          T_RadiusDamage(self, self, 550, NULL, 250, MOD_EVIL_DEPOT, 0);
          gi.WriteByte(svc_temp_entity);
          gi.WriteByte(TE_BFG_BIGEXPLOSION);
          gi.WritePosition(self->s.origin);
          gi.multicast(self->s.origin, MULTICAST_PVS);
        }

        if (attacker->client) {
                if (attacker->client->resp.team == TEAM_HUMAN)
                        attacker->client->resp.total_score -= 2;
                else
                        attacker->client->resp.total_score += 2;
        }


        //yase explosive depots. (too unreliable :/)
        /*if (damage)
                T_RadiusDamage (self, attacker, 200, self, 96, MOD_R_SPLASH, 0);*/

        if (self->target)
                G_UseTargets (self,attacker);

        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_ALIEN)
                        team_info.buildpool[TEAM_HUMAN] += COST_DEPOT*team_info.refund_fraction_h;
                else
                        team_info.points[TEAM_HUMAN] -= COST_DEPOT;
        }

        if (attacker && attacker->client) {
                log_killstructure (attacker, self);
                if (attacker->client->resp.team == TEAM_HUMAN) {
                        attacker->client->resp.teamkills += 15;
                        CheckTKs (attacker);
                }
        }

        G_FreeEdict(self);
}

void depot_pain (edict_t *self, edict_t *other, float kick, int damage)
{               

}

void SP_ammo_depot (edict_t *self)
{                                                                                                               
        // pre-caches

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_HUMAN] -= COST_DEPOT;

        self->s.modelindex = gi.modelindex("models/objects/depot/tris.md2");
        VectorSet (self->mins, -16, -16, -24);
        VectorSet (self->maxs, 16, 16, 0);

        CheckSolid (self);

        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->takedamage = DAMAGE_YES;

        self->flags |= FL_CLIPPING;
        self->health = self->max_health = 300;
        self->gib_health = -100;
        self->mass = 250;
        self->pain_debounce_time = level.time;
        self->classname = "ammo_depot";
        self->s.renderfx |= RF_IR_VISIBLE;

        self->s.frame = 0;      
        self->monsterinfo.scale = 0.5;
        self->die = depot_die;
        self->touch = depot_touch;

        self->enttype = ENT_AMMO_DEPOT;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        gi.linkentity (self);
}



