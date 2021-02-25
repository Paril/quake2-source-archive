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
#include "m_healer.h"

void healer_think (edict_t *self);

#define HEAL_NO_HEALTH  1
#define HEAL_NO_ARMOR   2
// 4
// 16
// 32
// 64
void healer_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int damage = 5;

        if (self->target_ent)
                self = self->target_ent;

        //stop breeders killing spawns
        if (self->groundentity && self->groundentity->flags & FL_CLIPPING) {
                damage = 1000;
                T_Damage (self, other, other, vec3_origin, self->s.origin, vec3_origin, damage, 10, DAMAGE_NO_KNOCKBACK, MOD_HEALER);
                return;
        }

        if (other == world && VectorLength(self->velocity) == 0)
        {
                float old = self->s.origin[2];
                ClipBBoxToEights (self);
                self->s.origin[2] = old;
        }

        //r1: if engi decides to drop stuff on it... boom.
        if (other->flags & FL_CLIPPING) {
        	if (other->spawnflags & SPAWNFLAG_BREEDER_PUSH) return; //Dont allow breeders to kill depots

                if (!VectorCompare (self->velocity, vec3_origin))
                        other = self;
                damage = 1000;
                T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, damage, 10, DAMAGE_NO_KNOCKBACK, MOD_HEALER);
                return;
        }

        //prevent further interactions
        if (!other->client)
                return;

        if(other->client->resp.team == TEAM_ALIEN) 
        {
                int c;

                if (level.time < other->wait)             //hack cause wait is used in menus and
                        return;                                                   //makes it seem lagged/broken if client is
                                                                                          //in menu when sitting on healer
#if 0
OMG someone explain me what the hell does this!
                if (level.framenum != self->count) {
                        self->air_finished = self->delay;
                        self->delay = 0;
                }

                self->delay ++;

                other->wait = level.time + 0.05f;

                if (random() * self->air_finished > 1)
                        return;
#else
	if (other->client->svframeflags & csfv_heal_once) return;
	other->client->svframeflags |= csfv_heal_once;
#endif

                c = other->client->resp.class_type;

                if (self->enttype == ENT_EVIL_HEALER)
                {
                        other->health -= 3;
                        if (other->health <= 3)
                                T_Damage (other, self, self, vec3_origin, vec3_origin, vec3_origin, 3, 0, DAMAGE_NO_PROTECTION, MOD_HEALER);
                        else if (other->health <= 0)
                                gi.dprintf ("shouldn't be here\n");

                        if (self->touch_debounce_time<level.time) {
                                if (!(self->spawnflags & 64))
                                        gi.sound (other, CHAN_AUTO, SoundIndex (organ_healer1), 1, ATTN_NORM, 0);
                                self->touch_debounce_time=level.time+2;
                        }

                        return;
                }

                // breeder on healer gets helped
                if (c == CLASS_BREEDER && other->client->build_timer > level.time)
                        other->client->build_timer -= 0.1;
                else if (c == CLASS_GUARDIAN) {
                        other->s.modelindex = 255;
                if (g_guardian_regen_mode > 0)
                        other->fly_sound_debounce_time = level.framenum + 10;

                }

                if (!(self->spawnflags & HEAL_NO_HEALTH))
                        other->health += 4;

                if(other->health > other->max_health)
                        other->health = other->max_health;

                if (!(self->spawnflags & HEAL_NO_ARMOR)) 
                {
                        if (other->fly_sound_debounce_time <= level.framenum && other->client->armor) {
                                if (other->client->resp.class_type == CLASS_STALKER) {
                                        //int amount;
                                        RepairArmor (other, NULL, 2, 0);

                                        other->fly_sound_debounce_time = level.framenum + 1;
#ifdef GUARDIAN_REGEN_AT_HEALER
                                } else if (other->client->resp.class_type == CLASS_GUARDIAN) {
                                        if (!(other->client->resp.upgrades & UPGRADE_CELL_WALL))
                                                RepairArmor (other, NULL, 1, 0);
                                        other->fly_sound_debounce_time = level.framenum + 1;
#endif
                                } else if (other->client->resp.class_type == CLASS_DRONE) {
                                        RepairArmor (other, NULL, 1, 0);
                                        other->fly_sound_debounce_time =level.framenum + 1;
                                } else if (other->client->resp.class_type == CLASS_KAMIKAZE) {
                                        RepairArmor (other, NULL, 1, 0);
                                        other->fly_sound_debounce_time =level.framenum + 3;
                                }
                        }
                }

                //if(other->client->healwait < level.time)
                if (!(self->spawnflags & 4) && classlist[c].healfunc) {
                        int response;
                        response = classlist[c].healfunc (other, self);
                        if (response == 1) {
                                other->client->healwait+=level.time;
                                if (!(self->spawnflags & 32)) {
                                        //gi.cprintf (other,PRINT_MEDIUM,"Inventory refilled! %.1f seconds to another refill.\n",other->client->healwait - level.time);

                                        // do some effects
                                        other->client->ps.stats[STAT_PICKUP_ICON] = imagecache[i_health];
                                        other->client->ps.stats[STAT_PICKUP_STRING] = CS_GENERAL+game.maxclients+1;
                                        other->client->pickup_msg_time = level.time + 3.0f;

                                        other->client->bonus_alpha = 0.1;

                                }

                        } else if (response == 0) {
                                if (other->touch_debounce_time < level.time) {
                                        if (!(self->spawnflags & 32))
                                                gi.cprintf( other, PRINT_MEDIUM, "You must wait %.1f seconds for another refill!\n", other->client->healwait - level.time);
                                        other->touch_debounce_time = level.time + 2;
                                }
                        }
                }

                if (self->touch_debounce_time<level.time) {
                        if (!(self->spawnflags & 64))
                                gi.sound (other, CHAN_AUTO, SoundIndex (organ_healer1), 1, ATTN_NORM, 0);
                        self->touch_debounce_time=level.time+2;
                }
        } else if (other->client->resp.team == TEAM_HUMAN) {
                if (!(self->spawnflags & 16))
                        T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, damage, 10, DAMAGE_NO_KNOCKBACK | DAMAGE_IGNORE_RESISTANCES, MOD_HEALER);
        }

}

