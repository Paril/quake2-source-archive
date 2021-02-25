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

void obstacle_think (edict_t *self);

void obstacle_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int damage = 100;

        if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID) {
                T_Damage (self, world, world, vec3_origin, vec3_origin, plane->normal, 10000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
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
                if((other->client && other->client->resp.team == TEAM_ALIEN) || other->svflags & SVF_MONSTER) {
                  if (self->target_ent && (VectorLength(self->velocity) < 1))
                    self->target_ent = NULL;
                  return;
                }
        }

        //r1: if engi decides to drop stuff on it... boom.
        if (other->flags & FL_CLIPPING) {
        	if (other->spawnflags & SPAWNFLAG_BREEDER_PUSH) return; //Dont allow breeders to kill depots

                if (!VectorCompare (self->velocity, vec3_origin))
                        other = self;
                damage = 1000;
        }

        //stop breeders killing spawns
        if (self->groundentity && self->groundentity->enttype == ENT_TELEPORTER) {
                other = self;
                damage = 1000;
        }

        if (other->takedamage) {
          edict_t*attacker = self;
          if ((other != self) && self->target_ent && self->target_ent->inuse && self->target_ent->client) {
              if (obstacle_frags->value && self->target_ent->client->resp.class_type == CLASS_BREEDER && VectorLength(self->velocity) > 1)
                attacker = self->target_ent;
          }
          T_Damage (other, self, attacker, vec3_origin, other->s.origin, vec3_origin, damage, 0, DAMAGE_TOUCH, MOD_OBSTACLE);
        }

        if (self->target_ent && (VectorLength(self->velocity) < 1))
          self->target_ent = NULL;
}

mframe_t obstacle_frames_stand [] =
{
        { NULL, 0, obstacle_think }
};

mmove_t obstacle_move_ver1 = {OBST_VER1, OBST_VER1, obstacle_frames_stand, NULL};
mmove_t obstacle_move_ver2 = {OBST_VER2, OBST_VER2, obstacle_frames_stand, NULL};
mmove_t obstacle_move_ver3 = {OBST_VER3, OBST_VER3, obstacle_frames_stand, NULL};
mmove_t obstacle_move_ver4 = {OBST_VER4, OBST_VER4, obstacle_frames_stand, NULL};

void obstacle_work(edict_t *self)
{
        if(self->delay == 2)
                self->monsterinfo.currentmove = &obstacle_move_ver2;
        else if(self->delay == 3)
                self->monsterinfo.currentmove = &obstacle_move_ver3;
        else if(self->delay == 4)
                self->monsterinfo.currentmove = &obstacle_move_ver4;
        else
                self->monsterinfo.currentmove = &obstacle_move_ver1;

        self->monsterinfo.stand = obstacle_work;
        self->touch = obstacle_touch;
        //self->solid = SOLID_BBOX;
        self->delay = 0;
        //gi.linkentity (self);
}


mframe_t obstacle_frames_grow [] =
{
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL }
};

mmove_t obstacle_move_grow = {OBST_START_S, OBST_START_E, obstacle_frames_grow, obstacle_work};

void obstacle_grow(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if(!self->groundentity)
                return;

        if (self->groundentity->flags & FL_CLIPPING || self->groundentity->enttype == ENT_COCOON) {
                T_Damage (self, world, world, vec3_origin, vec3_origin, plane->normal, 10000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
                return;
        }

        self->monsterinfo.currentmove = &obstacle_move_grow;
        self->touch = NULL;
}


mframe_t obstacle_frames_seed [] =
{
        { NULL, 0, NULL }
};

mmove_t obstacle_move_seed = {0, 0, obstacle_frames_seed, NULL};

void obstacle_seed(edict_t *self)
{
        self->monsterinfo.currentmove = &obstacle_move_seed;
        self->touch = obstacle_grow;
}

void obstacle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
        IntelligentThrowGib (self, sm_meat_index, 64, GIB_ORGANIC, 1, 4);

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
        self->s.frame = OBST_DEAD;
        self->enttype = ENT_NULL;
        self->movetype = MOVETYPE_TOSS;
        self->svflags |= SVF_DEADMONSTER;
        self->nextthink = level.time + 3 + random() + crandom();
        self->think = G_FreeEdict;
        self->takedamage = DAMAGE_NO;
        self->deadflag = DEAD_DEAD;

        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                        team_info.buildpool[TEAM_ALIEN] += COST_OBSTACLE*team_info.refund_fraction_a;
                else
                        team_info.points[TEAM_ALIEN] -= COST_OBSTACLE;
        }

        if (self->target)
                G_UseTargets (self,attacker);

        if (meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_LAVA)
                G_FreeEdict (self);
        else
                gi.linkentity (self);
}

void obstacle_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        self->monsterinfo.healtime = level.time + 7;
}


void SP_monster_obstacle (edict_t *self)
{
        trace_t tr;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        // pre-caches
        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_ALIEN] -= COST_OBSTACLE;


        self->s.modelindex = gi.modelindex("models/objects/organ/obstacle/tris.md2");
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 32);
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->takedamage = DAMAGE_YES;

        if (!self->owner)
                CheckSolid (self);

        tr = gi.trace (self->s.origin,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
        if ((tr.ent && tr.ent != world) || tr.allsolid) {
                if (!(self->spawnflags & 16384))
                        team_info.points[1] -= COST_OBSTACLE;

                if (self->enttype) {
                        gi.positioned_sound (self->s.origin,self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_NORM, 0);
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_MOREBLOOD);
                        gi.WritePosition (self->s.origin);
                        gi.WriteDir (vec3_origin);
                        gi.multicast (self->s.origin, MULTICAST_PVS);
                        ThrowGib2 (self->s.origin, sm_meat_index, random() * 12, GIB_ORGANIC);
                        ThrowGib2 (self->s.origin, sm_meat_index, random() * 12, GIB_ORGANIC);
                }
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
                        team_info.points[1] -= COST_OBSTACLE;  // refund
                        G_FreeEdict(self);
                        return;
                }else
                        self->owner = NULL;
        }

        self->health = self->max_health = 700;
        self->gib_health = -100;
        self->mass = 900;

        self->s.frame = 0;
        self->delay = (randomMT()%4) + 1;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        self->monsterinfo.stand = obstacle_seed;
        self->monsterinfo.scale = 0.5;
        self->pain = obstacle_pain;
        self->die = obstacle_die;
        self->s.renderfx |= RF_IR_VISIBLE;

        self->classname = "monster_obstacle";

        if (self->enttype) {
                gi.sound (self, CHAN_AUTO, SoundIndex (organ_organe2), 1, ATTN_NORM, 0);
        } else {
                //map spawned, randomize the angles to avoid repeating patterns
                self->s.angles[1] = crandom() * 180;
        }

        self->enttype = ENT_OBSTACLE;

        walkmonster_start(self);

        obstacle_seed(self);

        gi.linkentity (self);
}

void obstacle_think (edict_t *self)
{
        if (self->monsterinfo.healtime < level.time) {
                if (self->health < self->max_health) {
                        self->health += 10;
                        if (self->health > self->max_health)
                                self->health = self->max_health;
                        self->monsterinfo.healtime = level.time + .5f;
                } else {
                        self->monsterinfo.healtime = level.time + .69f;
                        self->health += 1;
                        if (self->health > 2500)
                                self->health = 2500;
                }
        }
}
