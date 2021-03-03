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

// jitdebugdraw 

#include "gl_local.h"


typedef struct {
	vec3_t	start;
	vec3_t	end;
	vec3_t	color;
	float	timeleft;
} debug_line_t;


typedef struct {
	vec3_t	pos;
	float	radius;
	vec3_t	color;
	float	timeleft;
} debug_sphere_t;


typedef struct {
	int				debug_line_capacity;
	debug_line_t	*debug_line_array;
	int				debug_sphere_capacity;
	int				debug_sphere_last_free;
	debug_sphere_t	*debug_sphere_array;
	int				last_time_ms;
} debug_draw_global_t;

static debug_draw_global_t g_dd;


int GetFreeDebugLine (void)
{
	if (g_dd.debug_line_capacity)
	{
		int index;

		for (index = 0; index < g_dd.debug_line_capacity; ++index)
		{
			if (g_dd.debug_line_array[index].timeleft <= 0.0f)
				return index;
		}

		// No free element found, resize:
		{
			int old_capacity = g_dd.debug_line_capacity;
			int new_capacity = old_capacity * 2;
			debug_line_t *new_array;

			new_array = malloc(sizeof(debug_line_t) * new_capacity);
			memset(new_array + old_capacity, 0, sizeof(debug_line_t) * (new_capacity - old_capacity));
			memcpy(new_array, g_dd.debug_line_array, sizeof(debug_line_t) * g_dd.debug_line_capacity);
			free(g_dd.debug_line_array);
			g_dd.debug_line_capacity = new_capacity;
			g_dd.debug_line_array = new_array;
			return old_capacity; // ends up being the first index in the new area of the array.
		}
	}
	else
	{
		// First run, add 1 element and return 0 index.
		g_dd.debug_line_array = malloc(sizeof(debug_line_t));
		memset(g_dd.debug_line_array, 0, sizeof(debug_line_t));
		g_dd.debug_line_capacity = 1;
		return 0;
	}
}

// This really just adds them to the list to be drawn later
int Draw_DebugLine (const vec_t *start, const vec_t *end, float r, float g, float b, float time, int id)
{
	int index;
	debug_line_t *debug_line;

	if (id >= 0 && id < g_dd.debug_line_capacity)
		index = id;
	else
		index = GetFreeDebugLine();

	debug_line = g_dd.debug_line_array + index;
	VectorCopy(start, debug_line->start);
	VectorCopy(end, debug_line->end);
	debug_line->color[0] = r;
	debug_line->color[1] = g;
	debug_line->color[2] = b;
	debug_line->timeleft = time;

	return index;
}


int GetFreeDebugSphere (void)
{
	if (g_dd.debug_sphere_capacity)
	{
		int index;

		for (index = g_dd.debug_sphere_last_free; index < g_dd.debug_sphere_capacity; ++index)
		{
			if (g_dd.debug_sphere_array[index].timeleft <= 0.0f)
				return index;
		}

		for (index = 0; index < g_dd.debug_sphere_last_free; ++index)
		{
			if (g_dd.debug_sphere_array[index].timeleft <= 0.0f)
				return index;
		}

		// No free element found, resize:
		{
			int old_capacity = g_dd.debug_sphere_capacity;
			int new_capacity = old_capacity * 2;
			debug_sphere_t *new_array;

			new_array = malloc(sizeof(debug_sphere_t) * new_capacity);
			memset(new_array + old_capacity, 0, sizeof(debug_sphere_t) * (new_capacity - old_capacity));
			memcpy(new_array, g_dd.debug_sphere_array, sizeof(debug_sphere_t) * g_dd.debug_sphere_capacity);
			free(g_dd.debug_sphere_array);
			g_dd.debug_sphere_capacity = new_capacity;
			g_dd.debug_sphere_array = new_array;
			return old_capacity; // ends up being the first index in the new area of the array.
		}
	}
	else
	{
		// First run, add 1 element and return 0 index.
		g_dd.debug_sphere_array = malloc(sizeof(debug_sphere_t));
		memset(g_dd.debug_sphere_array, 0, sizeof(debug_sphere_t));
		g_dd.debug_sphere_capacity = 1;
		return 0;
	}
}

// This really just adds them to the list to be drawn later
int Draw_DebugSphere (const vec_t *pos, float radius, float r, float g, float b, float time, int id)
{
	int index;
	debug_sphere_t *debug_sphere;

	if (id >= 0 && id < g_dd.debug_sphere_capacity)
		index = id;
	else
		index = GetFreeDebugSphere();

	debug_sphere = g_dd.debug_sphere_array + index;
	VectorCopy(pos, debug_sphere->pos);
	debug_sphere->radius = radius;
	debug_sphere->color[0] = r;
	debug_sphere->color[1] = g;
	debug_sphere->color[2] = b;
	debug_sphere->timeleft = time;

	return index;
}

void R_DrawDebug (void)
{
	if (!g_dd.last_time_ms)
	{
		g_dd.last_time_ms = Sys_Milliseconds();
		return;
	}
	else
	{
		int time_ms = Sys_Milliseconds();
		int time_diff_ms = time_ms - g_dd.last_time_ms;
		float time_diff = (float)time_diff_ms / 1000.0f;
		int index;
		qboolean begun = false;

		g_dd.last_time_ms = time_ms;

		// Debug lines
		for (index = 0; index < g_dd.debug_line_capacity; ++index)
		{
			debug_line_t *debug_line = g_dd.debug_line_array + index;

			if (debug_line->timeleft > 0)
			{
				debug_line->timeleft -= time_diff;

				if (!begun)
				{
					qgl.Disable(GL_TEXTURE_2D);
					qgl.Disable(GL_LIGHTING);
					qgl.Begin(GL_LINES);
					begun = true;
				}

				qgl.Color3fv(debug_line->color);
				qgl.Vertex3fv(debug_line->start);
				qgl.Vertex3fv(debug_line->end);
			}
		}

		// Debug spheres
		for (index = 0; index < g_dd.debug_sphere_capacity; ++index)
		{
			debug_sphere_t *debug_sphere = g_dd.debug_sphere_array + index;

			if (debug_sphere->timeleft >  0)
			{
				float radius = debug_sphere->radius;
				int vert, horiz;

				debug_sphere->timeleft -= time_diff;

				if (!begun)
				{
					qgl.Disable(GL_TEXTURE_2D);
					qgl.Disable(GL_LIGHTING);
					qgl.Begin(GL_LINES);
					begun = true;
				}

				qgl.Color3fv(debug_sphere->color);

				for (vert = 0; vert <= 1; ++vert)
				{
					for (horiz = 0; horiz <= 1; ++horiz)
					{
						float horizsign = (float)(horiz * 2 - 1); // + or - 1
						float vertsign = (float)(vert * 2 - 1);

						qgl.Vertex3f(debug_sphere->pos[0], debug_sphere->pos[1], debug_sphere->pos[2] + radius * vertsign);
						qgl.Vertex3f(debug_sphere->pos[0] + radius * horizsign, debug_sphere->pos[1], debug_sphere->pos[2]);
						qgl.Vertex3f(debug_sphere->pos[0], debug_sphere->pos[1], debug_sphere->pos[2] + radius * vertsign);
						qgl.Vertex3f(debug_sphere->pos[0], debug_sphere->pos[1] + radius * horizsign, debug_sphere->pos[2]);
					}
				}
			}
		}

		if (begun)
		{
			qgl.End();
			qgl.Enable(GL_TEXTURE_2D);
		}
	}
}

