/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_bot.c
  Description: AI algorithms for generic monsters.

\**********************************************************/

#include "g_local.h"

void BOT_EnemyChaseDir (edict_t *actor, vec3_t dest, float dist);
void find_node_hide(edict_t *ent);
void find_node_hide2(edict_t *ent);
qboolean M_CheckBottom (edict_t *self);
void SV_FixCheckBottom (edict_t *self);
qboolean ai_checkattack (edict_t *self, float dist);
qboolean FindTarget (edict_t *self);
map_node_t *nearest_node(edict_t *ent);
map_node_t *hide_node(edict_t *ent, edict_t *ent2);
qboolean SV_CloseEnough (edict_t *ent, edict_t *ent2, float dist);
trace_t SV_PushEntity (edict_t *ent, vec3_t move);
void TakeCover(edict_t *ent, float dist);
qboolean Vec_CloseEnough (edict_t *ent, vec3_t end, float dist);
void GoAmbush(edict_t *ent, edict_t *enemy, float dist);
void draw_line(vec3_t start, vec3_t end, float draw_time);
int HideFromGrenades(edict_t *ent);
void scrBOT_run (edict_t *self, float dist);
void draw_vertex(vec3_t start);
void draw_line(vec3_t start, vec3_t end, float draw_time);

/*
======================
BOT_MoveToGoal
- First function called by the bot when walking.
The call sequence is the following:
   
     BOT_MoveToGoal (edict_t *ent, float dist)
     BOT_CloseEnough BOT_CloseEnough (ent, goal->origin, dist)
     BOT_StepDirection (edict_t *ent, float yaw, float dist)
         BOT_movestep (edict_t *ent, vec3_t move, qboolean relink)
     BOT_NewChaseDir (edict_t *actor, map_node_t *node, float dist)

======================
*/
void BOT_MoveToGoal (edict_t *ent, float dist)
{
	map_node_t	*goal;
   vec3_t forward;
   
   // Check if we have a destination
	goal = ent->monsterinfo.goalnode;

   // Check if bot is falling
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return;

   // If the next step hits the ambush goal, return immediately
	if ((ent->status & STATUS_MONSTER_AMBUSH) && ent->monsterinfo.goalambush 
      && BOT_CloseEnough (ent, ent->monsterinfo.goalambush->origin, dist))
      {
      ent->monsterinfo.covers = 0;
      ent->monsterinfo.goalambush = NULL;
      ent->status &= ~STATUS_MONSTER_AMBUSH;
		/*
      if (visible(ent->enemy, ent))
         {
         GoAmbush(ent, ent->enemy, dist);
         return;
         }
		*/
      //ent->monsterinfo.goalambush = NULL;
      VectorSubtract (ent->enemy->s.origin, ent->s.origin, forward);
	   ent->s.angles[YAW] = vectoyaw(forward);
      ent->monsterinfo.reload(ent);
      return;
      }

   // If the next step hits the enemy, return immediately
	if (ent->enemy && SV_CloseEnough (ent, ent->enemy, dist) )
      {
		ent->monsterinfo.attack(ent);
		return;
      }
	else 
   if (goal && BOT_CloseEnough (ent, goal->origin, dist))
      {
      // If bot is hiding
      if (ent->monsterinfo.shots <= 0)
         {        
         if (visible(ent->enemy, ent))
            {
            GoAmbush(ent, ent->enemy, dist);
            }
         else
            {
            VectorSubtract (ent->enemy->s.origin, ent->s.origin, forward);
	         ent->s.angles[YAW] = vectoyaw(forward);
            ent->monsterinfo.reload(ent);
            }
         }
      else if (ent->status & STATUS_MONSTER_HIDE)
         {
         // If bot is not visible to enemy, then stop running like an idiot,
         // you are in safe place
         if (!ent->monsterinfo.chaser)
            {
            ent->monsterinfo.stand(ent);
            return;
            }
         else if (!visible(ent, ent->monsterinfo.chaser))
            {
				if (!(ent->type & TYPE_SLAVE_ACTOR))
					{
					ent->status &= ~STATUS_MONSTER_HIDE;
					ent->enemy = ent->monsterinfo.chaser;
					//ent->enemy = NULL;
					}
				ent->monsterinfo.chaser = NULL;
            ent->monsterinfo.stand(ent);
            return;
            }
         else
         // Otherwise, keep looking for safe spot
            find_node_hide2(ent);
         }
      else 
      // If not hiding, then go to next node in route
         find_node_target(ent);

       //If bot has no route, just sit there
		if (!(ent->status & STATUS_MONSTER_NO_GOAL))
			if (!ent->monsterinfo.goalnode)
				{				
				ent->monsterinfo.stand(ent);
				return;
				}
		ent->status &= ~STATUS_MONSTER_NO_GOAL;
      BOT_NewChaseDir (ent, ent->monsterinfo.goalnode, dist);
      return;
      }

   // If bot cannot make the step
	if (/*!(rand() & 3) || */!BOT_StepDirection (ent, ent->ideal_yaw, dist))
	   {
      ent->monsterinfo.trail_time = level.time + 1.5;
		if (ent->inuse)
         BOT_NewChaseDir (ent, goal, dist);
      return;
	   }

	// Chose a new direction every 2.5 to 5.5 seconds
	if (level.time > ent->monsterinfo.trail_time)
	   {
      ent->monsterinfo.trail_time = level.time + .5 + (rand() & 1);
		if (ent->inuse)
         {
         // If goal node is not visible, then chose another one.
         if (goal && !visible_vector(ent->s.origin, goal->origin))
            {
            ent->monsterinfo.goalnode = NULL;
            ent->monsterinfo.lastnode = NULL;
            goal = NULL;
            }
         BOT_NewChaseDir (ent, goal, dist);
         }
	   }
}

