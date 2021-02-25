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

/*------------------------------------------------------------------------*/
/* GRAPPLE                                                                                                                                */
/*------------------------------------------------------------------------*/

//FIXME: use alienweaponthink for updating the target grapple position
// clientbeginserverframe for pulling as usual
// enable pull only when hit something solid enough
// remove unnecessary complexity
// make pull only add certain amount of velocity, not replace it

#include "g_local.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t /*@out@*/result);

#if 1 // insta-fire toggle, 0 = on
// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
        gclient_t *cl;

        if (!self->owner || !self->owner->inuse) {
                G_FreeEdict(self);
                return;
        }

        cl = self->owner->client;
        if (cl) {
                cl->ctf_grapple = NULL;
                cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        }
        G_FreeEdict(self);
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

        if (other == self->owner)
                return;

        if (self->spawnflags < CTF_GRAPPLE_STATE_FLY)
                return;

        if (surf && (surf->flags & SURF_SKY))
        {
                CTFResetGrapple(self);
                return;
        }

        if (other->client)
                return;

/*              CTFResetGrapple(self);
                return;
        }*/
        //VectorCopy(vec3_origin, self->velocity);

//      self->nextthink = level.time + 60.0;
        self->think=NULL;
        self->nextthink = 0;

        self->velocity[0] = 0;
        self->velocity[1] = 0;
        self->velocity[2] = 0;

//      PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

        //if (other->client) {
                //T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_UNKNOWN);
        //      CTFResetGrapple(self);
        //      return;
        //}


        self->spawnflags = CTF_GRAPPLE_STATE_PULL; // we're on hook
        self->touch_debounce_time = level.time;
        self->enemy = other;

        self->solid = SOLID_NOT;

/*      if (self->owner->client->silencer_shots)
                volume = 0.2;

        //gi.sound (self->owner, CHAN_RELIABLE+CHAN_AUTO, SoundIndex (weapons_shotweb1), volume, ATTN_NORM, 0);
        //gi.sound (self, CHAN_AUTO, SoundIndex (weapons_shotweb1), volume, ATTN_NORM, 0);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BLOOD);
        gi.WritePosition (self->s.origin);
        if (!plane)
                gi.WriteDir (vec3_origin);
        else
                gi.WriteDir (plane->normal);
        gi.multicast (self->s.origin, MULTICAST_PVS);
*/
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
        vec3_t  offset, start, end, f, r;
//      vec3_t  dir;
        float   distance;

        AngleVectors (self->owner->client->v_angle, f, r, NULL);
        VectorSet(offset, 16, 16, self->owner->viewheight-8);
        P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

        //VectorSubtract(start, self->owner->s.origin, offset);

        //VectorSubtract (start, self->s.origin, dir);
        //distance = VectorLength(dir);

        distance = Distance (start, self->s.origin);

        // don't draw cable if close
        if (distance < 128)
                return;

#if 0
        if (distance > 256)
                return;

        // check for min/max pitch
        vectoangles (dir, angles);
        if (angles[0] < -180)
                angles[0] += 360;
        if (fabs(angles[0]) > 45)
                return;

        trace_t tr; //!!

        tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr.ent != self) {
                CTFResetGrapple(self);
                return;
        }
#endif

        // adjust start for beam origin being in middle of a segment
        VectorMA (start, 8, f, start);

        VectorCopy (self->s.origin, end);
        // adjust end z for end spot since the monster is currently dead
//      end[2] = self->absmin[2] + self->size[2] / 2;

        gi.WriteByte (svc_temp_entity);
#if 0 //def USE_GRAPPLE_CABLE
        gi.WriteByte (TE_GRAPPLE_CABLE);
        gi.WriteShort (self->owner - g_edicts);
        gi.WritePosition (self->owner->s.origin);
        gi.WritePosition (end);
        gi.WritePosition (offset);
#else
        gi.WriteByte (TE_MEDIC_CABLE_ATTACK);  // TE_MEDIC_CABLE_ATTACK
        gi.WriteShort (self - g_edicts);
        gi.WritePosition (end);
        gi.WritePosition (start);
