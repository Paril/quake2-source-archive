#include "g_local.h"
#include "c_base.h"
#include "c_botnav.h"

nodes_t		nodes[MAX_NODES];
nodeinfo_t	nodeinfo[MAX_NODES];

qboolean Bot_CanReachSpotDirectly(edict_t *ent, vec3_t target);

/*
Init the note table system
*/
void Bot_InitNodes(void)
{
	int i, l;

	numnodes = -1;

	for (i = 0; i < MAX_NODES; i++)
	{
		nodes[i].origin[0] = 0.0;
		nodes[i].origin[1] = 0.0;
		nodes[i].origin[2] = 0.0;
		nodes[i].flag = 0;
		nodes[i].duckflag	= 0;

		for (l = 0; l < MAX_NODES ; l++)
		{
			nodes[i].dist[l] = (double)NODE_INFINITY;
		}
	}
}

qboolean Bot_FindNode(edict_t *self, float radius, int flag)
{
	int		i;
	vec3_t	dvec;

	if (flag == -1)	//find all node types
	{
		for (i = 0; i <= numnodes; i++)
		{
			dvec[0] = nodes[i].origin[0] - self->s.origin[0];
			dvec[1] = nodes[i].origin[1] - self->s.origin[1];
			dvec[2] = nodes[i].origin[2] - self->s.origin[2];

			if (VectorLength(dvec) <= radius
				&& visible2(self->s.origin, nodes[i].origin))
				return true;
		}
		return false;
	}
	else	//find special node types
	{
		for (i = 0; i <= numnodes; i++)
		{
			dvec[0] = nodes[i].origin[0] - self->s.origin[0];
			dvec[1] = nodes[i].origin[1] - self->s.origin[1];
			dvec[2] = nodes[i].origin[2] - self->s.origin[2];

			if (VectorLength(dvec) <= radius
				&& visible2(self->s.origin, nodes[i].origin)
				&& nodes[i].flag == flag)
				return true;
		}
		return false;
	}
}

int Bot_FindNodeAtEnt(vec3_t	spot)
{
	int		i, best = -1;
	vec3_t	dvec;
	vec_t	bestdist = 180, dist;

	for (i = 0; i <= numnodes; i++)
	{
		dvec[0] = nodes[i].origin[0] - spot[0];
		dvec[1] = nodes[i].origin[1] - spot[1];
		dvec[2] = nodes[i].origin[2] - spot[2];
		
		dist = VectorLength(dvec);
		if (dist < bestdist)
		{
			if (visible2(spot, nodes[i].origin))
			{
				best = i;
				bestdist = dist;
			}
		}
	}
	return best;
}

int RecalculateCurrentNode(edict_t *ent)
{
	int		dist, bestdist = 200;
	int		i, n, bestnode;
	vec3_t	dvec;

	n = ent->client->b_currentnode;
	bestnode = n;
	
	for (i = n; i >= 0;i--)
	{
		if (ent->client->b_path[i])
		{
			dvec[0] = nodes[ent->client->b_path[i]].origin[0] - ent->s.origin[0];
			dvec[1] = nodes[ent->client->b_path[i]].origin[1] - ent->s.origin[1];
			dvec[2] = nodes[ent->client->b_path[i]].origin[2] - ent->s.origin[2];

			dist = VectorLength(dvec);
			if ((dist < bestdist) && Bot_CanReachSpotDirectly(ent, nodes[ent->client->b_path[i]].origin))
			{
				bestdist = dist;
				bestnode = i;
			}
		}
	}
	return bestnode;
}

qboolean visible2 (vec3_t spot1, vec3_t spot2)
{
	trace_t	trace;
	vec3_t	mins = {0, 0, 0};
	vec3_t	maxs = {0, 0, 0};

	trace = gi.trace (spot1, mins, maxs, spot2, NULL, MASK_SOLID);
	
	if (trace.fraction == 1.0)
		return true;
	return false;
}

qboolean visible_node (vec3_t spot1, vec3_t spot2)
{
	trace_t	tr;

	tr = gi.trace (spot1, NULL, NULL, spot2, NULL, MASK_SOLID);
	
	if (tr.fraction == 1.0 || (tr.ent && (Q_stricmp(tr.ent->classname, "func_door") == 0)))
		return true;
	return false;
}

