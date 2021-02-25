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

// NOTE: Do .H file, and do spawn functions..
// Give breeder 20-30 seconds before laying another egg.
// Breeder lay animation and lay function..
// Monster, becoming cocoon.

#include "g_local.h"
#include "m_cocoon.h"

void cocoon_regrow_large(edict_t *self);
void cocoon_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void cocoon_think (edict_t *self);

mframe_t cocoon_frames_grow2 [] = {
        { NULL, 0, cocoon_think },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, cocoon_think },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, cocoon_think },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, cocoon_think },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL }
};

#ifdef CLIENT_ANIM
mmove_t cocoon_move_wait_large = {14, 14, cocoon_frames_grow2, NULL};
#else
mmove_t cocoon_move_wait_large = {LARGE_S, LARGE_E, cocoon_frames_grow2, NULL};
#endif

void cocoon_wait_large(edict_t *self)
{
#ifdef CLIENT_ANIM
        self->s.modelindex = gi.modelindex("models/cocoon/idle.md2");
        self->s.effects |= EF_ANIM_ALLFAST;
#endif

        VectorSet (self->mins, -16, -16, -12);
        VectorSet (self->maxs, 16, 16, 16);
        self->svflags &= ~SVF_DEADMONSTER;
        self->solid = SOLID_BBOX;
        if (!(self->spawnflags & 32))
                self->movetype = MOVETYPE_STEP;
        self->touch=cocoon_touch;
        self->monsterinfo.currentmove = &cocoon_move_wait_large;

        gi.linkentity (self);
}

void cocoon_die_large (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

        // no multiple kills
        if (self->deadflag)
                return;

        self->svflags &= ~SVF_DEADMONSTER;

        // always allow killing ourselves and some MoDs always kill us
        if (attacker != self && (meansOfDeath != MOD_TRIGGER_HURT && meansOfDeath != MOD_CRUSH && meansOfDeath != MOD_LAVA)) {
                int regrow = damage < 30 || meansOfDeath == MOD_PULSELASER; // if death damage was under 30, regrow

                //for (n= 0; n < 2; n++)
                        //ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 60, GIB_ORGANIC);
                IntelligentThrowGib (self, sm_meat_index, 70, GIB_ORGANIC, 2, 5);

                if (dmg_override) {
                  dmgoverride_t dmg;
                  if (OverrideDamage(&dmg, self, inflictor, attacker, 0, meansOfDeath))
                    regrow = !(dmg.flags & do_kill_egg);
                }
                
                if (regrow)
                {
                        if ((self->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))
                                self->health = self->max_health;
                        else
                                self->health = (int)((float)self->max_health * 0.75);

                        cocoon_regrow_large(self);
                        return;

                }

                // if xmins, don't allow killing of map spawned cocoons
                if ((self->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value) && meansOfDeath != MOD_OUT_OF_MAP && attacker->enttype != ENT_COCOON) {
                        self->health = self->max_health;

                        cocoon_regrow_large(self);
                        return;
                }
        }

        if(attacker && attacker->client && !level.intermissiontime)
        {
                gi.bprintf (PRINT_HIGH, customformat("s", msg_spawnkilled_a->string, "\n"), attacker->client->pers.netname);
                team_info.spawnkills++;

                log_killstructure (attacker, self);

                if(attacker->client->resp.team == TEAM_HUMAN)
                {
                        attacker->client->resp.kills[SPAWNKILLS]++;
                        if (pointstest->value == 0)
                                attacker->client->resp.total_score += 10;
                        if(++attacker->client->resp.score > MAX_SCORE->value) {
                                attacker->client->resp.score = MAX_SCORE->value;
                                team_info.points[TEAM_HUMAN] -= 10;
                        }
                }
                else if (attacker->client->resp.team == TEAM_ALIEN)
                {
                        attacker->client->resp.score--;
                        if (pointstest->value == 0)
                                attacker->client->resp.total_score -= 10;
                        attacker->client->resp.teamkills += 33;
                        CheckTKs (attacker);
                }
        } else {
                SendToTeam (TEAM_ALIEN, customformat(NULL, msg_spawndeath_a->string, "\n"));
                /*if (team_info.spawns[TEAM_ALIEN] <= 3) {
                        SendToAdmins ("Warning, possible basetking on spider team by %s", self->*/
        }

        team_info.spawns[TEAM_ALIEN]--;

        // do some funky efx!
        // both are same sounds?
        //gi.sound (self, CHAN_AUTO, SoundIndex (misc_ssdie), 1, ATTN_NORM, 0);
        gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 0.75, ATTN_IDLE, 0);

        //IntelligentThrowGib (self, sm_meat_index, 256, GIB_ORGANIC, 2, 6);

        // ceiling egg?
        if (!(self->spawnflags & 32)) {
                VectorSet (self->mins, -16, -16, -12);
                VectorSet (self->maxs, 16, 16, 8);
        }

