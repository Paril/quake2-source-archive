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

// Turret code for Gloom, derived from id's turret code in g_turret.c
// check visible

// 10/18/98 Cyberdog - Initial code

#include "g_local.h"
#include "turret.h"

#define MAXTARGETDISTANCE       512
#define LG_STRENGTH     325

#define MG_STRENGTH     100

void AnglesNormalize(vec3_t vec)
{
        while(vec[0] > 360)
                vec[0] -= 360;
        while(vec[0] < 0)
                vec[0] += 360;
        while(vec[1] > 360)
                vec[1] -= 360;
        while(vec[1] < 0)
                vec[1] += 360;
}

//void fire_beam(edict_t *self, vec3_t start, vec3_t dir, int dmg);

void turret_laser_die_animate(edict_t *ent)
{
        ent->s.frame ++;

        if(ent->s.frame > 7)
        {
                G_FreeEdict(ent);
                return;
        }

        ent->nextthink = level.time + 0.05f;
}

void turret_rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        //vec3_t                origin;

        if (other == ent->owner)
                return;

        if (ent->owner->owner && other == ent->owner->owner)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

        T_RadiusDamage(ent, ent->owner, turret_damage->value, other, 200, MOD_R_SPLASH, 0);
        
        if (ent->waterlevel) {
                temp_point (TE_ROCKET_EXPLOSION_WATER, ent->s.origin);
        } else {
                temp_point (TE_EXPLOSION2, ent->s.origin);
        }

        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

void turret_laser_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        //vec3_t                origin;

        if (other == ent->owner)
                return;

        if (ent->owner->owner && other == ent->owner->owner)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }

//      if (ent->owner->client)
//              PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

        // calculate position for the explosion entity
        //VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

        if (other->client)
        {
                if (other->client->resp.team == TEAM_ALIEN)
                        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, DAMAGE_ENERGY, MOD_TARGET_LASER);
                else
                        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, (int)(ent->dmg * 0.25), 0, DAMAGE_ENERGY, MOD_TARGET_LASER);
        }
        else
                if (other->takedamage)
                        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_TARGET_LASER);

        //gi.sound (ent, CHAN_AUTO, SoundIndex (turret_Tlasexplode), 1, ATTN_NORM, 0);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BLASTER2);//te_impact);
        gi.WritePosition (ent->s.origin);
        gi.WriteDir (plane->normal);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        G_FreeEdict (ent);
}

static void laser_home (edict_t *ent)
{
        edict_t *blip;
        vec3_t  targetdir;
        vec_t   speed;

        if (ent->enemy->health < 1 || !visible(ent, ent->enemy)) {
                ent->think = NULL;
                ent->nextthink = 0;
                return;
        }

        blip = ent->enemy;

        VectorSubtract(blip->s.origin, ent->s.origin, targetdir);

        // target acquired, nudge our direction toward it
        if (blip->client->resp.class_type == CLASS_HATCHLING || blip->client->resp.class_type == CLASS_KAMIKAZE) {
                targetdir[2] -= 10;
                VectorScale (ent->velocity, 1.01, ent->velocity);
                VectorNormalize(targetdir);
                VectorScale(targetdir, turret_homingturnscale->value, targetdir);
        } else {
                VectorNormalize(targetdir);
                VectorScale(targetdir, .25, targetdir);
        }

        
        VectorAdd(targetdir, ent->movedir, targetdir);
        VectorNormalize(targetdir);
        VectorCopy(targetdir, ent->movedir);
        vectoangles(targetdir, ent->s.angles);
        speed = VectorLength(ent->velocity);
        VectorScale(targetdir, speed, ent->velocity);

        if (++ent->count > 50)
                G_FreeEdict (ent);
        else
                ent->nextthink = level.time + FRAMETIME;
}

