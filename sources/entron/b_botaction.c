/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_botaction.c
  Description: AI algorithms for scripted monsters.

\**********************************************************/

#include "g_local.h"
void GoAmbush(edict_t *ent, edict_t *enemy, float dist);
void BOT_tellteam (edict_t *self);
edict_t *nearest_foe (edict_t *self);
edict_t *closest_foe (edict_t *self);

/*
=============
BOT is running:
=============
*/
void BOT_run (edict_t *self, float dist)
{
   // FIXME: Don't run if you are dead: In case somethig weird happens.
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }

	// Script run if bot has a script:
   if (self->scr.index > -1 || self->activator)
      {
      scrBOT_run(self, dist);
      return;
      }

	// * Stop attacking if enemy is dead
   // * Medic will chose a friend as an enemy to heal it
	// * Prevent bots from attacking friends
   if (self->enemy)
      {
      if (self->enemy->health <= 0)
			{
         self->enemy = self->oldenemy;
			if (self->enemy->health <= 0)
				self->enemy = NULL;
			}
      else if (self->type & TYPE_MONSTER_TEAMMATE) 
         {
         if (self->enemy->team_data != self->team_data)
            self->enemy = NULL;
         }
      else if (self->team_data && self->enemy->team_data == self->team_data)
         self->enemy = NULL;
		if (self->monsterinfo.nextview < level.time)
			{
			self->status &= ~STATUS_MONSTER_ENEMYVIEW;
			//gi.dprintf ("Clean\n");
			}
      }

   // While monster is not hiding:
   // The whole code below (AI) is based on enemy not hiding.
	// If bot is hiding, then just run away.
   if (self->status & STATUS_MONSTER_HIDE)
		{
		// Communicate enemy status to nearby friends:
		if (self->status & STATUS_MONSTER_COMMUNICATE)
			if (self->monsterinfo.search_time < level.time)
				{         
				self->monsterinfo.search_time = level.time + 1 + (rand() & 3);
				BOT_tellteam (self);
				}
		BOT_MoveToGoal (self, dist);
		return;
		}

	// No enemy is around -> Stand
	if (!self->enemy) 
		{
		self->monsterinfo.stand(self);
		return;
		}

   // Communicate enemy status to nearby friends:
   if (self->status & STATUS_MONSTER_COMMUNICATE)
      if (self->monsterinfo.search_time < level.time)
         {         
         self->monsterinfo.search_time = level.time + 1 + (rand() & 3);
         BOT_tellteam (self);
         }
     
   // * Attack if enemy is around 
	// * Ambush bots check for grenades
	// * Ambush bots check for hide spot if possible
   if (self->monsterinfo.nextattack < level.time)
		{
      self->monsterinfo.nextattack = level.time + 1 + random();
      // Avoid grenades.
		if (self->type & TYPE_MONSTER_AMBUSH && !(self->status & STATUS_MONSTER_AMBUSH))
			{
			self->monsterinfo.nextattack = level.time + .3;
			//HideFromGrenades(self);
			}
		if (visible(self, self->enemy) /*&& infront(self, self->enemy)*/)
			{
			if (self->monsterinfo.shots > 0 || (range(self, self->enemy) == RANGE_MELEE))
				self->monsterinfo.attack(self);
			return;
			}
      // If monster can't see the hide spot anymore, find another hide spot.
      if (self->status & STATUS_MONSTER_AMBUSH && self->monsterinfo.goalambush)
			if (!visible_vector(self->s.origin, self->monsterinfo.goalambush->origin))
				GoAmbush(self, self->enemy, dist);
      }

   // If failed to hide after 3 times, reload in place, don't try to hide 
   if (self->monsterinfo.nextcover && self->monsterinfo.nextcover < level.time)
		{
      if (self->monsterinfo.covers > 3)
			{
         vec3_t forward;
         VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
         self->s.angles[YAW] = vectoyaw(forward);
         self->monsterinfo.reload(self);
         }
      else
			{
         GoAmbush(self, self->enemy, dist);
         self->monsterinfo.covers++;
         self->monsterinfo.nextcover = level.time + 1 + random();
         }         
      return;
      }  
	// Just run away if monster if ambushing
   else if (self->status & STATUS_MONSTER_AMBUSH)
		{
		BOT_MoveToGoal (self, dist);
		return;
		}
   // Go hide if reload is needed:
	else if (self->monsterinfo.shots <= 0)
		{
      if (visible(self->enemy, self))
         {
         GoAmbush(self, self->enemy, dist);
         self->monsterinfo.covers++;
         self->monsterinfo.nextcover = level.time + 1 + random();
         }
      else
			{
         vec3_t forward;
         VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	      self->s.angles[YAW] = vectoyaw(forward);
         self->monsterinfo.reload(self);
         }  
      return;
      }
	// Find new node every 30 seconds:
	if (self->volume < level.time) //!self->monsterinfo.goalnode || 
		{      
		self->volume = level.time + 30;
      find_node_target(self);
		if (!self->monsterinfo.goalnode)
			{
         if (!visible(self, self->enemy))
				{
            self->monsterinfo.stand(self);
            return;
            }
         }
      else
			BOT_NewChaseDir (self, self->monsterinfo.goalnode, dist);
      }

   BOT_MoveToGoal (self, dist);
}