/*
======================
BOT_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean BOT_StepDirection (edict_t *ent, float yaw, float dist)
{
	vec3_t	move, oldorigin;
	float		delta;
	
	ent->ideal_yaw = yaw;

	// Set monster yaw to same as moving yaw
	M_ChangeYaw (ent);
	
	yaw = yaw * M_PI / 180; 

   // Topaz Fix:
	move[0] = cos(yaw) * dist;
	move[1] = sin(yaw) * dist;
	move[2] = 0;

	VectorCopy (ent->s.origin, oldorigin);
	if (BOT_movestep (ent, move, false))
	   {
		delta = ent->s.angles[YAW] - ent->ideal_yaw;
		if (delta > 45 && delta < 315)
		   {		// not turned far enough, so don't take the step
			VectorCopy (oldorigin, ent->s.origin);
		   }
		gi.linkentity (ent);
		G_TouchTriggers (ent);
		return true;
	   }
	gi.linkentity (ent);
	G_TouchTriggers (ent);
	return false;
}

/*
=============
BOT_movestep
Called by bots.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/

qboolean BOT_movestep (edict_t *ent, vec3_t move, qboolean relink)
{
	//float		dz;
	vec3_t	oldorg, neworg, end/*, side*/;
	trace_t	trace;
	//int		i;
	//float		stepsize;
	vec3_t	test;
	int		contents;

// try the move	
	VectorCopy (ent->s.origin, oldorg);
	VectorAdd (ent->s.origin, move, neworg);
	VectorCopy (neworg, end);
