/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_utils.c
  Description: General physics/utils/memory management 
               routines.

\**********************************************************/

#include "g_local.h"

qboolean CloseEnough (vec3_t spot1, vec3_t spot2);
qboolean reacheable (map_node_t *node1, map_node_t *node2);
qboolean reacheable_vector(vec3_t self, vec3_t other);
void SpawnPool(vec3_t point);

map_node_t *G_SpawnNode (void)
{
	if (num_nodes >= MAX_NODES)
      {
		gi.error ("MN_Alloc: no free map nodes");
      return NULL;
      }
		
   //memset (&g_nodes[globals.num_nodes], 0, sizeof(g_nodes[0]));   

   g_nodes[num_nodes].number = num_nodes;   
   g_nodes[num_nodes].type |= NODE_TYPE_INUSE;  
   num_nodes++;
   return &g_nodes[num_nodes - 1];
}

ambush_node_t *G_SpawnAmbush (void)
{
	if (num_ambushes >= MAX_NODES)
      {
		gi.error ("MN_Alloc: no free ambush nodes");
      return NULL;
      }
		
   //memset (&g_nodes[globals.num_nodes], 0, sizeof(g_nodes[0]));   

   g_ambushes[num_ambushes].number = num_ambushes;   
   g_ambushes[num_ambushes].type |= NODE_TYPE_INUSE;  
   num_ambushes++;
   return &g_ambushes[num_ambushes - 1];
}

map_node_t *nearnode (map_node_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_nodes;
	else
		from++;

	for ( ; from < &g_nodes[num_nodes]; from++)
	   {
		if (!visible_vector(from->origin, org))
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = org[j] - from->origin[j];
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	   }
	return NULL;
}


ambush_node_t *nearambush (ambush_node_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_ambushes;
	else
		from++;

	for ( ; from < &g_ambushes[num_ambushes]; from++)
	   {
		if (!reacheable_vector(from->origin, org))
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = org[j] - from->origin[j];
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	   }
	return NULL;
}


map_node_t *pathnode (map_node_t *node1, map_node_t *node2)
{
   int i, hops = 10000;
   map_node_t *node_head = NULL;
   max_hops = hops;
   node_head = NULL;
   if (node1 == node2)
      return node2;
   if (!node1 || !node2)
      return NULL;
	for (i = 0; i < num_nodes; i++)
		g_nodes[i].type &= ~NODE_TYPE_VISITED;

   node1->type |= NODE_TYPE_VISITED;
   for (i = 0; node1->adjacent[i]; i++)
      {
      node_hops = 1;
      if (node2 == node1->adjacent[i])
         {
         hops = 1;
         return node2;
         }
      findnodepath (node1->adjacent[i], node2); //
      if (max_hops < hops)
         {
         hops = max_hops;
         node_head = node1->adjacent[i];
         }
      } 
   node1->type &= ~NODE_TYPE_VISITED;
   if (max_hops == 10000)
      return NULL;
   return node_head;
}

/*
*** findpath ***
This function is a recursive algorithm that checks every node on the map and only
keeps track of the shortest number of hops required to reach a target. The adjacent
node which leads to the shortest path is the only thing that is needed by the 
navigate function.
*/

void findnodepath (map_node_t *node1, map_node_t *node2)
{
   int i;
   node_hops++;
   node1->type |= NODE_TYPE_VISITED;
   for (i = 0; node1->adjacent[i]; i++)
      {
      // If node is adjacent:
      if (node2 == node1->adjacent[i])
         {
         if (node_hops < max_hops)
             max_hops = node_hops;
         node_hops--;
         node1->type &= ~NODE_TYPE_VISITED;
         return;
         }
         // If node is not adjacent;
      else if (!(node1->adjacent[i]->type & NODE_TYPE_VISITED))
         {
         findnodepath (node1->adjacent[i], node2);
         }
      }
   node_hops--;
   node1->type &= ~NODE_TYPE_VISITED;
}


