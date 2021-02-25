/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_scrbot.c
  Description: Scripted robot AI

\**********************************************************/

#include "g_local.h"

/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/
void scrBOT_MoveToGoal (edict_t *ent, float dist);
void scrBOT_NewChaseDir (edict_t *actor, map_node_t *node, float dist);
void BOT_EnemyChaseDir (edict_t *actor, vec3_t dest, float dist);
qboolean SV_CloseEnough (edict_t *ent, edict_t *ent2, float dist);
void SV_FixCheckBottom (edict_t *ent);
qboolean reacheable_vector(vec3_t self, vec3_t other);

void scrBOT_run (edict_t *self, float dist)
{
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }
   if (self->volume < level.time)
      {
		self->volume = level.time + 30;
      find_node_target(self);
		if (!self->monsterinfo.goalnode)
			{
         self->monsterinfo.stand(self);
         if (self->scr.nextcmd < 0)
				self->scr.nextcmd = 0;
			return;
         }
      else
			scrBOT_NewChaseDir (self, self->monsterinfo.goalnode, dist);
      }

   if ((self->status & STATUS_SLAVE_ATTACK) 
		&& self->monsterinfo.nextattack < level.time)
      if (visible(self, self->enemy))
         {
         self->monsterinfo.attack(self);
         return;
         }

	scrBOT_MoveToGoal (self, dist);
}

void scrBOT_stand (edict_t *self, float dist)
{
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }      
	if (!(self->spawnflags & 1) && (self->monsterinfo.idle) && (level.time > self->monsterinfo.idle_time))
	   {
		if (self->monsterinfo.idle_time)
		   {
			self->monsterinfo.idle (self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		   }
		else
		   {
			self->monsterinfo.idle_time = level.time + random() * 15;
		   }
	   }
}



void scrBOT_NewChaseDir (edict_t *actor, map_node_t *node, float dist)
{
	float	deltax, deltay;
	float	d[3], s[3];
   float	tdir, olddir, turnaround;
   //	vec3_t	move;
   // Run for a hiding spot.
   if (actor->enemy && visible(actor, actor->enemy))/*&& reacheable_vector(actor->s.origin, actor->enemy->s.origin)*/
      {
      BOT_EnemyChaseDir (actor, actor->enemy->s.origin, dist);
      return;
      }
   if (!node)
      {
      find_node_target(actor);
      node = actor->monsterinfo.goalnode;
      if (!node)
         {
         actor->enemy = NULL;
         actor->monsterinfo.stand(actor);
         return;
         }
      }
   actor->monsterinfo.goalnode = node;

	//olddir = anglemod((int) (actor->ideal_yaw / 45) * 45);
	olddir = anglemod((int) actor->ideal_yaw);
	turnaround = anglemod(olddir - 180);

   deltax = node->origin[0] - actor->s.origin[0];
	deltay = node->origin[1] - actor->s.origin[1];

	if (deltax > 10)
		d[1] = 0;
	else if (deltax < -10)
		d[1] = 180;
	else
		d[1] = DI_NODIR;
	if (deltay < -10)
		d[2] = 270;
	else if (deltay > 10)
		d[2] = 90;
	else
		d[2] = DI_NODIR;


   // Try direct route
	//if (!(rand() & 3))
	   {
	   VectorSubtract (node->origin, actor->s.origin, s);
	   VectorNormalize (s);   
	   vectoangles (s, s);   
      if (s[1] != turnaround && BOT_StepDirection(actor, s[1], dist))
			return;
	   }   

   if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	   {
		if (d[1] == 0)
			tdir = d[2] == 90 ? 45 : 315;
		else
			tdir = d[2] == 90 ? 135 : 215;
			
		if (tdir != turnaround && BOT_StepDirection(actor, tdir, dist))
			return;
	   }
   // Try other directions
	if (((rand() & 3) & 1) ||  abs(deltay) > abs(deltax))
	   {
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	   }

	if (d[1] != DI_NODIR && d[1] != turnaround && 
       BOT_StepDirection(actor, d[1], dist))
			return;

	if (d[2] != DI_NODIR && d[2] != turnaround &&
	    BOT_StepDirection(actor, d[2], dist))
			return;


	if (olddir!=DI_NODIR && BOT_StepDirection(actor, olddir, dist))
			return;

	if (rand() & 1) 	
	   {
		for (tdir = 0; tdir <= 315; tdir += 45)
			if (tdir != turnaround && BOT_StepDirection(actor, tdir, dist))
					return;
	   }
	else
	   {
		for (tdir = 315; tdir >= 0; tdir -= 45)
			if (tdir != turnaround && BOT_StepDirection(actor, tdir, dist))
					return;
	   }

	if (turnaround != DI_NODIR && BOT_StepDirection(actor, turnaround, dist))
			return;

   // Can't move.
	actor->ideal_yaw = olddir;

   if (!M_CheckBottom (actor))
		SV_FixCheckBottom (actor);
}


void scrBOT_MoveToGoal (edict_t *ent, float dist)
{
	map_node_t	*goal;
   
   // Check if we have a destination
	goal = ent->monsterinfo.goalnode;

   // Check if bot is falling
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return;

   // If the next step hits the enemy, return immediately
	if (ent->enemy &&  SV_CloseEnough (ent, ent->enemy, dist) )
      {
      // Scripted monsters return control to activator.
      if (ent->scr.index == -1)
         if (ent->activator)
            {
            ent->activator->scr.nextcmd = 0;
            ent->monsterinfo.stand(ent);
            ent->enemy = NULL;
            ent->monsterinfo.goalnode = NULL;
            ent->activator = NULL;
            return;
            }
      if (ent->scr.nextcmd < 0)
         {
         ent->scr.nextcmd = 0;	
         ent->enemy = NULL;
         ent->monsterinfo.goalnode = NULL;
			ent->monsterinfo.stand(ent);
         }
      //ent->monsterinfo.stand(ent);
      return;
      }
   else if (goal && BOT_CloseEnough (ent, goal->origin, dist))
      {
      // If bot is hiding
      // If not hiding, then go to next node in route
      find_node_target(ent);
      // If bot has no route, just sit there
		if (!(ent->status & STATUS_MONSTER_NO_GOAL))
			if (!ent->monsterinfo.goalnode)
				{			
				gi.dprintf ("No route\n");
				ent->monsterinfo.stand(ent);
				return;
				}
		ent->status &= ~STATUS_MONSTER_NO_GOAL;
      scrBOT_NewChaseDir (ent, ent->monsterinfo.goalnode, dist);
      return;
      }

   // If bot cannot make the step
   //  
	if (/*!(rand() & 3 ) || */!BOT_StepDirection (ent, ent->ideal_yaw, dist))
	   {
      ent->monsterinfo.trail_time = level.time + 1.5;
		if (ent->inuse)
         scrBOT_NewChaseDir (ent, goal, dist);
      return;
	   }

	if (level.time > ent->monsterinfo.trail_time)
	   {
      ent->monsterinfo.trail_time = level.time + .5 + (rand() & 1);
		if (ent->inuse)
         {
         // If goal node is not visible, then chose another one.
         /*
         if (goal && !visible_vector(ent->s.origin, goal->origin))
            {
            ent->monsterinfo.goalnode = NULL;
            ent->monsterinfo.lastnode = NULL;
            goal = NULL;
            }
         */
         scrBOT_NewChaseDir (ent, goal, dist);
         }
	   }
}
