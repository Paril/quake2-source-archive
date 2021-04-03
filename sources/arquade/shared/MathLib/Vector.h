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

// Vector.h

/*
==============================================================================

	vec2Base
 
==============================================================================
*/

#ifdef GAME_IS_BEING_COMPILED_NOT_ENGINE_GO_AWAY
template<typename TType>
class  vec2Base
{
public:
	/**
	 * Data
	 */
	TType X, Y;

	/**
	 * Constructors
	 */
	vec2Base()
	: X(0), Y(0) {}

	inline vec2Base(const vec2Base &Vec)
	: X(Vec[0]), Y(Vec[1]) {}

	inline vec2Base(const TType Value)
	: X(Value), Y(Value) {}

	inline vec2Base(const TType Values[2])
	: X(Values[0]), Y(Values[1]) {}

	inline vec2Base(const TType InX, const TType InY)
	: X(InX), Y(InY) {}

	/**
	 * Destructors
	 */
	~vec2Base() {}

	/**
	 * Operators
	 */
	inline bool operator !=(const vec2Base &Vec) { return (X != Vec[0] || Y != Vec[1]); }
	inline const bool operator !=(const vec2Base &Vec) const { return (X != Vec[0] || Y != Vec[1]); }

	inline bool operator ==(const vec2Base &Vec) { return (X == Vec[0] && Y == Vec[1]); }
	inline const bool operator ==(const vec2Base &Vec) const { return (X == Vec[0] && Y == Vec[1]); }

	inline const TType &operator [](const sint32 Index) const { return (&X)[Index]; }
	inline TType &operator [](const sint32 Index) { return (&X)[Index]; }

	inline operator TType *() { return (&X); }
	inline operator const TType *() const { return (&X); }

	/**
	 * Functions
	 */
	inline const bool Compare(const vec2Base &Vec) const { return (X == Vec[0] && Y == Vec[1]); }

	inline void Copy(const vec2Base &Vec) { X = Vec[0]; Y = Vec[1]; }

	inline void Invert() { X = -X; Y = -Y; }
	inline vec2Base GetInverted() const { return vec2Base(-X, -Y); }

	inline const bool IsZero() const { return (X == 0 && Y == 0); }

	inline void Set(const TType Number) { X = Number; Y = Number; }
	inline void Set(const TType InX, const TType InY) { X = InX; Y = InY; }
	inline void Set(const vec2Base &Vec) { X = Vec[0]; Y = Vec[1]; }
};

/*
==============================================================================

	vec2f
 
==============================================================================
*/

class  vec2f : public vec2Base<float>
{
public:
	/**
	 * Constructors
	 */
	vec2f() {}
	inline vec2f(const vec2f &Vec) : vec2Base(Vec) {}
	inline vec2f(const float Value) : vec2Base(Value) {}
	inline vec2f(const float Values[2]) : vec2Base(Values) {}
	inline vec2f(const float InX, const float InY) : vec2Base(InX, InY) {}

	/**
	 * Operators
	 */
	inline vec2f operator *(const vec2f &Vec) const { return vec2f(X*Vec[0], Y*Vec[1]); }
	inline vec2f operator *(const float Scale) const { return vec2f(X*Scale, Y*Scale); }

	inline vec2f &operator *=(const vec2f &Vec)
	{
		X *= Vec[0];
		Y *= Vec[1];

		return *this;
	}
	inline vec2f &operator *=(const float Scale)
	{
		X *= Scale;
		Y *= Scale;

		return *this;
	}

	inline vec2f operator +(const vec2f &Vec) const { return vec2f(X+Vec[0], Y+Vec[1]); }
	inline vec2f &operator +=(const vec2f &Vec)
	{
		X += Vec[0];
		Y += Vec[1];

		return *this;
	}

	inline vec2f operator -(const vec2f &Vec) const { return vec2f(X-Vec[0], Y-Vec[1]); }
	inline vec2f &operator -=(const vec2f &Vec)
	{
		X -= Vec[0];
		Y -= Vec[1];

		return *this;
	}

