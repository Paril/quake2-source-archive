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

#include "bot_main.h"
#include "bot_manager.h"
#include "../game/game.h"
#include "bot_observe.h"
#include "bot_debug.h"


#define MOVE_SPEED 400 // 400 is the running speed in Quake2
#define BOT_UCMD_TIME 25 // 25ms is 40fps.  Should be enough for trick jumps


extern usercmd_t g_lastplayercmd;
extern usercmd_t g_playercmd;

// Spin and move randomly. :)
void BotDance (unsigned int botindex)
{
	botmovedata_t *movement = bots.movement + botindex;

	movement->yawspeed = 3.0f * 360.0f;
	movement->forward = (short)nu_rand(MOVE_SPEED * 2) - MOVE_SPEED;
	movement->side = (short)nu_rand(MOVE_SPEED * 2) - MOVE_SPEED;
	movement->up = (short)nu_rand(MOVE_SPEED * 2) - MOVE_SPEED;
	movement->shooting = false;
}

edict_t *g_botent = NULL;

trace_t BotTrace (const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end)
{
	return bi.trace(start, mins, maxs, end, g_botent, MASK_PLAYERSOLID);
}

#define STRAFE_PMOVE_TRIES 8
#define STRAFE_PMOVE_TIME 500 // try over .5sec
#define STRAFE_PMOVE_MSEC 20 // 50fps

void BotRandomStrafeJump (unsigned int botindex, int msec)
{
	edict_t *ent = bots.ents[botindex];
	botmovedata_t *movedata = bots.movement + botindex;
	struct gclient_s *client = ent->client;
	pmove_t pm[STRAFE_PMOVE_TRIES];
	float yawspeed[STRAFE_PMOVE_TRIES];
	pmove_t pm_in;
	int i;
	vec3_t velocity;
	float best_velocity_sq = -1;
	int best_velocity_index = 0;
	float velocity_sq;
	float yaw = ent->s.angles[YAW];
	short yaw_short = ANGLE2SHORT(yaw) - ent->client->ps.pmove.delta_angles[YAW];
/*

	movedata->time_since_last_turn += msec;

	if (movedata->time_since_last_turn >= STRAFE_PMOVE_TIME)
	{
		movedata->time_since_last_turn = 0;
	}
	else
	{
		return;
	}*/

	g_botent = ent;
	memset(&pm_in, 0, sizeof(pm_in));
	memcpy(&pm_in.s, &client->ps.pmove, sizeof(pm_in.s));
	pm_in.trace = BotTrace;
	pm_in.pointcontents = bi.pointcontents;
	VectorCopy(ent->mins, pm_in.mins);
	VectorCopy(ent->maxs, pm_in.maxs);

	for (i = 0; i < STRAFE_PMOVE_TRIES; ++i)
	{
		int pm_time;

		memcpy(&pm[i], &pm_in, sizeof(pmove_t));
		pm[i].cmd.upmove = nu_rand(1) < .5 ? 0 : 400;
		pm[i].cmd.sidemove = nu_rand(1) < .5 ? -400 : 400;
		pm[i].cmd.forwardmove = nu_rand(4) > 1 ? 400 : -400;
		pm[i].cmd.msec = STRAFE_PMOVE_MSEC;
		yawspeed[i] = nu_rand(300.0f) - nu_rand(300.0f); // +/- 200deg/sec

		for (pm_time = 0; pm_time < STRAFE_PMOVE_TIME; pm_time += STRAFE_PMOVE_MSEC)
		{
			pm[i].cmd.angles[YAW] = yaw_short + ANGLE2SHORT(yawspeed[i] * (float)pm_time / 1000.0f); // todo: angular velocity, convert to short, etc.
			bi.Pmove(&pm[i]);
		}

		VectorCopy(pm[i].s.velocity, velocity); // convert short to float
		velocity_sq = VectorLengthSquared(velocity);

		if (velocity_sq > best_velocity_sq)
		{
			best_velocity_sq = velocity_sq;
			best_velocity_index = i;
		}
	}

	//bi.ClientThink(ent, &pm[best_velocity_index].cmd);
	movedata->forward = pm[best_velocity_index].cmd.forwardmove;
	movedata->up = pm[best_velocity_index].cmd.upmove;
	movedata->side = pm[best_velocity_index].cmd.sidemove;
	movedata->yawspeed = yawspeed[best_velocity_index];
}