void healer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}

mframe_t healer_frames_work [] =
{
        { NULL, 0, healer_think },
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

#ifdef CLIENT_ANIM
mmove_t healer_move_work = {9, 9, healer_frames_work, NULL};
#else
mmove_t healer_move_work = {HEAL_WORK_S, HEAL_WORK_E, healer_frames_work, NULL};
#endif

void healer_work(edict_t *self)
{
        edict_t *workbox;

        self->monsterinfo.currentmove = &healer_move_work;
        self->touch = healer_touch;     // touch = heal_touch

#ifdef CLIENT_ANIM
        self->s.modelindex = gi.modelindex("models/healer/work.md2");
        self->s.effects |= EF_ANIM_ALLFAST;

        gi.linkentity(self);
#endif

        workbox = G_Spawn();
        workbox->touch = healer_touch;
        workbox->target_ent = self;
        self->owner = workbox; //evil haxage(TM)
        workbox->classname = "healerbox";
        workbox->movetype = MOVETYPE_NONE;
        workbox->svflags |= SVF_NOCLIENT;
        workbox->solid = SOLID_TRIGGER;

        VectorCopy (self->s.angles,workbox->s.angles);
        VectorCopy (self->s.origin,workbox->s.origin);

        VectorSet (workbox->mins, -16, -16, 0);
        VectorSet (workbox->maxs, 16, 16, 8);

        //gi.setmodel (workbox, workbox->model);
        gi.linkentity (workbox);
        G_TouchSolids (workbox);

        //gi.dprintf ("healer work.\n");
}


mframe_t healer_frames_grow [] =
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
        { NULL, 0, NULL }
};

mmove_t healer_move_grow = {HEAL_GROW_S, HEAL_GROW_E, healer_frames_grow, healer_work};

