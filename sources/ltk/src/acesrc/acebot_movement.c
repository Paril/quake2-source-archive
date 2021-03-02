///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LicenseToKill/src/acesrc/acebot_movement.c 27    22/10/99 7:13 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/acebot_movement.c $
 * 
 * 27    22/10/99 7:13 Riever
 * New ladder detection function enables creation of ladder nodes.
 * Distance checks added for ladder climbing.
 * Specfial handling to reduce velocity at ladder mount point.
 * Ledge handling code allows jumping up small obstacles.
 * Air Walking codce found and removed.
 * Ladder climbing code refined and working.
 * 
 * 26    20/10/99 20:33 Riever
 * Removed a velocity hack
 * 
 * 25    16/10/99 18:10 Riever
 * Messing with wander code - marked up.
 * 
 * 24    16/10/99 17:03 Riever
 * Added jump to shoot when line of sight is blocked.
 * 
 * 23    16/10/99 13:11 Riever
 * Changed special move code to get jumps working better.
 * 
 * 22    10/10/99 9:34 Riever
 * Anglechange before random move added
 * Changed special move code.
 * 
 * 21    6/10/99 20:46 Riever
 * Taught bots to aim round teammates and not fire at walls.
 * 
 * 20    6/10/99 17:40 Riever
 * Added TeamPlay state STATE_POSITION to enable bots to seperate and
 * avoid centipede formations.
 * 
 * 19    1/10/99 18:19 Riever
 * Removed a debug print
 * 
 * 18    30/09/99 8:03 Riever
 * Put a distance limit on where crouching starts in the random combat
 * code.
 * 
 * 17    27/09/99 14:28 Riever
 * Added (UNUSED) code for AntPathMove as a reference for anyone who feels
 * like experimenting with it in another bot.
 * 
 * 16    21/09/99 22:35 Riever
 * Comments...
 * 
 * 15    21/09/99 21:29 Riever
 * Changed checkeyes focalpoint to 64
 * 
 * 14    21/09/99 21:25 Riever
 * Steve was messing with the drown time so I took that bit out.
 * 
 * 13    21/09/99 20:19 Riever
 * Set teamPauseTime in attack routine.
 * 
 * 12    21/09/99 20:14 Riever
 * Reduced backwards movement on ledge avoidance.
 * 
 * 11    21/09/99 12:21 Riever
 * Ledge checking code inserted in movement functions to try to prevent
 * sky diving...
 * 
 * 10    21/09/99 11:18 Riever
 * Changed logic checking in ledge code to avoid jumping off roofs.
 * 
 * 9     18/09/99 19:20 Riever
 * NODE_DOOR special handling added. I sweated blood over this so there's
 * better not be any complaints!
 * 
 * 8     18/09/99 9:51 Riever
 * Reduced ledge check to 100 (was 400)
 * Added some DroneBot code
 * 
 * 7     14/09/99 14:37 Riever
 * ForwardMove is now forced if only weapon available is the knife
 * 
 * 6     14/09/99 14:15 Riever
 * Made check for WEAPON_READY before firing
 * 
 * 5     14/09/99 14:09 Riever
 * Modified bot stupidity to always be off target a little if not
 * ltk_skill == 10
 * 
 * 4     14/09/99 8:35 Riever
 * Updated comments - tested  - works
 * 
 * 3     14/09/99 8:13 Riever
 * Now using ltk_skill cvar to control bot accuracy
 * 
 * 2     13/09/99 19:52 Riever
 * Added headers
 *
 */
	
///////////////////////////////////////////////////////////////////////
//  acebot_movement.c - This file contains all of the 
//                      movement routines for the ACE bot
//           
///////////////////////////////////////////////////////////////////////

#include "g_local.h"
#include "acebot.h"

qboolean ACEAI_CheckShot(edict_t *self);
void ACEMV_ChangeBotAngle (edict_t *ent);
qboolean	ACEND_LadderForward( edict_t *self );

