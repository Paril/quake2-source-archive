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
 * $Header: /LicenseToKill/src/acesrc/acebot_nodes.c 29    26/11/99 0:48 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/acebot_nodes.c $
 * 
 * 29    26/11/99 0:48 Riever
 * Link creation safety checks..
 * 
 * 28    27/10/99 18:49 Riever
 * Added some debug code to handle lightlevel checks.
 * 
 * 27    22/10/99 20:53 Riever
 * Placed own path showing code under ltk_showpath control.
 * 
 * 26    22/10/99 20:30 Riever
 * Checking ltk_showpath set up right.
 * 
 * 25    22/10/99 7:36 Riever
 * Safety check to prevent falling damage links being created.
 * 
 * 24    22/10/99 7:27 Riever
 * Updated node file version to 3
 * 
 * 23    22/10/99 7:17 Riever
 * Stopped node files of less than 100 nodes being saved - this will
 * prevent good node files being over-written when the game glitches.
 * 
 * 22    21/10/99 8:15 Riever
 * Added ltk_showpath CVAR to toggle display of bot path selection
 * information.
 * 
 * 21    20/10/99 20:26 Riever
 * Fixed Ladder node creation.
 * 
 * 20    10/10/99 9:33 Riever
 * Found a bug in reverse link - fixed!
 * 
 * 19    6/10/99 17:54 Riever
 * ... and another!
 * 
 * 18    6/10/99 17:53 Riever
 * Removed another debug print.
 * 
 * 17    6/10/99 17:51 Riever
 * Removed debug prints and disabled path visibility code.
 * 
 * 16    6/10/99 17:40 Riever
 * Added TeamPlay state STATE_POSITION to enable bots to seperate and
 * avoid centipede formations.
 * 
 * 15    29/09/99 13:32 Riever
 * Changed all node linkage code to check a trace.
 * Changed nearest node code to find a reliable reachable node.
 * Changed ShowPath function to show the AntPath to the destination, not
 * the array path.
 * 
 * 14    28/09/99 7:47 Riever
 * Switched out a couple of debug prints that were appearing in non debug
 * mode
 * 
 * 13    27/09/99 20:46 Riever
 * Changed trace tests for node linkage and detection to use vec3_origins
 * 
 * 12    27/09/99 16:01 Riever
 * Added "self" to ACEND_UpdateNodeEdge calls
 * 
 * 11    27/09/99 14:59 Riever
 * Changed ACE link code to TRACE IT FIRST!!! I know, no-one else will
 * believe that this wasn't in there either *grin*....
 * 
 * 10    27/09/99 14:53 Riever
 * Changed nearest node check to mask ALL (temporary until I find the
 * problem with thin walls in this game!)
 * Increased node table version number to 2 because node density increased
 * in acebot.h to 96.
 * 
 * 9     27/09/99 14:29 Riever
 * Modified ACEND_FollowPath to utilise the AntPath code
 * 
 * 8     26/09/99 8:01 Riever
 * ACEND_ReverseLink function in place ready for new path system.
 * 
 * 7     21/09/99 11:58 Riever
 * fixed INVALID node searches
 * 
 * 6     18/09/99 19:21 Riever
 * NODE_DOOR creation fixed to place the node in the center of the closed
 * door.
 * 
 * 5     18/09/99 8:08 Riever
 * NODE_DOOR creation now places node at center of the door to help bot
 * navigation. Still need to sort out non rotating doors.
 * 
 * 4     17/09/99 17:06 Riever
 * Changed node table save directory to be "terrain" as advised by William
 * 
 * 3     17/09/99 17:04 Riever
 * New node structure implemented
 * Link structure added
 * Found nodetable creation bug at timelimit and fixed it.
 * Changed load and save code to be "game directory" friendly.
 * Made node table version number a CONSTANT
 * 
 * 2     13/09/99 19:52 Riever
 * Added headers
 *
 */

///////////////////////////////////////////////////////////////////////	
//
//  acebot_nodes.c -   This file contains all of the 
//                     pathing routines for the ACE bot.
// 
///////////////////////////////////////////////////////////////////////

#include "g_local.h"
#include "acebot.h"

#define LTK_NODEVERSION 3
#define TRACE_DIST_LADDER 16

// flags
qboolean newmap=true;

// Total number of nodes that are items
int numitemnodes; 

// Total number of nodes
int numnodes; 

// For debugging paths
int show_path_from = -1;
int show_path_to = -1;

