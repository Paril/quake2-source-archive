/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mapent.c
  Description: Map entities (trampolines, 
               pressure platforms, etc). Part I

\**********************************************************/

#include "g_local.h"

#define	STOP_EPSILON	0.1

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);

void randomspeaker_think (edict_t *self)
{
   static char outputdir[256] = {0};
   int i = rand() % self->style + 1;
   sprintf(outputdir, "%s%i.wav", self->message, i);
   gi.sound (self, CHAN_VOICE, gi.soundindex(outputdir), self->volume, ATTN_NORM, 0);
   self->nextthink = level.time + self->wait + rand() % (int) self->delay;
}

void SP_target_randomspeaker (edict_t *ent)
{
   ent->classname = "Random Speaker";
   ent->takedamage = DAMAGE_NO;  //AIM
   ent->movetype = MOVETYPE_FLY;
	ent->solid = SOLID_NOT;
   ent->style = (int) ent->style;
   if (!ent->delay) ent->delay = 30;
   if (!ent->volume) ent->volume = 1;
   ent->think = randomspeaker_think;
   ent->nextthink = level.time + ent->wait + rand() % (int) ent->delay;
   gi.linkentity(ent);
}

void SP_func_trampoline(edict_t *ent)
{
   ent->flipping = FLIP_TRAMPOLINE;
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BSP;
	if (!ent->speed)
		ent->speed = 1.5;
	gi.setmodel (ent, ent->model); 
   gi.linkentity(ent);
}


void SP_func_node_wall(edict_t *ent)
{
   ent->flipping = FLIP_NODE_WALL;
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BSP;
   ent->inuse = true;
	gi.setmodel (ent, ent->model); 
   gi.linkentity(ent);
}


void pushrotating_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   float delta, vel; 
   vec3_t v;
   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + .2;
   if (self->spawnflags & 4)
      {
      delta = self->s.origin[1] - other->s.origin[1];
      vel = other->velocity[2];
      }
	else if (self->spawnflags & 8)
      {
      delta = other->s.origin[0] - self->s.origin[0];
      vel = other->velocity[2];
      }
	else 
      {
      delta = other->s.origin[0] - self->s.origin[0];
      vel = other->velocity[1];      
      }
   if (vel < 0)
      vel = -vel;
   if (vel < 10) vel = 10;
   else if (vel > 400) vel = 400;
   if (delta > 80) delta = 80;
   else if (delta < -80) delta = -80;
   delta = delta * vel * other->mass / (self->mass * 5);
   VectorScale (self->movedir, delta, v);
   VectorAdd (self->s.angles, v, self->s.angles);

   if (self->spawnflags & 4)
      {
      if (self->s.angles[2] < self->pos1[2])
          self->s.angles[2] = self->pos1[2];
      else if (self->s.angles[2] > self->pos2[2])
          self->s.angles[2] = self->pos2[2];
      }
   else if (self->spawnflags & 8) 
      {
      if (self->s.angles[0] < self->pos1[0])
          self->s.angles[0] = self->pos1[0];
      else if (self->s.angles[0] > self->pos2[0])
          self->s.angles[0] = self->pos2[0];
      }
   else 
      {
      if (self->s.angles[1] < self->pos1[1])
          self->s.angles[1] = self->pos1[1];
      else if (self->s.angles[1] > self->pos2[1])
          self->s.angles[1] = self->pos2[1];
      }
   gi.linkentity(self);
}


void SP_func_pushrotating(edict_t *ent)
{
   ent->movetype = MOVETYPE_PUSH;
   ent->solid = SOLID_BSP;
   ent->inuse = true;
   ent->touch = pushrotating_touch;

   // set the axis of rotation
	VectorClear(ent->movedir);
   
   if (ent->spawnflags & 4)
      ent->movedir[2] = 1.0;
	else if (ent->spawnflags & 8)
	   ent->movedir[0] = 1.0;
	else 
	   ent->movedir[1] = 1.0;

	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 128)
		ent->s.effects |= EF_ANIM_ALLFAST;

   VectorAdd(ent->s.angles, ent->move_angles, ent->pos2);
   VectorSubtract(ent->s.angles, ent->move_angles, ent->pos1);

   if (!ent->speed)
      ent->speed = 5;
   if (!ent->mass)
      ent->mass = 4000;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}


