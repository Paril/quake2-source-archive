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

extern bot_render_import_t ri;


// The ri. functions will be NULL for dedicated servers, so these functions just do a safety check to prevent crashes.

int DrawDebugSphere (const vec_t *pos, float radius, float r, float g, float b, float time, int id)
{
	if (bot_debug->value)
	{
		if (ri.DrawDebugSphere)
			return ri.DrawDebugSphere(pos, radius, r, g, b, time, id);
	}

	return -1;
}


int DrawDebugLine (const vec_t *start, const vec_t *end, float r, float g, float b, float time, int id)
{
	if (bot_debug->value)
	{
		if (ri.DrawDebugLine)
			return ri.DrawDebugLine(start, end, r, g, b, time, id);
	}

	return -1;
}
