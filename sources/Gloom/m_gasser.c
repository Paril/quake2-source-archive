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
#include "m_obstacle.h"

void gasser_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int damage = 4;

        if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID) {
                T_Damage (self, world, world, vec3_origin, vec3_origin, plane->normal, 10000, 0, DAMAGE_NO_PROTECTION, MOD_GASSER);
                return;
        }

        if (other == world && VectorLength(self->velocity) == 0)
        {
                float old = self->s.origin[2];
                ClipBBoxToEights (self);
                self->s.origin[2] = old;
        }

        //can't lay on cocoons
        if (self->groundentity && (self->groundentity->enttype == ENT_COCOON)) {
                other = self;
                damage = 1000;
        } else {
                // Don't hurt aliens
                if((other->client && other->client->resp.team == TEAM_ALIEN) || other->svflags & SVF_MONSTER)
                        return;
        }

        //r1: if engi decides to drop stuff on it... boom.
        if (other->flags & FL_CLIPPING) {
        	if (other->spawnflags & SPAWNFLAG_BREEDER_PUSH) return; //Dont allow breeders to kill depots

                if (!VectorCompare (self->velocity, vec3_origin))
                        other = self;
                damage = 1000;
        }

        if ((self->groundentity && self->groundentity->enttype == ENT_TELEPORTER) || (!self->groundentity && other->enttype == ENT_TELEPORTER)) {
                other = self;
                damage = 1000;
        }

        if (other->takedamage)
                T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, damage, 3, DAMAGE_TOUCH, MOD_OBSTACLE);
}

void smoke_animate(edict_t *ent);
void gasser_fire_gas (edict_t *self)
{
        edict_t *gas;
        trace_t tr;
        vec3_t end;

        gas=G_Spawn();
        VectorCopy (self->s.origin, end);

        end[2] -= 16;
        tr = gi.trace (self->s.origin, 0, 0, end, self, MASK_SOLID);

        gas->classname = "gasball";
        VectorCopy(self->s.origin,gas->s.origin);
        gas->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
        gas->s.frame = 10;
        gas->s.skinnum = 1;
        gas->s.renderfx = RF_TRANSLUCENT;
        gas->enttype = ENT_GASSER_GAS;
        gas->movetype = MOVETYPE_BOUNCE;
        gas->clipmask = MASK_SHOT;
        gas->solid = SOLID_NOT;
        gas->flags |= FL_NOWATERNOISE;
        gas->owner = self;
        gas->nextthink = level.time + 0.1f;
        gas->think = smoke_animate;
        if (tr.fraction != 1) {
                gas->velocity[2] = 250 + crandom()*100;
                gas->velocity[0] = crandom()* 120;
                gas->velocity[1] = crandom()* 120;
        } else {
                gas->velocity[2] = 100;
        }
        gi.linkentity(gas);
}

void gasser_animate_think (edict_t *self);
void gasser_launch_think (edict_t *self)
{
        self->s.frame++;

        if (self->s.frame == 6) {
                gasser_fire_gas (self);
                self->count = 0;
        } else if (self->s.frame == 8) {
                self->think = gasser_animate_think;
        }
        self->nextthink = level.time + .1f;
}

void gasser_animate_think (edict_t *self)
{
        if (self->pain_debounce_time < level.time && self->monsterinfo.healtime < level.time) {
                self->health += 1;
                self->monsterinfo.healtime = level.time + 1;
                if (self->health>self->max_health)
                        self->health=self->max_health;
        }

        self->s.frame++;
        if (self->s.frame > 12) {
                if (self->count == 10) {
                        self->think = gasser_launch_think;
                        self->s.frame = 1;
                } else {
                        self->count++;
                        self->s.frame = 8;
                }
        }
        self->nextthink = level.time + .1f;
}

void gasser_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (!self->deadflag)
                gasser_fire_gas (self);
}