	inline vec2f operator /(const vec2f &Vec) const { return vec2f(X/Vec[0], Y/Vec[1]); }
	inline vec2f operator /(const float Number) const
	{
		const float InvNumber = 1.0f / Number;
		return vec2f(X*InvNumber, Y*InvNumber);
	}

	inline vec2f &operator /=(const vec2f &Vec)
	{
		X /= Vec[0];
		Y /= Vec[1];

		return *this;
	}
	inline vec2f &operator /=(const float Number)
	{
		const float InvNumber = 1.0f / Number;

		X *= InvNumber;
		Y *= InvNumber;

		return *this;
	}

	inline bool operator <(const vec2f &Vec) { return (X < Vec[0] && Y < Vec[1]); }
	inline const bool operator <(const vec2f &Vec) const { return (X < Vec[0] && Y < Vec[1]); }

	inline vec2f &operator =(const vec2f &Vec)
	{
		X = Vec[0];
		Y = Vec[1];

		return *this;
	}

	inline bool operator >(const vec2f &Vec) { return (X > Vec[0] && Y > Vec[1]); }
	inline const bool operator >(const vec2f &Vec) const { return (X > Vec[0] && Y > Vec[1]); }

	/**
	 * Functions
	 */
	inline void Clear() { *(sint32*)&X = 0; *(sint32*)&Y = 0; }

	const bool Compare(const vec2f &Vec, const float Epsilon) const
	{
		if (Q_fabs(X-Vec[0]) > Epsilon)
			return false;
		if (Q_fabs(Y-Vec[1]) > Epsilon)
			return false;
		return true;
	}

	const bool IsNearlyZero(const float Epsilon = SMALL_NUMBER) { return (Q_fabs(X) <= Epsilon && Q_fabs(Y) <= Epsilon); }

	inline void Scale(const float Scale) { X *= Scale; Y *= Scale; }
	inline void Scale(const vec2f &Vec) { X *= Vec[0]; Y *= Vec[1]; }
};
#endif

/*
==============================================================================

	vec3Base
 
==============================================================================
*/

template<typename TType>
class  vec3Base
{
public:
	/**
	 * Data
	 */
	TType X, Y, Z;

	/**
	 * Constructors
	 */
	vec3Base()
	: X(0), Y(0), Z(0) {}

	inline vec3Base(const vec3Base &Vec)
	: X(Vec[0]), Y(Vec[1]), Z(Vec[2]){}

	inline vec3Base(const TType Value)
	: X(Value), Y(Value), Z(Value) {}

	inline vec3Base(const TType Values[3])
	: X(Values[0]), Y(Values[1]), Z(Values[2]) {}

	inline vec3Base(const TType InX, const TType InY, const TType InZ)
	: X(InX), Y(InY), Z(InZ) {}

	/**
	 * Destructors
	 */
	~vec3Base() {}

	/**
	 * Operators
	 */
	inline bool operator !=(const vec3Base &Vec) { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2]); }
	inline const bool operator !=(const vec3Base &Vec) const { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2]); }

	inline bool operator ==(const vec3Base &Vec) { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2]); }
	inline const bool operator ==(const vec3Base &Vec) const { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2]); }

	inline const TType &operator [](const sint32 Index) const { return (&X)[Index]; }
	inline TType &operator [](const sint32 Index) { return (&X)[Index]; }

	inline operator TType *() { return (&X); }
	inline operator const TType *() const { return (&X); }

	/**
	 * Functions
	 */
	inline const bool Compare(const vec3Base &Vec) const { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2]); }

	inline void Copy(const vec3Base &Vec) { X = Vec[0]; Y = Vec[1]; Z = Vec[2]; }

	inline void Invert() { X = -X; Y = -Y; Z = -Z; }
	inline vec3Base GetInverted() const { return vec3Base(-X, -Y, -Z); }

	inline void Abs() { X = Q_fabs(X); Y = Q_fabs(Y); Z = Q_fabs(Z); }
	inline vec3Base GetAbs() const { return vec3Base(Q_fabs(X), Q_fabs(Y), Q_fabs(Z)); }

	inline const bool IsZero() const { return (X == 0 && Y == 0 && Z == 0); }

	inline void Set(const TType Number) { X = Number; Y = Number; Z = Number; }
	inline void Set(const TType InX, const TType InY, const TType InZ) { X = InX; Y = InY; Z = InZ; }
	inline void Set(const vec3Base &Vec) { X = Vec[0]; Y = Vec[1]; Z = Vec[2]; }
};

