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

#include "bot_main.h"
#include "bot_manager.h"
#include "bot_goals.h"
#include "../game/game.h"


typedef struct {
	bot_objective_type_t	objective_type;
	const edict_t			*ent;
	int						team_index;
	int						player_index; // for player-specific objectives (ex: capturing a flag can only be done by the player carrying the flag).
	qboolean				active;
} bot_objective_t;

#define MAX_BOT_OBJECTIVES 32 // don't think we'll have more than 32 flags/capture zones/etc. in any game mode.  Increase if needed.

bot_objective_t		g_bot_objectives[MAX_BOT_OBJECTIVES];
int					g_bot_num_objectives = 0;


// Called for the game for objective type game modes (flags, bases, etc).
void BotAddObjective (bot_objective_type_t objective_type, int player_index, int team_index, const edict_t *ent)
{
	int i;
	int first_free = MAX_BOT_OBJECTIVES;

	// Don't add an objective if it's already there.
	for (i = 0; i < MAX_BOT_OBJECTIVES; ++i)
	{
		bot_objective_t *objective = g_bot_objectives + i;

		if (objective->active && objective->ent == ent && objective->objective_type == objective_type && objective->team_index == team_index && objective->player_index == objective->player_index)
		{
			return;
		}
	}

	for (i = 0; i < MAX_BOT_OBJECTIVES; ++i)
	{
		bot_objective_t *objective = g_bot_objectives + i;

		if (!objective->active)
		{
			objective->active = true;
			objective->objective_type = objective_type;
			objective->player_index = player_index;
			objective->team_index = team_index;
			objective->ent = ent;
			++g_bot_num_objectives;
			return;
		}
	}

	assert(i < MAX_BOT_OBJECTIVES);
}

// Eample: when flag is picked up, the flag objective is removed.
void BotRemoveObjective (bot_objective_type_t objective_type, const edict_t *ent)
{
	int i;

	for (i = 0; i < MAX_BOT_OBJECTIVES; ++i)
	{
		bot_objective_t *objective = g_bot_objectives + i;

		if (objective->active && objective->ent == ent && objective->objective_type == objective_type)
		{
			objective->active = false;
			--g_bot_num_objectives;
		}
	}
}

// Called from the game at the start of rounds/matches so we know to get a fresh start.
void BotClearObjectives (void)
{
	memset(g_bot_objectives, 0, sizeof(g_bot_objectives)); // I love oldschool C.
	g_bot_num_objectives = 0;
}


void BotSetGoal (int bot_index, botgoaltype_t goaltype, vec3_t position)
{
	botgoal_t *goal = bots.goals + bot_index;

	goal->type = goaltype;
	goal->active = true;
	goal->changed = true;
	VectorCopy(position, goal->pos);
}


void BotRetryGoal (int bot_index)
{
	bots.goals[bot_index].changed = true;
}


void BotClearGoals (void)
{
	memset(bots.goals, 0, sizeof(bots.goals));
}


void BotPathfindComplete (int bot_index)
{
	botgoal_t *goal = bots.goals + bot_index;
	goal->active = false;

	if (bot_debug->value)
	{
		bi.dprintf("Path find complete. Deactivating goal.\n");
	}
}


void BotUpdateGoals (int msec)
{
	int bot_index;

	for (bot_index = 0; bot_index < bots.count; ++bot_index)
	{
		botgoal_t *goal = bots.goals + bot_index;

		if (goal->active)
		{
			goal->timeleft_msec -= msec;

			if (goal->timeleft_msec <= 0)
				goal->active = false;
		}

		if (!goal->active)
		{
			qboolean wander = true;

			// 20% chance of wandering for a few seconds for now.
			if (nu_rand(1) < 0.2f)
			{
				if (bot_debug->value)
					bi.dprintf("Looking for a goal.\n");

				if (g_bot_num_objectives > 0)
				{
					int random_objective_index = (int)nu_rand(g_bot_num_objectives);
					int i;
					int active_objective_index = 0;

					// some of the objectives might not be active, so we have to do this loop
					for (i = 0; i < MAX_BOT_OBJECTIVES && active_objective_index <= random_objective_index; ++i)
					{
						bot_objective_t *objective = g_bot_objectives + i;
						
						if (objective->active)
						{
							if (active_objective_index == random_objective_index)
							{
								if (bot_debug->value)
									bi.dprintf("Going with objective %d (%d).\n", active_objective_index, i);

								goal->active = true;
								goal->type = BOT_GOAL_REACH_POSITION;
								goal->changed = true;
								goal->timeleft_msec = 60000;
								VectorCopy(objective->ent->s.origin, goal->pos);
								wander = false;
								break;
							}

							++active_objective_index;
						}
					}
				}
				else
				{
					if (bot_debug->value)
						bi.dprintf("No objectives available.\n");
				}
			}

			if (wander)
			{
				int random_point = (int)nu_rand(g_bot_waypoints.num_points); // todo: pick randomly select several and use the most player-popular one.

				if (bot_debug->value)
					bi.dprintf("Wandering.\n");

				goal->active = true;
				goal->type = BOT_GOAL_WANDER;
				goal->changed = true;
				VectorCopy(g_bot_waypoints.positions[random_point], goal->pos);
				goal->timeleft_msec = 10000; // 10 seconds
			}
		}

		if (goal->changed)
		{
			botmovedata_t *movement = bots.movement + bot_index;
			edict_t *ent = bots.ents[bot_index];

			// Avoid doing a bunch of pathfinds in one update
			if (movement->time_til_try_path <= 0 && bots.time_since_last_pathfind > 0)
			{
				goal->changed = false;
				movement->waypoint_path.active = AStarFindPathFromEntityToPos(ent, goal->pos, &bots.movement[bot_index].waypoint_path);
				movement->time_til_try_path = 200 + (int)nu_rand(1000); // force some delay between path finds so bots don't spaz out and try to path find every single update.
				bots.time_since_last_pathfind = 0;

				if (!movement->waypoint_path.active)
					goal->timeleft_msec = 100 + (int)nu_rand(500); // couldn't path find to goal.  Try to find another one quickly.

				if (bot_debug->value)
				{
					bi.dprintf("Pathfinding to goal at %g, %g, %g: %s.\n", goal->pos[0], goal->pos[1], goal->pos[2], movement->waypoint_path.active ? "Success" : "Failed");
				}
			}
		}
	}
}
