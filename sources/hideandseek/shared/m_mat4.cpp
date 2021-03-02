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
// m_mat4.c
//

#include "shared.h"

mat4x4_t	mat4x4Identity = { 
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1 
};

/*
Multiply matrices together for combined results.

Identity
[ 1,   0,   0,   0 ]
[ 0,   1,   0,   0 ]
[ 0,   0,   1,   0 ]
[ 0,   0,   0,   1 ]

Rotation on X
[ 1,   0,   0,   0 ]
[ 0,   cx,  -sx, 0 ]
[ 0,   sx,  cx,  0 ]
[ 0,   0,   0,   1 ]

Rotation on Y
[ cy,  0,   sy,  0 ]
[ 0,   1,   0,   0 ]
[ -sy, 0,   cy,  0 ]
[ 0,   0,   0,   1 ]

Rotation on Z
[ cz,  -sz, 0,   0 ]
[ sz,  cz,  0,   0 ]
[ 0,   0,   1,   0 ]
[ 0,   0,   0,   1 ]

Scale
[ sx,  0,   0,   0 ]
[ 0,   sy,  0,   0 ]
[ 0,   0,   sz,  0 ]
[ 0,   0,   0,   1 ]

Translation
[ 1,   0,   0,   0 ]
[ 0,   1,   0,   0 ]
[ 0,   0,   1,   0 ]
[ tx,  ty,  tz,  1 ]
*/

/*
=============================================================================

	4x4 MATRICES

=============================================================================
*/

/*
===============
Matrix4_Compare
===============
*/
bool Matrix4_Compare(const mat4x4_t a, const mat4x4_t b)
{
	for (int i=0 ; i<16 ; i++)
	{
		if (a[i] != b[i])
			return false;
	}

	return true;
}


/*
===============
Matrix4_Copy
===============
*/
void Matrix4_Copy(const mat4x4_t a, mat4x4_t b)
{
	b[0 ] = a[0 ];
	b[1 ] = a[1 ];
	b[2 ] = a[2 ];
	b[3 ] = a[3 ];
	b[4 ] = a[4 ];
	b[5 ] = a[5 ];
	b[6 ] = a[6 ];
	b[7 ] = a[7 ];
	b[8 ] = a[8 ];
	b[9 ] = a[9 ];
	b[10] = a[10];
	b[11] = a[11];
	b[12] = a[12];
	b[13] = a[13];
	b[14] = a[14];
	b[15] = a[15];
}


/*
===============
Matrix4_Copy2D
===============
*/
void Matrix4_Copy2D(const mat4x4_t m1, mat4x4_t m2)
{
	m2[0] = m1[0];
	m2[1] = m1[1];
	m2[4] = m1[4];
	m2[5] = m1[5];
	m2[12] = m1[12];
	m2[13] = m1[13];
}


/*
===============
Matrix4_Identity
===============
*/
void Matrix4_Identity(mat4x4_t mat)
{
	mat[0 ] = mat4x4Identity[0 ];
	mat[1 ] = mat4x4Identity[1 ];
	mat[2 ] = mat4x4Identity[2 ];
	mat[3 ] = mat4x4Identity[3 ];
	mat[4 ] = mat4x4Identity[4 ];
	mat[5 ] = mat4x4Identity[5 ];
	mat[6 ] = mat4x4Identity[6 ];
	mat[7 ] = mat4x4Identity[7 ];
	mat[8 ] = mat4x4Identity[8 ];
	mat[9 ] = mat4x4Identity[9 ];
	mat[10] = mat4x4Identity[10];
	mat[11] = mat4x4Identity[11];
	mat[12] = mat4x4Identity[12];
	mat[13] = mat4x4Identity[13];
	mat[14] = mat4x4Identity[14];
	mat[15] = mat4x4Identity[15];
}


/*
===============
Matrix4_Matrix3
===============
*/
void Matrix4_Matrix3(const mat4x4_t in, mat3x3_t out)
{
	out[0][0] = in[0 ];
	out[0][1] = in[4 ];
	out[0][2] = in[8 ];

	out[1][0] = in[1 ];
	out[1][1] = in[5 ];
	out[1][2] = in[9 ];

	out[2][0] = in[2 ];
	out[2][1] = in[6 ];
	out[2][2] = in[10];
}