#ifdef CLIENT_ANIM
        self->s.effects &= ~EF_ANIM_ALLFAST;
        self->s.modelindex = gi.modelindex("models/objects/cocoon/tris.md2");
#endif

        self->solid = SOLID_NOT;
        self->s.frame = LEXPL_S;
        self->movetype = MOVETYPE_TOSS;
        self->classname = "cocoon_dead";
        self->svflags |= SVF_DEADMONSTER;
        self->nextthink = level.time + 6 + random() * 4 + crandom();
        self->think = G_FreeEdict;

        self->takedamage = DAMAGE_NO;
        self->teleport_time=self->nextthink*2;

        // this is not an egg anymore
        self->enttype = ENT_NULL;

        self->deadflag = DEAD_DEAD;

        if (!(self->spawnflags & 16384))
        {
                if (attacker && attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                        team_info.buildpool[TEAM_ALIEN] += COST_EGG*team_info.refund_fraction_a;
                else
                        team_info.points[TEAM_ALIEN] -= COST_EGG;
        }

        if (self->target)
                G_UseTargets (self,attacker);

        // free cocoon instantly under certain circumstances
        if (meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_LAVA)
                G_FreeEdict (self);
        else
                gi.linkentity (self);
}

mframe_t cocoon_frames_regrow1 [] =
{
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
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

mmove_t cocoon_move_regrow_large = {LGROW_S, LGROW_E, cocoon_frames_regrow1, cocoon_wait_large};

void cocoon_regrow_large(edict_t *self)
{
        if (!(self->spawnflags & 32)) {
                VectorSet (self->mins, -16, -16, -12);
                VectorSet (self->maxs, 16, 16, 8);
        }

#ifdef CLIENT_ANIM
        self->s.effects &= ~EF_ANIM_ALLFAST;
        self->s.modelindex = gi.modelindex("models/objects/cocoon/tris.md2");
#endif

        //self->solid = SOLID_NOT;

        IntelligentThrowGib (self, sm_meat_index, 64, GIB_ORGANIC, 0, 4);

        gi.sound (self, CHAN_AUTO, SoundIndex(organ_growegg2), 1, ATTN_IDLE, 0);

        gi.linkentity (self);

        self->monsterinfo.currentmove = &cocoon_move_regrow_large;
}

void cocoon_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        if (self->spawnflags & 32)
                self->gravity = -1;

        if (!((self->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))) {
                // no spawning under fire
                self->teleport_time = level.time + 5;

                // delay natural healing
                self->pain_debounce_time = level.time + 10;
        }

        // ?
        if (self->air_finished > level.time)
                self->air_finished = level.time + 120;
}