/*
=============
BOT is walking:
=============
*/
void BOT_walk (edict_t *self, float dist)
{

   // FIXME: Don't walk if you are dead: In case somethig weird happens.
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }      

	// Script run if bot has a script:
   if (self->scr.index > -1 || self->activator)
      {
      scrBOT_run(self, dist);
      return;
      }

	// Find new node every 30 seconds:
   if (!self->monsterinfo.goalnode || self->volume < level.time)
      {      
		self->volume = level.time + 30;
      find_node_target(self);
		if (!self->monsterinfo.goalnode)
			{
         self->monsterinfo.stand(self);
			return;
         }
      else
			BOT_NewChaseDir (self, self->monsterinfo.goalnode, dist);
      }

	// Search for enemy if bot has no enemy.
   if (self->monsterinfo.search_time < level.time)
      {         
      if (!self->enemy)
         self->enemy = nearest_foe(self);
      if (self->enemy)
         self->monsterinfo.run(self);
      self->monsterinfo.search_time = level.time + .6; // + 1 + (rand() & 3)
      }
   BOT_MoveToGoal (self, dist);
}

/*
=============
BOT is standing:
=============
*/
void BOT_stand (edict_t *self, float dist)
{

   // FIXME: Don't stand if you are dead: In case somethig weird happens.
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }      

	// Search for enemy if bot has no enemy.
   if (self->monsterinfo.search_time < level.time)
      {         
      if (!self->enemy)
         self->enemy = nearest_foe(self);
		else
			{
         if (visible(self, self->enemy))
            {
            self->monsterinfo.attack(self);
            return;
            }
         if ((self->status & STATUS_MONSTER_NOSHOOT) 
            && self->monsterinfo.nextambush < level.time)
            {
            self->status &= ~STATUS_MONSTER_NOSHOOT;
            self->monsterinfo.run(self);
            return;
            }
			}
      self->monsterinfo.search_time = level.time + .6 + random();
		}

	// Play idle anim if any:
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

/*
=============
BOT is stationary
==============
*/
void BOT_close (edict_t *self, float dist)
{
   // FIXME: Don't stand if you are dead: In case somethig weird happens.
   if (self->health <= 0)
      {
      self->monsterinfo.currentmove = NULL;
      return;
      }      
	// Search for closest enemy;
   if (self->monsterinfo.search_time < level.time)
      {         
      if (!self->enemy)
         self->enemy = closest_foe(self);
      self->monsterinfo.search_time = level.time + 1 + (rand() & 3);
      }
}


/*
=============
BOT is moving:
==============
*/
void BOT_move (edict_t *self, float dist)
{
	BOT_walkmove (self, self->s.angles[YAW], dist);
}

/*
=============
BOT is strafing left:
==============
*/
void BOT_move_left (edict_t *self, float dist)
{
	BOT_walkmove (self, self->s.angles[YAW] + 90, dist);
}

/*
=============
BOT is strafing right:
==============
*/
void BOT_move_right (edict_t *self, float dist)
{
	BOT_walkmove (self, self->s.angles[YAW] - 90, dist);
}


/*
=============
BOT is attacking:
==============
*/
void BOT_charge (edict_t *self, float dist)
{
	vec3_t	v;
   if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		{
		self->enemy = NULL;
		self->monsterinfo.stand(self);
      return;
		}
	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);

	if (dist)
		BOT_walkmove (self, self->s.angles[YAW], dist);
}