void bfg_think (edict_t *self)
{
        edict_t *ent;

        ent = NULL;
        while ((ent = findradius_c(ent, self, 256)) != NULL)
        {
                if (ent->client->resp.team != TEAM_ALIEN)
                        continue;

                if (!CanDamage (ent, self))
                        continue;

                T_Damage (ent, self, self->owner, vec3_origin, vec3_origin, vec3_origin, 5, 0, DAMAGE_ENERGY|DAMAGE_NO_KNOCKBACK, MOD_TARGET_LASER);

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_BFG_LASER);
                gi.WritePosition (self->s.origin);
                gi.WritePosition (ent->s.origin);
                gi.multicast (self->s.origin, MULTICAST_PHS);
        }

        self->nextthink = level.time + FRAMETIME;
}


void fire_turret_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *laser;
        trace_t tr;
        
        if (turret_mode->value == 3)
        {
                int             i;
                vec3_t  begin, end;
                edict_t *ignore;

                VectorMA (start, 2048, dir, end);

                start[2] += 14;

                gi.sound (self, CHAN_AUTO, SoundIndex (weapons_davelas), 1, ATTN_NORM, 0);

                VectorCopy (start, begin);

                ignore = self;

                //gi.bprintf (PRINT_HIGH, "turret %p is firing! %d\n", self, self->fly_sound_debounce_time);

                for (i = 0; i < 50; i++)
                {
                        tr = gi.trace (begin, NULL, NULL, end, ignore, MASK_PLAYERSOLID);
                        if (tr.ent && tr.ent->takedamage)
                        {
                                T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 9, 15, DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK, MOD_TARGET_LASER);
                                VectorCopy (tr.endpos, begin);
                                ignore = tr.ent;
                        }
                        else
                                break;
                }

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_BFG_LASER);
                gi.WritePosition (start);
                gi.WritePosition (tr.endpos);
                gi.multicast (self->s.origin, MULTICAST_PVS);

                return;
        }

        laser = G_Spawn();

                                                                                                                
        VectorCopy (start, laser->s.origin);
        //VectorCopy (dir, laser->movedir);
        vectoangles (dir, laser->s.angles);
        VectorScale (dir, speed, laser->velocity);
        laser->movetype = MOVETYPE_FLYMISSILE;
        laser->clipmask = MASK_SHOT;
        laser->solid = SOLID_BBOX;
        laser->owner = self;
        //laser->nextthink = level.time + 4.0;
        //laser->think = G_FreeEdict;
        laser->dmg = damage;
        laser->s.sound = SoundIndex (misc_lasfly);
        laser->s.renderfx |= RF_FULLBRIGHT;
        //laser->svflags |= SVF_DEADMONSTER;
        laser->svflags |= SVF_NOPREDICTION;
        laser->classname = "turretlaser";
        laser->enttype = ENT_TURRET_BLOB;
        laser->enemy = self->enemy;

        if (turret_mode->value == 0) {
                gi.sound (self, CHAN_AUTO, SoundIndex (weapons_davelas), 1, ATTN_NORM, 0);
                laser->s.modelindex = gi.modelindex ("models/objects/tlaser/tris.md2");
                laser->touch = turret_laser_touch;
                VectorClear (laser->mins);
                VectorClear (laser->maxs);
        } else if (turret_mode->value == 1) {
                gi.sound (self, CHAN_AUTO, gi.soundindex ("weapons/bfg__l1a.wav"), 1, ATTN_NORM, 0);
                laser->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
                laser->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
                if (turret_bfg->value) {
                        laser->think = bfg_think;
                        laser->nextthink = level.time + .1f;
                }
                VectorSet (laser->mins, -8, -8, -8);
                VectorSet (laser->maxs, 8, 8, 8);
                laser->touch = turret_laser_touch;
        } else if (turret_mode->value == 2)
        {
                gi.sound (self, CHAN_AUTO, SoundIndex(weapons_rocklf1a), 1, ATTN_NORM, 0);
                laser->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
                laser->touch = turret_rocket_touch;
                laser->s.effects |= EF_ROCKET;
        } else
          laser->touch = turret_laser_touch;

        if (turret_home->value && turret_bfg->value != 1) {
                if (laser->enemy && laser->enemy->s.effects & EF_DOUBLE) {
                        VectorScale (laser->velocity, turret_homingvelocityscale->value, laser->velocity);
                        laser->s.effects |= EF_IONRIPPER;

                        if (turret_home->value == 2)
                        {
                                laser->think = laser_home;

                                //r1: set initial directions or it could go in a straight line if the target
                                //    moves out of LOS while firing - meaning when it sees the target it will
                                //    turn through any angle it pleases to track it.
                                laser->think (laser);   
                        }
                }
        }

        tr = gi.trace (laser->s.origin, tv(-2,-2,-2), tv(2,2,2), laser->s.origin, laser, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                if (tr.ent != self->owner)
                        laser->touch (laser, tr.ent, NULL, NULL);
        }

        gi.linkentity (laser);
}

