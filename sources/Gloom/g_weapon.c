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
#include "c4.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t /*@out@*/result);


/*void temp_point (int type, vec3_t origin) {
        gi.dprintf ("temp_point: t=%d, o=%s\n", type, vtos(origin));
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(type);
        gi.WritePosition(origin);
}

void temp_line (int type, vec3_t origin, vec3_t tr_endpos) {
        gi.dprintf ("temp_line: t=%d, o=%s, t=%s\n", type, vtos(origin), vtos(tr_endpos));
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(type);
        gi.WritePosition(origin);
        gi.WritePosition(tr_endpos);
}

void temp_impact (int type, vec3_t origin, vec3_t movedir) {
        gi.dprintf ("temp_impact: t=%d, o=%s, m=%s\n", type, vtos(origin), vtos(movedir));
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(type);
        gi.WritePosition(origin);
        gi.WriteDir(movedir);
}

void temp_splash (int type, int count, vec3_t origin, vec3_t movedir, int style) {
        gi.dprintf ("temp_splash: t=%d, c=%d, o=%s, m=%s, t=%d\n", type, count, vtos(origin), vtos(movedir), style);
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(type);
        gi.WriteByte(count);
        gi.WritePosition(origin);
        gi.WriteDir(movedir);
        gi.WriteByte(style);
}*/


/*
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
*/
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod, int dflags)
{
        trace_t         tr;
        vec3_t          dir;
        vec3_t          forward, right, up;
        vec3_t          end;
        float           r;
        float           u;
        vec3_t          water_start = {0,0,0};
        qboolean        water = false;
        int                     content_mask = MASK_SHOT | MASK_WATER;

        if (self->client)
                tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);

        if (!self->client || !(tr.fraction < 1.0f && !(tr.startsolid && tr.ent && tr.ent->takedamage)))
        {
                vectoangles (aimdir, dir);
                AngleVectors (dir, forward, right, up);

                r = crandom()*hspread;
                u = crandom()*vspread;
                VectorMA (start, 8192, forward, end);
                VectorMA (end, r, right, end);
                VectorMA (end, u, up, end);

                if (gi.pointcontents (start) & MASK_WATER)
                {
                        water = true;
                        VectorCopy (start, water_start);
                        content_mask &= ~MASK_WATER;
                }

                //gi.dprintf ("tracing to %s\n", vtos (end));
                tr = gi.trace (start, NULL, NULL, end, self, content_mask);

                // see if we hit water
                if (tr.contents & MASK_WATER)
                {
                        int             color;

                        water = true;
                        VectorCopy (tr.endpos, water_start);

                        if (!VectorCompare (start, tr.endpos))
                        {
                                if (tr.contents & CONTENTS_WATER)
                                {
                                        if (strcmp(tr.surface->name, "*brwater") == 0)
                                                color = SPLASH_BROWN_WATER;
                                        else
                                                color = SPLASH_BLUE_WATER;
                                }
                                else if (tr.contents & CONTENTS_SLIME)
                                        color = SPLASH_SLIME;
                                else if (tr.contents & CONTENTS_LAVA)
                                        color = SPLASH_LAVA;
                                else
                                        color = SPLASH_UNKNOWN;

                                if (color != SPLASH_UNKNOWN)
                                {
                                        temp_splash (TE_SPLASH, 8, tr.endpos, tr.plane.normal, color);
                                        gi.multicast (tr.endpos, MULTICAST_PVS);
                                }

                                // change bullet's course when it enters water
                                VectorSubtract (end, start, dir);
                                vectoangles (dir, dir);
                                AngleVectors (dir, forward, right, up);
                                r = crandom()*hspread*2;
                                u = crandom()*vspread*2;
                                VectorMA (water_start, 8192, forward, end);
                                VectorMA (end, r, right, end);
                                VectorMA (end, u, up, end);
                        }

                        // re-trace ignoring water this time
                        tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
                }
        }


        // send gun puff / flash
        if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
        {
                if (tr.fraction < 1.0f || (tr.startsolid && tr.ent && tr.ent->takedamage))
                {
                        if (tr.ent->takedamage)
                        {
                                if (tr.fraction < 0.08f && self->client && tr.ent->client && self->client->resp.team == tr.ent->client->resp.team &&
                                        (tr.ent->client->resp.class_type == CLASS_BIO || tr.ent->client->resp.class_type == CLASS_ENGINEER) && tr.ent->health > 0)
                                {
                                        if (tr.ent->delay < level.time)
                                        {
                                                tr.ent->delay = level.time + 2;
                                                gi.centerprintf (tr.ent, "%s %s requests your attention!", classlist[self->client->resp.class_type].classname, self->client->pers.netname);
                                        }
                                        //return;
                                }

                                if(mod == MOD_PULSELASER)
                                {
                                        T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY | dflags, mod);
                                }
                                else if (mod == MOD_MGTURRET)
                                {
                                        if (self->target_ent == tr.ent)
                                                damage = 1;
                                        T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET | dflags, mod);
                                }
                                else
                                {
                                        T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET | dflags, mod);
                                }
                        }
                        else
                        {
                                if (strncmp (tr.surface->name, "sky", 3) != 0)
                                {
                                        if (te_impact != TE_LASER_SPARKS) {
                                                temp_impact (te_impact, tr.endpos, tr.plane.normal);
                                        } else {
                                                temp_splash (te_impact, 20, tr.endpos, tr.plane.normal, 0xf3f3f1f1);
                                        }

                                        gi.multicast (tr.endpos, MULTICAST_PVS);

                                        if(te_impact == TE_LASER_SPARKS)
                                        {
                                                temp_impact (TE_BLASTER, tr.endpos, tr.plane.normal);
                                                gi.multicast (tr.endpos, MULTICAST_PVS);
                                        }

//                                      if (self->client)
//                                              PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
                                }
                        }
                }
        }

        // if went through water, determine where the end and make a bubble trail
        if (water)
        {
                vec3_t  pos;

                VectorSubtract (tr.endpos, water_start, dir);
                VectorNormalize (dir);
                VectorMA (tr.endpos, -2, dir, pos);
                if (gi.pointcontents (pos) & MASK_WATER)
                        VectorCopy (pos, tr.endpos);
                else
                        tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

                VectorAdd (water_start, tr.endpos, pos);
                VectorScale (pos, 0.5f, pos);

                temp_line (TE_BUBBLETRAIL, water_start, tr.endpos);
                gi.multicast (pos, MULTICAST_PVS);
        }
}


/*
fire_bullet

Fires a single round.   Used for machinegun and chaingun.       Would be fine for
pistols, rifles, etc....
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
        fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod, 0);
}


/*
fire_shotgun

Shoots shotgun pellets. Used by shotgun and super shotgun.
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
        int             i;

        for (i = 0; i < count; i++)
                fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod, 0);
}


/*
fire_blaster

Fires a single blaster bolt.    Used by the blaster and hyper blaster.
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int mod = MOD_HYPERBLASTER;
        if (other == self->owner || other->enttype == ENT_BLASTER)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (self);
                return;
        }

        if (self->enttype == ENT_TARGET_BLASTER)
                mod = MOD_TARGET_BLASTER;

//      if (self->owner->client)
//              PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

        if (other->takedamage)
        {
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 15, DAMAGE_BULLET, mod);
        }
        else
        {
                if (!plane) {
                        temp_impact (TE_BLASTER, self->s.origin, vec3_origin);
                } else {
                        temp_impact (TE_BLASTER, self->s.origin, plane->normal);
                }

                gi.multicast (self->s.origin, MULTICAST_PVS);

                T_RadiusDamage(self,self->owner,90,self,100,MOD_HYPERBLASTER, 0);
        }

        G_FreeEdict (self);
}

void blaster_think (edict_t *self)
{
        G_FreeEdict (self);
}

edict_t* fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int type)
{
        edict_t *bolt;
        trace_t tr;

        VectorNormalize (dir);

        bolt = G_Spawn();
        bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
        //bolt->s.modelindex = gi.modelindex ("models/objects/tlaser/tris.md2");
        bolt->s.effects |= effect;
        //bolt->s.renderfx |= RF_FULLBRIGHT;
        bolt->s.sound = SoundIndex (misc_lasfly);

        VectorCopy (start, bolt->s.origin);
        VectorCopy (start, bolt->s.old_origin);
        vectoangles (dir, bolt->s.angles);
        VectorScale (dir, speed, bolt->velocity);
        bolt->movetype = MOVETYPE_FLYMISSILE;
        //bolt->svflags |= SVF_DEADMONSTER;
        bolt->svflags |= SVF_NOPREDICTION;
        bolt->clipmask = MASK_SHOT;
        bolt->solid = SOLID_BBOX;
        bolt->owner = self;
        bolt->touch = blaster_touch;
        bolt->nextthink = level.time + 5;
        bolt->think = blaster_think;
        bolt->dmg = damage;
        bolt->classname = "blaster bolt";
        bolt->enttype = type;
        gi.linkentity (bolt);

        tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
                bolt->touch (bolt, tr.ent, NULL, NULL);
        }
        
        return bolt;
}

/*
fire_grenade
*/

// GLOOM - Smoke grenade, code modelled after regular grenade code
void smoke_animate(edict_t *ent)
{
        edict_t* target = NULL;
        //vec3_t v;
        int base_smoke, radius;
        float   dist;
        int affected_team = TEAM_NONE;

        ent->nextthink = level.time + 0.1f;

        ent->s.frame ++;

        if (ent->enttype == ENT_GAS_SPORE) {
                T_GasDamage(ent, ent->owner, 19, ent, 195, MOD_GAS_SPORE, 0);
        } else if (ent->enttype == ENT_GASSER_GAS) {
                T_GasDamage(ent, ent->owner, 10, ent->owner, 175, MOD_GASSER, 0);
        }

        if (ent->enttype == ENT_GAS_SPORE || ent->enttype == ENT_GASSER_GAS) {
                //target->client->smokeintensity=0;
                affected_team = TEAM_HUMAN;
        } else {
                affected_team = TEAM_ALIEN;
        }

        // GLOOM - Adjust screen blend for player if near exploded smoke grenades
        // Blend changes (in player_state) are only sent 10 times/sec so there's no point in doing this in ClientThink
        // Other possibilities : Adjust blend based on the direction player is facing (i.e. into smoke, away from smoke)

        if (ent->enttype == ENT_SMOKE_GRENADE)
                radius = 450;
        else
                radius = 300;

        if (ent->enttype == ENT_SMOKE_GRENADE || ent->enttype == ENT_GASSER_GAS) {
                while ((target = findradius_c(target, ent, radius)) != NULL)
                {
                        //if (target == ignore)
                                //continue;
                        if (!visible(ent,target))
                                continue;

                        base_smoke=target->client->smokeintensity;

                        if(target->client->resp.team == affected_team)
                        {
                                //VectorSubtract(ent->s.origin, target->s.origin, v);
                                dist = Distance (ent->s.origin, target->s.origin);
                                if (dist < radius)      {
        //                              gi.cprintf(ent, PRINT_HIGH, "Near smoke grenade - %f\n", dist);
                                        if (ent->enttype == ENT_SMOKE_GRENADE) {
                                                //float effect = 100 - (300-dist/100;
                                                //if (effect > target->client->smokeintensity)
                                                //      target->client->smokeintensity += effect;
                                                if (dist / 20 > 0)
                                                        target->client->smokeintensity += 18 - dist/20;
                                        } else {
                                                if (dist / 10 > 0)
                                                        target->client->smokeintensity += 20 - dist/10;
                                        }
                                }
                        }

                        if (target->client->smokeintensity < base_smoke)
                                target->client->smokeintensity=base_smoke-3;

                        if (target->client->smokeintensity > 100)
                                target->client->smokeintensity = 100;
                }
        }

        if(ent->s.frame >= 36) {
                if (ent->enttype == ENT_SMOKE_GRENADE) {
                        ent->owner->s.sound = 0;
                        ent->owner->think = G_FreeEdict;
                        ent->owner->nextthink = level.time + 6;
                }
                G_FreeEdict(ent);
        }
}

