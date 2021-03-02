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
// mathlib.c
//

#include "shared.h"

vec2_t vec2Origin =
{
	0, 0
};

vec3_t vec3Origin =
{
	0, 0, 0
};

vec4_t vec4Origin =
{
	0, 0, 0, 0
};

/*
=============================================================================

	MATHLIB

=============================================================================
*/

vec3_t m_byteDirs[NUMVERTEXNORMALS] = {
	{-0.525731f,	0.000000f,	0.850651f},		{-0.442863f,	0.238856f,	0.864188f},		{-0.295242f,	0.000000f,	0.955423f},
	{-0.309017f,	0.500000f,	0.809017f},		{-0.162460f,	0.262866f,	0.951056f},		{0.000000f,		0.000000f,	1.000000f},
	{0.000000f,		0.850651f,	0.525731f},		{-0.147621f,	0.716567f,	0.681718f},		{0.147621f,		0.716567f,	0.681718f},
	{0.000000f,		0.525731f,	0.850651f},		{0.309017f,		0.500000f,	0.809017f},		{0.525731f,		0.000000f,	0.850651f},
	{0.295242f,		0.000000f,	0.955423f},		{0.442863f,		0.238856f,	0.864188f},		{0.162460f,		0.262866f,	0.951056f},
	{-0.681718f,	0.147621f,	0.716567f},		{-0.809017f,	0.309017f,	0.500000f},		{-0.587785f,	0.425325f,	0.688191f},
	{-0.850651f,	0.525731f,	0.000000f},		{-0.864188f,	0.442863f,	0.238856f},		{-0.716567f,	0.681718f,	0.147621f},
	{-0.688191f,	0.587785f,	0.425325f},		{-0.500000f,	0.809017f,	0.309017f},		{-0.238856f,	0.864188f,	0.442863f},
	{-0.425325f,	0.688191f,	0.587785f},		{-0.716567f,	0.681718f,	-0.147621f},	{-0.500000f,	0.809017f,	-0.309017f},
	{-0.525731f,	0.850651f,	0.000000f},		{0.000000f,		0.850651f,	-0.525731f},	{-0.238856f,	0.864188f,	-0.442863f},
	{0.000000f,		0.955423f,	-0.295242f},	{-0.262866f,	0.951056f,	-0.162460f},	{0.000000f,		1.000000f,	0.000000f},
	{0.000000f,		0.955423f,	0.295242f},		{-0.262866f,	0.951056f,	0.162460f},		{0.238856f,		0.864188f,	0.442863f},
	{0.262866f,		0.951056f,	0.162460f},		{0.500000f,		0.809017f,	0.309017f},		{0.238856f,		0.864188f,	-0.442863f},
	{0.262866f,		0.951056f,	-0.162460f},	{0.500000f,		0.809017f,	-0.309017f},	{0.850651f,		0.525731f,	0.000000f},
	{0.716567f,		0.681718f,	0.147621f},		{0.716567f,		0.681718f,	-0.147621f},	{0.525731f,		0.850651f,	0.000000f},
	{0.425325f,		0.688191f,	0.587785f},		{0.864188f,		0.442863f,	0.238856f},		{0.688191f,		0.587785f,	0.425325f},
	{0.809017f,		0.309017f,	0.500000f},		{0.681718f,		0.147621f,	0.716567f},		{0.587785f,		0.425325f,	0.688191f},
	{0.955423f,		0.295242f,	0.000000f},		{1.000000f,		0.000000f,	0.000000f},		{0.951056f,		0.162460f,	0.262866f},
	{0.850651f,		-0.525731f,	0.000000f},		{0.955423f,		-0.295242f,	0.000000f},		{0.864188f,		-0.442863f,	0.238856f},
	{0.951056f,		-0.162460f,	0.262866f},		{0.809017f,		-0.309017f,	0.500000f},		{0.681718f,		-0.147621f,	0.716567f},
	{0.850651f,		0.000000f,	0.525731f},		{0.864188f,		0.442863f,	-0.238856f},	{0.809017f,		0.309017f,	-0.500000f},
	{0.951056f,		0.162460f,	-0.262866f},	{0.525731f,		0.000000f,	-0.850651f},	{0.681718f,		0.147621f,	-0.716567f},
	{0.681718f,		-0.147621f,	-0.716567f},	{0.850651f,		0.000000f,	-0.525731f},	{0.809017f,		-0.309017f,	-0.500000f},
	{0.864188f,		-0.442863f,	-0.238856f},	{0.951056f,		-0.162460f,	-0.262866f},	{0.147621f,		0.716567f,	-0.681718f},
	{0.309017f,		0.500000f,	-0.809017f},	{0.425325f,		0.688191f,	-0.587785f},	{0.442863f,		0.238856f,	-0.864188f},
	{0.587785f,		0.425325f,	-0.688191f},	{0.688191f,		0.587785f,	-0.425325f},	{-0.147621f,	0.716567f,	-0.681718f},
	{-0.309017f,	0.500000f,	-0.809017f},	{0.000000f,		0.525731f,	-0.850651f},	{-0.525731f,	0.000000f,	-0.850651f},
	{-0.442863f,	0.238856f,	-0.864188f},	{-0.295242f,	0.000000f,	-0.955423f},	{-0.162460f,	0.262866f,	-0.951056f},
	{0.000000f,		0.000000f,	-1.000000f},	{0.295242f,		0.000000f,	-0.955423f},	{0.162460f,		0.262866f,	-0.951056f},
	{-0.442863f,	-0.238856f,	-0.864188f},	{-0.309017f,	-0.500000f,	-0.809017f},	{-0.162460f,	-0.262866f,	-0.951056f},
	{0.000000f,		-0.850651f,	-0.525731f},	{-0.147621f,	-0.716567f,	-0.681718f},	{0.147621f,		-0.716567f,	-0.681718f},
	{0.000000f,		-0.525731f,	-0.850651f},	{0.309017f,		-0.500000f,	-0.809017f},	{0.442863f,		-0.238856f,	-0.864188f},
	{0.162460f,		-0.262866f,	-0.951056f},	{0.238856f,		-0.864188f,	-0.442863f},	{0.500000f,		-0.809017f,	-0.309017f},
	{0.425325f,		-0.688191f,	-0.587785f},	{0.716567f,		-0.681718f,	-0.147621f},	{0.688191f,		-0.587785f,	-0.425325f},
	{0.587785f,		-0.425325f,	-0.688191f},	{0.000000f,		-0.955423f,	-0.295242f},	{0.000000f,		-1.000000f,	0.000000f},
	{0.262866f,		-0.951056f,	-0.162460f},	{0.000000f,		-0.850651f,	0.525731f},		{0.000000f,		-0.955423f,	0.295242f},
	{0.238856f,		-0.864188f,	0.442863f},		{0.262866f,		-0.951056f,	0.162460f},		{0.500000f,		-0.809017f,	0.309017f},
	{0.716567f,		-0.681718f,	0.147621f},		{0.525731f,		-0.850651f,	0.000000f},		{-0.238856f,	-0.864188f,	-0.442863f},
	{-0.500000f,	-0.809017f,	-0.309017f},	{-0.262866f,	-0.951056f,	-0.162460f},	{-0.850651f,	-0.525731f,	0.000000f},
	{-0.716567f,	-0.681718f,	-0.147621f},	{-0.716567f,	-0.681718f,	0.147621f},		{-0.525731f,	-0.850651f,	0.000000f},
	{-0.500000f,	-0.809017f,	0.309017f},		{-0.238856f,	-0.864188f,	0.442863f},		{-0.262866f,	-0.951056f,	0.162460f},
	{-0.864188f,	-0.442863f,	0.238856f},		{-0.809017f,	-0.309017f,	0.500000f},		{-0.688191f,	-0.587785f,	0.425325f},
	{-0.681718f,	-0.147621f,	0.716567f},		{-0.442863f,	-0.238856f,	0.864188f},		{-0.587785f,	-0.425325f,	0.688191f},
	{-0.309017f,	-0.500000f,	0.809017f},		{-0.147621f,	-0.716567f,	0.681718f},		{-0.425325f,	-0.688191f,	0.587785f},
	{-0.162460f,	-0.262866f,	0.951056f},		{0.442863f,		-0.238856f,	0.864188f},		{0.162460f,		-0.262866f,	0.951056f},
	{0.309017f,		-0.500000f,	0.809017f},		{0.147621f,		-0.716567f,	0.681718f},		{0.000000f,		-0.525731f,	0.850651f},
	{0.425325f,		-0.688191f,	0.587785f},		{0.587785f,		-0.425325f,	0.688191f},		{0.688191f,		-0.587785f,	0.425325f},
	{-0.955423f,	0.295242f,	0.000000f},		{-0.951056f,	0.162460f,	0.262866f},		{-1.000000f,	0.000000f,	0.000000f},
	{-0.850651f,	0.000000f,	0.525731f},		{-0.955423f,	-0.295242f,	0.000000f},		{-0.951056f,	-0.162460f,	0.262866f},
	{-0.864188f,	0.442863f,	-0.238856f},	{-0.951056f,	0.162460f,	-0.262866f},	{-0.809017f,	0.309017f,	-0.500000f},
	{-0.864188f,	-0.442863f,	-0.238856f},	{-0.951056f,	-0.162460f,	-0.262866f},	{-0.809017f,	-0.309017f,	-0.500000f},
	{-0.681718f,	0.147621f,	-0.716567f},	{-0.681718f,	-0.147621f,	-0.716567f},	{-0.850651f,	0.000000f,	-0.525731f},
	{-0.688191f,	0.587785f,	-0.425325f},	{-0.587785f,	0.425325f,	-0.688191f},	{-0.425325f,	0.688191f,	-0.587785f},
	{-0.425325f,	-0.688191f,	-0.587785f},	{-0.587785f,	-0.425325f,	-0.688191f},	{-0.688191f,	-0.587785f,	-0.425325f}
};

