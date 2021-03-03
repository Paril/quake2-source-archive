#include "g_local.h"

distance_t		distance_table[MAX_NODES][MAX_NODES];
nodeinfo_t		node_info[MAX_NODES];
int			node_flags[MAX_NODES];
vec3_t			node_list[MAX_NODES];

counter_t Find_Nearest_Node (edict_t *self)
{
	counter_t		start_node, i;
	distance_t		dist, best_dist;
	vec3_t	dir;
	trace_t	tr;
	int		content_mask;

	if((unsigned int)self == 0xcccccccc)
	{
		return NO_NODES;
	}

	if(!self)
	{
		return NO_NODES;
	}

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW;

	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	start_node = NO_NODES;
	// search for nearest node to nodetest ent
	for (i = 0; i < node_count; i++)		
	{
		VectorSubtract (self->s.origin, node_list[i], dir);
		dist = (distance_t)VectorLengthSquared(dir);

		if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
			continue;

		//tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		if (dist < best_dist)
		{
			tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, node_list[i], self, content_mask);

			if(tr.fraction == 1)
			{
				start_node = i;
				best_dist = dist;
			}
		}
	}
	if(start_node == NO_NODES)
		return NO_NODES;
	else
		return start_node;
}

/*
===============================
spawn_node

spawns the node in the game for
debugging purposes
returns true if successful, false if not

this is only for "visual" debugging purposes.
===============================
*/
qboolean spawn_node (vec3_t org)
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
//	node->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	node->s.modelindex = gi.modelindex ("sprites/nodemk.sp2");

	gi.linkentity (node);
	
	return true;
}

/*
===============================
should_link_up

decides if node should link up
returns true if yes, false if no
===============================
*/
qboolean should_link_up (edict_t *self, vec3_t node1, vec3_t node2)
{

	float	height;
	trace_t	tr;
	vec3_t	start, end;
	float	grade, dist;
	


	height = node1[2] - node2[2];

	VectorSubtract (node1, node2, start);
	dist = VectorLength (start);

	grade = height / dist;
			
	if (dist < 400 && dist > MAX_NODE_DIST)
	{
		if (grade < -0.51 || grade > 0.51)
		{
//			gi.dprintf ("Grade failure: %f\n", grade);
			//if (self->client)
			//	safe_bprintf (PRINT_MEDIUM, "Grade failure\n");
			return false;
		}

		//if (abs(height) > 96)
		//	return false;

	}

	//if (!Q_stricmp (self->classname, "leaf"))
	//{
		if (abs (height) > 80)
			return false;
	//}


	//if (height > 56)
	//	return false;

//	VectorCopy (self->s.origin, start);
	VectorCopy (self->s.origin, start);

	VectorCopy (start, end);
	end[2] -= 32;

	// func_plat
	tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
	if (tr.ent)
	{
		//gi.dprintf ("Linking node #%d over %s\n", node_count, tr.ent->classname);

		if (!strcmp ("func_plat", tr.ent->classname))
		{
			//safe_bprintf (PRINT_MEDIUM, "linking node %d over %s\n",
			//	node_count, tr.ent->classname);

//			gi.dprintf ("Node over plat with state", node1);
			if (tr.ent->moveinfo.state == 2)//STATE_UP)
			{
//				gi.dprintf (" UP\n");
				//safe_bprintf (PRINT_MEDIUM, " UP\n");
				if (height >= 0)
				{
					//gi.dprintf ("Okay to link\n");
					//safe_bprintf (PRINT_MEDIUM, "Okay to link\n");
					return true;
				}
				else 
				{
					//gi.dprintf ("DO NOT LINK\n");
					//safe_bprintf (PRINT_MEDIUM, "DO NOT LINK\n");
					return false;
				}

			}
			else if (tr.ent->moveinfo.state == 1)//STATE_BOTTOM)			
			{
//				gi.dprintf (" BOTTOM\n");
				if (height <= 0)
				//	gi.dprintf ("Okay to link\n");
					return true;
				else
				//	gi.dprintf ("DO NOT LINK\n");
					return false;
			}
		}
	}

	return true;

}


