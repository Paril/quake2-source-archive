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
#include "m_spiker.h"

void spiker_rot_setup(edict_t *self);
void spiker_regrow(edict_t *self);
void spiker_work(edict_t *self);

mframe_t spiker_frames_grow [] =
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
        { NULL, 0, NULL }
};

mmove_t spiker_move_grow = {SPIKE_GROW_S, SPIKE_GROW_E, spiker_frames_grow, spiker_work};

void spiker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int damage = random()*10+1;

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
        if (self->groundentity && (self->groundentity->enttype == ENT_COCOON || self->groundentity->enttype == ENT_TELEPORTER)) {
                other = self;
                damage = 1000;
        } else {
                // Don't hurt aliens
                if((other->client && other->client->resp.team == TEAM_ALIEN) || other->svflags & SVF_MONSTER)
                        return;
        }

        if (other->flags & FL_CLIPPING) {
        	if (other->spawnflags & SPAWNFLAG_BREEDER_PUSH) return; //Dont allow breeders to kill depots

                if (!VectorCompare (self->velocity, vec3_origin))
                        other = self;
                damage = 1000;
        }

        if (other->takedamage)
                T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, damage, 3, DAMAGE_TOUCH, MOD_SPIKER_TOUCH);
}

void spiker_grow (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!self->groundentity)
                return;

        if (self->groundentity->flags & FL_CLIPPING || self->groundentity->enttype == ENT_COCOON) {
                T_Damage (self, world, world, vec3_origin, vec3_origin, plane->normal, 10000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
                G_FreeEdict (self);
                return;
        }

        self->monsterinfo.currentmove = &spiker_move_grow;
        self->touch = NULL;
}

void SpikeCheckRegrow(edict_t *self)
{
        if(--self->count <= 0)
                spiker_regrow(self);
}

mframe_t spiker_frames_delay [] =
{
        { NULL, 0, SpikeCheckRegrow }
};

mmove_t spiker_move_delay = {SPIKE_SHOOT_E, SPIKE_SHOOT_E, spiker_frames_delay, NULL};

void spiker_delay(edict_t *self)
{
        self->monsterinfo.currentmove = &spiker_move_delay;
}

mframe_t spiker_frames_regrow [] =
{
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL }
};

void spiker_prepare (edict_t *self);

mmove_t spiker_move_regrow = {SPIKE_REGROW_S, SPIKE_REGROW_E, spiker_frames_regrow, spiker_prepare};

void spiker_regrow(edict_t *self)
{
        self->monsterinfo.currentmove = &spiker_move_regrow;
}

mframe_t spiker_frames_shoot [] =
{
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL },
        { NULL, 0, NULL }
};

mmove_t spiker_move_shoot = {SPIKE_SHOOT_S, SPIKE_SHOOT_E, spiker_frames_shoot, spiker_delay};

void spiker_shoot(edict_t *self)
{
        int n;
        float mult = 0.07;
        float len;
        vec3_t start, aim, temp = {0,0,0};

//      int distanceToTarget;
//      vec3_t halfAcceleration, dirToTarget;

        if(!self->enemy)
                return;

        if (self->enemy->client && self->enemy->client->spikes >  spiker_max_spikes_per_client->value)
                return;

        // Launch Spikes
        VectorCopy(self->s.origin, start);
        start[2] += 37.0;

        VectorCopy(self->enemy->velocity, temp);
        temp[2] = 0;
        //VectorSubtract(self->enemy->s.origin, self->s.origin, delta);
        //len = VectorLength(delta);
        len = Distance (self->enemy->s.origin, self->s.origin);
        VectorScale(temp,(len/1200),temp);

        if (len < 50)
                mult = 0.03;
        else if (len < 100)
                mult = 0.04;
        else if (len < 150)
                mult = 0.05;
        else if (len < 200)
                mult = 0.06;

        VectorSubtract(self->enemy->s.origin, start, aim);
        VectorAdd(aim,temp,aim);
        VectorNormalize(aim);

        if (self->enemy->client)
                self->enemy->client->spikes += 7 - (classlist[self->enemy->client->resp.class_type].frags_needed/1.2f);

        for(n=0; n<(int)spiker_spike_count->value; n++)
        {
                temp[0] = aim[0] + crandom()*mult;
                temp[1] = aim[1] + crandom()*mult;
                temp[2] = aim[2] + crandom()*(mult*0.66f);

                if (spiker_mode->value == 1)
                        fire_acid (self, start, temp, spiker_speed->value);
                else if (spiker_mode->value == 2)
                        fire_web (self, start, temp, spiker_speed->value);
                else if (spiker_mode->value == 3)
                        fire_fire (self, start, temp, 100);
                else if (spiker_mode->value == 4)
                        fire_grenade (self, start, temp, spiker_damage->value, spiker_speed->value, 1.0, 200);
                else if (spiker_mode->value == 5)
                        fire_drunk_missile (self, start, temp, spiker_damage->value, spiker_speed->value, spiker_damage->value, spiker_damage->value);
                else if (spiker_mode->value == 6)
                        fire_piercing (self, start, temp, TE_GUNSHOT, 0, 0, spiker_damage->value, 10, MOD_SPIKE);
                else
                        fire_spike (self, start, temp, spiker_damage->value+random()*spiker_damage_random->value, (int)spiker_speed->value);
        }

        gi.sound (self, CHAN_AUTO, SoundIndex (organ_twang), 1, ATTN_STATIC, 0);

        if (--self->dmg == 0)
        {
                self->count = (int)spiker_regrow_time->value + random() * 5;
                self->monsterinfo.currentmove = &spiker_move_shoot;
        }
}

