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

// Templates.h

#include "TTypes.h"

/*
==============================================================================

	Template functions
 
==============================================================================
*/
template<typename TType>
static inline TType Align(const TType &Number, const TType &Alignment)
{
	return (TType)(((intptr)Number + Alignment-1) & ~(Alignment-1));
}

template<typename TType>
static inline TType Min(const TType &A, const TType &B)
{
	return (A<=B) ? A : B;
}
template<>
static inline float Min(const float &A, const float &B)
{
	return (A<=B) ? A : B;
}
template<>
static inline sint32 Min(const sint32 &A, const sint32 &B)
{
	return (A<=B) ? A : B;
}

template<typename TType>
static inline TType Max(const TType &A, const TType &B)
{
	return (A>=B) ? A : B;
}
template<>
static inline float Max(const float &A, const float &B)
{
	return (A>=B) ? A : B;
}
template<>
static inline sint32 Max(const sint32 &A, const sint32 &B)
{
	return (A>=B) ? A : B;
}

template<typename TType>
static inline TType Clamp(const TType &V, const TType &L, const TType &H)
{
	return (V<L) ? L : (V>H) ? H : V;
}
template<>
static inline float Clamp(const float &V, const float &L, const float &H)
{
	return (V<L) ? L : (V>H) ? H : V;
}
template<>
static inline sint32 Clamp(const sint32 &V, const sint32 &L, const sint32 &H)
{
	return (V<L) ? L : (V>H) ? H : V;
}

template<typename TType>
static inline TType Bound(const TType &V, const TType &L, const TType &H)
{
	return (V>=H) ? V : (L<V) ? V : (L>H) ? H : L;
}
template<>
static inline float Bound(const float &V, const float &L, const float &H)
{
	return (V>=H) ? V : (L<V) ? V : (L>H) ? H : L;
}
template<>
static inline sint32 Bound(const sint32 &V, const sint32 &L, const sint32 &H)
{
	return (V>=H) ? V : (L<V) ? V : (L>H) ? H : L;
}

template<typename TType>
static inline bool IsPowOfTwo(const TType &Value)
{
	return (bool)(Value > 0 && (Value & (Value-1)) == 0);
}
template<>
static inline bool IsPowOfTwo(const sint32 &Value)
{
	return (bool)(Value > 0 && (Value & (Value-1)) == 0);
}
template<>
static inline bool IsPowOfTwo(const uint32 &Value)
{
	return (bool)(Value > 0 && (Value & (Value-1)) == 0);
}

#ifndef GAME_IS_BEING_COMPILED_NOT_ENGINE_GO_AWAY
//#include "TArray.h"
//#include "TAutoPtr.h"
//#include "TMap.h"
//#include "TString.h"
#endif