//=============================================================
// CanMoveSafely (dronebot)
//=============================================================
// Checks for lava and slime
#define	MASK_DEADLY				(CONTENTS_LAVA|CONTENTS_SLIME)
#define TRACE_DIST_SHORT 48
#define TRACE_DOWN 96
#define VEC_ORIGIN tv(0,0,0)
//

qboolean	CanMoveSafely(edict_t	*self, vec3_t angles)
{
	vec3_t	dir, angle, dest1, dest2;
	trace_t	trace;
	float	this_dist;

//	self->bot_ai.next_safety_time = level.time + EYES_FREQ;

	VectorClear(angle);
	angle[1] = angles[1];

	AngleVectors(angle, dir, NULL, NULL);
	
	// create a position in front of the bot
	VectorMA(self->s.origin, TRACE_DIST_SHORT, dir, dest1);

	// Modified to check  for crawl direction
	//trace = gi.trace(self->s.origin, mins, self->maxs, dest, self, MASK_SOLID);
	//TempLaser (self->s.origin, dest1);
	trace = gi.trace(self->s.origin, tv(-16,-16,0), tv(16,16,0), dest1, self, MASK_PLAYERSOLID);

	// Check if we are looking inside a wall!
	if (trace.startsolid)
		return (true);

	if (trace.fraction > 0)
	{	// check that destination is onground, or not above lava/slime
		dest1[0] = trace.endpos[0];
		dest1[1] = trace.endpos[1];
		dest1[2] = trace.endpos[2] - 28;
		this_dist = trace.fraction * TRACE_DIST_SHORT;

		if (gi.pointcontents(dest1) & MASK_PLAYERSOLID)
			return (true);


		// create a position a distance below it
		VectorCopy( trace.endpos, dest2);
		dest2[2] -= TRACE_DOWN;
		//TempLaser (trace.endpos, dest2);
		trace = gi.trace(trace.endpos, VEC_ORIGIN, VEC_ORIGIN, dest2, self, MASK_PLAYERSOLID | MASK_DEADLY);

		if( (trace.fraction == 1.0) // long drop!
			|| (trace.contents & MASK_DEADLY) )	// avoid SLIME or LAVA
		{
			return (false);
		}
		else
		{
			return (true);
		}
	}
	//gi.bprintf(PRINT_HIGH,"Default failure from LAVACHECK\n");
	return (true);
}
///////////////////////////////////////////////////////////////////////
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_CanMove(edict_t *self, int direction)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);
	
	if(direction == MOVE_LEFT)
		angles[1] += 90;
	else if(direction == MOVE_RIGHT)
		angles[1] -= 90;
	else if(direction == MOVE_BACK)
		angles[1] -=180;

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 36, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
		
	VectorSet(offset, 36, 0, -100); // RiEvEr reduced drop distance
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	//AQ2 ADDED MASK_SOLID
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID|MASK_OPAQUE);
	
	if(tr.fraction == 1.0 || tr.contents & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		if(debug_mode)
			debug_printf("%s: move blocked\n",self->client->pers.netname);
		return false;	
	}
	
	return true; // yup, can move
}