/*
===============================
add_node_to_list

add node to node list and creates
the links for the nodes
===============================
*/
void add_node_to_list (edict_t *self, counter_t count)
{
	trace_t			tr;
	int				content_mask;
	counter_t		i;
	vec3_t			start, end, dir;
	vec3_t			mins, maxs;
	distance_t		dist;
	qboolean		node_visible = false;
//	distance_t		height;
	int				max_allowable_dist;

		//VectorSet (leaf->mins, -16, -16, -24);
		//VectorSet (leaf->maxs, 16, 16, 32);

	VectorSet (mins, -16, -16, -24);
	VectorSet (maxs, 16, 16, 32);
	if (Q_stricmp (self->classname, "leaf"))
	{
		// not a "leaf"
		max_allowable_dist = MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED;
	}
	else
		max_allowable_dist = MAX_NODE_DIST_SQUARED * 16;


	// ignore monsters and deadmonsters so that these
	// won't block view
	/*
	content_mask =	CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME |
					CONTENTS_WINDOW | CONTENTS_PLAYERCLIP |
					CONTENTS_MONSTERCLIP | CONTENTS_MONSTER |
					CONTENTS_DEADMONSTER |CONTENTS_SOLID;
	*/

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME;

	// add node to list
	
	node_list[count][0] = self->s.origin[0];
	node_list[count][1] = self->s.origin[1];
	node_list[count][2] = self->s.origin[2];
	
	/*
	node_list[count][0] = self->s.old_origin[0];
	node_list[count][1] = self->s.old_origin[1];
	node_list[count][2] = self->s.old_origin[2];
	*/

	//spawn_node (node_list[node_count]);

	//gi.dprintf ("Added node #%d\n", node_count);

	// now add node to adjacency matrix and link it to
	// all nearby nodes
	for (i = 0; i < count; i++)
	{
		if (i == count)
		{
			// this next line is not really needed
			// but for "style" it is included.  Note:
			// the value is already zero, not INFINITY
			//Amatrix[count][i] = 0;		
			distance_table[count][i] = 0;
			continue;
		}

		// all nodes start out as not visible
		node_visible = false;

		start[0] = node_list[count][0];
		start[1] = node_list[count][1];
		start[2] = node_list[count][2];// + 22;
		
		end[0] = node_list[i][0];
		end[1] = node_list[i][1];
		end[2] = node_list[i][2];// + 22;
		
		//VectorSubtract (node_list[i], node_list[node_count], dir);
		VectorSubtract (end, start, dir);
		//dist = (distance_t)VectorLength (dir);


		dist = (distance_t)VectorLengthSquared (dir);

		//gi.dprintf ("--> DIST from node#%d to %d = %f\n", node_count, i, dist);


		// too far away, don't bother to link-up
		//if (dist >(distance_t)(MAX_NODE_DIST * 1.5) )
		//	continue;

		//if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
		if (dist > max_allowable_dist)
			continue;

		//dist = (distance_t)VectorLength (dir);
		
		// check if it is visible to node
			tr = gi.trace(start, mins, maxs, end, self, content_mask);

			//tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);

			if (tr.fraction == 1)
			{
				//Amatrix[count][i] = dist;	// node is visible so link up
				//distance_table[count][i] = dist;
				node_visible = true;

				//safe_bprintf (PRINT_MEDIUM, "linking node %d to %d\n", count, i);
			//gi.dprintf ("-->linking node#%d to %d, dist =%d\n", node_count, i, dist);
			} 
			else if (tr.ent->health < 1)
			{

				// already used this node before and it was marked visible
				// no need to check for visibility again
				
				if (distance_table[count][i] < INFINITY)
					node_visible = true;
				else if (distance_table[i][count] < INFINITY)
					node_visible = true;
				

				if (!node_visible)
				{
					vec3_t corner1, corner2, raised_middle;
					trace_t start_to_corner, end_to_corner;
					trace_t start_to_middle, end_to_middle;

					// graphical representation of function

					//
					//  c1				2
					//	  +----------------
					//	  |
					//  1 |			c2
					//

					// nodes 1 and 2 can not "see" each other because of the
					// corner of the wall so do corner checking.
					// check for corner1.  If corner1 can "see" both nodes 1
					// and nodes two then nodes 1 and nodes 2 should be linked.
					// In the above example, corner2 can not see either nodes
					// but if one node is not seen then corner2 is not valid.
					// corner1 in the above example can see both nodes.


					corner1[0] = start[0];
					corner1[1] = end[1];
					corner1[2] = (start[2] + end[2])/2;
					
					
					if (gi.pointcontents (corner1) != CONTENTS_SOLID)
					{
						start_to_corner = gi.trace (start, mins, maxs, corner1, self, content_mask);
						end_to_corner = gi.trace (end, mins, maxs, corner1, self, content_mask);

						if (start_to_corner.fraction == 1)
							if (end_to_corner.fraction == 1)
						{
							node_visible = true;
							//safe_bprintf (PRINT_MEDIUM, "Corner 1 is true for %d->%d\n", count, i);

							// move the node to the corner then
							//node_list[count][0] = corner1[0];
							//node_list[count][1] = corner1[1];
							//node_list[count][2] = corner1[2];

							//start[0] = node_list[count][0];
							//start[1] = node_list[count][1];
							//start[2] = node_list[count][2];
		
							//end[0] = node_list[i][0];
							//end[1] = node_list[i][1];
							//end[2] = node_list[i][2];
		
							//VectorSubtract (end, start, dir);


	
						}
					}

					if (!node_visible)
					{
						// corner 1 check failed so check for corner 2

						corner2[0] = end[0];
						corner2[1] = start[1];
						corner2[2] = (start[2] + end[2])/2;
						
						if (gi.pointcontents (corner2) != CONTENTS_SOLID)
						{
							start_to_corner = gi.trace (start, mins, maxs, corner2, self, content_mask);
							end_to_corner = gi.trace (end, mins, maxs, corner2, self, content_mask);

							if (start_to_corner.fraction == 1)
								if (end_to_corner.fraction == 1)
							{
								node_visible = true;
							//safe_bprintf (PRINT_MEDIUM, "Corner 2 is true for %d->%d\n", count, i);

								// move the node to the corner then
								//node_list[count][0] = corner2[0];
								//node_list[count][1] = corner2[1];
								//node_list[count][2] = corner2[2];

								//start[0] = node_list[count][0];
								//start[1] = node_list[count][1];
								//start[2] = node_list[count][2];
		
								//end[0] = node_list[i][0];
								//end[1] = node_list[i][1];
								//end[2] = node_list[i][2];
		
								//VectorSubtract (end, start, dir);


							}
						}
					}

					if (!node_visible)
					{
						// if reached this point then both corner checks failed.
						// now check for stairs blocking the view

						raised_middle[0] = (start[0] + end[0])/2;
						raised_middle[1] = (start[1] + end[1])/2;
						raised_middle[2] = (start[2] + end[2])/2 + 22;

						if (gi.pointcontents (raised_middle) != CONTENTS_SOLID)
						{
							start_to_middle = gi.trace (start, vec3_origin, vec3_origin, raised_middle, self, content_mask);
							end_to_middle = gi.trace (end, vec3_origin, vec3_origin, raised_middle, self, content_mask);

							if (start_to_middle.fraction == 1)
								if (end_to_middle.fraction == 1)
							{
							//safe_bprintf (PRINT_MEDIUM, "raised middle is true for %d->%d\n", count, i);

								node_visible = true;
							}
						}

						if (node_visible)
						{
							// raised_middle check shows visiblity
							// need to verify
							
							float frac_dist1, frac_dist2, frac_dist3, frac_dist4;
							end[2] = start[2]; // make z coordinate of end to be same as start so it will hit a barrier if one exists											
							tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);

							if (tr.fraction < 1)
							{
								// barrier found now check it
								frac_dist1 = tr.fraction * 100;
								start[2] -= 8;
								end[2] = start[2];
								tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);
								frac_dist2 = tr.fraction * 100;

								start[2] += 16;
								end[2] = start[2];
								tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);
								frac_dist3 = tr.fraction * 100;

								start[2] += 4;
								end[2] = start[2];
								tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);
								frac_dist4 = tr.fraction * 100;

								if ( abs (frac_dist2 - frac_dist1) < 10)
								{
									if (abs(frac_dist3 - frac_dist1) < 10)
									{
										if (abs(frac_dist4 - frac_dist1) < 10)
										{
										// the difference in distances is less than 10%
										// could be steep wall that is a barrier
											node_visible = false;
										}
									}
								}

								if (!node_visible)
								{
									// if wall barrier found now check to see if
									// any of the frac distances is not 100

									// if any of the distances is 100, then probably
									// a high step that does not really block node
									if (frac_dist2 == 100 || frac_dist3 == 100 || frac_dist4 == 100)
										node_visible = true;

								}
							}				

						}
					}
				}

				/*
				if (self->client)
				{
					safe_bprintf (PRINT_MEDIUM, "For connecting %d to %d\n", count, i);

						if (!node_visible)
						{
							safe_bprintf (PRINT_MEDIUM, "%d and %d is NOT connected\n", count, i);
							safe_bprintf (PRINT_MEDIUM, "offending classname is %s\n", tr.ent->classname);
						}
						else
						{
							safe_bprintf (PRINT_MEDIUM, "%d and %d is connected\n", count, i);
							safe_bprintf (PRINT_MEDIUM, "allowed classname is %s\n", tr.ent->classname);
						}
				}*/


				/*

					if (self->client)
					{
						vec3_t corner1, corner2, raised_middle;
						trace_t start_to_corner, end_to_corner;
						trace_t start_to_middle, end_to_middle;
					

						
						safe_bprintf (PRINT_MEDIUM, "For connecting %d to %d\n", count, i);

						corner1[0] = start[0];
						corner1[1] = end[1];
						corner1[2] = (start[2] + end[2])/2;
						
						
						start_to_corner = gi.trace (start, self->mins, self->maxs, corner1, self, content_mask);
						end_to_corner = gi.trace (end, self->mins, self->maxs, corner1, self, content_mask);
						
						if (start_to_corner.fraction == 1 && end_to_corner.fraction == 1)
						{
							safe_bprintf (PRINT_MEDIUM, "corner 1 is true\n");
							node_visible = true;
						}							

						corner2[0] = end[0];
						corner2[1] = start[1];
						corner2[2] = (start[2] + end[2])/2;
						
						start_to_corner = gi.trace (start, self->mins, self->maxs, corner2, self, content_mask);
						end_to_corner = gi.trace (end, self->mins, self->maxs, corner2, self, content_mask);
				
						if (start_to_corner.fraction == 1 && end_to_corner.fraction == 1)
						{
							safe_bprintf (PRINT_MEDIUM, "corner 2 is true\n");
							node_visible = true;
						}							

						raised_middle[0] = (start[0] + end[0])/2;
						raised_middle[1] = (start[1] + end[1])/2;
						raised_middle[2] = (start[2] + end[2])/2 + 32;

						start_to_middle = gi.trace (start, vec3_origin, vec3_origin, raised_middle, self, content_mask);
						end_to_middle = gi.trace (end, vec3_origin, vec3_origin, raised_middle, self, content_mask);

						if (start_to_middle.fraction == 1 && end_to_middle.fraction == 1)
						{
							safe_bprintf (PRINT_MEDIUM, "raised middle is true\n");
							node_visible = true;
						}

						if (!node_visible)
						{
							safe_bprintf (PRINT_MEDIUM, "%d and %d is NOT connected\n", count, i);
							safe_bprintf (PRINT_MEDIUM, "offending classname is %s\n", tr.ent->classname);
						}
						else
						{
							safe_bprintf (PRINT_MEDIUM, "%d and %d is connected\n", count, i);
							safe_bprintf (PRINT_MEDIUM, "allowed classname is %s\n", tr.ent->classname);
						}

						


					}
					//	safe_bprintf (PRINT_MEDIUM, "Monster block ignored\n");

					*/
			}
			else
				continue;

		dist = (distance_t)VectorLength (dir);

		//height = abs(node_list[count][2] - node_list[i][2]);

		if (node_visible)
		{
			if (should_link_up (self, node_list[count], node_list[i]) )
			{

			// check if it is visible to node
			/*
			tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);

			if (tr.fraction == 1)
			{
				//Amatrix[count][i] = dist;	// node is visible so link up
				distance_table[count][i] = dist;
			//gi.dprintf ("-->linking node#%d to %d, dist =%d\n", node_count, i, dist);
			}
			else if (tr.ent)
			{
				if (tr.ent->svflags & SVF_MONSTER || tr.ent->client)
					distance_table[count][i] = dist;
			}
			//else
			//gi.dprintf ("-----> NO LINK for node#%d to %d\n", node_count, i);
			*/

			//if (node_visible)
			//{
				distance_table[count][i] = dist;
				//safe_bprintf (PRINT_MEDIUM, "---> LINKING node %d to %d at dist %d\n", count, i, dist);
			//}
			}
		//}
		//else
		//{
		//	if (self->client && node_visible)
		//		safe_bprintf (PRINT_MEDIUM, " NOT LINKING node %d to %d\n", node_count, i);
		//}


			if (should_link_up (self, node_list[i], node_list[count]) )
			{
			// check if it is visible to node
			/*
			tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);

			if (tr.fraction == 1)
			{
				//Amatrix[i][count] = dist;	// node is visible so link up
				distance_table[i][count] = dist;
				//gi.dprintf ("----->linking node#%d to %d\n", node_count, i);
			}
			else if (tr.ent)
			{
				if (tr.ent->svflags & SVF_MONSTER || tr.ent->client)
					distance_table[count][i] = dist;
			}

			//else
				//gi.dprintf ("-----> NO LINK for node#%d to %d\n", node_count, i);
		*/

			//if (node_visible)
			//{
				distance_table[i][count] = dist;
				//safe_bprintf (PRINT_MEDIUM, "---> LINKING node %d to %d at dist %d\n", i, count, dist);
			//}
			}
		}
		//else
		//{
		//	if (self->client && node_visible)
		//		safe_bprintf (PRINT_MEDIUM, " NOT LINKING node %d to %d\n", i, node_count);
		//}


	}


	// increment node counter by 1
	//node_count++;
	self->last_node_created = count;
}