void Bot_PlaceNode(vec3_t spot, int flag, int duckflag)
{
	if (numnodes < MAX_NODES - 1)
	{
		// don't place nodes in lava/slime
		if (gi.pointcontents(spot) & (CONTENTS_LAVA | CONTENTS_SLIME))
		{
			nprintf(PRINT_HIGH, "Lava/Slime position, node NOT placed!\n");
			return;
		}

		// ok let's place that node
		numnodes++;
		VectorCopy (spot, nodes[numnodes].origin);
		nodes[numnodes].flag = flag;

		if (duckflag == 1)
			nodes[numnodes].duckflag = 1;
		else
			nodes[numnodes].duckflag = 0;

		// print a nice debug message
		if (flag == NORMAL_NODE && duckflag)
			nprintf(PRINT_HIGH, "Normal node %d placed, duckflag on!\n", numnodes);
		else if (flag == NORMAL_NODE && !duckflag)
			nprintf(PRINT_HIGH, "Normal node %d placed!\n", numnodes);
		else if (flag == INAIR_NODE)
			nprintf(PRINT_HIGH, "In-Air node %d placed!\n", numnodes);
		else if (flag == PLAT_NODE)
			nprintf(PRINT_HIGH, "Plat node %d placed!\n", numnodes);
		else if (flag == BUTTON_NODE)
			nprintf(PRINT_HIGH, "Button node %d placed!\n", numnodes);
		else if (flag == TELEPORT_NODE)
			nprintf(PRINT_HIGH, "Teleporter node %d placed!\n", numnodes);
		else if (flag == TRAIN_NODE)
			nprintf(PRINT_HIGH, "Train node %d placed!\n", numnodes);
		else if (flag == LADDER_NODE)
		{
			//edict_t	*arrow;

			nprintf(PRINT_HIGH, "Ladder node %d placed!\n", numnodes);
			
			/*arrow = G_Spawn();
			VectorCopy (spot, arrow->s.origin);
			
			arrow->movetype = MOVETYPE_NONE;
			arrow->clipmask = MASK_SHOT;
			arrow->solid = SOLID_NOT;
			arrow->s.effects = 0;
			VectorClear (arrow->mins);
			VectorClear (arrow->maxs);
			arrow->s.modelindex = gi.modelindex ("models/objects/gibs/skull/tris.md2");
			arrow->classname = "debugarrow";
			gi.linkentity (arrow);*/
		}
	}
}

void Bot_CalcNode(edict_t *self, int nindex)
{
	int			i;
	vec3_t		dvec, down, midair,addvect;
	double		dist;
	trace_t		tr;
	vec_t		height;
	vec3_t  mins = {0, 0, 0},maxs = {0, 0, 0};

	//go through all nodes
	for (i = 0; i <= numnodes; i++)
	{
		//node == currentnode so dist is 1 (FIXME: 0 better ?)
		if (i == nindex)
		{
			nodes[nindex].dist[i] = 1;
			continue;
		}
		
		//look if node is visible from current node
		if (visible2(nodes[i].origin, nodes[nindex].origin))
		{
			dvec[0] = nodes[i].origin[0] - nodes[nindex].origin[0];
			dvec[1] = nodes[i].origin[1] - nodes[nindex].origin[1];
			dvec[2] = nodes[i].origin[2] - nodes[nindex].origin[2];

			dist = (double) VectorLength(dvec);

			if (dist > 160) //check if current node is in range
				continue;

			if (gi.pointcontents(nodes[i].origin) & (CONTENTS_WATER)
				|| gi.pointcontents(nodes[nindex].origin) & (CONTENTS_WATER))
			{
				// at least one node is in water so don't check for midair position!
				
				//connect the two nodes
				nodes[nindex].dist[i] = dist;
				nodes[i].dist[nindex] = dist;

				nprintf(PRINT_HIGH, "Water route!\n");
			}
			else if (nodes[nindex].flag == LADDER_NODE
				|| nodes[i].flag == LADDER_NODE)
			{
				// at least one node is a ladder node so don't check for midair position!
				
				//connect the two nodes
				nodes[nindex].dist[i] = dist;
				nodes[i].dist[nindex] = dist;

				nprintf(PRINT_HIGH, "Ladder route!\n");
			}
			else if (nodes[nindex].flag == INAIR_NODE && nodes[i].flag == INAIR_NODE)
			{
				// both nodes are in-air nodes...check if we can connect them

				height = nodes[nindex].origin[2] - nodes[i].origin[2];
					
				if(height > 20)
				{
					//connect only one direction
					nodes[nindex].dist[i] = dist;
				}
				else if(height < - 20)
				{
					//connect only one direction
					nodes[i].dist[nindex] = dist;
				}
				
				//else connect no direction
			}
			else
			{
				//find the mid of the line between the two nodes
				VectorSubtract(nodes[i].origin, nodes[nindex].origin, addvect);
				VectorScale(addvect, 0.5, addvect);
				VectorAdd(nodes[nindex].origin, addvect, midair);

				//find a spot some units below our mid-point
				VectorCopy(midair, down);
				down[2] -= 60;

				//trace down to see if we are on ground
				tr = gi.trace(midair, mins, maxs, down, self, MASK_SOLID);

				if (tr.fraction == 1.0)	//we are not on ground -> midair
				{
					nprintf(PRINT_HIGH, "Midair route!\n");

					height = nodes[nindex].origin[2] - nodes[i].origin[2];
					
					if(height > 35)
					{
						//connect only one direction
						nodes[nindex].dist[i] = dist;
					}
					else if(height < - 35)
					{
						//connect only one direction
						nodes[i].dist[nindex] = dist;
					}
					else
					{
						//connect the two nodes
						nodes[nindex].dist[i] = dist;
						nodes[i].dist[nindex] = dist;
					}
				}
				else //we are on ground
				{
					//connect the two nodes
					nodes[nindex].dist[i] = dist;
					nodes[i].dist[nindex] = dist;
				}
			}
		}
	}
}

