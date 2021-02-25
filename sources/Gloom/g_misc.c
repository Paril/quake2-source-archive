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

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
        ent->count ^= 1;                // toggle state
        if (mapdebugmode->value >= 2)
                gi.dprintf ("areaportalstate: %s: %i = %i\n", ent->targetname, ent->style, ent->count);
        gi.SetAreaPortalState (ent->style, (qboolean)ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
        if (CheckTargetnameEnt (ent))
                return;
        ent->enttype = ENT_AREAPORTAL;
        ent->use = Use_Areaportal;
        ent->count = 0;         // always start closed;
}

/*
Misc functions
*/
void VelocityForDamage (int damage, /*@out@*/vec3_t v)
{
        v[0] = 100 * crandom();
        v[1] = 100 * crandom();
        v[2] = 200 + 100 * random();

        if (damage < 50) {
                VectorScale (v, 0.7f, v);
        } else {
                VectorScale (v, 1.2f, v);
        }
}

void ClipGibVelocity (edict_t *ent)
{
        if (ent->velocity[0] < -300)
                ent->velocity[0] = -300;
        else if (ent->velocity[0] > 300)
                ent->velocity[0] = 300;
        if (ent->velocity[1] < -300)
                ent->velocity[1] = -300;
        else if (ent->velocity[1] > 300)
                ent->velocity[1] = 300;
        if (ent->velocity[2] < 200)
                ent->velocity[2] = 200; // always some upwards
        else if (ent->velocity[2] > 500)
                ent->velocity[2] = 500;
}


/*
gibs
*/
void gib_think (edict_t *self)
{
        self->nextthink = level.time + FRAMETIME;

        if (++self->s.frame > 9) //sanity
        {
                self->think = G_FreeEdict;
                self->nextthink = level.time + 8 + random()*10;
        }
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t  normal_angles, right;

/* this hits always, so we don't want it
        if (!self->groundentity)
                return;
*/
        // if hit ground,
        if (plane)
        {
                // FIXME: use this with 1.4
                //gi.sound (self, CHAN_AUTO, SoundIndex (organ_sludge), 1.0, ATTN_STATIC, 0);
                gi.sound (self, CHAN_AUTO, SoundIndex (misc_fhit3), 0.4, ATTN_STATIC, 0);

                vectoangles (plane->normal, normal_angles);
                AngleVectors (normal_angles, NULL, right, NULL);
                vectoangles (right, self->s.angles);

                // only play end anim (squish) for sm_meat
                if (self->s.modelindex == sm_meat_index)
                {
                        self->think = gib_think;
                        self->nextthink = level.time + FRAMETIME;
                }
                self->touch=NULL; // damn tilted walls
        }
}

//used for practically everything :)
//so proto'd in g_local
void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        G_FreeEdict (self);
}

//intelligent ThrowGib function. If there are a lot of other ents around me,
//don't throw as many gibs since we would cause extra lag. However if it's
//pretty much clear, lets have some fun.

void IntelligentThrowGib (edict_t *self, int modelindex, int damage, int type, int mingibs, int maxgibs)
{
        int count, num, i;
        static  edict_t *last = NULL;
        static  int     last_frame = 0;
        static  int last_count;

        count = num = 0;

        /*while ((e = findradius_all (e, self->s.origin, 500)) != NULL) {
                if (Q_stricmp (e->classname, "bde") && Q_stricmp (e->classname, "player"))
                        count++;
        }*/

        //check from throwing from the same ent and at the same frame
        //if so the pvs/phs shouldn't have changed
        //FIXED: take into account new gibs spawned between multiple calls
        if (!(last == self && last_frame == level.framenum)) {
                edict_t *e;
                e = NULL;
                for (e = g_edicts + game.maxclients + 1; e <= &g_edicts[globals.num_edicts]; e++) {
                        if (!e->inuse)
                                continue;
                        if (e->svflags & SVF_NOCLIENT)
                                continue;
                        if (e->s.modelindex == 0)
                                continue;
                        if (gi.inPVS (e->s.origin, self->s.origin))
                                count++;
                }

                last = self;
                last_frame = level.framenum;
                last_count = count;
        } else {
                count = last_count;
        }

        if (count < 15)
                num = maxgibs;
        else if (count < 20)
                num = (int)((float)(maxgibs) / 1.5);
        else if (count < 30)
                num = (int)((float)(maxgibs) / 2);
        else if (count < 35)
                num = (int)((float)(maxgibs) / 4);
        else
                return;

        if (num < mingibs)
                num = mingibs;

        if (!num)
                return;

        //r1: if were running high bandwidth mode lets lag the crap out of everything :)
        if (bandwidth_mode->value)
                num *= (int)bandwidth_mode->value + 1;

        //r1: fix for same-frame gibbings not counting new gibs spawned here
        last_count += num;

        for (i = 0; i < num; i++) {
                edict_t *gib;
                vec3_t vd;
                float   vscale;

                gib = G_Spawn();

                VectorCopy(self->s.origin, gib->s.origin);

                gib->s.modelindex = modelindex;
                gib->solid = SOLID_NOT;
                gib->clipmask = MASK_SOLID;
                gib->flags |= FL_NO_KNOCKBACK;
                gib->die = gib_die;
                gib->classname = "gib";

                if (type == GIB_ORGANIC)
                {
                        gib->movetype = MOVETYPE_TOSS;
                        gib->touch = gib_touch;
                        gib->s.effects = EF_GIB;
                        vscale = 0.5f;
                }
                else if(type == GIB_METALLIC)
                {
                        gib->movetype = MOVETYPE_BOUNCE;
                        vscale = 1.0f;
                }
                else    
                {
                        gib->movetype = MOVETYPE_TOSS;
                        gib->s.effects |= EF_GREENGIB;
                        vscale = 0.5f;
                }

                VelocityForDamage (damage, vd);
                VectorMA (self->velocity, vscale, vd, gib->velocity);
                ClipGibVelocity (gib);

                gib->velocity[0] += crandom() * 200;
                gib->velocity[1] += crandom() * 200;
                gib->velocity[2] += crandom() * 200 + 100;

                gib->avelocity[0] = random()*600;
                gib->avelocity[1] = random()*600;
                gib->avelocity[2] = random()*600;

                gib->think = G_FreeEdict;
                gib->nextthink = level.time + 3.5f + random() * 3;

                gi.linkentity (gib);
        }
}

void ThrowGib (edict_t *self, int modelindex, int damage, int type)
{
        edict_t *gib;
        vec3_t  vd;
        vec3_t  origin;
        vec3_t  size;
        float   vscale;

        gib = G_Spawn();

        VectorScale (self->size, 0.5f, size);
        VectorAdd (self->absmin, size, origin);
        gib->s.origin[0] = origin[0] + crandom() * size[0];
        gib->s.origin[1] = origin[1] + crandom() * size[1];
        gib->s.origin[2] = origin[2] + crandom() * size[2];

        //gi.setmodel (gib, gibname);
        gib->s.modelindex = modelindex;
        gib->solid = SOLID_NOT;
        gib->clipmask = MASK_SOLID;
        //gib->flags |= FL_NO_KNOCKBACK;
        gib->takedamage = DAMAGE_YES;
        gib->die = gib_die;
        gib->classname="gib";

        if (type == GIB_ORGANIC)
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->touch = gib_touch;
                gib->s.effects |= EF_GIB;
                vscale = 0.5f;
        }
        else if(type == GIB_METALLIC)
        {
                gib->movetype = MOVETYPE_BOUNCE;
                vscale = 1.0f;
        }
        else    
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->s.effects |= EF_GREENGIB;
                vscale = 0.5f;
        }

        VelocityForDamage (damage, vd);
        VectorMA (self->velocity, vscale, vd, gib->velocity);
        ClipGibVelocity (gib);
        gib->avelocity[0] = random()*600;
        gib->avelocity[1] = random()*600;
        gib->avelocity[2] = random()*600;

        gib->think = G_FreeEdict;

        if(bandwidth_mode->value == 0)
                gib->nextthink = level.time + 6.7f + crandom();
        else
                gib->nextthink = level.time + 3 + random()*10;

        gi.linkentity (gib);
}