/*
=================
DirToByte

This isn't a real cheap function to call!
=================
*/
byte DirToByte(const vec3_t dirVec)
{
	if (!dirVec)
		return 0;

	byte best = 0;
	float bestDot = 0;
	for (byte i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		float dot = DotProduct(dirVec, m_byteDirs[i]);
		if (dot > bestDot)
		{
			bestDot = dot;
			best = i;
		}
	}

	return best;
}


/*
=================
ByteToDir
=================
*/
void ByteToDir(const byte dirByte, vec3_t dirVec)
{
	if (dirByte >= NUMVERTEXNORMALS)
	{
		Vec3Clear(dirVec);
		return;
	}

	Vec3Copy(m_byteDirs[dirByte], dirVec);
}

// ===========================================================================

/*
==============
FloatToByte
==============
*/
byte FloatToByte(float x)
{
	union
	{
		float			f;
		uint32			i;
	} f2i;

	// Shift float to have 8bit fraction at base of number
	f2i.f = x + 32768.0f;
	f2i.i &= 0x7FFFFF;

	// Then read as integer and kill float bits...
	return (byte)min(f2i.i, 255);
}


/*
===============
ColorNormalizef
===============
*/
float ColorNormalizef(const float *in, float *out)
{
	float f = max(max(in[0], in[1]), in[2]);

	if (f > 1.0f)
	{
		f = 1.0f / f;
		out[0] = in[0] * f;
		out[1] = in[1] * f;
		out[2] = in[2] * f;
	}
	else
	{
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
	}

	return f;
}