/*
===============
Matrix4_Multiply
===============
*/
void Matrix4_Multiply(const mat4x4_t a, const mat4x4_t b, mat4x4_t product)
{
	product[0 ] = a[0]*b[0 ] + a[4]*b[1 ] + a[8 ]*b[2 ] + a[12]*b[3 ];
	product[1 ] = a[1]*b[0 ] + a[5]*b[1 ] + a[9 ]*b[2 ] + a[13]*b[3 ];
	product[2 ] = a[2]*b[0 ] + a[6]*b[1 ] + a[10]*b[2 ] + a[14]*b[3 ];
	product[3 ] = a[3]*b[0 ] + a[7]*b[1 ] + a[11]*b[2 ] + a[15]*b[3 ];
	product[4 ] = a[0]*b[4 ] + a[4]*b[5 ] + a[8 ]*b[6 ] + a[12]*b[7 ];
	product[5 ] = a[1]*b[4 ] + a[5]*b[5 ] + a[9 ]*b[6 ] + a[13]*b[7 ];
	product[6 ] = a[2]*b[4 ] + a[6]*b[5 ] + a[10]*b[6 ] + a[14]*b[7 ];
	product[7 ] = a[3]*b[4 ] + a[7]*b[5 ] + a[11]*b[6 ] + a[15]*b[7 ];
	product[8 ] = a[0]*b[8 ] + a[4]*b[9 ] + a[8 ]*b[10] + a[12]*b[11];
	product[9 ] = a[1]*b[8 ] + a[5]*b[9 ] + a[9 ]*b[10] + a[13]*b[11];
	product[10] = a[2]*b[8 ] + a[6]*b[9 ] + a[10]*b[10] + a[14]*b[11];
	product[11] = a[3]*b[8 ] + a[7]*b[9 ] + a[11]*b[10] + a[15]*b[11];
	product[12] = a[0]*b[12] + a[4]*b[13] + a[8 ]*b[14] + a[12]*b[15];
	product[13] = a[1]*b[12] + a[5]*b[13] + a[9 ]*b[14] + a[13]*b[15];
	product[14] = a[2]*b[12] + a[6]*b[13] + a[10]*b[14] + a[14]*b[15];
	product[15] = a[3]*b[12] + a[7]*b[13] + a[11]*b[14] + a[15]*b[15];
}


