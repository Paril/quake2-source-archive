#ifndef _WIN32
#include "g_local.h"
#include <dirent.h>

static	char	findbase[MAX_OSPATH];
static	char	findpath[MAX_OSPATH];
static	char	findpattern[MAX_OSPATH];
static	DIR		*fdir;

int glob_match(char *pattern, char *text);

/* Like glob_match, but match PATTERN against any final segment of TEXT.  */
static int glob_match_after_star(char *pattern, char *text)
{
	register char *p = pattern, *t = text;
	register char c, c1;

	while ((c = *p++) == '?' || c == '*')
		if (c == '?' && *t++ == '\0')
			return 0;

	if (c == '\0')
		return 1;

	if (c == '\\')
		c1 = *p;
	else
		c1 = c;

	while (1) {
		if ((c == '[' || *t == c1) && glob_match(p - 1, t))
			return 1;
		if (*t++ == '\0')
			return 0;
	}
}

/* Match the pattern PATTERN against the string TEXT;
   return 1 if it matches, 0 otherwise.

   A match means the entire string TEXT is used up in matching.

   In the pattern string, `*' matches any sequence of characters,
   `?' matches any character, [SET] matches any character in the specified set,
   [!SET] matches any character not in the specified set.

   A set is composed of characters or ranges; a range looks like
   character hyphen character (as in 0-9 or A-Z).
   [0-9a-zA-Z_] is the set of characters allowed in C identifiers.
   Any other character in the pattern must be matched exactly.

   To suppress the special syntactic significance of any of `[]*?!-\',
   and match the character exactly, precede it with a `\'.
*/

int glob_match(char *pattern, char *text)
{
	register char *p = pattern, *t = text;
	register char c;

	while ((c = *p++) != '\0')
		switch (c) {
		case '?':
			if (*t == '\0')
				return 0;
			else
				++t;
			break;

		case '\\':
			if (*p++ != *t++)
				return 0;
			break;

		case '*':
			return glob_match_after_star(p, t);

		case '[':
			{
				register char c1 = *t++;
				int invert;

				if (!c1)
					return (0);

				invert = ((*p == '!') || (*p == '^'));
				if (invert)
					p++;

				c = *p++;
				while (1) {
					register char cstart = c, cend = c;

					if (c == '\\') {
						cstart = *p++;
						cend = cstart;
					}
					if (c == '\0')
						return 0;

					c = *p++;
					if (c == '-' && *p != ']') {
						cend = *p++;
						if (cend == '\\')
							cend = *p++;
						if (cend == '\0')
							return 0;
						c = *p++;
					}
					if (c1 >= cstart && c1 <= cend)
						goto match;
					if (c == ']')
						break;
				}
				if (!invert)
					return 0;
				break;

			  match:
				/* Skip the rest of the [...] construct that already matched.  */
				while (c != ']') {
					if (c == '\0')
						return 0;
					c = *p++;
					if (c == '\0')
						return 0;
					else if (c == '\\')
						++p;
				}
				if (invert)
					return 0;
				break;
			}

		default:
			if (c != *t++)
				return 0;
		}

	return *t == '\0';
}

const char *Sys_FindFirst (const char *filespec)
{
	struct dirent *d;
	char *p;

	if (fdir)
		TDM_Error ("Sys_FindFirst without close");

	strcpy (findbase, filespec);

	if ((p = strrchr(findbase, '/')) != NULL)
	{
		*p = 0;
		strcpy (findpattern, p + 1);
	}
	else
		strcpy (findpattern, "*");

	if (!strcmp(findpattern, "*.*"))
		strcpy(findpattern, "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;

	while ((d = readdir(fdir)) != NULL)
	{
		if (!*findpattern || glob_match(findpattern, d->d_name))
		{
			snprintf (findpath, sizeof(findpath)-1, "%s/%s", findbase, d->d_name);
			return findpath;
		}
	}

	return NULL;
}

const char *Sys_FindNext (void)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;

	while ((d = readdir(fdir)) != NULL)
	{
		if (!*findpattern || glob_match(findpattern, d->d_name))
		{
			snprintf (findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
			return findpath;
		}
	}

	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);

	fdir = NULL;
}

void Sys_DebugBreak (void)
{
	asm ("int $3");
}

#endif
