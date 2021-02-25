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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "q_shared.h"

#include <cassert>
#include <cstdarg>

//====================================================================================

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char	*va(const char *format, ...)
{
	va_list		argptr;
	static char	string[1024];
	
	va_start (argptr, format);
	vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	return string;	
}

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
bool COM_Parse(com_parse_t &parse)
{
	size_t len = 0;

// skip whitespace
whitespace:
	char c;

	while ((c = *parse.start) <= ' ')
	{
		if (c == 0)
		{
			parse.token = std::string_view();
			return false;
		}

		parse.start++;
	}

// skip // comments
	if (c == '/' && parse.start[1] == '/')
	{
		while (*parse.start && *parse.start != '\n')
			parse.start++;
		goto whitespace;
	}
// handle quoted strings specially
	else if (c == '\"')
	{
		parse.start++;
		const char *token_start = parse.start;

		while (1)
		{
			c = *parse.start++;

			if (c == '\"' || !c)
			{
				parse.token = std::string_view(token_start, len);
				return true;
			}

			len++;
		}
	}

	const char *token_start = parse.start;

// parse a regular word
	do
	{
		len++;
		c = *parse.start++;
	} while (c > 32);
	
	parse.token = std::string_view(token_start, len - 1);
	return true;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

#include <charconv>

static std::string strtolower(const std::string &str)
{
	std::string copy(str);
	std::transform(copy.cbegin(), copy.cend(), copy.begin(), [](const char &c) { return tolower(c); });
	return copy;
}

bool userinfo_t::Parse(const char *s)
{
	if (*s != '\\')
		return false;

	const std::string_view str(s);

	// these chars can up parsing
	if (str.find_first_of("\";", 0, 2) != std::string::npos)
		return false;

	_values.clear();

	size_t key_start = 1, key_end = 0;
	
	while ((key_end = str.find_first_of('\\', key_start)) != std::string::npos)
	{
		const size_t value_end = str.find_first_of('\\', key_end + 1);
		const std::string_view key = str.substr(key_start, key_end - key_start);
		const size_t value_start = key_end + 1;
		const std::string_view value = str.substr(key_end + 1, (value_end == std::string::npos) ? value_end : (value_end - value_start));
		Set(strtolower(std::string(key.data(), key.size())).data(), std::string(value.data(), value.size()).data());

		if (value_end == std::string::npos)
			break;

		key_start = value_end + 1;
	}

	return true;
}

bool userinfo_t::Encode(char *output, size_t output_size)
{
	size_t offset = 0;
	*output = 0;

	char scratch[MAX_INFO_KEY + MAX_INFO_VALUE + 2 + 1];
	char *pos = output;

	for (auto &value : _values)
	{
		snprintf(scratch, sizeof(scratch), "\\%s\\%s", value.first.c_str(), value.second.string.c_str());

		const size_t written = strlen(scratch);

		if (offset + written >= output_size - 1)
		{
			*output = 0;
			return false;
		}

		strcpy(pos, scratch);
		pos += written;
		offset += written;
	}

	return true;
}