/*
===============
ColorNormalizeb
===============
*/
float ColorNormalizeb(const float *in, byte *out)
{
	float f = max(max(in[0], in[1]), in[2]);

	if (f > 1.0f)
	{
		f = 1.0f / f;
		out[0] = FloatToByte(in[0] * f);
		out[1] = FloatToByte(in[1] * f);
		out[2] = FloatToByte(in[2] * f);
	}
	else
	{
		out[0] = FloatToByte(in[0]);
		out[1] = FloatToByte(in[1]);
		out[2] = FloatToByte(in[2]);
	}

	return f;
}

// ===========================================================================

/*
===============
Q_ftol
===============
*/
#ifdef id386
__declspec_naked long Q_ftol(float f)
{
	static int	tmp;
	__asm {
		fld dword ptr [esp+4]
		fistp tmp
		mov eax, tmp
		ret
	}
}
#endif // id386


/*
===============
Q_FastSqrt

5% margin of error
===============
*/
#ifdef id386
float Q_FastSqrt(float value)
{
	float result;
	__asm {
		mov eax, value
		sub eax, 0x3f800000
		sar eax, 1
		add eax, 0x3f800000
		mov result, eax
	}
	return result;
}
#endif // id386


/*
===============
Q_RSqrtf

1/sqrt, faster but not as precise
===============
*/
float Q_RSqrtf(float number)
{
	float	y;

	if (number == 0.0f)
		return 0.0f;
	*((int *)&y) = 0x5f3759df - ((* (int *) &number) >> 1);
	return y * (1.5f - (number * 0.5f * y * y));
}