/*
===============================
any_nodes_nearby

checks if there are any nearby nodes
returns true if nodes nearby, false if not
===============================
*/

counter_t any_nodes_nearby (edict_t *self)
{
	trace_t			tr;
	int				content_mask;
	counter_t			i;
	vec3_t			start, end, dir;
	distance_t		dist, best_dist;
	counter_t		nearest_node = NO_NODES;


	if (!node_count)
		return nearest_node;		// there's no nodes yet!


	// ignore monsters and deadmonsters so that these
	// won't block view
	/*
	content_mask =	CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME |
					CONTENTS_WINDOW | CONTENTS_PLAYERCLIP |
					CONTENTS_MONSTERCLIP | CONTENTS_MONSTER |
					CONTENTS_DEADMONSTER | CONTENTS_SOLID;*/

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW; 

	// self == entity that is spawning the waypoint	
	VectorCopy (self->s.old_origin, start);

	//best_dist = (distance_t)MAX_LINK_DIST;
	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	for (i = 0; i <node_count; i++)
	{
		//tr = gi.trace (node1, vec3_origin, vec3_origin, node2, self, content_mask);

		//if (node_list[i][0] == INFINITY)
		//	continue;

		end[0] = node_list[i][0];
		end[1] = node_list[i][1];
		end[2] = node_list[i][2];


		VectorSubtract (end, start, dir);
		//dist = (distance_t)VectorLength (dir);
		dist = (distance_t)VectorLengthSquared (dir);
		//if (dist > MAX_NODE_DIST)
		//if (dist > 128)
		//	continue;

		if (dist > MAX_NODE_DIST_SQUARED)
			continue;

		tr = gi.trace (start, vec3_origin, vec3_origin, end, self, content_mask);
		
		if (tr.fraction == 1)
		{
			if (dist < best_dist)
			{
				best_dist = dist;
				nearest_node = i;
			}
		}
	}

	return nearest_node;//false;
	
}