//#define WAYPOINT_REACHED_EPSILON_SQ		2304.0f // 48 * 48
#define WAYPOINT_REACHED_EPSILON_SQ		1024.0f // 32 * 32

void BotFollowWaypoint (unsigned int bot_index, int msec)
{
	if (bot_index < MAX_BOTS)
	{
		botmovedata_t *movement = bots.movement + bot_index;
		bot_waypoint_path_t *path = &movement->waypoint_path;

		if (path->active)
		{
			edict_t *ent = bots.ents[bot_index];
			int waypoint_index = path->nodes[path->current_node];
			float dist_sq;
			vec3_t bot_to_current_waypoint, current_to_next_waypoint;
			vec3_t vec_diff;
			vec3_t forward, right;
			float forward_dot, right_dot;
			qboolean waypoint_passed = false;
			static float test_vel_offset = 0.1f;
			float test_threshold = 20.0f;
			const vec_t *current_waypoint_pos = movement->waypoint_path.end_pos;
			const vec_t *next_waypoint_pos = movement->waypoint_path.end_pos;

			VectorSubtract(g_bot_waypoints.positions[waypoint_index], ent->s.origin, bot_to_current_waypoint);

			// Get the current waypoint position.
			if (movement->waypoint_path.current_node < movement->waypoint_path.num_points)
			{
				current_waypoint_pos = g_bot_waypoints.positions[waypoint_index];
			}

			dist_sq = VectorSquareDistance(current_waypoint_pos, ent->s.origin); // distance from bot to current target waypoint.

			// Get the next waypoint position.
			if (movement->waypoint_path.current_node < movement->waypoint_path.num_points - 1)
			{
				int next_waypoint_index = path->nodes[path->current_node + 1];

				next_waypoint_pos = g_bot_waypoints.positions[next_waypoint_index];
			}

			// To prevent bots from circling back to waypoints that were not quite touched, check if we've passed them and are moving toward the text waypoint
			VectorSubtract(next_waypoint_pos, current_waypoint_pos, current_to_next_waypoint);

			if (VectorLengthSquared(current_to_next_waypoint) > 4.0f) // 2 quake units
			{
				if (DotProduct(bot_to_current_waypoint, current_to_next_waypoint) < 0)
					waypoint_passed = true;
			}

			if (waypoint_passed || dist_sq <= WAYPOINT_REACHED_EPSILON_SQ)
			{
				if (waypoint_passed)
					DrawDebugSphere(g_bot_waypoints.positions[waypoint_index], 8.0f, 0.8f, 1.0f, 0.0f, 1.0f, -1);
				else
					DrawDebugSphere(g_bot_waypoints.positions[waypoint_index], 8.0f, 0.0f, 1.0f, 0.8f, 1.0f, -1);

				current_waypoint_pos = next_waypoint_pos;

				// Stop if we've reached our destination.
				if (path->current_node >= path->num_points - 1)
				{
					// Check this again, as we may have a final position that's different from the last waypoint position.
					dist_sq = VectorSquareDistance(next_waypoint_pos, ent->s.origin);

					if (dist_sq <= WAYPOINT_REACHED_EPSILON_SQ)
					{
						path->active = false;
						BotPathfindComplete(bot_index); // We've reached our destination -- time for a new goal?
						return;
					}
				}
				else
				{
					++path->current_node;
					waypoint_index = path->nodes[path->current_node];

					if (waypoint_index < 0 || waypoint_index > MAX_WAYPOINTS)
					{
						// This should never happen, but just in case...
						assert(0);
						path->active = false;
						return;
					}
				}
			}
			
			if (!BotCanReachPosition(ent, ent->s.origin, current_waypoint_pos, &movement->need_jump))
			{
				DrawDebugSphere(current_waypoint_pos, 9.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1);
				path->active = false;
				BotRetryGoal(bot_index);
				return;
			}

			DrawDebugSphere(current_waypoint_pos, 7.5f, 1.0f, 1.0f, 1.0f, 0.1f, -1);
			BotSetDesiredAimAnglesFromPoint(bot_index, current_waypoint_pos);

			// Don't look directly down at waypoints when jumping.  Looks stupid.
			if (movement->desired_angles[PITCH] > 15.0f)
				movement->desired_angles[PITCH] = 15.0f;

			VectorSubtract(current_waypoint_pos, ent->s.origin, vec_diff);

			// Figure out where the target is relative to our current facing
			AngleVectors(ent->s.angles, forward, right, NULL);
			forward_dot = DotProduct2(forward, vec_diff);
			right_dot = DotProduct2(right, vec_diff);

			// Default moving forward
			movement->forward = MOVE_SPEED;
			movement->side = 0;

			// If our target is significantly to the side or behind us, adjust
			if (forward_dot < 0.0f)
			{
				movement->forward = 0;

				if (forward_dot < -test_threshold)
					movement->forward = -MOVE_SPEED;
			}

			if (right_dot > test_threshold)
				movement->side = MOVE_SPEED;
			else if (right_dot < -test_threshold)
				movement->side = -MOVE_SPEED;

			// 50% chance of jumping each frame... in case we're not touching the ground or something
			if (movement->need_jump && nu_rand(1.0f) > 0.5f)
				movement->up = MOVE_SPEED;
			else
				movement->up = 0;
		}
	}
}