/*
==============================================================================

	vec3f
 
==============================================================================
*/

class  vec3f : public vec3Base<float>
{
public:
	/**
	 * Constructors
	 */
	vec3f() {}
	inline vec3f(const vec3f &Vec) : vec3Base(Vec) {}
	inline vec3f(const float Value) : vec3Base(Value) {}
	inline vec3f(const float Values[3]) : vec3Base(Values) {}
	inline vec3f(const float InX, const float InY, const float InZ) : vec3Base(InX, InY, InZ) {}

	/**
	 * Operators
	 */
	inline vec3f operator *(const vec3f &Vec) const { return vec3f(X*Vec[0], Y*Vec[1], Z*Vec[2]); }
	inline vec3f operator *(const float Scale) const { return vec3f(X*Scale, Y*Scale, Z*Scale); }

	inline vec3f &operator *=(const vec3f &Vec)
	{
		X *= Vec[0];
		Y *= Vec[1];
		Z *= Vec[2];

		return *this;
	}
	inline vec3f &operator *=(const float Scale)
	{
		X *= Scale;
		Y *= Scale;
		Z *= Scale;

		return *this;
	}

	inline vec3f operator +(const vec3f &Vec) const { return vec3f(X+Vec[0], Y+Vec[1], Z+Vec[2]); }
	inline vec3f &operator +=(const vec3f &Vec)
	{
		X += Vec[0];
		Y += Vec[1];
		Z += Vec[2];

		return *this;
	}

	inline vec3f operator -(const vec3f &Vec) const { return vec3f(X-Vec[0], Y-Vec[1], Z-Vec[2]); }
	inline vec3f &operator -=(const vec3f &Vec)
	{
		X -= Vec[0];
		Y -= Vec[1];
		Z -= Vec[2];

		return *this;
	}

	inline vec3f operator /(const vec3f &Vec) const { return vec3f(X/Vec[0], Y/Vec[1], Z/Vec[2]); }
	inline vec3f operator /(const float Number) const
	{
		const float InvNumber = 1.0f / Number;
		return vec3f(X*InvNumber, Y*InvNumber, Z*InvNumber);
	}

	inline vec3f &operator /=(const vec3f &Vec)
	{
		X /= Vec[0];
		Y /= Vec[1];
		Z /= Vec[2];

		return *this;
	}
	inline vec3f &operator /=(const float Number)
	{
		const float InvNumber = 1.0f / Number;

		X *= InvNumber;
		Y *= InvNumber;
		Z *= InvNumber;

		return *this;
	}

	inline bool operator <(const vec3f &Vec) { return (X < Vec[0] && Y < Vec[1] && Z < Vec[2]); }
	inline const bool operator <(const vec3f &Vec) const { return (X < Vec[0] && Y < Vec[1] && Z < Vec[2]); }

	inline vec3f &operator =(const vec3f &Vec)
	{
		X = Vec[0];
		Y = Vec[1];
		Z = Vec[2];

		return *this;
	}

	inline bool operator >(const vec3f &Vec) { return (X > Vec[0] && Y > Vec[1] && Z > Vec[2]); }
	inline const bool operator >(const vec3f &Vec) const { return (X > Vec[0] && Y > Vec[1] && Z > Vec[2]); }