void show_nodes (void)
{
	counter_t	i;
	vec3_t		org;

	for (i = 0; i < node_count; i++)
	{
		//org = node_list[i];
		VectorCopy (node_list[i], org);
		spawn_node (org);
	}
}

void show_visible_nodes (edict_t *ent)
{
	counter_t	i;
	vec3_t		org;

	for (i = 0; i < node_count; i++)
	{
		//org = node_list[i];
		VectorCopy (node_list[i], org);
			spawn_node (org);
	}
}

/*
===============================
make_node

make nodes for pathing purposes
returns true if successful, false if not
===============================
*/
void mapping_toggle (void)
{
	if (!path_time)
	{
		safe_bprintf (PRINT_MEDIUM, "mapping turned on\n");
		path_time = level.time + 120;
	}
	else
	{
		safe_bprintf (PRINT_MEDIUM, "mapping turned off\n");
		path_time = 0;
	}

}

qboolean make_node (edict_t *self)
{

//	float	i;	
	counter_t		node;
	trace_t			tr;
	int				content_mask;
	vec3_t			dir;

	if (!(skill->value))
		return false;	// skill 0, don't use any waypoints

	if (node_count >= MAX_NODES)
	{
		//safe_bprintf (PRINT_MEDIUM, "Reach max %d, %d\n", node_count, MAX_NODES);
		return false;
	}

	if (level.time > path_time)
	{
		//safe_bprintf (PRINT_MEDIUM, "No new nodes were created in past two minutes.  Pathing turned off.\n");
		return false;
	}

	//if (!self->client)
	//	return false;


	if (path_not_time_yet > level.time)
	{
	
	  	if (self->svflags & SVF_MONSTER)
			return false;

		if (self->client)
			if (level.framenum < 4)
		return false;

	}
	  //	if (self->svflags & SVF_MONSTER)
		//	return false;

//	if (self->client)
//		return false;

//		if (!Q_stricmp (self->classname, "leaf"))
//		gi.dprintf ("leaflet\n");

		//gi.dprintf ("bug %s\n", self->classname);

	//if (!Q_stricmp (self->classname, "leaf"))
	//	gi.dprintf ("leaflet %d\n", node_count);

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW; 

	tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, node_list[self->last_node_created], self, content_mask);
	
	if (tr.fraction == 1)
	{
		// last node created is still visible
		// now check to see if it is too far away
		VectorSubtract (node_list[self->last_node_created], self->s.origin, dir);
		
		if (VectorLengthSquared(dir) < MAX_NODE_DIST_SQUARED)
		{
			// it is still too close so return
			
			//if (print_time < level.time)
			//{
				//safe_bprintf (PRINT_MEDIUM, "Last node still too close\n");
			//	print_time = level.time + 0.1;
			//}

			return false;
		}
	}
	else
	{
		// last node created is not visible
		// now check if current location will place node in wall

		vec3_t mins, maxs, end;

		VectorSet (mins, -16, -16, -16);
		VectorSet (maxs, 16, 16, 16);
		VectorCopy (self->s.origin, end);
		end[2] -= 4;
		tr = gi.trace (self->s.origin, mins, maxs, end, self, content_mask);
		
		if (tr.startsolid)
			return false;
	}
	
	if (self->client)
	{

		if (!M_CheckBottom(self))
		{
			if (!M_CheckMapBottom (self))
			{
				//safe_bprintf (PRINT_MEDIUM, "For node #%d, player not on solid ground\n", node_count);
				return false;
			}
		}
	}


