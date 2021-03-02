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

// Platform.h

// =========================================================================
// Windows
//
#ifdef WIN32

// unknown pragmas are SUPPOSED to be ignored, but....
# pragma warning(disable : 4244)	// 'conversion' conversion from 'type1' to 'type2', possible loss of data

// Off by default, force on for level 3 warning level
# pragma warning(3 : 4056)	// overflow in floating point constant arithmetic
# pragma warning(disable : 4191)	// 'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
# pragma warning(3 : 4254)	// 'operator' : conversion from 'type1' to 'type2', possible loss of data
# pragma warning(disable : 4996)	// unsafe
# pragma warning(disable : 4305)
# pragma warning(disable : 6011)
# pragma warning(disable : 6001)
# pragma warning(disable : 6031)
# pragma warning(disable : 6385)
# pragma warning(disable : 6386)

# pragma intrinsic(memcmp)

# define HAVE___INLINE 1
# define HAVE___FASTCALL 1
# define HAVE__SNPRINTF 1
# define HAVE__STRICMP 1
# define HAVE__VSNPRINTF 1
# define HAVE__CDECL 1

# define BUILDSTRING		"Win32"

# ifndef _DEBUG
#  ifdef _M_IX86
#   define CPUSTRING		"x86"
#  elif defined _M_ALPHA
#   define CPUSTRING		"AXP"
#  endif
# else // _DEBUG
#  ifdef _M_IX86
#   define CPUSTRING		"x86 Debug"
#  elif defined _M_ALPHA
#   define CPUSTRING		"AXP Debug"
#  endif
# endif // _DEBUG

typedef __int16				sint16;
typedef __int32				sint32;
typedef __int64				sint64;
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

# define strdup _strdup

# define __declspec_naked __declspec(naked)

// =========================================================================
// Generic Unix
//
#elif defined __unix__ || defined __wasm__
#  define GL_FORCEFINISH

#  define HAVE_INLINE 1
#  define HAVE_STRCASECMP 1

# ifdef _GNU_SOURCE
#  define HAVE_SINCOSF 1
# endif

#  define __declspec
#  define __declspec_naked

//
// Linux
//
# if defined __linux__
#  include <stdint.h>

#  define BUILDSTRING		"Linux"

#  ifndef _DEBUG
#   ifdef __i386__
#    define CPUSTRING		"i386"
#   elif defined(__alpha__)
#    define CPUSTRING		"AXP"
#   endif
#  else // _DEBUG
#   ifdef __i386__
#    define CPUSTRING		"i386 Debug"
#   elif defined(__alpha__)
#    define CPUSTRING		"AXP Debug"
#   endif
#  endif // _DEBUG

//
// FreeBSD
//
# elif defined __FreeBSD__
#  include <machine/param.h>
#  if __FreeBSD_version < 500000
#   include <inttypes.h>
#  else
#   include <stdint.h>
#  endif

#  define BUILDSTRING		"FreeBSD"

#  ifndef _DEBUG
#   ifdef __i386__
#    define CPUSTRING		"i386"
#   elif defined(__alpha__)
#    define CPUSTRING		"AXP"
#   endif
#  else // _DEBUG
#   ifdef __i386__
#    define CPUSTRING		"i386 Debug"
#   elif defined(__alpha__)
#    define CPUSTRING		"AXP Debug"
#   endif
#  endif // _DEBUG

# endif

typedef int16_t				sint16;
typedef int32_t				sint32;
typedef int64_t				sint64;
typedef uint16_t			uint16;
typedef uint32_t			uint32;
typedef uint64_t			uint64;

#endif	// __unix__

// =========================================================================

#ifndef HAVE__CDECL
# define __cdecl
#endif

#ifndef HAVE___FASTCALL
# define __fastcall
#endif

#ifdef HAVE___INLINE
# ifndef inline
#  define inline __inline
# endif
#elif !defined(HAVE_INLINE)
# ifndef inline
#  define inline
# endif
#endif

#ifdef HAVE__SNPRINTF
# ifndef snprintf 
#  define snprintf _snprintf
# endif
#endif

#ifdef HAVE_STRCASECMP
# ifndef Q_stricmp
#  define Q_stricmp(s1,s2) strcasecmp ((s1), (s2))
# endif
# ifndef Q_strnicmp
#  define Q_strnicmp(s1,s2,n) strncasecmp ((s1), (s2), (n))
# endif
#endif

#ifdef HAVE__STRICMP
# ifndef Q_stricmp
#  define Q_stricmp(s1, s2) _stricmp((s1), (s2))
# endif
# ifndef Q_strnicmp
#  define Q_strnicmp(s1, s2, n) _strnicmp((s1), (s2), (n))
# endif
#endif

#ifdef HAVE__VSNPRINTF
# ifndef vsnprintf
#  define vsnprintf _vsnprintf
# endif
#endif

// =========================================================================

#if (defined(_M_IX86) || defined(__i386__)) && !defined(C_ONLY) && !defined(__unix__) // FIXME: make this work with unix
# define id386
#else
# ifdef id386
#  undef id386
# endif
#endif

#ifndef BUILDSTRING
# define BUILDSTRING	"Unknown"
#endif

#ifndef CPUSTRING
# define CPUSTRING		"Unknown"
#endif