static void throw_cloud (edict_t *ent)
{
        edict_t *cloud;

        cloud = G_Spawn();
        VectorCopy (ent->s.origin, cloud->s.origin);
        cloud->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
        cloud->s.renderfx = RF_TRANSLUCENT;
        cloud->s.frame = 10;
        cloud->movetype = MOVETYPE_BOUNCE;
        cloud->owner = ent;
        cloud->think = smoke_animate;
        cloud->enttype = ENT_SMOKE_GRENADE;
        cloud->nextthink = level.time + 0.1f;

        gi.linkentity (cloud);
}

static void Spore_Explode (edict_t *cloud)
{
        cloud->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
        cloud->s.renderfx = RF_TRANSLUCENT;
        cloud->movetype = MOVETYPE_BOUNCE;
        cloud->s.frame = 10;
        cloud->s.skinnum = 1;
        cloud->think = smoke_animate;
        cloud->s.event = EV_OTHER_TELEPORT;
        cloud->nextthink = level.time + 0.1f;
}

void fire_shrapnel (edict_t *ent)
{
        int             i, attempts;
        trace_t tr;
        vec3_t  end, dist;
        vec3_t  mins = {-1,-1,-1};
        vec3_t  maxs = {1,1,1};
        vec3_t  start;

        attempts = 0;

        for (i = shrapnel_count->value; i; i--)
        {
        	vec3_t dir = {crandom(), crandom(), crandom()};
        	VectorNormalize(dir);

                VectorMA(ent->s.origin, 1024, dir, end);
                VectorMA(ent->s.origin, 0.5, dir, start);

                tr = gi.trace(start, mins, maxs, end, ent, MASK_SHOT);

                if (tr.startsolid || tr.fraction < 8.0f/1024 || !tr.ent || !tr.ent->takedamage)
                {
                        VectorSubtract (ent->s.origin, tr.endpos, dist);
                        if (VectorLength (dist) < 16)
                        {
                                if (++attempts < 100)
                                        i++;
                                continue;
                        }
                }

                if (!tr.ent || (tr.fraction == 1.0 && !tr.startsolid))
                        continue;
/*
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_DEBUGTRAIL);
                gi.WritePosition (start);
                gi.WritePosition (tr.endpos);
                gi.multicast (start, MULTICAST_PVS);
*/
                if (tr.ent && tr.ent->takedamage)
                {
                        //gi.bprintf (PRINT_HIGH, "shrap hit for %d dmg\n", (int)shrapnel_damage->value);
                        T_Damage (tr.ent, ent, ent->owner, end, tr.endpos, tr.plane.normal, shrapnel_damage->value, 0, DAMAGE_BULLET, MOD_SHRAPNEL);
                }
                else
                {
                        if (random() < .5)
                                temp_impact (TE_GUNSHOT, tr.endpos, tr.plane.normal);
                }

        }
}

void spiker_shoot(edict_t *self);
static void Grenade_Explode (edict_t *ent)
{
        vec3_t          origin;


//gi.bprintf (PRINT_HIGH,"BOOM at %f,%f,%f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);

        // stop explosion if player isn't valid
        if (!ent->owner->inuse || (ent->owner->client && ent->owner->client->resp.team != TEAM_HUMAN))
        {
                G_FreeEdict(ent);
                return;
        }

        // GLOOM - Smoke grenade explosion
        if (ent->enttype == ENT_SMOKE_GRENADE)
        {
                ent->s.sound = SoundIndex (weapons_gg_on);

                ent->think = throw_cloud;
                ent->nextthink = level.time + 0.1f;
                return;
        }

        if (ent->enttype == ENT_FLASH_GRENADE)
        {
                //int len;
                //vec3_t delta;
                edict_t *spiker = NULL;
                while ((spiker = G_Find3 (spiker, ENT_SPIKER)))
                {
                        //VectorSubtract (ent->s.origin, spiker->s.origin, delta);
                        //len = VectorLength (delta);
                        if (Distance (ent->s.origin, spiker->s.origin) <= ent->dmg_radius * 2.5)
                        {
                                if (spiker->dmg && spiker->count <= 0)
                                {
                                        spiker->enemy = ent;
                                        spiker->dmg = 1;
                                        spiker_shoot (spiker);
                                }
                                spiker->count = 40 + random() * 20;
                        }
                }
                T_RadiusDamage(ent, ent->owner, ent->dmg*3, ent, ent->dmg_radius*2, MOD_FLASH, 0);
        }
        else {
                //T_RadiusDamage(ent, ent->owner, ent->dmg * 1.66, ent, ent->dmg_radius, MOD_GRENADE, DAMAGE_IGNORE_CLIENTS);
                //T_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius * 0.9, MOD_GRENADE, DAMAGE_IGNORE_STRUCTS);
                T_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius, MOD_GRENADE, 0);

                if (ent->owner && ent->owner->client &&
                        ((ent->owner->client->resp.class_type == CLASS_COMMANDO && (ent->owner->client->resp.upgrades & UPGRADE_CMDO_SHRAPNEL)) ||
                        (ent->owner->client->resp.class_type == CLASS_EXTERM && (ent->owner->client->resp.upgrades & UPGRADE_XT_SHRAPNEL))))
                        fire_shrapnel (ent);
        }

        VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

        if (ent->waterlevel)
        {
                if (ent->groundentity) {
                        temp_point (TE_GRENADE_EXPLOSION_WATER, origin);
                } else {
                        temp_point (TE_ROCKET_EXPLOSION_WATER, origin);
                }
        } else {
                temp_point (TE_GRENADE_EXPLOSION, origin);
        }

        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

static void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  //tk redirection magic
  if (attacker->client && attacker->client->resp.team == TEAM_ALIEN) //easy case first
    self->tkredirect = attacker;
  else
  if (attacker->client && attacker->client->resp.team == TEAM_HUMAN) { //needs some magic here to try guess correctly
#warning "TODO: finish proper tk redirection in nades."
    if (inflictor->enttype == ENT_ROCKET || inflictor->enttype == ENT_GRENADE_NEW)
      if (inflictor->spawnframenum > self->spawnframenum)  //older one is at fault. TODO: Only apply for lifes > 0.5 sec since spawn
        self->tkredirect = inflictor->owner;
  }



        self->takedamage = DAMAGE_NO;
        self->nextthink = level.time + .1f;
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == ent->owner)
                return;

        if (ent->enttype == ENT_FLASH_GRENADE)
        {
                Grenade_Explode (ent);
                return;
        }

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

        // don't play sound on organic grens
        if (ent->enttype != ENT_GAS_SPORE && ent->enttype != ENT_SPIKE_SPORE && ent->enttype != ENT_PROXY_SPORE && ent->touch_debounce_time < level.time) {
                int i = randomMT() & 3;
                if (i == 0)
                        gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_hgrenb1a), 1, ATTN_IDLE, 0);
                else if (i == 1)
                        gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_hgrenb2a), 1, ATTN_IDLE, 0);
                else
                        gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_grenlb1b), 1, ATTN_IDLE, 0);
        }

        ent->touch_debounce_time = level.time + .2f;
}

edict_t *fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
        edict_t *grenade;
        vec3_t  dir;
        vec3_t  forward, right, up;

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();
        grenade->enttype = ENT_GRENADE_NEW;
        VectorCopy (start, grenade->s.origin);
        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        VectorSet (grenade->avelocity, 300, 300, 300);
        grenade->movetype = MOVETYPE_BOUNCE;
        //grenade->clipmask = MASK_SHOT;
        grenade->clipmask = MASK_SOLID | CONTENTS_MONSTER;
        grenade->solid = SOLID_BBOX;
        grenade->svflags = SVF_DEADMONSTER; //Don't block players
        grenade->flags |= FL_NO_HURTSPARKS;
        grenade->flags |= FL_NO_KNOCKBACK;
        grenade->classname = "grenade";
        grenade->teleport_time = level.time;

        grenade->owner = self;
        
        if (self && self->client)
          grenade->air_finished = self->client->resp.class_type;

        // handles bouncing sound
        grenade->touch = Grenade_Touch;

        grenade->nextthink = level.time + timer;

        grenade->s.renderfx = RF_IR_VISIBLE;

        // throw sound
        gi.sound (self, CHAN_AUTO, SoundIndex (weapons_hgrent1a), 1, ATTN_IDLE, 0);


        // if no dmg, spawn smoke gren
        if (!damage)
        {
                grenade->dmg = 1000;
                grenade->s.modelindex = gi.modelindex ("models/objects/ggrenade/tris.md2");
                grenade->think = Grenade_Explode;
                grenade->enttype = ENT_SMOKE_GRENADE;
        } else {

                // ticking sound for grenade
                grenade->s.sound = SoundIndex (weapons_hgrenc1b);
                grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");

                grenade->dmg = damage;
                grenade->dmg_radius = damage_radius;

                grenade->s.effects = EF_PLASMA;
                grenade->health = 66;
                grenade->hurtflags = WC_GRENADE | WC_ROCKET | WC_EXSHELL | WC_C4;
                grenade->takedamage = DAMAGE_YES;
                grenade->die = Grenade_Die;
                grenade->think = Grenade_Explode;
                grenade->enttype = ENT_GRENADE;
        }

        gi.linkentity (grenade);
        return grenade;
}

