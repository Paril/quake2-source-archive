#ifndef __STRL_H
#define __STRL_H

#define _STRL_TRUNCATE (-1)

#ifdef NEED_STRLCAT
size_t strlcat_s(char *dest, char *src, size_t n, int count);
#define strlcat(d,s,n) strlcat_s(d,s,n,_STRL_TRUNCATE)
#else
#define strlcat_s(d,s,n,c) strlcat(d,s,n)
#endif

#ifdef NEED_STRLCPY
size_t strlcpy_s(char *dest, char *src, size_t n, int count);
#define strlcpy(d,s,n) strlcpy_s(d,s,n,_STRL_TRUNCATE)
#else
#define strlcpy_s(d,s,n,c) strlcpy(d,s,n)
#endif

#endif


#ifdef WIN32
#define snprintf(b,s,f,...) _snprintf_s(b,s,_TRUNCATE,f,__VA_ARGS__)
#define vsnprintf(b,s,f,...) vsnprintf_s(b,s,_TRUNCATE,f,__VA_ARGS__)
#define strdup(s) _strdup(s)
#define stricmp(s1,s2) _stricmp(s1,s2)
#endif