// flying monsters don't step up
/*
	if ( ent->flags & (FL_SWIM | FL_FLY) )
	   {
	   // try one move with vertical motion, then one without
		for (i = 0; i < 2; i++)
		   {
			VectorAdd (ent->s.origin, move, neworg);
		   if (i == 0)
		      {
            if (ent->status & STATUS_MONSTER_AMBUSH)
			      dz = ent->s.origin[2] - ent->monsterinfo.goalvector[2];
            else if (ent->monsterinfo.goalnode)
   				dz = ent->s.origin[2] - ent->monsterinfo.goalnode->origin[2];
            else if (ent->enemy)
				   dz = ent->s.origin[2] - ent->enemy->s.origin[2];
			   if (dz > 8)
  					neworg[2] -= 8;
			   else if (dz > 0)
  					neworg[2] -= dz;
			   else if (dz < -8)
  					neworg[2] += 8;
			   else
  					neworg[2] += dz;
		      }
			trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID);
	
			// fly monsters don't enter water voluntarily
			if (ent->flags & FL_FLY)
			   {
				if (!ent->waterlevel)
				   {
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (contents & MASK_WATER)
						return false;
				   }
			   }
			// swim monsters don't exit water voluntarily
			if (ent->flags & FL_SWIM)
			   {
				if (ent->waterlevel < 2)
				   {
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
						return false;
				   }
			   }
  
			if (trace.fraction == 1)
			   {
				VectorCopy (trace.endpos, ent->s.origin);
				if (relink)
				   {
					gi.linkentity (ent);
					G_TouchTriggers (ent);
				   }
				return true;
			   }
			
			if (!ent->enemy)
				break;
		   }		
		return false;
	   }
*/
// push down from a step height above the wished position
	/*
	if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
	   stepsize = STEPSIZE;
	else
		stepsize = 1;
	*/
	//stepsize = 18;
	//gi.dprintf ("%s -> %f\n", ent->classname, stepsize);

	neworg[2] += 18;
	end[2] -= 66;

	//draw_line(, , 1);	
	//draw_vertex(neworg);
	//draw_vertex(end);

	trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
	//trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_SOLID);

	if (trace.allsolid)
      {
		if (trace.ent->takedamage)
			{
			if (trace.ent == ent->enemy)
				{
				if (ent->scr.index == -1 && ent->monsterinfo.attack)
					ent->monsterinfo.attack(ent);
				return false;
				}
		   if (ent->blocked)
			   {
	         if (ent->scr.on_block >= 0 && ent->scr.block_time < level.time)
					ent->blocked(ent, trace.ent);			
				}
			if (trace.ent->solid != SOLID_BSP && !trace.ent->client)
				if (!trace.ent->enemy && !trace.ent->monsterinfo.goalnode)
					{
					if (trace.ent->enemy == ent)
						VectorScale(move, 3, move);
					SV_PushEntity (trace.ent, move);
					gi.linkentity(trace.ent);
					}
			}
      return false;
      }  

	if (trace.startsolid)
	   {
		neworg[2] -= 18; //stepsize
		trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	   }


	// don't go in to water
	if (ent->waterlevel == 0)
	   {
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2] + 1;	
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER)
			{
			ent->status |= STATUS_MONSTER_ENEMYVIEW;
			ent->monsterinfo.nextview = level.time + 8;
			return false;
			}
	   }

	if (trace.fraction == 1)
	   {
	   // if monster had the ground pulled out, go ahead and fall
		if (ent->flags & FL_PARTIALGROUND )
		   {
			VectorAdd (ent->s.origin, move, ent->s.origin);
			if (relink)
			   {
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			   }
			ent->groundentity = NULL;
			return true;
		   }
		return false;		// walked off an edge
	   }

// check point traces down for dangling corners
	//draw_vertex(trace.endpos);
	//gi.dprintf ("%f -> %f\n", neworg[2], trace.endpos[2]);
	if (trace.endpos[2] == ent->s.origin[2])
		{			
		trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, trace.endpos, ent, MASK_MONSTERSOLID);
		if (trace.fraction < 1.0)
			return false;
		}	
	else 
		{
		trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, trace.endpos, ent, CONTENTS_MONSTER);
		if (trace.fraction < 1.0)
			return false;
		}

	VectorCopy (trace.endpos, ent->s.origin);

	if (!M_CheckBottom (ent))
	   {
		if ( ent->flags & FL_PARTIALGROUND )
		   {	// entity had floor mostly pulled out from underneath it
			   // and is trying to correct
			if (relink)
			   {
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			   }
			return true;
		   }
		VectorCopy (oldorg, ent->s.origin);
		ent->status |= STATUS_MONSTER_ENEMYVIEW;
		ent->monsterinfo.nextview = level.time + 8;
		return false;
	   }

	if (ent->flags & FL_PARTIALGROUND )
	   {
		ent->flags &= ~FL_PARTIALGROUND;
	   }
	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	   {
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	   }
	return true;
}


/*
================
SV_NewChaseDir

  Check for new direction for the bot.
  If monster is hiding, the run for nearest hiding node.
  Else If the bot sees the enemy, the bot will get to the enemy.
  Else If the bot has a enemy, then take route to enemy.
    If there is no route to enemy, then just walk around.
  Else Just walk around.
================
*/