	inline vec3f operator ^(const vec3f &Vec) const { return Cross(Vec); }
	inline float operator |(const vec3f &Vec) const { return Dot(Vec); }

	// Paril, unary operators
	inline vec3f operator - () const { return vec3f(-X, -Y, -Z); }

	/**
	 * Functions
	 */
	inline void Clear() { *(sint32 *)&X = 0; *(sint32 *)&Y = 0; *(sint32 *)&Z = 0; }

	const bool Compare(const vec3f &Vec, const float Epsilon) const
	{
		if (Q_fabs(X-Vec[0]) > Epsilon)
			return false;
		if (Q_fabs(Y-Vec[1]) > Epsilon)
			return false;
		if (Q_fabs(Z-Vec[2]) > Epsilon)
			return false;
		return true;
	}

	inline vec3f Cross(const vec3f &Vec) const { return vec3f(Y*Vec[2] - Z*Vec[1], Z*Vec[0] - X*Vec[2], X*Vec[1] - Y*Vec[0]); }

	inline float Dist(const vec3f &Vec) { return sqrtf((X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2])); }
	inline const float Dist(const vec3f &Vec) const { return sqrtf((X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2])); }
	inline float DistFast(const vec3f &Vec) { return Q_FastSqrt((X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2])); }
	inline const float DistFast(const vec3f &Vec) const { return Q_FastSqrt((X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2])); }
	inline float DistSq(const vec3f &Vec) { return (X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2]); }
	inline const float DistSq(const vec3f &Vec) const { return (X-Vec[0])*(X-Vec[0])+(Y-Vec[1])*(Y-Vec[1])+(Z-Vec[2])*(Z-Vec[2]); }

	inline float Dot(const vec3f &Vec) { return X*Vec[0] + Y*Vec[1] + Z*Vec[2]; }
	inline const float Dot(const vec3f &Vec) const { return X*Vec[0] + Y*Vec[1] + Z*Vec[2]; }

	const bool IsNearlyZero(const float Epsilon = SMALL_NUMBER) { return (Q_fabs(X) <= Epsilon && Q_fabs(Y) <= Epsilon && Q_fabs(Z) <= Epsilon); }

	inline const float Length() const { return sqrtf(X*X + Y*Y + Z*Z); }
	inline const float LengthFast() const { return Q_FastSqrt(X*X + Y*Y + Z*Z); }
	inline const float LengthSq() const { return X*X + Y*Y + Z*Z; }

	float Normalize()
	{
		float Len = Length();

		if (Len)
			Scale(1.0f / Len);
		else
			Clear();

		return Len;
	}
	float NormalizeFast()
	{
		float Len = Q_RSqrtf(Dot(*this));
		Scale (Len);
		return (Len) ? (1.0f / Len) : 0;
	}

	inline vec3f GetNormalized ()
	{
		vec3f val (*this);
		val.Normalize ();
		return val;
	}

	inline vec3f GetNormalizedFast ()
	{
		vec3f val (*this);
		val.NormalizeFast ();
		return val;
	}

	inline vec3f GetNormalized (float &length)
	{
		vec3f val (*this);
		length = val.Normalize ();
		return val;
	}

	inline vec3f GetNormalizedFast (float &length)
	{
		vec3f val (*this);
		length = val.NormalizeFast ();
		return val;
	}

	float ToYaw ()
	{
		float	yaw;

		if (X == 0)
		{
			yaw = 0;
			if (Y > 0)
				yaw = 90;
			else if (Y < 0)
				yaw = -90;
		}
		else
		{
			yaw = atan2f(Y, X) * 180 / M_PI;
			if (yaw < 0)
				yaw += 360;
		}

		return yaw;
	}

	inline void Scale(const float Scale) { X *= Scale; Y *= Scale; Z *= Scale; }
	inline void Scale(const vec3f &Vec) { X *= Vec[0]; Y *= Vec[1]; Z *= Vec[2]; }

	void ToVectors (vec3f *forward, vec3f *right, vec3f *up)
	{
		float sr, sp, sy, cr, cp, cy;
		Q_SinCosf(DEG2RAD (X), &sp, &cp);
		Q_SinCosf(DEG2RAD (Y), &sy, &cy);
		Q_SinCosf(DEG2RAD (Z), &sr, &cr);

		if (forward)
			forward->Set (cp* cy, cp*sy, -sp);
		if (right)
			right->Set (-1 * sr * sp * cy + -1 * cr * -sy,
						-1 * sr * sp * sy + -1 * cr * cy,
						-1 * sr * cp);
		if (up)
			up->Set (cr * sp * cy + -sr * -sy,
						cr * sp * sy + -sr * cy,
						cr * cp);
	};

	inline vec3f MultiplyAngles (const float scale, const vec3f &b)
	{
		return vec3f(	X + b.X * scale,
						Y + b.Y * scale,
						Z + b.Z * scale);
	}

	vec3f ToAngles ()
	{
		float	yaw, pitch;
		
		if (Y == 0 && X == 0)
		{
			yaw = 0;
			if (Z > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			if (X)
				yaw = atan2f(Y, X) * (180.0f / M_PI);
			else if (Y > 0)
				yaw = 90;
			else
				yaw = 270;

			if (yaw < 0)
				yaw += 360;

			float forward = sqrtf(X * X + Y * Y);
			pitch = atan2f(Z, forward) * (180.0f / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		return vec3f (-pitch, yaw, 0);
	}
};

// Global vector operations
inline vec3f operator * (const float &l, const vec3f &r)
{
	return (r * l);
};

/*
==============================================================================

	vec4Base
 
==============================================================================
*/

#ifndef GAME_IS_BEING_COMPILED_NOT_ENGINE_GO_AWAY
template<typename TType>
class vec4Base
{
public:
	/**
	 * Data
	 */
	TType X, Y, Z, W;

	/**
	 * Constructors
	 */
	vec4Base() {}

	inline vec4Base(const vec4Base &Vec)
	: X(Vec[0]), Y(Vec[1]), Z(Vec[2]), W(Vec[3]) {}

	inline vec4Base(const TType Value)
	: X(Value), Y(Value), Z(Value), W(Value) {}

	inline vec4Base(const TType Values[4])
	: X(Values[0]), Y(Values[1]), Z(Values[2]), W(Values[3]) {}

	inline vec4Base(const TType InX, const TType InY, const TType InZ, const TType InW)
	: X(InX), Y(InY), Z(InZ), W(InW) {}

	/**
	 * Destructors
	 */
	~vec4Base() {}

	/**
	 * Operators
	 */
	inline bool operator !=(const vec4Base &Vec) { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2] || W != Vec[3]); }
	inline const bool operator !=(const vec4Base &Vec) const { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2] || W != Vec[3]); }

	inline bool operator ==(const vec4Base &Vec) { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2] && W == Vec[3]); }
	inline const bool operator ==(const vec4Base &Vec) const { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2] && W == Vec[3]); }

	inline const TType &operator [](const sint32 Index) const { return (&X)[Index]; }
	inline TType &operator [](const sint32 Index) { return (&X)[Index]; }

	inline operator TType *() { return (&X); }
	inline operator const TType *() const { return (&X); }

	/**
	 * Functions
	 */
	inline const bool Compare(const vec4Base &Vec) const { return (X == Vec[0] && Y == Vec[1] && Z == Vec[2] && W == Vec[3]); }

	inline void Copy(const vec4Base &Vec) { X = Vec[0]; Y = Vec[1]; Z = Vec[2]; W = Vec[3]; }

	inline void Invert() { X = -X; Y = -Y; Z = -Z; W = -W; }
	inline vec4Base GetInverted() const { return vec4Base(-X, -Y, -Z, -W); }

	inline const bool IsZero() const { return (X == 0 && Y == 0 && Z == 0 && W == 0); }

	inline void Set(const TType Number) { X = Number; Y = Number; Z = Number; W = Number; }
	inline void Set(const TType InX, const TType InY, const TType InZ, const TType InW) { X = InX; Y = InY; Z = InZ; W = InW; }
	inline void Set(const vec4Base &Vec) { X = Vec[0]; Y = Vec[1]; Z = Vec[2]; W = Vec[3]; }
};