//#define HDEF1_ACCURACYTOLERANCE 8
#define HDEF1_VERTICALCAP 35

static float AngleSubtract( float a1, float a2 ) {
  float a;
  
  a = a1 - a2;
  while ( a > 180 ) {
    a -= 360; 
  }
  while ( a < -180 ) {
    a += 360;
  }
  return a;
}

//this code provided by Timbo (used in tremulous, http://tremulous.sourceforge.net)
//works a lot nicer than existing gloom turret code, plus doesn't shoot through
//the base etc :)

static qboolean hdef1_trackenemy( edict_t *self )
{
        vec3_t  dirToTarget, angleToTarget, angularDiff, temp2;
        float    temp;
        float    HDEF1_ACCURACYTOLERANCE;
        vec3_t enemyOrigin;
        int len;

        HDEF1_ACCURACYTOLERANCE = self->count;

        VectorCopy (self->enemy->s.origin, enemyOrigin);

        if (self->accel) {
                float mult;
                VectorCopy(self->enemy->velocity,temp2);
                temp2[2] = 0;
                //VectorSubtract(self->enemy->s.origin, self->s.origin, delta);
                //len = VectorLength(delta);
                len = Distance (self->enemy->s.origin, self->s.origin);
                mult = len/(1200.0f - (len));
                VectorScale(temp2,mult,temp2);
                VectorAdd(enemyOrigin,temp2,enemyOrigin);
        }

        if (self->enemy->client->resp.class_type == CLASS_HATCHLING || self->enemy->client->resp.class_type == CLASS_KAMIKAZE)
                enemyOrigin[2] -= 20;

        VectorSubtract( enemyOrigin, self->s.origin, dirToTarget );
        
        VectorNormalize( dirToTarget );
        vectoangles( dirToTarget, angleToTarget );

        angularDiff[ PITCH ] = AngleSubtract( self->s.angles[ PITCH ], angleToTarget[ PITCH ] );
        angularDiff[ YAW ] = AngleSubtract( self->s.angles[ YAW ], angleToTarget[ YAW ] );

        if( angularDiff[ PITCH ] < -HDEF1_ACCURACYTOLERANCE ) {
                self->s.angles[ PITCH ] += self->speed;
                angularDiff[ PITCH ] = AngleSubtract( self->s.angles[ PITCH ], angleToTarget[ PITCH ] );
                if( angularDiff[ PITCH ] > HDEF1_ACCURACYTOLERANCE ) {
                        self->s.angles[ PITCH ] -= self->speed / 2;
                        if( angularDiff[ PITCH ] < -HDEF1_ACCURACYTOLERANCE )
                                self->s.angles[ PITCH ] += self->speed / 4;
                }
        } else if( angularDiff[ PITCH ] > HDEF1_ACCURACYTOLERANCE ) {
                self->s.angles[ PITCH ] -= self->speed;
                angularDiff[ PITCH ] = AngleSubtract( self->s.angles[ PITCH ], angleToTarget[ PITCH ] );
                if( angularDiff[ PITCH ] < -HDEF1_ACCURACYTOLERANCE ) {
                        self->s.angles[ PITCH ] += self->speed / 2;
                        if( angularDiff[ PITCH ] > HDEF1_ACCURACYTOLERANCE )
                                self->s.angles[ PITCH ] -= self->speed / 4;
                }
        } else
                self->s.angles[ PITCH ] = angleToTarget[ PITCH ];

        temp = fabs( self->s.angles[ PITCH ] );
        if( temp > 180 )
                temp -= 360;
        
        if( temp < -HDEF1_VERTICALCAP )
                self->s.angles[ PITCH ] = (-360)+HDEF1_VERTICALCAP;
        //else if( temp > HDEF1_VERTICALCAP )
        //      self->s.angles[ PITCH ] = -HDEF1_VERTICALCAP;
        
        if( angularDiff[ YAW ] < -HDEF1_ACCURACYTOLERANCE ) {
                self->s.angles[ YAW ] += self->speed;
                angularDiff[ YAW ] = AngleSubtract( self->s.angles[ YAW ], angleToTarget[ YAW ] );
                if( angularDiff[ YAW ] > HDEF1_ACCURACYTOLERANCE ){
                        self->s.angles[ YAW ] -= self->speed / 2;
                        angularDiff[ YAW ] = AngleSubtract( self->s.angles[ YAW ], angleToTarget[ YAW ] );
                        if( angularDiff[ YAW ] < HDEF1_ACCURACYTOLERANCE )
                                self->s.angles[ YAW ] += self->speed / 4;
                }
        } else if( angularDiff[ YAW ] > HDEF1_ACCURACYTOLERANCE ) {
                self->s.angles[ YAW ] -= self->speed;
                angularDiff[ YAW ] = AngleSubtract( self->s.angles[ YAW ], angleToTarget[ YAW ] );
                if( angularDiff[ YAW ] < -HDEF1_ACCURACYTOLERANCE ) {
                        self->s.angles[ YAW ] += self->speed / 2;
                        angularDiff[ YAW ] = AngleSubtract( self->s.angles[ YAW ], angleToTarget[ YAW ] );
                        if( angularDiff[ YAW ] > HDEF1_ACCURACYTOLERANCE )
                                self->s.angles[ YAW ] -= self->speed / 4;
                }
        } else
                self->s.angles[ YAW ] = angleToTarget[ YAW ];


        //gi.linkentity ( self );

        if( abs( angleToTarget[ YAW ] - self->s.angles[ YAW ] ) <= HDEF1_ACCURACYTOLERANCE &&
                        abs( angleToTarget[ PITCH ] - self->s.angles[ PITCH ] ) <= HDEF1_ACCURACYTOLERANCE )
                return true;
                
        return false;
}

