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

// TTypes.h

#ifndef NULL
# define NULL ((void *)0)
#endif

#define null nullptr
#define var auto

#ifndef BIT
# define BIT(x) (1<<(x))
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

// ===========================================================================

typedef unsigned char byte;
typedef unsigned short ushort;

#ifdef _WIN64
typedef unsigned __int64	intptr;
#elif defined(__LP32__)
typedef unsigned int		intptr;
#else
typedef unsigned long		intptr;
#endif

typedef byte	bvec2_t[2];
typedef byte	bvec3_t[3];
typedef byte	bvec4_t[4];

typedef double	dvec2_t[2];
typedef double	dvec3_t[3];
typedef double	dvec4_t[4];

typedef sint32	index_t;
typedef sint32	ivec2_t[2];
typedef sint32	ivec3_t[3];
typedef sint32	ivec4_t[4];

typedef sint16	svec2_t[2];
typedef sint16	svec3_t[3];
typedef sint16	svec4_t[4];

typedef float	vec2_t[2];
typedef float	vec3_t[3];
typedef float	vec4_t[4];

typedef	float	quat_t[4];
typedef float	mat3x3_t[3][3];
typedef float	mat4x4_t[16];

/*
==============================================================================

	Template type definitions

	Defines some properties that are used in some template functions,
	just to optimize things.
==============================================================================
*/
template<typename TType>
class dataType_Atomic
{
public:
	typedef TType InitType;
	enum { NeedsCtor = 0 };

	template<typename TRetType>
	static inline TRetType TypeHash(const TType &Var) { return static_cast<TRetType>(Var); }
};

template<typename TType>
class dataType_Constructed
{
public:
	typedef const TType &InitType;
	enum { NeedsCtor = 1 };
};

// Default
template<typename TType>
class dataType : public dataType_Constructed<TType> { };

template<typename TType>
class dataType<TType*> : public dataType_Atomic<TType*> { };

template<> class dataType<char> : public dataType_Atomic<char> {};
template<> class dataType<bool> : public dataType_Atomic<bool> {};
template<> class dataType<byte> : public dataType_Atomic<byte> {};
template<> class dataType<double> : public dataType_Atomic<double> {};
template<> class dataType<float> : public dataType_Atomic<float> {};
template<> class dataType<int> : public dataType_Atomic<int> {};
template<> class dataType<short> : public dataType_Atomic<short> {};
template<> class dataType<uint32> : public dataType_Atomic<uint32> {};
template<> class dataType<ushort> : public dataType_Atomic<ushort> {};
