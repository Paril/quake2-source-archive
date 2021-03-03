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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "q_shared.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <math.h> // jitskm


vec3_t vec3_origin = {0,0,0};

//============================================================================
void RotatePointAroundVector (vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
	// jit - rewritten using code here for reference: http://www.blitzbasic.com/Community/posts.php?topic=57616
	float u = dir[0], v = dir[1], w = dir[2];
	float x = point[0], y = point[1], z = point[2];
	float ux = u * x, uy = u * y, uz = u * z, vx = v * x, vy = v * y, vz = v * z, wx = w * x, wy = w * y, wz = w * z;
	float uu = u * u, ww = w * w, vv = v * v;
	float s = (float)sin(DEG2RAD(degrees));
	float c = (float)cos(DEG2RAD(degrees));

	dst[0] = u * (ux + vy + wz) + (x * (vv + ww) - u * (vy + wz)) * c + (vz - wy) * s;
	dst[1] = v * (ux + vy + wz) + (y * (uu + ww) - v * (ux + wz)) * c + (wx - uz) * s;
	dst[2] = w * (ux + vy + wz) + (z * (uu + vv) - w * (ux + vy)) * c + (uy - vx) * s;
}


float DampIIR (float src, float dest, float factor, float dt)
{
	return (src * factor + dest * dt) / (factor + dt);
}


void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * ((float)M_PI * 2.0f / 360.0f);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * ((float)M_PI * 2.0f / 360.0f);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * ((float)M_PI * 2.0f / 360.0f);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}

	if (up)
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}

void ProjectPointOnPlane (vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0f / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

// assumes "src" is normalized
void PerpendicularVector (vec3_t dst, const vec3_t src)
{
	int	pos;
	int i;
	float minelem = 1.0f;
	vec3_t tempvec;

	// find the smallest magnitude axially aligned vector
	for (pos = 0, i = 0; i < 3; ++i)
	{
		if (fabs(src[i]) < minelem)
		{
			pos = i;
			minelem = fabs(src[i]);
		}
	}

	tempvec[0] = tempvec[1] = tempvec[2] = 0.0f;
	tempvec[pos] = 1.0f;

	// project the point onto the plane defined by src
	ProjectPointOnPlane(dst, tempvec, src);
	VectorNormalize(dst);
}



/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}


/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
}


//============================================================================


//float Q_fabs (float f)
//{
//#if 0
//	if (f >= 0)
//		return f;
//	return -f;
//#else
//	int tmp = * ( int * ) &f;
//	tmp &= 0x7FFFFFFF;
//	return * ( float * ) &tmp;
//#endif
//}

#if defined _M_IX86 && !defined C_ONLY
#pragma warning (disable:4035)
__declspec( naked ) long Q_ftol( float f )
{
	static int tmp;
	__asm fld dword ptr [esp+4]
	__asm fistp tmp
	__asm mov eax, tmp
	__asm ret
}
#pragma warning (default:4035)
#endif

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
 =================
 AxisCompare -- taken from Quake2Evolved
 =================
*/
qboolean AxisCompare (const vec3_t axis1[3], const vec3_t axis2[3])
{
	if (axis1[0][0] != axis2[0][0] || axis1[0][1] != axis2[0][1] || axis1[0][2] != axis2[0][2])
		return false;
	if (axis1[1][0] != axis2[1][0] || axis1[1][1] != axis2[1][1] || axis1[1][2] != axis2[1][2])
		return false;
	if (axis1[2][0] != axis2[2][0] || axis1[2][1] != axis2[2][1] || axis1[2][2] != axis2[2][2])
		return false;

	return true;
}

float	anglemod(float a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	//a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	a = a - 360.0 * floor(a / 360.0); // jit - avoid integer overflow when playing on servers with a long uptime.
	return a;
}

	int		i;
	vec3_t	corners[2];


// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
#if !id386 || defined __unix__ 
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float	dist1, dist2;
	int		sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}
	
// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		assert(0);
		break;
	}

	sides = 0;
	
	if (dist1 >= p->dist)
		sides = 1;

	if (dist2 < p->dist)
		sides |= 2;

	//assert(sides != 0);

	return sides;
}
#else
#pragma warning( disable: 4035 )

__declspec( naked ) int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	static int bops_initialized;
	static int Ljmptab[8];

	__asm {

		push ebx
			
		cmp bops_initialized, 1
		je  initialized
		mov bops_initialized, 1
		
		mov Ljmptab[0*4], offset Lcase0
		mov Ljmptab[1*4], offset Lcase1
		mov Ljmptab[2*4], offset Lcase2
		mov Ljmptab[3*4], offset Lcase3
		mov Ljmptab[4*4], offset Lcase4
		mov Ljmptab[5*4], offset Lcase5
		mov Ljmptab[6*4], offset Lcase6
		mov Ljmptab[7*4], offset Lcase7
			
initialized:

		mov edx,ds:dword ptr[4+12+esp]
		mov ecx,ds:dword ptr[4+4+esp]
		xor eax,eax
		mov ebx,ds:dword ptr[4+8+esp]
		mov al,ds:byte ptr[17+edx]
		cmp al,8
		jge Lerror
		fld ds:dword ptr[0+edx]
		fld st(0)
		jmp dword ptr[Ljmptab+eax*4]
Lcase0:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase1:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase2:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase3:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase4:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase5:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ebx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase6:
		fmul ds:dword ptr[ebx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase7:
		fmul ds:dword ptr[ecx]
		fld ds:dword ptr[0+4+edx]
		fxch st(2)
		fmul ds:dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[4+ecx]
		fld ds:dword ptr[0+8+edx]
		fxch st(2)
		fmul ds:dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul ds:dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul ds:dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
LSetSides:
		faddp st(2),st(0)
		fcomp ds:dword ptr[12+edx]
		xor ecx,ecx
		fnstsw ax
		fcomp ds:dword ptr[12+edx]
		and ah,1
		xor ah,1
		add cl,ah
		fnstsw ax
		and ah,1
		add ah,ah
		add cl,ah
		pop ebx
		mov eax,ecx
		ret
Lerror:
		int 3
	}
}
#pragma warning( default: 4035 )
#endif