void Spike_Explode (edict_t *ent)
{
        vec3_t  dir, start;
        int             n;
        float random;
        edict_t *sent = NULL;

        VectorCopy (ent->s.origin, start);
        start[2] += 16.0;

        T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius, MOD_SPIKESPORE, 0);

        while ((sent = findradius(sent, ent->s.origin, ent->dmg_radius * 2.5f)) != NULL)
        {
                if (!sent->takedamage)
                        continue;

                if (sent->classname[0] && !Q_stricmp(sent->classname, "target_laser"))
                        T_Damage (sent,sent,ent->owner,vec3_origin,vec3_origin,vec3_origin,100,10,0,MOD_SPIKE);
        }

        random = random() * 2 * M_PI;

        for(n=0; n<SPIKE_GRENADE_SPIKE_COUNT; n++)
        {
                //dir[0] = sin((((float) (n * (360.0/SPIKE_GRENADE_SPIKE_COUNT))) * M_PI) / 180.0f);
                //dir[1] = cos((((float) (n * (360.0/SPIKE_GRENADE_SPIKE_COUNT))) * M_PI) / 180.0f);
                 //and cos((float) r + n *.... cos(r + (float) n
                dir[0] = cos(random + (float) n * (2.0f * M_PI / (float) SPIKE_GRENADE_SPIKE_COUNT));
                dir[1] = sin(random + (float) n * (2.0f * M_PI / (float) SPIKE_GRENADE_SPIKE_COUNT));
                dir[2] = 0;
                fire_spike (ent, start, dir, 100, SPIKE_GRENADE_SPIKE_SPEED);
        }

        G_FreeEdict (ent);
}

void fire_spike_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer, int radius, int damage)
{
        edict_t *grenade;
        vec3_t  dir;
        vec3_t  forward, right, up;

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();

        VectorCopy (start, grenade->s.origin);
        grenade->s.modelindex = self->client ? gi.modelindex ("models/objects/sspore/tris.md2") : 0;
        grenade->s.renderfx = RF_IR_VISIBLE;

        //FIXME: HACK HACK ETC (eh?)

        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        VectorSet (grenade->avelocity, 300, 300, 300);
        if (self->client && self->client->resp.class_type == CLASS_GUARDIAN)
                grenade->movetype = MOVETYPE_TOSS;
        else
                grenade->movetype = MOVETYPE_BOUNCE;
        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;
        grenade->owner = self;

        // makes sure that if it touches sky, it gets removed
        grenade->touch = Grenade_Touch;

        grenade->nextthink = level.time + timer;
        grenade->dmg_radius = radius;
        grenade->radius_dmg = damage;
        grenade->enttype = ENT_SPIKE_SPORE;
        grenade->think = Spike_Explode;
        grenade->classname = "spike spore";

        if (self->client)
                gi.sound(self, CHAN_AUTO, SoundIndex (weapons_webshot1), 1, ATTN_NORM, 0);

        gi.linkentity (grenade);
}


void fire_spore (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer)
{
        edict_t *grenade;
        vec3_t  dir;
        vec3_t  forward, right, up;

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();

        VectorCopy (start, grenade->s.origin);
        grenade->s.modelindex = self->client ? gi.modelindex ("models/objects/spore/tris.md2") : 0;
        grenade->s.renderfx = RF_IR_VISIBLE;

        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        VectorSet (grenade->avelocity, 300, 300, 300);

        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;

        grenade->owner = self;
        
        grenade->movetype = MOVETYPE_BOUNCE;

        // makes sure that if it touches sky, it gets removed
        grenade->touch = Grenade_Touch;

        grenade->think = Spore_Explode;
        grenade->nextthink = level.time + timer;

        grenade->enttype = ENT_GAS_SPORE;
        grenade->classname = "gas spore";

        if (self->client)
                gi.sound(self, CHAN_AUTO, SoundIndex (weapons_webshot1), 1, ATTN_NORM, 0);

        gi.linkentity (grenade);
}

void proxy_explode (edict_t *ent,int count,int damage, qboolean nasty)
{       
        vec3_t  dir, start;
        int             n;

        if (ent->deadflag)
                return;

        ent->deadflag = DEAD_DEAD;

        VectorCopy (ent->s.origin, start);
        start[2] += 16.0;

        if (!ent->owner || !ent->owner->inuse || (ent->owner->client && ent->owner->client->resp.class_type != CLASS_GUARDIAN))
        {
                ent->owner = ent;
        }

        T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius, MOD_SPIKESPORE, 0);

        if (nasty)
                T_RadiusDamage (ent,ent->owner,170,ent,140,MOD_SPIKE, 0);

        for(n=0; n < count; n++)
        {
                dir[0] = crandom()*180;
                dir[1] = crandom()*180;
                if (nasty)
                        dir[2] = random()*15;
                else
                        dir[2] = 0;
                fire_spike (ent, start, dir, damage, SPIKE_GRENADE_SPIKE_SPEED);
        }

        G_FreeEdict (ent);
}

static void Proxy_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == ent->owner)
                return;

        if (other->client) {
                if (other->client->resp.team == TEAM_ALIEN)
                        return;
                if (other->client->resp.team == TEAM_HUMAN && other->health > 0) {
                        proxy_explode (ent,5,30,false);
                        return;
                }
        }
        else
        {
                ent->movetype = MOVETYPE_NONE;
                VectorClear (ent->velocity);
        }

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

        if (ent->touch_debounce_time < level.time)
                gi.sound (ent, CHAN_AUTO, SoundIndex (weapons_grenlb1b), 1, ATTN_NORM, 0);

        ent->touch_debounce_time = level.time + .1f;
}

void proxy_shower (edict_t *self);
void proxy_check (edict_t *ent)
{
        edict_t *targ = NULL;

        if (ent->owner && (!ent->owner->inuse || ent->owner->client && (ent->owner->client && ent->owner->client->resp.class_type != CLASS_GUARDIAN)))
        {
                G_FreeEdict(ent);
                return;
        }

        ent->nextthink = level.time + .2f;

        while ((targ = findradius_c (targ,ent,150))) {
                if (targ->client->resp.team == TEAM_HUMAN && visible (ent,targ)) {
                        //proxy_explode(ent,10,100,true);
                        proxy_shower (ent);
                        break;
                }
        }
}

void proxy_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        proxy_explode (self,3,30,false);

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                attacker->client->resp.total_score++;

}

void spiketrop_make_semisolid(edict_t *ent)
{
  if (--ent->newcount > 0) return;

  ent->takedamage = DAMAGE_YES;

  ent->svflags |= SVF_DEADMONSTER;
  ent->solid = SOLID_BBOX;
  ent->clipmask = MASK_SHOT & ~CONTENTS_DEADMONSTER;
  ent->prethink = NULL;
  gi.linkentity(ent);
}

void spiketrop_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->enttype == ENT_PROXYSPIKE && !ent->groundentity) { //If touched other spike become non solid to avoid sticking each other
	  ent->svflags &= ~SVF_DEADMONSTER;
	  ent->solid = SOLID_NOT;
          ent->clipmask = MASK_SOLID;
	  ent->prethink = spiketrop_make_semisolid;
	  ent->newcount = 1+randomMT()%3;
	}

        if (other == world && ent->velocity[2] < 0)
        {
                VectorClear (ent->avelocity);
                VectorClear (ent->velocity);
                VectorCopy (plane->normal, ent->pos1);
        } else if (other->client && other->client->resp.team == TEAM_HUMAN && other != ent->target_ent) {
                T_Damage (other, ent, ent->owner?ent->owner:ent, vec3_origin, ent->s.origin, vec3_origin, 10, 0, 0, MOD_SPIKE);
//gi.cprintf(other, PRINT_HIGH, "SPIKED! (normal)\n");
        }

        ent->takedamage = DAMAGE_YES;
}

void spiketrop_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        temp_impact (TE_GUNSHOT, self->s.origin, self->pos1);
        gi.multicast (self->s.origin, MULTICAST_PVS);
        G_FreeEdict (self);
}

void spiketrop_think (edict_t *ent)
{
        vec3_t temp;


        VectorCopy (ent->s.origin, temp);
        temp[2] -= ent->maxs[2];
        
        if (gi.pointcontents (temp) & CONTENTS_SOLID)
        {
                ent->s.origin[2]--;
        }
        else
        {
                if (ent->random == 0)
                {
                        ent->s.origin[2]--;
                        temp[2]--;
                        if (gi.pointcontents (temp) & CONTENTS_SOLID)
                                ent->random = 1;
                }
                else
                {
                        G_FreeEdict (ent);
                        return;
                }
        }

        temp[2] += ent->maxs[2]*2;

        if (gi.pointcontents (temp) & CONTENTS_SOLID)
        {
                G_FreeEdict (ent);
        }
        else
        {
                ent->nextthink = level.time + .1f;
        }

        gi.linkentity (ent);
}

/*void spiketrop_takedamage (edict_t *ent)
{
        ent->takedamage = DAMAGE_YES;
        ent->think = spiketrop_think;
        ent->nextthink = level.time + ent->delay;
}*/

static void spike_heatseek(edict_t*ent)
{
  int c;
  float s, d;
  vec3_t dir;

  if (ent->groundentity) {
    if (ent->groundentity == world)
      ent->prethink = NULL; //wont move anymore
    ent->target_ent = NULL;
    return;
  }

  c = gi.pointcontents(ent->s.origin);
  if (c & CONTENTS_LAVA) {
    G_FreeEdict(ent);
    return;
  }

  //Pick target if needed
  if (!ent->target_ent || !ent->target_ent->inuse || !ent->target_ent->solid ||
     !(ent->target_ent->s.renderfx & RF_IR_VISIBLE) || Distance(ent->target_ent->s.origin, ent->s.origin) > 200 - (randomMT()&127)) {
    int score = -999;
    int s;

    edict_t*targ;

    ent->target_ent = NULL;

    targ = world;
    while ((targ = findradius_c(targ, ent, 200)) != NULL) {

      //Only aim for clients
      if (targ > g_edicts+game.maxclients) break;

      //Dont care of team, they'r heat seeking, not smart seeking
      if (!(targ->s.renderfx & RF_IR_VISIBLE) || !gi.inPVS(ent->s.origin, targ->s.origin))
        continue;
      
      s = (randomMT()&127) - Distance(targ->s.origin, ent->s.origin)*0.5f;
      if (s > score) {
        score = s;
        ent->target_ent = targ;
      }
    }
  }


  if (ent->target_ent) {
    float z, xyd;
    vec3_t o;
    VectorSubtract(ent->target_ent->s.origin, ent->s.origin, dir);
    xyd = dir[0]*dir[0] + dir[1]*dir[1];
    z = dir[2];
    
    d = -(dir[2]+ent->target_ent->maxs[2]);
    
    //Overshoot
    if (VectorNormalize2(ent->velocity, o) > 16)
      VectorMA(dir, 16, o, dir);

    dir[2] = 0;

    //Predict a bit
    if (d > 0 && (dir[0]*ent->velocity[0] + dir[1]*ent->velocity[1]) >= 0) {
      if (d > 50) d = 50;
      VectorMA(dir, d, ent->target_ent->velocity, dir);
      dir[2] = 0;
    }
    
    //Randomize
    if (dir[0]*dir[0] + dir[1]*dir[1] > 45*45) {
      dir[0] += crandom()*ent->size[1]*2.5f;
      dir[1] += crandom()*ent->size[2]*2.5f;
      d = VectorNormalize(dir);
      dir[0] += crandom()*0.175f;
      dir[1] += crandom()*0.175f;
    } else {
      d = VectorNormalize(dir);
      dir[0] += crandom()*0.5f;
      dir[1] += crandom()*0.5f;
    }
    
    if (ent->target_ent->health <= 0) {
      dir[0] += crandom()*0.55f;
      dir[1] += crandom()*0.55f;
    }

    if (xyd <= 33*33) { //Increase chances of hitting without being too obvious
      edict_t*other = ent->target_ent;
      if (z+8 >= other->mins[2] && z-8 <= other->maxs[2] && other->client && other->client->resp.team == TEAM_HUMAN) {
//gi.cprintf(other, PRINT_HIGH, "SPIKED! (hax=%.1f [%.1f/%.1f/%.f1])\n", sqrt(xyd), other->mins[2], z, other->maxs[2]);
        T_Damage(other, ent, ent->owner?ent->owner:ent, vec3_origin, ent->s.origin, vec3_origin, 10, 0, 0, MOD_SPIKE);
      }
    }

    //Accelerate    
    s = sqrt(ent->velocity[0]*ent->velocity[0]+ent->velocity[1]*ent->velocity[1]);
    if (s > d) {
      if (s < 330) {
        s += 65;
        if (s > 330) s = 330;
      }
    } else
    if (s > d-100) {
      if (s > 150) {
        s -= 35;
        if (s < 150) s = 150;
      }
    }

    //Aim
    if (s > 1) {
      float rd = (2-ent->random)/s;
      float ri = ent->random;
      ent->velocity[0] = (ent->velocity[0]*rd + dir[0]*ri)*s*0.5f;
      ent->velocity[1] = (ent->velocity[1]*rd + dir[1]*ri)*s*0.5f;
    }
  }
}