// Wander around with no navmesh
#define BOT_WANDER_TRACE_DISTANCE 500.0f
#define BOT_WANDER_SPIN_DIST 16.0f // 16 units from a wall - practically touching, time to turn around
#define BOT_WANDER_RAND_ANGLE_FAR 10.0f
#define BOT_WANDER_RAND_ANGLE_NEAR 100.0f
#define BOT_WANDER_TRACE_FAR 400.0f // Beyond this point, use far angles
#define BOT_WANDER_TRACE_NEAR 48.0f // Interpolate between this and far for the rand angles, use near rand angles below this value
#define WALKABLE_NORMAL_Z 0.707106f // for walkable slopes (hills/ramps)
#define BOT_WANDER_MAX_TURN_SPEED_FAR 180.0f // degrees/sec
#define BOT_WANDER_MAX_TURN_SPEED_NEAR 180.0f / 0.2f // Can spin 180 in 100ms
#define BOT_WANDER_CAST_INTERVAL 200 // cast every 200ms

void BotWanderNoNav (unsigned int botindex, int msec)
{
	botmovedata_t *movement = bots.movement + botindex;
	edict_t *ent = bots.ents[botindex];
	vec_t *pos = ent->s.origin;
	float yaw = ent->s.angles[YAW];
	float rand_angle;
	float yaw_right;
	float yaw_left;
	vec3_t vec_right;
	vec3_t vec_left;
	trace_t trace_left, trace_right;
	vec3_t mins;
	vec3_t maxs;
	float max_turn_speed = BOT_WANDER_MAX_TURN_SPEED_FAR;

	// If we're wandering but have a target to aim at, move toward it and randomly strafe left and right.
	if (movement->aim_target)
	{
		movement->forward = MOVE_SPEED;

		if (nu_rand(1.0f) < 0.1f)
		{
			float r = nu_rand(1.0f);

			if (r < 0.3f)
				movement->side = MOVE_SPEED;
			else if (r > 0.7f)
				movement->side = -MOVE_SPEED;
			else
				movement->side = 0.0f;
		}

		if (nu_rand(1.0f) < 0.05f)
			movement->up = MOVE_SPEED;
		else
			movement->up = 0;

		return;
	}

	if (movement->time_since_last_turn > BOT_WANDER_CAST_INTERVAL)
	{
		rand_angle = BOT_WANDER_RAND_ANGLE_FAR;

		if (movement->last_trace_dist < BOT_WANDER_TRACE_FAR)
		{
			float ratio = (movement->last_trace_dist - BOT_WANDER_TRACE_NEAR) / BOT_WANDER_TRACE_FAR;

			if (ratio < 0.0f)
				ratio = 0.0f;

			rand_angle = BOT_WANDER_RAND_ANGLE_FAR * ratio + BOT_WANDER_RAND_ANGLE_NEAR * (1.0f - ratio);
			max_turn_speed = BOT_WANDER_MAX_TURN_SPEED_FAR * ratio + BOT_WANDER_MAX_TURN_SPEED_NEAR * (1.0f - ratio);
		}

		yaw_right = (yaw + nu_rand(rand_angle));
		yaw_left = (yaw - nu_rand(rand_angle));

		// Cast out 9000 units in a couple random directions, and see which cast goes further.
		vec_right[0] = pos[0] + cos(DEG2RAD(yaw_right)) * BOT_WANDER_TRACE_DISTANCE;
		vec_right[1] = pos[1] + sin(DEG2RAD(yaw_right)) * BOT_WANDER_TRACE_DISTANCE;
		vec_right[2] = pos[2];
		vec_left[0] = pos[0] + cos(DEG2RAD(yaw_left)) * BOT_WANDER_TRACE_DISTANCE;
		vec_left[1] = pos[1] + sin(DEG2RAD(yaw_left)) * BOT_WANDER_TRACE_DISTANCE;
		vec_left[2] = pos[2];

		VectorCopy(ent->mins, mins);
		VectorCopy(ent->maxs, maxs);
		mins[2] += 18.0f; // Account for steps and slopes

		if (mins[2] > 0)
			mins[2] = -1;

		trace_left = bi.trace(pos, mins, maxs, vec_left, ent, MASK_PLAYERSOLID);
		trace_right = bi.trace(pos, mins, maxs, vec_right, ent, MASK_PLAYERSOLID);

		// If tracing to the left is further, go toward the left direction.  If tracing toward the right is further, go right.
		// If we hit a hill or ramp on the shorter trace, it's impossible to tell which distance is effectively longer, so carry on with what we were doing.
		if (trace_left.fraction > trace_right.fraction && trace_right.plane.normal[2] < WALKABLE_NORMAL_Z) 
		{
			float tracedist = trace_left.fraction * BOT_WANDER_TRACE_DISTANCE;

			if (tracedist < BOT_WANDER_SPIN_DIST)
				yaw_left -= 170.0f;

			movement->yawspeed = (yaw_left - yaw) * 1000.0f / (float)BOT_WANDER_CAST_INTERVAL;
			movement->last_trace_dist = tracedist;
			movement->time_since_last_turn = 0;
		}
		else if (trace_right.fraction > trace_left.fraction && trace_left.plane.normal[2] < WALKABLE_NORMAL_Z)
		{
			float tracedist = trace_right.fraction * BOT_WANDER_TRACE_DISTANCE;

			if (tracedist < BOT_WANDER_SPIN_DIST)
				yaw_right += 170.0f;

			movement->yawspeed = (yaw_right - yaw) * 1000.0f / (float)BOT_WANDER_CAST_INTERVAL;
			movement->last_trace_dist = tracedist;
			movement->time_since_last_turn = 0;
		}
	}

	if (movement->time_since_last_turn > 500)
	{
		movement->yawspeed = nu_rand(BOT_WANDER_RAND_ANGLE_FAR) - nu_rand(BOT_WANDER_RAND_ANGLE_FAR);
		movement->time_since_last_turn = 0;
	}

	if (movement->yawspeed < -max_turn_speed)
		movement->yawspeed = -max_turn_speed;

	if (movement->yawspeed > max_turn_speed)
		movement->yawspeed = max_turn_speed;

	movement->forward = MOVE_SPEED;
}

