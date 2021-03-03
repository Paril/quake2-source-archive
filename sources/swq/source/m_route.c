#include "g_local.h"

navigator graph[MAX_NODES];		

void display_dijkstra_path (int source, int target);

void initialize_tables (void)
{
	counter_t	i = 0, j = 0;

	gi.dprintf (".....initializing node tables\n");
	for (i = 0; i < MAX_NODES; i++)
	{
		//node_list[i][0] = INFINITY;

		/*
		node_list[i][0] = INFINITY + INFINITY;
		node_list[i][1] = INFINITY + INFINITY;
		node_list[i][2] = INFINITY + INFINITY;
		*/

		node_flags[i] = 0;

		for (j = 0; j < MAX_NODES; j++)
		{
			distance_table[i][j] = (distance_t)INFINITY;// + INFINITY;

			graph[i].dist[j] = (distance_t)INFINITY;
			graph[i].last[j] = (counter_t)kNoPredecessor;//(-1);
		}
	}

	node_count = 0;
}

qboolean Dijkstra_ShortestPath (counter_t source, counter_t target)
{
 
	counter_t		i, k, kNew;
	distance_t		minDist;
	counter_t		path[MAX_NODES];

	if (source == target)
		return false;

	// initialize state
	for (i = 0; i < node_count; i++)
	{
		node_info[i].iPredecessor	= kNoPredecessor;
		node_info[i].iDistance		= (distance_t)INFINITY;
		node_info[i].iState			= SP_tentative;
	}

	// start position, find a path from source to target
	node_info[source].iDistance = (distance_t)0;
	node_info[source].iState = SP_permanent;

	// k is working (permanent) node
	k = source;

	do
	{
		// kNew is the tentatively labeled node with smallest path size
		//kNew	= (counter_t)INFINITY; 
		kNew = (counter_t)kNoPredecessor;
		minDist = (distance_t)INFINITY;
	
		// is there a better path from k
		for (i = 0; i < node_count; i++)
		{
			distance_t	nodeIDistance;
			distance_t	nodeKDistance;
			distance_t	distanceKI;

			nodeIDistance = node_info[i].iDistance;
			nodeKDistance = node_info[k].iDistance;
			distanceKI = distance_table[k][i];

			if ( (distanceKI != INFINITY) && (node_info[i].iState == SP_tentative) )
			{
				if ( (nodeKDistance + distanceKI) < nodeIDistance)
				{
					node_info[i].iPredecessor = k;
					node_info[i].iDistance = nodeIDistance = nodeKDistance + distanceKI;
				}
			}

			if ( (nodeIDistance < minDist) && (node_info[i].iState == SP_tentative) )
			{
				kNew = i;
				minDist = nodeIDistance;
			}
		} // end for-i

		// bail out if no path can be found

		if (kNew == kNoPredecessor)		
			return false;

		// make that node permanent; there cannot exist a shorter path from source to k
		k = kNew;	
		node_info[k].iState		= SP_permanent;
	} while (k != target);

	// copy path to output array
	i = 0; k = target;

	do
	{
		path[i++]	= k;
		k			= node_info[k].iPredecessor;

	} while (k != kNoPredecessor);

	i = 0; minDist = (distance_t)0;	
	do
	{
		if (i > 0)
		{
			minDist = minDist + distance_table[path[i]][path[i-1]];
			graph[path[i]].last[target] = path[i-1];
			graph[path[i]].dist[target] = (distance_t)minDist;
		}
	} while (path[i++] != source);

	return true;
}

float show_route_time;