void turret_think (edict_t *self);
void mgturret_think (edict_t *self);

void turret_refill (edict_t *self)
{
        self->dmg++;
        self->think = turret_think;
        self->nextthink = level.time + .1f;
}

/*void mgturret_refill (edict_t *self)
{
        self->dmg += 5;
        self->think = mgturret_think;
        self->nextthink = level.time + .1;
}*/

void turret_think (edict_t *self)
{
        edict_t *e;

        self->nextthink = level.time + 0.1f;

        //not built
        if (!self->random)
                return;

        // Advance any firing animation before targetting another player
        if (self->s.frame >= FRAME_shot01 && self->s.frame <= FRAME_maxshot)
        {
                self->fly_sound_debounce_time++;
                if (++self->s.frame == FRAME_maxshot)
                {
                        //self->delay = level.time + 2;
                        self->fly_sound_debounce_time = 0;
                        self->s.frame = FRAME_idle;             // Non-firing fully constructed turret
                        return;
                }

                // Fire the projectile at the appropriate frame
                //if (self->s.frame == FRAME_shot02 && self->enemy)
                if (self->enemy && ((turret_mode->value == 3 && self->fly_sound_debounce_time <= 5) || self->s.frame == FRAME_shot02))
                {
                        int mult = 1;
                        vec3_t  f, r, u;
                        vec3_t  start;
                        //int           damage;

                        //int           speed;

                        // Ensure the projectile originates from the barrel's tip
                        AngleVectors (self->s.angles, f, r, u);
                        VectorMA (self->s.origin, self->move_origin[0], f, start);
                        VectorMA (start, self->move_origin[1], r, start);
                        VectorMA (start, self->move_origin[2], u, start);

                        //damage = 80;
                        //speed = 1000;

                        //if (self->enemy->client->resp.team == TEAM_HUMAN)
                        //      self->enemy = NULL;

                        //VectorSubtract (self->s.origin, self->enemy->s.origin, f);
                        //VectorNormalize (f);

                        //screw up the aim slightly if its a hatchie or kami (yuck)
                        if (self->enemy->client->resp.class_type == CLASS_HATCHLING || self->enemy->client->resp.class_type == CLASS_KAMIKAZE)
                        {
                                int i, j;
                                vec3_t  velo;
                                VectorCopy (self->enemy->velocity, velo);
                                j = VectorLength (velo);
                                j /= 100;
                                for (i =0; i <3;i++)
                                        f[i] += (crandom() / (16 - j));
                        }

                        //if (turrettest->value) {
                                //gi.sound(self, CHAN_AUTO, SoundIndex (weapons_railgf1a), 1, ATTN_NORM, 0);
                                //self->delay = level.time + 2;
                                //fire_rail (self, start, f, 100, 50, true);
                                //temp_lightning (self->enemy->s.number, self->s.number, self->enemy->s.origin, self->s.origin);
                        //} else {
                                //self->delay = level.time + .7;

                                //matches 1.1d
                                self->delay = level.time + turret_delay->value;
                                if (self->health == 666)
                                        mult = 5;
                                fire_turret_laser (self, start, f, (int)turret_damage->value * mult, turret_speed->value, 100, 95);
                                //fire_rail (self, start, f, turret_damage->value, 25, true);
                        //}
                }
        }


        //if (self->enemy)
                //VectorSubtract(self->enemy->s.origin,self->s.origin,dist);
        
        if (!self->enemy || self->enemy->health < 1 || !visible(self, self->enemy) || Distance (self->enemy->s.origin, self->s.origin) > MAXTARGETDISTANCE ){
                // Attempt targetting player
                self->enemy = e = NULL;

                while ((e = findradius_c(e, self, MAXTARGETDISTANCE)) != NULL)
                {
                        int x;
                        if (self->health == 666)
                                x = TEAM_HUMAN;
                        else
                                x = TEAM_ALIEN;
                        if (e->client->resp.team == x && visible(self, e))
                        {
                                if (!(e->s.effects & EF_DOUBLE) &&
                                        e->client->resp.class_type == CLASS_HATCHLING &&
                                        (e->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE) &&
                                        sqrt(e->velocity[0]*e->velocity[0] + e->velocity[1]*e->velocity[1]) <= 250)
                                {
                                        if (e->groundentity)
                                                continue;
                                        else
                                        {
                                                trace_t tr;
                                                vec3_t  origin;

                                                VectorCopy (e->s.origin, origin);
                                                origin[2] -= 24;

                                                tr = gi.trace (e->s.origin, e->mins, e->maxs, origin, e, MASK_SOLID);
                                                if (tr.fraction != 1.0)
                                                        continue;
                                        }
                                }
                                self->enemy = e;
                                break;
                        }
                }
        }

        //if (self->enemy)
                //VectorSubtract(self->enemy->s.origin,self->s.origin,dist);

        // If the turret is already targetting continue turning to enemy
        if (self->enemy && self->enemy->health > 0 && Distance (self->enemy->s.origin, self->s.origin) < MAXTARGETDISTANCE )
        {
                if (self->enemy->s.effects & EF_DOUBLE)
                        self->count = 16;
                else
                        self->count = 8;
                if (hdef1_trackenemy(self) && (self->s.frame == FRAME_idle && self->delay < level.time))
                {
                        //self->dmg++;
                        self->s.frame = FRAME_shot01;
                }
        }
}