// array for node data
node_t nodes[MAX_NODES]; 
short int path_table[MAX_NODES][MAX_NODES];

///////////////////////////////////////////////////////////////////////
// NODE INFORMATION FUNCTIONS
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Determin cost of moving from one node to another
///////////////////////////////////////////////////////////////////////
int ACEND_FindCost(int from, int to)
{
	// RiEvEr - Bug Hunting
	int curnode = INVALID;
	int cost=1; // Shortest possible is 1

	// If we can not get there then return invalid
	if( (from == INVALID) || (to == INVALID) || 
		(path_table[from][to] == INVALID)	)
		return INVALID;

	// Otherwise check the path and return the cost
	curnode = path_table[from][to];

	// Find a path (linear time, very fast)
	while(curnode != to)
	{
		curnode = path_table[curnode][to];
		if(curnode == INVALID) // something has corrupted the path abort
			return INVALID;
		cost++;
	}
	
	return cost;
}

///////////////////////////////////////////////////////////////////////
// Find a close node to the player within dist.
//
// Faster than looking for the closest node, but not very 
// accurate.
///////////////////////////////////////////////////////////////////////
int ACEND_FindCloseReachableNode(edict_t *self, int range, int type)
{
	vec3_t v;
	int i;
	trace_t tr;
	float dist;
	vec3_t maxs,mins;

	VectorCopy(self->mins,mins);
	mins[2] += 16;
	VectorCopy(self->maxs,maxs);
	maxs[2] -= 16;

	range *= range;

	for(i=0;i<numnodes;i++)
	{
		if(type == NODE_ALL || type == nodes[i].type) // check node type
		{
		
			VectorSubtract(nodes[i].origin,self->s.origin,v); // subtract first

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

			if(dist < range) // square range instead of sqrt
			{
				// make sure it is visible
				//AQ2 ADDED MASK_SOLID
				//trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, nodes[i].origin, self, MASK_SOLID|MASK_OPAQUE);
				tr = gi.trace (self->s.origin, mins, maxs, nodes[i].origin, self, MASK_ALL);

				if(tr.fraction == 1.0)
					return i;
			}
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////
// Find the closest node to the player within a certain range
///////////////////////////////////////////////////////////////////////
int ACEND_FindClosestReachableNode(edict_t *self, int range, int type)
{
	int i;
	float closest = 99999;
	float dist;
	int node=-1;
	vec3_t v;
	trace_t tr;
	float rng;
	vec3_t maxs,mins;

	VectorCopy(self->mins,mins);
	VectorCopy(self->maxs,maxs);
	
	// For Ladders, do not worry so much about reachability
	if(type == NODE_LADDER)
	{
		VectorCopy(vec3_origin,maxs);
		VectorCopy(vec3_origin,mins);
	}
	else
	{
		mins[2] += 18; // Stepsize
		maxs[2] -= 16; // Duck a little.. 
	}

	rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)	
	
	for(i=0;i<numnodes;i++)
	{		
		if(type == NODE_ALL || type == nodes[i].type) // check node type
		{
			VectorSubtract(nodes[i].origin, self->s.origin,v); // subtract first

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
		
			if(dist < closest && dist < rng) 
			{
				// make sure it is visible
				//AQ2 added MASK_SOLID
				tr = gi.trace (self->s.origin, mins, maxs, nodes[i].origin, self, MASK_SOLID|MASK_OPAQUE);
//				tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, nodes[i].origin, self, MASK_ALL);
				if( (tr.fraction == 1.0) ||
					(	(tr.fraction > 0.9) // may be blocked by the door itself!
						&& (Q_stricmp(tr.ent->classname, "func_door_rotating") == 0)	)	
					)
				{
					node = i;
					closest = dist;
				}
			}
		}
	}
	
	return node;
}

///////////////////////////////////////////////////////////////////////
// BOT NAVIGATION ROUTINES
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Set up the goal
///////////////////////////////////////////////////////////////////////
void ACEND_SetGoal(edict_t *self, int goal_node)
{
	int node;

	self->goal_node = goal_node;
	node = ACEND_FindClosestReachableNode(self, NODE_DENSITY*3, NODE_ALL);
	
	if(node == -1)
		return;
	
	if(debug_mode)
		debug_printf("%s new start node selected %d\n",self->client->pers.netname,node);
	
	
	self->current_node = node;
	self->next_node = self->current_node; // make sure we get to the nearest node first
	self->node_timeout = 0;

}

///////////////////////////////////////////////////////////////////////
// Move closer to goal by pointing the bot to the next node
// that is closer to the goal
///////////////////////////////////////////////////////////////////////
qboolean ACEND_FollowPath(edict_t *self)
{
	vec3_t v;
	
	//////////////////////////////////////////
	// Show the path (uncomment for debugging)
	show_path_from = self->current_node;
	show_path_to = self->goal_node;

	if( ltk_showpath->value )
	{
		ACEND_DrawPath(self);
	}
	//////////////////////////////////////////

	// Try again?
	if(self->node_timeout ++ > 30)
	{
		if(self->tries++ > 3)
			return false;
		else
			ACEND_SetGoal(self,self->goal_node);
	}

	//RiEvEr - new path code & algorithm
	// This part checks if we are off course
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
	// This will fill self->pathList with the information we need
	if( 
		SLLempty(&self->pathList)				// We have no path and
		&& (self->current_node != self->goal_node)	// we're not at our destination
		)
	{
		if( !AntStartSearch( self, self->current_node, self->goal_node))	// Set up our pathList
		{
			// Failed to find a path
			if( debug_mode )
				gi.bprintf(PRINT_HIGH,"%s: Target at(%i) - No Path \n",
					self->client->pers.netname, self->goal_node, self->next_node);
			return false;
		}
//		return true;
	}
	//R

	// Are we there yet?
	VectorSubtract(self->s.origin,nodes[self->next_node].origin,v);
	
	if(VectorLength(v) < 32) 
	{
		// reset timeout
		self->node_timeout = 0;

		if(self->next_node == self->goal_node)
		{
			if(debug_mode)
				debug_printf("%s reached goal!\n",self->client->pers.netname);	
			
			ACEAI_PickLongRangeGoal(self); // Pick a new goal
		}
		else
		{
			self->current_node = self->next_node;
//			self->next_node = path_table[self->current_node][self->goal_node];
			// Removethe front entry from the list
			SLLpop_front(&self->pathList);
			// Get the next node - if there is one!
			if( !SLLempty(&self->pathList))
				self->next_node = SLLfront( &self->pathList);
			else
			{
				// We messed up...
				if( debug_mode)
					gi.bprintf(PRINT_HIGH, "Trying to read an empty SLL nodelist!\n");
				self->next_node = INVALID;
			}
		}
	}
	
	if(self->current_node == -1 || self->next_node ==-1)
		return false;
	
	// Set bot's movement vector
	VectorSubtract (nodes[self->next_node].origin, self->s.origin , self->move_vector);
	
	return true;
}


///////////////////////////////////////////////////////////////////////
// MAPPING CODE
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Capture when the grappling hook has been fired for mapping purposes.
///////////////////////////////////////////////////////////////////////
void ACEND_GrapFired(edict_t *self)
{
/*	int closest_node;
	
	if(!self->owner)
		return; // should not be here
	
	// Check to see if the grapple is in pull mode
	if(self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
	{
		// Look for the closest node of type grapple
		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_GRAPPLE);
		if(closest_node == -1 ) // we need to drop a node
		{	
			closest_node = ACEND_AddNode(self,NODE_GRAPPLE);
			 
			// Add an edge
			ACEND_UpdateNodeEdge(self, self->owner->last_node,closest_node);
		
			self->owner->last_node = closest_node;
		}
		else
			self->owner->last_node = closest_node; // zero out so other nodes will not be linked
	}*/
}


///////////////////////////////////////////////////////////////////////
// Check for adding ladder nodes
///////////////////////////////////////////////////////////////////////
qboolean ACEND_CheckForLadder(edict_t *self)
{
	int closest_node;

	// If there is a ladder and we are moving up, see if we should add a ladder node
	if (gi.pointcontents(self->s.origin) & CONTENTS_LADDER && self->velocity[2] > 0)
	{
		//debug_printf("contents: %x\n",tr.contents);

		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_LADDER); 
		if(closest_node == -1)
		{
			closest_node = ACEND_AddNode(self,NODE_LADDER);
	
			// Now add link
		    ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			
			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			self->last_node = closest_node; // set visited to last
		}
		return true;
	}
	return false;
}

//=======================================================
// LadderForward
//=======================================================
//
// The ACE code version of this doesn't work!

qboolean	ACEND_LadderForward( edict_t *self )//, vec3_t angles )
{
	vec3_t	dir, angle, dest, min, max;
	trace_t	trace;
	int closest_node;


	VectorClear(angle);
	angle[1] = self->s.angles[1];

	AngleVectors(angle, dir, NULL, NULL);
	VectorCopy(self->mins,min);
	min[2] += 22;
	VectorCopy(self->maxs,max);
	VectorMA(self->s.origin, TRACE_DIST_LADDER, dir, dest);

	trace = gi.trace(self->s.origin, min, max, dest, self, MASK_ALL);

	//TempLaser(self->s.origin, dest);
	if (trace.fraction == 1.0)
		return (false);

//	safe_bprintf(PRINT_HIGH,"Contents forward are %d\n", trace.contents);
	if (trace.contents & CONTENTS_LADDER || trace.contents &CONTENTS_DETAIL)
	{
		// Debug print
//		safe_bprintf(PRINT_HIGH,"contents: %x\n",trace.contents);

		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_LADDER); 
		if(closest_node == -1)
		{
			closest_node = ACEND_AddNode(self,NODE_LADDER);
	
			// Now add link
		    ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			
			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			self->last_node = closest_node; // set visited to last
		}
		return (true);
	}		
	return (false);
}