void spew_spiketrops (edict_t *self, edict_t *owner, int count, float duration, float dur_random)
{
        float vs = 1.25f; //Increase spew range since nowdays spikes rarely hit
        
        int i, d;
        edict_t *e;
        vec3_t dir, end;
        trace_t tr;
        edict_t*target;

        gi.unlinkentity(self);

        for (i = 0; i < count; i++)
        {
                e = G_Spawn();
                e->classname = "spiketrop";
                e->touch = spiketrop_touch;
                e->solid = SOLID_NOT;
//                e->prethink = spiketrop_damage_delay;
                e->owner = owner;
                e->count = 3;

                e->clipmask = MASK_SOLID;


                VectorSet (e->mins, -4, -4, -8);
                VectorSet (e->maxs, 4, 4, 8);

                e->enttype = ENT_PROXYSPIKE;
                e->takedamage = DAMAGE_NO;
                e->health = 50;

                e->die = spiketrop_die;
                //e->think = spiketrop_takedamage;
                //e->nextthink = level.time + FRAMETIME;
                e->delay = duration + random() * dur_random;

                //e->think = spiketrop_think;

                e->think = G_FreeEdict;
                e->nextthink = level.time + e->delay;

                e->s.modelindex = gi.modelindex ("models/objects/spike/tris.md2");

                e->movetype = MOVETYPE_BOUNCE;
                e->s.effects |= EF_SPHERETRANS;

		for (d = 0; d < 3; d++) {
                  e->velocity[0] = crandom();
                  e->velocity[1] = crandom();
                  e->velocity[2] = 0;
                  VectorNormalize(e->velocity);
                  VectorMA(self->s.origin, 40, e->velocity, end);
                  VectorMA(self->s.origin, 8, e->velocity, e->s.origin);
                  e->velocity[2] = crandom()*250;
                  if (self->groundentity) e->velocity[2] = 200+fabs(e->velocity[2]);
                  e->velocity[1] *= 250*vs;
                  e->velocity[0] *= 250*vs;
                  tr = gi.trace(e->s.origin, e->mins, e->maxs, end, e, MASK_SHOT & ~CONTENTS_DEADMONSTER);
                  if (tr.startsolid) continue;
                  if (tr.fraction > 0.2f && !tr.allsolid) break;
                }


                spiketrop_make_semisolid(e);

                e->avelocity[0] = crandom() * 180;
                e->avelocity[1] = crandom() * 180;
                e->avelocity[2] = crandom() * 180;
                e->s.angles[0] = crandom() * 180;
                e->s.angles[1] = crandom() * 180;
                e->s.angles[2] = crandom() * 180;

		e->random = 0.35f+0.65f*fabs(crandom()); //1.0f for best aim, 0.0f for no aim
                e->prethink = spike_heatseek;
                if (i)
                  e->target_ent = target;

                spike_heatseek(e);
                target = e->target_ent;
                
                gi.linkentity (e);
        }
}

void proxy_shower (edict_t *self)
{
        //vec3_t temp;

        if (self->owner && (!self->owner->inuse || self->owner->client && (self->owner->client && self->owner->client->resp.class_type != CLASS_GUARDIAN)))
        {
                G_FreeEdict(self);
                return;
        }

        spew_spiketrops (self, self->owner, 18, 6, 6);

        G_FreeEdict (self);
}

void fire_spike_proxy_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer, int radius, int damage)
{
        edict_t *grenade;
        vec3_t  dir;
        int activespores, besttime, max;
        vec3_t  forward, right, up;
        edict_t *e, *first;

        if (spiketest->value) {
                max = 1;
                e = NULL;
        
                while ((e = G_Find3 (e, ENT_PROXYSPIKE)) != NULL) {
                        if (e->owner == self)
                                spiketrop_die (e, e, e, 0, vec3_origin);
                }
        } else {
                max = 2;
        }

        e = first = NULL;
        activespores = 0;
        besttime = 0;

        while ((e = G_Find3 (e, ENT_PROXY_SPORE)) != NULL) {
                if (e->owner == self) {
                        if (e->air_finished < besttime || !besttime) {
                                first = e;
                                besttime = e->air_finished;
                        }
                        if (++activespores == max)
                                break;
                }
        }

        if (activespores == max)
                proxy_explode(first,3,5,false);

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();

        VectorCopy (start, grenade->s.origin);
        grenade->s.modelindex = gi.modelindex ("models/objects/sspore/tris.md2");
        grenade->s.renderfx |= RF_IR_VISIBLE;

        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        VectorSet (grenade->avelocity, 300, 300, 300);
        grenade->movetype = MOVETYPE_TOSS;
        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;
        VectorSet(grenade->mins,-8,-8,-8);
        VectorSet(grenade->maxs,8,8,8);
        grenade->owner = self;
        grenade->air_finished = level.time;
        grenade->die = proxy_die;
        grenade->takedamage = DAMAGE_YES;
        grenade->health = 20;

        if (spiketest->value) {
                grenade->nextthink = level.time + 30;
                grenade->think = proxy_shower;
        } else {
                grenade->nextthink = level.time + 1;
                grenade->think = proxy_check;
        }
        grenade->touch = Proxy_Touch;
        grenade->classname = "proxyspore";
        grenade->enttype = ENT_PROXY_SPORE;
        
        grenade->target_ent = grenade;	//So overrides can detect which is spore and which spike

        gi.sound(self, CHAN_AUTO, SoundIndex (weapons_webshot1), 1, ATTN_NORM, 0);

        gi.linkentity (grenade);
}

void rocket_explode (edict_t *ent, edict_t *other)
{
        if (!dmg_override || !GetExplosionInfo(NULL, ent, ent->owner, MOD_R_SPLASH)) {
          T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius+150, MOD_R_SPLASH, DAMAGE_IGNORE_CLIENTS);
          T_RadiusDamage(ent, ent->owner, ent->radius_dmg * 0.7f, other, ent->dmg_radius, MOD_R_SPLASH, DAMAGE_IGNORE_STRUCTS);
        } else
          T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius , MOD_R_SPLASH, 0);

        if (ent->owner->client && ent->owner->client->resp.class_type == CLASS_HEAVY && (ent->owner->client->resp.upgrades & (UPGRADE_HT_SHRAPNEL)))
                fire_shrapnel (ent);
        
        if (ent->waterlevel) {
                temp_point (TE_ROCKET_EXPLOSION_WATER, ent->s.origin);
        } else {
                temp_point (TE_EXPLOSION2, ent->s.origin);
        }

        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

static void rocket_explode_wrapper (edict_t *ent)
{
        rocket_explode (ent, ent);
}