void SP_func_node (edict_t *self)
{
   trace_t tr;
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }
	tr = gi.trace (self->s.origin, NULL, NULL, self->s.origin, self, CONTENTS_SOLID);
   if (tr.startsolid)
      {
      gi.dprintf ("-->Node Inhibited. Node was in brush.\n");
		G_FreeEdict (self);
      return;
      }
   self->flipping |= FLIP_NODE;
}

void SP_func_ambush (edict_t *self)
{
   trace_t tr;
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }
	tr = gi.trace (self->s.origin, NULL, NULL, self->s.origin, self, CONTENTS_SOLID);
   if (tr.startsolid)
      {
      gi.dprintf ("-->Ambush Node Inhibited. Node was in brush.\n");
		G_FreeEdict (self);
      return;
      }
   self->flipping |= FLIP_AMBUSH_NODE;
}


void tell_node(edict_t *ent)
{  
   map_node_t *node = NULL;
	while ((node = nearnode(node, ent->s.origin, 420)) != NULL)
	   {
      gi.centerprintf(ent, "Near NODE %i\n", node->number);
      }
}


void bullet_create()
{
   bullet_index = 0;
}

// Leave this line commented if you don't want debug mode on.


qboolean visible_vector(vec3_t self, vec3_t other)
{
	trace_t	trace;
   //self[2] += 22;
   //other[2] += 22;
   VectorCopy (self, Ent1->s.origin);
	trace = gi.trace (self, Ent1->mins, Ent1->maxs, other, Ent1, MASK_OPAQUE);	
   //self[2] -= 22;
   //other[2] -= 22;
	if (trace.fraction == 1.0)
		return true;
	return false;
}

qboolean reacheable_vector(vec3_t self, vec3_t other)
{
	trace_t	trace;
   vec3_t ground1, ground2;
   self[2] += 22;
   other[2] += 22;
   VectorCopy (self, Ent1->s.origin);
	trace = gi.trace (self, Ent1->mins, Ent1->maxs, other, Ent1, MASK_OPAQUE);	
   self[2] -= 22;
   other[2] -= 22;
	// Check one foot off the ground.
   if (trace.fraction == 1.0)
      {
      ground1[0] = self[0];
      ground1[1] = self[1];
      ground1[2] = self[2] - 8192;
      ground2[0] = other[0];
      ground2[1] = other[1];
      ground2[2] = other[2] - 8192;
      trace = gi.trace (self, NULL, NULL, ground1, NULL, MASK_SOLID);
      ground1[0] = trace.endpos[0];
      ground1[1] = trace.endpos[1];
      ground1[2] = trace.endpos[2] + 1;
      trace = gi.trace (other, NULL, NULL, ground2, NULL, MASK_SOLID);
      ground2[0] = trace.endpos[0];
      ground2[1] = trace.endpos[1];
      ground2[2] = trace.endpos[2] + 1;
      trace = gi.trace (ground1, NULL, NULL, ground2, NULL, MASK_OPAQUE);
      if (trace.fraction == 1.0)
		   return true;
      }
	return false;
}

qboolean st_visible_vector(vec3_t self, vec3_t other)
{
	trace_t	trace;
   self[2] += 22;
   other[2] += 22;
   VectorCopy (self, Ent1->s.origin);
	trace = gi.trace (self, Ent1->mins, Ent1->maxs, other, Ent1, MASK_OPAQUE);	
   self[2] -= 22;
   other[2] -= 22;
	if (trace.fraction == 1.0)
		return true;
   else if (trace.ent->flipping == FLIP_DOOR)
      {
      //VectorSet (Ent1->mins, -24, -24, 0);
      //VectorSet (Ent1->maxs, 24, 24, 16);
      trace = gi.trace (trace.endpos, NULL, NULL, other, trace.ent, MASK_OPAQUE);
      //trace = gi.trace (trace.endpos, Ent1->mins, Ent1->maxs, other, trace.ent, MASK_OPAQUE);
      ///VectorSet (Ent1->mins, -8, -8, 0);
      //VectorSet (Ent1->maxs, 8, 8, 16);
	   if (trace.fraction == 1.0)
		   return true;
      }
	return false;
}