void ThrowGib2 (vec3_t origin, int modelindex, int damage, int type)
{
        edict_t *gib;
        
        //float vscale;

        gib = G_Spawn();

        VectorCopy(origin,gib->s.origin);

        gib->s.modelindex = modelindex;
        gib->solid = SOLID_NOT;
        gib->flags |= FL_NO_KNOCKBACK;
        gib->takedamage = DAMAGE_YES;
        gib->die = gib_die;
        gib->classname="gib";

        if (type == GIB_ORGANIC)
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->touch = gib_touch;
                gib->s.effects |= EF_GIB;
                //vscale = 0.5;
        }
        else if(type == GIB_METALLIC)
        {
                gib->movetype = MOVETYPE_BOUNCE;
                //vscale = 1.0;
        }
        else    
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->s.effects |= EF_GREENGIB;
                //vscale = 0.5;
        }

        //VelocityForDamage (damage, vd);
        //ClipGibVelocity (gib);
        gib->avelocity[0] = random()*600;
        gib->avelocity[1] = random()*600;
        gib->avelocity[2] = random()*600;

        gib->velocity[0] = crandom() * 100;
        gib->velocity[1] = crandom() * 100;
        gib->velocity[2] = crandom() * 100;

        gib->think = G_FreeEdict;
        if(bandwidth_mode->value == 0)
                gib->nextthink = level.time + 6.7f + crandom();
        else
                gib->nextthink = level.time + 3 + random()*10;

        gi.linkentity (gib);
}

void gib_explode (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other->s.modelindex == ent->s.modelindex)
                return;

        T_RadiusDamage (ent, world, 300, ent, 200, MOD_HEALER, 0);
        BecomeExplosion1 (ent);
}

void ThrowExplodeGib (edict_t *self, int modelindex, int damage, int type)
{
        edict_t *gib;
        vec3_t  origin;
        vec3_t  size;
        float   vscale;

        gib = G_Spawn();

        VectorScale (self->size, 0.5, size);
        VectorAdd (self->absmin, size, origin);
        gib->s.origin[0] = origin[0] + crandom() * size[0];
        gib->s.origin[1] = origin[1] + crandom() * size[1];
        gib->s.origin[2] = origin[2] + crandom() * size[2];

        //gi.setmodel (gib, gibname);
        gib->s.modelindex = modelindex;
        gib->solid = SOLID_BBOX;
        gib->clipmask = MASK_SOLID;
        //gib->flags |= FL_NO_KNOCKBACK;
        gib->takedamage = DAMAGE_NO;
        gib->die = gib_die;
        gib->classname="egib";
        gib->touch = gib_explode;

        if (type == GIB_ORGANIC)
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->s.effects |= EF_GIB;
                vscale = 1.5f;
        }
        else if(type == GIB_METALLIC)
        {
                gib->movetype = MOVETYPE_BOUNCE;
                vscale = 1.0f;
        }
        else    
        {
                gib->movetype = MOVETYPE_TOSS;
                gib->s.effects |= EF_GREENGIB;
                vscale = 0.5f;
        }

        gib->gravity = 0.25f;

        //VelocityForDamage (damage, vd);
        //ClipGibVelocity (gib);
        //VectorMA (self->velocity, vscale, vd, gib->velocity);

        gib->velocity[0] = 300 * crandom();
        gib->velocity[1] = 300 * crandom();
        gib->velocity[2] = 100 + 300 * random();

        gib->avelocity[0] = random()*600;
        gib->avelocity[1] = random()*600;
        gib->avelocity[2] = random()*600;

        gib->think = G_FreeEdict;

        if(bandwidth_mode->value == 0)
                gib->nextthink = level.time + 6.7f + crandom();
        else
                gib->nextthink = level.time + 3 + random()*10;

        gi.linkentity (gib);
}

/*void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
        vec3_t  vd;
        float   vscale;

        VectorClear (self->mins);
        VectorClear (self->maxs);

        self->s.modelindex2 = 0;
        gi.setmodel (self, gibname);
        self->s.renderfx = 0;
        self->s.skinnum = 0;
        self->s.frame = 0;
        self->classname="headgib";
        self->solid = SOLID_NOT;
        self->s.effects |= EF_GIB;
        self->s.effects &= ~EF_FLIES;
        self->s.sound = 0;
        self->flags |= FL_NO_KNOCKBACK;
        self->svflags &= ~SVF_MONSTER;
        self->takedamage = DAMAGE_YES;
        self->die = gib_die;

        if (type == GIB_ORGANIC)
        {
                self->movetype = MOVETYPE_TOSS;
                self->touch = gib_touch;
                vscale = 0.5;
        }
        else if(type == GIB_METALLIC)
        {
                self->movetype = MOVETYPE_BOUNCE;
                vscale = 1.0;
        }
        else    
        {
                self->movetype = MOVETYPE_TOSS;
                self->touch = gib_touch;
                self->s.effects |= EF_GREENGIB;
                vscale = 0.5;
        }

        VelocityForDamage (damage, vd);
        VectorMA (self->velocity, vscale, vd, self->velocity);
        ClipGibVelocity (self);

        self->avelocity[YAW] = crandom()*600;

        self->think = G_FreeEdict;

         if(bandwidth_mode->value == 0.0)
                self->nextthink = level.time + 2.0 + crandom();
        else
                self->nextthink = level.time + 3 + random()*10;

        gi.linkentity (self);
}*/

/*void clienthead_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        self->svflags |= SVF_NOCLIENT;
        self->think = NULL;
        gi.unlinkentity (self);
        //gi.bprintf (PRINT_HIGH,"head died\n");
}

void clienthead_unlink (edict_t *self)
{
        self->svflags |= SVF_NOCLIENT;
        self->think = NULL;
        gi.unlinkentity (self);
        //gi.bprintf (PRINT_HIGH,"head unlinked\n");
}*/


/*
debris
*/
void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        G_FreeEdict (self);
}

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin)
{
        edict_t *chunk;
        vec3_t  v;

        chunk = G_Spawn();
        VectorCopy (origin, chunk->s.origin);
        gi.setmodel (chunk, modelname);
        v[0] = 100 * crandom();
        v[1] = 100 * crandom();
        v[2] = 100 + 100 * crandom();
        VectorMA (self->velocity, speed, v, chunk->velocity);
        chunk->movetype = MOVETYPE_BOUNCE;
        chunk->solid = SOLID_NOT;
        chunk->avelocity[0] = random()*600;
        chunk->avelocity[1] = random()*600;
        chunk->avelocity[2] = random()*600;
        chunk->think = G_FreeEdict;
        chunk->nextthink = level.time + 5 + random()*5;
        chunk->s.frame = 0;
        chunk->s.skinnum =0;
        chunk->s.renderfx = 0;
        chunk->s.effects = 0;
        chunk->flags = 0;
        chunk->classname = "debris";
        chunk->takedamage = DAMAGE_YES;
        chunk->die = debris_die;
        gi.linkentity (chunk);
}