void mgturret_think (edict_t *self)
{
        edict_t *e;

        //not built
        if (!self->random)
                return;

        // Advance any firing animation before targetting another player
        if (self->s.frame == FRAME_mg_shot01) {
                // Fire the projectile at the appropriate frame
                if (self->s.frame == FRAME_mg_shot01 && self->enemy) {
                        vec3_t  f, r, u;
                        vec3_t  start;
                        int             damage;

                        // Ensure the projectile originates from the barrel's tip
                        AngleVectors (self->s.angles, f, r, u);
                        VectorMA (self->s.origin, self->move_origin[0], f, start);
                        VectorMA (start, self->move_origin[1], r, start);
                        VectorMA (start, self->move_origin[2], u, start);

                        damage = 11;
                        VectorSubtract( self->enemy->s.origin, self->s.origin, f );

                        if (self->enemy->client->resp.class_type == CLASS_HATCHLING || self->enemy->client->resp.class_type == CLASS_KAMIKAZE)
                                f[2] -= 20;
                        VectorNormalize( f );
                        
                        gi.WriteByte (svc_muzzleflash);
                        gi.WriteShort (self-g_edicts);
                        gi.WriteByte (MZ_MACHINEGUN);
                        gi.multicast (self->s.origin, MULTICAST_PVS);

                        self->radius_dmg++;
                        if (self->radius_dmg > 20) {
                                self->radius_dmg = 0;
                                self->nextthink = level.time + 2;
                                return;
                        }

                        fire_lead (self, start, f, 8 + random(), 5, TE_GUNSHOT, 50, 50, MOD_MGTURRET, DAMAGE_PIERCING);
                        //fire_rail (self, start, f, 100, 0);

                        self->delay = level.time + .05f;
                        self->s.frame = FRAME_mg_idle;
                }
        }

        //if (self->enemy)
        //      VectorSubtract(self->enemy->s.origin,self->s.origin,dist);
        
        if (!self->enemy || self->enemy->health < 1 || !visible(self, self->enemy) || Distance (self->enemy->s.origin, self->s.origin) > MAXTARGETDISTANCE )
        {
                // Attempt targetting player
                self->enemy = e = NULL;

                while ((e = findradius_c(e, self, MAXTARGETDISTANCE)) != NULL)
                {
                        if (e->client->resp.team == TEAM_ALIEN && classlist[e->client->resp.class_type].frags_needed < 4 && visible(self, e))
                        {
                                if (e->groundentity &&
                                        (e->client->resp.class_type == CLASS_HATCHLING || e->client->resp.class_type == CLASS_KAMIKAZE) &&
                                        (e->client->glow_time < level.time || (e->client->resp.class_type == CLASS_HATCHLING && (e->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))) &&
                                        sqrt(e->velocity[0]*e->velocity[0] + e->velocity[1]*e->velocity[1]) <= 250)
                                                continue;

                                self->enemy = e;
                                break;
                        }
                }
        }

        //if (self->enemy)
                //VectorSubtract(self->enemy->s.origin,self->s.origin,dist);

        // If the turret is already targetting continue turning to enemy
        if (self->enemy && Distance (self->enemy->s.origin, self->s.origin) < MAXTARGETDISTANCE && self->enemy->health > 0)
        {
                if (hdef1_trackenemy(self) && self->delay < level.time) {
                        self->s.frame = FRAME_mg_shot01;
                        self->nextthink = level.time + 0.1f;
                }
        }

        if (!self->enemy)
                self->nextthink = level.time + 1;
        else
                self->nextthink = level.time + 0.1f;
}

