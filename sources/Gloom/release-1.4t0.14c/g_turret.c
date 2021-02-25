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

// g_turret.c

#include "g_local.h"

void AnglesNormalize(vec3_t vec);

float SnapToEights(float x)
{
        x *= 8.0f;
        if (x > 0.0f)
                x += 0.5f;
        else
                x -= 0.5f;
        return 0.125f * (int)x;
}


void turret_blocked(edict_t *self, edict_t *other)
{
        edict_t *attacker;

        // BEGIN Teamplay
        if (self->owner == other)
                return;
        // END Teamplay
        
        if (other->takedamage)
        {
                if (self->teammaster->owner)
                        attacker = self->teammaster->owner;
                else
                        attacker = self->teammaster;
                T_Damage (other, self, attacker, vec3_origin, other->s.origin, vec3_origin, 40 + random() * 10, 20 + random() * 10, 0, MOD_CRUSH);
        } else if (other->die) {
                other->die (other, self, self, 9999, vec3_origin);
        }
}

/*QUAKED turret_breach (0 0 0) ?
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

"speed"         default 50
"dmg"           default 10
"angle"         point this forward
"target"        point this at an info_notnull at the muzzle tip
"minpitch"      min acceptable pitch angle : default -30
"maxpitch"      max acceptable pitch angle : default 30
"minyaw"        min acceptable yaw angle   : default 0
"maxyaw"        max acceptable yaw angle   : default 360
*/

void turret_breach_fire (edict_t *self)
{
        vec3_t  f, r, u;
        vec3_t  start;
        int             speed;

        AngleVectors (self->s.angles, f, r, u);
        VectorMA (self->s.origin, self->move_origin[0], f, start);
        VectorMA (start, self->move_origin[1], r, start);
        VectorMA (start, self->move_origin[2], u, start);

        speed = (int)self->accel;
        fire_rocket (self->owner, start, f, self->dmg, speed, 240, self->dmg);
        gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
}

void turret_breach_think (edict_t *self)
{
        edict_t *ent;
        vec3_t  current_angles;
        vec3_t  delta;
        // BEGIN teamplay
        qboolean        leaveturret = false; 
        vec3_t          target, forward;
        int                     i;
        // END teamplay

        VectorCopy (self->s.angles, current_angles);
        AnglesNormalize(current_angles);

        AnglesNormalize(self->movedir);
        if (self->movedir[PITCH] > 180)
                self->movedir[PITCH] -= 360;

        // clamp angles to mins & maxs
        if (self->movedir[PITCH] > self->pos1[PITCH])
                self->movedir[PITCH] = self->pos1[PITCH];
        else if (self->movedir[PITCH] < self->pos2[PITCH])
                self->movedir[PITCH] = self->pos2[PITCH];

        if ((self->movedir[YAW] < self->pos1[YAW]) || (self->movedir[YAW] > self->pos2[YAW]))
        {
                float   dmin, dmax;

                dmin = fabs(self->pos1[YAW] - self->movedir[YAW]);
                if (dmin < -180)
                        dmin += 360;
                else if (dmin > 180)
                        dmin -= 360;
                dmax = fabs(self->pos2[YAW] - self->movedir[YAW]);
                if (dmax < -180)
                        dmax += 360;
                else if (dmax > 180)
                        dmax -= 360;
                if (fabs(dmin) < fabs(dmax))
                        self->movedir[YAW] = self->pos1[YAW];
                else
                        self->movedir[YAW] = self->pos2[YAW];
        }

        VectorSubtract (self->movedir, current_angles, delta);
        if (delta[0] < -180)
                delta[0] += 360;
        else if (delta[0] > 180)
                delta[0] -= 360;
        if (delta[1] < -180)
                delta[1] += 360;
        else if (delta[1] > 180)
                delta[1] -= 360;
        delta[2] = 0;

        if (delta[0] > self->speed * FRAMETIME)
                delta[0] = self->speed * FRAMETIME;
        if (delta[0] < -1 * self->speed * FRAMETIME)
                delta[0] = -1 * self->speed * FRAMETIME;
        if (delta[1] > self->speed * FRAMETIME)
                delta[1] = self->speed * FRAMETIME;
        if (delta[1] < -1 * self->speed * FRAMETIME)
                delta[1] = -1 * self->speed * FRAMETIME;

        VectorScale (delta, 1.0/FRAMETIME, self->avelocity);

        for (ent = self->teammaster; ent; ent = ent->teamchain)
                ent->avelocity[1] = self->avelocity[1];

        self->nextthink = level.time + FRAMETIME;

        // BEGIN Teamplay       
        // if we have a driver, adjust his velocities
        if (self->owner)
        {
                if (self->owner->health > 0 && self->owner->client->resp.turret)
                {               
                        // a player is controlling the turret, move towards view angles
                        for (i=0; i<3; i++)
                                self->movedir[i] = self->owner->client->v_angle[i];

                        AnglesNormalize(self->movedir);

                        // FIXME: do a tracebox from up and behind towards the turret, to try and keep them from
                        // getting stuck inside the rotating turret
                        // x & y
                        AngleVectors(self->s.angles, forward, NULL, NULL);
                        VectorScale(forward, 48, forward);
                        VectorSubtract(self->s.origin, forward, target);

                        VectorAdd(target, tv(0,0,12), self->owner->s.origin);
                        VectorClear (self->owner->velocity);

                        self->owner->client->frozenmode = PMF_NO_PREDICTION;

                        //freeze their input, the only way to escape is with +use
                        self->owner->client->frozentime = 1;
                        self->owner->client->ps.viewangles[YAW] = self->s.angles[YAW];
                        self->owner->client->ps.viewangles[PITCH] = self->s.angles[PITCH];
                        
                        gi.linkentity(self->owner);

                        // should the turret shoot now?
                        if ((self->owner->client->buttons & BUTTON_ATTACK) && self->decel < level.time) {
                                turret_breach_fire (self);
                                self->decel = level.time + self->wait;
                        }
                } else {

                        // turret driver got wasted
                        leaveturret = true;
                        self->owner->client->frozentime = 0;
                        self->delay = level.time + 2; //can't be mounted for 2 secs after hopping off
                }

                // has the player abondoned the turret?
                if (leaveturret)
                {
                        // level the gun
                        self->movedir[0] = 0;
                        
                        ent = self->owner;
                        if (ent->inuse && ent->client->resp.team && ent->health > 0) {
                                // throw them back from turret
                                AngleVectors(self->s.angles, forward, NULL, NULL);
                                VectorScale(forward, -300, forward);
                                forward[2] = forward[2] + 150;
                                if (forward[2] < 80)
                                        forward[2] = 80;

                                for (i=0; i<3; i++)
                                        ent->velocity[i] = forward[i];
                                
                                ent->movetype = MOVETYPE_WALK;

                                gi.linkentity (ent);

                                ent->client->frozentime = 0;
                                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                                ent->client->ps.gunindex = gi.modelindex(ent->client->weapon->view_model);
                        }

                        self->owner->client->resp.turret = NULL;
                        self->owner = NULL;
                }
        } 
        else if (level.time > self->delay)
        {       
                // check if a player has mounted the turret

                // find a player
                edict_t *ent;
                vec3_t  target, forward;
                vec3_t  dir;
                int             i;
                float xyspeed;

                ent = &g_edicts[0];
                ent++;

                if (level.framenum % 10 == 0) {
                        for (i=0; i<maxclients->value; i++, ent++)
                        {
                                if (!ent->inuse)
                                        continue;

                                //only 'humans' can use it
                                //lame excuse otherwise their bbox gets clipped and they are crushed :]
                                if (ent->client->resp.class_type != CLASS_GRUNT && ent->client->resp.class_type != CLASS_SHOCK && ent->client->resp.class_type != CLASS_BIO && ent->client->resp.class_type != CLASS_HEAVY && ent->client->resp.class_type != CLASS_COMMANDO)
                                        continue;

                                // determine distance from turret seat location

                                // x & y
                                AngleVectors(self->s.angles, forward, NULL, NULL);
                                VectorScale(forward, 72, forward);
                                VectorSubtract(self->s.origin, forward, target);

                                VectorSubtract (target, ent->s.origin, dir);
                                if (fabs(dir[2]) < 64)
                                        dir[2] = 0;

                                xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

                                if (VectorLength(dir) < 32 && xyspeed < 50)
                                {       
                                        // player has taken control of turret
                                        self->owner = ent;
                                        ent->client->ps.gunindex = 0;
                                        ent->client->resp.turret = self;
                                        ent->movetype = MOVETYPE_NONE;  // don't let them move, or they'll get stuck

                                        if (ent->client->resp.flashlight)
                                        {
                                                ent->client->resp.flashlight->svflags |= SVF_NOCLIENT;
                                                ent->client->resp.flashlight->think = NULL;
                                                ent->client->resp.flashlight->nextthink = 0;
                                                gi.unlinkentity (ent->client->resp.flashlight);
                                        }

                                        // turn off client side prediction for this player
                                        ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

                                        //r1: only one person please!
                                        break;
                                }
                        }
                }
        }
}