mframe_t cocoon_frames_seed [] =
{
        { NULL, 0, NULL },
        { NULL, 0, NULL },
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

mmove_t cocoon_move_seed = {SEED_S, SEED_E, cocoon_frames_seed, cocoon_wait_large};

void cocoon_grow_seed(edict_t *self)
{
        // initiate growing animation
        self->monsterinfo.currentmove = &cocoon_move_seed;

        self->monsterinfo.melee = cocoon_regrow_large;
        self->monsterinfo.stand = cocoon_wait_large;
}

void SP_monster_cocoon_use (edict_t *self, edict_t *other, edict_t *activator)
{

        if (self->deadflag == DEAD_DEAD)
                return;

        if (self->enttype == ENT_COCOON_D) {
                // turn disabled cocoon into functioning one

                gi.unlinkentity (self);

                VectorSet (self->mins, -16, -16, -12);
                VectorSet (self->maxs, 16, 16, 16);
                KillBox (self);

                walkmonster_start (self);

                self->enttype = ENT_COCOON;

                self->monsterinfo.stand (self);

                self->s.renderfx |= RF_IR_VISIBLE;
                self->svflags &= ~(SVF_DEADMONSTER | SVF_NOCLIENT);
                self->touch = cocoon_touch;

                AddToSpawnlist (self, TEAM_ALIEN);

                team_info.spawns[TEAM_ALIEN]++;

        } else {
                // turn functioning cocoon into disabled one
                //cocoon_regrow_large (self);

                self->think = NULL;
                self->nextthink = 0;
                self->s.renderfx = 0;
                self->solid = SOLID_BBOX;
                self->touch = NULL;

                self->svflags |= SVF_DEADMONSTER | SVF_NOCLIENT;

                VectorSet (self->mins, -16, -16, -12);
                VectorSet (self->maxs, 16, 16, 8);
                self->s.frame = LEXPL_S;
                if (self->spawnflags & 16)
                        self->takedamage = DAMAGE_YES;
                else
                        self->takedamage = DAMAGE_NO;

                self->enttype = ENT_COCOON_D;

                team_info.spawns[TEAM_ALIEN]--;
        }

        gi.linkentity (self);
}


void egg_check_fall(edict_t*ent)
{
  //If moving upwards check if lost attachment to ceiling (allow some margin for ledges and small bumps)
  if (ent->velocity[2] > 1) {
    vec3_t end = {ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]+8};
    trace_t tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
    if (tr.fraction == 1.0f || tr.surface && (tr.surface->flags & SURF_SKY) || (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD))
      ent->spawnflags &= ~32;
  }

  //Fall down if not ceiling egg anymore
  if (!(ent->spawnflags & 32)) {
    float speed = VectorNormalize(ent->velocity);
    if (speed > 80) speed = 80;
    VectorScale(ent->velocity, speed, ent->velocity);

    gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_webshot2), 1, ATTN_IDLE, 0);
    ent->s.angles[2] = 0;
    ent->gravity = 1;
    ent->postthink = NULL;
  }
}


void egg_check_mapspawn(edict_t*self)
{
  vec3_t top = {self->s.origin[0], self->s.origin[1], self->s.origin[2]+1024};
  trace_t tr = gi.trace(self->s.origin, self->mins, self->maxs, top, self, MASK_MONSTERSOLID);
  if (!tr.allsolid) VectorCopy(tr.endpos, self->s.origin);
  self->postthink = egg_check_fall;
}