static void turret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        edict_t* base;
        edict_t* gun;

        if (self->deadflag)
                return;

        if (!Q_stricmp(self->classname, "turretbase"))
        {
                base = self;
                gun = self->target_ent;
        }
        else
        {
                gun = self;
                base = self->target_ent;
        }

        if (attacker->client) {
                log_killstructure (attacker, self);
                if (attacker->client->resp.team == TEAM_HUMAN) {
                  if (attacker->client->pers.turrettk < level.time-60)
                    attacker->client->pers.turrettk = level.time-60;
                  else
                    attacker->client->pers.turrettk += 60;
              	  if (gun->newcount > 3 || attacker->client->pers.turrettk >= level.time) //Need to be poked at least 3x to count as tk, or tk too many unbuilt ones
                          attacker->client->resp.total_score -= 2;
                } else
                {
                        //if (self->random && attacker->client->resp.class_type == CLASS_HATCHLING)
                        //      T_Damage (attacker, self, self, vec3_origin, vec3_origin, vec3_origin, 462, 0, 0, MOD_BOMB);
                        if (gun->newcount)
                          attacker->client->resp.total_score += 2;
                }
        }

        gun->deadflag = base->deadflag = DEAD_DEAD;
        gun->takedamage = base->takedamage = DAMAGE_NO;

        if (gun->target)
                G_UseTargets (gun, attacker);

        if (!(gun->spawnflags & 16384))
        {
                int cost;

                if (gun->spawnflags & 64)
                        cost = COST_MGTURRET;
                else
                        cost = COST_TURRET;

                if (attacker->client && attacker->client->resp.team == TEAM_ALIEN)
                        team_info.buildpool[TEAM_HUMAN] += cost*((gun->spawnflags&32768)?team_info.refund_fraction_h:1);
                else
                        team_info.points[TEAM_HUMAN] -= cost;
        }

        if (attacker->client && attacker->client->resp.team == TEAM_HUMAN) {
                attacker->client->resp.teamkills += 16;
                CheckTKs (attacker);
        }

        base->maxs[2] -= gun->maxs[2]-gun->mins[2];
        gi.linkentity(base);

        base->think = BecomeExplosion1;
        base->nextthink = level.time + .5f;

        BecomeExplosion1 (gun);
}

