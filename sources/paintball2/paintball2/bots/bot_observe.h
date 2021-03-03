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

#define MAX_PLAYERS_TO_RECORD 32
#define MAX_PLAYER_OBSERVATION_PATH_POINTS 2048
#define MAX_RECORDED_PATHS 1024

#define PMOVE_8BIT_SCALE 4 // pmove commands are scaled by this value so they can be stored in 8 bits and save memory


typedef struct {
	signed char forward;
	signed char side;
	signed char up;
	unsigned short msec;
	short angle;
} player_input_data_t;


typedef struct {
	vec3_t				start_pos; // should we use shorts instead?  What about for larger maps?
	player_input_data_t	input_data[MAX_PLAYER_OBSERVATION_PATH_POINTS]; // todo: dynamically size?
	int					current_index;
	qboolean			path_active;
	vec3_t				end_pos;
	pmove_t				last_pm;
	vec3_t				last_pos;
	float				last_waypoint_time;
	vec3_t				last_waypoint_pos;
	qboolean			was_on_ladder;
} player_observation_t;


typedef struct {
	vec3_t					start_pos;
	vec3_t					end_pos;
	float					time;
	int						bot_failures; // todo: each time a bot fails to complete this path, increment this value and discard the path if it fails too frequently
	player_input_data_t		*input_data;
	int						total_points;
} player_recorded_path_t;


typedef struct {
	int						num_paths;
	int						path_capacity;
	player_recorded_path_t	*paths;
} player_recorded_paths_t;



extern player_recorded_paths_t		g_player_paths;