/*
fire_rocket
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        // stop if player isn't valid
        if (!ent->owner->inuse || (ent->owner->client && ent->owner->client->resp.team != TEAM_HUMAN && !ent->owner->client->pers.adminpermissions))
        {
                G_FreeEdict(ent);
                return;
        }

        if (other == ent->owner || other->enttype == ENT_ROCKET)
                return;
                

        if (surf && (surf->flags & SURF_SKY))
        {
                //if (ent->owner && ent->owner->client)
                        //ent->owner->target_ent = NULL;
                G_FreeEdict (ent);
                return;
        }

        if (ent->deadflag)
                return;
        
        if (plane) VectorMA(ent->s.origin, 1.5, plane->normal, ent->s.origin);

        ent->deadflag = DEAD_DEAD;
        ent->takedamage = DAMAGE_NO;
        
        if (other->takedamage)
        {
                if (other->client)
                        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg * 0.9f, 300, DAMAGE_RADIUS, MOD_ROCKET);
                else
                        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 300, DAMAGE_RADIUS, MOD_ROCKET);
        }

        rocket_explode (ent, other);
}

static void rocket_aimdir(edict_t*ent)
{
    vec3_t dir;
    VectorNormalize2(ent->velocity, dir);
    vectoangles(dir, ent->s.angles);
    if (dir[2] == -1) ent->postthink = NULL;
}
static void rocket_fall(edict_t*ent)
{
    trace_t tr;
    if (ent->nextthink > level.time) {
      vec3_t end;
      VectorMA(ent->s.origin, 0.5f, ent->velocity, end);
      tr = gi.trace(ent->s.origin, tv(-1,-1,-1), tv(1,1,1), end, ent, MASK_SHOT);
   } else
      tr.fraction = 0;
    
    if (tr.fraction < 1) {
      ent->prethink = NULL;
      ent->postthink = rocket_aimdir;
      ent->nextthink = level.time + 12;
      ent->think = rocket_explode_wrapper;
      ent->movetype = MOVETYPE_TOSS;
      ent->s.effects = ent->s.effects & ~EF_ROCKET | EF_GRENADE;
    }
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *rocket;
        trace_t tr;

        rocket = G_Spawn();
        VectorCopy (start, rocket->s.origin);
        rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
        rocket->s.effects |= EF_ROCKET;
        rocket->s.renderfx |= RF_IR_VISIBLE;
        rocket->s.sound = SoundIndex (weapons_rockfly);

        VectorCopy (dir, rocket->movedir);
        vectoangles (dir, rocket->s.angles);
        VectorScale (dir, speed, rocket->velocity);
        rocket->movetype = MOVETYPE_FLYMISSILE;

        //r1: ht rockets pass through dead bodies (was MASK_SHOT)
        rocket->clipmask = MASK_SOLID | CONTENTS_MONSTER;
        rocket->solid = SOLID_BBOX;
        rocket->enttype = ENT_ROCKET;
        rocket->flags |= FL_NO_KNOCKBACK;
        rocket->flags |= FL_NO_HURTSPARKS;
        rocket->owner = self;

        //VectorSet (self->mins, -1, -1, -1);
        //VectorSet (self->maxs, 1, 1, 1);

        //r1: no prediction
        rocket->svflags |= SVF_NOPREDICTION;
        
        rocket->touch = rocket_touch;
        rocket->nextthink = level.time + 12;
        rocket->think = rocket_explode_wrapper;
        rocket->dmg = damage;
        rocket->radius_dmg = radius_damage;
        rocket->dmg_radius = damage_radius;
        rocket->classname = "rocket";

        rocket->health = 66;
        rocket->hurtflags = WC_GRENADE | WC_ROCKET | WC_EXSHELL | WC_C4;
        rocket->takedamage = DAMAGE_YES;
        rocket->die = Grenade_Die;

	if (self->client && self->client->resp.upgrades & UPGRADE_MORTAR_ENABLED) {
        	rocket->nextthink = level.time + 1.5;
        	rocket->think = rocket_fall;
        	rocket->prethink = rocket_fall;
	}

        // FIXME: does this hit dead bodies?
        // YES! ack. fixed.
        tr = gi.trace (rocket->s.origin, tv(-1,-1,-1), tv(1,1,1), rocket->s.origin, rocket, MASK_SOLID | CONTENTS_MONSTER);
        if (tr.fraction < 1.0)
        {
                if (tr.ent != self->owner)
                        rocket->touch (rocket, tr.ent, NULL, NULL);
        }

        gi.linkentity (rocket);
}

void drunk_missile_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == ent->owner || other->enttype == ENT_ROCKET)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

        if (ent->deadflag)
                return;

        ent->deadflag = DEAD_DEAD;
        
        if (other->takedamage)
                T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent->owner, ent->dmg_radius, ent->count, 0);
        else
                T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius, ent->count, 0);

        //T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius * 2, MOD_R_SPLASH, DAMAGE_IGNORE_CLIENTS);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_EXPLOSION1);
        gi.WritePosition (ent->s.origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

void drunk_missile_think (edict_t *ent)
{
        int i;

        for (i = 0; i < 3; i++) {
                ent->velocity[i] += crandom() * 50;
        }

        ent->nextthink = level.time + .1f;
}

edict_t* fire_drunk_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *rocket;
        trace_t tr;

        rocket = G_Spawn();
        VectorCopy (start, rocket->s.origin);
        VectorCopy (dir, rocket->movedir);
        vectoangles (dir, rocket->s.angles);
        VectorScale (dir, speed, rocket->velocity);
        rocket->movetype = MOVETYPE_FLYMISSILE;

        //r1: ht rockets pass through dead bodies (was MASK_SHOT)
        rocket->clipmask = MASK_SOLID | CONTENTS_MONSTER;
        rocket->solid = SOLID_TRIGGER;
        rocket->s.effects |= EF_ROCKET;
        rocket->enttype = ENT_ROCKET;
        VectorClear (rocket->mins);
        VectorClear (rocket->maxs);
        rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
        rocket->owner = self;
        rocket->svflags |= SVF_DEADMONSTER;
        rocket->touch = drunk_missile_touch;
        rocket->nextthink = level.time + .1f;
        rocket->think = drunk_missile_think;
        rocket->dmg = damage;
        rocket->radius_dmg = radius_damage;
        rocket->dmg_radius = damage_radius;
        rocket->s.sound = SoundIndex (weapons_rockfly);
        rocket->classname = "drunkrocket";
        rocket->s.renderfx |= RF_IR_VISIBLE;

        rocket->takedamage = DAMAGE_NO;
        
        rocket->count = MOD_DRUNK_MISSILE;

        tr = gi.trace (rocket->s.origin, tv(-1,-1,-1), tv(1,1,1), rocket->s.origin, rocket, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                if (tr.ent != self->owner)
                        rocket->touch (rocket, tr.ent, NULL, NULL);
        }

        gi.linkentity (rocket);
  return rocket;
}

void fire_grim_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
  edict_t*rocket = fire_drunk_missile(self, start, dir, damage, speed, damage_radius, radius_damage);
  rocket->classname = "grimrocket";
  rocket->count = MOD_GRIM_ROCKET;
}





static void bay_missile_explode(edict_t *ent)
{
  T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, ent->count, DAMAGE_LINEAR);

  gi.WriteByte (svc_temp_entity);
  gi.WriteByte (TE_EXPLOSION1);
  gi.WritePosition (ent->s.origin);
  gi.multicast (ent->s.origin, MULTICAST_PVS);

  G_FreeEdict (ent);
}

static void bay_missile_step(edict_t *ent)
{
  vec3_t forward;
  int i;
  ent->speed += 56*FRAMETIME;

  if (ent->speed > 512) ent->speed = 512;

  for (i = 0; i < 3; i++)
    ent->s.angles[i] = fmod(ent->s.angles[i]+ent->pos1[i]*FRAMETIME, 360);

  AngleVectors(ent->s.angles, forward, NULL, NULL);
  for (i = 0; i < 3; i++)
    forward[i] += crandom()*0.075f;
  VectorNormalize(forward);
  VectorScale(forward, ent->speed, ent->velocity);
  VectorCopy(ent->s.origin, ent->pos2);
  VectorAdd(ent->s.origin, ent->velocity, ent->s.origin);

  if (level.time > ent->air_finished) ent->teammaster = NULL;
  trace_t tr = gi.trace(ent->pos2, ent->mins, ent->maxs, ent->s.origin, ent->teammaster, MASK_SHOT);
  if (tr.fraction < 1 || tr.startsolid) {
    VectorAdd(tr.endpos, tr.plane.normal, ent->s.origin);
    bay_missile_explode(ent);
    return;
  }

  gi.linkentity(ent);
}

void fire_bay_missile(edict_t *owner, vec3_t start, vec3_t dir, vec3_t bias, int damage, float radius)
{
  int i;
  edict_t*ent = G_Spawn();
  ent->classname = "missile";
  ent->enttype = ENT_ROCKET;

  VectorSet(ent->mins, -4, -4, -4);
  VectorSet(ent->maxs,  4,  4,  4);
  VectorCopy(start, ent->pos2);
  VectorCopy(start, ent->s.origin);
  VectorCopy(bias, ent->pos1);
  vectoangles(dir, ent->s.angles);
  
  ent->s.modelindex = gi.modelindex("models/objects/rocket/tris.md2");
  ent->s.effects = EF_ROCKET;
  ent->s.renderfx = RF_IR_VISIBLE;
  ent->s.sound = SoundIndex(weapons_rockfly);

  ent->count = MOD_MECH_MISSILE;
  ent->speed = 64;
  ent->air_finished = level.time+0.25f;
  ent->teammaster = ent->owner = owner;
  ent->dmg = damage;
  ent->dmg_radius = radius;

  ent->prethink = bay_missile_step;
  ent->think = bay_missile_explode;
  ent->nextthink = level.time+10;

  ent->pos1[2] = 0;
  if (VectorLength(ent->pos1) > 270) {
    bay_missile_explode(ent);
    return;
  }
  ent->pos1[2] = bias[2];
  if (ent->pos1[2] > 90) ent->pos1[2] = 90;
  else
  if (ent->pos1[2] < -90) ent->pos1[2] = -90;

  gi.linkentity(ent);
}


/*
fire_rail
*/
void SpawnDamage2 (int type, vec3_t origin, vec3_t normal, int count, int color);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean turret)
{
        vec3_t          from;
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore;
        int                     mask;
        int                     i, hax;
        int                     points;
        int			dflags = 0;
        //qboolean      water;
        int hits = 0;
        //vec3_t                endpos;

        VectorMA (start, 8192, aimdir, end);
        VectorCopy (start, from);
        ignore = self;
        //water = false;
        mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

        hax = 0;

        //tr = gi.trace (from, NULL, NULL, end, self, MASK_SHOT);
        //VectorCopy(tr.endpos,endpos);
        tr = gi.trace (from, NULL, NULL, end, ignore, mask);
        while (ignore)
        {
                points = damage;
                if (hax++ > 20)
                        break;

                if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
                {
                        mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
                        //water = true;
                }
                //r1: crash fix (see fire_piercing)
                //else
                //{
                        if (tr.ent->takedamage) //(tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
                                ignore = tr.ent;
                        else
                                ignore = NULL;

                        if (tr.ent && tr.ent != self && tr.ent->takedamage)
                        {
                                if (tr.ent->client)
                                {
                                        if (turret)
                                        {
                                                if (tr.ent->client->resp.team == TEAM_HUMAN)
                                                        points /= 4;
                                                else if (tr.ent->client->resp.class_type == CLASS_STINGER)
                                                        points /= 2.25f;
                                                else if (tr.ent->client->resp.class_type == CLASS_STALKER)
                                                        points *= 0.85f;
                                        }
                                        else
                                        {
                                                if (tr.ent->client->resp.class_type == CLASS_STINGER)
                                                        points /= 4;

                                                if (tr.ent->client->resp.class_type == CLASS_STALKER)
                                                        points /= 1.2f;

                                                if (tr.ent->client->resp.team == TEAM_ALIEN)
                                                        hits++;
                                        }
                                }
                                damage /= 2;
                                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, points, kick, dflags, turret ? MOD_RAIL_TURRET : MOD_RAIL);
                                dflags += DAMAGE_PASS_2;
                        }
                //}

                VectorCopy (tr.endpos, from);

                tr = gi.trace (from, NULL, NULL, end, ignore, mask);
        }

        // just skews points
        if (!turret && hits > 1)
        {
                int bonus = pow(3,hits);
                gi.bprintf (PRINT_HIGH,"%s got Jose smoking crack bitches (you think I'm coding that? HAH!)\n",self->client->pers.netname);
                gi.cprintf (self,PRINT_HIGH,"%d hits! %d bonus points awarded.\n",hits,bonus);
                self->client->resp.total_score += bonus;
                self->client->resp.score += (hits - 1);
                if (self->client->resp.score > MAX_SCORE->value)
                        self->client->resp.score = MAX_SCORE->value;
        }

        // send gun puff / flash
        /*gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_RAILTRAIL);
        gi.WritePosition (start);
        gi.WritePosition (tr.endpos);
        gi.multicast (self->s.origin, MULTICAST_PVS);*/

        for (i = 0; i < game.maxclients; i++) {
          edict_t*player = g_edicts+i+1;
          if (!player->inuse || !gi.inPVS(player->s.origin, start) && !gi.inPVS(player->s.origin, tr.endpos)) continue;
          temp_line((player != self) ? TE_RAILTRAIL : TE_BUBBLETRAIL, start, tr.endpos);
          gi.unicast(player, false);
        }

        if (!turret)
                SpawnDamage2 (TE_LASER_SPARKS, tr.endpos, tr.plane.normal, 20, 15);
}