void BecomeExplosion1 (edict_t *self)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_EXPLOSION1);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        G_FreeEdict (self);
}

void BecomeExplosion2 (edict_t *self)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_EXPLOSION2);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        G_FreeEdict (self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
        this path_corner targeted touches it
*/

void path_corner_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t          v;
        edict_t         *next;

        if (other->movetarget != self)
                return;
        
        if (other->enemy)
                return;

        if (self->pathtarget)
        {
                char *savetarget;

                savetarget = self->target;
                self->target = self->pathtarget;
                G_UseTargets (self, other);
                self->target = savetarget;
        }

        if (self->target)
                next = G_PickTarget(self->target);
        else
                next = NULL;

        if ((next) && (next->spawnflags & 1))
        {
                VectorCopy (next->s.origin, v);
                v[2] += next->mins[2];
                v[2] -= other->mins[2];
                VectorCopy (v, other->s.origin);
                next = G_PickTarget(next->target);
        }

        other->goalentity = other->movetarget = next;

        if (self->wait)
        {
                other->monsterinfo.pausetime = level.time + self->wait;
                other->monsterinfo.stand (other);
                return;
        }

        if (!other->movetarget)
        {
                other->monsterinfo.pausetime = level.time + 100000000;
                other->monsterinfo.stand (other);
        }
        else
        {
                VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
                other->ideal_yaw = vectoyaw (v);
        }
}

