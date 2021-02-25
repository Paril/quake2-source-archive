/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_debug.c
  Description: Network Navigation Engine (NNE) relaxation.

\**********************************************************/

#include "g_local.h"

#define DEBUGNODE

qboolean reacheable (map_node_t *node1, map_node_t *node2);
qboolean st_visible_vector(vec3_t self, vec3_t other);
qboolean reacheable_vector(vec3_t self, vec3_t other);
ambush_node_t *G_SpawnAmbush (void);

void spawn_ambushes()
{
   ambush_node_t *node;
   edict_t *ent = g_edicts;
	g_ambushes =  gi.TagMalloc (MAX_NODES * sizeof(g_ambushes[0]), TAG_LEVEL);
	num_ambushes = 0;
	for ( ; ent < &g_edicts[globals.num_edicts]; ent++)
	   {
		if (!ent->inuse)
			continue;
		if (ent->flipping & FLIP_AMBUSH_NODE)
         {
         node = G_SpawnAmbush();
         if (node)
            VectorCopy (ent->s.origin, node->origin);
         G_FreeEdict (ent);
         }
	   }
}

#ifdef DEBUGNODE
void spawn_node_ent (map_node_t *node1);
void draw_nodes(edict_t *ent);
void node_ent_think(edict_t *ent);
int debug_node;

// Debugging information:
void draw_nodes(edict_t *ent)
{
   int j;
   edict_t *node_ent;
     

   if (debug_node)
      return;

   debug_node = 1;
   node_ent = G_Spawn();
   VectorCopy(ent->s.origin, node_ent->s.origin);
   node_ent->delay = level.time + 10;
   node_ent->nextthink = level.time + FRAMETIME;
   node_ent->think = node_ent_think;
   gi.linkentity(node_ent);

   for (j = 0; j < num_nodes; j++)
      if (visible_vector (g_nodes[j].origin, ent->s.origin))
         spawn_node_ent (&g_nodes[j]);
}

void spawn_nodes()
{

  int i;
  int j;
  int k;
  map_node_t *node;
  edict_t *ent = g_edicts;
  FILE *stream;
  char line[512]; 

  if (deathmatch->value)
     return;

  if ((stream = fopen("Nodes.Log", "w+t" )) == NULL)
     {
     gi.error("Debug Mode: Cannot open NodeLog file\n");
     return;
     }
  gi.dprintf("NODE DEBUG: [ON]. Saved to Quake2/Nodes.Log\n");
  Ent1 = G_Spawn();
  VectorSet (Ent1->mins, -8, -8, 0);
  VectorSet (Ent1->maxs, 8, 8, 16);
   // Topaz: Initialize all nodes for this game
	g_nodes =  gi.TagMalloc (MAX_NODES * sizeof(g_nodes[0]), TAG_LEVEL);
	num_nodes = 0;

  gi.dprintf("*** NNE START ***\n");
  gi.dprintf("Deleting intermediate entities...\n");
	for ( ; ent < &g_edicts[globals.num_edicts]; ent++)
	   {
		if (!ent->inuse)
			continue;
		if (ent->flipping & FLIP_NODE)
         {
         node = G_SpawnNode();
         if (node)
            VectorCopy (ent->s.origin, node->origin);
         G_FreeEdict (ent);
         }
	   }
  sprintf (line, "== Node Navigation Engine ==\n"); 
  fputs(line, stream);
  sprintf (line, "(c) 1998 Endor Productions. All rights reserved.\n"); 
  fputs(line, stream);
  sprintf (line, "Written by Nawfel 'Topaz' Tricha.\n\n"); 
  fputs(line, stream);
  sprintf (line, "Number of nodes on the map: %i of %i\n", num_nodes, max_nodes); 
  fputs(line, stream);
  sprintf (line, "Vising nodes...\n"); 
  fputs(line, stream);

  gi.dprintf("Number of nodes: %i\n", num_nodes);
  gi.dprintf("Linking Nodes \n");
  // Check for visible nodes:
  for (j = 0; j < num_nodes; j++)
     {
     k = 0;
     for (i = 0; i < num_nodes; i++)
       {
       if (k < 16 && j != i)
          if (st_visible_vector(g_nodes[j].origin, g_nodes[i].origin)
             && reacheable(&g_nodes[i], &g_nodes[j]) 
				 /*&& reacheable_vector(g_nodes[i].origin, g_nodes[j].origin)*/)
             {
             g_nodes[j].adjacent[k++] = &g_nodes[i];
             sprintf (line, "Node %i is visible from node %i\n", j, i);
             fputs(line, stream);
             }
       }
     g_nodes[j].nodes = k;
     }

  sprintf (line, "\nCompressing node mesh...\n", j);
  fputs(line, stream);

  for (j = 0; j < num_nodes; j++)
     if (!g_nodes[j].nodes)
        {
        sprintf (line, "WARNING: Node %i @ (%.2f, %.2f, %.2f) inhibited. No connection to world\n",
                  j, g_nodes[j].origin[0], g_nodes[j].origin[1], g_nodes[j].origin[2]);
        fputs(line, stream);
        }
    
  sprintf (line, "Processing BFS on nodes for paths...\n");
  fputs(line, stream);
  for (j = 0; j < num_nodes; j++)
     {
     for (i = 0; i < num_nodes; i++)
        {
        node = pathnode(&g_nodes[j], &g_nodes[i]);
        if (!node)
           {
           g_nodes[j].route[i] = g_nodes[j].number;
           }
        else
           {
           g_nodes[j].route[i] = node->number;
           sprintf (line, "Path found from node %i to node %i through node [%i]\n",
                j, i, g_nodes[j].route[i]);
           fputs(line, stream);
           }
        }
     }   

   gi.dprintf("*** Nodes Vised ***\n");
   sprintf (line, "\nLog Out: Closing file");
   fputs(line, stream);
   fclose (stream);
   debug_node = 0;
}