/*
===============
Q_RSqrtd

1/sqrt, faster but not as precise
===============
*/
double Q_RSqrtd(double number)
{
	double	y;

	if (number == 0.0)
		return 0.0;
	*((int *)&y) = 0x5f3759df - ((* (int *) &number) >> 1);
	return y * (1.5f - (number * 0.5 * y * y));
}


/*
===============
Q_log2
===============
*/
int Q_log2(int val)
{
	int answer = 0;
	while(val >>= 1)
		answer++;
	return answer;
}

// ===========================================================================

/*
====================
Q_CalcFovY

Calculates aspect based on fovX and the screen dimensions
====================
*/
float Q_CalcFovY (float fovX, float width, float height)
{
	if (fovX < 1 || fovX > 179)
		Com_Printf (PRNT_ERROR, "Bad fov: %f\n", fovX);

	return (atanf(height / (width / tan (fovX / 360.0f * M_PI)))) * ((180.0f / M_PI) * 2);
}


// ===========================================================================

/*
===============
NormToLatLong
===============
*/
void NormToLatLong(const vec3_t normal, byte out[2])
{
	if (normal[0] == 0 && normal[1] == 0)
	{
		if (normal[2] > 0)
		{
			out[0] = 0;
			out[1] = 0;
		}
		else
		{
			out[0] = 128;
			out[1] = 0;
		}
	}
	else
	{
		int		angle;

		angle = (int)(acosf(normal[2]) * 255.0 / (M_PI * 2.0f)) & 0xff;
		out[0] = angle;
		angle = (int)(atan2f(normal[1], normal[0]) * 255.0 / (M_PI * 2.0f)) & 0xff;
		out[1] = angle;
	}
}

