/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_sentry.c
  Description: Sentry gun (Unfinished) 

\**********************************************************/

#include "g_local.h"
#include "beholder.h"
void Sentry_f(edict_t *self);
void sentry_install(edict_t *self);
void sentry_think(edict_t *self);
void sentry_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void sentry_pain (edict_t *self, edict_t *other, float kick, int damage);
void sentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void sentry_attack(edict_t *self);

#define SENTRY_BOUND_MIN_X -16
#define SENTRY_BOUND_MIN_Y -16
#define SENTRY_BOUND_MIN_Z 0
#define SENTRY_BOUND_MAX_X 16
#define SENTRY_BOUND_MAX_Y 16
#define SENTRY_BOUND_MAX_Z 16

#define CMOD_SENTRY           255

void sentry_think (edict_t *self)
{
   edict_t *nearby_unit = NULL;
   vec3_t pointer, 
          oldangles;

   // If somebody attacks sentry, then sentry will retaliate regardless of whom
   // it was attacking first. This is here in case an idiot teamate makes the 
   // move. If the original attacker runs away, then go ahead and attack any 
   // enemy around. When original attacker comes back, attack him first.
   
   // If attacker is not around for 10 seconds... Then become normal again.
   if (self->oldenemy)
      if (self->powerarmor_time < level.time)
         {
         self->oldenemy = NULL;
         self->s.renderfx &= ~RF_SHELL_RED;
         gi.sound(self, CHAN_BODY, gi.soundindex("world/deactivated.wav"), 1, ATTN_STATIC, 0); 
         }

   // If enemy is still around, then attack if visible:
   if (self->oldenemy != NULL)
      {
      self->s.renderfx |= RF_SHELL_RED;
      if (self->oldenemy->health > 0)
         {
         VectorSubtract (self->oldenemy->s.origin, self->s.origin, pointer);
         vectoangles(pointer, pointer);
         VectorCopy(pointer, self->s.angles);
         if (visible(self, self->oldenemy))
            {
            self->powerarmor_time = level.time + 10;
            self->enemy = self->oldenemy;
            sentry_attack(self);
            self->nextthink = level.time + .1;
            self->s.angles[0] = 0;
            return;
            }
         // Don't cool off if attacker is not visible. Stay on alert.
         self->s.angles[0] = 0;
         }    
      else  // Teamate/enemy destroyed, Play cool off sound.
         {
         self->oldenemy = NULL;
         self->s.renderfx &= ~RF_SHELL_RED;
         gi.sound(self, CHAN_ITEM, gi.soundindex("medic/medatck3.wav"), 1, ATTN_NORM, 0);
         }
      }

   // If already engaged. Keep on attacking, don't look for another enemy if
   // current enemy is still visible and alive.
   VectorCopy(self->s.angles, oldangles);
   if (self->enemy)
      {
      if (self->enemy->health > 0)
         {
         VectorSubtract (self->enemy->s.origin, self->s.origin, pointer);
         vectoangles(pointer, pointer);
         VectorCopy(pointer, self->s.angles);         
         if (visible(self, self->enemy))
            {
            sentry_attack(self);
            self->nextthink = level.time + .1;
            self->s.angles[0] = 0;
            return;
            }
         else  // Enemy ran away (whimp). Play cool off sound.
            {
            self->enemy = NULL;
            gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);       
            }
         self->s.angles[0] = 0;
         }    
      else  // Enemy destroyed, Play cool off sound.
         {
         self->enemy = NULL;
         gi.sound(self, CHAN_ITEM, gi.soundindex("tank/death.wav"), 1, ATTN_NORM, 0);       
         }
      self->nextthink = level.time + 1;
      return;
      }   

         
   // Search and attack visible enemies: If enemy is found then attack
   // every 10th of a second.
   while ((nearby_unit = findradius(nearby_unit, self->s.origin, 600)) != NULL)
      {
      if (!nearby_unit->client && !(nearby_unit->svflags & SVF_MONSTER))
         continue;
      if (nearby_unit->team_data == self->team_data)
         continue;
      if (nearby_unit == self)
         continue;
      if (!nearby_unit->takedamage)
         continue;
      if (nearby_unit->health <= 0)
         continue;
      VectorSubtract (nearby_unit->s.origin, self->s.origin, pointer);
      vectoangles(pointer, pointer);
      VectorCopy(pointer, self->s.angles);
      if (!visible(self, nearby_unit))
         continue;
        // Attack enemy, play heat up sound: 
        gi.sound(self, CHAN_ITEM, gi.soundindex("tank/pain.wav"), 1, ATTN_NORM, 0);       
        self->enemy = nearby_unit;
        sentry_attack(self);
        self->nextthink = level.time + .1;
        self->s.angles[0] = 0;
        return;
      }
   self->s.angles[0] = 0;
   VectorCopy(oldangles, self->s.angles);
   
   self->enemy = NULL;
   self->nextthink = level.time + 3;
}

void sentry_attack(edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
   gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/machinegun/gun1.wav"), 1, ATTN_NORM, 0);
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_ACTOR_MACHINEGUN_1], forward, right, start);
   VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;
	start[2] += self->viewheight;
	VectorSubtract (target, start, forward);
	VectorNormalize (forward);
   fire_bullet (self, start, forward, self->dmg, 4, CMOD_SENTRY);
}

void sentry_use(edict_t *self, edict_t *other, edict_t *activator)
{
   if (self->think)
      {
      gi.sound(self, CHAN_BODY, gi.soundindex("world/deactivated.wav"), 1, ATTN_STATIC, 0); 
      self->think = NULL;
      self->nextthink = 0;
      }
   else
      {
      self->think = sentry_think;
      self->nextthink = level.time + 3;
      }
}


void SP_str_sentry(edict_t *sentry)
{
   edict_t *tripod;   
   tripod = G_Spawn();
   VectorCopy(sentry->s.origin, tripod->s.origin);
   tripod->model = "models/world/tripod/tris.md2";
   tripod->s.modelindex = gi.modelindex (tripod->model);
   tripod->takedamage = DAMAGE_NO;
	tripod->solid = SOLID_BBOX;
   tripod->movetype = MOVETYPE_STEP;
   tripod->flags &= ~FL_NO_KNOCKBACK;
   VectorSet (tripod->mins, -32, -32, 0);
	VectorSet (tripod->maxs, 32, 32, 32);

   gi.unlinkentity(sentry);
   sentry->delay = sentry->speed + level.time;
   sentry->takedamage = DAMAGE_NO;
	sentry->solid = SOLID_BBOX;
   sentry->movetype = MOVETYPE_STEP;
   sentry->flags &= ~FL_NO_KNOCKBACK;
   sentry->model = "models/world/sentry/tris.md2";
   sentry->s.modelindex = gi.modelindex (sentry->model);
   
   sentry->s.sound = gi.soundindex("world/amb3.wav");

   VectorSet (sentry->mins, SENTRY_BOUND_MIN_X, 
             SENTRY_BOUND_MIN_Y, SENTRY_BOUND_MIN_Z);
	VectorSet (sentry->maxs, SENTRY_BOUND_MAX_X, 
             SENTRY_BOUND_MAX_Y, SENTRY_BOUND_MAX_Z);

   sentry->viewheight = 22;
   sentry->enemy = NULL;
   sentry->oldenemy = NULL;
   sentry->use = sentry_use;
   if (!sentry->dmg)
      sentry->dmg = 10;
   if (sentry->spawnflags & 1)
      sentry->use(sentry, NULL, NULL);
   gi.linkentity(tripod);
   gi.linkentity(sentry);

}

