/*
Copyright (c) 2014-2015 Nathan "jitspoe" Wulf, Digital Paint

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


#include <float.h>

#include "bot_main.h"
#include "bot_debug.h"
#include "bot_manager.h"
#include "bot_observe.h"
#include "bot_files.h"
#include "bot_waypoints.h"




//typedef short shortpos_t[3]; // net positions are compressed to shorts at 1/8 unit, anyway, so might as well store waypoints at that resolution to save memory.


static int g_debug_trace_count = 0;

typedef short vec3short_t[3];


#define VEC_TO_SHORT_SCALE 8.0f // same as quake2


void Vec3ToShort3 (const vec3_t v, short *s)
{
	s[0] = (short)(v[0] * VEC_TO_SHORT_SCALE);
	s[1] = (short)(v[1] * VEC_TO_SHORT_SCALE);
	s[2] = (short)(v[2] * VEC_TO_SHORT_SCALE);
}


void Short3ToVec3 (const vec3short_t s, vec_t *v)
{
	v[0] = (float)s[0] / VEC_TO_SHORT_SCALE;
	v[1] = (float)s[1] / VEC_TO_SHORT_SCALE;
	v[2] = (float)s[2] / VEC_TO_SHORT_SCALE;
}


bot_waypoints_t g_bot_waypoints;


void PositionToColor (const vec3_t pos, float *r, float *g, float *b)
{
	int some_val = (int)pos[0] + (int)pos[1] * 2 + (int)pos[2] * 3;
	int ri, gi, bi;

	some_val *= 462346215; // abritrary number
	ri = some_val & 0xFF;
	gi = (some_val >> 8) & 0xFF;
	bi = (some_val >> 16) & 0xFF;

	*r = (float)ri / 255.0f;
	*g = (float)gi / 255.0f;
	*b = (float)bi / 255.0f;
}


void WeightToColor (int waypoint_index, float *r, float *g, float *b)
{
	*r = 1.0f;
	*b = 1.0f;
	*g = (float)g_bot_waypoints.usage_weights[waypoint_index] / 10.0f;

	if (*g > 1.0f)
		*g = 1.0f;
}


void BotInitWaypoints (void)
{
	int i, j;
	vec3_t zero = { 0.0f, 0.0f, 0.0f };

#ifdef BOT_DEBUG
	for (i = 0; i < g_bot_waypoints.num_points; ++i)
	{
		if (g_bot_waypoints.debug_ids[i] >= 0)
		{
			// draw with a time of 0 so it will get cleared out.
			DrawDebugSphere(zero, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, g_bot_waypoints.debug_ids[i]);

			for (j = 0; j < MAX_WAYPOINT_CONNECTIONS; ++j)
			{
				if (g_bot_waypoints.connections[i].debug_ids[j] >= 0)
					DrawDebugLine(zero, zero, 0.0f, 0.0f, 0.0f, 0.0f, g_bot_waypoints.connections[i].debug_ids[j]);
			}
		}
	}
#endif

	memset(&g_bot_waypoints, 0, sizeof(g_bot_waypoints));
	memset(&g_bot_waypoints.debug_ids, -1, sizeof(g_bot_waypoints.debug_ids));
	memset(&g_bot_waypoints.connections, -1, sizeof(g_bot_waypoints.connections));
}

vec3_t standing_mins = { -16, -16, -24 };
vec3_t standing_maxs = { 16, 16, 32 };
vec3_t crouching_mins = { -16, -16, -24 }; // same as standing
vec3_t crouching_maxs = { 16, 16, 4 };


// used for qsort
int WaypointDistCompareFunc (const void *data1, const void *data2)
{
	int i1 = *(int *)data1;
	int i2 = *(int *)data2;
	float dist1, dist2;

	dist1 = g_bot_waypoints.temp_dists_sq[i1];
	dist2 = g_bot_waypoints.temp_dists_sq[i2];

	return dist1 - dist2;
}


// Compute and sort the square distances from all waypoints to this one
void WaypointSortSquareDistances (int waypoint_index)
{
	// potential opt: we could hash positions to avoid going through every single waypoint.
	int i;
	vec3_t diff_vec;

	for (i = 0; i < g_bot_waypoints.num_points; ++i)
	{
		VectorSubtract(g_bot_waypoints.positions[i], g_bot_waypoints.positions[waypoint_index], diff_vec);
		g_bot_waypoints.temp_dists_sq[i] = VectorLengthSquared(diff_vec);
		g_bot_waypoints.temp_sorted_indexes[i] = i;
	}

	qsort(g_bot_waypoints.temp_sorted_indexes, g_bot_waypoints.num_points, sizeof(g_bot_waypoints.temp_sorted_indexes[0]), WaypointDistCompareFunc);
}


typedef enum {
	CLOSER_POINT_SUCCESS,
	CLOSER_POINT_COLLISION,
	CLOSER_POINT_HEIGHT,
	CLOSER_POINT_DISTANCE
} closer_point_result_t;


qboolean BotCanReachPosition (const edict_t *ent, const vec3_t pos1, const vec3_t pos2, qboolean *need_jump)
{
	trace_t trace;
	qboolean hit_something = false;
	qboolean local_need_jump = false;

	trace = bi.trace(pos1, crouching_mins, crouching_maxs, pos2, ent, MASK_PLAYERSOLID);
	++g_debug_trace_count;

	if (trace.fraction < 1.0f || trace.startsolid)
		hit_something = true;

	// todo: we MUST come up with a way to filter out players!
	//if (trace.ent && trace.ent->client)
	//	hit_something = false; // we need a better way to do this, as there could be a solid wall behind the client, but we don't want other clients blocking paths...

	if (hit_something) // try a jump node
	{
		vec3_t jump_pos;

		if (trace.startsolid) // this isn't going to get any better doing another raycast, so just bail out early
			return false;

		if (trace.plane.normal[2] < -0.8f) // hit a ceiling
			return false;

		VectorCopyAddZ(pos1, jump_pos, 56.0f);

		// Just use crouching mins/maxes here to avoid doing more traces later.  It's possible that we'll generate an impossible node connection
		// (one that requires jumping while crouched), but those seem rare enough to not waste the extra cycles, since traces/raycast are slow.
		trace = bi.trace(pos1, crouching_mins, crouching_maxs, jump_pos, ent, MASK_PLAYERSOLID);
		++g_debug_trace_count;
		VectorCopy(trace.endpos, jump_pos);
		trace = bi.trace(jump_pos, crouching_mins, crouching_maxs, pos2, ent, MASK_PLAYERSOLID);
		++g_debug_trace_count;

		if (!trace.startsolid && trace.fraction == 1.0f)
		{
			hit_something = false;
			// todo: node types (ladder, crouch, jump, etc)?
		}

		local_need_jump = true;
	}

	if (need_jump)
	{
		vec3_t diff_vec;

		VectorSubtract(pos2, pos1, diff_vec);

		// more than 45 degrees from current location
		if (diff_vec[2] * diff_vec[2] >= (diff_vec[1] * diff_vec[1] + diff_vec[0] * diff_vec[0]))
			local_need_jump = true;

		*need_jump = local_need_jump;
	}

	return !hit_something;
}


// Helper function to find the X closest points, given a squared distance
closer_point_result_t TryInsertCloserPoint (const edict_t *ent, int *closest_points, float *closest_points_dist_sq, const vec3_t pos1, const vec3_t pos2, int point_index)
{
	int closest_point_index;
	float old_dist_sq;
	float dist_sq;

	dist_sq = g_bot_waypoints.temp_dists_sq[point_index];

	if (pos1[2] - pos2[2] < -56.0f) // too high to reach
		return CLOSER_POINT_HEIGHT; // todo: ladder and water checks

	for (closest_point_index = 0; closest_point_index < MAX_WAYPOINT_CONNECTIONS; ++closest_point_index)
	{
		old_dist_sq = closest_points_dist_sq[closest_point_index];

		if (dist_sq < old_dist_sq && dist_sq < MAX_WAYPOINT_DIST_SQ)
		{
			if (BotCanReachPosition(ent, pos1, pos2, NULL)) // no collisions
			{
				int shift_index;

				// Shift all the previous points down and insert the new one.
				for (shift_index = MAX_WAYPOINT_CONNECTIONS - 1; shift_index > closest_point_index; --shift_index)
				{
					closest_points_dist_sq[shift_index] = closest_points_dist_sq[shift_index - 1];
					closest_points[shift_index] = closest_points[shift_index - 1];
				}

				closest_points_dist_sq[closest_point_index] = dist_sq;
				closest_points[closest_point_index] = point_index;
				return CLOSER_POINT_SUCCESS;
			}
			else // hit_something
			{
				return CLOSER_POINT_COLLISION;
			}
		}
	}

	return CLOSER_POINT_DISTANCE;
}

// Look at all the nearby waypoints and try to add connections to them.
void BotWaypointUpdateConnections (int waypoint_index, const edict_t *ent)
{
	int i;
	int closest_points[MAX_WAYPOINT_CONNECTIONS];
	float closest_points_dist_sq[MAX_WAYPOINT_CONNECTIONS];
	int skip_nodes[MAX_WAYPOINT_CONNECTIONS];
	int skip_count = 0;
	int old_connection_index = 0;
	int connected_node;
	int close_node_index = 0;

	// Initialize, to make sure we don't have any invalid data set from a previous use of the waypoint (in case it was replaced)
	// And populate with nodes that haven't changed, since we know we can reach them.
	for (i = 0; i < MAX_WAYPOINT_CONNECTIONS; ++i)
	{
		closest_points[i] = -1;
		closest_points_dist_sq[i] = FLT_MAX;
		connected_node = g_bot_waypoints.connections[waypoint_index].nodes[i];

		// If the positions of nodes we were previously connected to
		if (connected_node != -1 && g_bot_waypoints.last_moved_times[waypoint_index] < bots.level_time)
		{
			if (g_bot_waypoints.last_moved_times[connected_node] < bots.level_time)
			{
				vec3_t vec_diff;
				vec_t dist_sq;

				VectorSubtract(g_bot_waypoints.positions[waypoint_index], g_bot_waypoints.positions[connected_node], vec_diff);
				dist_sq = VectorLengthSquared(vec_diff);
				closest_points[close_node_index] = connected_node;
				closest_points_dist_sq[close_node_index] = dist_sq;
				skip_nodes[close_node_index] = connected_node;
				++close_node_index;
			}
		}

		g_bot_waypoints.connections[waypoint_index].nodes[i] = -1;
		g_bot_waypoints.connections[waypoint_index].weights[i] = FLT_MAX;
	}

	skip_count = close_node_index;
	WaypointSortSquareDistances(waypoint_index);

	// Loop through all the existing waypoints and find the closest positions.
	for (i = 0; i < g_bot_waypoints.num_points; ++i)
	{
		int sorted_waypoint = g_bot_waypoints.temp_sorted_indexes[i];

		if (g_bot_waypoints.temp_dists_sq[sorted_waypoint] > MAX_WAYPOINT_DIST_SQ)
			break;

		if (sorted_waypoint != waypoint_index) // ignore self
		{
			int skip_index;
			qboolean skip = false;

			// Skip the ones we were previously connected to, as those are already added in the list
			// todo: optimize - we could sort this so we don't have to check every element of the skip list against every waypoint id...
			for (skip_index = 0; skip_index < skip_count; ++skip_index)
			{
				if (skip_nodes[skip_index] == sorted_waypoint)
				{
					skip = true;
					break;
				}
			}

			if (!skip)
			{
				closer_point_result_t result;
				result = TryInsertCloserPoint(ent, closest_points, closest_points_dist_sq, g_bot_waypoints.positions[waypoint_index], g_bot_waypoints.positions[sorted_waypoint], sorted_waypoint);

				if (result == CLOSER_POINT_DISTANCE)
					break;
			}
		}
	}

	for (i = 0; i < MAX_WAYPOINT_CONNECTIONS; ++i)
	{
		int debug_id = g_bot_waypoints.connections[waypoint_index].debug_ids[i];

		g_bot_waypoints.connections[waypoint_index].nodes[i] = closest_points[i];

		if (closest_points_dist_sq[i] < 1.0f) // avoid divide by 0.
			closest_points_dist_sq[i] = 1.0f;

		g_bot_waypoints.connections[waypoint_index].weights[i] = 1.0f / Q_rsqrt(closest_points_dist_sq[i]); // just store distance for now.  We may try to convert this to a time value later.

#ifdef BOT_DEBUG
		if (g_bot_waypoints.connections[waypoint_index].nodes[i] >= 0)
		{
			float r, g, b;
			vec3_t frompos, topos;

			VectorCopy(g_bot_waypoints.positions[waypoint_index], frompos);
			VectorCopy(g_bot_waypoints.positions[g_bot_waypoints.connections[waypoint_index].nodes[i]], topos);
			frompos[2] += 5.0f;
			topos[2] -= 5.0f;
			WeightToColor(waypoint_index, &r, &g, &b);
			g_bot_waypoints.connections[waypoint_index].debug_ids[i] = DrawDebugLine(frompos, topos, r, g, b, 99999999.9f, debug_id);
		}
		else if (debug_id >= 0)
		{
			// "draw" a line with 0 time so it clears out the old debug line.
			DrawDebugLine(g_bot_waypoints.positions[waypoint_index], g_bot_waypoints.positions[waypoint_index], 1, .5, 1, 0, debug_id);
			g_bot_waypoints.connections[waypoint_index].debug_ids[i] = -1;
		}
#endif
	}
}

#define MAX_WAYPOINT_USAGE_WEIGHT 255

#define MAX_NODES_ALREADY_UPDATED_CHECK 32

void BotAddWaypointAtIndex (const edict_t *ent, int waypoint_index, const vec3_t pos, qboolean replacing, int usage_weight)
{
	int i;
	int debug_sphere_id = g_bot_waypoints.debug_ids[waypoint_index]; // This value should be initialized to -1, so a new ID will be generated when the debug sphere is drawn
	int nodes_updated[MAX_NODES_ALREADY_UPDATED_CHECK];
	int nodes_updated_count = 0;
	int debug_connection_updated_count = 0;

	if (usage_weight > MAX_WAYPOINT_USAGE_WEIGHT)
		usage_weight = MAX_WAYPOINT_USAGE_WEIGHT;

	g_bot_waypoints.usage_weights[waypoint_index] = usage_weight;

	// Only update the last moved time if the position actually changes.
	if (!VectorCompare(pos, g_bot_waypoints.positions[waypoint_index]))
	{
		VectorCopy(pos, g_bot_waypoints.positions[waypoint_index]);
		g_bot_waypoints.last_moved_times[waypoint_index] = bots.level_time;
	}

	if (waypoint_index >= g_bot_waypoints.num_points)
		g_bot_waypoints.num_points = waypoint_index + 1;

	if (replacing)
	{
		for (i = 0; i < g_bot_waypoints.num_points; ++i)
		{
			int j;

			 for (j = 0; j < MAX_WAYPOINT_CONNECTIONS; ++j)
			 {
				 if (g_bot_waypoints.connections[i].nodes[j] == waypoint_index)
				 {
					 BotWaypointUpdateConnections(i, ent);
					 ++debug_connection_updated_count;

					 if (nodes_updated_count < MAX_NODES_ALREADY_UPDATED_CHECK)
					 {
						 nodes_updated[nodes_updated_count] = i;
						 ++nodes_updated_count;
					 }
				 }
			 }
		}
	}

#ifdef BOT_DEBUG
	{
		float r, g, b;

		WeightToColor(waypoint_index, &r, &g, &b);
		g_bot_waypoints.debug_ids[waypoint_index] = DrawDebugSphere(pos, 5.0f, r, g, b, 99999.0f, debug_sphere_id);
	}
#endif

	BotWaypointUpdateConnections(waypoint_index, ent);

	// Since we just added a new waypoint, update all the ones it connects to, since they should probably connect to it as well.
	// Unless we already updated them above.
	for (i = 0; i < MAX_WAYPOINT_CONNECTIONS; ++i)
	{
		int connected_node = g_bot_waypoints.connections[waypoint_index].nodes[i];

		if (connected_node >= 0)
		{
			qboolean already_updated = false;
			int j;

			for (j = 0; j < nodes_updated_count; ++j)
			{
				if (nodes_updated[j] == connected_node)
				{
					already_updated = true;
					break;
				}
			}

			if (!already_updated)
			{
				BotWaypointUpdateConnections(connected_node, ent);
				++debug_connection_updated_count;
			}
		}
	}

	//bi.dprintf("%d node connections updated.\n", debug_connection_updated_count);
}


#define RANDOM_WAYPOINT_REMOVAL_POOL_SIZE 8 // When randomly removing a waypoint, pick between 4 at random, and see which is "best" to remove.


void BotTryAddWaypoint (const edict_t *ent, const vec3_t pos)
{
	vec3_t diff_vec;
	float dist_sq;
	int new_waypoint_index;
	int i;

	// Make sure we're more than the min distance away from existing waypoints before inserting a new one.
	// While we're looping through, might as well generate a list of the closest waypoints as well
	for (i = 0; i < g_bot_waypoints.num_points; ++i)
	{
		VectorSubtract(g_bot_waypoints.positions[i], pos, diff_vec);
		dist_sq = VectorLengthSquared(diff_vec);

		if (dist_sq < MIN_WAYPOINT_DIFF_SQ)
		{
			int waypoint_usage = g_bot_waypoints.usage_weights[i];

			BotAddWaypointAtIndex(ent, i, pos, true, waypoint_usage + 1);
			return;
		}
	}

	if (g_bot_waypoints.num_points < MAX_WAYPOINTS)
	{
		BotAddWaypointAtIndex(ent, g_bot_waypoints.num_points, pos, false, 1);
	}
	else
	{
		int waypoint_connection_count = 0;
		int best_waypoint_index = -1;
		float total_weight;
		float lowest_total_weight = FLT_MAX;
		int i, j;

		// Randomly try a few times to find a waypoint that has max connections and the lowest total weight of connections.
		// This means it's likely surrounded by nearby nodes and can be removed without affecting much
		for (i = 0; i < RANDOM_WAYPOINT_REMOVAL_POOL_SIZE; ++i)
		{
			new_waypoint_index = (int)nu_rand(MAX_WAYPOINTS);
			waypoint_connection_count = 0;
			total_weight = 0.0f;

			for (j = 0; j < MAX_WAYPOINT_CONNECTIONS; ++j)
			{
				if (g_bot_waypoints.connections[new_waypoint_index].nodes[j] >= 0)
				{
					waypoint_connection_count++;
					total_weight += g_bot_waypoints.connections[new_waypoint_index].weights[j];
				}
			}

			if (waypoint_connection_count == MAX_WAYPOINT_CONNECTIONS && total_weight < lowest_total_weight)
			{
				best_waypoint_index = new_waypoint_index;
				lowest_total_weight = total_weight;
			}
		}

		// If we didn't find a good candidate, the new_waypoint_index will just be whatever we last tried.
		if (best_waypoint_index >= 0)
			new_waypoint_index = best_waypoint_index;

		BotAddWaypointAtIndex(ent, new_waypoint_index, pos, true, 1);
	}
}

// TODO: Trace check, as the closest waypoint may be behind a wall.
int ClosestWaypointToPosition (const vec3_t pos, float *sq_dist)
{
	int i;
	float best_dist = FLT_MAX;
	float dist;
	int best_node = -1;

	for (i = 0; i < g_bot_waypoints.num_points; ++i)
	{
		dist = VectorSquareDistance(pos, g_bot_waypoints.positions[i]);

		if (dist < best_dist)
		{
			best_dist = dist;
			best_node = i;
		}
	}

	if (sq_dist)
		*sq_dist = best_dist;

	return best_node;
}


#define WAYPOINT_ADD_TIME 1.0f // if this much time has elapsed since the last time a player has added a waypoint, try adding another.
#define WAYPOINT_ADD_TIME_GLOBAL_MAX 0.3f // won't try to add waypoints from any player unless 300ms has passed (prevent overload when lots of players are present).
#define WAYPOINT_ADD_DIST 128.0f // if a player has moved at least this distance after adding the last waypoint, try adding another.
#define WAYPOINT_ADD_DIST_LADDER 50.0f // ladders need better resolution to ensure connections

#ifdef _DEBUG
extern cvar_t *bot_remove_near_waypoints;
#endif

void BotAddPotentialWaypointFromPmove (player_observation_t *observation, const edict_t *ent, const pmove_t *pm)
{
	vec3_t diff;
	float dist_sq;
	qboolean on_ladder = false;
	float waypoint_add_dist_sq = WAYPOINT_ADD_DIST * WAYPOINT_ADD_DIST;

	// This block of code does nothing.  Ignore it for now.
#ifdef _DEBUG
	if (bot_remove_near_waypoints->value)
	{
		float sq_dist;
		int node = ClosestWaypointToPosition(ent->s.origin, &sq_dist);

		if (node >= 0)
		{
			if (sq_dist < 1024.0f)
			{
				// No means for deleting waypoints currently.  Maybe we can add this later, but it's not really necessary.
				///RemoveWaypoint(node);
			}
		}

		return;
	}
#endif

	VectorSubtract(ent->s.origin, observation->last_waypoint_pos, diff);
	dist_sq = VectorLengthSquared(diff);

	// check ladder
	{
		vec3_t forward;
		vec3_t forward_spot;
		trace_t trace;

		AngleVectors(pm->viewangles, forward, NULL, NULL);
		VectorNormalize(forward);
		VectorMA(ent->s.origin, 1.0f, forward, forward_spot);
		trace = pm->trace(ent->s.origin, pm->mins, pm->maxs, forward_spot);

		if ((trace.fraction < 1.0f) && (trace.contents & CONTENTS_LADDER))
		{
			on_ladder = true;
			waypoint_add_dist_sq = WAYPOINT_ADD_DIST_LADDER * WAYPOINT_ADD_DIST_LADDER;
		}
	}

	if ((bots.level_time - bots.last_waypoint_add_time > WAYPOINT_ADD_TIME_GLOBAL_MAX) &&
		(dist_sq >= waypoint_add_dist_sq || bots.level_time - observation->last_waypoint_time >= WAYPOINT_ADD_TIME || observation->was_on_ladder && !on_ladder))
	{
		qboolean add_waypoint = false;
		vec3_t waypoint_pos;

		// If we just got off a ladder, add a waypoint at the very top
		if (observation->was_on_ladder && !on_ladder)
		{
			// on ladder last frame, so use previous origin
			add_waypoint = true;
			VectorCopy(observation->last_pos, waypoint_pos);
			waypoint_pos[2] += 16.0f; // move waypoint 16 units up to deal with small steps at the tops of ladders and ensure LOS
		}
		else if (bi.IsGroundEntityWorld(pm->groundentity) || on_ladder) // add waypoints if on the ground or on ladders
		{
			add_waypoint = true;
			VectorCopy(ent->s.origin, waypoint_pos);
		}

		if (add_waypoint)
		{
			BotTryAddWaypoint(ent, waypoint_pos); // todo: waypoint types (ladder, ground, jump, etc?)
			VectorCopy(waypoint_pos, observation->last_waypoint_pos);
			observation->last_waypoint_time = bots.level_time;
		}
	}

	if (g_debug_trace_count)
	{
		//bi.dprintf("Traces: %d\n", g_debug_trace_count);
		g_debug_trace_count = 0;
	}

	observation->was_on_ladder = on_ladder;
}

#define WAYPOINT_VERSION 1

void BotWriteWaypoints (const char *mapname)
{
	char filename[MAX_QPATH];
	FILE *fp;

	if (!mapname || !*mapname)
		return;

	Com_sprintf(filename, sizeof(filename), "botnav/%s.botnav", mapname);
	FS_CreatePath(filename);
	fp = fopen(filename, "wb");

	if (fp)
	{
		int waypoint_index;
		int version = WAYPOINT_VERSION;

		fwrite("PB2BW", sizeof("PB2BW"), 1, fp);
		fwrite(&version, sizeof(version), 1, fp);
		fwrite(&g_bot_waypoints.num_points, sizeof(g_bot_waypoints.num_points), 1, fp);

		for (waypoint_index = 0; waypoint_index < g_bot_waypoints.num_points; ++waypoint_index)
		{
			vec3short_t vs;
			Vec3ToShort3(g_bot_waypoints.positions[waypoint_index], vs);
			fwrite(vs, sizeof(vs), 1, fp);
		}

		fclose(fp);
	}
}


void BotReadWaypoints (const char *mapname)
{
	char filename[MAX_QPATH];
	FILE *fp;

	BotInitWaypoints();
	Com_sprintf(filename, sizeof(filename), "botnav/%s.botnav", mapname);
	fp = fopen(filename, "rb");

	if (fp)
	{
		char header[6];
		fread(header, sizeof(header), 1, fp);

		if (memcmp(header, "PB2BW", sizeof(header)) == 0)
		{
			int version;
			fread(&version, sizeof(version), 1, fp);

			if (version == WAYPOINT_VERSION)
			{
				int num_waypoints, waypoint_index;
				vec3short_t pos_short;
				vec3_t pos;

				fread(&num_waypoints, sizeof(num_waypoints), 1, fp);

				for (waypoint_index = 0; waypoint_index < num_waypoints; ++waypoint_index)
				{
					fread(pos_short, sizeof(pos_short), 1, fp);
					Short3ToVec3(pos_short, pos);
					BotTryAddWaypoint(NULL, pos);
				}
			}
			else
			{
				bi.dprintf("%s - Bot waypoint version differs (%d != %d).  Discarding.\n", filename, version, WAYPOINT_VERSION);
			}
		}
		else
		{
			bi.dprintf("%s - Invalid bot waypoint file or format has changed.  Discarding.\n", filename);
		}

		fclose(fp);
	}
}