///////////////////////////////////////////////////////////////////////
// This routine is called to hook in the pathing code and sets
// the current node if valid.
///////////////////////////////////////////////////////////////////////
void ACEND_PathMap(edict_t *self)
{
	int closest_node;
	// Removed last_update checks since this stopped multiple node files being built
	vec3_t v;

	// Special node drawing code for debugging
	if( ltk_showpath->value )
	{
		if(show_path_to != -1)
			ACEND_DrawPath( self );
	}
	
	// Just checking lightlevels - uncomment to use
//	if( debug_mode && !self->is_bot)
//		safe_bprintf(PRINT_HIGH,"LightLevel = %d\n", self->light_level);

	////////////////////////////////////////////////////////
	// Special check for ladder nodes
	///////////////////////////////////////////////////////
	// Replace non-working ACE version with mine.
//	if(ACEND_CheckForLadder(self)) // check for ladder nodes
	if(ACEND_LadderForward(self)) // check for ladder nodes
		return;

	// Not on ground, and not in the water, so bail
    if(!self->groundentity && !self->waterlevel)
		return;

	////////////////////////////////////////////////////////
	// Lava/Slime
	////////////////////////////////////////////////////////
	VectorCopy(self->s.origin,v);
	v[2] -= 18;
	if(gi.pointcontents(v) & (CONTENTS_LAVA|CONTENTS_SLIME))
		return; // no nodes in slime
	
    ////////////////////////////////////////////////////////
	// Jumping
	///////////////////////////////////////////////////////
	if(self->is_jumping)
	{
	   // See if there is a closeby jump landing node (prevent adding too many)
		closest_node = ACEND_FindClosestReachableNode(self, 64, NODE_JUMP);

		if(closest_node == INVALID)
			closest_node = ACEND_AddNode(self,NODE_JUMP);
		
		// Now add link
		if(self->last_node != -1)
			ACEND_UpdateNodeEdge(self, self->last_node, closest_node);	   

		self->is_jumping = false;
		return;
	}

/*	////////////////////////////////////////////////////////////
	// Grapple
	// Do not add nodes during grapple, added elsewhere manually
	////////////////////////////////////////////////////////////
	if(ctf->value && self->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
		return;*/
	 
	// Iterate through all nodes to make sure far enough apart
	closest_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	////////////////////////////////////////////////////////
	// Special Check for Platforms
	////////////////////////////////////////////////////////
	if(self->groundentity && self->groundentity->use == Use_Plat)
	{
		if(closest_node == INVALID)
			return; // Do not want to do anything here.

		// Here we want to add links
		if(closest_node != self->last_node && self->last_node != INVALID)
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   

		self->last_node = closest_node; // set visited to last
		return;
	}
	 
	 ////////////////////////////////////////////////////////
	 // Add Nodes as needed
	 ////////////////////////////////////////////////////////
	 if(closest_node == INVALID)
	 {
		// Add nodes in the water as needed
		if(self->waterlevel)
			closest_node = ACEND_AddNode(self,NODE_WATER);
		else
		    closest_node = ACEND_AddNode(self,NODE_MOVE);
		
		// Now add link
		if(self->last_node != -1)
			ACEND_UpdateNodeEdge(self, self->last_node, closest_node);	   
			
	 }
	 else if(closest_node != self->last_node && self->last_node != INVALID)
	 	ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
	
	 self->last_node = closest_node; // set visited to last
	
}

