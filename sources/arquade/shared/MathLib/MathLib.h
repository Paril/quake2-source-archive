/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or v

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// MathLib.h

// ===========================================================================

class  vec3f; // forward declaration

extern vec2_t	vec2Origin;
#ifndef GAME_IS_BEING_COMPILED_NOT_ENGINE_GO_AWAY
extern vec4_t	vec4Origin;
#endif

#if SHARED_ALLOW_4x4_MATRIX
extern mat4x4_t	mat4x4Identity;
#endif

#if SHARED_ALLOW_3x3_MATRIX
extern mat3x3_t	axisIdentity;
#endif

#if SHARED_ALLOW_QUATERNIONS
extern quat_t	quatIdentity;
#endif

// ===========================================================================

#ifndef M_PI
# define M_PI			3.14159265358979323846f		// matches value in gcc v2 math.h
#endif

// angle indexes
enum
{
	PITCH,		// up / down
	YAW,		// left / right
	ROLL		// fall over
};

inline float DEG2RAD (const float v)
{
	return v * (M_PI / 180.0f);
}

inline float RAD2DEG (const float v)
{
	return v * (180.0f / M_PI);
}


inline sint16 ANGLE2SHORT (float x)
{
	return ((sint32)(x*65536/360) & 65535);
}

inline float SHORT2ANGLE (sint16 x)
{
	return (x*(360.0f/65536));
}


inline uint8 ANGLE2BYTE (float x)
{
	return ((sint32)(x*256/360) & 255);
}

inline float BYTE2ANGLE (uint8 x)
{
	return (x*(360.0f/256));
}

// ===========================================================================

#define LARGE_EPSILON 0.1
#define SMALL_EPSILON 0.01
#define TINY_EPSILON 0.001

#define TINY_NUMBER (1.e-8)
#define SMALL_NUMBER (1.e-4)

#ifndef ArrayCount
# define ArrayCount(a) (sizeof(a)/sizeof((a)[0]))
#endif

// ===========================================================================

uint8		FloatToByte(float x);

float		ColorNormalizef(const float *in, float *out);
float		ColorNormalizeb(const float *in, uint8 *out);

// ===========================================================================

#ifdef id386
long	Q_ftol (float f);
float	Q_FastSqrt (float value);
#else // id386
inline long Q_ftol (float f) { return ((long)f); }
inline float Q_FastSqrt (float value) { return sqrt(value); }
#endif // id386
inline float Q_fabs (float val) { return ((val < 0.f) ? -val : val); }

// Template functions
template <class CType, sint32 numElements>
inline void VecxAdd (const CType a, const CType b, CType c)
{
	for (sint32 i = 0; i < numElements; i++)
		c[i] = a[i] + b[i];
}

template <class CType, sint32 numElements>
inline void VecxAverage (const CType a, const CType b, CType c)
{
	for (sint32 i = 0; i < numElements; i++)
		c[i] = (a[i] + b[i])*0.5f;
}

template <class CType, sint32 numElements>
inline void VecxClear (CType a)
{
	for (sint32 i = 0; i < numElements; i++)
		*(sint32*)&a[i] = 0;
}

template <class CType, sint32 numElements>
inline bool VecxCompare (const CType a, const CType b)
{
	for (sint32 i = 0; i < numElements; i++)
	{
		if (a[i] != b[i])
			return false;
	}
	return true;
}

template <class CType, sint32 numElements>
inline void VecxCopy (const CType a, CType b)
{
	for (sint32 i = 0; i < numElements; i++)
		b[i] = a[i];
}

template <class CType, sint32 numElements>
inline void VecxIdentity (CType a)
{
	for (sint32 i = 0; i < numElements; i++)
		a[i] = 0;
}

template <class CType, sint32 numElements>
inline void VecxInverse (CType a)
{
	for (sint32 i = 0; i < numElements; i++)
		a[i] = -a[i];
}