/*
==============================================================================

	vec4b
 
==============================================================================
*/

class vec4b : public vec4Base<uint8>
{
public:
	/**
	 * Constructors
	 */
	vec4b() {}
	inline vec4b(const vec4b &Vec) : vec4Base(Vec) {}
	inline vec4b(const uint8 Value) : vec4Base(Value) {}
	inline vec4b(const uint8 Values[3]) : vec4Base(Values) {}
	inline vec4b(const uint8 InX, const uint8 InY, const uint8 InZ, const uint8 InW) : vec4Base(InX, InY, InZ, InW) {}

 	/**
	 * Operators
	 */
	inline vec4b operator +(const vec4b &Vec) const { return vec4b(X+Vec[0], Y+Vec[1], Z+Vec[2], W+Vec[3]); }
	inline vec4b operator +=(const vec4b &Vec)
	{
		X = Min<uint8>(X+Vec[0], 255);
		Y = Min<uint8>(Y+Vec[1], 255);
		Z = Min<uint8>(Z+Vec[2], 255);
		W = Min<uint8>(W+Vec[3], 255);

		return *this;
	}

	inline vec4b operator -(const vec4b &Vec) const { return vec4b(X-Vec[0], Y-Vec[1], Z-Vec[2], W-Vec[3]); }
	inline vec4b operator -=(const vec4b &Vec)
	{
		X = Max<uint8>(X-Vec[0], 0);
		Y = Max<uint8>(Y-Vec[1], 0);
		Z = Max<uint8>(Z-Vec[2], 0);
		W = Max<uint8>(W-Vec[3], 0);

		return *this;
	}