/*
===============
Matrix4_Multiply2D
===============
*/
void Matrix4_Multiply2D(const mat4x4_t m1, const mat4x4_t m2, mat4x4_t out)
{
	out[0]  = m1[0] * m2[0] + m1[4] * m2[1];
	out[1]  = m1[1] * m2[0] + m1[5] * m2[1];
	out[4]  = m1[0] * m2[4] + m1[4] * m2[5];
	out[5]  = m1[1] * m2[4] + m1[5] * m2[5];
	out[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[12];
	out[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[13];
}


/*
===============
Matrix4_Multiply_Vec3
===============
*/
void Matrix4_Multiply_Vec3(const mat4x4_t m, const vec3_t v, vec3_t out)
{
	out[0] = m[0]*v[0] + m[1]*v[1] + m[2 ]*v[2];
	out[1] = m[4]*v[0] + m[5]*v[1] + m[6 ]*v[2];
	out[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];
}


/*
===============
Matrix4_Multiply_Vec4
===============
*/
void Matrix4_Multiply_Vec4(const mat4x4_t m, const vec4_t v, vec4_t out)
{
	out[0] = m[0]*v[0] + m[4]*v[1] + m[8 ]*v[2] + m[12]*v[3];
	out[1] = m[1]*v[0] + m[5]*v[1] + m[9 ]*v[2] + m[13]*v[3];
	out[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3];
	out[3] = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3];
}

/*
===============
Matrix4_MultiplyFast
===============
*/
void Matrix4_MultiplyFast(const mat4x4_t a, const mat4x4_t b, mat4x4_t product)
{
	product[0]  = a[0] * b[0 ] + a[4] * b[1 ] + a[8 ] * b[2];
	product[1]  = a[1] * b[0 ] + a[5] * b[1 ] + a[9 ] * b[2];
	product[2]  = a[2] * b[0 ] + a[6] * b[1 ] + a[10] * b[2];
	product[3]  = 0.0f;
	product[4]  = a[0] * b[4 ] + a[4] * b[5 ] + a[8 ] * b[6];
	product[5]  = a[1] * b[4 ] + a[5] * b[5 ] + a[9 ] * b[6];
	product[6]  = a[2] * b[4 ] + a[6] * b[5 ] + a[10] * b[6];
	product[7]  = 0.0f;
	product[8]  = a[0] * b[8 ] + a[4] * b[9 ] + a[8 ] * b[10];
	product[9]  = a[1] * b[8 ] + a[5] * b[9 ] + a[9 ] * b[10];
	product[10] = a[2] * b[8 ] + a[6] * b[9 ] + a[10] * b[10];
	product[11] = 0.0f;
	product[12] = a[0] * b[12] + a[4] * b[13] + a[8 ] * b[14] + a[12];
	product[13] = a[1] * b[12] + a[5] * b[13] + a[9 ] * b[14] + a[13];
	product[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14];
	product[15] = 1.0f;
}


/*
===============
Matrix4_MultiplyFast
===============
*/
void Matrix4_MultiplyFast2(const mat4x4_t m1, const mat4x4_t m2, mat4x4_t out)
{
	out[0]  = m1[0] * m2[0] + m1[4] * m2[1] + m1[12] * m2[3];
	out[1]  = m1[1] * m2[0] + m1[5] * m2[1] + m1[13] * m2[3];
	out[2]  = m2[2];
	out[3]  = m2[3];
	out[4]  = m1[0] * m2[4] + m1[4] * m2[5] + m1[12] * m2[7];
	out[5]  = m1[1] * m2[4] + m1[5] * m2[5] + m1[13] * m2[7];
	out[6]  = m2[6];
	out[7]  = m2[7];
	out[8]  = m1[0] * m2[8] + m1[4] * m2[9] + m1[12] * m2[11];
	out[9]  = m1[1] * m2[8] + m1[5] * m2[9] + m1[13] * m2[11];
	out[10] = m2[10];
	out[11] = m2[11];
	out[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[12] * m2[15];
	out[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[13] * m2[15];
	out[14] = m2[14];
	out[15] = m2[15];
}


/*
===============
Matrix4_Rotate
===============
*/
void Matrix4_Rotate(mat4x4_t a, const float angle, const float x, const float y, const float z)
{
	mat4x4_t	m, b;
	float		c;
	float		s;
	float		mc, t1, t2;

	Q_SinCosf(DEG2RAD(angle), &s, &c);
	mc = 1 - c;
	
	m[0]  = (x * x * mc) + c;
	m[5]  = (y * y * mc) + c;
	m[10] = (z * z * mc) + c;

	t1 = y * x * mc;
	t2 = z * s;
	m[1] = t1 + t2;
	m[4] = t1 - t2;

	t1 = x * z * mc;
	t2 = y * s;
	m[2] = t1 - t2;
	m[8] = t1 + t2;

	t1 = y * z * mc;
	t2 = x * s;
	m[6] = t1 + t2;
	m[9] = t1 - t2;

	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0;
	m[15] = 1;

	Matrix4_Copy (a, b);
	Matrix4_MultiplyFast (b, m, a);
}


/*
===============
Matrix4_Scale
===============
*/
void Matrix4_Scale(mat4x4_t m, const float x, const float y, const float z)
{
	m[0] *= x;		m[4] *= y;		m[8 ] *= z;
	m[1] *= x;		m[5] *= y;		m[9 ] *= z;
	m[2] *= x;		m[6] *= y;		m[10] *= z;
	m[3] *= x;		m[7] *= y;		m[11] *= z;
}


/*
===============
Matrix4_Scale2D
===============
*/
void Matrix4_Scale2D(mat4x4_t m, const float x, const float y)
{
	m[0] *= x;
	m[1] *= x;
	m[4] *= y;
	m[5] *= y;
}


/*
===============
Matrix4_Stretch2D
===============
*/
void Matrix4_Stretch2D(mat4x4_t m, const float s, const float t)
{
	m[0] *= s;
	m[1] *= s;
	m[4] *= s;
	m[5] *= s;
	m[12] = s * m[12] + t;
	m[13] = s * m[13] + t;
}


/*
===============
Matrix4_Translate
===============
*/
void Matrix4_Translate(mat4x4_t m, const float x, const float y, const float z)
{
	m[12] = m[0] * x + m[4] * y + m[8 ] * z + m[12];
	m[13] = m[1] * x + m[5] * y + m[9 ] * z + m[13];
	m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
	m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}


/*
===============
Matrix4_Translate2D
===============
*/
void Matrix4_Translate2D(mat4x4_t m, const float x, const float y)
{
	m[12] += x;
	m[13] += y;
}


/*
===============
Matrix4_Transpose
===============
*/
void Matrix4_Transpose(const mat4x4_t m, mat4x4_t ret)
{
	ret[0 ] = m[0]; ret[1 ] = m[4]; ret[2 ] = m[8 ]; ret[3 ] = m[12];
	ret[4 ] = m[1]; ret[5 ] = m[5]; ret[6 ] = m[9 ]; ret[7 ] = m[13];
	ret[8 ] = m[2]; ret[9 ] = m[6]; ret[10] = m[10]; ret[11] = m[14];
	ret[12] = m[3]; ret[13] = m[7]; ret[14] = m[11]; ret[15] = m[15];
}