// for(int i=0; i<256 ; i++) { r_sintableByte[i] = sin((float)i / 255.0f * (M_PI * 2.0f)); }
static const float r_sintableByte[] =
{
	0.000000f,	0.024637f,	0.049260f,	0.073853f,
	0.098400f,	0.122888f,	0.147302f,	0.171626f,
	0.195845f,	0.219946f,	0.243914f,	0.267733f,
	0.291390f,	0.314870f,	0.338158f,	0.361242f,
	0.384106f,	0.406737f,	0.429121f,	0.451244f,
	0.473094f,	0.494656f,	0.515918f,	0.536867f,
	0.557489f,	0.577774f,	0.597707f,	0.617278f,
	0.636474f,	0.655284f,	0.673696f,	0.691698f,
	0.709281f,	0.726434f,	0.743145f,	0.759405f,
	0.775204f,	0.790532f,	0.805381f,	0.819740f,
	0.833602f,	0.846958f,	0.859800f,	0.872120f,
	0.883910f,	0.895163f,	0.905873f,	0.916034f,
	0.925638f,	0.934680f,	0.943154f,	0.951057f,
	0.958381f,	0.965124f,	0.971281f,	0.976848f,
	0.981823f,	0.986201f,	0.989980f,	0.993159f,
	0.995734f,	0.997705f,	0.999070f,	0.999829f,
	0.999981f,	0.999526f,	0.998464f,	0.996795f,
	0.994522f,	0.991645f,	0.988165f,	0.984086f,
	0.979410f,	0.974139f,	0.968276f,	0.961826f,
	0.954791f,	0.947177f,	0.938988f,	0.930229f,
	0.920905f,	0.911023f,	0.900587f,	0.889604f,
	0.878081f,	0.866025f,	0.853444f,	0.840344f,
	0.826734f,	0.812622f,	0.798017f,	0.782928f,
	0.767363f,	0.751332f,	0.734845f,	0.717912f,
	0.700543f,	0.682749f,	0.664540f,	0.645928f,
	0.626924f,	0.607539f,	0.587785f,	0.567675f,
	0.547219f,	0.526432f,	0.505325f,	0.483911f,
	0.462204f,	0.440216f,	0.417960f,	0.395451f,
	0.372702f,	0.349726f,	0.326539f,	0.303153f,
	0.279583f,	0.255843f,	0.231948f,	0.207912f,
	0.183749f,	0.159476f,	0.135105f,	0.110653f,
	0.086133f,	0.061561f,	0.036951f,	0.012320f,
	-0.012320f,	-0.036952f,	-0.061561f,	-0.086133f,
	-0.110653f,	-0.135105f,	-0.159476f,	-0.183750f,
	-0.207912f,	-0.231948f,	-0.255843f,	-0.279583f,
	-0.303153f,	-0.326539f,	-0.349727f,	-0.372702f,
	-0.395451f,	-0.417960f,	-0.440216f,	-0.462204f,
	-0.483912f,	-0.505325f,	-0.526432f,	-0.547220f,
	-0.567675f,	-0.587785f,	-0.607539f,	-0.626924f,
	-0.645928f,	-0.664540f,	-0.682749f,	-0.700543f,
	-0.717912f,	-0.734845f,	-0.751332f,	-0.767363f,
	-0.782928f,	-0.798017f,	-0.812622f,	-0.826734f,
	-0.840344f,	-0.853444f,	-0.866025f,	-0.878081f,
	-0.889604f,	-0.900587f,	-0.911023f,	-0.920906f,
	-0.930229f,	-0.938988f,	-0.947177f,	-0.954791f,
	-0.961826f,	-0.968276f,	-0.974139f,	-0.979410f,
	-0.984086f,	-0.988165f,	-0.991645f,	-0.994522f,
	-0.996795f,	-0.998464f,	-0.999526f,	-0.999981f,
	-0.999829f,	-0.999070f,	-0.997705f,	-0.995734f,
	-0.993159f,	-0.989980f,	-0.986201f,	-0.981823f,
	-0.976848f,	-0.971281f,	-0.965124f,	-0.958381f,
	-0.951056f,	-0.943154f,	-0.934680f,	-0.925638f,
	-0.916034f,	-0.905873f,	-0.895163f,	-0.883910f,
	-0.872119f,	-0.859800f,	-0.846958f,	-0.833602f,
	-0.819740f,	-0.805381f,	-0.790532f,	-0.775204f,
	-0.759405f,	-0.743145f,	-0.726433f,	-0.709281f,
	-0.691698f,	-0.673696f,	-0.655284f,	-0.636474f,
	-0.617278f,	-0.597707f,	-0.577774f,	-0.557489f,
	-0.536866f,	-0.515918f,	-0.494656f,	-0.473093f,
	-0.451244f,	-0.429120f,	-0.406736f,	-0.384106f,
	-0.361242f,	-0.338158f,	-0.314869f,	-0.291390f,
	-0.267733f,	-0.243914f,	-0.219946f,	-0.195845f,
	-0.171626f,	-0.147302f,	-0.122888f,	-0.098400f,
	-0.073852f,	-0.049260f,	-0.024637f,	0.000000f
};

/*
===============
LatLongToNorm
===============
*/
void LatLongToNorm(const byte latLong[2], vec3_t out)
{
	float cos_a = r_sintableByte[(latLong[0] + 64) & 255];
	float sin_a = r_sintableByte[latLong[0]];
	float cos_b = r_sintableByte[(latLong[1] + 64) & 255];
	float sin_b = r_sintableByte[latLong[1]];

	Vec3Set(out, cos_b * sin_a, sin_b * sin_a, cos_a);
}