void CheckRange(edict_t *self)
{
        edict_t *ent = NULL;
        edict_t*newenemy = NULL;
        qboolean haxage = false;

        if (level.framenum & 1)
                return;

        //gi.bprintf (PRINT_HIGH, "check @ %d\n", level.framenum);

        if (self->count <= 0) 
        {
                while ( (ent = findradius_c (ent,self,spiker_distance->value)) != NULL) 
                {

                        if (ent->client->resp.team != TEAM_HUMAN)
                                continue;

                        if (ent->client->resp.class_type == CLASS_COMMANDO && !(ent->s.renderfx & RF_IR_VISIBLE) && ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
                          if (ent == self->enemy && SETTING_INT(spiker_flags) & 4 && visible(self, ent))
                            haxage = true;
                          continue;
                        }

                        if(visible(self, ent))
                        {
                                if (self->enemy == ent) 
                                {
                                        haxage = true;
                                        continue;
                                }
                                haxage = false;
                                newenemy = ent;
                                spiker_shoot(self);
//                              spiker_shoot(self);
//                              spiker_shoot(self);
                                break;
                        }
                }

                if (haxage) {
                        spiker_shoot(self);
//                      spiker_shoot(self);
//                      spiker_shoot(self);
                        newenemy = self->enemy;
                }
                
                self->enemy = newenemy;
        }


        if (self->monsterinfo.healtime < level.time && self->health < self->max_health){
                self->health+=5;
                if (self->health > self->max_health)
                        self->health = self->max_health;
                self->monsterinfo.healtime = level.time +.3f;
        }
}


mframe_t spiker_frames_work [] =
{
        { NULL, 0, CheckRange } // Do player checking.
};

mmove_t spiker_move_work = {SPIKE_GROW_E, SPIKE_GROW_E, spiker_frames_work, NULL};

void spiker_prepare (edict_t *self)
{
        self->monsterinfo.currentmove = &spiker_move_work;
        self->monsterinfo.stand = spiker_work;
        self->dmg = 3;
}

void spiker_work(edict_t *self)
{
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 48);
        
        spiker_prepare (self);

        self->touch = spiker_touch;

        gi.linkentity (self);
}

mframe_t spiker_frames_seed [] =
{
        { NULL, 0, NULL }
};

mmove_t spiker_move_seed = {0, 0, spiker_frames_seed, NULL};

void spiker_seed(edict_t *self)
{
        self->monsterinfo.currentmove = &spiker_move_seed;
        self->touch = spiker_grow;
}

static void spiker_gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        gi.positioned_sound (self->s.origin,self, CHAN_AUTO, SoundIndex (misc_ssdie), 1, ATTN_NORM, 0);
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_MOREBLOOD);
        gi.WritePosition (self->s.origin);
        gi.WriteDir (vec3_origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);
        ThrowGib2 (self->s.origin, sm_meat_index, random() * 12, GIB_ORGANIC);
        G_FreeEdict (self);
}