	inline vec4b &operator =(const vec4b &Vec)
	{
		X = Vec[0];
		Y = Vec[1];
		Z = Vec[2];
		W = Vec[3];

		return *this;
	}

	inline operator sint32 *() { return (sint32 *)(&X); }
	inline operator uint32 *() { return (uint32 *)(&X); }
	inline operator void *() { return (void *)(&X); }

	/**
	 * Functions
	 */
	inline void Clear() { X = 0; Y = 0; Z = 0; W = 0; }
};

/*
==============================================================================

	vec4f
 
==============================================================================
*/

class vec4f : public vec4Base<float>
{
public:
	/**
	 * Constructors
	 */
	vec4f() {}
	inline vec4f(const vec4f &Vec) : vec4Base(Vec) {}
	inline vec4f(const float Value) : vec4Base(Value) {}
	inline vec4f(const float Values[3]) : vec4Base(Values) {}
	inline vec4f(const float InX, const float InY, const float InZ, const float InW) : vec4Base(InX, InY, InZ, InW) {}

 	/**
	 * Operators
	 */
	inline bool operator !=(const vec4f &Vec) { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2] || W != Vec[3]); }
	inline const bool operator !=(const vec4f &Vec) const { return (X != Vec[0] || Y != Vec[1] || Z != Vec[2] || W != Vec[3]); }

	inline vec4f operator *(const vec4f &Vec) const { return vec4f(X*Vec[0], Y*Vec[1], Z*Vec[2], W*Vec[3]); }
	inline vec4f operator *(const float Scale) const { return vec4f(X*Scale, Y*Scale, Z*Scale, W*Scale); }

	inline vec4f &operator *=(const vec4f &Vec)
	{
		X *= Vec[0];
		Y *= Vec[1];
		Z *= Vec[2];
		W *= Vec[3];

		return *this;
	}
	inline vec4f &operator *=(const float Scale)
	{
		X *= Scale;
		Y *= Scale;
		Z *= Scale;
		W *= Scale;

		return *this;
	}

	inline vec4f operator +(const vec4f &Vec) const { return vec4f(X+Vec[0], Y+Vec[1], Z+Vec[2], W+Vec[3]); }
	inline vec4f &operator +=(const vec4f &Vec)
	{
		X += Vec[0];
		Y += Vec[1];
		Z += Vec[2];
		W += Vec[3];

		return *this;
	}

	inline vec4f operator -(const vec4f &Vec) const { return vec4f(X-Vec[0], Y-Vec[1], Z-Vec[2], W-Vec[3]); }
	inline vec4f &operator -=(const vec4f &Vec)
	{
		X -= Vec[0];
		Y -= Vec[1];
		Z -= Vec[2];
		W -= Vec[3];

		return *this;
	}

	inline vec4f operator /(const vec4f &Vec) const { return vec4f(X/Vec[0], Y/Vec[1], Z/Vec[2], W/Vec[3]); }
	inline vec4f operator /(const float Number) const
	{
		const float InvNumber = 1.0f / Number;
		return vec4f(X*InvNumber, Y*InvNumber, Z*InvNumber, W*InvNumber);
	}

	inline vec4f &operator /=(const vec4f &Vec)
	{
		X /= Vec[0];
		Y /= Vec[1];
		Z /= Vec[2];
		W /= Vec[3];

		return *this;
	}
	inline vec4f &operator /=(const float Number)
	{
		const float InvNumber = 1.0f / Number;

		X *= InvNumber;
		Y *= InvNumber;
		Z *= InvNumber;
		W *= InvNumber;

		return *this;
	}

	inline bool operator <(const vec4f &Vec) { return (X < Vec[0] && Y < Vec[1] && Z < Vec[2] && W < Vec[3]); }
	inline const bool operator <(const vec4f &Vec) const { return (X < Vec[0] && Y < Vec[1] && Z < Vec[2] && W < Vec[3]); }

	inline vec4f &operator =(const vec4f &Vec)
	{
		X = Vec[0];
		Y = Vec[1];
		Z = Vec[2];
		W = Vec[3];

		return *this;
	}

	inline bool operator >(const vec4f &Vec) { return (X > Vec[0] && Y > Vec[1] && Z > Vec[2] && W > Vec[3]); }
	inline const bool operator >(const vec4f &Vec) const { return (X > Vec[0] && Y > Vec[1] && Z > Vec[2] && W > Vec[3]); }

	/**
	 * Functions
	 */
	inline void Clear() { *(sint32 *)&X = 0; *(sint32 *)&Y = 0; *(sint32 *)&Z = 0; *(sint32 *)&W = 0; }

	const bool Compare(const vec4f &Vec, const float Epsilon) const
	{
		if (Q_fabs(X-Vec[0]) > Epsilon)
			return false;
		if (Q_fabs(Y-Vec[1]) > Epsilon)
			return false;
		if (Q_fabs(Z-Vec[2]) > Epsilon)
			return false;
		if (Q_fabs(W-Vec[3]) > Epsilon)
			return false;
		return true;
	}

	const bool IsNearlyZero(const float Epsilon = SMALL_NUMBER) { return (Q_fabs(X) <= Epsilon && Q_fabs(Y) <= Epsilon && Q_fabs(Z) <= Epsilon && Q_fabs(W) <= Epsilon); }

	inline void Scale(const float Scale) { X *= Scale; Y *= Scale; Z *= Scale; W *= Scale; }
	inline void Scale(const vec4f &Vec) { X *= Vec[0]; Y *= Vec[1]; Z *= Vec[2]; W *= Vec[3]; }
};
#endif