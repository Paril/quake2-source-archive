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
// m_mat3.c
//

#include "shared.h"

mat3x3_t	axisIdentity =  { 
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
};

/*
=============================================================================

	3x3 MATRICES

	The 3x3 matrices are 4x4 matrices minus the origin and other parameters.
=============================================================================
*/

/*
===============
Matrix3_Angles
===============
*/
void Matrix3_Angles (mat3x3_t mat, vec3_t angles)
{
	float	c;
	float	pitch, yaw, roll;

	pitch = -asinf(mat[0][2]);
	c = cosf(pitch);
	pitch = RAD2DEG (pitch);

	if (fabs (c) > 0.005) {			// Gimball lock?
		c = 1.0f / c;
		yaw = RAD2DEG (atan2f((-1)*-mat[0][1] * c, mat[0][0] * c));
		roll = RAD2DEG (atan2f(-mat[1][2] * c, mat[2][2] * c));
	}
	else {
		if (mat[0][2] > 0)
			pitch = -90;
		else
			pitch = 90;
		yaw = RAD2DEG (atan2f(mat[1][0], (-1)*mat[1][1]));
		roll = 0;
	}

	angles[PITCH] = AngleModf (pitch);
	angles[YAW] = AngleModf (yaw);
	angles[ROLL] = AngleModf (roll);
}


/*
===============
Matrix3_Compare
===============
*/
bool Matrix3_Compare (mat3x3_t a, mat3x3_t b)
{
	if (a[0][0] != b[0][0]
	|| a[0][1] != b[0][1]
	|| a[0][2] != b[0][2])
		return false;

	if (a[1][0] != b[1][0]
	|| a[1][1] != b[1][1]
	|| a[1][2] != b[1][2])
		return false;

	if (a[2][0] != b[2][0]
	|| a[2][1] != b[2][1]
	|| a[2][2] != b[2][2])
		return false;

	return true;
}


/*
===============
Matrix3_Copy
===============
*/
void Matrix3_Copy (mat3x3_t in, mat3x3_t out)
{
	out[0][0] = in[0][0];
	out[0][1] = in[0][1];
	out[0][2] = in[0][2];

	out[1][0] = in[1][0];
	out[1][1] = in[1][1];
	out[1][2] = in[1][2];

	out[2][0] = in[2][0];
	out[2][1] = in[2][1];
	out[2][2] = in[2][2];
}


/*
===============
Matrix3_FromPoints
===============
*/
void Matrix3_FromPoints (vec3_t v1, vec3_t v2, vec3_t v3, mat3x3_t m)
{
	float		d;

	m[2][0] = (v1[1] - v2[1]) * (v3[2] - v2[2]) - (v1[2] - v2[2]) * (v3[1] - v2[1]);
	m[2][1] = (v1[2] - v2[2]) * (v3[0] - v2[0]) - (v1[0] - v2[0]) * (v3[2] - v2[2]);
	m[2][2] = (v1[0] - v2[0]) * (v3[1] - v2[1]) - (v1[1] - v2[1]) * (v3[0] - v2[0]);
	VectorNormalizef (m[2], m[2]);

	// This rotate and negate guarantees a vector not colinear with the original
	Vec3Set (m[1], m[2][2], -m[2][0], m[2][1]);
	d = -DotProduct (m[1], m[2]);
	Vec3MA (m[1], d, m[2], m[1]);
	VectorNormalizef (m[1], m[1]);
	CrossProduct (m[1], m[2], m[0]);
}


/*
===============
Matrix3_Identity
===============
*/
void Matrix3_Identity (mat3x3_t mat)
{
 	mat[0][0] = axisIdentity[0][0];
 	mat[0][1] = axisIdentity[0][1];
 	mat[0][2] = axisIdentity[0][2];

 	mat[1][0] = axisIdentity[1][0];
 	mat[1][1] = axisIdentity[1][1];
 	mat[1][2] = axisIdentity[1][2];

 	mat[2][0] = axisIdentity[2][0];
 	mat[2][1] = axisIdentity[2][1];
 	mat[2][2] = axisIdentity[2][2];
}


/*
===============
Matrix3_Matrix4
===============
*/
void Matrix3_Matrix4 (mat3x3_t in, vec3_t origin, mat4x4_t out)
{
	// Axis
	out[ 0] = in[0][0];
	out[ 1] = in[0][1];
	out[ 2] = in[0][2];
	out[ 3] = 0;
	out[ 4] = in[1][0];
	out[ 5] = in[1][1];
	out[ 6] = in[1][2];
	out[ 7] = 0;
	out[ 8] = in[2][0];
	out[ 9] = in[2][1];
	out[10] = in[2][2];
	out[11] = 0;

	// Origin
	out[12] = origin[0];
	out[13] = origin[1];
	out[14] = origin[2];
	out[15] = 1;
}


