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

//
// infostrings.c
//

#include "shared.h"

/*
=============================================================================

	INFO STRINGS

=============================================================================
*/

/*
================
Info_Print
================
*/
void Info_Print (char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;

	while (*s) {
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20) {
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf (0, "%s", key);

		if (!*s) {
			Com_Printf (0, "MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf (0, "%s\n", value);
	}
}


/*
===============
Info_ValueForKey

Searches the string for the given key and returns the associated value, or an empty string.

Uses two buffers to lessen the chance of stomping.
===============
*/
char *Info_ValueForKey (char *s, char *key)
{
	char		pkey[512];
	static char value[2][512];
	static int	valueIndex;
	char		*o;
	
	valueIndex ^= 1;
	if (*s == '\\')
		s++;

	for ( ; ; ) {
		o = pkey;
		while (*s != '\\') {
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueIndex];

		while (*s != '\\' && *s) {
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey))
			return value[valueIndex];

		if (!*s)
			return "";
		s++;
	}
}


/*
==================
Info_RemoveKey
==================
*/
void Info_RemoveKey (char *s, char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strstr (key, "\\"))
		return;

	for ( ; ; ) {
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\') {
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s) {
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey)) {
			strcpy (start, s);	// Remove this part
			return;
		}

		if (!*s)
			return;
	}

}


/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
bool Info_Validate (char *s)
{
	if (strstr (s, "\""))
		return false;
	if (strstr (s, ";"))
		return false;

	return true;
}


/*
==================
Info_SetValueForKey
==================
*/
void Info_SetValueForKey (char *s, char *key, char *value)
{
	char	newPair[MAX_INFO_STRING], *v;
	int		c;

	// Sanity check
	if (strstr (key, "\\")) {
		Com_Printf (PRNT_WARNING, "Can't use keys with a \\\n");
		return;
	}
	if (strstr (value, "\\")) {
		Com_Printf (PRNT_WARNING, "Can't use values with a \\\n");
		return;
	}
	if (strstr (key, ";")) {
		Com_Printf (PRNT_WARNING, "Can't use keys or values with a semicolon\n");
		return;
	}
	if (strstr (key, "\"")) {
		Com_Printf (PRNT_WARNING, "Can't use keys with a \"\n");
		return;
	}
	if (strstr (value, "\"")) {
		Com_Printf (PRNT_WARNING, "Can't use values with a \"\n");
		return;
	}
	if (strlen (key) > MAX_INFO_KEY-1 || strlen (value) > MAX_INFO_KEY-1) {
		Com_Printf (PRNT_WARNING, "Keys and values must be < %i characters.\n", MAX_INFO_KEY);
		return;
	}

	// Remove the key so it can be re-added cleanly
	Info_RemoveKey (s, key);
	if (!value || !strlen (value))
		return;

	// Generate the key and make sure it will fit
	Q_snprintfz (newPair, sizeof(newPair), "\\%s\\%s", key, value);
	if (strlen (newPair) + strlen (s) > MAX_INFO_STRING-1) {
		Com_Printf (PRNT_WARNING, "Info string length exceeded\n");
		return;
	}

	// Only copy ascii values
	s += strlen (s);
	for (v=newPair ; *v ; ) {
		c = *v++;
		c &= 127;	// Strip high bits
		if (c >= 32 && c < 127)
			*s++ = c;
	}
	*s = 0;
}
