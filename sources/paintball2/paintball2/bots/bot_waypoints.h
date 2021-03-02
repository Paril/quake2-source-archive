/*
Copyright (c) 2014 Nathan "jitspoe" Wulf, Digital Paint

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once

#ifndef _BOT_WAYPOINTS_H_
#define _BOT_WAYPOINTS_H_

#include "../game/q_shared.h"
#include "bot_importexport.h"

#define MAX_WAYPOINTS 2048
//#define MAX_WAYPOINTS 500 // small number to test - todo: use larger value when done

// Note: 6 seems too few, and 8 ends up being too CPU intensive with dynamic updatess, so going with 7 for this:
#define MAX_WAYPOINT_CONNECTIONS 7 // waypoint will connect with this many of the closest reachable nodes

#define MAX_WAYPOINT_DIST 256.0f
#define MAX_WAYPOINT_DIST_SQ 65536.0f
#define MIN_WAYPOINT_DIFF 32.0f
#define MIN_WAYPOINT_DIFF_SQ 1024.0f

typedef struct {
	float	weights[MAX_WAYPOINT_CONNECTIONS];
	int		nodes[MAX_WAYPOINT_CONNECTIONS];
	int		debug_ids[MAX_WAYPOINT_CONNECTIONS];
} bot_waypoint_connection_t;


typedef struct {
	vec3_t		positions[MAX_WAYPOINTS];
	bot_waypoint_connection_t connections[MAX_WAYPOINTS];
	int			usage_weights[MAX_WAYPOINTS]; // how often players touch this waypoint.
	int			debug_ids[MAX_WAYPOINTS];
	int			num_points;
	float		last_moved_times[MAX_WAYPOINTS]; // Time the waypoint has last been moved/replaced, so we can avoid recalculating some stuff.
	float		temp_dists_sq[MAX_WAYPOINTS]; // used for sorting
	int			temp_sorted_indexes[MAX_WAYPOINTS];
} bot_waypoints_t;


typedef struct {
	int			num_points;
	int			current_node;
	int			nodes[MAX_WAYPOINTS];
	vec3_t		end_pos; // position to reach after the last waypoint.
	qboolean	active;
} bot_waypoint_path_t;


extern vec3_t crouching_mins;
extern vec3_t crouching_maxs;

extern bot_waypoints_t g_bot_waypoints;

// bot_waypoints.c
qboolean BotCanReachPosition (const edict_t *ent, const vec3_t pos1, const vec3_t pos2, qboolean *need_jump);
void BotTryAddWaypoint (const edict_t *ent, const vec3_t pos);
int ClosestWaypointToPosition (const vec3_t pos, float *sq_dist);

// bot_astar.c
qboolean AStarFindPathFromNodeIndexes (int start_node, int end_node, bot_waypoint_path_t *path);
qboolean AStarFindPathFromPositions (const edict_t *ent, const vec3_t start_pos, const vec3_t end_pos, bot_waypoint_path_t *path);
qboolean AStarFindPathFromEntityToPos (const edict_t *ent, const vec3_t end_pos, bot_waypoint_path_t *path);
void AStarDebugStartPoint (vec3_t pos);
void AStarDebugEndPoint (vec3_t pos);

#endif // _BOT_WAYOINTS_H_