void spiker_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

        IntelligentThrowGib (self, sm_meat_index, 256, GIB_ORGANIC, 1, 6);
        //ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 64, GIB_ORGANIC);

        if(bandwidth_mode->value > 1.0)
                ThrowGib (self, sm_meat_index, 256, GIB_ORGANIC);

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_ALIEN) {
                attacker->client->resp.teamkills += 6;
                CheckTKs (attacker);
        }

        //VectorSet (self->mins, -16, -16, 0);
        //VectorSet (self->maxs, 16, 16, 8);
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 40);
        self->s.frame = SPIKE_DEAD;
        self->movetype = MOVETYPE_TOSS;
        self->touch = NULL;
        //self->takedamage = DAMAGE_YES;
        self->svflags &= ~SVF_MONSTER;
        self->enttype = ENT_NULL;
        self->health = 150;  //ENT_NULL takes higher damage than ENT_SPIKER, so 150 is a few bullets
        self->die = spiker_gib_die;
        //self->svflags |= SVF_DEADMONSTER;
        self->nextthink = level.time + 3 + random()*4 + crandom();
        self->think = G_FreeEdict;
        self->deadflag = DEAD_DEAD;

        if (attacker != self) {
          int sf = SETTING_INT(spiker_flags);

          if (sf & 1) {
            int i = -1;
            if (self->count <= 0 && self->health > -66) i--;
            for (; i < self->dmg; i++) {
              float angle = crandom()*180;
              vec3_t dir = {sin(angle), cos(angle), 0};
              fire_spike(self, self->s.origin, dir, spiker_damage->value+random()*spiker_damage_random->value, (int)spiker_speed->value);
            }
          }

          if (sf & 2 && meansOfDeath != MOD_CRUSH && meansOfDeath != MOD_LAVA) {
            float tmin, tmax;
            cvar_range(g_spiker_spike_time, &tmin, &tmax);
            spew_spiketrops (self, self, 5, tmin, tmax-tmin);
          }
        }


        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                        team_info.buildpool[TEAM_ALIEN] += COST_SPIKER*team_info.refund_fraction_a;
                else
                        team_info.points[TEAM_ALIEN] -= COST_SPIKER;
        }

        if (self->target)
                G_UseTargets (self,attacker);

        if (meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_LAVA)
                G_FreeEdict (self);
        else
                gi.linkentity (self);
}

void spiker_rot(edict_t *self)
{
        self->die(self, self, self, 0, self->s.origin);
}

mframe_t spiker_frames_shot [] =
{
        { NULL, 0, NULL }
};

mmove_t spiker_move_shot = {SPIKE_SHOOT_E, SPIKE_SHOOT_E, spiker_frames_shot, NULL};

void spiker_rot_setup(edict_t *self)
{
        self->think = spiker_rot;
        self->nextthink = level.time + 5 + random()*5;

        self->monsterinfo.currentmove = &spiker_move_shot;
}

void spiker_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        self->monsterinfo.healtime = level.time + 5;

        if (!self->enemy && other->client)
                self->enemy=other;
}

void spiker_use (edict_t *self, edict_t *other, edict_t *activator)
{
        spiker_prepare (self);
        self->count = 0;
        CheckRange (self); 
}

void SP_monster_spiker (edict_t *self)
{
        trace_t tr;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        // pre-caches
        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_ALIEN] -= COST_SPIKER;

        self->s.modelindex = gi.modelindex("models/objects/organ/spiker/tris.md2");
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 48);
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->takedamage = DAMAGE_YES;

        self->use = spiker_use;

        if (!self->owner)
                CheckSolid (self);

        tr = gi.trace (self->s.origin,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
        if (tr.allsolid && self->owner) { //breeder might place spiker too high, try moving it a bit downwards if needed so starts inside map
          vec3_t step = {self->s.origin[0], self->s.origin[1], self->s.origin[2]-16};
          tr = gi.trace(step,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
          if (!tr.allsolid) VectorCopy(tr.endpos, self->s.origin);
        }
        if ((tr.ent && tr.ent != world) || tr.allsolid) {
                if (!(self->spawnflags & 16384))
                        team_info.points[1] -= COST_SPIKER;

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
                if(self->owner->health <= 0  && (!self->owner->client || self->owner->client->ps.pmove.pm_flags & PMF_TIME_TELEPORT) || gi.pointcontents (self->s.origin) == CONTENTS_SOLID)
                {
                        team_info.points[1] -= COST_SPIKER;  // refund
                        G_FreeEdict(self);
                        return;
                }else
                        self->owner = NULL;
        }
        else
        {
                //r1: randomize angles
                self->s.angles[1] = crandom() * 180;
        }

        self->health = SPIKER_HEALTH;
        self->gib_health = SPIKER_GIB_HEALTH;
        self->mass = 450;

        self->s.frame = 0;
        self->monsterinfo.stand = spiker_seed;
        self->monsterinfo.scale = 0.5;
        //self->accel = (int)(random () * 17) + 1;
        self->pain = spiker_pain;
        self->die = spiker_die;
        self->s.renderfx |= RF_IR_VISIBLE;

        VectorCopy (self->s.origin, self->pos1);

        if (self->enttype)
                gi.sound (self, CHAN_AUTO, SoundIndex (organ_organe3), 1, ATTN_NORM, 0);

        self->enttype = ENT_SPIKER;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        walkmonster_start(self);

        spiker_seed(self);

        gi.linkentity (self);
}
