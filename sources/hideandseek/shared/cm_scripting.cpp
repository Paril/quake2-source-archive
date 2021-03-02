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
// alias.c
//

#include "shared.h"

Script::Script()
{
}

bool Script::ToBoolean (int index)
{
	return lua_toboolean(state, index) == 1 ? true : false;
}

lua_CFunction Script::ToCFunction (int index)
{
	return lua_tocfunction(state, index);
}

double Script::ToNumber (int index)
{
	return lua_tonumber(state, index);
}

int Script::ToInteger (int index)
{
	return lua_tointeger(state, index);
}

const char *Script::ToString (int index)
{
	return lua_tostring(state, index);
}

const void *Script::ToPointer (int index)
{
	return lua_topointer(state, index);
}

void *Script::ToUserData(int index)
{
	return lua_touserdata(state, index);
}

void Script::Push (bool boolean)
{
	lua_pushboolean(state, boolean ? 1 : 0);
}

void Script::Push(lua_CFunction f)
{
	lua_pushcfunction(state, f);
}

void Script::Push(int value)
{
	lua_pushinteger(state, value);
}

void Script::Push(void *p)
{
	lua_pushlightuserdata(state, p);
}

void Script::Push(const char *string)
{
	lua_pushstring(state, string);
}

void Script::Push()
{
	lua_pushnil(state);
}

void Script::Push(double n)
{
	lua_pushnumber(state, n);
}

void Script::SetGlobal (const char *string)
{
	lua_setglobal(state, string);
}

void Script::GetGlobal(const char *field)
{
	lua_getglobal(state, field);
}

void Script::GetField (int index, const char *str)
{
	lua_getfield(state, index, str);
}

void Script::SetField (int index, const char *str)
{
	lua_setfield(state, index, str);
}

void Script::Remove (int index)
{
	lua_remove(state, index);
}

void Script::Call (int arguments, int results)
{
	int errorC;
	if ((errorC = lua_pcall(state, arguments, results, 0)) != 0)
		Com_Printf (PRNT_ERROR, "Lua error: %s\n", lua_tostring(state, -1));
}

bool Script::IsNumber (int index)
{
	return lua_isnumber(state, index) == 1;
}

bool Script::IsString (int index)
{
	return lua_isstring(state, index) == 1;
}

bool Script::IsCFunction (int index)
{
	return lua_iscfunction(state, index) == 1;
}

bool Script::IsUserData (int index)
{
	return lua_isuserdata(state, index) == 1;
}

int Script::Type (int index)
{
	return lua_type(state, index);
}