#pragma once

int32_t irandom();

int32_t irandom(const int32_t &min, const int32_t &max);
inline int32_t irandom(const int32_t &max) { return irandom(0, max); }

vec_t frandom();
vec_t frandom(const vec_t &min, const vec_t &max);

inline vec_t frandom(const vec_t &max) { return frandom(0, max); }

inline vec_t crandom() { return frandom(-1.f, 1.f); }
inline vec_t crandom(const vec_t &max) { return frandom(-max, max); }

inline bool prandom(const int32_t &pct) { return irandom(100) < pct; }
inline bool prandom(const vec_t &pct) { return frandom(100.f) < pct; }