#endif

        //if (self->owner->client->resp.class_type == CLASS_HATCHLING && (self->owner->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))
                gi.unicast (self->owner, false);
        //else
        //      gi.multicast (start, MULTICAST_PVS);
}

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
        vec3_t hookdir, v;
        float vlen;

        if (self->enemy) {
                if (self->enemy->solid == SOLID_NOT) {
                        CTFResetGrapple(self);
                        return;
                }
                if (self->enemy->solid == SOLID_BBOX) {
                        VectorScale(self->enemy->size, 0.5, v);
                        VectorAdd(v, self->enemy->s.origin, v);
                        VectorAdd(v, self->enemy->mins, self->s.origin);
                        gi.linkentity (self);
                } else
                        VectorCopy(self->enemy->velocity, self->velocity);
        }

        if (!self->owner || !self->owner->inuse) {
                CTFResetGrapple (self);
                return;
        }

        CTFGrappleDrawCable(self);

        if (self->spawnflags > CTF_GRAPPLE_STATE_FLY) {
                // pull player toward grapple
                // this causes icky stuff with prediction, we need to extend
                // the prediction layer to include two new fields in the player
                // move stuff: a point and a velocity.  The client should add
                // that velociy in the direction of the point
                //vec3_t forward, up;

                //AngleVectors (self->owner->client->v_angle, forward, NULL, up);

                VectorCopy(self->owner->s.origin, v);
                v[2] += self->owner->viewheight;
                VectorSubtract (self->s.origin, v, hookdir);

                vlen = VectorLength(hookdir);


                if (0 && self->spawnflags == CTF_GRAPPLE_STATE_PULL && vlen < 64) {
                        self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
                        self->spawnflags = CTF_GRAPPLE_STATE_HANG;
                        VectorScale (self->owner->velocity , 0.01, self->owner->velocity);
                } else /*if (self->spawnflags == CTF_GRAPPLE_STATE_PULL)*/ {
                  VectorNormalize (hookdir);
/*
                  float v = fabs(self->s.origin[2]-self->owner->s.origin[2])-15;
                  if (v > 10) v= 10;
                  if (v < 1) {
                    v = 1;
//                  self->owner->velocity[2]*=0.01f;
//                  self->s.origin[2] = self->owner->s.origin[2];
                    hookdir[2] = 0;
                  }*/
                  float speed = vlen*10;
                  if (speed > CTF_GRAPPLE_PULL_SPEED) speed = CTF_GRAPPLE_PULL_SPEED;
//                gi.dprintf("%.3f %.5f\n", speed, v);
                  VectorScale(hookdir, speed, hookdir);
                  
                  VectorCopy(hookdir, self->owner->velocity);

                        // I think gravity is always added to player -ank
                        //SV_AddGravity(self->owner);

                        if (vlen < 32)  //HAXORED NON MOVEMENT SLIPPY SLIDE GRAPPE (in conjunction w/clienthink)
                                VectorScale (self->owner->velocity , 0.01, self->owner->velocity);

                }
        }
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
        edict_t *grapple;
        trace_t tr;

        VectorNormalize (dir);

        if (self->client->ctf_grapple)
                G_FreeEdict(self->client->ctf_grapple);

        grapple = G_Spawn();
        VectorCopy (start, grapple->s.origin);
//      VectorCopy (start, grapple->s.old_origin);
        vectoangles (dir, grapple->s.angles);
        VectorScale (dir, speed, grapple->velocity);
        //VectorAdd(grapple->s.origin, grapple->velocity, grapple->s.origin);
        //VectorCopy (grapple->s.origin, grapple->s.old_origin);

        grapple->s.event = EV_OTHER_TELEPORT; // lerp fix

        grapple->movetype = MOVETYPE_FLYMISSILE;
        grapple->clipmask = MASK_SHOT & ~CONTENTS_MONSTER;
        //grapple->clipmask = MASK_SOLID;

        grapple->solid = SOLID_BBOX;
        grapple->classname = "grapple";
        grapple->svflags |= SVF_NOCLIENT;
        //grapple->s.effects |= effect;

        grapple->owner = self;
        grapple->touch = CTFGrappleTouch;
        grapple->nextthink = level.time + 2.0f;
        grapple->think = CTFResetGrapple;
        grapple->dmg = damage;
        self->client->ctf_grapple = grapple;
        grapple->spawnflags = CTF_GRAPPLE_STATE_FLY;
        gi.linkentity (grapple);

        tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
                grapple->touch (grapple, tr.ent, NULL, NULL);   
        }
}       
#else
/*** insta-fire grapple code follows -ank */
void CTFResetGrapple(edict_t *self)
{
        //r1: this got called from ClientDisconnect, self->owner was NULL. why... who knows.
        //FIXME: maybe this should be called with the client ent instead of the grapple ent.
        if (self->owner)
                self->owner->client->ctf_grapple = NULL;

        G_FreeEdict(self);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
        vec3_t  offset, start, end, f, r;
        vec3_t  dir;
        float   distance;

        AngleVectors (self->owner->client->v_angle, f, r, NULL);
        VectorSet(offset, 16, 16, self->owner->viewheight-8);
        P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

        //VectorSubtract(start, self->owner->s.origin, offset);

        //VectorSubtract (start, self->s.origin, dir);
        //distance = VectorLength(dir);
        distance = Distance (start, self->s.origin);

        // don't draw cable if close
        if (distance < 128)
                return;

#if 0
        if (distance > 256)
                return;

        // check for min/max pitch
        vectoangles (dir, angles);
        if (angles[0] < -180)
                angles[0] += 360;
        if (fabs(angles[0]) > 45)
                return;

        trace_t tr; //!!

        tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr.ent != self) {
                CTFResetGrapple(self);
                return;
        }