void SP_monster_cocoon (edict_t *self)
{
        trace_t tr;
        // pre-caches
//      sound_pain  = SoundIndex (cocoon_drud1);

        self->svflags |= SVF_DEADMONSTER; //never become completely nonsolid

        // handle multiple spawn groups
        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        self->s.modelindex = gi.modelindex("models/objects/cocoon/tris.md2");
        VectorSet (self->mins, -16, -16, -12);
        VectorSet (self->maxs, 16, 16, 16);

        if (!self->owner)
                CheckSolid (self);

        if(self->owner){
                // if owner, check that he's not dead (because of the delayed laying)
                if(self->owner->health < 1 && (!self->owner->client || self->owner->client->ps.pmove.pm_flags & PMF_TIME_TELEPORT)) //only if hasn't already moved and died elsewhere
                {
                        team_info.points[TEAM_ALIEN] -= COST_EGG;  // refund
                        G_FreeEdict(self);
                        return;
                }
        }

        self->solid = SOLID_BBOX;

        self->ideal_yaw = self->s.angles[1];

        if (!self->enttype) {

                // map spawned egg
                self->flags |= FL_MAP_SPAWNED;

                if (!level.framenum)
                {
                        team_info.maxpoints[TEAM_ALIEN] -= COST_EGG;

                        //randomize when players can spawn at the start of the game, prevents
                        //a whole ton of configstrings simultaneously.

                        //FIXME: maybe after they choose team set configstring to most likely class?
                        self->teleport_time = level.time + reconnect_wait->value + random() * 5;
                }

                //map spawned, randomize the angles to avoid repeating patterns
                //Now can do this, yaw is take from ideal_yaw instead of angles
                self->s.angles[1] = crandom() * 180;

                self->s.origin[2]+=16;

                if (ceiling_eggs->value >= 2) {
                        trace_t tr;
                        vec3_t end,start;

                        VectorCopy(self->s.origin,start);
                        VectorCopy(self->s.origin,end);
                        start[2] += 32;
                        end[2] += 1024;
                        tr = gi.trace (start,self->mins,self->maxs,end,self,MASK_SOLID);

                        if (tr.fraction != 1.0) {
                                //gi.dprintf ("moved egg from %f,%f,%f to %f,%f,%f\n",self->s.origin[0],self->s.origin[1],self->s.origin[2],tr.endpos[0],tr.endpos[1],tr.endpos[2]);
                                VectorCopy (tr.endpos,self->s.origin);
                                //VectorCopy (tr.plane.normal,self->s.angles);
                                //self->s.origin[2] -=48;
                        } else {
                                //cocoon_die_large (self,self,self,10000,vec3_origin);
                                G_FreeEdict(self);
                                return;
                        }

                        self->flags |= FL_FLY;
                        self->flags |= FL_NO_KNOCKBACK;
                        self->s.angles[2] = -180;
                        self->gravity = -1;
                        self->velocity[2] = -1;
                        self->movetype = MOVETYPE_STEP;
                        self->spawnflags |= 32;
                } else {
                        if (self->spawnflags & 32) {
                                self->s.origin[2] -= 64;
                                self->s.angles[2] = -180;
                                self->gravity = -2;
                                self->movetype = MOVETYPE_TOSS;
                                self->flags |= FL_FLY;
                                self->flags |= FL_NO_KNOCKBACK;
                        } else {
                                self->movetype = MOVETYPE_STEP;
                        }
                }
        } else {

                // player spawned
                if (self->spawnflags & 32) {
                        self->s.angles[2] = -180;
                        self->gravity = -2;
                        self->movetype = MOVETYPE_TOSS;
                        self->flags |= FL_FLY;
                        self->flags |= FL_NO_KNOCKBACK;
                }
                tr = gi.trace (self->s.origin,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
                if ((tr.ent && tr.ent != world) || tr.allsolid) {

                        if (!(self->spawnflags & 16384))
                                team_info.points[TEAM_ALIEN] -= COST_EGG;

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

                self->air_finished = level.time + 120;
        }

        self->owner = NULL;

        self->enttype = ENT_COCOON;

        self->health = self->max_health = 300;
        self->gib_health = -60;
        self->mass = 500;
        
        if (self->spawnflags & 32) {
          if (self->flags & FL_MAP_SPAWNED) //existing maps assumed egg would fly upwards forever
            self->postthink = egg_check_mapspawn;
          else
            self->postthink = egg_check_fall;
        }

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        self->monsterinfo.melee = cocoon_regrow_large;
        self->monsterinfo.stand = cocoon_grow_seed;

        self->die = cocoon_die_large;
        self->takedamage = DAMAGE_YES;
        self->pain = cocoon_pain;
        self->monsterinfo.scale = MODEL_SCALE;

        self->svflags |= SVF_MONSTER;

        self->use = SP_monster_cocoon_use;

        if (self->spawnflags & 8) {
                // disabled cocoon
                VectorSet (self->mins, -16, -16, -12);
                VectorSet (self->maxs, 16, 16, 8);
                self->s.frame = LEXPL_S;
                if (self->spawnflags & 16)
                        self->takedamage = DAMAGE_YES;
                else
                        self->takedamage = DAMAGE_NO;
                self->touch = NULL;

                // why send them to clients?
                self->svflags |= SVF_DEADMONSTER;

                self->enttype = ENT_COCOON_D;
        } else {
                self->touch = cocoon_touch;

                self->monsterinfo.stand (self);

                self->s.renderfx |= RF_IR_VISIBLE;

                // sets most of monster stuff, look to g_monster.c/monster_start
                walkmonster_start (self);

                AddToSpawnlist (self, TEAM_ALIEN);

                team_info.spawns[TEAM_ALIEN]++;
        }

        gi.linkentity (self);
}

void cocoon_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int damage = 10;
        edict_t *ent = NULL;

        if (self->spawnflags & 32)
                self->gravity = -1;

        if (other == world && VectorLength(self->velocity) == 0)
        {
                float old = self->s.origin[2];
                ClipBBoxToEights (self);
                self->s.origin[2] = old;
        }

        if (other->client && other->client->resp.team == TEAM_ALIEN) {
                // make egg transparent if touched by member of team
                while ((ent = findradius_c(ent,self,32)) != NULL) {
                        self->touch_debounce_time = level.time + 2;
                        self->solid = SOLID_NOT;
                        gi.linkentity (self);
                        break;
                }
                // also no direct dmg to friendly members
                damage = 0;

        }

        // if on client, kill ourself | and allow a new abuse way that humans already used to instant kill eggs
//        if (self->groundentity && self->groundentity->client) {
//                other = self;
//                damage = 1000;
//        }

        if ((self->groundentity && self->groundentity->enttype == ENT_COCOON) || other->enttype == ENT_COCOON) {
                // we're on another egg
                damage = 1000;

                // if xmins, don't allow killing map spawns
                if (self->groundentity && self->groundentity->enttype == ENT_COCOON && (self->groundentity->flags & FL_MAP_SPAWNED) && (level.time < 60*xmins->value)) {
                        other = self;
                } 
                else 
                {
                        // nuke both eggs on stacking (a valid tactic and feature too!)
                        T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, damage, 10, DAMAGE_NO_KNOCKBACK | DAMAGE_IGNORE_RESISTANCES, MOD_EGG);
                        other = self;
                }
        }

        if ((self->groundentity && self->groundentity->enttype == ENT_TELEPORTER) || (!self->groundentity && other->enttype == ENT_TELEPORTER)) {
                // always disallow building on top of teles
                other = self;
                damage = 1000;
        } else if (other->flags & FL_CLIPPING) {
                // kill any stuff built over it (engi stuff uses FL_CLIPPING)
                damage = 1000;
        }

        if (other->takedamage && damage) {
                T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK | DAMAGE_IGNORE_RESISTANCES | DAMAGE_TOUCH, MOD_EGG);
        }

}

void cocoon_think (edict_t *self)
{
        qboolean checkqueue = false;

        if (self->spawnflags & 32)
                self->gravity = -1;

        if (self->health < self->max_health && self->pain_debounce_time < level.time) {
                self->health+=2;
                if (self->health>self->max_health)
                        self->health=self->max_health;
        }
        
        if (self->teleport_time && self->teleport_time <level.time) {
          self->teleport_time = 0;
          checkqueue = true;
        }

        if (self->solid == SOLID_NOT && self->touch_debounce_time < level.time) {
                checkqueue = true;
                self->solid = SOLID_BBOX;
                gi.linkentity (self);
        }
        
        if (checkqueue) SpawnProcessQueue(self);
}