void SP_path_corner (edict_t *self)
{
        if (!self->targetname)
        {
                gi.dprintf ("REMOVED: path_corner with no targetname at %s\n", vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->solid = SOLID_TRIGGER;
        self->touch = path_corner_touch;
        VectorSet (self->mins, -8, -8, -8);
        VectorSet (self->maxs, 8, 8, 8);
        self->svflags |= SVF_NOCLIENT;
        gi.linkentity (self);
}

/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null (edict_t *self)
{
        G_FreeEdict (self);
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
        VectorCopy (self->s.origin, self->absmin);
        VectorCopy (self->s.origin, self->absmax);
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF       1


static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (self->spawnflags & START_OFF)
        {
                gi.configstring (CS_LIGHTS+self->style, self->message);
                self->spawnflags &= ~START_OFF;
        }
        else
        {
                gi.configstring (CS_LIGHTS+self->style, self->message);
                self->spawnflags |= START_OFF;
        }
}

void playerconfigstring(edict_t*player, int cs, char*s)
{
  if (!player->inuse || !player->client) return;

  gi.WriteByte(13/*svc_configstring*/);
  gi.WriteShort(cs);
  gi.WriteString(s);
  gi.unicast(player, 0);
}

void configplayerlights(edict_t*player)
{
  if (!player->inuse || !player->client) return;

  gi.WriteByte(13/*svc_configstring*/);
  gi.WriteShort(CS_LIGHTS+31);
  gi.WriteString((player->client->resp.team == TEAM_ALIEN)?"m":"a");
  gi.unicast(player, 0);
}


void SP_light (edict_t *self)
{
        // no targeted lights in deathmatch, because they cause global messages
        if (!self->targetname)
        {
                G_FreeEdict (self);
                return;
        }

        if (!self->message)
                self->message = "a";


        if (self->style >= 32)
        {
                self->use = light_use;
                if (self->spawnflags & START_OFF)
                        gi.configstring (CS_LIGHTS+self->style, self->message);
                else
                        gi.configstring (CS_LIGHTS+self->style, "z");
        }
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN   the wall will not be present until triggered
                                it will then blink in to existance; it will
                                kill anything that was in it's way

TOGGLE                  only valid for TRIGGER_SPAWN walls
                                this allows the wall to be turned on and off

START_ON                only valid for TRIGGER_SPAWN walls
                                the wall will initially be present
*/

void func_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (self->solid == SOLID_NOT)
        {
                self->solid = SOLID_BSP;
                self->svflags &= ~SVF_NOCLIENT;
                KillBox (self);
        }
        else
        {
                self->solid = SOLID_NOT;
                self->svflags |= SVF_NOCLIENT;
        }
        gi.linkentity (self);

        if (!(self->spawnflags & 2))
                self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
        self->movetype = MOVETYPE_PUSH;
        gi.setmodel (self, self->model);

        if (self->spawnflags & 8)
                self->s.effects |= EF_ANIM_ALL;
        if (self->spawnflags & 16)
                self->s.effects |= EF_ANIM_ALLFAST;

        self->enttype = ENT_FUNC_WALL;

        // just a wall
        if ((self->spawnflags & 7) == 0)
        {
                self->solid = SOLID_BSP;
                gi.linkentity (self);
                return;
        }

        // it must be TRIGGER_SPAWN
        if (!(self->spawnflags & 1))
        {
//              gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
                self->spawnflags |= 1;
        }

        // yell if the spawnflags are odd
        if (self->spawnflags & 4)
        {
                if (!(self->spawnflags & 2))
                {
                        gi.dprintf("WARNING: func_wall START_ON without TOGGLE\n");
                        self->spawnflags |= 2;
                }
        }

        self->use = func_wall_use;
        if (self->spawnflags & 4)
        {
                self->solid = SOLID_BSP;
        }
        else
        {
                self->solid = SOLID_NOT;
                self->svflags |= SVF_NOCLIENT;
        }
        gi.linkentity (self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        // only squash thing we fall on top of
        if (!plane)
                return;
        if (plane->normal[2] < 1.0)
                return;
        if (other->takedamage == DAMAGE_NO)
                return;
        T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release (edict_t *self)
{
        self->movetype = MOVETYPE_TOSS;
        self->touch = func_object_touch;
}

void func_object_use (edict_t *self, edict_t *other, edict_t *activator)
{
        self->solid = SOLID_BSP;
        self->svflags &= ~SVF_NOCLIENT;
        self->use = NULL;
        KillBox (self);
        func_object_release (self);
}

void SP_func_object (edict_t *self)
{
        gi.setmodel (self, self->model);

        self->mins[0] += 1;
        self->mins[1] += 1;
        self->mins[2] += 1;
        self->maxs[0] -= 1;
        self->maxs[1] -= 1;
        self->maxs[2] -= 1;

        if (!self->dmg)
                self->dmg = 100;

        if (self->spawnflags == 0)
        {
                self->solid = SOLID_BSP;
                self->movetype = MOVETYPE_PUSH;
                self->think = func_object_release;
                self->nextthink = level.time + 2 * FRAMETIME;
        }
        else
        {
                self->solid = SOLID_NOT;
                self->movetype = MOVETYPE_PUSH;
                self->use = func_object_use;
                self->svflags |= SVF_NOCLIENT;
        }

        if (self->spawnflags & 2)
                self->s.effects |= EF_ANIM_ALL;
        if (self->spawnflags & 4)
                self->s.effects |= EF_ANIM_ALLFAST;

        self->clipmask = MASK_MONSTERSOLID;

        gi.linkentity (self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        vec3_t  origin;
        vec3_t  chunkorigin;
        vec3_t  size;
        int             count;
        int             mass;

        // bmodel origins are (0 0 0), we need to adjust that here
        VectorScale (self->size, 0.5, size);
        VectorAdd (self->absmin, size, origin);
        VectorCopy (origin, self->s.origin);

        self->takedamage = DAMAGE_NO;
        self->enttype = ENT_FUNC_EXPLOSIVE;

        if (self->dmg)
                T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE, 0);

        VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
        VectorNormalize (self->velocity);
        VectorScale (self->velocity, 150, self->velocity);

        // start chunks towards the center
        VectorScale (size, 0.5f, size);

        mass = self->mass;
        if (!mass)
                mass = 50;

        // big chunks
        if (mass >= 100)
        {
                count = mass / 100;
                if (count > 8)
                        count = 8;
                while(count--)
                {
                        chunkorigin[0] = origin[0] + crandom() * size[0];
                        chunkorigin[1] = origin[1] + crandom() * size[1];
                        chunkorigin[2] = origin[2] + crandom() * size[2];
                        ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
                }
        }

        // small chunks
        count = mass / 25;
        if (count > 8)
                count = 8;
        while(count--)
        {
                chunkorigin[0] = origin[0] + crandom() * size[0];
                chunkorigin[1] = origin[1] + crandom() * size[1];
                chunkorigin[2] = origin[2] + crandom() * size[2];
                ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
        }

        G_UseTargets (self, attacker);

        if (self->dmg)
                BecomeExplosion1 (self);
        else
                G_FreeEdict (self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
        if (mapdebugmode->value)
                gi.dprintf ("[explode] func_explosive {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        func_explosive_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
        self->solid = SOLID_BSP;
        self->svflags &= ~SVF_NOCLIENT;
        self->use = NULL;
        KillBox (self);
        gi.linkentity (self);
}

void SP_func_explosive (edict_t *self)
{
        self->movetype = MOVETYPE_PUSH;

        gi.modelindex ("models/objects/debris1/tris.md2");
        gi.modelindex ("models/objects/debris2/tris.md2");

        gi.setmodel (self, self->model);

        if (self->spawnflags & 1)
        {
                self->svflags |= SVF_NOCLIENT;
                self->solid = SOLID_NOT;
                self->use = func_explosive_spawn;
        }
        else
        {
                self->solid = SOLID_BSP;
                if (self->targetname)
                        self->use = func_explosive_use;
        }

        if (self->spawnflags & 2)
                self->s.effects |= EF_ANIM_ALL;
        if (self->spawnflags & 4)
                self->s.effects |= EF_ANIM_ALLFAST;

        if (self->use != func_explosive_use)
        {
                if (!self->health)
                        self->health = 100;
                self->die = func_explosive_explode;
                self->takedamage = DAMAGE_YES;
        }

        if (st.classes)
                self->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        //float ratio;
        //vec3_t        v;

        //if ((!other->groundentity) || (other->groundentity == self))
        //      return;

        //ratio = (float)other->mass / (float)self->mass;
        //VectorSubtract (self->s.origin, other->s.origin, v);
        //M_walkmove (self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode (edict_t *self)
{
        int mass, count;

        T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL, 0);

        mass = self->mass;

        // big chunks
        if (mass >= 100)
        {
                count = mass / 100;
                if (count > 8)
                        count = 8;
                while(count--)
                {
                        ThrowDebris (self, "models/objects/debris1/tris.md2", 1, self->s.origin);
                }
        }

        // small chunks
        count = mass / 25;
        if (count > 16)
                count = 16;
        while(count--)
        {
                ThrowDebris (self, "models/objects/debris2/tris.md2", 2, self->s.origin);
        }

        if (self->groundentity)
                BecomeExplosion2 (self);
        else
                BecomeExplosion1 (self);
}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        self->takedamage = DAMAGE_NO;
        self->nextthink = level.time + 2 * FRAMETIME;
        self->think = barrel_explode;
        self->activator = attacker;
}

void SP_misc_explobox (edict_t *self)
{
        //if (deathmatch->value)
        //{     // auto-remove for deathmatch
        //      G_FreeEdict (self);
        //      return;
        //}

        gi.modelindex ("models/objects/debris1/tris.md2");
        gi.modelindex ("models/objects/debris2/tris.md2");
        gi.modelindex ("models/objects/debris3/tris.md2");

        self->solid = SOLID_BBOX;
        self->movetype = MOVETYPE_STEP;

        self->model = "models/objects/barrels/tris.md2";
        self->s.modelindex = gi.modelindex (self->model);
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 40);

        if (!self->health)
                self->health = 100;

        if (!self->dmg)
                self->dmg = 150;

        self->die = barrel_delay;
        self->takedamage = DAMAGE_YES;

        self->touch = barrel_touch;

        self->think = M_droptofloor;
        self->nextthink = level.time + 2 * FRAMETIME;

        CheckSolid (self);

        gi.linkentity (self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use (edict_t *ent, edict_t *other, edict_t *activator)
{
        /*
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BOSSTPORT);
        gi.WritePosition (ent->s.origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
        */
        G_FreeEdict (ent);
}

void misc_blackhole_think (edict_t *self)
{
        if (++self->s.frame < 19)
                self->nextthink = level.time + FRAMETIME;
        else
        {               
                self->s.frame = 0;
                self->nextthink = level.time + FRAMETIME;
        }
}

void tt_think (edict_t *self) {
        int num;
        edict_t *hit;
        int i;
        edict_t *touch[MAX_EDICTS];

        num = gi.BoxEdicts (self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);

        // be careful, it is possible to have an entity in this
        // list removed before we get to it (killtriggered)
        for (i=0 ; i<num ; i++)
        {
                hit = touch[i];
                if (!hit->inuse)
                        continue;
                if (hit->client) {
                        hit->health = 0;
                        meansOfDeath = MOD_TRIGGER_HURT;
                        player_die (hit, self, self, 100000, vec3_origin);
                        continue;
                }
                if (hit->takedamage) {
                        hit->die(hit, self, self, 0, vec3_origin);
                        break;
                }
        }

        self->nextthink = level.time + 30;
}

void blocker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
//void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void SP_misc_blackhole (edict_t *ent)
{
        //self->solid = SOLID_BSP;
        //self->svflags &= ~SVF_NOCLIENT;

        if (ent->count < 2)
        {
                ent->solid = SOLID_BBOX;
                ent->svflags &= ~SVF_NOCLIENT;
        }
        else
        {
                ent->solid = SOLID_TRIGGER;
                ent->touch = blocker_touch;
                ent->spawnflags = 2;
                ent->style = 0;
        }

        ent->classname = "florb";

        ent->use = NULL;
        ent->movetype = MOVETYPE_NONE;
        //ent->solid = SOLID_BSP;
        //ent->dmg = 5000;
        
        //ent->touch = hurt_touch;
        VectorSet (ent->mins, -64, -64, -64);
        VectorSet (ent->maxs, 64, 64, 64);

        ClipBBoxToEights (ent);

        ent->s.effects |= EF_SPHERETRANS;

        //ent->s.modelindex = 1;
        //ent->s.renderfx = RF_TRANSLUCENT;
        //ent->use = misc_blackhole_use;

        if (ent->count == 1)
        {
                ent->think = tt_think;
                ent->nextthink = level.time + 30;
                tt_think(ent);
        }
        else if (ent->count == 3)
        {
                ent->svflags |= SVF_NOCLIENT;
        }

        gi.linkentity (ent);
}

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/

void misc_eastertank_think (edict_t *self)
{
        if (++self->s.frame < 293)
                self->nextthink = level.time + FRAMETIME;
        else
        {               
                self->s.frame = 254;
                self->nextthink = level.time + FRAMETIME;
        }
}

static void touch_die (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        T_Damage (other,ent,ent,vec3_origin,vec3_origin,vec3_origin,10000,100,DAMAGE_NO_PROTECTION,MOD_TRIGGER_HURT);
}

static void laser_death (edict_t *self)
{
        edict_t *ent;
        vec3_t start;

        ent = NULL;

        VectorCopy (self->s.origin, start);

        while ((ent = findradius(ent, self->s.origin, 500)) != NULL)
        {
                if (!ent->takedamage || ent->health <= 0)
                        continue;
                if (!visible(self, ent))
                        continue;

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_BFG_LASER);
                gi.WritePosition (start);
                gi.WritePosition (ent->s.origin);
                gi.multicast (self->s.origin, MULTICAST_PVS);

                gi.sound (self, CHAN_AUTO, SoundIndex (weapons_laser2), 1.0, ATTN_NORM, 0);

                T_Damage (ent, self, self, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_TARGET_LASER);
        }

        self->nextthink = level.time + 1;
}

static void area_death (edict_t *self)
{
        edict_t *ent;
        vec3_t start;

        ent = NULL;

        VectorCopy (self->s.origin, start);

        while ((ent = findradius(ent, self->s.origin, 1000)) != NULL)
        {
                int die = 0;
                if (!ent->takedamage || ent->health <= 0 || ent->flags & FL_MAP_SPAWNED || ent->enttype >= ENT_CORPSE && ent->enttype <= ENT_NOBLOOD_BODY)
                        continue;

                for (int x = 0; !die && x < self->num_clusters; x++)
                  for (int y = 0; !die && y < ent->num_clusters; y++)
                    if (self->clusternums[x] == ent->clusternums[y])
                      die = 1;

                if (!die) continue;

                die = ent->gib_health*0.25f;
                if (die < 1) die = 25;
                T_Damage (ent, self, self, vec3_origin, ent->s.origin, vec3_origin, ent->health + randomMT()%die, 0, DAMAGE_NO_PROTECTION, MOD_EXIT);
        }

        self->nextthink = level.time + 1;
        
        if (self->spawnflags & 8) {
          self->spawnflags ^= 1;
          if (self->spawnflags & 1) {
            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_BFG_BIGEXPLOSION);
            gi.WritePosition(self->s.origin);
            gi.multicast(self->s.origin, MULTICAST_PVS);
          }
        }
}


void SP_misc_deathball (edict_t *ent)
{
        if (ent->spawnflags & 4) {
          ent->classname = "deatharea";
          ent->think = area_death;
          ent->nextthink = level.time + 1;
        } else {
          ent->classname = "deathballffs";
          ent->solid = SOLID_BBOX;
          VectorSet (ent->mins, -64, -64, -64);
          VectorSet (ent->maxs, 64, 64, 64);
          ent->s.modelindex = gi.modelindex ("models/objects/dengine/tris.md2");
          ent->touch = touch_die;

          if (ent->spawnflags & 1) {
            ent->think = laser_death;
            ent->nextthink = level.time + 1;
          }

        }

        gi.linkentity (ent);
}

void SP_misc_eastertank (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -32, -32, -16);
        VectorSet (ent->maxs, 32, 32, 32);
        ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
        ent->s.frame = 254;
        ent->think = misc_eastertank_think;
        ent->nextthink = level.time + 2 * FRAMETIME;
        gi.linkentity (ent);
}

/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick_think (edict_t *self)
{
        if (++self->s.frame < 247)
                self->nextthink = level.time + FRAMETIME;
        else
        {               
                self->s.frame = 208;
                self->nextthink = level.time + FRAMETIME;
        }
}

void SP_misc_easterchick (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -32, -32, 0);
        VectorSet (ent->maxs, 32, 32, 32);
        ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
        ent->s.frame = 208;
        ent->think = misc_easterchick_think;
        ent->nextthink = level.time + 2 * FRAMETIME;
        gi.linkentity (ent);
}

/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick2_think (edict_t *self)
{
        if (++self->s.frame < 287)
                self->nextthink = level.time + FRAMETIME;
        else
        {               
                self->s.frame = 248;
                self->nextthink = level.time + FRAMETIME;
        }
}

void SP_misc_easterchick2 (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -32, -32, 0);
        VectorSet (ent->maxs, 32, 32, 32);
        ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
        ent->s.frame = 248;
        ent->think = misc_easterchick2_think;
        ent->nextthink = level.time + 2 * FRAMETIME;
        gi.linkentity (ent);
}

/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think (edict_t *ent)
{
        ent->s.frame = (ent->s.frame + 1) % 16;
        ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_NOT;
        ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
        ent->s.frame = randomMT() % 16;
        gi.linkentity (ent);

        ent->think = misc_banner_think;
        ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        int             n;

        if (self->health > -80)
                return;

        gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_NORM, 0);
        for (n= 0; n < 4; n++)
                ThrowGib (self, sm_meat_index, damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier (edict_t *ent)
{
        if (deathmatch->value && (bandwidth_mode->value<2))
        {       // auto-remove for deathmatch
                G_FreeEdict (ent);
                return;
        }

        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        ent->s.modelindex=gi.modelindex ("models/deadbods/dude/tris.md2");
        ent->classname = "corpse";

        // Defaults to frame 0
        if (ent->spawnflags & 2)
                ent->s.frame = 1;
        else if (ent->spawnflags & 4)
                ent->s.frame = 2;
        else if (ent->spawnflags & 8)
                ent->s.frame = 3;
        else if (ent->spawnflags & 16)
                ent->s.frame = 4;
        else if (ent->spawnflags & 32)
                ent->s.frame = 5;
        else
                ent->s.frame = 0;

        VectorSet (ent->mins, -16, -16, 0);
        VectorSet (ent->maxs, 16, 16, 16);
        ent->deadflag = DEAD_DEAD;
        ent->takedamage = DAMAGE_YES;
        ent->svflags |= SVF_MONSTER|SVF_DEADMONSTER;
        ent->die = misc_deadsoldier_die;
        gi.linkentity (ent);
}

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"         How fast the Viper should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_viper_use  (edict_t *self, edict_t *other, edict_t *activator)
{
        self->svflags &= ~SVF_NOCLIENT;
        self->use = train_use;
        train_use (self, other, activator);
}

void SP_misc_viper (edict_t *ent)
{
        if (!ent->target)
        {
                gi.dprintf ("REMOVED: misc_viper without a target at %s\n", vtos(ent->absmin));
                G_FreeEdict (ent);
                return;
        }

        if (!ent->speed)
                ent->speed = 300;

        ent->movetype = MOVETYPE_PUSH;
        ent->solid = SOLID_NOT;
        ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
        VectorSet (ent->mins, -16, -16, 0);
        VectorSet (ent->maxs, 16, 16, 32);

        ent->think = func_train_find;
        ent->nextthink = level.time + FRAMETIME;
        ent->use = misc_viper_use;
        ent->svflags |= SVF_NOCLIENT;
        ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

        gi.linkentity (ent);
}

/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72) 
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -176, -120, -24);
        VectorSet (ent->maxs, 176, 120, 72);
        ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
        gi.linkentity (ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"   how much boom should the bomb make?
*/
void misc_viper_bomb_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        G_UseTargets (self, self->activator);

        self->s.origin[2] = self->absmin[2] + 1;
        T_RadiusDamage (self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB, 0);
        BecomeExplosion2 (self);
}