void BOT_NewChaseDir (edict_t *actor, map_node_t *node, float dist)
{
	float	deltax, deltay;
	float	d[3], s[3];
   float	tdir, olddir, turnaround;
   //	vec3_t	move;
   // Run for a hiding spot.
   if ((actor->status & STATUS_MONSTER_AMBUSH) && actor->monsterinfo.goalambush)
      {
      BOT_EnemyChaseDir (actor, actor->monsterinfo.goalambush->origin, dist);
      return;
      }
   if (actor->status & STATUS_MONSTER_HIDE)
      {
      if (!actor->monsterinfo.goalnode)
         { 
         find_node_hide(actor);
         node = actor->monsterinfo.goalnode;
         }
      }   
   // ---> Ambush monsters don't follow enemy. They keep a distance.
   else if (!(actor->type & TYPE_MONSTER_AMBUSH) 
      && actor->enemy && !(actor->status & STATUS_MONSTER_ENEMYVIEW)
		&& visible(actor, actor->enemy))
      {
      actor->monsterinfo.goalnode = NULL;
      BOT_EnemyChaseDir (actor, actor->enemy->s.origin, dist);
      return;
      }
   if (!node)
      {
      find_node_target(actor);
      node = actor->monsterinfo.goalnode;
      if (!node)
         {
         //actor->enemy = NULL;
         actor->monsterinfo.stand(actor);
         return;
         }
      }
   actor->monsterinfo.goalnode = node;

	olddir = anglemod((int) (actor->ideal_yaw / 45) * 45);
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
	//if (1/(d[1] != DI_NODIR && d[2] != DI_NODIR)
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


qboolean BOT_CloseEnough (edict_t *ent, vec3_t end, float dist)
{
	int		i;
   // Do not check for Z coordinates, so node can be up in the air.
	for (i = 0; i < 2; i++)
	   {
		if (end[i] > ent->absmax[i] + dist)
			return false;
		if (end[i] < ent->absmin[i] - dist)
			return false;
	   }
	return true;
}





/*****************\
BOT_EnemyChaseDir 
\*****************/
void BOT_EnemyChaseDir (edict_t *actor, vec3_t dest, float dist)
{
	float	deltax,deltay;
	float	d[3], s[3];
	float	tdir, olddir, turnaround;

	//olddir = anglemod(/*(int) (actor->ideal_yaw / 45) * 45*/);
   olddir = anglemod((int) actor->ideal_yaw);
	turnaround = anglemod(olddir - 180);

	deltax = dest[0] - actor->s.origin[0];
	deltay = dest[1] - actor->s.origin[1];
	if (deltax > 10)
		d[1]= 0;
	else if (deltax < -10)
		d[1]= 180;
	else
		d[1]= DI_NODIR;
	if (deltay < -10)
		d[2]= 270;
	else if (deltay > 10)
		d[2]= 90;
	else
		d[2]= DI_NODIR;

// try direct route
	//if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	   {
	   VectorSubtract (dest, actor->s.origin, s);
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

// try other directions
	if ( ((rand()&3) & 1) ||  abs(deltay)>abs(deltax))
	   {
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	   }

	if (d[1] != DI_NODIR && d[1] != turnaround 
       && BOT_StepDirection(actor, d[1], dist))
			return;

	if (d[2] != DI_NODIR && d[2] != turnaround	
      && BOT_StepDirection(actor, d[2], dist))
			return;

/* there is no direct path to the player, so pick another direction */

	if (olddir != DI_NODIR && BOT_StepDirection(actor, olddir, dist))
		return;

	if (rand()&1) 	/*randomly determine direction of search*/
	   {
		for (tdir = 0; tdir <= 315; tdir += 45)
			if (tdir != turnaround && BOT_StepDirection(actor, tdir, dist))
					return;
	   }
	else
	   {
		for (tdir = 315 ; tdir >= 0; tdir -= 45)
			if (tdir != turnaround && BOT_StepDirection(actor, tdir, dist) )
					return;
	   }

	if (turnaround != DI_NODIR && BOT_StepDirection(actor, turnaround, dist))
			return;

	actor->ideal_yaw = olddir;		// can't move

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

	if (!M_CheckBottom (actor))
		SV_FixCheckBottom (actor);
}


qboolean BOT_walkmove (edict_t *ent, float yaw, float dist)
{
	vec3_t	move;
	
	if (!ent->groundentity /*&& !(ent->flags & (FL_FLY|FL_SWIM))*/)
		return false;

	yaw = yaw * M_PI / 180;	
	move[0] = cos(yaw) * dist;
	move[1] = sin(yaw) * dist;
	move[2] = 0;
	return BOT_movestep(ent, move, true);
}