qboolean reacheable (map_node_t *node1, map_node_t *node2)
{
   int i;
   float range;
   trace_t trace;
   vec3_t forward, right, dist, start, end;
   
   //if (!node2 || !node1)
      //return false;

   VectorSubtract(node2->origin, node1->origin, dist);
   range = VectorLength(dist);
   range /= 4;
	VectorNormalize (dist);
   vectoangles (dist, end);
   AngleVectors (end, forward, right, NULL);
   VectorSet(dist, range, 0, 0);
   // Project to the path
   G_ProjectSource (node1->origin, dist, forward, right, start);
   for (i = 0; i < 4; i++)
      {
      VectorCopy(start, end);
      end[2] -= 72;
      trace = gi.trace (start, NULL, NULL, end, NULL, MASK_OPAQUE);
      if (trace.fraction == 1)
         return false;
      G_ProjectSource (start, dist, forward, right, start);
      }
   return true;
}

void tracegravity (map_node_t *node1, map_node_t *node2)
{
   int i;
   float range;
   trace_t trace;
   vec3_t forward, right, dist, start, end;
   
   VectorSubtract(node2->origin, node1->origin, dist);
   range = VectorLength(dist);
   range /= 4;
	VectorNormalize (dist);
   vectoangles (dist, end);
   AngleVectors (end, forward, right, NULL);
   VectorSet(dist, range, 0, 0);
   // Project to the path
   gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (node1->origin);  //self->s.origin
	gi.WritePosition (node2->origin);
	gi.multicast (node1->origin, MULTICAST_PHS);
   G_ProjectSource (node1->origin, dist, forward, right, start);
   for (i = 0; i < 4; i++)
      {
      VectorCopy(start, end);
      end[2] -= 72;
      trace = gi.trace (start, NULL, NULL, end, NULL, MASK_SHOT);
      gi.WriteByte (svc_temp_entity);
	   gi.WriteByte (TE_BFG_LASER);
	   gi.WritePosition (start);  //self->s.origin
	   gi.WritePosition (trace.endpos);
	   gi.multicast (start, MULTICAST_PHS);
      G_ProjectSource (start, dist, forward, right, start);
      }
}

void close_doors()
{
   edict_t *door = g_edicts;
	for ( ; door < &g_edicts[globals.num_edicts]; door++)
	   {
		if (!door->inuse)
			continue;
      //if (door->flipping == FLIP_DOOR)
        // {
         //door->solid == SOLID_BSP;
         //gi.setmodel (door, door->model);
         //gi.linkentity(door);
         //continue;
         //}
      if (door->flipping == FLIP_NODE_WALL)
         {
         door->svflags |= SVF_NOCLIENT;
		   door->solid = SOLID_NOT;
         G_FreeEdict(door);
         }
	   }

}

qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist)
{
	int		i;
	
	for (i=0 ; i<3 ; i++)
	{
		if (goal->absmin[i] > ent->absmax[i] + dist)
			return false;
		if (goal->absmax[i] < ent->absmin[i] - dist)
			return false;
	}
	return true;
}

void fix_slope(edict_t *ent)
{
	/*
	trace_t	trace;
	ent->pos1[0] = ent->s.origin[0];
	ent->pos1[1] = ent->s.origin[1];
	ent->pos1[2] = ent->s.origin[2] - 300;
	trace = gi.trace (ent->s.origin, NULL, NULL, ent->pos1, ent, CONTENTS_SOLID);
	if (trace.fraction < 1)
		{
		//self->s.angles[1] = 
		vectoangles(trace.plane.normal, ent->pos1);
		//gi.dprintf ("slope\n");
		ent->s.angles[2] = ent->pos1[2]; // + 90
		ent->s.angles[0] = ent->pos1[0] + 90;
		}
	*/
	if (ent->type & TYPE_MONSTER_DROPBLOOD)
		{
		if (rand() & 1)
			{
			VectorClear(ent->velocity);
			SpawnPool(ent->s.origin);
			}
		}
	ent->think = NULL;
	ent->nextthink = 0;
}