//	if (!(self->client))
//		return false;
//	for (path_time = 0; path_time < 100; path_time++)
//	for (i = 0.0; i < 5; i++)
//		Dijkstra_ShortestPath (0,5);

//	return false;

	/*
	if (path_time < level.time)
	{
		safe_bprintf (PRINT_MEDIUM, "Reverse Path dist = %d\n", graph[5].dist[0]);
		safe_bprintf (PRINT_MEDIUM, "Path dist = %d\n", graph[0].dist[5]);

		path_time = level.time + 0.1;
	}*/

	


	if (node_count >= MAX_NODES)
	{
		//safe_bprintf (PRINT_MEDIUM, "Reach max %d, %d\n", node_count, MAX_NODES);
		return false;
	}

	if (!node_count)
	{
		// no nodes yet so create one
		add_node_to_list (self, node_count);
		//if (self->client)
		//	safe_bprintf (PRINT_MEDIUM, "Adding node #%d\n", node_count);
		path_time = level.time + 120;
		node_count++;
		//spawn_node (self->s.origin);
		return true;
	}
	else
	{
		node = any_nodes_nearby (self);

		if (node == NO_NODES)
		{
			add_node_to_list (self, node_count);
		//if (self->client)
		//	safe_bprintf (PRINT_MEDIUM, "Adding new node #%d\n", node_count);

			node_count++;
			path_time = level.time + 120;
			return true;
		}
		else if (node != self->last_node_created)
		{
			add_node_to_list (self, node);

//			safe_bprintf (PRINT_MEDIUM, "Using node #%d\n", node);
			return true;
		}

	}

	return false;
}