void ClearBounds (vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs)
{
	int		i;
	vec_t	val;

	for (i=0 ; i<3 ; i++)
	{
		val = v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}


int _VectorCompare (vec3_t v1, vec3_t v2)
{
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2])
			return 0;
			
	return 1;
}



vec_t VectorNormalizeRetLen (vec3_t v)
{
	float ilength;

	ilength = Q_rsqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (ilength)
	{
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;

		return 1.0f / ilength;
	}
	else
	{
		return 0.0f;
	}
}


void VectorNormalize (vec3_t v)
{
	float ilength;

	ilength = Q_rsqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (ilength)
	{
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
}


vec_t VectorNormalize2RetLen (vec3_t v, vec3_t out)
{
	float ilength;

	ilength = Q_rsqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (ilength)
	{
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;

		return 1.0f / ilength;
	}
	else
	{
		return 0.0f;
	}
}


void _VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}


vec_t _DotProduct (vec3_t v1, vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

vec_t VectorSquareDistance (const vec_t *a, const vec_t *b) // jitvec
{
	register float dx = a[0] - b[0];
	register float dy = a[1] - b[1];
	register float dz = a[2] - b[2];

	return dx * dx + dy * dy + dz * dz;
}

void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}

void _VectorCopy (vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}


void _CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}


vec_t VectorLength (vec3_t v)
{
	float ilength;

	ilength = Q_rsqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (ilength)
		return 1.0f / ilength;
	else
		return 0.0f;
}


void _VectorInverse (vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void _VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

// jitodo -- benchmark compare to rsqrt w/optimizations -- use Enigma's faster func? - http://www.gamedev.net/community/forums/topic.asp?topic_id=139956
float Q_rsqrt (float number) // jit - Fast approximation reciprocal square root
{
	int i;
	float x2, y;

	if (number == 0.0)
		return 0.0;

	x2 = number * 0.5f;
	y = number;
	i = * (int *) &y;			// evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);	// Newton's? approximation (googling this value yeilds interesting results)
	y = * (float *) &i;
	y = y * (1.5f - (x2 * y * y));	// this can be done a second time

	return y;
}


int Q_log2 (int val)
{
	int answer = 0;

	while (val >>= 1)
		answer++;

	return answer;
}


int Q_Round (float val) // jit
{
	if (val >= 0.0f)
	{
		val += 0.5f;
		return (int)val;
	}
	else
	{
		val -= 0.5f;
		return (int)val;
	}
}

// Clamp the values from -1 to 1.  Invalid floating point values are generated outside of these bounds
float Q_asin (vec_t v) // jit
{
	if (v > 1.0f)
		v = 1.0f;

	if (v < -1.0f)
		v = -1.0f;

	return asin(v);
}

//============================================================================
// jitskm
// taken from qfusion

void Quat_Identity (quat_t q)
{
	q[0] = 0;
	q[1] = 0;
	q[2] = 0;
	q[3] = 1;
}

void Quat_Copy (const quat_t q1, quat_t q2)
{
	q2[0] = q1[0];
	q2[1] = q1[1];
	q2[2] = q1[2];
	q2[3] = q1[3];
}

qboolean Quat_Compare (const quat_t q1, const quat_t q2)
{
	if (q1[0] != q2[0] || q1[1] != q2[1] || q1[2] != q2[2] || q1[3] != q2[3])
		return false;

	return true;
}

void Quat_Conjugate (const quat_t q1, quat_t q2)
{
	q2[0] = -q1[0];
	q2[1] = -q1[1];
	q2[2] = -q1[2];
	q2[3] = q1[3];
}

vec_t Quat_Normalize (quat_t q)
{
	vec_t length = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];

	if (length != 0)
	{
		vec_t ilength = 1.0 / sqrt(length);
		q[0] *= ilength;
		q[1] *= ilength;
		q[2] *= ilength;
		q[3] *= ilength;
	}

	return length;
}
/*
vec_t Quat_Inverse (const quat_t q1, quat_t q2)
{
	Quat_Conjugate(q1, q2);

	return Quat_Normalize(q2);
}
*/
void Matrix_Quat (vec3_t m[3], quat_t q)
{
	vec_t tr, s;

	tr = m[0][0] + m[1][1] + m[2][2];

	if (tr > 0.00001)
	{
		s = sqrt(tr + 1.0);
		q[3] = s * 0.5; s = 0.5 / s;
		q[0] = (m[2][1] - m[1][2]) * s;
		q[1] = (m[0][2] - m[2][0]) * s;
		q[2] = (m[1][0] - m[0][1]) * s;
	}
	else
	{
		int i, j, k;

		i = 0;

		if (m[1][1] > m[0][0])
			i = 1;

		if (m[2][2] > m[i][i])
			i = 2;

		j = (i + 1) % 3;
		k = (i + 2) % 3;

		s = sqrt( m[i][i] - (m[j][j] + m[k][k]) + 1.0 );

		q[i] = s * 0.5; if( s != 0.0 ) s = 0.5 / s;
		q[j] = (m[j][i] + m[i][j]) * s;
		q[k] = (m[k][i] + m[i][k]) * s;
		q[3] = (m[k][j] - m[j][k]) * s;
	}

	Quat_Normalize(q);
}

