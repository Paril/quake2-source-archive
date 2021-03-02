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
// m_angles.c
//

#include "shared.h"

/*
===============
AngleModf
===============
*/
float AngleModf (float a)
{
	return (360.0f/65536.0f) * ((int)(a*(65536.0f/360.0f)) & 65535);
}


/*
===============
Angles_Matrix3
===============
*/
void Angles_Matrix3 (vec3_t angles, mat3x3_t axis)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = DEG2RAD (angles[YAW]);
	Q_SinCosf(angle, &sy, &cy);
	angle = DEG2RAD (angles[PITCH]);
	Q_SinCosf(angle, &sp, &cp);
	angle = DEG2RAD (angles[ROLL]);
	Q_SinCosf(angle, &sr, &cr);

	// Forward
	axis[0][0] = cp * cy;
	axis[0][1] = cp * sy;
	axis[0][2] = -sp;

	// Left
	axis[1][0] = (-1 * sr * sp * cy + -1 * cr * -sy) * -1;
	axis[1][1] = (-1 * sr * sp * sy + -1 * cr * cy) * -1;
	axis[1][2] = (-1 * sr * cp) * -1;

	// Up
	axis[2][0] = (cr * sp * cy + -sr * -sy);
	axis[2][1] = (cr * sp * sy + -sr * cy);
	axis[2][2] = cr * cp;
}


/*
===============
Angles_Vectors
===============
*/
void Angles_Vectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = DEG2RAD (angles[YAW]);
	Q_SinCosf(angle, &sy, &cy);
	angle = DEG2RAD (angles[PITCH]);
	Q_SinCosf(angle, &sp, &cp);
	angle = DEG2RAD (angles[ROLL]);
	Q_SinCosf(angle, &sr, &cr);

	if (forward) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if (right) {
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}
	if (up) {
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}


/*
===============
LerpAngle
===============
*/
float LerpAngle (float a2, float a1, float frac)
{
	if (a1 - a2 > 180)
		a1 -= 360;
	if (a1 - a2 < -180)
		a1 += 360;

	return a2 + frac * (a1 - a2);
}


/*
===============
VecToAngles
===============
*/
void VecToAngles (vec3_t vec, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (vec[1] == 0 && vec[0] == 0) {
		yaw = 0;
		if (vec[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else {
		if (vec[0])
			yaw = atan2f(vec[1], vec[0]) * (180.0f / M_PI);
		else if (vec[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
		pitch = atan2f(vec[2], forward) * (180.0f / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}


/*
===============
VecToAngleRolled
===============
*/
void VecToAngleRolled (vec3_t value, float angleYaw, vec3_t angles)
{
	float	forward, yaw, pitch;

	yaw = atan2f(value[1], value[0]) * 180.0f / M_PI;
	forward = sqrtf(value[0]*value[0] + value[1]*value[1]);
	pitch = atan2f(value[2], forward) * 180.0f / M_PI;

	if (pitch < 0)
		pitch += 360;

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = -angleYaw;
}


/*
===============
VecToYaw
===============
*/
float VecToYaw (vec3_t vec)
{
	float	yaw;

	if (vec[PITCH] == 0) {
		yaw = 0;
		if (vec[YAW] > 0)
			yaw = 90;
		else if (vec[YAW] < 0)
			yaw = -90;
	}
	else {
		yaw = atan2f(vec[YAW], vec[PITCH]) * 180 / M_PI;
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}