///////////////////////////////////////////////////////////////////////
// Handle special cases of crouch/jump
//
// If the move is resolved here, this function returns
// true.
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_SpecialMove(edict_t *self, usercmd_t *ucmd)
{
	vec3_t dir,forward,right,start,end,offset;
	vec3_t top;
	trace_t tr; 
	
	// Get current direction
	VectorCopy(self->client->ps.viewangles,dir);
	dir[YAW] = self->s.angles[YAW];
	AngleVectors (dir, forward, right, NULL);

	VectorSet(offset, 0, 0, 0); // changed from 18,0,0
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	offset[0] += 18;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	// trace it
	start[2] += 18; // so they are not jumping all the time
	end[2] += 18;
	tr = gi.trace (start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);
		
//RiEvEr	if(tr.allsolid)
	if( tr.fraction < 1.0)
	{
		// Check for crouching
		start[2] -= 14;
		end[2] -= 14;

		// Set up for crouching check
		VectorCopy(self->maxs,top);
		top[2] = 0.0; // crouching height
		tr = gi.trace (start, self->mins, top, end, self, MASK_PLAYERSOLID);
		
		// Crouch
//RiEvEr		if(!tr.allsolid) 
		if( tr.fraction == 1.0 )
		{
			ucmd->forwardmove = 400; 
			ucmd->upmove = -400; 
			return true;
		}
		
		// Check for jump
		start[2] += 32;
		end[2] += 32;
		tr = gi.trace (start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

//RiEvEr		if(!tr.allsolid)
		if( tr.fraction == 1.0)
		{	
			ucmd->forwardmove = 400;
			ucmd->upmove = 400;
			return true;
		}
	}
	
	return false; // We did not resolve a move here
}


///////////////////////////////////////////////////////////////////////
// Checks for obstructions in front of bot
//
// This is a function I created origianlly for ACE that
// tries to help steer the bot around obstructions.
//
// If the move is resolved here, this function returns true.
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_CheckEyes(edict_t *self, usercmd_t *ucmd)
{
	vec3_t  forward, right;
	vec3_t  leftstart, rightstart,focalpoint;
	vec3_t  /* upstart,*/upend;
	vec3_t  dir,offset;

	trace_t traceRight,traceLeft,/*traceUp,*/ traceFront; // for eyesight

	// Get current angle and set up "eyes"
	VectorCopy(self->s.angles,dir);
	AngleVectors (dir, forward, right, NULL);
	
	// Let them move to targets by walls
	if(!self->movetarget)
//		VectorSet(offset,200,0,4); // focalpoint 
		VectorSet(offset,64,0,4); // focalpoint 
	else
		VectorSet(offset,36,0,4); // focalpoint 
	
	G_ProjectSource (self->s.origin, offset, forward, right, focalpoint);

	// Check from self to focalpoint
	// Ladder code
	VectorSet(offset,36,0,0); // set as high as possible
	G_ProjectSource (self->s.origin, offset, forward, right, upend);
	//AQ2 ADDED MASK_SOLID
	traceFront = gi.trace(self->s.origin, self->mins, self->maxs, upend, self, MASK_SOLID|MASK_OPAQUE);
		
	if(traceFront.contents & 0x8000000) // using detail brush here cuz sometimes it does not pick up ladders...??
	{
		ucmd->upmove = 400;
		ucmd->forwardmove = 400;
		return true;
	}
	
	// If this check fails we need to continue on with more detailed checks
	if(traceFront.fraction == 1)
	{	
		ucmd->forwardmove = 400;
		return true;
	}

	VectorSet(offset, 0, 18, 4);
	G_ProjectSource (self->s.origin, offset, forward, right, leftstart);
	
	offset[1] -= 36; // want to make sure this is correct
	//VectorSet(offset, 0, -18, 4);
	G_ProjectSource (self->s.origin, offset, forward, right, rightstart);

	traceRight = gi.trace(rightstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);
	traceLeft = gi.trace(leftstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);

	// Wall checking code, this will degenerate progressivly so the least cost 
	// check will be done first.
		
	// If open space move ok
	if(traceRight.fraction != 1 || traceLeft.fraction != 1 || strcmp(traceLeft.ent->classname,"func_door")!=0)
	{
//@@ weird code...
/*		// Special uppoint logic to check for slopes/stairs/jumping etc.
		VectorSet(offset, 0, 18, 24);
		G_ProjectSource (self->s.origin, offset, forward, right, upstart);

		VectorSet(offset,0,0,200); // scan for height above head
		G_ProjectSource (self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);
			
		VectorSet(offset,200,0,200*traceUp.fraction-5); // set as high as possible
		G_ProjectSource (self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);

		// If the upper trace is not open, we need to turn.
		if(traceUp.fraction != 1)*/
		{						
			if(traceRight.fraction > traceLeft.fraction)
				self->s.angles[YAW] += (1.0 - traceLeft.fraction) * 45.0;
			else
				self->s.angles[YAW] += -(1.0 - traceRight.fraction) * 45.0;
			
			ucmd->forwardmove = 400;
			ACEMV_ChangeBotAngle(self);
			return true;
		}
	}
				
	return false;
}

///////////////////////////////////////////////////////////////////////
// Make the change in angles a little more gradual, not so snappy
// Subtle, but noticeable.
// 
// Modified from the original id ChangeYaw code...
///////////////////////////////////////////////////////////////////////
void ACEMV_ChangeBotAngle (edict_t *ent)
{
	float	ideal_yaw;
	float   ideal_pitch;
	float	current_yaw;
	float   current_pitch;
	float	move;
	float	speed;
	vec3_t  ideal_angle;
			
	// Normalize the move angle first
	VectorNormalize(ent->move_vector);

	current_yaw = anglemod(ent->s.angles[YAW]);
	current_pitch = anglemod(ent->s.angles[PITCH]);
	
	vectoangles (ent->move_vector, ideal_angle);

	ideal_yaw = anglemod(ideal_angle[YAW]);
	ideal_pitch = anglemod(ideal_angle[PITCH]);

	// Yaw
	if (current_yaw != ideal_yaw)
	{	
		move = ideal_yaw - current_yaw;
		speed = ent->yaw_speed;
		if (ideal_yaw > current_yaw)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		if (move > 0)
		{
			if (move > speed)
				move = speed;
		}
		else
		{
			if (move < -speed)
				move = -speed;
		}
		ent->s.angles[YAW] = anglemod (current_yaw + move);	
	}

	// Pitch
	if (current_pitch != ideal_pitch)
	{	
		move = ideal_pitch - current_pitch;
		speed = ent->yaw_speed;
		if (ideal_pitch > current_pitch)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		if (move > 0)
		{
			if (move > speed)
				move = speed;
		}
		else
		{
			if (move < -speed)
				move = -speed;
		}
		ent->s.angles[PITCH] = anglemod (current_pitch + move);	
	}
}

///////////////////////////////////////////////////////////////////////
// Set bot to move to it's movetarget. (following node path)
///////////////////////////////////////////////////////////////////////
void ACEMV_MoveToGoal(edict_t *self, usercmd_t *ucmd)
{
	// If a rocket or grenade is around deal with it
	// Simple, but effective (could be rewritten to be more accurate)
	if(strcmp(self->movetarget->classname,"rocket")==0 ||
	   strcmp(self->movetarget->classname,"grenade")==0)
	{
		VectorSubtract (self->movetarget->s.origin, self->s.origin, self->move_vector);
		ACEMV_ChangeBotAngle(self);
		if(debug_mode)
			debug_printf("%s: Oh crap a rocket!\n",self->client->pers.netname);
		
		// strafe left/right
		if(rand()%1 && ACEMV_CanMove(self, MOVE_LEFT))
				ucmd->sidemove = -400;
		else if(ACEMV_CanMove(self, MOVE_RIGHT))
				ucmd->sidemove = 400;
		return;

	}
	else
	{
		// Set bot's movement direction
		VectorSubtract (self->movetarget->s.origin, self->s.origin, self->move_vector);
		ACEMV_ChangeBotAngle(self);
		ucmd->forwardmove = 400;
		return;
	}
}

///////////////////////////////////////////////////////////////////////
// Main movement code. (following node path)
///////////////////////////////////////////////////////////////////////
void ACEMV_Move(edict_t *self, usercmd_t *ucmd)
{
	vec3_t dist;
	int current_node_type=-1;
	int next_node_type=-1;
	int i;
	float	distance;

	// Get current and next node back from nav code.
	if(!ACEND_FollowPath(self))
	{
		if( 
			( !teamplay->value ) ||
			(teamplay->value && level.time >= (self->teamPauseTime))
			)
		{
			self->state = STATE_WANDER;
			self->wander_timeout = level.time + 1.0;
			return;
		}
		else
		{
			// Teamplay mode - just fan out and chill
			self->state = STATE_POSITION;
			self->goal_node = INVALID;
			self->wander_timeout = level.time + 1.0;
			return;
		}
	}

	current_node_type = nodes[self->current_node].type;
	next_node_type = nodes[self->next_node].type;
		
	///////////////////////////
	// Move To Goal
	///////////////////////////
	if (self->movetarget)
		ACEMV_MoveToGoal(self,ucmd);

/*	////////////////////////////////////////////////////////
	// Grapple
	///////////////////////////////////////////////////////
	if(next_node_type == NODE_GRAPPLE)
	{
		ACEMV_ChangeBotAngle(self);
		ACEIT_ChangeWeapon(self,FindItem("grapple"));	
		ucmd->buttons = BUTTON_ATTACK;
		return;
	}
	// Reset the grapple if hangin on a graple node
	if(current_node_type == NODE_GRAPPLE)
	{
		CTFPlayerResetGrapple(self);
		return;
	}*/
	////////////////////////////////////////////////////////
	// Doors - RiEvEr
	///////////////////////////////////////////////////////
	if(current_node_type == NODE_DOOR)
	{
		// check to see if door is open?
		for(i=0;i<num_items;i++)
		{
			if(item_table[i].node == self->current_node)
			{
				if( (item_table[i].ent->moveinfo.state != STATE_TOP) &&
					(item_table[i].ent->moveinfo.state != STATE_UP)		)
				{
					vec3_t	difference;
					float	distance;

					// Find distance to the door
					VectorSubtract (self->s.origin, nodes[self->current_node].origin, difference);
					distance = VectorLength(difference);

					// If not close enough carry on moving
					if( (distance > 64.0) 
						&& ( VectorLength(item_table[i].ent->avelocity) == 0.0) )
					{
						ucmd->forwardmove = 200;
						return;
					}

					// Within range
					if( self->last_door_time < (level.time - 2.0) )
					{
						Cmd_OpenDoor_f ( self );	// Open the door
						self->last_door_time = level.time; // wait!
					}
					ucmd->forwardmove = -200; //walk backwards a little
				    return; // Wait for door to open
				}
				else if( VectorLength(item_table[i].ent->avelocity) > 0.0)
				{
					ucmd->forwardmove = -200; //walk backwards a little
					return;
				}
			}
		}
	}
	
	////////////////////////////////////////////////////////
	// Platforms
	///////////////////////////////////////////////////////
	if(current_node_type != NODE_PLATFORM && next_node_type == NODE_PLATFORM)
	{
		// check to see if lift is down?
		for(i=0;i<num_items;i++)
			if(item_table[i].node == self->next_node)
				if(item_table[i].ent->moveinfo.state != STATE_BOTTOM)
				    return; // Wait for elevator
	}
	if(current_node_type == NODE_PLATFORM && next_node_type == NODE_PLATFORM)
	{
		// Move to the center
		self->move_vector[2] = 0; // kill z movement	
		if(VectorLength(self->move_vector) > 10)
			ucmd->forwardmove = 200; // walk to center
				
		ACEMV_ChangeBotAngle(self);
		
		return; // No move, riding elevator
	}

	////////////////////////////////////////////////////////
	// Jumpto Nodes
	///////////////////////////////////////////////////////
	if(next_node_type == NODE_JUMP || 
	  (current_node_type == NODE_JUMP && next_node_type != NODE_ITEM && nodes[self->next_node].origin[2] > self->s.origin[2]))
	{
		// Set up a jump move
		ucmd->forwardmove = 400;
		ucmd->upmove = 400;

		ACEMV_ChangeBotAngle(self);

		// RiEvEr - kill the velocity hack
/*		VectorCopy(self->move_vector,dist);
		VectorScale(dist,440,self->velocity);*/

		return;
	}
	
	////////////////////////////////////////////////////////
	// Ladder Nodes
	///////////////////////////////////////////////////////

	// Find the distance vector to the next node
	VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist);
	// Lose the vertical component
	dist[2] = 0;
	// Get the absolute length
	distance = VectorLength(dist);

	if(next_node_type == NODE_LADDER && //(gi.pointcontents(self->s.origin) & CONTENTS_LADDER) &&
		nodes[self->next_node].origin[2] > self->s.origin[2] &&
		distance < NODE_DENSITY)
	{
		// Otherwise move as fast as we can
		ucmd->forwardmove = 100; // Reduced from 400
		self->velocity[2] = 320; // Reduced from 320
		ucmd->sidemove = 0;
		
		ACEMV_ChangeBotAngle(self);
		
		return;

	}
	// If getting off the ladder 
	if(
		(current_node_type == NODE_LADDER && next_node_type != NODE_LADDER &&
	   nodes[self->next_node].origin[2] > self->s.origin[2])
	   )
	{
		ucmd->forwardmove = 200; // Reduced from 400
//		ucmd->upmove = 200;
		self->velocity[2] = 200;
		ACEMV_ChangeBotAngle(self);
		return;
	}
	// If getting onto the ladder 
	if(
		(current_node_type != NODE_LADDER && next_node_type == NODE_LADDER )//&&
//	   (nodes[self->next_node].origin[2] > self->s.origin[2])
	   )
	{
		ucmd->forwardmove = 100; // Reduced from 400
//		ucmd->upmove = 200;
//		self->velocity[2] = 200;
		ACEMV_ChangeBotAngle(self);
		return;
	}

	//==============================
	// LEDGES etc..
	// If trying to jump up a ledge
	if(
	   (current_node_type == NODE_MOVE &&
	   nodes[self->next_node].origin[2] > self->s.origin[2]+16 && distance < NODE_DENSITY)
	   )
	{
		ucmd->forwardmove = 400; 
		ucmd->upmove = 400;
		ACEMV_ChangeBotAngle(self);
		return;
	}
	////////////////////////////////////////////////////////
	// Water Nodes
	///////////////////////////////////////////////////////
	if(current_node_type == NODE_WATER)
	{
		// We need to be pointed up/down
		ACEMV_ChangeBotAngle(self);

		// If the next node is not in the water, then move up to get out.
		if(next_node_type != NODE_WATER && !(gi.pointcontents(nodes[self->next_node].origin) & MASK_WATER)) // Exit water
			ucmd->upmove = 400;
		
		ucmd->forwardmove = 300;
		return;

	}
	
	// Falling off ledge?
/*	if(!self->groundentity)
	{
		ACEMV_ChangeBotAngle(self);

		self->velocity[0] = self->move_vector[0] * 360;
		self->velocity[1] = self->move_vector[1] * 360;
	
		return;
	}*/
		
	// Check to see if stuck, and if so try to free us
	// Also handles crouching
 	if(VectorLength(self->velocity) < 37)
	{
		// Keep a random factor just in case....
		if(random() > 0.1 && ACEMV_SpecialMove(self, ucmd))
			return;
		
		self->s.angles[YAW] += random() * 180 - 90; 

		ACEMV_ChangeBotAngle(self);

		ucmd->forwardmove = 400;
		
		return;
	}

	ACEMV_ChangeBotAngle(self);

	// Otherwise move as fast as we can
	// If it's safe to move forward (I can't believe ACE didn't check this!
	if( ACEMV_CanMove( self, MOVE_FORWARD) || (current_node_type == NODE_LADDER) )
	{
		ucmd->forwardmove = 400; 
	}
	else
	{
		// Forget about this route!
//		ucmd->forwardmove = -100; 
		self->movetarget = NULL;
	}	
}