/*
fire_piercing
*/
void fire_piercing (edict_t *self, vec3_t start, vec3_t aimdir, int te_impact, int hspread, int vspread, int damage, int kick, int mod)
{
        vec3_t          from;
        vec3_t          dir;
        vec3_t          forward, right, up;
        vec3_t          end;
        float           r;
        float           u;
        trace_t         tr;
        edict_t         *ignore;
        int                     mask, hits;
        int dflags = 0;
        //qboolean      water;

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        r = crandom()*hspread;
        u = crandom()*vspread;
        VectorMA (start, 8192, forward, end);
        VectorMA (end, r, right, end);
        VectorMA (end, u, up, end);

        VectorCopy (start, from);

        ignore = self;
        //water = false;
        hits = 0;
        mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

        // FIXME: copy in the bubble trail routine
        // FIXME: make this just call fire_lead with DAMAGE_PIERCING added, I don't think multiple hits is worthwhile
        // probability of having multiple piercing calls on same frame on later targets is very small and
        // without multihits, the damage doesn't go over absorb

        while (hits < 4)
        {
                tr = gi.trace (from, NULL, NULL, end, ignore, mask);

                if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
                {
                        mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
                        //water = true;
                }

        //r1: removed 'else', what happens if one shoots an entity that takes damage
        //but also has 'slime' set (for warping effect)? infinite loop, thats what
        //      else
        //      {
                        if (tr.ent && tr.ent->takedamage && tr.ent != self)
                        {
                                // novelty: draw the blood spew backwards if piercing
                                //if (ignore)
                                //      VectorInverse (tr.plane.normal);

                                if (tr.fraction < 0.08f && self->client && tr.ent->client && self->client->resp.team == tr.ent->client->resp.team &&
                                        (tr.ent->client->resp.class_type == CLASS_BIO || tr.ent->client->resp.class_type == CLASS_ENGINEER) && tr.ent->health > 0)
                                {
                                        if (tr.ent->delay < level.time)
                                        {
                                                tr.ent->delay = level.time + 2;
                                                gi.centerprintf (tr.ent, "%s %s requests your attention!", classlist[self->client->resp.class_type].classname, self->client->pers.netname);
                                        }
                                        //return;
                                }

                                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, dflags | DAMAGE_PIERCING, mod);
                                dflags += DAMAGE_PASS_3;
                                damage /= 3;

                                if (tr.ent->flags & FL_CLIPPING || tr.ent->solid == SOLID_BSP)
                                        // we hit human structures, end loop
                                        break;
                                else
                                        // we don't want to hit the same ent again
                                        ignore = tr.ent;

                                hits++; // fail safe

                        } else
                                // we hit something that doesn't take damage, end loop
                                break;
                //}

                VectorCopy (tr.endpos, from);
        }

        // send gun puff / flash
        if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
        {
                temp_impact(te_impact, tr.endpos, tr.plane.normal);
                gi.multicast (tr.endpos, MULTICAST_PVS);
        }
}

/*
fire_explosive

This is an internal support routine used for explosive based weapons.
*/
void fire_explosive (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
        trace_t         tr;
        vec3_t          dir;
        vec3_t          forward, right, up;
        vec3_t          end;
        float           r;
        float           u;
        int                     content_mask = MASK_SHOT;
        edict_t         te = {0};

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        r = crandom()*hspread;
        u = crandom()*vspread;
        VectorMA (start, 8192, forward, end);
        VectorMA (end, r, right, end);
        VectorMA (end, u, up, end);

        //if (self->client)
        //{
                tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);

                if (!(tr.fraction < 1.0) && !(tr.startsolid && tr.ent && tr.ent->takedamage))
                {
                        tr = gi.trace (start, NULL, NULL, end, self, content_mask);
                }

        //}

        // send gun puff / flash
        if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
        {
                if (tr.fraction < 1.0f || (tr.startsolid && tr.ent && tr.ent->takedamage))
                {
                        if (tr.ent && tr.ent->takedamage)
                        {
                                if (!tr.ent->client)
                                {
                                        if (tr.ent->enttype == ENT_COCOON)
                                                damage *= 3.5;
                                        else
                                                damage *= 4;
                                }
                                else
                                        damage *= 1.1f;
                                T_Damage (tr.ent, self, self, aimdir, self->s.origin, tr.plane.normal, damage, 0, 0, mod);
                        } else {
                                // hack for raddamage
                                if (self->client)
                                {
                                        te.owner = self;
                                        VectorCopy(tr.endpos,te.s.origin);
                                        T_RadiusDamage(&te, self, damage/3, NULL, 196, MOD_R_SPLASH, 0);
                                }
                        }

                        temp_point (te_impact, tr.endpos);
                        gi.multicast (tr.endpos, MULTICAST_PVS);
                }
        }

}

static void fire_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == self->owner || other->enttype == ENT_FLAMES)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (self);
                return;
        }

        if (other->takedamage)
        {
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 20, 0, DAMAGE_NO_KNOCKBACK, MOD_BURNED);
                //G_FreeEdict (self);
        }
}

static void fire_think(edict_t *self)
{
  int fm = SETTING_INT(fire_mode);
  
  if (fm != 1) {
    if (gi.pointcontents(self->s.origin) & (CONTENTS_SOLID | CONTENTS_WATER)) return;
  }

  if (fm == 2 || fm == 3) {
    float dmg = 12;    //44 was way too much
    float radius = 40; //52 was too much too
    int   fl = DAMAGE_NO_KNOCKBACK | DAMAGE_NO_END;

    vec3_t dist;
    VectorSubtract(self->s.origin, self->pos3, dist);
    if (dist[0]*dist[0]+dist[1]*dist[1]+dist[2]*dist[2] < radius) fl &= ~DAMAGE_NO_END; //if fire is stuck (speed < sqrt(radius)), dont exclude end
    
    SegmentRadiusDamage(self->pos3, self->s.origin, self, self->owner, dmg, /*knockback*/0, self->owner, radius, self->gib1, fl);
    VectorCopy(self->s.origin, self->pos3);
  } else
  if (fm == 1) {
        if (firetest->value)
                T_RadiusDamage (self, self->owner, 11 - firetest->value, self->owner, 52 - firetest->value, self->gib1, DAMAGE_NO_KNOCKBACK | DAMAGE_NO_FALLOFF);
        else
                T_RadiusDamage (self, self->owner, 44, self->owner, 52, self->gib1, DAMAGE_NO_KNOCKBACK);
  }


  VectorScale (self->velocity, 0.95f, self->velocity);
  self->velocity[2] += 5.0;

  if (--self->newcount == 3) {
    self->s.sound = 0;
    self->s.renderfx |= RF_TRANSLUCENT;
  } else
  if (self->newcount <= 0)
    self->think = G_FreeEdict;

  self->nextthink = level.time + FRAMETIME;
}

edict_t* fire_fire (edict_t *self, vec3_t start, vec3_t dir, float damage_radius)
{
        edict_t *fire;
        int fm = SETTING_INT(fire_mode);

        if (gi.pointcontents(start) & (CONTENTS_SOLID | CONTENTS_WATER)) return NULL;

	if (fm < 0 || fm > 3) { //invalid fire mode
	  gi.sound(self, CHAN_AUTO, SoundIndex(misc_keytry), 1, ATTN_NORM, 0);
	  return NULL;
	}

        fire = G_Spawn();
        VectorCopy (start, fire->s.origin);
        VectorCopy (start, fire->pos3);
        
        fire->gib1 = MOD_BURNED;

        dir[0] += crandom()*0.1f;
        dir[1] += crandom()*0.1f;
        dir[2] += crandom()*0.1f;
        VectorScale (dir, 700, fire->velocity);
        VectorAdd(fire->velocity,self->velocity,fire->velocity);
        VectorClear (fire->mins);
        VectorClear (fire->maxs);

        if(rand() & 1)
                fire->s.modelindex = firea_index;
        else
                fire->s.modelindex = fireb_index;

        fire->owner = self;
        fire->enttype = ENT_FLAMES;
        if (fm < 2)
          fire->touch = fire_touch;
        fire->classname = "fire blast";

        fire->think = fire_think;
        fire->nextthink = level.time + 0.1;
        fire->s.frame = randomMT() & 7;
        fire->newcount = 0.8f/FRAMETIME;

        fire->movetype = MOVETYPE_FLYMISSILE;
        fire->clipmask = MASK_SOLID;
        fire->flags |= FL_NOWATERNOISE;

        fire->solid = SOLID_NOT;
        fire->s.effects |= EF_ANIM_ALLFAST | EF_SPHERETRANS | EF_HYPERBLASTER;// | EF_FLAG1;
        
        if (!fm) {
          fire->svflags |= SVF_DEADMONSTER;
          fire->solid = SOLID_BBOX;
        }
        
        if (fire->touch) {
                trace_t tr;
                tr = gi.trace (fire->s.origin, tv(-1,-1,-1), tv(1,1,1), fire->s.origin, fire, MASK_SHOT);
                if (tr.fraction < 1.0)
                {
                        if (tr.ent != self)
                                fire->touch (fire, tr.ent, NULL, NULL);
                }
        }

        gi.linkentity (fire);

	return fire->inuse?fire:NULL;
}

/*
fire_melee
*/
edict_t* fire_melee (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, float dist)
{
        vec3_t          from;
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore;
        int                     mask;
        
        if (dmg_override)
          dist = OverrideSlashDistance(self, dist, MOD_MELEE);

        VectorMA (start, dist, aimdir, end);
        VectorCopy (start, from);
        ignore = self;
        mask = MASK_SHOT;

        tr = gi.trace (from, NULL, NULL, end, ignore, mask);

        /*gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_DEBUGTRAIL);
        gi.WritePosition (from);
        gi.WritePosition (end);
        gi.multicast (self->s.origin, MULTICAST_PVS);*/

        //damage *= (1.5f + -tr.fraction);
        //gi.bprintf (PRINT_HIGH, "depth dmg: multiplied by %f\n", (1.75 + -tr.fraction));
//      if (self->client)
//              PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

        if (tr.fraction < 1.0 || tr.startsolid)
        //r1: recoded less conditional checks
        if (tr.ent && tr.ent->takedamage)       {
                if (tr.ent->classname) {
                        if (self->waterlevel && (self->client->resp.class_type == CLASS_HATCHLING || self->client->resp.class_type == CLASS_KAMIKAZE)) {
                                if (tr.ent->enttype == ENT_HUMAN_BODY || tr.ent->enttype == ENT_INFEST || tr.ent->svflags & SVF_MONSTER) {
                                        return NULL;
                                }
                        }
                
                        // structure reduce?
                        if (tr.ent->svflags & SVF_MONSTER) {
                                damage /=4;
                        }
                }

                if (tr.ent->client) {
                        
                        if (tr.ent->client->resp.class_type == CLASS_MECH) {
                                if (self->client->resp.class_type==CLASS_HATCHLING || self->client->resp.class_type==CLASS_KAMIKAZE) {
                                        damage /=3;
                                }

                                //if (self->client->resp.class_type == CLASS_STINGER) {
                                //      damage *= 1.2;
                                //}
                        }

                } else {
                        // hitting something else than other player
                        if (self->client->resp.class_type == CLASS_GUARDIAN) {
                                damage *= 0.66f;

                        } else if (self->client->resp.class_type == CLASS_DRONE) {
                                damage *= 0.4f;
                        }
                }

                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_MELEE);
                return tr.ent;
        }
        else
        {
                if (!(self->waterlevel && (self->client->resp.class_type == CLASS_HATCHLING || self->client->resp.class_type == CLASS_KAMIKAZE))) {
                        gi.sound(self, CHAN_AUTO, SoundIndex (tank_thud), 1, ATTN_IDLE, 0);

                        temp_impact (TE_SPARKS, tr.endpos, tr.plane.normal);
                        gi.multicast (tr.endpos, MULTICAST_PVS);
                }
        }
        return NULL;
}