void BotWander (unsigned int bot_index, int msec)
{
	/* No longer necesssary?  Wandering is now a goal.
	botmovedata_t *movement = bots.movement + bot_index;
	edict_t *ent = bots.ents[bot_index];

	if (movement->time_til_try_path <= 0)
	{
		int random_point = (int)nu_rand(g_bot_waypoints.num_points); // todo: pick randomly select several and use the most player-popular one.

		if (AStarFindPathFromEntityToPos(ent, g_bot_waypoints.positions[random_point], &movement->waypoint_path))
		{
			if (bot_debug->value)
				bi.dprintf("Wandering to random waypoint.\n");

			movement->waypoint_path.active = true;
			BotFollowWaypoint(bot_index, msec);
			return;
		}

		movement->time_til_try_path = 1500; // If we didn't find a path, don't try to pathfind again for 1.5 seconds.
	}*/

	BotWanderNoNav(bot_index, msec);
}


// Temporary test function that just remaps the last player cmd to bot movement
void BotCopyPlayer (unsigned int botindex, int msec)
{
	botmovedata_t *movement = bots.movement + botindex;
	float playeryaw = SHORT2ANGLE(g_playercmd.angles[YAW]);
	float oldplayeryaw = SHORT2ANGLE(g_lastplayercmd.angles[YAW]);


	movement->yawspeed = (playeryaw - oldplayeryaw) * 1000.0f / ((float)msec);
	movement->forward = g_playercmd.forwardmove;
	movement->side = g_playercmd.sidemove;
	movement->up = g_playercmd.upmove;
	movement->shooting = (g_playercmd.buttons & 1) != 0;
}