void boulder_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	ratio;
	vec3_t	v;
   if (self->touch_debounce_time > level.time)
      return;
   ratio = VectorLength(self->velocity) * self->dmg / 100;
   if (ratio > 30 )
      {
      T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, ratio, 100, 0, MOD_CRUSH);
      return;
      }
   if (!other->velocity[0] && !other->velocity[1] && !other->velocity[2])
      return;
   self->touch_debounce_time = level.time + .3;
	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract (self->s.origin, other->s.origin, v);
   VectorNormalize(v);
   v[2] = 0;
   VectorScale(v, ratio * VectorLength (other->velocity), v); //90 * 
   VectorAdd(self->velocity, v, self->velocity);
}

void boulder_blocked  (edict_t *self, edict_t *other)
{
	float	ratio;
	if (!(other->svflags & SVF_MONSTER) && !(other->client))
      return;
   ratio = VectorLength(self->velocity) * self->dmg / 100;
   if (ratio > 30 )
      T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, ratio, 100, 0, MOD_CRUSH);
}

void B_droptofloor (edict_t *ent)
{
	vec3_t		end;
	trace_t		trace;
   int n;

	VectorCopy (ent->s.origin, end);
   end[0] = ent->s.origin[0];
   end[1] = ent->s.origin[1];
	end[2] = ent->s.origin[2] - 90;
	trace = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
   if (trace.plane.normal[2] != 1.0)
      {
      trace.plane.normal[2] = 0;
      VectorScale (trace.plane.normal, ent->mass / 100, end);
      VectorAdd (ent->velocity, end, ent->velocity);
      }
   else
      { 
	   for (n = 0; n < 2; n++)
		   if (ent->velocity[n] > 0)
		      {
			   ent->velocity[n] -= 3;
			   if (ent->velocity[n] < 0)
   				ent->velocity[n] = 0;
		      }
		   else
		      {
			   ent->velocity[n] += 3;
			   if (ent->velocity[n] > 0)
				   ent->velocity[n] = 0;
		      }
      }
   if (ent->velocity[0] || ent->velocity[1] || ent->velocity[2])
      ent->s.sound = gi.soundindex("world/quake.wav");
   else
      ent->s.sound = 0;
   ent->nextthink = level.time + 2 * FRAMETIME;
}

void SP_func_ball(edict_t *ent)
{
	ent->s.modelindex = gi.modelindex("models/objects/boulder/tris.md2");
	ent->movetype = MOVETYPE_ROLL;
	ent->solid = SOLID_BBOX;
   if (ent->style)
      ent->s.renderfx = RF_TRANSLUCENT;
   if (!ent->mass)
      ent->mass = 100;
   if (!ent->dmg)
      ent->dmg = 140;
   ent->blocked = boulder_blocked;
	VectorSet (ent->mins, -38, -38, -38);
	VectorSet (ent->maxs, 38, 38, 38);
   VectorClear(ent->velocity);
	ent->touch = boulder_touch;
	ent->think = B_droptofloor;
	ent->nextthink = level.time + 2 * FRAMETIME;
   ent->clipmask = MASK_SHOT;   
	gi.linkentity (ent);	
}

void shieldwall_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + self->speed;
   T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);
   VectorScale(other->velocity, -4, other->velocity);
}

void shieldwall_use (edict_t *self, edict_t *other, edict_t *activator)
{
   if (self->touch)
      {
      self->touch = NULL;
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
      }
   else
      {
      self->touch = shieldwall_touch;
      if (self->spawnflags & 2)
         self->solid = SOLID_NOT;
      else      
         self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox (self);
      }
	gi.linkentity (self);
}

void SP_func_shieldwall(edict_t *ent)
{
   //ent->movetype = MOVETYPE_PUSH;
   ent->movetype = MOVETYPE_NONE;
   ent->use = shieldwall_use;
   ent->touch = shieldwall_touch;
   if (ent->spawnflags & 2)
      ent->solid = SOLID_NOT;
   else      
      ent->solid = SOLID_BSP;
   if (!(ent->spawnflags & 1))
      ent->use(ent, NULL, NULL);
   if (ent->speed < .1)
      ent->speed = .3;
   gi.setmodel (ent, ent->model);
   gi.linkentity(ent);
}