///////////////////////////////////////////////////////////////////////
// Wandering code (based on old ACE movement code) 
///////////////////////////////////////////////////////////////////////
//
// RiEvEr - this routine is of a very poor standard and has a LOT of problems
// Maybe replace this with the DroneBot or ReDeMpTiOn wander code?
//
void ACEMV_Wander(edict_t *self, usercmd_t *ucmd)
{
	vec3_t  temp;
	
	// Do not move
	if(self->next_move_time > level.time)
		return;

	// Special check for elevators, stand still until the ride comes to a complete stop.
	if(self->groundentity != NULL && self->groundentity->use == Use_Plat)
		if(self->groundentity->moveinfo.state == STATE_UP ||
		   self->groundentity->moveinfo.state == STATE_DOWN) // only move when platform not
		{
			self->velocity[0] = 0;
			self->velocity[1] = 0;
			self->velocity[2] = 0;
			self->next_move_time = level.time + 0.5;
			return;
		}
	
	
	// Is there a target to move to
	if (self->movetarget)
		ACEMV_MoveToGoal(self,ucmd);
		
	////////////////////////////////
	// Swimming?
	////////////////////////////////
	VectorCopy(self->s.origin,temp);
	temp[2]+=24;

	if(gi.pointcontents (temp) & MASK_WATER)
	{
		// If drowning and no node, move up
		if(self->client->next_drown_time > 0)
		{
			ucmd->upmove = 1;	
			self->s.angles[PITCH] = -45; 
		}
		else
			ucmd->upmove = 15;	

		ucmd->forwardmove = 300;
	}
//	else
//		self->client->next_drown_time = 0; // probably shound not be messing with this, but
	
	////////////////////////////////
	// Lava?
	////////////////////////////////
	temp[2]-=48;	
	if(gi.pointcontents(temp) & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		//	safe_bprintf(PRINT_MEDIUM,"lava jump\n");
		self->s.angles[YAW] += random() * 360 - 180; 
		ucmd->forwardmove = 400;
		ucmd->upmove = 400;
		return;
	}

//@@	if(ACEMV_CheckEyes(self,ucmd))
//		return;
	
	// Check for special movement if we have a normal move (have to test)
 	if(VectorLength(self->velocity) < 37)
	{
		if(random() > 0.1 && ACEMV_SpecialMove(self,ucmd))
			return;

		self->s.angles[YAW] += random() * 180 - 90; 

		if(!M_CheckBottom && !self->groundentity) // if there is ground continue otherwise wait for next move
			ucmd->forwardmove = 0;
		else if( ACEMV_CanMove( self, MOVE_FORWARD))
			ucmd->forwardmove = 200;
		
		return;
	}
	
	// Otherwise move as fast as we can
	// If it's safe to move forward (I can't believe ACE didn't check this!
	if( ACEMV_CanMove( self, MOVE_FORWARD))
	{
		ucmd->forwardmove = 400; 
	}
	else
	{
		// Need a "findbestdirection" routine in here
		// Forget about this route!
		ucmd->forwardmove = -100; 
		self->movetarget = NULL;
	}	

}