int Bot_ShortestPath (int source, int target)
{
	int		i, k, kNew, j;
	double	minDist;

	//clear path buffer
	for (j = 0; j < 100; j++)
		path_buffer[j] = -1;

	//direct path, source=target
	if (source == target)
	{
		path_buffer[0]	= target;
		first_pathnode	= 0;
		return VALID_PATH;
	}

	// initialize state
	for (i = 0; i < numnodes; i++)
	{
		nodeinfo[i].predecessor	= noPredecessor;
		nodeinfo[i].dist		= (double)NODE_INFINITY;
		nodeinfo[i].state		= tentative;
	}

	// initialize start position
	nodeinfo[source].dist = 0;
	nodeinfo[source].state = permanent;

	// k is working (permanent) node
	k = source;

	do
	{
		// kNew is the tentatively labeled node with smallest path size
		kNew	= NODE_INFINITY; 
		minDist = (double)NODE_INFINITY;
	
		// is there a better path from k
		for (i = 0; i < numnodes; i++)
		{
			double	nodeIdist;
			double	nodeKdist;
			double	distKI;

			distKI = nodes[k].dist[i];
			nodeIdist = nodeinfo[i].dist;
			nodeKdist = nodeinfo[k].dist;

			if ( (distKI != NODE_INFINITY) && (nodeinfo[i].state == tentative) )
			{
				if ( (nodeKdist + distKI) < nodeIdist)
				{
					nodeinfo[i].predecessor = k;
					nodeinfo[i].dist = nodeIdist = nodeKdist + distKI;
				}
			}

			if ( (nodeIdist < minDist) && (nodeinfo[i].state == tentative) )
			{
				kNew = i;
				minDist = nodeIdist;
			}
		} // end for-i

		// bail out if no path can be found

		if (kNew == NODE_INFINITY)
			return NO_PATH;

		// make that node permanent; there cannot exist a shorter path from source to k
		k = kNew;	
		nodeinfo[k].state		= permanent;
	} while (k != target);

	// copy path to output array
	i = 0; k = target;
	do
	{
		path_buffer[i++]	= k;
		k					= nodeinfo[k].predecessor;

		if (k != noPredecessor)
			first_pathnode	= i;

	} while (k != noPredecessor && i < 100);

	return VALID_PATH;
}