// Wordup


typedef struct
{
        float   initial_slow;
        float   extra_slow;
        float   initial_delay;
        float   extra_delay;
        float   max_slow;
        float   max_delay;
} slowlist_t;

/*
<appuru>         Engineer - 50% reduction initial, 7 seconds initial; 30% slow increment, 7 second increment; 80% reduction max, 20 seconds max. 
<appuru>         Grunt - 50% reduction initial, 7 seconds initial; 30% slow increment, 7 second increment; 80% reduction max, 20 seconds max.
<appuru>         Shock Trooper - 40%, 7 seconds initial; 40% slow increment, 7 second increment; 80% reduction max, 20 seconds max.
<appuru>         Biotech - 40%, 7 seconds initial; 40% slow increment, 7 second increment; 80% reduction max, 20 seconds max.
<appuru>         Heavy Trooper - 30%, 7 seconds initial; 50% slow increment, 7 second increment; 80% reduction max, 20 seconds max.
<appuru>         Commando - 30%, 5 seconds initial; 25% slow increment, 5 second increment; 80% reduction max, 15 seconds max.
<appuru>         Exterminator - 20%, 4 seconds initial; 30% slow increment, 4 second increment; 80% reduction max, 15 seconds max. 
<appuru>         Mech - 20%, 4 seconds initial; 25% slow increment, 4 second increment; 70% reduction max, 10 seconds max.*/

/*
#define         CLASS_GRUNT             0
#define         CLASS_HATCHLING 1
#define         CLASS_HEAVY             2
#define         CLASS_COMMANDO  3
#define         CLASS_DRONE             4
#define         CLASS_MECH              5
#define         CLASS_SHOCK             6
#define         CLASS_STALKER   7
#define         CLASS_BREEDER   8
#define         CLASS_ENGINEER  9
#define         CLASS_GUARDIAN  10
#define         CLASS_KAMIKAZE  11
#define         CLASS_EXTERM    12
#define         CLASS_STINGER   13
#define         CLASS_WRAITH    14
#define         CLASS_BIO               15*/
/*
static const slowlist_t slowlist[] = 
{
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0.6,   0.5,    7,      7,      0.9,    1},     //ht
        {0.5,   0.25,   5,      5,      0.8,    1},     //commando
        {0},
        {0.4,   0.25,   4,      4,      0.7,    1},     //mech
        {0.5,   0.4,    7,      7,      0.9,    1},     //st
        {0},
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //engi
        {0},
        {0},
        {0.4,   0.3,    6,      4,      0.6,    1},     //exterm
        {0},
        {0},
        {0.4,   0.4,    7,      7,      0.8,    1},     //bio
};
*/
static const slowlist_t slowlist[] = 
{
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
        {0},
        {0},
        {0.8,   0.3,    7,      7,      0.9,    1},     //grunt
};

void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);
static void Web_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

        if (other == ent->owner || other->enttype == ENT_DRONEWEB)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

        if(other->takedamage && other->client && other->client->invincible_framenum < level.framenum)
        {
                if(other->client->resp.team == TEAM_HUMAN || teamdamage->value)
                {
                        int             speed, oldspeed;
                        int             c;

                        c = other->client->resp.class_type;

                        if (c == CLASS_EXTERM && other->flags & FL_POWER_ARMOR)
                        {
                                other->client->resp.inventory[ITEM_AMMO_CELLS] -= 15;
                                if (other->client->resp.inventory[ITEM_AMMO_CELLS] < 0)
                                {
                                        other->client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                                        gi.cprintf(other, PRINT_HIGH, "Power cells overheated!\n");
                                        gi.sound(other, CHAN_AUTO, SoundIndex (weapons_pulseout), 1, ATTN_NORM, 0);
                                        other->client->last_move_time = level.time + 5;
                                        if (other->client->resp.upgrades & UPGRADE_COOLANT)
                                                other->client->last_move_time -= 2.5;
                                        other->flags &= ~FL_POWER_ARMOR;
                                }
                                SpawnDamage (TE_SCREEN_SPARKS, ent->s.origin, plane->normal, 15);
                                G_FreeEdict (ent);
                                return;
                        }

                        /*
                        if (classlist[other->client->resp.class_type].frags_needed < 1) {
                                other->client->frozentime += 1.25;
                        } else if (classlist[other->client->resp.class_type].frags_needed <= 3) {
                                other->client->frozentime += 0.9;
                                //T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, 1, 0, 0, MOD_ACID);
                        } else if (classlist[other->client->resp.class_type].frags_needed < 5) {
                                other->client->frozentime += 0.7;
                        } else {
                                other->client->frozentime += 0.5;
                        }

                        if (ent->owner->client && ent->owner->client->resp.upgrades & UPGRADE_MUCUS_GLANDS)
                                other->client->frozentime *= 1.2f;

                        //gi.bprintf (PRINT_HIGH, "frozen for %f sec\n", other->client->frozentime);
                        if (other->client->frozentime > 3.0)
                                other->client->frozentime = 3.0;

                        other->velocity[0] /= 2;
                        other->velocity[1] /= 2;
                        other->velocity[2] /= 2;

                        other->client->frozenmode = PMF_TIME_TELEPORT;
                        */


                        oldspeed = (int)((float)DEFAULT_VELOCITY * other->client->slow_threshold);
                        oldspeed = 0;

                        float tscale = (other->client->slow_timer >= 1.2f)?1.0f:2.85f;
                        other->client->slow_timer += 1.2f*tscale;
                        float max = 3.2f - classlist[other->client->resp.class_type].frags_needed*0.14f;
                        if (other->client->slow_timer > max) other->client->slow_timer = max;
                        other->client->slow_threshold = (2.0f+classlist[other->client->resp.class_type].frags_needed)/18.4f;

                        speed = (int)((float)DEFAULT_VELOCITY * other->client->slow_threshold);

                        if (speed != oldspeed && !other->client->alienstick)
                                SetClientVelocity (other, speed);
                        
                        T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, 1, 0, 0, MOD_ACID);
                }
        }

        if (other->takedamage && !other->client)
        {
                if ((other->enttype == ENT_TURRET || other->enttype == ENT_MGTURRET || other->enttype == ENT_TURRETBASE) && (other->random))
                {
                        if (other->enttype == ENT_TURRETBASE && other->target_ent)
                                other = other->target_ent;
                        other->enemy = ent->owner;
                        other->nextthink = level.time + .3f + (crandom() / 8);
                        //if (ent->owner->client && ent->owner->client->resp.upgrades & UPGRADE_MUCUS_GLANDS)
                        //      other->nextthink += .11f;
                }
        }

        // limit sound only further hits
        if (ent->fly_sound_debounce_time < level.framenum)
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_webshot2), 1, ATTN_IDLE, 0);

        temp_splash (TE_SPLASH, 8, ent->s.origin, plane->normal, SPLASH_SLIME);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

void fire_web (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
{
        edict_t *grenade;
        vec3_t  dir;
        vec3_t  forward, right, up;
        trace_t tr;

        //gi.dprintf("Boink!\n");

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();
        VectorCopy (start, grenade->s.origin);
        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        VectorCopy (self->s.angles, grenade->s.angles);
        grenade->s.angles[YAW] -= 180;
        //VectorInverse (grenade->s.angles);
        
        //VectorSet (grenade->avelocity, 300, 300, 300);
        grenade->movetype = MOVETYPE_TOSS;
        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;
        grenade->svflags |= SVF_DEADMONSTER;

        //r1: r1q2 specific no prediction
        grenade->svflags |= SVF_NOPREDICTION;

        VectorClear (grenade->mins);
        VectorClear (grenade->maxs);
        grenade->s.modelindex = gi.modelindex ("models/objects/web/ball.md2");
        grenade->owner = self;
        grenade->enttype = ENT_DRONEWEB;
        grenade->touch = Web_Touch;
        grenade->gravity = 1.35;
        grenade->classname = "web";

        //grenade->gravity = 3;

        // no sound for close hits
        grenade->fly_sound_debounce_time = level.framenum + 1;

        tr = gi.trace (grenade->s.origin, tv(-1,-1,-1), tv(1,1,1), grenade->s.origin, grenade, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                if (tr.ent != self)
                        grenade->touch (grenade, tr.ent, NULL, NULL);
        }

        gi.linkentity (grenade);
}

void spike_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == self->owner || other->enttype == ENT_SPIKE)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (self);
                return;
        }

        if (other->client && (other->client->resp.team == TEAM_HUMAN || teamdamage->value))
        {
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 50, 0, MOD_SPIKE);
                G_FreeEdict (self);
                return;
        }

        if (!other->client && other->takedamage)
        {
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 50, 0, MOD_SPIKE);
                G_FreeEdict (self);
                return;
        }

        //gi.bprintf (PRINT_HIGH, "hit %s\n", other->classname);

        // Hit wall or friendly
        // limit sound only further hits
        if (self->fly_sound_debounce_time < level.framenum)
                gi.sound (self, CHAN_AUTO, SoundIndex (world_ric1), 1, ATTN_NORM, 0);

        temp_impact (TE_GUNSHOT, self->s.origin, plane->normal);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        G_FreeEdict (self);
}

static void ProjectileStep(edict_t*self)
{
  if (self->timestamp != level.framenum) {
    float ft = FRAMETIME;
    vec3_t end = {self->s.origin[0]+self->velocity[0]*ft, self->s.origin[1]+self->velocity[1]*ft, self->s.origin[2]+self->velocity[2]*ft};
    trace_t tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_SHOT);
    if (tr.allsolid) tr.fraction = 0;
    VectorMA(self->s.origin, tr.fraction*ft, self->velocity, self->s.origin);
    gi.linkentity(self);
    self->timestamp = level.framenum;
    if (tr.fraction < 1.0)
      self->touch(self, tr.ent, &tr.plane, tr.surface);
  }
}