void spawn_node_ent (map_node_t *node1)
{
   edict_t *node;
   node = G_Spawn();
   VectorCopy (node1->origin, node->s.origin);
   if (node1->nodes)
      node->s.modelindex = gi.modelindex ("models/debug/node/tris.md2");
   else
      node->s.modelindex = gi.modelindex ("models/items/invulner/tris.md2");   
   VectorSet(node->avelocity, 90, 90, 90);
   node->nextthink = level.time + 9;
   node->think = G_FreeEdict;
   node->movetype = MOVETYPE_FLYMISSILE;
   gi.linkentity(node);
}

void node_ent_think(edict_t *ent)
{
   int i, j;

   if (level.time > ent->delay)
      {
      debug_node = 0;
      G_FreeEdict(ent);
      return;
      }

   for (j = 0; j < num_nodes; j++)
      {
      for (i = 0; g_nodes[j].adjacent[i]; i++)
         {
         if (visible_vector (g_nodes[j].adjacent[i]->origin, ent->s.origin))
            {
            gi.WriteByte (svc_temp_entity);
	         gi.WriteByte (TE_BFG_LASER);
	         gi.WritePosition (g_nodes[j].origin);  //self->s.origin
	         gi.WritePosition (g_nodes[j].adjacent[i]->origin);
	         gi.multicast (g_nodes[j].origin, MULTICAST_PHS);
            }
         }
      }
   ent->nextthink = level.time + FRAMETIME;
}

#else

// Normal code:
void spawn_nodes()
{

  int i;
  int j;
  int k;
  map_node_t *node;
  edict_t *ent = g_edicts;

  if (deathmatch->value)
     return;

  Ent1 = G_Spawn();
  VectorSet (Ent1->mins, -8, -8, 0);
  VectorSet (Ent1->maxs, 8, 8, 16);
   // Topaz: Initialize all nodes for this game
	g_nodes =  gi.TagMalloc (MAX_NODES * sizeof(g_nodes[0]), TAG_LEVEL);
	num_nodes = 0;

  gi.dprintf("*** NNE START ===\n");
  gi.dprintf("Deleting intermediate entities...\n");
  for ( ; ent < &g_edicts[globals.num_edicts]; ent++)
	   {
		if (!ent->inuse)
			continue;
		if (ent->flipping & FLIP_NODE)
         {
         node = G_SpawnNode();
         if (node)
            VectorCopy (ent->s.origin, node->origin);
         G_FreeEdict (ent);
         }
	   }

  gi.dprintf("Number of nodes: %i\n", num_nodes);
  gi.dprintf("Linking Nodes \n");
  // Check for visible nodes:
  for (j = 0; j < num_nodes; j++)
     {
     k = 0;
     for (i = 0; i < num_nodes; i++)
       {
       if (k < 16 && j != i)
          if (st_visible_vector(g_nodes[j].origin, g_nodes[i].origin)
             && reacheable (&g_nodes[i], &g_nodes[j])
				 /*&& reacheable_vector(g_nodes[i].origin, g_nodes[j].origin)*/)
             {
             g_nodes[i].adjacent[k] = &g_nodes[j];
             k++;
             }
       }
     g_nodes[j].nodes = k;
     }

  for (j = 0; j < num_nodes; j++)
     {
     k = 0;
     for (i = 0; i < num_nodes; i++)
        {
        node = pathnode(&g_nodes[j], &g_nodes[i]);
        if (!node)
           {
           g_nodes[j].route[i] = g_nodes[j].number;
           }
        else
           {
           g_nodes[j].route[i] = node->number;
           }
        }
     }   
   gi.dprintf("*** Nodes Vised ***\n");
}

#endif