///////////////////////////////////////////////////////////////////////
// Attack movement routine
//
// NOTE: Very simple for now, just a basic move about avoidance.
//       Change this routine for more advanced attack movement.
///////////////////////////////////////////////////////////////////////
void ACEMV_Attack (edict_t *self, usercmd_t *ucmd)
{
	float c;
	vec3_t  target;
	vec3_t  angles;
	vec3_t	attackvector;
	float	dist;
	
	// Randomly choose a movement direction
	c = random();
	
	if(c < 0.2 && ACEMV_CanMove(self,MOVE_LEFT))
		ucmd->sidemove -= 400; 
	else if(c < 0.4 && ACEMV_CanMove(self,MOVE_RIGHT))
		ucmd->sidemove += 400; 

//AQ2 CHANGE
	// Don't stand around if all you have is a knife!
	if( self->client->pers.weapon == FindItem(KNIFE_NAME) )
	{
		ucmd->forwardmove += 400;
	}
	else
	{
		if(c < 0.6 && ACEMV_CanMove(self,MOVE_FORWARD))
			ucmd->forwardmove += 400; 
		else if(c < 0.8 && ACEMV_CanMove(self,MOVE_FORWARD))
			ucmd->forwardmove -= 400;
	}
//AQ2 END
	// Check distance to enemy
	VectorSubtract( self->s.origin, self->enemy->s.origin, attackvector);
	dist = VectorLength( attackvector);

	if( (dist < 600) && !(self->client->pers.weapon == FindItem(KNIFE_NAME)) )
	{
		// Randomly choose a vertical movement direction
		c = random();

		if(c < 0.15)
			ucmd->upmove += 200;
		else
			ucmd->upmove -= 200;
	}
	
	// Set the attack 
	//@@ Check this doesn't break grenades!
	if((self->client->weaponstate == WEAPON_READY)||(self->client->weaponstate == WEAPON_FIRING))
	{
		// Only shoot if the weapon is ready and we can hit the target!
		//@@ Removed to try to help RobbieBoy!
//		if( ACEAI_CheckShot( self ))
			ucmd->buttons = BUTTON_ATTACK;
//		else
//			ucmd->upmove = 200;
	}
	
	// Aim
	VectorCopy(self->enemy->s.origin,target);

	// modify attack angles based on accuracy (mess this up to make the bot's aim not so deadly)
//	target[0] += (random()-0.5) * 20;
//	target[1] += (random()-0.5) * 20;

//AQ2 ADD - RiEvEr
	// Alter aiming based on skill level
	if( ltk_skill->value < 10 )
	{
		short int	up, right;
		up = (random() < 0.5)? -1 :1;
		right = (random() < 0.5)? -1 : 1;

		// Not that complex. We miss by 0 to 80 units based on skill value and random factor
		target[0] += ( right * (10 - ltk_skill->value +((8*(10 - ltk_skill->value)) *random())) );
		target[2] += ( up * (10 - ltk_skill->value +((8*(10 - ltk_skill->value)) *random())) );
	}
//AQ2 END
	
	// Set direction
	VectorSubtract (target, self->s.origin, self->move_vector);
	vectoangles (self->move_vector, angles);
	VectorCopy(angles,self->s.angles);

	// Store time we last saw an enemy
	// This value is used to decide if we initiate a long range search or not.
	self->teamPauseTime = level.time;
	
//	if(debug_mode)
//		debug_printf("%s attacking %s\n",self->client->pers.netname,self->enemy->client->pers.netname);
}