qboolean Bot_SaveNodes(void)
{
	int		i, j;
#ifdef  _WIN32
	int l;
#endif
	float	dist;
	FILE	*output;
	char	file[256];
	cvar_t	*game_dir;
	const char	nodetable_version[4]	= "v02\0";
	const char	nodetable_id[19]		= "CHAOSDM NODE TABLE\0";
	int		dntgvalue = dntg->value;


	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	l =  sprintf(file, ".\\");
	l += sprintf(file + l, game_dir->string);
	l += sprintf(file + l, "\\nodes\\");
	l += sprintf(file + l, level.mapname);
	l += sprintf(file + l, ".ntb");
#else
	strcpy(file, "./");
	strcat(file, game_dir->string);
	strcat(file, "/nodes/");
	strcat(file, level.mapname);
	strcat(file, ".ntb");
#endif

	output = fopen (file, "wb");

	if (!output)
		return false;

	//check1
	fwrite (nodetable_id, sizeof(const char), 19, output);
	fwrite (nodetable_version, sizeof(const char), 4, output);
	fwrite (&numnodes, sizeof(int), 1, output);

	//dynamic node table generation on/off
	fwrite (&dntgvalue, sizeof(int), 1, output);

	for (i = 0; i < numnodes; i++)
	{
		fwrite (&nodes[i].flag, sizeof(int), 1, output);
		fwrite (&nodes[i].duckflag, sizeof(int), 1, output);
		fwrite (&nodes[i].origin, sizeof(vec3_t), 1, output);

		for (j = 0; j < numnodes; j++)
		{
			dist = (float)nodes[i].dist[j];
			fwrite (&dist, sizeof(float),1, output);
		}
	}
	//check2
	fwrite (nodetable_id, sizeof(const char), 19, output);
	fwrite (nodetable_version, sizeof(const char), 4, output);
	
	Com_Printf ("%d nodes written to %s\n", numnodes, file);

	fclose (output);
	return true;

}

qboolean Bot_LoadNodes(void)
{
	int		i, j;
#ifdef  _WIN32
	int l;
#endif
	float	dist;
	FILE	*input;
	char	file[256];
	cvar_t	*game_dir;
	const char	nodetable_version[4]	= "v02\0";
	const char	nodetable_id[19]		= "CHAOSDM NODE TABLE\0";
	char	id_buffer[28], version_buffer[5];
	int		dntgvalue;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	l =  sprintf(file, ".\\");
	l += sprintf(file + l, game_dir->string);
	l += sprintf(file + l, "\\nodes\\");
	l += sprintf(file + l, level.mapname);
	l += sprintf(file + l, ".ntb");
#else
	strcpy(file, "./");
	strcat(file, game_dir->string);
	strcat(file, "/nodes/");
	strcat(file, level.mapname);
	strcat(file, ".ntb");
#endif

	input = fopen (file, "rb");

	if(!input)
		return false;

	//check 1
	fread (id_buffer, sizeof(const char), 19, input);
	fread (version_buffer, sizeof(const char), 4, input);
	fread (&numnodes, sizeof(int), 1, input);

	//dynamic node table generation on/off
	fread (&dntgvalue, sizeof(int), 1, input);

	if (dntgvalue == 1)
	{
		Com_Printf ("\nDynamic Node Table Generation ON\n");
		gi.cvar_set("dntg", "1");
	}
	else
	{
		Com_Printf ("\nDynamic Node Table Generation OFF\n");
		gi.cvar_set("dntg", "0");
	}

	if (numnodes > MAX_NODES)
	{
		numnodes = 0;
		return false;
	}

	if (!(strcmp(id_buffer, nodetable_id) == 0))
		return false;
	if (!(strcmp(version_buffer, nodetable_version) == 0))
		return false;

	for (i = 0; i < numnodes; i++)
	{
		fread (&nodes[i].flag, sizeof(int), 1, input);
		fread (&nodes[i].duckflag, sizeof(int), 1, input);
		fread (&nodes[i].origin, sizeof(vec3_t), 1, input);

		for (j = 0; j < numnodes; j++)
		{
			fread(&dist, sizeof(float), 1, input);
			nodes[i].dist[j] = (double) dist;
		}
	}

	//check 2
	fread (id_buffer, sizeof(const char), 19, input);
	fread (version_buffer, sizeof(const char), 4, input);
	
	if (!(strcmp(id_buffer, nodetable_id) == 0))
		return false;
	if (!(strcmp(version_buffer, nodetable_version) == 0))
		return false;

	fclose (input);
	Com_Printf ("%d nodes read from %s\n", numnodes, file);
	return true;
}