//spawns a node path tester
void nodetest_activate (edict_t *self)
{
	counter_t		start_node, end_node, i;
	distance_t		best_dist;
	int		content_mask, skin_blocked, skin_ok;
	counter_t		old_node;
	int				safeblocking;

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW;
	skin_blocked = 0xf2f2f0f0 | 0xd0d1d2d3 | 0xf3f3f1f1 | 0xdcdddedf | 0xe0e1e2e3;
	skin_ok = 0xf2f2f0f0 | 0xf2f2f0f0 | 0xf2f2f0f0;

	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	start_node = Find_Nearest_Node (self);

	if(start_node == NO_NODES)
		return;

	spawn_templaser(self->s.origin, node_list[start_node]);

	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	end_node = Find_Nearest_Node (self->enemy);

	if(end_node == NO_NODES)
		return;

	spawn_templaser(self->enemy->s.origin, node_list[end_node]);
	self->path_node = start_node;

	i=0;
//	gi.dprintf("nodecount = %i\n", node_count);
	while(i<node_count)
	{
		safeblocking = 0;

		if(self->path_node == end_node)
		{
			break;
		}
		old_node = self->path_node;
		if (graph[old_node].dist[end_node] < INFINITY)
		{
			// path already calculated before
			self->path_node = graph[old_node].last[end_node];
		}
		else if (Dijkstra_ShortestPath (old_node, end_node))
		{
			self->path_node = graph[old_node].last[end_node];
		}
		else
		{
			return;
		}
		spawn_templaser(node_list[old_node], node_list[self->path_node]);
		i++;
	}
}

void nodetest_think (edict_t *self)
{
	nodetest_activate (self);

	self->nextthink = level.time + 1;
}

void create_nodetest (edict_t *self)
{
	edict_t *ent;

	ent = G_Spawn();

	VectorCopy(self->s.origin, ent->s.origin);
	VectorCopy(self->s.angles, ent->s.angles);
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/monsters/jawa/tris.md2");
	ent->owner = self;
	ent->enemy = self;
	ent->nextthink = level.time + 1;
	ent->think = nodetest_think;
	ent->classname = "nodetest";

	gi.linkentity (ent);
}

