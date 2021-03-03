#ifdef _WIN32
#include "g_local.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
HANDLE	findhandle = INVALID_HANDLE_VALUE;

const char *Sys_FindFirst (const char *path)
{
	WIN32_FIND_DATA	findinfo;
	char			*p;

	if (findhandle != INVALID_HANDLE_VALUE)
		TDM_Error ("Sys_FindFirst without close");

	strncpy (findbase, path, sizeof(findbase)-1);

	p = strrchr (findbase, '/');
	if (p)
		p[0] = '\0';

	findhandle = FindFirstFile (path, &findinfo);

	if (findhandle == INVALID_HANDLE_VALUE)
		return NULL;

	snprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.cFileName);
	return findpath;
}

const char *Sys_FindNext (void)
{
	WIN32_FIND_DATA	findinfo;

	if (findhandle == INVALID_HANDLE_VALUE)
		return NULL;

	if (!FindNextFile (findhandle, &findinfo))
		return NULL;

	snprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.cFileName);
	return findpath;
}

void Sys_FindClose (void)
{
	if (findhandle != INVALID_HANDLE_VALUE)
		FindClose (findhandle);

	findhandle = INVALID_HANDLE_VALUE;
}

void Sys_DebugBreak (void)
{
	__asm int 3;
}

#endif