///////////////////////////////////////////////////////////////////////
// Init node array (set all to INVALID)
///////////////////////////////////////////////////////////////////////
void ACEND_InitNodes(void)
{
	numnodes = 1;
	numitemnodes = 1;
	memset(nodes,0,sizeof(node_t) * MAX_NODES);
	memset(path_table,INVALID,sizeof(short int)*MAX_NODES*MAX_NODES);
			
}

///////////////////////////////////////////////////////////////////////
// Show the node for debugging (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowNode(int node)
{
	edict_t *ent;

//	return; // commented out for now. uncommend to show nodes during debugging,
	        // but too many will cause overflows. You have been warned.

	ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if(nodes[node].type == NODE_MOVE)
		ent->s.renderfx = RF_SHELL_BLUE;
	else if (nodes[node].type == NODE_WATER)
		ent->s.renderfx = RF_SHELL_RED;
	else			
		ent->s.renderfx = RF_SHELL_GREEN; // action nodes

	ent->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
	ent->owner = ent;
	ent->nextthink = level.time + 600.0;	// 10 minutes is long enough!
	ent->think = G_FreeEdict;                
	ent->dmg = 0;

	VectorCopy(nodes[node].origin,ent->s.origin);
	gi.linkentity (ent);

}

///////////////////////////////////////////////////////////////////////
// Draws the current path (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_DrawPath(edict_t *self)
{
	int current_node, goal_node, next_node;

	current_node = show_path_from;
	goal_node = show_path_to;

	// RiEvEr - rewritten to use Ant system
	AntStartSearch( self, current_node, goal_node);

	next_node = SLLfront(&self->pathList);

	// Now set up and display the path
	while( current_node != goal_node && current_node != INVALID)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (nodes[current_node].origin);
		gi.WritePosition (nodes[next_node].origin);
		gi.multicast (nodes[current_node].origin, MULTICAST_PVS);
		current_node = next_node;
		SLLpop_front( &self->pathList);
		next_node = SLLfront(&self->pathList);
	}