void bouncewall_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   vec3_t end, offset, forward, right;
   trace_t tr;
   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + .3;
   VectorSet(offset, 90, 0, 0);
   //VectorCopy(self->velocity, forward);
   AngleVectors (other->s.angles, forward, right, NULL);
   //VectorNormalize(forward);
   G_ProjectSource (other->s.origin, offset, forward, right, end);   
   tr = gi.trace (other->s.origin, NULL, NULL, end, other, MASK_SHOT);
   if (tr.fraction < 1.0)
      {
      ClipVelocity (other->velocity, tr.plane.normal, other->velocity, self->speed * 6);
      }     
}

void SP_func_bouncewall(edict_t *ent)
{
   ent->movetype = MOVETYPE_NONE;
   ent->solid = SOLID_BSP;
   ent->touch = bouncewall_touch;
   gi.setmodel (ent, ent->model);
   if (!ent->speed)
      ent->speed = 2;
   gi.linkentity(ent);
}


void oteleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;

	if (other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);

	VectorClear (other->s.angles);

	// kill anything at the destination
	KillBox (other);

	gi.linkentity (other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_oteleporter (edict_t *ent)
{
	edict_t		*trig;

	if (!ent->target)
	{
		gi.dprintf ("object teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	trig = G_Spawn ();
	trig->touch = oteleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy (ent->s.origin, trig->s.origin);
	VectorSet (trig->mins, -16, -16, 0);
	VectorSet (trig->maxs, 16, 16, 24);
	gi.linkentity (ent);
	gi.linkentity (trig);
	
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_oteleporter_dest (edict_t *ent)
{
	ent->solid = SOLID_NOT;
	gi.linkentity (ent);
}

void flamethink(edict_t *ent)
{
   ent->s.frame += ent->style;
   if (ent->s.frame > 11 || ent->s.frame < 0)
		{
		ent->style = -ent->style;
      ent->s.frame += ent->style;
		}
   ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_flame(edict_t *ent)
{
   switch (ent->style)
      {
      case 1:
         ent->s.modelindex = gi.modelindex("models/objects/flame2/tris.md2");
         break;
      case 2:
         ent->s.modelindex = gi.modelindex("models/objects/flame3/tris.md2");
         break;
      default:
         ent->s.modelindex = gi.modelindex("models/objects/flame1/tris.md2");
         break;
      }
	ent->s.renderfx = RF_TRANSLUCENT;
   gi.linkentity (ent);
	ent->s.frame = rand() & 7;
   ent->think = flamethink;
   ent->nextthink = level.time + FRAMETIME;
}

void SP_target_destination(edict_t *ent)
{
	//VectorSet(ent->mins, -8, -8, -8);
	//VectorSet(ent->maxs, 8, 8, 8);
   gi.linkentity (ent);
}

void step_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (!other->client)
      return;

   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + .1;

   if (self->s.origin[2] <= self->style)
      return;

   self->s.origin[2] -= 3;

   if (self->s.origin[2] < self->style)
      self->s.origin[2] = self->style;

   self->nextthink = level.time + 1.2;
   gi.linkentity(self);
}

void step_think(edict_t *ent)
{
   if (ent->s.origin[2] >= ent->gravity)
      return;
   ent->s.origin[2] += 1;
   ent->nextthink = level.time + .1;
   gi.linkentity(ent);
}


void SP_func_step(edict_t *ent)
{
   char x[256] = {0};
   ent->movetype = MOVETYPE_PUSH;
   ent->solid = SOLID_BSP;
   ent->touch = step_touch;
   ent->think = step_think;

   // set the axis of rotation
	VectorClear(ent->movedir);
   
	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 128)
		ent->s.effects |= EF_ANIM_ALLFAST;

   if (!ent->style)
      ent->style = 64;
   ent->gravity = ent->s.origin[2];
   ent->style = ent->s.origin[2] - ent->style;
   
   if (!ent->mass)
      ent->mass = 100;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

}