void turret_breach_finish_init (edict_t *self)
{
        // get and save info for muzzle location
        if (!self->target)
        {
                gi.dprintf("REMOVED: %s at %s needs a target\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }
        else
        {
                self->target_ent = G_PickTarget (self->target);
                if (!self->target_ent) {
                        gi.dprintf ("REMOVED: %s at %s: Couldn't find target '%s'!!\n", self->classname, vtos2(self), self->target);
                        G_FreeEdict (self);
                        return;
                }
                VectorSubtract (self->target_ent->s.origin, self->s.origin, self->move_origin);
                G_FreeEdict(self->target_ent);
        }

        self->teammaster->dmg = self->dmg;
        self->think = turret_breach_think;
        self->think (self);
}

void SP_turret_breach (edict_t *self)
{
        self->solid = SOLID_BSP;
        self->movetype = MOVETYPE_PUSH;
        gi.setmodel (self, self->model);

        if (!self->speed)
                self->speed = 50;

        if (!self->dmg)
                self->dmg = 500;

        if (!self->wait)
                self->wait = 1;

        if (!self->accel)
                self->accel = 600;

        if (!st.minpitch)
                st.minpitch = -30;
        if (!st.maxpitch)
                st.maxpitch = 30;
        if (!st.maxyaw)
                st.maxyaw = 360;

        self->pos1[PITCH] = -1 * st.minpitch;
        self->pos1[YAW]   = st.minyaw;
        self->pos2[PITCH] = -1 * st.maxpitch;
        self->pos2[YAW]   = st.maxyaw;

        self->ideal_yaw = self->s.angles[YAW];
        self->movedir[YAW] = self->ideal_yaw;

        self->blocked = turret_blocked;

        self->think = turret_breach_finish_init;
        self->nextthink = level.time + FRAMETIME;
        gi.linkentity (self);
}


/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.
*/

void SP_turret_base (edict_t *self)
{
        self->solid = SOLID_BSP;
        self->movetype = MOVETYPE_PUSH;
        gi.setmodel (self, self->model);
        self->blocked = turret_blocked;
        gi.linkentity (self);
}