void healer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
        IntelligentThrowGib (self, sm_meat_index, 64, GIB_ORGANIC, 2, 6);

        if(bandwidth_mode->value > 1.0)
        {
                ThrowGib (self, sm_meat_index, damage, GIB_ORGANIC);
                ThrowGib (self, sm_meat_index, damage, GIB_ORGANIC);
        }

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_ALIEN) {
                attacker->client->resp.teamkills += 10;
                CheckTKs (attacker);
        }

        if (self->owner) //shouldn't be required, but you never know
                G_FreeEdict (self->owner);

        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 8);
        self->solid = SOLID_NOT;
        self->s.frame = HEAL_DEAD;
        self->movetype = MOVETYPE_TOSS;
        self->takedamage = DAMAGE_NO;
        self->enttype = ENT_NULL;
        self->svflags |= SVF_DEADMONSTER;
        self->nextthink = level.time + 3 + random()*3 + crandom();
        self->think = G_FreeEdict;
        self->deadflag = DEAD_DEAD;

#ifdef CLIENT_ANIM
        self->s.modelindex = gi.modelindex("models/objects/organ/healer/tris.md2");
        self->s.effects &= ~EF_ANIM_ALLFAST;
#endif

        if (!(self->spawnflags & 16384))
        {
                if (attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                        team_info.buildpool[TEAM_ALIEN] += COST_HEALER*team_info.refund_fraction_a;
                else
                        team_info.points[TEAM_ALIEN] -= COST_HEALER;
        }

        if (self->target)
                G_UseTargets (self,attacker);

        if (meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_LAVA)
                G_FreeEdict (self);
        else
                gi.linkentity (self);
}

void healer_grow(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID) {
                healer_die(self,self,self,10000,vec3_origin);
                return;
        }

        if(!self->groundentity)
                return;

        if (self->groundentity->flags & FL_CLIPPING || self->groundentity->enttype == ENT_COCOON) {
                T_Damage (self, world, world, vec3_origin, vec3_origin, plane->normal, 10000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
                return;
        }

        self->monsterinfo.currentmove = &healer_move_grow;
        //self->monsterinfo.stand = &healer_frames_work;
        self->touch = NULL;
        //gi.dprintf ("healer_grow.\n");
        //walkmonster_start(self);
}


mframe_t healer_frames_seed [] =
{
        { NULL, 0, NULL }
};

mmove_t healer_move_seed = {0, 0, healer_frames_seed, NULL};

void healer_seed(edict_t *self)
{
        self->monsterinfo.currentmove = &healer_move_seed;
        self->touch = healer_grow;
        //gi.dprintf ("healer is seeded.\n");
}


void healer_think (edict_t *self)
{
        if (self->health < self->max_health)
                self->health++;

        if (gi.pointcontents(self->s.origin) & !MASK_DEADSOLID)
                healer_die(self,self,self,10000,vec3_origin);

        //self->nextthink = level.time + 30;
}


void SP_monster_healer (edict_t *self)
{
        trace_t tr;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }
        // pre-caches

        if (!self->enttype && !level.framenum)
                team_info.maxpoints[TEAM_ALIEN] -= COST_HEALER;

        self->s.modelindex = gi.modelindex("models/objects/organ/healer/tris.md2");
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 32);
        self->movetype = MOVETYPE_BOUNCE;
        self->solid = SOLID_BBOX;
        self->takedamage = DAMAGE_YES;

        if (!self->owner)
                CheckSolid (self);

        tr = gi.trace (self->s.origin,self->mins,self->maxs,self->s.origin,self,MASK_SHOT);
        if ((tr.ent && tr.ent != world) || tr.allsolid) {
                if (!(self->spawnflags & 16384))
                        team_info.points[1] -= COST_HEALER;

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
                        team_info.points[1] -= COST_HEALER;  // refund
                        G_FreeEdict(self);
                        return;
                }else
                        self->owner = NULL;
        }

        self->health = HEALER_HEALTH;
        self->gib_health = HEALER_GIB_HEALTH;
        self->mass = 250;

        self->think = healer_think;
        self->nextthink = level.time + 1;

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        self->s.frame = 0;
        self->monsterinfo.stand = healer_seed;
        self->monsterinfo.scale = 0.5;
        self->pain = healer_pain;
        self->die = healer_die;
        self->s.renderfx |= RF_IR_VISIBLE;

        if (self->enttype)
                gi.sound (self, CHAN_AUTO, SoundIndex (organ_organe4), 1, ATTN_NORM, 0);

        self->enttype = ENT_HEALER;

        walkmonster_start(self);

        healer_seed(self);

        gi.linkentity (self);
}



