#pragma once

template<typename T>
constexpr T bit(const T &place) { return static_cast<T>(1u << static_cast<typename std::make_unsigned<T>::type>(place)); }

#include <type_traits>

#define BITFLAG_TYPE(T) std::underlying_type_t<T>
#define MAKE_BITFLAGS(T) \
constexpr T operator~ (const T &a) { return static_cast<T>(~static_cast<BITFLAG_TYPE(T)>(a)); } \
constexpr T operator| (const T &a, const T &b) { return static_cast<T>(static_cast<BITFLAG_TYPE(T)>(a) | (static_cast<BITFLAG_TYPE(T)>(b))); } \
constexpr T operator& (const T &a, const T &b) { return static_cast<T>(static_cast<BITFLAG_TYPE(T)>(a) & (static_cast<BITFLAG_TYPE(T)>(b))); } \
constexpr T operator^ (const T &a, const T &b) { return static_cast<T>(static_cast<BITFLAG_TYPE(T)>(a) ^ (static_cast<BITFLAG_TYPE(T)>(b))); } \
inline T &operator|= (T &a, const T &b) { return a = (a | b); } \
inline T &operator&= (T &a, const T &b) { return a = (a & b); } \
inline T &operator^= (T &a, const T &b) { return a = (a ^ b); }