/*
	next_node = path_table[current_node][goal_node];

	// Now set up and display the path
	while(current_node != goal_node && current_node != -1)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (nodes[current_node].origin);
		gi.WritePosition (nodes[next_node].origin);
		gi.multicast (nodes[current_node].origin, MULTICAST_PVS);
		current_node = next_node;
		next_node = path_table[current_node][goal_node];
	}*/
}

///////////////////////////////////////////////////////////////////////
// Turns on showing of the path, set goal to -1 to 
// shut off. (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowPath(edict_t *self, int goal_node)
{
	show_path_from = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
	show_path_to = goal_node;
}

///////////////////////////////////////////////////////////////////////
// Add a node of type ?
///////////////////////////////////////////////////////////////////////
int ACEND_AddNode(edict_t *self, int type)
{
	vec3_t v1,v2;
	int i;

	// Block if we exceed maximum
	if (numnodes + 1 > MAX_NODES)
		return false;
	
	// Set location
	VectorCopy(self->s.origin, nodes[numnodes].origin);

	// Set type
	nodes[numnodes].type = type;
	// Set number - RiEvEr
	nodes[numnodes].nodenum = numnodes;

	// Clear out the link information - RiEvEr
	for( i = 0; i< MAXLINKS; i++)
	{
		nodes[numnodes].links[i].targetNode = INVALID;
	}

	/////////////////////////////////////////////////////
	// ITEMS
	// Move the z location up just a bit.
	if(type == NODE_ITEM)
	{
		nodes[numnodes].origin[2] += 16;
		numitemnodes++;
	}

	// Teleporters
	if(type == NODE_TELEPORTER)
	{
		// Up 32
		nodes[numnodes].origin[2] += 32;
	}

	// Doors
	if(type == NODE_DOOR)
	{
		vec3_t	position;
		// Find mid point of door max and min and put the node there
		VectorCopy(self->s.origin, position);
        // find center of door
		position[0] = position[0] + self->mins[0] + ((self->maxs[0] - self->mins[0]) /2);
		position[1] = position[1] + self->mins[1] + ((self->maxs[1] - self->mins[1]) /2);
		position[2] -= 16; // lower it a little
		// Set location
		VectorCopy(position, nodes[numnodes].origin);
	}

	if(type == NODE_LADDER)
	{
		nodes[numnodes].type = NODE_LADDER;
				
		if(debug_mode)
		{
			debug_printf("Node added %d type: Ladder\n",numnodes);
			ACEND_ShowNode(numnodes);
		}
		
		numnodes++;
		return numnodes-1; // return the node added

	}

	// For platforms drop two nodes one at top, one at bottom
	if(type == NODE_PLATFORM)
	{
		VectorCopy(self->maxs,v1);
		VectorCopy(self->mins,v2);
		
		// To get the center
		nodes[numnodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
		nodes[numnodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
		nodes[numnodes].origin[2] = self->maxs[2];
			
		if(debug_mode)	
			ACEND_ShowNode(numnodes);
		
		numnodes++;

		nodes[numnodes].origin[0] = nodes[numnodes-1].origin[0];
		nodes[numnodes].origin[1] = nodes[numnodes-1].origin[1];
		nodes[numnodes].origin[2] = self->mins[2]+64;
		
		nodes[numnodes].type = NODE_PLATFORM;

		// Add a link 
		//RiEvEr modified to pass in calling entity
		ACEND_UpdateNodeEdge(self, numnodes, numnodes-1);			
		
		if(debug_mode)
		{
			debug_printf("Node added %d type: Platform\n",numnodes);
			ACEND_ShowNode(numnodes);
		}

		numnodes++;

		return numnodes -1;
	}
		
	if(debug_mode)
	{
		if(nodes[numnodes].type == NODE_MOVE)
			debug_printf("Node added %d type: Move\n",numnodes);
		else if(nodes[numnodes].type == NODE_TELEPORTER)
			debug_printf("Node added %d type: Teleporter\n",numnodes);
		else if(nodes[numnodes].type == NODE_ITEM)
			debug_printf("Node added %d type: Item\n",numnodes);
		else if(nodes[numnodes].type == NODE_WATER)
			debug_printf("Node added %d type: Water\n",numnodes);
		else if(nodes[numnodes].type == NODE_GRAPPLE)
			debug_printf("Node added %d type: Grapple\n",numnodes);

		ACEND_ShowNode(numnodes);
	}
		
	numnodes++;
	
	return numnodes-1; // return the node added
}

// RiEvEr
//=======================================
// ReverseLink
//=======================================
// Takes the path BACK to where we came from
// and tries to link the two nodes
// This helps make good path files
//
void ACEND_ReverseLink( edict_t *self, int from, int to )
{
	int	i;
	trace_t	trace;
	vec3_t	min,max;
	
	if(from == INVALID || to == INVALID || from == to)
		return; // safety

	// Need to trace from -> to and check heights
	// if from is much lower than to, forget it
	if( (nodes[from].origin[2]+32.0) < (nodes[to].origin[2]) )
	{
		// May not be able to jump that high so do not allow the return link
		return;
	}
	VectorCopy(self->mins, min);
//	if( (nodes[from].origin[2]) < (nodes[2].origin[2]) )
		min[2]  =0;	// Allow for steps etc.
	VectorCopy(self->maxs, max);
//	if( (nodes[from].origin[2]) > (nodes[2].origin[2]) )
		max[2] =0;	// Could be a downward sloping feature above our head


	// This should not be necessary, but I've heard that before!
	// Now trace it again
	trace = gi.trace( nodes[from].origin, min, max, nodes[to].origin, self, MASK_SOLID);
	if( trace.fraction < 1.0)
	{
		// can't get there for some reason
		return;
	}
	// Add the link
	path_table[from][to] = to;

	// Checks if the link exists and then may create a new one - RiEvEr
	for( i=0; i<MAXLINKS; i++)
	{
		if ( nodes[from].links[i].targetNode == to)
			break;
		if ( nodes[from].links[i].targetNode == INVALID)
		{
			// RiEvEr
			// William uses a time factor here, whereas I use distance
			// His is possibly more efficient
			vec3_t	v;
			float thisCost;

			VectorSubtract(nodes[from].origin, nodes[to].origin, v); // subtract first
			thisCost = VectorLength(v);
			nodes[from].links[i].targetNode = to;
			nodes[from].links[i].cost = thisCost;
			break;
		}
	}

	// Now for the self-referencing part, linear time for each link added
	for(i=0;i<numnodes;i++)
		if(path_table[i][from] != INVALID)
			if(i == to)
				path_table[i][to] = INVALID; // make sure we terminate
			else
				path_table[i][to] = path_table[i][from];
		
//	if(debug_mode)
//		debug_printf("ReverseLink %d -> %d\n", from, to);
}
//R
	
///////////////////////////////////////////////////////////////////////
// Add/Update node connections (paths)
///////////////////////////////////////////////////////////////////////
void ACEND_UpdateNodeEdge(edict_t *self, int from, int to)
{
	int i;
	trace_t	trace;
	vec3_t	min,max;
	
	if(from == INVALID || to == INVALID || from == to)
		return; // safety

	// Try to stop impossible links!
	// If it looks higher than a jump...
	if( nodes[to].origin[2] > nodes[from].origin[2]+36)
	{
		// If we are coming from a move or jump node
		if( (nodes[from].type == NODE_MOVE) ||
			(nodes[from].type == NODE_JUMP)	)
		{
			// No if the to node is the same, it's illegal
			if( (nodes[to].type == NODE_MOVE) ||
				(nodes[to].type == NODE_JUMP)	)
			{
				// Too high - not possible!!
				return;
			}
		}
	}
	// Do not allow creation of nodes where the falling distance would kill you!
	if( (nodes[from].origin[2]) > (nodes[to].origin[2] + 180) )
		return;

/*	VectorCopy(self->mins, min);
	// If going up
//	if( (nodes[from].origin[2]) < (nodes[to].origin[2]) )
		min[2] = 0;	// Allow for steps up etc.
	VectorCopy(self->maxs, max);
	// If going down
//	if( (nodes[from].origin[2]) > (nodes[to].origin[2]) )
		max[2] = 0;	// door node linking*/
	VectorCopy( vec3_origin, min);
	VectorCopy( vec3_origin, max);

	// Now trace it - more safety stuff!
	trace = gi.trace( nodes[from].origin, min, max, nodes[to].origin, self, MASK_SOLID);

	if( trace.fraction < 1.0)
	{
		// can't do it
		return;
	}
	// Add the link
	path_table[from][to] = to;

	// Checks if the link exists and then may create a new one - RiEvEr
	for( i=0; i<MAXLINKS; i++)
	{
		if ( nodes[from].links[i].targetNode == to)
			break;
		if ( nodes[from].links[i].targetNode == INVALID)
		{
			// RiEvEr
			// William uses a time factor here, whereas I use distance
			// His is possibly more efficient
			vec3_t	v;
			float thisCost;

			VectorSubtract(nodes[from].origin, nodes[to].origin, v); // subtract first
			thisCost = VectorLength(v);
			nodes[from].links[i].targetNode = to;
			nodes[from].links[i].cost = thisCost;
			break;
		}
	}

	// Now for the self-referencing part, linear time for each link added
	for(i=0;i<numnodes;i++)
		if(path_table[i][from] != INVALID)
			if(i == to)
				path_table[i][to] = INVALID; // make sure we terminate
			else
				path_table[i][to] = path_table[i][from];
		
//	if(debug_mode)
//		debug_printf("Link %d -> %d\n", from, to);
	// RiEvEr - check for the link going back the other way
	// Reverse the input data so it works properly!
	ACEND_ReverseLink( self, to, from );
	// R
}

///////////////////////////////////////////////////////////////////////
// Remove a node edge
///////////////////////////////////////////////////////////////////////
void ACEND_RemoveNodeEdge(edict_t *self, int from, int to)
{
	int i;

	if(debug_mode) 
		debug_printf("%s: Removing Edge %d -> %d\n", self->client->pers.netname, from, to);
		
	path_table[from][to] = INVALID; // set to invalid			

	// Make sure this gets updated in our path array
	for(i=0;i<numnodes;i++)
		if(path_table[from][i] == to)
			path_table[from][i] = INVALID;
}

///////////////////////////////////////////////////////////////////////
// This function will resolve all paths that are incomplete
// usually called before saving to disk
///////////////////////////////////////////////////////////////////////
void ACEND_ResolveAllPaths()
{
	int i, from, to;
	int num=0;

//	return;	// RiEvEr - disabled since it will interfere with the optimiser

	safe_bprintf(PRINT_HIGH,"Resolving all paths...");

	for(from=0;from<numnodes;from++)
	{
		for(to=0;to<numnodes;to++)
		{
			// update unresolved paths
			// Not equal to itself, not equal to -1 and equal to the last link
			if(from != to && path_table[from][to] == to)
			{
				num++;

				// Now for the self-referencing part linear time for each link added
				for(i=0;i<numnodes;i++)
					if(path_table[i][from] != -1)
						if(i == to)
							path_table[i][to] = -1; // make sure we terminate
						else
							path_table[i][to] = path_table[i][from];
			}
		}
	}

	safe_bprintf(PRINT_MEDIUM,"done (%d updated)\n",num);
}

///////////////////////////////////////////////////////////////////////
// Save to disk file
//
// Since my compression routines are one thing I did not want to
// release, I took out the compressed format option. Most levels will
// save out to a node file around 50-200k, so compression is not really
// a big deal.
///////////////////////////////////////////////////////////////////////
void ACEND_SaveNodes()
{
	FILE *pOut;
	char filename[60];
	int i,j;
	int version;
	cvar_t	*game_dir;

	version = LTK_NODEVERSION;

	// Stop overwriting good node tables with bad!
	if( numnodes < 100)
		return;

	game_dir = gi.cvar ("game", "", 0);

	//@@ change 'nav' to 'terrain' to line up with William
#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\terrain\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".ltk");
#else
#ifndef AMIGA
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
#else
	strcpy(filename, game_dir->string);
#endif
	strcat(filename, "/terrain/");
	strcat(filename,level.mapname);
	strcat(filename,".ltk");
#endif
	
	// Resolve paths
	ACEND_ResolveAllPaths();

	safe_bprintf(PRINT_MEDIUM,"Saving node table...");

/*	strcpy(filename,"action\\nav\\");
	strcat(filename,level.mapname);
	strcat(filename,".nod");*/

	if((pOut = fopen(filename, "wb" )) == NULL)
		return; // bail
	
	fwrite(&version,sizeof(int),1,pOut); // write version
	fwrite(&numnodes,sizeof(int),1,pOut); // write count
	fwrite(&num_items,sizeof(int),1,pOut); // write facts count
	
	fwrite(nodes,sizeof(node_t),numnodes,pOut); // write nodes
	
	for(i=0;i<numnodes;i++)
		for(j=0;j<numnodes;j++)
			fwrite(&path_table[i][j],sizeof(short int),1,pOut); // write count
		
	fwrite(item_table,sizeof(item_table_t),num_items,pOut); 		// write out the fact table

	fclose(pOut);
	
	safe_bprintf(PRINT_MEDIUM,"done.\n");
}

///////////////////////////////////////////////////////////////////////
// Read from disk file
///////////////////////////////////////////////////////////////////////
void ACEND_LoadNodes(void)
{
	FILE *pIn;
	int i,j;
	char filename[60];
	int version;
	cvar_t	*game_dir;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\terrain\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".ltk");
#else
#ifndef AMIGA
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
#else
	strcpy(filename, game_dir->string);
#endif
	strcat(filename, "/terrain/");
	strcat(filename,level.mapname);
	strcat(filename,".ltk");
#endif
/*
	strcpy(filename,"action\\nav\\");
	strcat(filename,level.mapname);
	strcat(filename,".nod");*/

	if((pIn = fopen(filename, "rb" )) == NULL)
    {
		// Create item table
		safe_bprintf(PRINT_MEDIUM, "ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(false);
		safe_bprintf(PRINT_MEDIUM, "done.\n");
		return; 
	}

	// determin version
	fread(&version,sizeof(int),1,pIn); // read version
	
	if(version == LTK_NODEVERSION) 
	{
		safe_bprintf(PRINT_MEDIUM,"ACE: Loading node table...");

		fread(&numnodes,sizeof(int),1,pIn); // read count
		fread(&num_items,sizeof(int),1,pIn); // read facts count
		
		fread(nodes,sizeof(node_t),numnodes,pIn);

		for(i=0;i<numnodes;i++)
			for(j=0;j<numnodes;j++)
				fread(&path_table[i][j],sizeof(short int),1,pIn); // write count
	
		fread(item_table,sizeof(item_table_t),num_items,pIn);
		fclose(pIn);
	}
	else
	{
		// Create item table
		safe_bprintf(PRINT_MEDIUM, "ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(false);
		safe_bprintf(PRINT_MEDIUM, "done.\n");
		return; // bail
	}
	
	safe_bprintf(PRINT_MEDIUM, "done.\n");
	
	ACEIT_BuildItemNodeTable(true);

}