template <class CType, sint32 numElements>
inline void VecxMA (const CType v, const float scale, const CType b, CType o)
{
	for (sint32 i = 0; i < numElements; i++)
		o[i] = v[i]+b[i]*scale;
}

template <class CType, sint32 numElements>
inline void VecxNegate (const CType a, CType b)
{
	for (sint32 i = 0; i < numElements; i++)
		b[i] = -a[i];
}

template <class CType, sint32 numElements>
inline void VecxScale (const CType in, const float scale, CType out)
{
	for (sint32 i = 0; i < numElements; i++)
		out[i] = in[i]*scale;
}

template <class CType, sint32 numElements>
inline void VecxSubtract (const CType a, const CType b, CType c)
{
	for (sint32 i = 0; i < numElements; i++)
		c[i] = a[i]-b[i];
}

template <class CType, sint32 numElements>
inline float VecxDistSquared (const CType v1, const CType v2)
{
	float returnValue = 0;
	for (sint32 i = 0; i < numElements; i++)
		returnValue += (v1[i] - v2[i]) * (v1[i] - v2[i]);
	return returnValue;
}

template <class CType, sint32 numElements>
inline float VecxDist (const CType v1, const CType v2)
{
	return sqrtf(VecxDistSquared<CType, numElements>(v1, v2));
}

template <class CType, sint32 numElements>
inline float VecxDistFast (const CType v1, const CType v2)
{
	return Q_FastSqrt(VecxDistSquared<CType, numElements>(v1, v2));
}

template <class CType, sint32 numElements>
inline float DotxProduct (const CType x, const CType y)
{
	float returnValue = 0;
	for (sint32 i = 0; i < numElements; i++)
		returnValue += (x[i] * y[i]);
	return returnValue;
}

// vec2_t manipulation
inline float Dot2Product (const vec2_t x, const vec2_t y)
{
	return DotxProduct<vec2_t, 2>(x, y);
}

inline void Vec2Add (const vec2_t a, const vec2_t b, vec2_t out)
{
	VecxAdd <vec2_t, 2> (a, b, out);
}

inline void Vec2Clear (vec2_t in)
{
	VecxClear <vec2_t, 2> (in);
}

inline bool Vec2Compare (const vec2_t v1, const vec2_t v2)
{
	return VecxCompare<vec2_t, 2> (v1, v2);
}

inline void Vec2Copy (const vec2_t a, vec2_t b)
{
	VecxCopy <vec2_t, 2> (a, b);
}

inline float Vec2DistSquared (const vec2_t v1, const vec2_t v2)
{
	return VecxDistSquared<vec2_t, 2> (v1, v2);
}

inline float Vec2Dist (const vec2_t v1, const vec2_t v2)
{
	return VecxDist<vec2_t, 2>(v1, v2);
}

inline float Vec2DistFast (const vec2_t v1, const vec2_t v2)
{
	return VecxDistFast<vec2_t, 2>(v1, v2);
}

inline void Vec2Identity (vec2_t in)
{
	VecxIdentity<vec2_t, 2> (in);
}

inline void Vec2Inverse (vec2_t v)
{
	VecxClear <vec2_t, 2> (v);
}

inline void Vec2MA (const vec2_t v, const float s, const vec2_t b, vec2_t o)
{
	VecxMA <vec2_t, 2> (v, s, b, o);
}

inline void Vec2Negate (const vec2_t a, vec2_t b)
{
	VecxNegate <vec2_t, 2> (a, b);
}

inline void Vec2Scale (const vec2_t in, const float s, vec2_t out)
{
	VecxScale <vec2_t, 2> (in, s, out);
}

inline void Vec2Set (vec2_t v, const float x, const float y)
{
	v[0] = x;
	v[1] = y;
}

inline void Vec2Subtract (const vec2_t a, const vec2_t b, vec2_t c)
{
	VecxSubtract <vec2_t, 2> (a, b, c);
}

