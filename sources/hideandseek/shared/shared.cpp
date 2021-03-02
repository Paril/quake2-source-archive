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
// shared.c
//

#include "shared.h"

/*
==============================================================================

	PARSING
 
==============================================================================
*/

/*
============
Com_Parse

Parse a token out of a string
============
*/
static char com_token[MAX_TOKEN_CHARS];
char *Com_Parse(char **dataPtr)
{
	int		c;
	int		len;
	char	*data;

	data = *dataPtr;
	len = 0;
	com_token[0] = 0;
	
	if (!data)
	{
		*dataPtr = NULL;
		return "";
	}
		
	// Skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
		{
			*dataPtr = NULL;
			return "";
		}

		data++;
	}
	
	// Skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

	// Handle quoted strings specially
	if (c == '\"')
	{
		data++;
		for ( ; ; )
		{
			c = *data++;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				*dataPtr = data;
				return com_token;
			}
			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// Parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c > 32);

	if (len >= MAX_TOKEN_CHARS)
		len = 0;
	com_token[len] = 0;

	*dataPtr = data;
	return com_token;
}


/*
============
Com_DefaultExtension

If there is no extnsion in 'path', suffix 'extension'.
============
*/
void Com_DefaultExtension(char *path, char *extension, size_t size)
{
	if (!path[0])
		return;

	// If path doesn't have an .ext, append extension (extension should include the .)
	char *src = path + strlen(path) - 1;
	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;		// It has an extension
		src--;
	}

	Q_strcatz(path, extension, size);
}


/*
============
Com_FileBase
============
*/
void Com_FileBase(char *in, char *out)
{
	char *s, *s2;

	s = in + strlen(in) - 1;
	while (s != in && *s != '.')
		s--;

	for (s2=s ; s2 != in && *s2 != '/' ; s2--) ;

	if (s-s2 < 2)
	{
		out[0] = 0;
	}
	else
	{
		s--;
		strncpy (out, s2+1, s-s2);
		out[s-s2] = 0;
	}
}


/*
============
Com_FileExtension
============
*/
void Com_FileExtension(char *path, char *out, size_t size)
{
	while (*path && *path != '.')
		path++;
	if (!*path)
		return;

	uint32 i;
	path++;
	for (i=0 ; i<size-1 && *path ; i++, path++)
		out[i] = *path;
	out[i] = 0;
}


/*
============
Com_FilePath

Returns the path up to, but not including the last /
============
*/
void Com_FilePath(char *path, char *out, size_t size)
{
	if (size)
	{
		char *s = path + strlen(path) - 1;
		while (s != path && *s != '/')
			s--;

		Q_strncpyz(out, path, size);
		if (s-path < (int)size) // FIXME
			out[s-path] = '\0';
	}
}


/*
============
Com_NormalizePath
============
*/
void Com_NormalizePath(char *Dest, size_t DestSize, const char *Source)
{
	// Convert '\\' to '/'
	size_t OutLength = 0;
	for (size_t SrcIndex=(Source[0] == '\\' || Source[0] == '/') ? 1 : 0 ; Source[SrcIndex] && OutLength<DestSize-2 ; )
	{
		if (Source[SrcIndex] == '\\')
			Dest[OutLength] = '/';
		else
			Dest[OutLength] = Source[SrcIndex];

		OutLength++;
		SrcIndex++;
	}
	Dest[OutLength] = '\0';

	// Strip out "./" and collapse "../"
	for (char *Cur=strchr(Dest, '/') ; Cur != '\0' ; )
	{
		Cur++;

		char *Fwd;
		if (*Cur == '/')
		{
			// Skip all slashes
			for (Fwd=Cur ; (*Fwd == '/') ; Fwd++, OutLength--) ;
		}
		else if (*Cur == '.' && *(Cur+1) == '/')
		{
			// Find where we're skipping to, past all instances of "./"
			for (Fwd=Cur ; (*Fwd == '.' && *(Fwd+1) == '/') ; Fwd+=2, OutLength-=2) ;
		}
		else if (*Cur == '.' && *(Cur+1) == '.' && *(Cur+2) == '/')
		{
			// Find where we're skipping to, past all isntances of "../"
			int NumBackSteps = 0;
			for (Fwd=Cur ; (*Fwd == '.' && *(Fwd+1) == '.' && *(Fwd+2) == '/') ; Fwd+=3, OutLength-=3)
				NumBackSteps++;

			// Find where we're moving back to
			while (NumBackSteps--)
				for (Cur--, OutLength-- ; (*(Cur-1) != '/' && Cur-Dest) ; Cur--, OutLength--) ;
		}
		else
		{
			// Move along, nothing to see here folks
			Cur = strchr(Cur, '/');
			continue;
		}

		memmove(Cur, Fwd, OutLength-(Cur-Dest)+1);
		Cur = strchr(Dest, '/');
	}
}


/*
============
Com_SkipPath
============
*/
char *Com_SkipPath(char *pathName)
{	
	char *last = pathName;
	while (*pathName)
	{
		switch (*pathName)
		{
		case '/':
		case '\\':
			last = pathName+1;
			break;
		}
		pathName++;
	}
	return last;
}


/*
============
Com_SkipRestOfLine
============
*/
void Com_SkipRestOfLine(char **dataPtr)
{
	int c;
	char *data = *dataPtr;
	while ((c = *data++) != 0)
	{
		if (c == '\n')
			break;
	}

	*dataPtr = data;
}


/*
============
Com_SkipWhiteSpace
============
*/
char *Com_SkipWhiteSpace(char *dataPtr, bool *hasNewLines)
{
	int		c;

	while ((c = *dataPtr) <= ' ')
	{
		switch (c)
		{
		case 0:
			return NULL;

		case '\n':
			*hasNewLines = true;
			break;
		}

		dataPtr++;
	}

	return dataPtr;
}


/*
============
Com_StripExtension
============
*/
void Com_StripExtension(char *dest, size_t size, const char *src)
{
	if (size)
	{
		while (--size && *src != '.' && (*dest++ = *src++)) ;
		*dest = '\0';
	}
}


/*
=============
Com_StripPadding

Removes spaces from the left/right of the string
=============
*/
void Com_StripPadding (char *in, char *dest)
{
	for (bool bHitChar=false ; *in ; )
	{
		if (bHitChar)
		{
			*dest++ = *in++;
		}
		else if (*in != ' ')
		{
			bHitChar = true;
			*dest++ = *in++;
		}
		else
		{
			*in++;
		}
	}
	*dest = '\0';

	dest = dest + strlen(dest)-1;
	while (*dest && *dest == ' ')
	{
		*dest = '\0';
		dest--;
	}
}
