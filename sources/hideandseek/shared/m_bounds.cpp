/*
Copyright (C) 1997-2001 Id Software, Inc.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//
// m_bounds.c
//

#include "shared.h"

/*
=============================================================================

	2D

=============================================================================
*/

/*
===============
AddBoundsTo2DBounds
===============
*/
void AddBoundsTo2DBounds (vec2_t inMins, vec2_t inMaxs, vec2_t outMins, vec2_t outMaxs)
{
	// X Vector
	if (inMins[0] < outMins[0])
		outMins[0] = inMins[0];
	if (inMaxs[0] > outMaxs[0])
		outMaxs[0] = inMaxs[0];

	// Y Vector
	if (inMins[1] < outMins[1])
		outMins[1] = inMins[1];
	if (inMaxs[1] > outMaxs[1])
		outMaxs[1] = inMaxs[1];
}


/*
===============
AddPointTo2DBounds
===============
*/
void AddPointTo2DBounds (vec2_t v, vec2_t mins, vec2_t maxs)
{
	// X Vector
	if (v[0] < mins[0])
		mins[0] = v[0];
	if (v[0] > maxs[0])
		maxs[0] = v[0];

	// Y Vector
	if (v[1] < mins[1])
		mins[1] = v[1];
	if (v[1] > maxs[1])
		maxs[1] = v[1];
}


/*
===============
Clear2DBounds
===============
*/
void Clear2DBounds (vec2_t mins, vec2_t maxs)
{
	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -999999;
}

/*
=============================================================================

	3D

=============================================================================
*/

/*
===============
AddPointToBounds
===============
*/
void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs)
{
	// X Vector
	if (v[0] < mins[0])
		mins[0] = v[0];
	if (v[0] > maxs[0])
		maxs[0] = v[0];

	// Y Vector
	if (v[1] < mins[1])
		mins[1] = v[1];
	if (v[1] > maxs[1])
		maxs[1] = v[1];

	// Z Vector
	if (v[2] < mins[2])
		mins[2] = v[2];
	if (v[2] > maxs[2])
		maxs[2] = v[2];
}


/*
=================
BoundsAndSphereIntersect
=================
*/
bool BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t centre, float radius)
{
	return (bool)(mins[0] <= centre[0]+radius
		&& mins[1] <= centre[1]+radius
		&& mins[2] <= centre[2]+radius
		&& maxs[0] >= centre[0]-radius
		&& maxs[1] >= centre[1]-radius
		&& maxs[2] >= centre[2]-radius);
}


/*
=================
BoundsIntersect
=================
*/
bool BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2)
{
	return (bool)(mins1[0] <= maxs2[0]
		&& mins1[1] <= maxs2[1]
		&& mins1[2] <= maxs2[2]
		&& maxs1[0] >= mins2[0]
		&& maxs1[1] >= mins2[1]
		&& maxs1[2] >= mins2[2]);
}


/*
===============
ClearBounds
===============
*/
void ClearBounds (vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 999999;
	maxs[0] = maxs[1] = maxs[2] = -999999;
}


/*
===============
MinMins
===============
*/
void MinMins (vec3_t a, vec3_t b, vec3_t out)
{
	out[0] = min (a[0], b[0]);
	out[1] = min (a[1], b[1]);
	out[2] = min (a[2], b[2]);
}


/*
===============
MaxMaxs
===============
*/
void MaxMaxs (vec3_t a, vec3_t b, vec3_t out)
{
	out[0] = max (a[0], b[0]);
	out[1] = max (a[1], b[1]);
	out[2] = max (a[2], b[2]);
}


/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	vec3_t	corner;

	corner[0] = fabsf(mins[0]) > fabsf(maxs[0]) ? fabsf(mins[0]) : fabsf(maxs[0]);
	corner[1] = fabsf(mins[1]) > fabsf(maxs[1]) ? fabsf(mins[1]) : fabsf(maxs[1]);
	corner[2] = fabsf(mins[2]) > fabsf(maxs[2]) ? fabsf(mins[2]) : fabsf(maxs[2]);

	return (float)Vec3Length (corner);
}
