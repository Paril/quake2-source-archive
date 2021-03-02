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
// m_quad.c
//

#include "shared.h"

quat_t		quatIdentity = {
	0, 0, 0, 1
};

/*
=============================================================================

	QUATERNIONS

=============================================================================
*/

/*
===============
Quat_ConcatTransforms
===============
*/
void Quat_ConcatTransforms (quat_t q1, vec3_t v1, quat_t q2, vec3_t v2, quat_t q, vec3_t v)
{
	Quat_Multiply (q1, q2, q);
	Quat_TransformVector (q1, v2, v);
	v[0] += v1[0]; v[1] += v1[1]; v[2] += v1[2];
}


/*
===============
Quat_Copy
===============
*/
void Quat_Copy (quat_t q1, quat_t q2)
{
	q2[0] = q1[0];
	q2[1] = q1[1];
	q2[2] = q1[2];
	q2[3] = q1[3];
}


/*
===============
Quat_Conjugate
===============
*/
void Quat_Conjugate (quat_t q1, quat_t q2)
{
	q2[0] = -q1[0];
	q2[1] = -q1[1];
	q2[2] = -q1[2];
	q2[3] = q1[3];
}


/*
===============
Quat_Identity
===============
*/
void Quat_Identity (quat_t q)
{
	Quat_Copy (quatIdentity, q);
}


/*
===============
Quat_Inverse
===============
*/
float Quat_Inverse (quat_t q1, quat_t q2)
{
	Quat_Conjugate (q1, q2);

	return Quat_Normalize (q2);
}


/*
===============
Quat_Normalize
===============
*/
float Quat_Normalize (quat_t q)
{
	float length;

	length = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
	if (length != 0) {
		float ilength = 1.0f / sqrtf(length);
		q[0] *= ilength;
		q[1] *= ilength;
		q[2] *= ilength;
		q[3] *= ilength;
	}

	return length;
}


/*
===============
Quat_Lerp
===============
*/
void Quat_Lerp (quat_t q1, quat_t q2, float t, quat_t out)
{
	quat_t	p1;
	float	omega, cosom, sinom, scale0, scale1;

	cosom = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
	if (cosom < 0.0) { 
		cosom = -cosom;
		p1[0] = -q1[0]; p1[1] = -q1[1];
		p1[2] = -q1[2]; p1[3] = -q1[3];
	}
	else {
		p1[0] = q1[0]; p1[1] = q1[1];
		p1[2] = q1[2]; p1[3] = q1[3];
	}

	if (cosom < 1.0 - 0.001) {
		omega = acosf(cosom);
		sinom = 1.0f / sinf(omega);
		scale0 = sinf((1.0f - t) * omega) * sinom;
		scale1 = sinf(t * omega) * sinom;
	}
	else { 
		scale0 = 1.0f - t;
		scale1 = t;
	}

	out[0] = scale0 * p1[0] + scale1 * q2[0];
	out[1] = scale0 * p1[1] + scale1 * q2[1];
	out[2] = scale0 * p1[2] + scale1 * q2[2];
	out[3] = scale0 * p1[3] + scale1 * q2[3];
}


/*
===============
Quat_Matrix3
===============
*/
void Quat_Matrix3 (quat_t q, mat3x3_t m)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];
	xx = q[0] * x2; xy = q[0] * y2; xz = q[0] * z2;
	yy = q[1] * y2; yz = q[1] * z2; zz = q[2] * z2;
	wx = q[3] * x2; wy = q[3] * y2; wz = q[3] * z2;

	m[0][0] = 1.0f - yy - zz; m[0][1] = xy - wz; m[0][2] = xz + wy;
	m[1][0] = xy + wz; m[1][1] = 1.0f - xx - zz; m[1][2] = yz - wx;
	m[2][0] = xz - wy; m[2][1] = yz + wx; m[2][2] = 1.0f - xx - yy;
}


/*
===============
Quat_Multiply
===============
*/
void Quat_Multiply (quat_t q1, quat_t q2, quat_t out)
{
	out[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	out[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	out[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	out[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
}


/*
===============
Quat_TransformVector
===============
*/
void Quat_TransformVector (quat_t q, vec3_t v, vec3_t out)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];
	xx = q[0] * x2; xy = q[0] * y2; xz = q[0] * z2;
	yy = q[1] * y2; yz = q[1] * z2; zz = q[2] * z2;
	wx = q[3] * x2; wy = q[3] * y2; wz = q[3] * z2;

	out[0] = (float)((1.0f - yy - zz) * v[0] + (xy - wz) * v[1] + (xz + wy) * v[2]);
	out[1] = (float)((xy + wz) * v[0] + (1.0f - xx - zz) * v[1] + (yz - wx) * v[2]);
	out[2] = (float)((xz - wy) * v[0] + (yz + wx) * v[1] + (1.0f - xx - yy) * v[2]);
}