//==========================
// AntPathMove
//==========================
//
qboolean	AntPathMove( edict_t *self )
{
	node_t *temp = &nodes[self->current_node];	// For checking our position

	if( level.time == (float)((int)level.time) )
	{
		if( 
			!AntLinkExists( self->current_node, SLLfront(&self->pathList) )
			&& ( self->current_node != SLLfront(&self->pathList) )
			)
		{
			// We are off the path - clear out the lists
			AntInitSearch( self );
		}
	}

	// Boot in our new pathing algorithm
	// This will fill ai.pathList with the information we need
	if( 
		SLLempty(&self->pathList)				// We have no path and
		&& (self->current_node != self->goal_node)	// we're not at our destination
		)
	{
		if( !AntStartSearch( self, self->current_node, self->goal_node))	// Set up our pathList
		{
			// Failed to find a path
//			gi.bprintf(PRINT_HIGH,"%s: Target at(%i) - No Path \n",
//				self->client->pers.netname, self->goal_node, self->next_node);
			return false;
		}
		return true;
	}

	// And change our pathfinding to use it
	if ( !SLLempty(&self->pathList) )								// We have a path 
	{
//		if( AtNextNode( self->s.origin, SLLfront(&self->pathList))	// we're at the nextnode
		if( ACEND_FindClosestReachableNode(self,NODE_DENSITY*0.66,NODE_ALL) == SLLfront(&self->pathList)	// we're at the nextnode
			|| self->current_node == SLLfront(&self->pathList) 
		)
		{
			self->current_node = SLLfront(&self->pathList);		// Show we're there
			SLLpop_front(&self->pathList);				// Remove the top node
		}
		if( !SLLempty(&self->pathList) )
			self->next_node = SLLfront(&self->pathList);	// Set our next destination
		else
			self->next_node = INVALID;				// We're at the target location
		return true;
	}
	return true;	// Pathlist is emptyand we are at our destination
}