void find_path (edict_t *self)
{
	counter_t		start_node, end_node, i;
	distance_t		dist, best_dist;
	vec3_t	dir;
	trace_t	tr;
	int		content_mask;

	if (!self->enemy)
		return; // no enemy to go to, so don't find a path

	
	/*
	content_mask =	CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME |
					CONTENTS_WINDOW | CONTENTS_PLAYERCLIP |
					CONTENTS_MONSTERCLIP | CONTENTS_MONSTER |
					CONTENTS_DEADMONSTER;
	*/

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW;

	//best_dist = (distance_t)MAX_LINK_DIST;
	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	start_node = NO_NODES;
	// search for nearest node
	for (i = 0; i < node_count; i++)		
	{
		VectorSubtract (self->s.origin, node_list[i], dir);
		//dist = (distance_t)VectorLength (dir);
		dist = (distance_t)VectorLengthSquared(dir);

		if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
			continue;

		//tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, node_list[i], self, content_mask);

		if (dist < best_dist && tr.fraction == 1)
		{
			start_node = i;
			best_dist = dist;
		}
	}

	if (start_node == NO_NODES)
	{
		safe_bprintf (PRINT_MEDIUM, "No initial node found\n");
		return;
	}

	//best_dist = MAX_LINK_DIST;	
	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	end_node = NO_NODES;
	// search for nearest node
	for (i = 0; i < node_count; i++)		
	{
		VectorSubtract (self->enemy->s.origin, node_list[i], dir);
//		dist = (distance_t)VectorLength (dir);
		dist = (distance_t)VectorLengthSquared(dir);

		if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
			continue;

		tr = gi.trace (self->enemy->s.origin, vec3_origin, vec3_origin, node_list[i], self->enemy, content_mask);

		if (dist < best_dist && tr.fraction == 1)
		{
			end_node = i;
			best_dist = dist;
		}
	}

	//end_node = 0;

	if (end_node == NO_NODES)
	{
		safe_bprintf (PRINT_MEDIUM, "No final node found\n");
		return;
	}

//	safe_bprintf (PRINT_MEDIUM, "Executing Dijkstra\n");

	if (graph[start_node].dist[end_node] < INFINITY)
	{
		// path already calculated before
		self->path_node = graph[start_node].last[end_node];
		safe_bprintf (PRINT_MEDIUM, "Path already calculated, Node found for find path\n");
	}
	else if (Dijkstra_ShortestPath (start_node, end_node))
	{
		safe_bprintf (PRINT_MEDIUM, "Node found\n");
		self->path_node = graph[start_node].last[end_node];
		//self->next_path_node = graph[self->next_path_node].last[end_node];		

	}
	else
		self->path_node = NO_NODES;

	if (self->path_node != NO_NODES)
	{
		vec3_t v;
		VectorSubtract (node_list[self->path_node], self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
		M_ChangeYaw (self);
	}


//	safe_bprintf (PRINT_MEDIUM, "find_path Display path list: ");

	display_dijkstra_path (start_node, end_node);
}

void find_next_node (edict_t *self)
{
	counter_t		start_node, end_node, i;
	distance_t		dist, best_dist;
	vec3_t	dir;
	trace_t	tr;
	int		content_mask;

	if (!self->enemy)
	{
		//safe_bprintf (PRINT_MEDIUM, "No enemy set\n");
		return; // no enemy to go to, so don't find a path
	}


	/*
	if (self->next_path_node != NO_NODES)
	{
		// instead of calculating next node to current position of enemy
		// continue on previous path (ie. previously calculated next node)
		// result: the monster will only calculate paths for every other
		// node instead of at every node
		self->path_node = self->next_path_node;
		self->next_path_node = NO_NODES;
		return;
	}*/

	/*
	content_mask =	CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME |
					CONTENTS_WINDOW | CONTENTS_PLAYERCLIP |
					CONTENTS_MONSTERCLIP | CONTENTS_MONSTER |
					CONTENTS_DEADMONSTER;
	*/

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW;
	
	//best_dist = (distance_t)MAX_LINK_DIST;
	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;
	
	//start_node = self->last_path_node;
	
	start_node = self->path_node;

	if (start_node == NO_NODES)
	{
		//safe_bprintf (PRINT_MEDIUM, "No initial node found for next node\n");
		return;
	}

	//best_dist = MAX_LINK_DIST;	
	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	end_node = NO_NODES;
	// search for nearest node
	for (i = 0; i < node_count; i++)		
	{
		VectorSubtract (self->enemy->s.origin, node_list[i], dir);
		//dist = (distance_t)VectorLength (dir);
		dist = (distance_t)VectorLengthSquared(dir);

		if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
			continue;

		tr = gi.trace (self->enemy->s.origin, vec3_origin, vec3_origin, node_list[i], self->enemy, content_mask);

		if (dist < best_dist && tr.fraction == 1)
		{
			end_node = i;
			best_dist = dist;
		}
	}

	//end_node = 0;

	if (end_node == NO_NODES)
	{
		//safe_bprintf (PRINT_MEDIUM, "No final node found for next node\n");
		return;
	}
//	safe_bprintf (PRINT_MEDIUM, "Executing Dijkstra\n");

	if (graph[start_node].dist[end_node] < INFINITY)
	{
		// path already calculated before
		self->path_node = graph[start_node].last[end_node];
		//safe_bprintf (PRINT_MEDIUM, "Path already calculated, Node found for next node\n");
	}
	else if (Dijkstra_ShortestPath (start_node, end_node))
	{
		self->path_node = graph[start_node].last[end_node];
		//safe_bprintf (PRINT_MEDIUM, "Node found for next node\n");
	}
	else
		self->path_node = NO_NODES;

	if (self->path_node != NO_NODES)
	{
		vec3_t v;
		VectorSubtract (node_list[self->path_node], self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
		M_ChangeYaw (self);
	}

	//safe_bprintf (PRINT_MEDIUM, "find_next_node Display path list: ");

//	display_dijkstra_path (start_node, end_node);
}

int okay;
edict_t *spawn_path (vec3_t org)
{
	edict_t		*node;

	node = G_Spawn ();
	VectorCopy (org, node->s.origin);
	node->movetype = MOVETYPE_NONE;
	node->solid = SOLID_NOT;
	VectorClear (node->mins);
	VectorClear (node->maxs);
	node->think = G_FreeEdict;
	node->nextthink = level.time + 300;
	node->classname = "node";
//	node->s.modelindex = gi.modelindex ("models/monsters/boss1/tris.md2");
//	node->s.modelindex = gi.modelindex ("models/monsters/mutant/tris.md2");	
//	node->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
//	node->s.modelindex = gi.modelindex ("sprites/nodemk.sp2");
	node->s.modelindex = 1;

	gi.linkentity (node);
	
//	return true;

	return node;

}

void PlayerTrail_LaserThink (edict_t *self)
{
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
//	static	vec3_t	lmins = {-8, -8, -8};
//	static	vec3_t	lmaxs = {8, 8, 8};

	VectorCopy (self->s.origin, start);
	VectorCopy (self->enemy->s.origin, end);

	//tr = gi.trace (start, NULL, NULL, end, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	tr = gi.trace (start, NULL, NULL, end, self, 0);
	
	VectorCopy (self->enemy->s.origin, self->s.old_origin);
	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->monsterinfo.pausetime)
		self->think = G_FreeEdict;
}

void PlayerTrail_AddLaserTrail (counter_t ptr, counter_t source);

edict_t *trail_points[64];

void allocate_trail_edicts (void)
{

	int i;

	for (i = 0; i < 64; i++)
		trail_points[i] = G_Spawn ();

}

void remove_previous_trail (void)
{
//	int i;
	edict_t		*ent= g_edicts;

	for ( ; ent < &g_edicts[globals.num_edicts]; ent++)
	{
		if (!Q_stricmp (ent->classname, "trail_beam"))
		{
			ent->think = G_FreeEdict;
			ent->nextthink = level.time + 0.1;//FRAMETIME;
			//safe_bprintf (PRINT_MEDIUM, "found beam\n");
			//G_FreeEdict (ent);
		}
	}


	//for (i = 0; i < 64; i++)
	//	VectorClear (trail_points[i]->s.origin);


}
void visible_trail (int source, int target)
{

	counter_t	ptr= 0;
	counter_t	i;
	counter_t	j;
	edict_t		*beam;


	if (level.time < show_route_time)
		return;

	show_route_time = level.time + 0.2;

	remove_previous_trail ();

	if (graph[source].dist[target] >= INFINITY)
		return;

	ptr = source;

	i = 0;

	VectorCopy (node_list[source], trail_points[0]->s.origin);
	while ( ptr != target)
	{
		if (ptr < 0 || ptr>node_count)
				break;
			
		//safe_bprintf (PRINT_MEDIUM, " %d", graph[ptr].last[target]);
		i++;
		VectorCopy (node_list[graph[ptr].last[target]], trail_points[i]->s.origin);

		ptr = graph[ptr].last[target];

	}

	for (j = 0; j < i; j ++)
	{
		
		beam = G_Spawn ();
		beam = trail_points[j];
		VectorCopy (beam->s.origin, beam->s.old_origin);

		beam->movetype = MOVETYPE_NONE;
		beam->solid = SOLID_NOT;
		beam->s.renderfx |= RF_BEAM;//|RF_TRANSLUCENT;
		beam->s.modelindex = 1;			// must be non-zero
		beam->s.frame = 4;
		VectorSet (beam->mins, -8, -8, -8);
		VectorSet (beam->maxs, 8, 8, 8);

		beam->classname = "trail_beam";
		beam->owner = beam;
		beam->s.skinnum = 0xf2f2f0f0 | 0xd0d1d2d3 | 0xf3f3f1f1 | 0xdcdddedf | 0xe0e1e2e3;

		beam->enemy = trail_points[j + 1];
		beam->think = PlayerTrail_LaserThink;
		beam->nextthink = level.time + 0.1;
		beam->monsterinfo.pausetime = level.time + 20;
		gi.linkentity (beam);
	
/*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (trail_points[j]->s.origin);
		gi.WritePosition (trail_points[j+1]->s.origin);
		gi.multicast (trail_points[j]->s.origin, MULTICAST_PHS);
*/

	}
		beam = G_Spawn ();
		beam = trail_points[i];
		VectorCopy (beam->s.origin, beam->s.old_origin);

		beam->movetype = MOVETYPE_NONE;
		beam->solid = SOLID_NOT;
		beam->s.renderfx |= RF_BEAM;//|RF_TRANSLUCENT;
		beam->s.modelindex = 1;			// must be non-zero
		beam->s.frame = 4;
		VectorSet (beam->mins, -8, -8, -8);
		VectorSet (beam->maxs, 8, 8, 8);

		beam->classname = "trail_beam";
		beam->owner = beam;
		beam->s.skinnum = 0xf2f2f0f0 | 0xd0d1d2d3 | 0xf3f3f1f1 | 0xdcdddedf | 0xe0e1e2e3;

		beam->enemy = trail_points[i];
		beam->think = PlayerTrail_LaserThink;
		beam->nextthink = level.time + 0.1;
		beam->monsterinfo.pausetime = level.time + 20;
		gi.linkentity (beam);
	
	  /*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (trail_points[i]->s.origin);
		gi.WritePosition (trail_points[i]->s.origin);
		gi.multicast (trail_points[i]->s.origin, MULTICAST_PHS);
	*/

	for (j = i; j < 64; j++)
	{
		beam = G_Spawn ();
		beam = trail_points[i];
		VectorCopy (beam->s.origin, beam->s.old_origin);

		beam->movetype = MOVETYPE_NONE;
		beam->solid = SOLID_NOT;
		beam->s.renderfx |= RF_BEAM;//|RF_TRANSLUCENT;
		beam->s.modelindex = 1;			// must be non-zero
		beam->s.frame = 4;
		VectorSet (beam->mins, -8, -8, -8);
		VectorSet (beam->maxs, 8, 8, 8);

		beam->classname = "trail_beam";
		beam->owner = beam;
		beam->s.skinnum = 0xf2f2f0f0 | 0xd0d1d2d3 | 0xf3f3f1f1 | 0xdcdddedf | 0xe0e1e2e3;

		beam->enemy = trail_points[i];
		beam->think = PlayerTrail_LaserThink;
		beam->nextthink = level.time + 0.1;
		gi.linkentity (beam);
	}
	
	okay = true;
}

qboolean	show_route = false;
void show_route_toggle (void)
{

	if (show_route)
	{
		safe_bprintf (PRINT_MEDIUM, "route display off\n");
		remove_previous_trail ();
		show_route = false;
	}
	else
	{
		safe_bprintf (PRINT_MEDIUM, "route display on\n");
		show_route = true;
	}

}
void display_dijkstra_path (int source, int target)
{
	counter_t i = 0, ptr;

//	if (show_route)
//		visible_trail (source, target);

	if(1)
		return;

	// NOTE: if a path exists from a->b but matrix was not fully calculated yet,
	// this function can lead to infinite looping or accessing an element in the
	// the matrix that does not exist.  So check for bad results.

	ptr = source;
	
	if (okay)
		return;

//	safe_bprintf (PRINT_MEDIUM, "Display path list: ");

	if (graph[source].dist[target] >= INFINITY)
		safe_bprintf (PRINT_MEDIUM, " No connection exists between %d and %d\n", source, target);
	else
	{
		//printf (" Shortest Path between nodes %d and %d = %f  \n", last, first, dist);
		safe_bprintf (PRINT_MEDIUM, " %d", source);
		okay=true;
		spawn_path (node_list[source]);
	
		while (ptr != target)
		{
			if (ptr < 0 || ptr>node_count)
				break;
			
			safe_bprintf (PRINT_MEDIUM, " %d", graph[ptr].last[target]);
			//spawn_path (node_list[graph[ptr].last[target]]);
			//PlayerTrail_AddLaserTrail ((counter_t)ptr, (counter_t)target);

			ptr = graph[ptr].last[target];
		}

	}

	safe_bprintf (PRINT_MEDIUM, "\ndist to target: %d\n", graph[source].dist[target]);
	
}



void PlayerTrail_AddLaserTrail (counter_t ptr, counter_t target)
{
	edict_t *currentspot;
	edict_t *lastspot;
	edict_t *beam;
	
	lastspot = spawn_path (node_list[graph[ptr].last[target]]);
	currentspot = spawn_path (node_list[ptr]);


	beam = G_Spawn ();
	beam = currentspot;
	VectorCopy (beam->s.origin, beam->s.old_origin);

	beam->movetype = MOVETYPE_NONE;
	beam->solid = SOLID_NOT;
	beam->s.renderfx |= RF_BEAM;//|RF_TRANSLUCENT;
	beam->s.modelindex = 1;			// must be non-zero
	beam->s.frame = 4;
	VectorSet (beam->mins, -8, -8, -8);
	VectorSet (beam->maxs, 8, 8, 8);

	beam->owner = beam;
	beam->s.skinnum = 0xf2f2f0f0 | 0xd0d1d2d3 | 0xf3f3f1f1 | 0xdcdddedf | 0xe0e1e2e3;

	beam->enemy = lastspot;
	beam->think = PlayerTrail_LaserThink;
	beam->nextthink = level.time + 0.1;
	gi.linkentity (beam);

}