#endif

        // adjust start for beam origin being in middle of a segment
        VectorMA (start, 8, f, start);

        VectorCopy (self->s.origin, end);
        // adjust end z for end spot since the monster is currently dead
//      end[2] = self->absmin[2] + self->size[2] / 2;

        gi.WriteByte (svc_temp_entity);
#if 0 //def USE_GRAPPLE_CABLE
        gi.WriteByte (TE_GRAPPLE_CABLE);
        gi.WriteShort (self->owner - g_edicts);
        gi.WritePosition (self->owner->s.origin);
        gi.WritePosition (end);
        gi.WritePosition (offset);
#else
        gi.WriteByte (TE_MEDIC_CABLE_ATTACK);  // TE_MEDIC_CABLE_ATTACK
        gi.WriteShort (self - g_edicts);
        gi.WritePosition (end);
        gi.WritePosition (start);
#endif
        gi.unicast (self->owner, false);
}

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
        vec3_t hookdir, v;
        float vlen;


        if(self->owner->wait)
                return;

        if (self->enemy) {
                // if bbox ent went away
                if (!self->enemy->inuse || self->enemy->solid != SOLID_BBOX) {
                        CTFResetGrapple(self);
                        return;
                }
                // update pos if we hit something moving (world is SOLID_BSP, others are ents)
                VectorScale(self->enemy->size, 0.5, v);
                VectorAdd(v, self->enemy->s.origin, v);
                VectorAdd(v, self->enemy->mins, self->s.origin);
                //gi.linkentity (self);
        }

        if (!self->owner || !self->owner->inuse) {
                CTFResetGrapple (self);
                return;
        }

        if (self->spawnflags < CTF_GRAPPLE_STATE_HANG)
                CTFGrappleDrawCable(self);

        VectorCopy(self->owner->s.origin, v);
        v[2] += self->owner->viewheight;
        VectorSubtract (self->s.origin, v, hookdir);
        vlen = VectorLength(hookdir);

        // FIXME: make proper stop check, eg. copy owner bbox to grapple, check for touch
        if (vlen > VectorLength (self->owner->size)) // 32
                vlen = 450; //CTF_GRAPPLE_PULL_SPEED;
        else {
                vlen = 0;
                self->spawnflags = CTF_GRAPPLE_STATE_HANG;
        }

        VectorNormalize (hookdir);
        VectorScale(hookdir, vlen, hookdir);

        VectorCopy(hookdir, self->owner->velocity);
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
        edict_t *grapple;
        trace_t tr;
        vec3_t end;

        VectorNormalize (dir);

        if (self->client->ctf_grapple)
                G_FreeEdict(self->client->ctf_grapple);

        VectorMA (start, speed, dir, end);

        tr = gi.trace (start, NULL, NULL, end, NULL, MASK_SHOT);

        //gi.dprintf("surf hit %s, flags %d val %d allsolid %d\n", tr.surface->name, tr.surface->flags, tr.surface->value, (int)tr.allsolid);

        if (tr.fraction == 1.0)
                return;

/* prevents client grappling
        if (tr.ent->client) {
                return;
        }
*/
        if (tr.surface->flags & SURF_SKY)
                return;

        grapple = G_Spawn();
        VectorCopy (tr.endpos, grapple->s.origin);

        grapple->classname = "grapple";

        grapple->owner = self;

        self->client->ctf_grapple = grapple;
        grapple->spawnflags = CTF_GRAPPLE_STATE_PULL;

        if (tr.ent->solid == SOLID_BBOX)
                grapple->enemy = tr.ent; // for directional updates

//      grapple->svflags = SVF_NOCLIENT;
//      gi.linkentity (grapple);
}       
#endif

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage)
{
        vec3_t  forward, right;
        vec3_t  start;
        vec3_t  offset;
//      float volume = 1.0;

        AngleVectors (ent->client->v_angle, forward, right, NULL);
//      VectorSet(offset, 24, 16, ent->viewheight-8+2);
        VectorSet(offset, 24, 4, ent->viewheight-8+2);
        VectorAdd (offset, g_offset, offset);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        VectorScale (forward, -2, ent->client->kick_origin);
        ent->client->kick_angles[0] = -1;

//      if (ent->client->silencer_shots)
                //volume = 0.2;

        //gi.sound (ent, CHAN_RELIABLE+CHAN_AUTO, SoundIndex (weapons_grapple_grfire), volume, ATTN_NORM, 0);
        
        CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED);

        //PlayerNoise(ent, start, PNOISE_WEAPON);
}