// vec3_t routines
inline void CrossProduct (const vec3_t v1, const vec3_t v2, vec3_t cr)
{
	cr[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cr[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cr[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline float Dot3Product (const vec3_t x, const vec3_t y)
{
	return DotxProduct<vec3_t, 3>(x, y);
}

inline void Vec3Add (const vec3_t a, const vec3_t b, vec3_t out)
{
	VecxAdd<vec3_t, 3>(a, b, out);
}

inline void Vec3Average (const vec3_t a, const vec3_t b, vec3_t c)
{
	VecxAverage<vec3_t, 3>(a, b, c);
}

inline void Vec3Clear (vec3_t a)
{
	VecxClear<vec3_t, 3>(a);
}

inline bool Vec3Compare (const vec3_t v1, const vec3_t v2)
{
	return VecxCompare<vec3_t, 3>(v1, v2);
}

inline void Vec3Copy (const vec3_t a, vec3_t b)
{
	VecxCopy<vec3_t, 3>(a, b);
}

inline float Vec3DistSquared (const vec3_t v1, const vec3_t v2)
{
	return VecxDistSquared<vec3_t, 3>(v1, v2);
}

inline float Vec3Dist (const vec3_t v1, const vec3_t v2)
{
	return VecxDist<vec3_t, 3>(v1, v2);
}

inline float Vec3DistFast (const vec3_t v1, const vec3_t v2)
{
	return VecxDistFast<vec3_t, 3>(v1, v2);
}

inline void Vec3Identity (vec3_t in)
{
	VecxIdentity<vec3_t, 3> (in);
}

inline void Vec3Inverse (vec3_t v)
{
	VecxInverse<vec3_t, 3> (v);
}

inline float Vec3Length (const vec3_t v)
{
	return sqrtf(Dot3Product(v,v));
}

inline float Vec3LengthFast (const vec3_t v)
{
	return Q_FastSqrt(Dot3Product(v,v));
}

inline void Vec3MA (const vec3_t v, float s, const vec3_t b, vec3_t o)
{
	VecxMA<vec3_t, 3>(v, s, b, o);
}

inline void Vec3Negate (const vec3_t a, vec3_t b)
{
	VecxNegate<vec3_t, 3>(a, b);
}

inline void Vec3Scale (const vec3_t in, float s, vec3_t out)
{
	VecxScale<vec3_t, 3>(in, s, out);
}

inline void Vec3Set (vec3_t v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

inline void Vec3Subtract (const vec3_t a, const vec3_t b, vec3_t c)
{
	VecxSubtract<vec3_t, 3>(a, b, c);
}

// vec4_t routines
inline float Dot4Product (const vec4_t x, const vec4_t y)
{
	return DotxProduct<vec4_t, 4>(x, y);
}

inline void Vec4Add (const vec4_t a, const vec4_t b, vec4_t out)
{
	VecxAdd <vec4_t, 4> (a, b, out);
}

inline void Vec4Clear (vec4_t in)
{
	VecxClear <vec4_t, 4> (in);
}

inline bool Vec4Compare (const vec4_t v1, const vec4_t v2)
{
	return VecxCompare<vec4_t, 4> (v1, v2);
}

inline void Vec4Copy (const vec4_t a, vec4_t b)
{
	VecxCopy <vec4_t, 4> (a, b);
}

inline float Vec4DistSquared (const vec4_t v1, const vec4_t v2)
{
	return VecxDistSquared<vec4_t, 4> (v1, v2);
}

inline float Vec4Dist (const vec4_t v1, const vec4_t v2)
{
	return VecxDist<vec4_t, 4>(v1, v2);
}

inline float Vec4DistFast (const vec4_t v1, const vec4_t v2)
{
	return VecxDistFast<vec4_t, 4>(v1, v2);
}

inline void Vec4Identity (vec4_t in)
{
	VecxIdentity<vec4_t, 4> (in);
}

inline void Vec4Inverse (vec4_t v)
{
	VecxClear <vec4_t, 4> (v);
}

inline void Vec4MA (const vec4_t v, const float s, const vec4_t b, vec4_t o)
{
	VecxMA <vec4_t, 4> (v, s, b, o);
}

inline void Vec4Negate (const vec4_t a, vec4_t b)
{
	VecxNegate <vec4_t, 4> (a, b);
}

inline void Vec4Scale (const vec4_t in, const float s, vec4_t out)
{
	VecxScale <vec4_t, 4> (in, s, out);
}

inline void Vec4Set (vec4_t v, const float x, const float y)
{
	v[0] = x;
	v[1] = y;
}

inline void Vec4Subtract (const vec4_t a, const vec4_t b, vec4_t c)
{
	VecxSubtract <vec4_t, 4> (a, b, c);
}

// ===========================================================================

inline sint32 Q_rint (float x)
{
	return ((x) < 0 ? ((sint32)((x)-0.5f)) : ((sint32)((x)+0.5f)));
}

inline void Q_SinCosf(const float X, float *Sin, float *Cos)
{
#if defined(HAVE_SINCOSF)
	sincosf(X, &Sin, &Cos);
#elif defined(id386)
	__asm fld X
	__asm fsincos
	__asm mov ecx, Cos
	__asm mov edx, Sin
	__asm fstp dword ptr [ecx]
	__asm fstp dword ptr [edx]
#else
	*Sin = sinf(X);
	*Cos = cosf(X);
#endif
}

float	Q_RSqrtf (float number);
double	Q_RSqrtd (double number);
sint32		Q_log2 (sint32 val);

template<typename TType>
inline TType Q_NearestPow(const TType &Value, const bool bRoundDown)
{
	TType Result;
	for (Result = 1; Result < Value; Result <<= 1) ;

	if (bRoundDown && Result > Value)
		Result >>= 1;

	return Result;
}

#ifndef GAME_IS_BEING_COMPILED_NOT_ENGINE_GO_AWAY
float	Q_CalcFovY (float fovX, float width, float height);

// ===========================================================================

void		NormToLatLong(const vec3_t normal, uint8 out[2]);
void		LatLongToNorm(const uint8 latlong[2], vec3_t out);
#endif
void		MakeNormalVectorsf (const vec3_t forward, vec3_t right, vec3_t up);
void		PerpendicularVector (const vec3_t src, vec3_t dst);
void		RotatePointAroundVector(vec3_t dest, const vec3_t dir, const vec3_t point, const float degrees);
float		VectorNormalizef (const vec3_t in, vec3_t out);
float		VectorNormalizeFastf (vec3_t v);

//
// m_angles.c
//
float		AngleModf (float a);
#if SHARED_ALLOW_3x3_MATRIX
void		Angles_Matrix3 (vec3_t angles, mat3x3_t axis);
#endif
void		Angles_Vectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
float		LerpAngle (float a1, float a2, float frac);
void		VecToAngles (vec3_t vec, vec3_t angles);
void		VecToAngleRolled (vec3_t value, float angleYaw, vec3_t angles);
float		VecToYaw (vec3_t vec);

//
// m_bounds.c
//
void		AddBoundsTo2DBounds (vec2_t inMins, vec2_t inMaxs, vec2_t outMins, vec2_t outMaxs);
void		AddPointTo2DBounds (vec2_t v, vec2_t mins, vec2_t maxs);
void		Clear2DBounds (vec2_t mins, vec2_t maxs);

void		AddPointToBounds (vec3f v, vec3f &mins, vec3f &maxs);
bool		BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t centre, float radius);
bool		BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
void		ClearBounds (vec3_t mins, vec3_t maxs);
void		MinMins (vec3_t a, vec3_t b, vec3_t out);
void		MaxMaxs (vec3_t a, vec3_t b, vec3_t out);
float		RadiusFromBounds (vec3_t mins, vec3_t maxs);

//
// m_mat3.c
//
#if SHARED_ALLOW_3x3_MATRIX
void		Matrix3_Angles (mat3x3_t mat, vec3_t angles);
bool		Matrix3_Compare (mat3x3_t a, mat3x3_t b);
void		Matrix3_Copy (mat3x3_t in, mat3x3_t out);
void		Matrix3_FromPoints (vec3_t v1, vec3_t v2, vec3_t v3, mat3x3_t m);
void		Matrix3_Identity (mat3x3_t mat);
#if SHARED_ALLOW_4x4_MATRIX
void		Matrix3_Matrix4 (mat3x3_t in, vec3_t origin, mat4x4_t out);
#endif
void		Matrix3_Multiply (mat3x3_t in1, mat3x3_t in2, mat3x3_t out);
#if SHARED_ALLOW_QUATERNIONS
void		Matrix3_Quat (mat3x3_t m, quat_t q);
#endif
void		Matrix3_Rotate (mat3x3_t a, float angle, float x, float y, float z);
void		Matrix3_TransformVector(const mat3x3_t m, const vec3_t v, vec3_t out);
void		Matrix3_Transpose (mat3x3_t in, mat3x3_t out);
#endif

//
// m_mat4.c
//
#if SHARED_ALLOW_4x4_MATRIX
bool		Matrix4_Compare(const mat4x4_t a, const mat4x4_t b);
void		Matrix4_Copy(const mat4x4_t a, mat4x4_t b);
void		Matrix4_Copy2D(const mat4x4_t m1, mat4x4_t m2);
void		Matrix4_Identity(mat4x4_t mat);
#if SHARED_ALLOW_3x3_MATRIX
void		Matrix4_Matrix3(const mat4x4_t in, mat3x3_t out);
#endif
void		Matrix4_Multiply(const mat4x4_t a, const mat4x4_t b, mat4x4_t product);
void		Matrix4_Multiply2D(const mat4x4_t m1, const mat4x4_t m2, mat4x4_t out);
void		Matrix4_Multiply_Vec3(const mat4x4_t m, const vec3_t v, vec3_t out);
void		Matrix4_Multiply_Vec4(const mat4x4_t m, const vec4_t v, vec4_t out);
void		Matrix4_MultiplyFast(const mat4x4_t a, const mat4x4_t b, mat4x4_t product);
void		Matrix4_MultiplyFast2(const mat4x4_t m1, const mat4x4_t m2, mat4x4_t out);
void		Matrix4_Rotate(mat4x4_t a, const float angle, const float x, const float y, const float z);
void		Matrix4_Scale(mat4x4_t m, const float x, const float y, const float z);
void		Matrix4_Scale2D(mat4x4_t m, const float x, const float y);
void		Matrix4_Stretch2D(mat4x4_t m, const float s, const float t);
void		Matrix4_Translate(mat4x4_t m, const float x, const float y, const float z);
void		Matrix4_Translate2D(mat4x4_t m, const float x, const float y);
void		Matrix4_Transpose(const mat4x4_t m, mat4x4_t ret);
#endif

//
// m_quat.c
//
#if SHARED_ALLOW_QUATERNIONS
void		Quat_ConcatTransforms (quat_t q1, vec3_t v1, quat_t q2, vec3_t v2, quat_t q, vec3_t v);
void		Quat_Copy (quat_t q1, quat_t q2);
void		Quat_Conjugate (quat_t q1, quat_t q2);
void		Quat_Identity (quat_t q);
float		Quat_Inverse (quat_t q1, quat_t q2);
float		Quat_Normalize (quat_t q);
void		Quat_Lerp (quat_t q1, quat_t q2, float t, quat_t out);
void		Quat_Matrix3 (quat_t q, mat3x3_t m);
void		Quat_Multiply (quat_t q1, quat_t q2, quat_t out);
void		Quat_TransformVector (quat_t q, vec3_t v, vec3_t out);
#endif

// ===========================================================================

#include "Vector.h"

extern vec3f vec3fOrigin;

// ===========================================================================

#define NUMVERTEXNORMALS	162
extern vec3_t	m_byteDirs[NUMVERTEXNORMALS];

uint8		DirToByte(const vec3_t dirVec);
uint8		DirToByte(const vec3f &dirVec);
void		ByteToDir(const uint8 dirByte, vec3_t dirVec);