/*
===============
Matrix3_Multiply
===============
*/
void Matrix3_Multiply (mat3x3_t in1, mat3x3_t in2, mat3x3_t out)
{
	out[0][0] = in1[0][0]*in2[0][0] + in1[0][1]*in2[1][0] + in1[0][2]*in2[2][0];
	out[0][1] = in1[0][0]*in2[0][1] + in1[0][1]*in2[1][1] + in1[0][2]*in2[2][1];
	out[0][2] = in1[0][0]*in2[0][2] + in1[0][1]*in2[1][2] + in1[0][2]*in2[2][2];

	out[1][0] = in1[1][0]*in2[0][0] + in1[1][1]*in2[1][0] +	in1[1][2]*in2[2][0];
	out[1][1] = in1[1][0]*in2[0][1] + in1[1][1]*in2[1][1] + in1[1][2]*in2[2][1];
	out[1][2] = in1[1][0]*in2[0][2] + in1[1][1]*in2[1][2] +	in1[1][2]*in2[2][2];

	out[2][0] = in1[2][0]*in2[0][0] + in1[2][1]*in2[1][0] +	in1[2][2]*in2[2][0];
	out[2][1] = in1[2][0]*in2[0][1] + in1[2][1]*in2[1][1] +	in1[2][2]*in2[2][1];
	out[2][2] = in1[2][0]*in2[0][2] + in1[2][1]*in2[1][2] +	in1[2][2]*in2[2][2];
}


/*
===============
Matrix3_Quat
===============
*/
void Matrix3_Quat (mat3x3_t m, quat_t q)
{
	float	tr, s;

	tr = m[0][0] + m[1][1] + m[2][2];
	if (tr > 0.00001) {
		s = sqrtf(tr + 1.0);
		q[3] = s * 0.5f; s = 0.5f / s;
		q[0] = (m[2][1] - m[1][2]) * s;
		q[1] = (m[0][2] - m[2][0]) * s;
		q[2] = (m[1][0] - m[0][1]) * s;
	}
	else {
		int		i, j, k;

		i = 0;
		if (m[1][1] > m[0][0]) i = 1;
		if (m[2][2] > m[i][i]) i = 2;
		j = (i + 1) % 3;
		k = (i + 2) % 3;

		s = sqrtf(m[i][i] - (m[j][j] + m[k][k]) + 1.0f);

		q[i] = s * 0.5f;
		if (s != 0.0f)
			s = 0.5f / s;
		q[j] = (m[j][i] + m[i][j]) * s;
		q[k] = (m[k][i] + m[i][k]) * s;
		q[3] = (m[k][j] - m[j][k]) * s;
	}

	Quat_Normalize (q);
}


/*
===============
Matrix3_Rotate
===============
*/
void Matrix3_Rotate (mat3x3_t a, float angle, float x, float y, float z)
{
	mat3x3_t	m, b;
	float		c;
	float		s;
	float		mc, t1, t2;

	Q_SinCosf(DEG2RAD(angle), &s, &c);
	mc = 1 - c;
	
	m[0][0] = (x * x * mc) + c;
	m[1][1] = (y * y * mc) + c;
	m[2][2] = (z * z * mc) + c;

	t1 = y * x * mc;
	t2 = z * s;
	m[0][1] = t1 + t2;
	m[1][0] = t1 - t2;

	t1 = x * z * mc;
	t2 = y * s;
	m[0][2] = t1 - t2;
	m[2][0] = t1 + t2;

	t1 = y * z * mc;
	t2 = x * s;
	m[1][2] = t1 + t2;
	m[2][1] = t1 - t2;

	Matrix3_Copy (a, b);
	Matrix3_Multiply (b, m, a);
}


/*
===============
Matrix3_TransformVector
===============
*/
void Matrix3_TransformVector(const mat3x3_t m, const vec3_t v, vec3_t out)
{
	out[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	out[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	out[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}


/*
===============
Matrix3_Transpose
===============
*/
void Matrix3_Transpose (mat3x3_t in, mat3x3_t out)
{
	out[0][0] = in[0][0];
	out[1][1] = in[1][1];
	out[2][2] = in[2][2];

	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[1][0] = in[0][1];
	out[1][2] = in[2][1];
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
}
