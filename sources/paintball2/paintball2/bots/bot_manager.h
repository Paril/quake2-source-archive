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

#ifndef _BOT_MANAGER_H_
#define _BOT_MANAGER_H_

#include "bot_goals.h"
#include "bot_waypoints.h"

//typedef struct edict_s edict_t;

#define MAX_BOTS 64

typedef struct botfollowpath_s {
	qboolean		on_path; // Are we actively following a path?
	int				path_index; // Which path are we following?
	int				index_in_path; // Where are we along that path?
} botfollowpath_t;

typedef struct botmovedata_s {
	int					timeleft; // time left for ucmd's.
	float				last_trace_dist;
	float				yawspeed; // degrees/sec
	float				pitchspeed;
	float				aimtimeleft; // when aiming, how much time is left before we reach our desired aim direction (so this can be randomized and not match the frame dt exactly)
	vec3_t				desired_angles;
	short				forward; // forward/back
	short				side; // strafe left/right
	short				up; // jump/crouch
	edict_t				*aim_target;
	vec3_t				last_target_pos;
	int					last_target_msec; // time since target was last seen, in ms.
	qboolean			shooting;
	short				time_since_last_turn; // used for wandering
	short				time_til_try_path; // used for wandering
	float				last_yaw;
	float				last_pitch;
	botfollowpath_t		path_info;
	bot_waypoint_path_t	waypoint_path;
	qboolean			need_jump; // need to jump next time we're on the ground.
} botmovedata_t;


typedef struct botmanager_s {
	edict_t				*ents[MAX_BOTS];
	botmovedata_t		movement[MAX_BOTS];
	botgoal_t			goals[MAX_BOTS];
	int					count; // total number of bots currently in the map
	char				names_to_readd[MAX_BOTS][64]; // bots to readd after map change
	int					num_to_readd; // number of botss to readd
	float				level_time;
	float				last_waypoint_add_time;
	char				levelname[MAX_QPATH];
	int					time_since_last_pathfind;
} botmanager_t;

extern botmanager_t bots;

#endif // _BOT_MANAGER_H_