/*
===============
MakeNormalVectorsf
===============
*/
void MakeNormalVectorsf(const vec3_t forward, vec3_t right, vec3_t up)
{
	// This rotate and negate guarantees a vector not colinear with the original
	Vec3Set(right, forward[2], -forward[0], forward[1]);

	float d = DotProduct(right, forward);
	Vec3MA(right, -d, forward, right);
	VectorNormalizef(right, right);
	CrossProduct(right, forward, up);
}


/*
===============
PerpendicularVector

Assumes "src" is normalized
===============
*/
void PerpendicularVector(const vec3_t src, vec3_t dst)
{
	int		pos = 5;
	float	minElem = 1.0f;
	vec3_t	tempVec;

	// Find the smallest magnitude axially aligned vector
	if (fabs(src[0]) < minElem)
	{
		pos = 0;
		minElem = fabsf(src[0]);
	}
	if (fabs(src[1]) < minElem)
	{
		pos = 1;
		minElem = fabsf(src[1]);
	}
	if (fabs(src[2]) < minElem)
	{
		pos = 2;
		minElem = fabsf(src[2]);
	}

	assert(pos != 5);

	Vec3Clear(tempVec);
	tempVec[pos] = 1.0F;

	// Project the point onto the plane defined by src
	ProjectPointOnPlane(dst, tempVec, src);

	// Normalize the result
	VectorNormalizef(dst, dst);
}


/*
===============
RotatePointAroundVector
===============
*/
void RotatePointAroundVector(vec3_t dest, const vec3_t dir, const vec3_t point, const float degrees)
{
	float c, s;
	Q_SinCosf(DEG2RAD(degrees), &s, &c);

	vec3_t vr, vu;
	MakeNormalVectorsf(dir, vr, vu);

	float t0, t1;
	t0 = vr[0] * c + vu[0] * -s;
	t1 = vr[0] * s + vu[0] *  c;
	dest[0] = (t0 * vr[0] + t1 * vu[0] + dir[0] * dir[0]) * point[0]
			+ (t0 * vr[1] + t1 * vu[1] + dir[0] * dir[1]) * point[1]
			+ (t0 * vr[2] + t1 * vu[2] + dir[0] * dir[2]) * point[2];

	t0 = vr[1] * c + vu[1] * -s;
	t1 = vr[1] * s + vu[1] *  c;
	dest[1] = (t0 * vr[0] + t1 * vu[0] + dir[1] * dir[0]) * point[0]
			+ (t0 * vr[1] + t1 * vu[1] + dir[1] * dir[1]) * point[1]
			+ (t0 * vr[2] + t1 * vu[2] + dir[1] * dir[2]) * point[2];

	t0 = vr[2] * c + vu[2] * -s;
	t1 = vr[2] * s + vu[2] *  c;
	dest[2] = (t0 * vr[0] + t1 * vu[0] + dir[2] * dir[0]) * point[0]
			+ (t0 * vr[1] + t1 * vu[1] + dir[2] * dir[1]) * point[1]
			+ (t0 * vr[2] + t1 * vu[2] + dir[2] * dir[2]) * point[2];
}


/*
===============
VectorNormalizef
===============
*/
float VectorNormalizef(const vec3_t in, vec3_t out)
{
	float length = Vec3Length(in);
	if (length)
	{
		float invLength = 1.0f/length;
		Vec3Scale(in, invLength, out);
	}
	else
	{
		Vec3Clear(out);
	}
		
	return length;
}


/*
===============
VectorNormalizeFastf
===============
*/
float VectorNormalizeFastf(vec3_t v)
{
	float invLength = Q_RSqrtf(DotProduct(v,v));

	v[0] *= invLength;
	v[1] *= invLength;
	v[2] *= invLength;

	if (invLength != 0)
		return 1.0f / invLength;

	return 0.0f;
}