static void turret_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        if (self->deadflag == DEAD_DEAD)
                return;

        if (self->enttype == ENT_TURRET)
        {
                if (!self->enemy && other->client && other->client->resp.team == TEAM_ALIEN && other->health > 0 && Distance (self->s.origin, other->s.origin) < MAXTARGETDISTANCE)
                {
                        self->enemy = other;
                }
        }
        else
        {
                if (self->health < self->target_ent->health)
                        self->target_ent->health = self->health;
                else
                        self->health = self->target_ent->health;
        }
}

static void turret_link(edict_t*base)
{
  edict_t*turret = base->target_ent;

  if (base->s.origin[0] != turret->s.origin[0] || base->s.origin[1] != turret->s.origin[1] || base->s.origin[2] != turret->s.origin[2]) {
    VectorCopy(base->s.origin, turret->s.origin);
    gi.linkentity(turret);
  }
}

static void turret_check_built(edict_t*base)
{
  if (base->health > base->max_health/8 && base->target_ent->newcount < 10)
    base->target_ent->newcount = 10;
}


// Spawn function
void SP_turret (edict_t *self)
{
        edict_t *base;

        if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
                G_FreeEdict(self);
                return;
        }

        //gi.dprintf("Spawning turret at (%f,%f,%f)\n", self->s.origin[0], self->s.origin[1], self->s.origin[2]);

        base = G_Spawn();

        // is map spawned?
        if (!self->enttype) {

                        //FIXME: .ent
                if (!Q_stricmp (level.mapname, "gloom5")) {

                        self->s.origin[2] += 32;
                        base->s.origin[2] += 32;

                } else if (!Q_stricmp (level.mapname, "gloom3a") || !Q_stricmp (level.mapname, "g2se") || !Q_stricmp (level.mapname, "sewer")) {

                        self->s.origin[2] += 16;
                        base->s.origin[2] += 16;
                }

                if (!level.framenum)
                        team_info.maxpoints[TEAM_HUMAN] -= COST_TURRET;

                self->random = base->random = 1;

                if (self->spawnflags & 64)
                        self->s.frame = FRAME_mg_idle;
                else
                        self->s.frame = FRAME_idle;
        } else {
                self->random = 0;
                self->s.frame = FRAME_build01;
        }

        self->flags |= FL_CLIPPING;
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->mass = 99999;
        if (self->spawnflags & 64) {
                self->max_health = self->health = MG_STRENGTH;
                self->speed = 30;
                self->count = 15;
                self->accel = 0;
                self->dmg = 200;
                self->classname = "mgturret";
                self->enttype = ENT_MGTURRET;
                self->s.modelindex = gi.modelindex("models/turret/mgun.md2");
                self->think = mgturret_think;                   // activate
        } else {
                self->think = turret_think;                     // activate

                self->count = 8;
                self->dmg = 25;
                //self->s.sound = SoundIndex (world_force1);
                //self->speed = 12;
                //self->
                self->speed = 16;
                self->max_health = self->health = LG_STRENGTH;
                self->classname = "turret";
                self->enttype = ENT_TURRET;

                //FIXME: is old model included in 1.3 distro?
                self->s.modelindex = gi.modelindex("models/turret/gun.md2");
        }


        if (self->spawnflags & 128) {
                VectorSet (self->mins, -8, -8, 0);
                VectorSet (self->maxs, 8, 8, 24);
        } else {
                VectorSet (self->mins, -16, -16, 0);
                VectorSet (self->maxs, 16, 16, 24);
        }
        //self->s.frame = 0;    // Fully constructed turret
        self->s.skinnum = 0;
        //self->s.frame = FRAME_build01;
        VectorSet (self->move_origin, 32, 0, 0);        // Define the location of barrel muzzle relative to turret origin
        self->takedamage = DAMAGE_AIM;
        self->die = turret_die;
        self->pain = turret_pain;
        
        
        
        self->s.renderfx |= RF_IR_VISIBLE;

        // this hum is annoying -ank
        //self->s.sound = SoundIndex (misc_turrethum);

        base->enttype = ENT_TURRETBASE;
        base->flags |= FL_CLIPPING;
        base->classname = "turretbase";
        base->movetype = MOVETYPE_STEP;
        base->solid = SOLID_BBOX;
        base->mass = 99999;
        base->s.renderfx |= RF_IR_VISIBLE;
        VectorCopy(self->s.origin, base->s.origin);
        base->s.modelindex = gi.modelindex("models/turret/base.md2");
        VectorSet (base->mins, -16, -16, -13);
        VectorSet (base->maxs, 16, 16, 0);
        //VectorSet (base->mins,-24,-24,-13);
        //VectorSet (base->maxs,24,24,0);
        base->s.frame = 0;
        base->takedamage = DAMAGE_AIM;
        base->max_health = base->health = self->health;
        base->die = turret_die;
        base->pain = turret_pain;

        self->s.origin[2]+= 16;

        self->velocity[2] = 100;
        //base->velocity[2] = -150;

        // Link the pieces together

        //r1: changed owner to target_ent. with owner, ents can't touch their owners
        //    so the new code of having the gun literally on top of the base using FL_CLIPPING
        //    would not have worked.
        self->target_ent = base;
        base->target_ent = self;

        if (st.classes)
                self->style = base->style = st.classes;

        if (st.hurtflags)
                self->hurtflags = st.hurtflags;

        self->nextthink = level.time + 1.5f;

        if (!(self->spawnflags & 128))
                gi.linkentity (self);
        gi.linkentity (base);


        self->spawnflags |= 32768;
        
        if (!(self->spawnflags & 256)) {
          base->postthink = turret_link;
          base->maxs[2] += self->maxs[2]-self->mins[2];
          self->solid = SOLID_NOT;
          self->takedamage = DAMAGE_NO;
          self->movetype = MOVETYPE_NONE;
        }

        base->think = turret_check_built;
        base->nextthink = level.time + FRAMETIME;

}