// TODO: symmetrical paths if the map is mirrored?

#define MAX_DIST_TO_PLAYER_PATH 256 // 256 quake units, ~ 9m/27ft

qboolean BotMoveTowardPlayerPath (unsigned int botindex, int msec)
{
	botmovedata_t *movement = bots.movement + botindex;
	edict_t *ent = bots.ents[botindex];
	int pathindex;
	float bestdistsq = MAX_DIST_TO_PLAYER_PATH * MAX_DIST_TO_PLAYER_PATH;
	int bestpathindex = -1;
	//short yawangle_short = ANGLE2SHORT(ent->s.angles[YAW

	for (pathindex = 0; pathindex < g_player_paths.num_paths; ++pathindex)
	{
		float distsq = VectorSquareDistance(ent->s.origin, g_player_paths.paths[pathindex].start_pos);

		if (distsq < bestdistsq)
		{
			// TODO: Prefer path facing same angle... ent->s.angles[YAW]
			// TODO: check line of sight/clear walking path
			// TODO: add some randomization (and ultimately proper path finding)
			bestpathindex = pathindex;
			bestdistsq = distsq;
		}
	}

	if (bestpathindex >= 0)
	{
		vec3_t vecToPathStart;

		VectorSubtract(g_player_paths.paths[bestpathindex].start_pos, ent->s.origin, vecToPathStart);
		// todo, orient and move toward path

		// todo: if within epsilon of path start...
		//bi.dprintf("Bot picked path %d.\n", bestpathindex);
		movement->path_info.on_path = true;
		movement->path_info.path_index = bestpathindex;
		movement->path_info.index_in_path = 0;
		return true;
	}
	else
	{
		// Found no path, bot will wander aimlessly. 
		return false;
	}
}


qboolean BotFollowPlayerPaths (unsigned int botindex, int msec)
{
	botmovedata_t *movement = bots.movement + botindex;
	edict_t *ent = bots.ents[botindex];

	if (!movement->path_info.on_path && g_player_paths.num_paths > 0)
	{
		if (!BotMoveTowardPlayerPath(botindex, msec))
			return false;

		/*
		// Find a path (just pick one randomly for now)
		int path_index = (int)nu_rand(g_player_paths.num_paths);

		bi.dprintf("Bot picked path %d.\n", path_index);
		movement->path_info.on_path = true;
		movement->path_info.path_index = path_index;
		movement->path_info.index_in_path = 0;
		*/
	}

	if (movement->path_info.on_path)
	{
		int index = movement->path_info.index_in_path;
		player_recorded_path_t *path = g_player_paths.paths + movement->path_info.path_index;
		int total_points = path->total_points;
		int msec_left = msec + movement->timeleft; // Since we probably won't end up exactly on the game time's 100ms interval, account for some overlap so bots don't "speedhack"
		usercmd_t cmd;

		memset(&cmd, 0, sizeof(cmd));

		while (msec_left > 0 && index < total_points)
		{
			player_input_data_t *input = path->input_data + index;
			int input_msec = input->msec;

			// At the end of the path run the input until the end of the game frame so we don't have to deal with little left overs moving from path to path...
			if (index == total_points - 1)
			{
				input_msec = msec_left;
			}

			msec_left -= input_msec;
			cmd.angles[YAW] = input->angle - ent->client->ps.pmove.delta_angles[YAW];
			cmd.msec = input_msec;
			cmd.forwardmove = (short)(input->forward * PMOVE_8BIT_SCALE);
			cmd.sidemove = (short)(input->side * PMOVE_8BIT_SCALE);
			cmd.upmove = (short)(input->up * PMOVE_8BIT_SCALE);
			cmd.buttons = 0;
			bi.ClientThink(ent, &cmd);

			++index;
		}

		if (index >= total_points)
			movement->path_info.on_path = false;
		else
			movement->path_info.index_in_path = index;

		movement->timeleft = msec_left;

		return true;
	}
	else
	{
		// TODO: navigate toward paths
		return false;
	}
}