void gasser_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        if (self->deadflag == DEAD_DEAD)
                return;

        if (attacker->client) {
                log_killstructure (attacker, self);
                if (attacker->client->resp.team == TEAM_HUMAN)
                        attacker->client->resp.total_score++;
                else
                        attacker->client->resp.total_score--;
        }

        gi.sound (self, CHAN_AUTO, SoundIndex (misc_ssdie), 1, ATTN_NORM, 0);

        //ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        IntelligentThrowGib (self, sm_meat_index, 128, GIB_ORGANIC, 1, 3);

        if(bandwidth_mode->value > 1.0)
        {
                ThrowGib (self, sm_meat_index, damage, GIB_ORGANIC);
                ThrowGib (self, sm_meat_index, damage, GIB_ORGANIC);
        }

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_ALIEN) {
                attacker->client->resp.teamkills += 5;
                CheckTKs (attacker);
        }

        self->solid = SOLID_NOT;
        self->s.frame = 6;
        self->movetype = MOVETYPE_TOSS;
        self->svflags |= SVF_DEADMONSTER;
        self->nextthink = level.time + 4.7f + crandom() + random() * 3;
        self->think = G_FreeEdict;
        self->takedamage = DAMAGE_NO;
        self->enttype = ENT_NULL;
        self->deadflag = DEAD_DEAD;

        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                        team_info.buildpool[TEAM_ALIEN] += COST_GASSER*team_info.refund_fraction_a;
                else
                        team_info.points[TEAM_ALIEN] -= COST_GASSER;
        }

        if (self->target)
                G_UseTargets (self,attacker);

        if (meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_LAVA)
                G_FreeEdict (self);
        else
                gi.linkentity (self);
}

void gasser_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        self->pain_debounce_time = level.time + 5;
}

void SP_monster_gasser (edict_t *self)
{
        trace_t tr;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }
        // pre-caches

        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_ALIEN] -= COST_GASSER;

        VectorSet (self->mins, -8, -8, -5);
        VectorSet (self->maxs, 8, 8, 0);
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->takedamage = DAMAGE_YES;

        self->use = gasser_use;

        //randomize starting frame
        self->count = random() * 10;
        self->s.frame = 8 + random() * 3;

        if (!self->owner) {
                CheckSolid (self);

                //map spawned, randomize the angles to avoid repeating patterns
                self->s.angles[1] = crandom() * 180;
        }

        tr = gi.trace (self->s.origin,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
        if ((tr.ent && tr.ent != world) || tr.allsolid) {
                if (!(self->spawnflags & 16384))
                        team_info.points[1] -= COST_GASSER;
                gi.positioned_sound (self->s.origin,self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_NORM, 0);
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_MOREBLOOD);
                gi.WritePosition (self->s.origin);
                gi.WriteDir (vec3_origin);
                gi.multicast (self->s.origin, MULTICAST_PVS);
                ThrowGib2 (self->s.origin, sm_meat_index, random() * 12, GIB_ORGANIC);
                ThrowGib2 (self->s.origin, sm_meat_index, random() * 12, GIB_ORGANIC);
                G_UseTargets (self, self);

                // if it couldn't be built
                if (self->owner)
                        self->owner->client->build_timer = level.time;

                G_FreeEdict(self);
                return;
        }

        if(self->owner){
                if(self->owner->health <= 0 || gi.pointcontents (self->s.origin) == CONTENTS_SOLID)
                {
                        team_info.points[1] -= COST_GASSER;  // refund
                        G_FreeEdict(self);
                        return;
                }else
                        self->owner = NULL;
        }

        self->health = self->max_health = 400;

        self->gib_health = -100;
        self->mass = 250;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        self->s.modelindex = gi.modelindex("models/objects/organ/gas/tris.md2");
        //self->s.frame = 8;
        self->s.renderfx |= RF_IR_VISIBLE;

        self->enttype = ENT_GASSER;

        self->touch = gasser_touch;
        self->svflags |= SVF_MONSTER;
        self->pain = gasser_pain;
        self->die = gasser_die;
        self->think = gasser_animate_think;
        self->nextthink = level.time + 0.1f;

        gi.linkentity (self);
}

