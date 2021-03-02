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

extern "C"
{
#include "..\lua\lua.h"
#include "..\lua\lualib.h"
#include "..\lua\lauxlib.h"
};

struct ScriptFunctionTable
{
	const char		*Name;
	lua_CFunction	Function;
};

struct ScriptGlobalTable
{
	const char		*Name;

	union
	{
		char		*String_Value;
		int			Int_Value;
		float		Float_Value;
	}				Value;

	const byte		Type;

	ScriptGlobalTable () :
	  Type(3)
	  {
		  Name = null;
		  Value.String_Value = null;
	  }

	ScriptGlobalTable (const char *name, char *value) :
	  Type(0)
	  {
		  Name = name;
		  Value.String_Value = value;
	  }

	ScriptGlobalTable(char *name, const int value) :
	  Type(1)
	  {
		  Name = name;
		  Value.Int_Value = value;
	  }

	ScriptGlobalTable(char *name, const float value) :
	  Type(2)
	{
		Name = name;
		Value.Float_Value = value;
	}
};

class Script
{
public:
	char name[128];
	lua_State *state;

	Script ();

	double ToNumber (int index);
	bool ToBoolean (int index);
	lua_CFunction ToCFunction (int index);
	int ToInteger (int index);
	const char *ToString (int index);
	const void *ToPointer (int index);
	void *ToUserData(int index);

	void Push (bool boolean);
	void Push(lua_CFunction f);
	void Push(int value);
	void Push(void *p);
	void Push(const char *string);
	void Push();
	void Push(double n);

	void SetGlobal (const char *str);
	void GetGlobal(const char *field);
	void GetField (int index, const char *str);
	void SetField (int index, const char *str);
	void Remove (int index);
	void Call (int arguments, int results);

	bool		IsNumber (int index);
	bool		IsString (int index);
	bool		IsCFunction (int index);
	bool		IsUserData (int index);
	int			Type (int index);
};