void misc_viper_bomb_prethink (edict_t *self)
{
        vec3_t  v;
        float   diff;

        self->groundentity = NULL;

        diff = self->timestamp - level.time;
        if (diff < -1.0)
                diff = -1.0;

        VectorScale (self->moveinfo.dir, 1.0 + diff, v);
        v[2] = diff;

        diff = self->s.angles[2];
        vectoangles (v, self->s.angles);
        self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *viper;

        self->solid = SOLID_BBOX;
        self->svflags &= ~SVF_NOCLIENT;
        self->s.effects |= EF_ROCKET;
        self->use = NULL;
        self->movetype = MOVETYPE_TOSS;
        self->prethink = misc_viper_bomb_prethink;
        self->touch = misc_viper_bomb_touch;
        self->activator = activator;

        viper = G_Find (NULL, FOFS(classname), "misc_viper");
        VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

        self->timestamp = level.time;
        VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb (edict_t *self)
{
        self->movetype = MOVETYPE_NONE;
        self->solid = SOLID_NOT;
        VectorSet (self->mins, -8, -8, -8);
        VectorSet (self->maxs, 8, 8, 8);

        self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");

        if (!self->dmg)
                self->dmg = 1000;

        self->use = misc_viper_bomb_use;
        self->svflags |= SVF_NOCLIENT;

        gi.linkentity (self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"         How fast it should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_strogg_ship_use  (edict_t *self, edict_t *other, edict_t *activator)
{
        self->svflags &= ~SVF_NOCLIENT;
        self->use = train_use;
        train_use (self, other, activator);
}

void SP_misc_strogg_ship (edict_t *ent)
{
        if (!ent->target)
        {
                gi.dprintf ("REMOVED: %s without a target at %s\n", ent->classname, vtos(ent->absmin));
                G_FreeEdict (ent);
                return;
        }

        if (!ent->speed)
                ent->speed = 300;

        ent->movetype = MOVETYPE_PUSH;
        ent->solid = SOLID_NOT;
        ent->s.modelindex = gi.modelindex ("models/ships/strogg1/tris.md2");
        VectorSet (ent->mins, -16, -16, 0);
        VectorSet (ent->maxs, 16, 16, 32);

        ent->think = func_train_find;
        ent->nextthink = level.time + FRAMETIME;
        ent->use = misc_strogg_ship_use;
        ent->svflags |= SVF_NOCLIENT;
        ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

        gi.linkentity (ent);
}


/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think (edict_t *self)
{
        self->s.frame++;
        if (self->s.frame < 38)
                self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use (edict_t *self, edict_t *other, edict_t *activator)
{
        self->s.frame = 0;
        self->think = misc_satellite_dish_think;
        self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -64, -64, 0);
        VectorSet (ent->maxs, 64, 64, 128);
        ent->s.modelindex = gi.modelindex ("models/objects/satellite/tris.md2");
        ent->use = misc_satellite_dish_use;
        gi.linkentity (ent);
}

/*QUAKED misc_dengine (0 1 0) (-2 -2 -12) (2 2 12)
*/
                        

void SP_misc_dengine (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
        ent->solid = SOLID_BBOX;
        VectorSet (ent->mins, -64, -64, 0);
        VectorSet (ent->maxs, 64, 64, 128);
        ent->s.modelindex = gi.modelindex ("models/objects/dengine/tris.md2");  
        gi.linkentity (ent);
}



/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1 (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
//      ent->solid = SOLID_BBOX;
        ent->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
        gi.linkentity (ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t *ent)
{
        ent->movetype = MOVETYPE_NONE;
//      ent->solid = SOLID_BBOX;
        ent->s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
        gi.linkentity (ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm (edict_t *ent)
{
        gi.setmodel (ent, "models/objects/gibs/arm/tris.md2");
        ent->solid = SOLID_NOT;
        ent->s.effects |= EF_GIB;
        ent->takedamage = DAMAGE_YES;
        ent->die = gib_die;
        ent->movetype = MOVETYPE_TOSS;
        ent->svflags |= SVF_MONSTER;
        ent->deadflag = DEAD_DEAD;
        ent->avelocity[0] = random()*200;
        ent->avelocity[1] = random()*200;
        ent->avelocity[2] = random()*200;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 30;
        gi.linkentity (ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg (edict_t *ent)
{
        gi.setmodel (ent, "models/objects/gibs/leg/tris.md2");
        ent->solid = SOLID_NOT;
        ent->s.effects |= EF_GIB;
        ent->takedamage = DAMAGE_YES;
        ent->die = gib_die;
        ent->movetype = MOVETYPE_TOSS;
        ent->svflags |= SVF_MONSTER;
        ent->deadflag = DEAD_DEAD;
        ent->avelocity[0] = random()*200;
        ent->avelocity[1] = random()*200;
        ent->avelocity[2] = random()*200;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 30;
        gi.linkentity (ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head (edict_t *ent)
{
        gi.setmodel (ent, "models/objects/gibs/head/tris.md2");
        ent->solid = SOLID_NOT;
        ent->s.effects |= EF_GIB;
        ent->takedamage = DAMAGE_YES;
        ent->die = gib_die;
        ent->movetype = MOVETYPE_TOSS;
        ent->svflags |= SVF_MONSTER;
        ent->deadflag = DEAD_DEAD;
        ent->avelocity[0] = random()*200;
        ent->avelocity[1] = random()*200;
        ent->avelocity[2] = random()*200;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 30;
        gi.linkentity (ent);
}

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character (edict_t *self)
{
        self->movetype = MOVETYPE_PUSH;
        gi.setmodel (self, self->model);
        self->solid = SOLID_BSP;
        self->s.frame = 12;
        gi.linkentity (self);
        return;
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *e;
        size_t  l;
        int             n;
        char    c;

        l = strlen(self->message);
        for (e = self->teammaster; e; e = e->teamchain)
        {
                if (!e->count)
                        continue;
                n = e->count - 1;
                if (n > l)
                {
                        e->s.frame = 12;
                        continue;
                }

                c = self->message[n];
                if (c >= '0' && c <= '9')
                        e->s.frame = c - '0';
                else if (c == '-')
                        e->s.frame = 10;
                else if (c == ':')
                        e->s.frame = 11;
                else
                        e->s.frame = 12;
        }
}

void SP_target_string (edict_t *self)
{
        if (!self->message)
                self->message = "";
        self->use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"         0 "xx"
                        1 "xx:xx"
                        2 "xx:xx:xx"
*/

#define CLOCK_MESSAGE_SIZE      16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_clock_reset (edict_t *self)
{
        self->activator = NULL;
        if (self->spawnflags & 1)
        {
                self->health = 0;
                self->wait = self->count;
        }
        else if (self->spawnflags & 2)
        {
                self->health = self->count;
                self->wait = 0;
        }
}

static void func_clock_format_countdown (edict_t *self)
{
        if (self->style == 0)
        {
                Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
                return;
        }

        if (self->style == 1)
        {
                Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
                if (self->message[3] == ' ')
                        self->message[3] = '0';
                return;
        }

        if (self->style == 2)
        {
                Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
                if (self->message[3] == ' ')
                        self->message[3] = '0';
                if (self->message[6] == ' ')
                        self->message[6] = '0';
                return;
        }
}

void func_clock_think (edict_t *self)
{
        if (!self->enemy)
        {
                self->enemy = G_Find (NULL, FOFS(targetname), self->target);
                if (!self->enemy)
                        return;
        }

        if (self->spawnflags & 1)
        {
                func_clock_format_countdown (self);
                self->health++;
        }
        else if (self->spawnflags & 2)
        {
                func_clock_format_countdown (self);
                self->health--;
        }
        else
        {
                struct tm       *ltime;
                time_t          gmtime;

                time(&gmtime);
                ltime = localtime(&gmtime);
                Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
                if (self->message[3] == ' ')
                        self->message[3] = '0';
                if (self->message[6] == ' ')
                        self->message[6] = '0';
        }

        self->enemy->message = self->message;
        self->enemy->use (self->enemy, self, self);

        if (((self->spawnflags & 1) && (self->health > self->wait)) ||
                ((self->spawnflags & 2) && (self->health < self->wait)))
        {
                if (self->pathtarget)
                {
                        char *savetarget;
                        char *savemessage;

                        savetarget = self->target;
                        savemessage = self->message;
                        self->target = self->pathtarget;
                        self->message = NULL;
                        G_UseTargets (self, self->activator);
                        self->target = savetarget;
                        self->message = savemessage;
                }

                if (!(self->spawnflags & 8))
                        return;

                func_clock_reset (self);

                if (self->spawnflags & 4)
                        return;
        }

        self->nextthink = level.time + 1;
}

void func_clock_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (!(self->spawnflags & 8))
                self->use = NULL;
        if (self->activator)
                return;
        self->activator = activator;
        self->think (self);
}

void SP_func_clock (edict_t *self)
{
        if (!self->target)
        {
                gi.dprintf("REMOVED: %s with no target at %s\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if ((self->spawnflags & 2) && (!self->count))
        {
                gi.dprintf("REMOVED: %s with no count at %s\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if ((self->spawnflags & 1) && (!self->count))
                self->count = 60*60;;

        func_clock_reset (self);

        self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);

        self->think = func_clock_think;

        if (self->spawnflags & 4)
                self->use = func_clock_use;
        else
                self->nextthink = level.time + 1;
}

void func_and_fail (edict_t * self)
{
        char *oldtarget = self->target;

        if (mapdebugmode->value)
                gi.dprintf ("[funcand] func_and {%s} triggered pathtarget because it wasn't targetted enough.\n",self->targetname);

        self->target = self->pathtarget;
        G_UseTargets (self, self->activator);
        self->target = oldtarget;
        self->nextthink = 0;
        self->think = NULL;
}

void func_and_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (self->pathtarget && self->count == 0) {
                self->nextthink = level.time + self->dmg / 10.0f;
                self->think = func_and_fail;
        }

        self->activator = activator;

        if (mapdebugmode->value)
                gi.dprintf ("[funcand] func_and {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        if (++self->count == self->health) {
                gi.dprintf ("triggered %d times ", self->count);
                if (self->fly_sound_debounce_time > level.framenum) {
                        G_UseTargets (self, self->activator);
                        if (self->spawnflags & 2) {
                                G_FreeEdict (self);
                        } else {
                                self->timestamp = 0;
                                self->count = 0;
                                return;
                        }
                } else {
                        if (!(self->spawnflags & 4)) {
                                self->fly_sound_debounce_time = 0;
                                self->count = 0;
                                return;
                        } else {
                                self->count = 1;
                        }
                }
        }

        self->fly_sound_debounce_time = level.framenum + self->dmg;
}

void SP_func_and (edict_t *self)
{
        if (CheckTargetEnt (self))
                return;

        if (self->accel)
                self->dmg = (int)self->accel * 10;

        if (!self->dmg)
                self->dmg = 1;

        if (!self->health)
                self->health = 2;

        self->use = func_and_use;
}

void countdown_think (edict_t *self)
{
        self->nextthink = level.time + 1;
        self->count--;
        if (self->count == 0) { 
                G_UseTargets (self, self->activator);

                if (self->spawnflags & 4) {
                        self->count = self->max_health;   
                } else {
                        G_FreeEdict (self);
                        return;
                }

                if (self->spawnflags & 8) {
                        self->think = NULL;
                        self->nextthink = 0;
                }
                
        }
}

void func_countdown_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (mapdebugmode->value)
                gi.dprintf ("[countdn] func_countdown {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        self->activator = activator;
        if (self->think == NULL) {
                self->nextthink = level.time + 1;
                self->think = countdown_think;
                self->think (self);
        } else {
                self->think = NULL;
                self->nextthink = 0;
        }
}

void SP_func_countdown (edict_t *self)
{
        if (CheckTargetEnt (self))
                return;

        if (!self->count)
        {
                gi.dprintf("REMOVED: %s with no count at %s\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->use = func_countdown_use;
        self->max_health = self->count;

        if (self->spawnflags & 2) {
                self->think = NULL;
        } else {
                self->nextthink = level.time + 1;
                self->think = countdown_think;
        }
}

//void SP_trigger_relay 

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        edict_t         *dest;
        int                     i;

        if (!other->client)
                return;

        if (self->style)
                if (other->client && !((self->style) & classtypebit[other->client->resp.class_type]))
                        return;

        dest = G_Find (NULL, FOFS(targetname), self->target);
        if (!dest)
        {
                gi.dprintf ("WARNING: teleporter_touch: Couldn't find destination %s\n", self->target);
                return;
        }

        if (other->client->ctf_grapple)
                CTFResetGrapple(other->client->ctf_grapple);

        // unlink to make sure it can't possibly interfere with KillBox
        gi.unlinkentity (other);

        VectorCopy (dest->s.origin, other->s.origin);
        VectorCopy (dest->s.origin, other->s.old_origin);
        other->s.origin[2] += 10;

        // clear the velocity and hold them in place briefly
        VectorClear (other->velocity);
        other->client->ps.pmove.pm_time = 160>>3;               // hold time
        other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

        // draw the teleport splash at source and on the player
        self->owner->s.event = EV_PLAYER_TELEPORT;
        other->s.event = EV_PLAYER_TELEPORT;

        //r1: remove invuln to prevent spawning to enemy w/invuln
        other->client->invincible_framenum = level.framenum;

        // set angles
        for (i=0 ; i<3 ; i++)
                other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

        VectorClear (other->s.angles);
        VectorClear (other->client->ps.viewangles);
        VectorClear (other->client->v_angle);

        // kill anything at the destination
        KillBox (other);

        gi.linkentity (other);
}

void teleporter_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (self->s.skinnum) {
                self->s.skinnum = 0;
                self->s.effects = 0;
                self->s.sound = 0;
        } else {
                self->s.skinnum = 1;
                self->s.effects = EF_TELEPORTER;
                self->s.sound = SoundIndex (world_amb10);
        }
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/

static void copy_owner_origin (edict_t *ent)
{
        VectorCopy (ent->owner->s.origin, ent->s.origin);
        ent->nextthink = 0;
        ent->think = NULL;
        gi.linkentity (ent);
}

void SP_misc_teleporter (edict_t *ent)
{
        edict_t         *trig;

        if (!ent->target)
        {
                gi.dprintf ("REMOVED: misc_teleporter at %s without a target.\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        gi.setmodel (ent, "models/objects/dmspot/tris.md2");
        if (!(ent->spawnflags & 1)) {
                ent->s.skinnum = 1;
                ent->s.effects = EF_TELEPORTER;
                ent->s.sound = SoundIndex (world_amb10);
        } else {
                if (!ent->targetname) {
                        gi.dprintf ("REMOVED: %s at %s with no targetname but START_OFF\n", ent->classname, vtos2(ent));
                        G_FreeEdict (ent);
                        return;
                }
                ent->s.skinnum = 0;
        }
        ent->use = teleporter_use;
        ent->solid = SOLID_BBOX;

        VectorSet (ent->mins, -24, -24, -24);
        VectorSet (ent->maxs, 24, 24, -16);

        CheckSolid (ent);

        gi.linkentity (ent);

        trig = G_Spawn ();
        trig->touch = teleporter_touch;
        trig->solid = SOLID_TRIGGER;
        trig->target = ent->target;
        trig->owner = ent;
        trig->think = copy_owner_origin;
        trig->nextthink = level.time + 2;
        VectorCopy (ent->s.origin, trig->s.origin);
        VectorSet (trig->mins, -8, -8, 8);
        VectorSet (trig->maxs, 8, 8, 24);

        gi.linkentity (trig);

        ent->target_ent = trig;
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t *ent)
{
        if (!ent->targetname)
        {
                gi.dprintf ("REMOVED: misc_teleporter_dest at %s without a targetname.\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        if (ent->model) {
                gi.setmodel (ent, ent->model);
        } else {
                gi.setmodel (ent, "models/objects/dmspot/tris.md2");
        }

        ent->s.skinnum = 0;
        
        if (ent->spawnflags & 1) {
                ent->svflags |= SVF_NOCLIENT;
                ent->solid = SOLID_NOT;
        } else {
                ent->solid = SOLID_BBOX;
                VectorSet (ent->mins, -24, -24, -24);
                VectorSet (ent->maxs, 24, 24, -16);
                CheckSolid (ent);
        }

        gi.linkentity (ent);
}

void infestation_use (edict_t *self, edict_t *other, edict_t *activator)
{
        G_FreeEdict (self);
}

void infest_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void InfestGasThink(edict_t *self);
void SP_misc_infestation (edict_t *ent)
{
        if (ent->spawnflags & 4) {
                ent->s.modelindex = gi.modelindex("models/objects/infested/tris.md2");
                VectorSet (ent->mins, -16, -16, -24);
                VectorSet (ent->maxs, 16, 16, -16);
        } else {
                ent->s.modelindex=sm_meat_index;
                VectorSet (ent->mins, -8, -8, 0);
                VectorSet (ent->maxs, 8, 8, 8);
        }

        CheckSolid (ent);

        ent->use = infestation_use;
        ent->solid=SOLID_BBOX;
        ent->movetype=MOVETYPE_TOSS;
        ent->classname="infest";
        //ent->flags |= FL_NO_KNOCKBACK;
        ent->owner = ent;
        ent->enttype = ENT_INFEST;

        ent->svflags = SVF_DEADMONSTER;

        if (ent->spawnflags & 1){
                ent->think = InfestGasThink;
                ent->s.effects |= EF_FLIES;
                ent->nextthink = level.time + 1.0f;
        }

        if (ent->spawnflags & 2){
                ent->damage_absorb = 6; // was 5
                ent->takedamage=DAMAGE_YES;
                if (!ent->health)
                        ent->health = INFEST_HEALTH; // was 1000
                ent->die=infest_die;
        }

        gi.linkentity (ent);
}

void touch_jumppad (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!other->client)
                return;

        other->groundentity=NULL;
        other->velocity[2]/=2;
        other->velocity[2]+= 500;
}

void SP_misc_jumppad (edict_t *ent)
{
        gi.setmodel (ent, "models/objects/dmspot/tris.md2");
        ent->s.skinnum = 1;
        ent->solid = SOLID_TRIGGER;
        ent->s.effects |= EF_SPHERETRANS | EF_ROTATE;
        ent->s.renderfx |= RF_IR_VISIBLE;
        VectorSet (ent->mins, -32, -32, -24);
        VectorSet (ent->maxs, 32, 32, -16);
        ent->touch=touch_jumppad;
        gi.linkentity (ent);
}


static void ModelFrame(edict_t *ent)
{
  char c;
  if (ent->s.frame != ent->gib1) {
    if (ent->s.frame < ent->gib1)
      ent->s.frame++;
    else
      ent->s.frame--;
    return;
  }
 
  c = *ent->team;
  if (!c) {
    ent->team = ent->message;
    c = ',';
  } else
    ent->team++;

  if (*ent->team < '0' || *ent->team > '9') {
    gi.dprintf("Invalid model animation framenum: %s\n", ent->team);
    ent->prethink = NULL;
    return;
  }
  
  ent->gib1 = strtol(ent->team, &ent->team, 10)&255;

  if (c == ',')
    ent->s.frame = ent->gib1;
  else
  if (c != '-') {
    gi.dprintf("Invalid model animation op: %c\n", c);
    ent->prethink = NULL;
    return;
  }
}

void SP_misc_md2 (edict_t *ent)
{
        gi.setmodel (ent, ent->model);
        
        if (st.model2)
          ent->s.modelindex2 = gi.modelindex(st.model2);
        if (st.model3)
          ent->s.modelindex3 = gi.modelindex(st.model3);
        
        // if ent->style is 0, set to 1, else style
        ent->s.skinnum = ent->style;
        ent->solid = SOLID_NOT;
        ent->s.frame = (int)ent->accel;
        VectorSet (ent->mins, -1, -1, -1);
        VectorSet (ent->maxs, 1, 1, 1);
        gi.linkentity (ent);
        
        if (ent->message && strlen(ent->message)) {
          ent->s.frame = ent->gib1 = strtol(ent->message, &ent->team, 10)&255;
          if (*ent->team == ',' || *ent->team == '-')
            ent->prethink = ModelFrame;
        }
}

/*
visible
returns 1 if the entity is visible to self, even if not infront ()

  FIXME: r1: made it not go through windows (stupid... ?)
*/
qboolean visible (edict_t *self, edict_t *other)
{
        vec3_t  spot1;
        vec3_t  spot2;
        trace_t trace;

        VectorCopy (self->s.origin, spot1);
        spot1[2] += self->viewheight;
        VectorCopy (other->s.origin, spot2);
        spot2[2] += other->viewheight;
        trace = gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE | CONTENTS_WINDOW);
        
        if (trace.fraction == 1.0)
                return true;
        return false;
}

qboolean CheckTargetEnt (edict_t * self)
{
        if (!self->target) {
                gi.dprintf ("REMOVED: %s (%s) at %s doesn't have a target\n", self->classname, self->targetname, vtos2(self));
                G_FreeEdict (self);
                return true;
        }

        return false;
}

qboolean CheckTargetnameEnt (edict_t *self)
{
        if (!self->targetname) {
                gi.dprintf ("REMOVED: %s (%s) at %s doesn't have a targetname\n", self->classname, self->target, vtos2(self));
                G_FreeEdict (self);
                return true;
        }

        return false;
}