void Quat_Multiply (const quat_t q1, const quat_t q2, quat_t out)
{
	// jitodo - optimize
	out[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	out[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	out[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	out[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
}

void Quat_Lerp (const quat_t q1, const quat_t q2, vec_t t, quat_t out)
{
	quat_t p1;
	vec_t omega, cosom, sinom, scale0, scale1;

	if (Quat_Compare(q1, q2))
	{
		Quat_Copy(q1, out); // jitodo - optimize, make quat_copy a define like vectorcopy
		return;
	}

	cosom = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

	if (cosom < 0.0)
	{ 
		cosom = -cosom;
		p1[0] = -q1[0]; p1[1] = -q1[1];
		p1[2] = -q1[2]; p1[3] = -q1[3];
	}
	else
	{
		p1[0] = q1[0]; p1[1] = q1[1];
		p1[2] = q1[2]; p1[3] = q1[3];
	}

	if (cosom < 1.0 - 0.001)
	{
		omega = acos(cosom);
		sinom = 1.0 / sin(omega);
		scale0 = sin((1.0 - t) * omega) * sinom;
		scale1 = sin(t * omega) * sinom;
	}
	else
	{ 
		scale0 = 1.0 - t;
		scale1 = t;
	}

	out[0] = scale0 * p1[0] + scale1 * q2[0];
	out[1] = scale0 * p1[1] + scale1 * q2[1];
	out[2] = scale0 * p1[2] + scale1 * q2[2];
	out[3] = scale0 * p1[3] + scale1 * q2[3];
}


#if 0// OLD_ROW_ORDER_TEST
void Quat_Matrix (const quat_t q, vec3_t m[3])
{
	vec_t wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];

	xx = q[0] * x2; xy = q[0] * y2; xz = q[0] * z2;
	yy = q[1] * y2; yz = q[1] * z2; zz = q[2] * z2;
	wx = q[3] * x2; wy = q[3] * y2; wz = q[3] * z2;

	m[0][0] = 1.0f - yy - zz;
	m[0][1] = xy - wz;
	m[0][2] = xz + wy;

	m[1][0] = xy + wz;
	m[1][1] = 1.0f - xx - zz;
	m[1][2] = yz - wx;

	m[2][0] = xz - wy;
	m[2][1] = yz + wx;
	m[2][2] = 1.0f - xx - yy;
}
#else
void Quat_Matrix (const quat_t q, vec3_t m[3])
{
	vec_t wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0];
	y2 = q[1] + q[1];
	z2 = q[2] + q[2];

	xx = q[0] * x2;
	xy = q[0] * y2;
	xz = q[0] * z2;

	yy = q[1] * y2;
	yz = q[1] * z2;

	zz = q[2] * z2;

	wx = q[3] * x2;
	wy = q[3] * y2;
	wz = q[3] * z2;

	m[0][0] = 1.0f - yy - zz;
	m[0][1] = xy + wz;
	m[0][2] = xz - wy;

	m[1][0] = xy - wz;
	m[1][1] = 1.0f - xx - zz;
	m[1][2] = yz + wx;

	m[2][0] = xz + wy;
	m[2][1] = yz - wx;
	m[2][2] = 1.0f - xx - yy;
}
#endif


void Quat_TransformVector (const quat_t q, const vec3_t v, vec3_t out)
{
	vec_t wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];
	xx = q[0] * x2; xy = q[0] * y2; xz = q[0] * z2;
	yy = q[1] * y2; yz = q[1] * z2; zz = q[2] * z2;
	wx = q[3] * x2; wy = q[3] * y2; wz = q[3] * z2;

	out[0] = (1.0f - yy - zz) * v[0] + (xy - wz) * v[1] + (xz + wy) * v[2];
	out[1] = (xy + wz) * v[0] + (1.0f - xx - zz) * v[1] + (yz - wx) * v[2];
	out[2] = (xz - wy) * v[0] + (yz + wx) * v[1] + (1.0f - xx - yy) * v[2];
}

void Quat_ConcatTransforms (const quat_t q1, const vec3_t v1, const quat_t q2, const vec3_t v2, quat_t q, vec3_t v)
{
	Quat_Multiply(q1, q2, q);
	Quat_TransformVector(q1, v2, v);
	v[0] += v1[0];
	v[1] += v1[1];
	v[2] += v1[2];
}

void Quat_FromEulerAngle (const vec3_t angle, quat_t q) // jitskm
{
	// http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/euleranglestoquaternions.html
	float yaw_over_2   = DEG2RAD(angle[YAW])   / 2.0f;
	float pitch_over_2 = DEG2RAD(angle[PITCH]) / 2.0f;
	float roll_over_2  = DEG2RAD(angle[ROLL])  / 2.0f;
	float cy = cos(yaw_over_2);
	float cp = cos(pitch_over_2);
	float cr = cos(roll_over_2);
	float sy = sin(yaw_over_2);
	float sp = sin(pitch_over_2);
	float sr = sin(roll_over_2);

	q[3] = cr*cp*cy + sr*sp*sy; // not sure if this should be 0 or 3
	q[1] = sr*cp*cy - cr*sp*sy; 
	q[2] = cr*sp*cy + sr*cp*sy;
	q[0] = cr*cp*sy + sr*sp*cy;
}

void Quat_ToEulerAngle (const quat_t q, vec3_t angle) // jitskm
{
	// http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/quaternionstoeulerangles.html#770931
	register float q0, q1, q2, q3, q02, q12, q22, q32;
	
	q0 = q[0]; q1 = q[1]; q2 = q[2]; q3 = q[3];
	q02 = q0*q0; q12 = q1*q1; q22 = q2*q2; q32 = q3*q3;

	angle[YAW]  = RAD2DEG(atan2(2.0f * (q2*q3 + q0*q1), (q02-q12-q22+q32))); // phi
	angle[ROLL] = RAD2DEG(Q_asin(-2.0f * (q1*q3 - q0*q2))); // theta
	angle[PITCH]   = RAD2DEG(atan2(2.0f * (q1*q2 + q0*q3), (q02+q12-q22-q32))); // psi
}

void Matrix3_Transpose (mat3_t in, mat3_t out)
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

void Matrix_TransformVector (vec3_t m[3], vec3_t v, vec3_t out)
{
	out[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	out[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	out[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}

/*
void Matrix_EulerAngles (vec3_t m[3], vec3_t angles)
{
	vec_t	c;
	vec_t	pitch, yaw, roll;

	pitch = -Q_asin(m[0][2]);
	c = cos(pitch);
	pitch = RAD2DEG(pitch);

	if (fabs(c) > 0.005)             // Gimball lock?
	{
		c = 1.0f / c;
		yaw = RAD2DEG(atan2(m[0][1] * c, m[0][0] * c));
		roll = RAD2DEG(atan2(-m[1][2] * c, m[2][2] * c));
	}
	else
	{
		if (m[0][2] > 0)
			pitch = -90;
		else
			pitch = 90;

		yaw = RAD2DEG(atan2(m[1][0], -m[1][1]));
		roll = 0;
	}

	angles[PITCH] = anglemod(pitch);
	angles[YAW]   = anglemod(yaw);
	angles[ROLL]  = anglemod(roll);
}
*/

void Matrix_EulerAngles2 (vec3_t m[3], vec3_t angles) // jitskm, modified to work the way I think it should.
{
	vec_t c;
	vec_t pitch = -Q_asin(m[0][2]);

	angles[PITCH] = RAD2DEG(pitch);
	c = 1.0f / fabs(cos(pitch));
	angles[YAW] = RAD2DEG(atan2(m[0][1] * c, m[0][0] * c));
	angles[ROLL] = -RAD2DEG(atan2(-m[1][2] * c, m[2][2] * c));
}


// jitskm
//====================================================================================

/*
============
COM_SkipPath
============
*/
char *COM_SkipPath (char *pathname)
{
	char *last;
	
	last = pathname;

	while (*pathname)
	{
		if (*pathname == '/')
			last = pathname + 1;

		pathname++;
	}

	return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension (const char *in, char *out, int out_size) // jitsecurity
{
	char *out_ext = NULL;
	int i = 1;

	while (*in && i < out_size)
	{
		*out++ = *in++;
		i++;

		if (*in == '.')
			out_ext = out;
	}

	if (out_ext)
		*out_ext = 0;
	else
		*out = 0;
}

/*
============
COM_FileExtension
============
*/
const char *COM_FileExtension (const char *in) // jit
{
	const char *out_ext = "";

	while (*in)
	{
		if (*in == '.')
			out_ext = in + 1;

		++in;
	}

	return out_ext;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase (char *in, char *out)
{
	char *s, *s2;
	
	s = in + strlen(in) - 1;
	
	while (s != in && *s != '.')
		s--;
	
	for (s2 = s ; s2 != in && *s2 != '/' ; s2--)
	;
	
	if (s-s2 < 2)
		out[0] = 0;
	else
	{
		s--;
		Q_strncpyz(out,s2+1, s-s2);
		out[s-s2] = 0;
	}
}

/*
============
COM_FilePath

Returns the path up to, but not including the last /
============
*/
void COM_FilePath (char *in, char *out)
{
	char *s;
	
	s = in + strlen(in) - 1;
	
	while (s != in && *s != '/')
		s--;

	strncpy(out, in, s-in); // Do NOT replace with Q_strncpyz
	out[s-in] = 0;
}


/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension (char *path, char *extension)
{
	char    *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strcat (path, extension);
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean	bigendien;

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
short	(*_BigShort) (short l);
short	(*_LittleShort) (short l);
int		(*_BigLong) (int l);
int		(*_LittleLong) (int l);
float	(*_BigFloat) (float l);
float	(*_LittleFloat) (float l);

short	BigShort(short l){return _BigShort(l);}
short	LittleShort(short l) {return _LittleShort(l);}
int		BigLong (int l) {return _BigLong(l);}
int		LittleLong (int l) {return _LittleLong(l);}
float	BigFloat (float l) {return _BigFloat(l);}
float	LittleFloat (float l) {return _LittleFloat(l);}

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int	LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap (float f)
{
	return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init (void)
{
	byte	swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner	
	if ( *(short *)swaptest == 1)
	{
		bigendien = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char	*va(char *format, ...)
{
	va_list		argptr;
	static char		string[1024];
	
	va_start(argptr, format);
	_vsnprintf(string, sizeof(string), format, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(string); // jitsecurity -- make sure string is null terminated.

	return string;	
}


char	com_token[MAX_TOKEN_CHARS];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char **data_p)
{
	unsigned char c; // jittext
	int		len;
	char	*data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;
	
	if (!data)
	{
		*data_p = NULL;
		return "";
	}

// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
		{
			*data_p = NULL;
			return "";
		}
		data++;
	}
	
// skip // comments
	if (c== '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;

		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;

		while (1)
		{
			c = *data++;

			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = data;
				return com_token;
			}

			if (len < MAX_TOKEN_CHARS - 1) // jitsecurity, [SkulleR] - buffer overrun fix
			{
				com_token[len] = c;
				len++;
			}
		}
	}

// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS - 1) // jitsecurity, [SkulleR] - buffer overrun fix
		{
			com_token[len] = c;
			len++;
		}

		data++;
		c = *data;
	} while (c > 32);

	if (len == MAX_TOKEN_CHARS)
	{
		Com_Printf("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}

	com_token[len] = 0;
	*data_p = data;
	return com_token;
}


/*
==============
COM_ParseExt

Parse a token out of a string
==============
*/
char *COM_ParseExt (char **data_p, qboolean nl) // jitrscript - from qfusion
{
	int		c;
	int		len;
	char	*data;
	qboolean newlines = false;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	if (!data)
	{
		*data_p = NULL;
		return "";
	}

// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
		{
			*data_p = NULL;
			return "";
		}
		if (c == '\n')
			newlines = true;
		data++;
	}

	if ( newlines && !nl ) {
		*data_p = data;
		return com_token;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = data;
				return com_token;
			}
			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c>32);

	if (len == MAX_TOKEN_CHARS)
	{
//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = data;
	return com_token;
}

/*
===============
Com_PageInMemory

===============
*/
int	paged_total;

void Com_PageInMemory (byte *buffer, int size)
{
	int		i;

	for (i=size-1 ; i>0 ; i-=4096)
		paged_total += buffer[i];
}



/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/


int Q_strncasecmp (const char *s1, const char *s2, int n)
{
	register int c1, c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return c1 - c2;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_strcasecmp (const char *s1, const char *s2)
{
	register int c1, c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return c1 - c2;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}


int Q_streq (const char *s1, const char *s2) // jit -- faster than !strcmp
{
	register int c1, c2;

	if (s1 && s2) // 1.831
	{
		do
		{
			c1 = *s1++;
			c2 = *s2++;
			
			if (c1 != c2)
				return 0;		// strings not equal
		} while (c1);
		
		return 1;		// strings are equal
	}
	else
	{
		if (s1 || s2)
			return 0;
		else
			return 1;
	}
}


int Q_strcaseeq (const char *s1, const char *s2) // jit
{
	register int c1, c2;

	if (s1 && s2)
	{
		do
		{
			c1 = *s1++;
			c2 = *s2++;

			if (c1 != c2)
			{
				if (c1 >= 'a' && c1 <= 'z')
					c1 -= ('a' - 'A');

				if (c2 >= 'a' && c2 <= 'z')
					c2 -= ('a' - 'A');

				if (c1 != c2)
					return 0; // strings not equal
			}
		} while (c1);
		
		return 1; // strings are equal
	}
	else
	{
		if (s1 || s2)
			return 0; // one null, one not, not equal.
		else
			return 1; // both null, "equal".
	}
}


void Com_sprintf (char *dest, int size, char *fmt, ...)
{
	int len;
	va_list argptr;
	char bigbuffer[0x10000];

	va_start(argptr, fmt);
	len = _vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(bigbuffer); // jitsecurity -- make sure string is null terminated.

	if (len >= size)
		Com_Printf("Com_sprintf: overflow of %i in %i.\n", len, size);

	assert(len < size); // jitdebug
	Q_strncpyz(dest, bigbuffer, size); // jitsecurity - make sure string is terminated.
}


/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
#define MAX_KEY_LEN 512 // jit
char *Info_ValueForKey (char *s, char *key)
{
	char	pkey[MAX_KEY_LEN];
	static	char value[2][MAX_KEY_LEN];	// use two buffers so compares work without stomping on each other
	static	int	valueindex = 0;
	char	*o;
	int		n;

	if (!s || !key) // jit
		return "";

	valueindex ^= 1;

	if (*s == '\\')
		s++;

	while (1)
	{
		o = pkey;
		n = 0; // jit

		while (*s != '\\')
		{
			if (!*s)
				return "";

			if (n < MAX_KEY_LEN) // jit
				*o++ = *s;

			n++;
			s++;
		}

		*o = 0;
		s++;
		o = value[valueindex];
		n = 0; // jit

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";

			if (n < MAX_KEY_LEN) // jit
				*o++ = *s;

			n++;
			s++;
		}

		*o = 0;

		if (Q_streq(key, pkey))
			return value[valueindex];

		if (!*s)
			return "";

		s++;
	}
}


void Info_RemoveKey (char *s, const char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strchr(key, '\\'))
	{
		//Com_Printf("Info_RemoveKey: Tried to remove illegal key '%s'.\n", LOG_WARNING|LOG_GENERAL, key);
		return;
	}

	for (;;)
	{
		start = s;

		if (*s == '\\')
			s++;

		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;
		s++;
		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;

		if (Q_streq(key, pkey))
		{
			//r1: overlapping src+dst with strcpy = no
			//strcpy (start, s);	// remove this part
			size_t memlen;

			memlen = strlen(s);
			memmove(start, s, memlen);
			*(start+memlen) = 0;
			Info_RemoveKey(start, key); // jit - recursively remove all keys
			return;
		}

		if (!*s)
			return;
	}
}


/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate (char *s)
{
	if (strstr (s, "\""))
		return false;

	if (strstr (s, ";"))
		return false;

	return true;
}


void Info_SetValueForKey (char *s, const char *key, const char *value)
{
	char	newi[MAX_INFO_STRING], *v;
	//int		c;
	unsigned char c; // jittext
	int		maxsize = MAX_INFO_STRING;

	if (strstr(key, "\\") || strstr(value, "\\"))
	{
		Com_Printf("Can't use keys or values with a \\.\n");
		return;
	}

	if (strstr(key, ";"))
	{
		Com_Printf("Can't use keys or values with a semicolon.\n");
		return;
	}

	if (strstr(key, "\"") || strstr(value, "\""))
	{
		Com_Printf("Can't use keys or values with a \".\n");
		return;
	}

	if (strlen(key) > MAX_INFO_KEY-1 || strlen(value) > MAX_INFO_KEY-1)
	{
		Com_Printf("Keys and values must be < 64 characters.\n");
		return;
	}

	Info_RemoveKey(s, key);

	if (!value || !strlen(value))
		return;

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) > maxsize)
	{
        Com_Printf("Info string length exceeded %s -- %s -- %s\n", s, key, value);
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = newi;

	while (*v)
	{
		c = *v++;
		// jittext c &= 127;		// strip high bits
		//if (c >= 32 && c < 127)
		if (c >= 32) // jittext
			*s++ = c;
	}

	*s = 0;
}

//====================================================================
// Hash Functions (c) 2004-2013 Nathan "jitspoe" Wulf
// jithash:
void hash_table_init (hash_table_t *table, unsigned int size, void *free_func)
{
	table->size = size;
	table->mask = size - 1;
	assert((size & table->mask) == 0); // size must be a power of 2.
	table->free_func = free_func;
	//table->table = Z_Malloc(sizeof(hash_node_t)*sizemask);
	table->table = malloc(sizeof(hash_node_t) * size);
	memset(table->table, 0, sizeof(hash_node_t) * size);
}

static void hash_node_free (hash_node_t *node, hash_table_t *table)
{
	if (table->free_func)
		table->free_func(node->data);

	//Z_Free(node->key);
	//Z_Free(node);
	free(node->key);
	free(node);
}

static void hash_node_free_recursive (hash_node_t *node, hash_table_t *table)
{
	if (node)
	{
		hash_node_free_recursive(node->next, table);
		hash_node_free(node, table);
	}
}

void hash_table_free (hash_table_t *table)
{
	register int i;
	register int size = table->size;
	
	for (i = 0; i < size; i++)
	{
		hash_node_free_recursive(table->table[i], table);
		table->table[i] = NULL;
	}

	//Z_Free(table->table);
	free(table->table);
	table->table = 0;
	table->free_func = NULL;
	table->size = 0;
	table->mask = 0;
}

void hash_print (hash_table_t *table)
{
	register int i, j, size = table->size;
	hash_node_t *node;

	for (i = 0; i < size; i++)
	{
		j = 0;
		node = table->table[i];

		while (node)
		{
			j++;
			node = node->next;
		}

		Com_Printf("%d: %d\n", i, j);
	}
}

#if 0
static unsigned short crctable[256] =
{
	0x0000,	0x1021,	0x2042,	0x3063,	0x4084,	0x50a5,	0x60c6,	0x70e7,
	0x8108,	0x9129,	0xa14a,	0xb16b,	0xc18c,	0xd1ad,	0xe1ce,	0xf1ef,
	0x1231,	0x0210,	0x3273,	0x2252,	0x52b5,	0x4294,	0x72f7,	0x62d6,
	0x9339,	0x8318,	0xb37b,	0xa35a,	0xd3bd,	0xc39c,	0xf3ff,	0xe3de,
	0x2462,	0x3443,	0x0420,	0x1401,	0x64e6,	0x74c7,	0x44a4,	0x5485,
	0xa56a,	0xb54b,	0x8528,	0x9509,	0xe5ee,	0xf5cf,	0xc5ac,	0xd58d,
	0x3653,	0x2672,	0x1611,	0x0630,	0x76d7,	0x66f6,	0x5695,	0x46b4,
	0xb75b,	0xa77a,	0x9719,	0x8738,	0xf7df,	0xe7fe,	0xd79d,	0xc7bc,
	0x48c4,	0x58e5,	0x6886,	0x78a7,	0x0840,	0x1861,	0x2802,	0x3823,
	0xc9cc,	0xd9ed,	0xe98e,	0xf9af,	0x8948,	0x9969,	0xa90a,	0xb92b,
	0x5af5,	0x4ad4,	0x7ab7,	0x6a96,	0x1a71,	0x0a50,	0x3a33,	0x2a12,
	0xdbfd,	0xcbdc,	0xfbbf,	0xeb9e,	0x9b79,	0x8b58,	0xbb3b,	0xab1a,
	0x6ca6,	0x7c87,	0x4ce4,	0x5cc5,	0x2c22,	0x3c03,	0x0c60,	0x1c41,
	0xedae,	0xfd8f,	0xcdec,	0xddcd,	0xad2a,	0xbd0b,	0x8d68,	0x9d49,
	0x7e97,	0x6eb6,	0x5ed5,	0x4ef4,	0x3e13,	0x2e32,	0x1e51,	0x0e70,
	0xff9f,	0xefbe,	0xdfdd,	0xcffc,	0xbf1b,	0xaf3a,	0x9f59,	0x8f78,
	0x9188,	0x81a9,	0xb1ca,	0xa1eb,	0xd10c,	0xc12d,	0xf14e,	0xe16f,
	0x1080,	0x00a1,	0x30c2,	0x20e3,	0x5004,	0x4025,	0x7046,	0x6067,
	0x83b9,	0x9398,	0xa3fb,	0xb3da,	0xc33d,	0xd31c,	0xe37f,	0xf35e,
	0x02b1,	0x1290,	0x22f3,	0x32d2,	0x4235,	0x5214,	0x6277,	0x7256,
	0xb5ea,	0xa5cb,	0x95a8,	0x8589,	0xf56e,	0xe54f,	0xd52c,	0xc50d,
	0x34e2,	0x24c3,	0x14a0,	0x0481,	0x7466,	0x6447,	0x5424,	0x4405,
	0xa7db,	0xb7fa,	0x8799,	0x97b8,	0xe75f,	0xf77e,	0xc71d,	0xd73c,
	0x26d3,	0x36f2,	0x0691,	0x16b0,	0x6657,	0x7676,	0x4615,	0x5634,
	0xd94c,	0xc96d,	0xf90e,	0xe92f,	0x99c8,	0x89e9,	0xb98a,	0xa9ab,
	0x5844,	0x4865,	0x7806,	0x6827,	0x18c0,	0x08e1,	0x3882,	0x28a3,
	0xcb7d,	0xdb5c,	0xeb3f,	0xfb1e,	0x8bf9,	0x9bd8,	0xabbb,	0xbb9a,
	0x4a75,	0x5a54,	0x6a37,	0x7a16,	0x0af1,	0x1ad0,	0x2ab3,	0x3a92,
	0xfd2e,	0xed0f,	0xdd6c,	0xcd4d,	0xbdaa,	0xad8b,	0x9de8,	0x8dc9,
	0x7c26,	0x6c07,	0x5c64,	0x4c45,	0x3ca2,	0x2c83,	0x1ce0,	0x0cc1,
	0xef1f,	0xff3e,	0xcf5d,	0xdf7c,	0xaf9b,	0xbfba,	0x8fd9,	0x9ff8,
	0x6e17,	0x7e36,	0x4e55,	0x5e74,	0x2e93,	0x3eb2,	0x0ed1,	0x1ef0
};
#endif

#if 1
#define HASHALG \
	c = *(s = key); \
	while(c) { \
		hashval += c; \
		c = *(++s); \
	}
#elif 0
#define HASHALG \
	s = key; \
	while(*s) \
		hashval = (hashval << 8) ^ crctable[(hashval >> 8) ^ *s++]; // this one sucks
#else
#define HASHALG \
	s = key;\
	while(*s)\
		hashval += *s++ ^ crctable[hashval & 0xFF];
#endif

void hash_add (hash_table_t *table, const unsigned char *key, void *data)
{
	unsigned int hashval = 0;
	register unsigned char c;
	hash_node_t *newnode;
	const unsigned char *s;

	hash_delete(table, key); // get rid of old value if it exists
	newnode = malloc(sizeof(hash_node_t));
	newnode->key = strdup(key);
	HASHALG;
	hashval &= table->mask;
	newnode->data = data;

#if 0 // jittest
	if (table->table[hashval])
	{
		static int hash_collisions=0;

		hash_collisions ++;
		Com_Printf("Hash collision %d: (%d) %s and %s.\n",
			hash_collisions, hashval,
			newnode->key, table->table[hashval]->key);
		hash_print(table);
	}
#endif

	newnode->next = table->table[hashval]; // account for collisions
	table->table[hashval] = newnode;
}
			
void *hash_get (hash_table_t *table, const unsigned char *key)
{
	register unsigned char c;
	register unsigned int hashval = 0;
	hash_node_t *node;
	const unsigned char *s;

	HASHALG;
	hashval &= table->mask;
	node = table->table[hashval];

	while (node)
	{
		if (Q_streq(node->key, key))
			return node->data;
		else
			node = node->next; // in case there were collisions
	}

	return NULL; // not found;
}

void hash_delete (hash_table_t *table, const unsigned char *key)
{
	register unsigned char c;
	unsigned int hashval = 0;
	hash_node_t *node;
	hash_node_t *prevnode;
	const unsigned char *s;

	HASHALG;
	hashval &= table->mask;
	prevnode = node = table->table[hashval];

	while(node)
	{
		if (Q_streq(node->key, key))
		{
			if (prevnode == node)
				table->table[hashval] = node->next;
			else
				prevnode->next = node->next;

			hash_node_free(node, table);
			return;
		}

		prevnode = node;
		node = node->next; // in case there were collisions
	}
}
// jithash
//====================================================================

qboolean FileExists (const char *filename) // jit
{
#ifdef WIN32
	return (_access(filename, 00) == 0);
#else
	return (access(filename, R_OK) == 0);
#endif
}

/*
==============
Q_snprintfz
==============
*/
int Q_snprintfz (char *dest, size_t size, const char *fmt, ...) // jitskm
{
	va_list	argptr;
	int len;

	if (size)
	{
		va_start(argptr, fmt);
		len = _vsnprintf(dest, size, fmt, argptr);
		va_end(argptr);
		dest[size-1] = 0;
		return len;
	}
	else
	{
		return 0;
	}
}

/*
==============
Q_strncpyz
==============
*/
void Q_strncpyz (char *dest, const char *src, size_t size)
{
	char *dest_cpy = dest;
	const char *src_cpy = src;
	size_t size_cpy = size; // for debugging

	if (size)
	{
		while (--size && (*dest++ = *src++));
		*dest = '\0';
		assert(size > 0);
	}
}

//==============
//Q_strncatz
// Taken from warsow source, original author unknown
//==============
void Q_strncatz (char *dest, const char *src, size_t size)
{
#ifdef HAVE_STRLCAT
	strlcat(dest, src, size);
#else
	if (size)
	{
		while (--size && *dest++);

		if (size)
		{
			dest--;
			size++;

			while (--size && (*dest++ = *src++));
		}

		*dest = '\0';
	}

	assert(size > 0);
#endif
}

// no-assert version, for buffers we know will overflow
void Q_strncpyzna (char *dest, const char *src, size_t size)
{
#ifdef HAVE_STRLCPY
	strlcpy(dest, src, size);
#else
	if (size)
	{
		while (--size && (*dest++ = *src++));
		*dest = '\0';
	}
#endif
}

#if 0 // tests showed this as being slower (5% overall FPS drop) than memcpy.  Maybe it's faster for aligned memory and would be useful later?
void *RB_MemCpy(void *dst, const void *src, size_t count)
{
	__asm
	{
		mov			esi, dword ptr [src]
		mov			edi, dword ptr [dst]

		cmp			dword ptr [count], 64
		jl			TryCopyQWord32

CopyQWord64:
		movq		mm0, [esi]
		movq		mm1, [esi + 8]
		movq		mm2, [esi + 16]
		movq		mm3, [esi + 24]
		movq		mm4, [esi + 32]
		movq		mm5, [esi + 40]
		movq		mm6, [esi + 48]
		movq		mm7, [esi + 56]
		add			esi, 64

		movntq		[edi], mm0
		movntq		[edi + 8], mm1
		movntq		[edi + 16], mm2
		movntq		[edi + 24], mm3
		movntq		[edi + 32], mm4
		movntq		[edi + 40], mm5
		movntq		[edi + 48], mm6
		movntq		[edi + 56], mm7
		add			edi, 64

		sub			dword ptr [count], 64
		cmp			dword ptr [count], 64
		jge			CopyQWord64

TryCopyQWord32:
		cmp			dword ptr [count], 32
		jl			TryCopyQWord16

CopyQWord32:
		movq		mm0, [esi]
		movq		mm1, [esi + 8]
		movq		mm2, [esi + 16]
		movq		mm3, [esi + 24]
		add			esi, 32

		movntq		[edi], mm0
		movntq		[edi + 8], mm1
		movntq		[edi + 16], mm2
		movntq		[edi + 24], mm3
		add			edi, 32

		sub			dword ptr [count], 32
		cmp			dword ptr [count], 32
		jge			CopyQWord32

TryCopyQWord16:
		cmp			dword ptr [count], 16
		jl			TryCopyQWord8

CopyQWord16:
		movq		mm0, [esi]
		movq		mm1, [esi + 8]
		add			esi, 16

		movntq		[edi], mm0
		movntq		[edi + 8], mm1
		add			edi, 16

		sub			dword ptr [count], 16
		cmp			dword ptr [count], 16
		jge			CopyQWord16

TryCopyQWord8:
		cmp			dword ptr [count], 8
		jl			TryCopyDWord

CopyQWord8:
		movq		mm0, [esi]
		add			esi, 8

		movntq		[edi], mm0
		add			edi, 8

		sub			dword ptr [count], 8
		cmp			dword ptr [count], 8
		jge			CopyQWord8

TryCopyDWord:
		cmp			dword ptr [count], 3
		jle			TryCopyWord

		mov			ecx, dword ptr [count]
		shr			ecx, 2
		mov			eax, ecx
		rep	movsd

		shl			eax, 2
		sub			dword ptr [count], eax

TryCopyWord:
		cmp			dword ptr [count], 1
		jle			TryCopyByte

		movsw

		sub			dword ptr [count], 2

TryCopyByte:
		cmp			dword ptr [count], 0
		je			CopyDone

		movsb

CopyDone:
		emms
		sfence
		mov			eax, [dst]
	}
}
#endif


// Remap for extended codes so they don't mess up the console.
static char char_remap[256] = {
	'\0','-', '-', '-', '_', '*', 't', '.', 'N', '-', '\n','#', '.', '>', '*', '*',
	'[', ']', '@', '@', '@', '@', '@', '@', '<', '>', '.', '-', '*', '-', '-', '-',
	' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '<',
	'(', '=', ')', '^', '!', 'O', 'U', 'I', 'C', 'C', 'R', '#', '?', '>', '*', '*',
	'[', ']', '@', '@', '@', '@', '@', '@', '<', '>', '*', 'X', '*', '-', '-', '-',
	' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
	'`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', '<'
};

// strip out the garbage characters such as color codes
// and map extended ascii to something readable...
void strip_garbage (char *cout, const char *cin, size_t size_out) // jit b43
{
	register const unsigned char *s;
	register unsigned char *sbuf;
	unsigned char *out = (unsigned char *)cout; // stupid gcc warnings
	unsigned char *in = (unsigned char *)cin;
	register unsigned char *sbuf_max = out + size_out - 1;
	int index;

	for (sbuf = out, s = in; *s; ++s)
	{
		switch (*s)
		{
		case UCHAR_COLOR:
			if (*(s + 1))
				++s;
			break;
		case UCHAR_ITALICS:
		case UCHAR_UNDERLINE:
		case UCHAR_ENDFORMAT:
			break;
		default:
			index = *s;
			assert(index >= 0 && index < 256);
			*sbuf = char_remap[index];
			++sbuf;

			if (sbuf >= sbuf_max)
			{
				assert(s[1] == NULL); // ok if sbuf == sbuf_max as long as s + 1 is null, otherwise we're truncating strings.
				*sbuf = 0;
				return;
			}
		}
	}

	*sbuf = 0;
}


// === jitsimd: Copied from DarkPlaces with minor modifications...

// for x86 cpus only...  (x64 has SSE2_PRESENT)
// code from SDL, shortened as we can expect CPUID to work
static int CPUID_Features(void)
{
	int features = 0;
# if defined(__GNUC__) && defined(__i386__)
        __asm__ (
"        movl    %%ebx,%%edi\n"
"        xorl    %%eax,%%eax                                           \n"
"        incl    %%eax                                                 \n"
"        cpuid                       # Get family/model/stepping/features\n"
"        movl    %%edx,%0                                              \n"
"        movl    %%edi,%%ebx\n"
        : "=m" (features)
        :
        : "%eax", "%ecx", "%edx", "%edi"
        );
# elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
        __asm {
        xor     eax, eax
        inc     eax
        cpuid                       ; Get family/model/stepping/features
        mov     features, edx
        }
# else
#  error no CPUID implementation
# endif
	return features;
}

qboolean Sys_HaveSSE (void)
{/*
	// COMMANDLINEOPTION: SSE: -nosse disables SSE support and detection
	if (COM_CheckParm("-nosse"))
		return false;

	// COMMANDLINEOPTION: SSE: -forcesse enables SSE support and disables detection
	if (COM_CheckParm("-forcesse") || COM_CheckParm("-forcesse2"))
		return true;
*/
	if (CPUID_Features() & (1 << 25))
		return true;

	return false;
}
// jitsimd ===