// Called every game frame (100ms) for each bot
void BotMove (unsigned int botindex, int msec)
{
	if (botindex < MAX_BOTS)
	{
		edict_t *ent = bots.ents[botindex];
		botmovedata_t *movement = bots.movement + botindex;

		if (ent)
		{
			qboolean move = true;

			if (bot_debug->value && skill->value < -1.0f)
			{
				movement->yawspeed = 0;
				movement->forward = 0;
				movement->side = 0;
				movement->up = 0;
				movement->shooting = false;
				move = false;
			}

			/**
			if (move && BotFollowPlayerPaths(botindex, msec))
			{
			}
			else*/
			{
				usercmd_t cmd;
				float yaw = ent->s.angles[YAW];
				float pitch = ent->s.angles[PITCH] * 3.0f;
				float ucmd_dt = BOT_UCMD_TIME / 1000.0f;

				movement->time_since_last_turn += msec;

				if (move)
				{
					if (movement->waypoint_path.active)
						BotFollowWaypoint(botindex, msec);
					else
						BotWander(botindex, msec);

					BotAimAndShoot(botindex, msec);
				}

				//BotDance(botindex);
				//BotWanderNoNav(botindex, msec);
				//BotCopyPlayer(botindex, msec);
				//BotRandomStrafeJump(botindex, msec);// return;

				// Break movement up into smaller steps so strafe jumping and such work better
				movement->timeleft += msec;

				while (movement->timeleft > 0)
				{
					movement->timeleft -= BOT_UCMD_TIME;
					yaw += movement->yawspeed * ucmd_dt;
					pitch += movement->pitchspeed * ucmd_dt;
					memset(&cmd, 0, sizeof(cmd));
					cmd.angles[YAW] = ANGLE2SHORT(yaw) - ent->client->ps.pmove.delta_angles[YAW];
					cmd.angles[PITCH] = ANGLE2SHORT(pitch) - ent->client->ps.pmove.delta_angles[PITCH];
					cmd.msec = BOT_UCMD_TIME;
					cmd.forwardmove = movement->forward;
					cmd.sidemove = movement->side;
					cmd.upmove = movement->up;
					cmd.buttons = movement->shooting ? 1 : 0;
					bi.ClientThink(ent, &cmd);
				}

				movement->last_yaw = yaw;
			}
		}

		if (movement->time_til_try_path > 0)
			movement->time_til_try_path -= msec;
	}
}


void BotUpdateMovement (int msec)
{
	int i;

	for (i = 0; i < bots.count; ++i)
	{
		BotMove(i, msec);
	}
	
	bots.time_since_last_pathfind += msec;
	g_lastplayercmd = g_playercmd;
}



void PmoveOriginToWorldOrigin (const pmove_t *pm, vec_t *origin_out)
{
	origin_out[0] = pm->s.origin[0] * 0.125f;
	origin_out[1] = pm->s.origin[1] * 0.125f;
	origin_out[2] = pm->s.origin[2] * 0.125f;
}