static void projectile_explode_touch(edict_t*self)
{
  cplane_t plane = {
    /*normal*/ {-self->s.origin[0], -self->s.origin[1], -self->s.origin[2]},
    /*dist*/ VectorLength(self->s.origin),
    /*type*/ 255,
    /*signbits*/ 0,
    /*pad*/ {0, 0}
  };
  
  csurface_t surf = {
    /*name*/ {0},
    /*flags*/ SURF_SKY,
    /*value*/ 0
  };
  
  VectorNormalize(plane.normal);
  
  self->touch(self, world, &plane, &surf);
  if (self->inuse && self->think == projectile_explode_touch)
    G_FreeEdict(self);
}

edict_t* fire_projectile(edict_t *self, vec3_t start, vec3_t dir, int enttype, float speed, void (*touch)(edict_t*, edict_t*, cplane_t*, csurface_t*))
{
  edict_t *bolt;

  VectorNormalize (dir);

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_NONE;
  bolt->clipmask = MASK_SHOT;
  bolt->prethink = ProjectileStep;
  bolt->svflags |= SVF_NOPREDICTION;
  bolt->solid = SOLID_BBOX;
  bolt->enttype = enttype;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  if (self->client || !self->owner) bolt->owner = self;
  else bolt->owner = self->owner;

  bolt->touch = touch;
  bolt->nextthink = level.time + 60;
  bolt->think = projectile_explode_touch;

  bolt->fly_sound_debounce_time = level.framenum + 1;

  gi.linkentity(bolt);
  
  return bolt;
}



void fire_spike (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
        edict_t *bolt;
        trace_t tr;

        VectorNormalize (dir);

        bolt = G_Spawn();
        VectorCopy (start, bolt->s.origin);
        VectorCopy (start, bolt->s.old_origin);
        vectoangles (dir, bolt->s.angles);
        VectorScale (dir, speed, bolt->velocity);
        bolt->movetype = MOVETYPE_NONE;
        bolt->clipmask = MASK_SHOT;
        bolt->prethink = ProjectileStep;

        //bolt->svflags = SVF_DEADMONSTER;
        
        //r1: r1q2 specific no prediction flag
        bolt->svflags |= SVF_NOPREDICTION;

        bolt->solid = SOLID_BBOX;
        bolt->enttype = ENT_SPIKE;

        VectorClear (bolt->mins);
        VectorClear (bolt->maxs);

        bolt->s.modelindex = gi.modelindex ("models/objects/spike/tris.md2");
        //bolt->s.effects |= effect;
        if(!self->client)                                       // fix for spike grenades
        {
                if(self->owner)
                        bolt->owner = self->owner;
                else
                        bolt->owner = self;
        } else {
                bolt->owner = self;
        }


        bolt->touch = spike_touch;
        bolt->nextthink = level.time + 50;
        bolt->think = G_FreeEdict;
        bolt->dmg = damage;
        //bolt->s.effects |= (EF_TELEPORTER * EF_GIB);
        bolt->classname = "spike";

        // no sound for close hits
        bolt->fly_sound_debounce_time = level.framenum + 1;

//        ProjectileStep(bolt);
        gi.linkentity (bolt);

        //tr = gi.trace (bolt->s.origin, bigmins, bigmaxs, dir, 

        tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
                bolt->touch (bolt, tr.ent, &tr.plane, tr.surface);
        }

}

void acid_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == self->owner || other->enttype == ENT_WRAITHBOMB)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (self);
                return;
        }

        // FIXME: double t_damages?
        if (other->client && other->client->resp.team == TEAM_HUMAN)
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 40, 10, DAMAGE_PIERCING | DAMAGE_RADIUS, MOD_ACID);
        else
                T_RadiusDamage (self, self->owner, 95, self->owner, 145, MOD_ACID, DAMAGE_PIERCING | DAMAGE_NO_KNOCKBACK);

        if (self->owner && self->owner->client && (self->owner->client->resp.upgrades & UPGRADE_VENOM_GLAND))
        {
                if (other->client && !other->deadflag && other->client->invincible_framenum <= level.framenum && (other->client->resp.class_type == CLASS_GRUNT ||
                        other->client->resp.class_type == CLASS_BIO || other->client->resp.class_type == CLASS_COMMANDO ||
                        other->client->resp.class_type == CLASS_HEAVY))
                {
                        if (!other->client->acid_duration)
                                other->client->acid_duration = 60;
                        else
                                other->client->acid_duration += 35;

                        other->client->acid_damage += 4;
                        other->client->acid_attacker = self->owner;
                }
                else
                {
                        edict_t *ent = NULL;

                        while (((ent = findradius_c (ent, self, 150)) != NULL))
                        {
                                if (!(visible(ent, self)))
                                        continue;

                                if (ent->client->invincible_framenum > level.framenum)
                                        continue;

                                if (ent->client->resp.class_type == CLASS_GRUNT || ent->client->resp.class_type == CLASS_BIO || ent->client->resp.class_type == CLASS_COMMANDO || ent->client->resp.class_type == CLASS_HEAVY)
                                {
                                        vec3_t dist;
                                        int len;

                                        if (!ent->client->acid_duration)
                                                ent->client->acid_duration = 30;
                                        else
                                                ent->client->acid_duration += 10;

                                        VectorSubtract (ent->s.origin, self->s.origin, dist);
                                        len = (int)VectorLength (dist);

                                        if (len < 50)
                                                ent->client->acid_damage += 3;
                                        else if (len < 100)
                                                ent->client->acid_damage += 2;
                                        else if (len < 150)
                                                ent->client->acid_damage += 1;

                                        ent->client->acid_attacker = self->owner; // save the poisoner
                                }
                        }
                }
        }

        gi.sound (self, CHAN_AUTO, SoundIndex (weapons_acid), 1, ATTN_IDLE, 0);

        /*
        temp_point (TE_BFG_BIGEXPLOSION, self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        while (((ent = findradius_c (ent, self, 200)) != NULL)) {
                if (!(visible(ent, self)))
                        continue;

                if (ent->client->invincible_framenum > level.framenum)
                        continue;

                if (ent->client->resp.class_type == CLASS_GRUNT || ent->client->resp.class_type == CLASS_BIO || ent->client->resp.class_type == CLASS_COMMANDO || ent->client->resp.class_type == CLASS_HEAVY)
                {
                        vec3_t dist;
                        int len;

                        if (!ent->client->acid_duration)
                                ent->client->acid_duration = 60;
                        else
                                ent->client->acid_duration += 30;

                        if (other->client && other->client->resp.team == TEAM_HUMAN)
                        {
                                ent->client->acid_damage += 5;
                        }
                        else
                        {
                                VectorSubtract (ent->s.origin, self->s.origin, dist);
                                len = (int)VectorLength (dist);

                                if (len < 50)
                                        ent->client->acid_damage += 4;
                                else if (len < 100)
                                        ent->client->acid_damage += 3;
                                else if (len < 150)
                                        ent->client->acid_damage += 2;
                                else
                                        ent->client->acid_damage += 1;
                        }


                        ent->client->acid_attacker = self->owner; // save the poisoner
                }
        }*/

        temp_splash (TE_SPLASH, 8, self->s.origin, plane->normal, SPLASH_SLIME);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        G_FreeEdict (self);
}

void fire_wraith_acid (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
        trace_t tr;
        vec3_t end;
        int n=0;
        const int dist=200;
        int loopAlert = 100;

        while (loopAlert)
        {
                VectorNormalize(dir);
                VectorScale(dir, dist, dir);
                VectorAdd(dir, start, end);

                /*gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_DEBUGTRAIL);
                gi.WritePosition (start);
                gi.WritePosition (end);
                gi.multicast (self->s.origin, MULTICAST_PVS);*/

                tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

                if (tr.fraction == 1.0f)
                {
                        n++;
                        dir[2]-=(dist/2)*n;
                        if (dir[2]<=-dist)
                                dir[2]=-dist;

                        VectorCopy(tr.endpos,start);
                        loopAlert--;
                        continue;
                }
                else
                {
                        break;
                }
        }
        
        VectorCopy(tr.endpos, end);
        if (tr.ent->takedamage)
        {
                if (self->client)
                {
                        if (tr.ent->client && !tr.ent->deadflag && tr.ent->client->invincible_framenum <= level.framenum && (tr.ent->client->resp.class_type == CLASS_GRUNT ||
                                tr.ent->client->resp.class_type == CLASS_BIO || tr.ent->client->resp.class_type == CLASS_COMMANDO ||
                                tr.ent->client->resp.class_type == CLASS_HEAVY))
                        {
                                if (!tr.ent->client->acid_duration)
                                        tr.ent->client->acid_duration = 60;
                                else
                                        tr.ent->client->acid_duration += 30;


                                tr.ent->client->acid_damage += 3;
                                tr.ent->client->acid_attacker = self;
                        }
                }
                T_Damage(tr.ent, self, self, dir, end, tr.plane.normal, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_ACID);
        }

        if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
        {
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_SPLASH);
                gi.WriteByte (8);
                gi.WritePosition (end);
                gi.WriteDir (tr.plane.normal);
                gi.WriteByte (SPLASH_SLIME);
                gi.multicast (end, MULTICAST_PVS);
        }
}

void fire_acid (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
{
        edict_t *grenade;
        vec3_t  dir;
        trace_t tr;
        vec3_t  forward, right, up;//, temp;

        //gi.dprintf("Boink!\n");

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        grenade = G_Spawn();
        VectorCopy (start, grenade->s.origin);
        VectorScale (aimdir, speed, grenade->velocity);
        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        //VectorScale (self->velocity, 2.66, temp);
        //VectorAdd (temp, grenade->velocity, grenade->velocity);
        VectorCopy (self->s.angles, grenade->s.angles);
        //VectorSet (grenade->avelocity, 300, 300, 300);
        grenade->movetype = MOVETYPE_TOSS;
        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;
        grenade->svflags |= SVF_DEADMONSTER;
        grenade->svflags |= SVF_NOPREDICTION;
        grenade->enttype = ENT_WRAITHBOMB;
        VectorClear (grenade->mins);
        VectorClear (grenade->maxs);
        grenade->s.modelindex = gi.modelindex ("models/objects/spore/tris.md2");
        grenade->s.skinnum = 1;
        grenade->gravity = 1;//.25;
        grenade->owner = self;
        grenade->touch = acid_touch;
        grenade->classname = "acidspore";

        tr = gi.trace (grenade->s.origin, tv(-1,-1,-1), tv(1,1,1), grenade->s.origin, grenade, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                if (tr.ent != self)
                        grenade->touch (grenade, tr.ent, NULL, NULL);
        }

        gi.linkentity (grenade);
}
