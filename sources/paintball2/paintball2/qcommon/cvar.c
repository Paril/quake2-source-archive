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
// cvar.c -- dynamic variable tracking

#include "qcommon.h"

cvar_t	*cvar_vars;

/*
============
Cvar_InfoValidate
============
*/
static qboolean Cvar_InfoValidate (const char *s)
{
	if (strstr(s, "\\"))
		return false;

	if (strstr(s, "\""))
		return false;

	if (strstr(s, ";"))
		return false;

	return true;
}

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (const char *var_name)
{
	cvar_t	*var;
	
	for (var = cvar_vars; var; var = var->next)
		if (var->name && Q_streq(var_name, var->name)) // jit - somehow this was null...
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (const char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar(var_name);

	if (!var)
		return 0;

	return atof(var->string);
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (const char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar(var_name);

	if (!var)
		return "";

	return var->string;
}


/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *Cvar_Get (const char *var_name, const char *var_value, int flags)
{
	cvar_t	*var;
	
	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_name))
		{
			Com_Printf("Invalid info cvar name.\n");
			return NULL;
		}
	}

	var = Cvar_FindVar(var_name);
	if (var)
	{
		var->flags |= flags;
		return var;
	}

	if (!var_value)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_value))
		{
			Com_Printf("Invalid info cvar value.\n");
			return NULL;
		}
	}

	var = Z_Malloc (sizeof(*var));
	var->name = CopyString (var_name);
	var->string = CopyString (var_value);
	var->modified = true;
	var->value = atof (var->string);

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	var->flags = flags;

	return var;
}

/*
============
Cvar_Set2
============
*/
cvar_t *Cvar_Set2 (const char *var_name, const char *value, qboolean force) // jitcvar
{
	cvar_t *var;
	var = Cvar_FindVar(var_name);

	return Cvar_FullSet(var_name, value, var ? var->flags : 0, force);
}

/*
============
Cvar_ForceSet
============
*/
cvar_t *Cvar_ForceSet (const char *var_name, const char *value)
{
	return Cvar_Set2(var_name, value, true);
}

/*
============
Cvar_Set
============
*/
cvar_t *Cvar_Set (const char *var_name, const char *value)
{
	return Cvar_Set2(var_name, value, false);
}

/*
============
Cvar_FullSet
============
*/
cvar_t *Cvar_FullSet (const char *var_name, const char *value, int flags, qboolean force) // jitcvar
{
	cvar_t	*var;

	var = Cvar_FindVar(var_name);

	if (!var)
	{	// create it
		return Cvar_Get(var_name, value, flags);
	}

	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate(value))
		{
			Com_Printf("Invalid info cvar value.\n");
			return var;
		}
	}

	if (!force)
	{
		if (var->flags & CVAR_NOSET)
		{
			Com_Printf("%s is write protected.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_LATCH)
		{
			if (var->latched_string)
			{
				if (Q_streq(value, var->latched_string))
					return var;

				Z_Free(var->latched_string);
			}
			else
			{
				if (Q_streq(value, var->string) && var->flags == flags)
					return var;
			}

			if (Com_ServerState())
			{
				Com_Printf("%s will be changed for next game.\n", var_name);
				var->latched_string = CopyString(value);
			}
			else
			{
				var->string = CopyString(value);
				var->value = atof(var->string);
				var->flags = flags;

				if (Q_streq(var->name, "game"))
				{
					FS_SetGamedir(var->string);
					FS_ExecAutoexec();
					FS_ExecConfig(); // jitconfig -- exec this game dir's config.
				}
			}

			return var;
		}
	}
	else
	{
		if (var->latched_string)
		{
			Z_Free(var->latched_string);
			var->latched_string = NULL;
		}
	}

	if (Q_streq(value, var->string) && var->flags == flags)
		return var;		// not changed

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true;	// transmit at next oportunity
	
	Z_Free(var->string);	// free the old value string
	var->string = CopyString(value);
	var->value = atof(var->string);
	var->flags = flags;

	return var;
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (const char *var_name, float value)
{
	char	val[32];

	if (value == (int)value)
		Com_sprintf(val, sizeof(val), "%i", (int)value);
	else
		Com_sprintf(val, sizeof(val), "%f", value);

	Cvar_Set(var_name, val);
}


/*
============
Cvar_GetLatchedVars

Any variables with latched values will now be updated
============
*/
void Cvar_GetLatchedVars (void)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
	{
		if (!var->latched_string)
			continue;
		Z_Free (var->string);
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = atof(var->string);
		if (Q_streq(var->name, "game"))
		{
			FS_SetGamedir (var->string);
			FS_ExecAutoexec ();
			// jitodo -- load config from this gamedir
		}
	}
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
	cvar_t			*v;

	// check variables
	v = Cvar_FindVar(Cmd_Argv(0));

	if (!v)
		return false;
		
	// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
		Com_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set(v->name, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_Set_f

Allows setting and defining of arbitrary cvars from console
============
*/
void Cvar_Set_f (void)
{
	int		c;
	int		flags;

	c = Cmd_Argc();

	if (c != 3 && c != 4)
	{
		Com_Printf("Usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4)
	{
		if (Q_streq(Cmd_Argv(3), "u"))
			flags = CVAR_USERINFO;
		else if (Q_streq(Cmd_Argv(3), "s"))
			flags = CVAR_SERVERINFO;
		else
		{
			Com_Printf("Flags can only be 'u' or 's'.\n");
			return;
		}

		Cvar_FullSet(Cmd_Argv(1), Cmd_Argv(2), flags, false);
	}
	else
	{
		Cvar_Set(Cmd_Argv(1), Cmd_Argv(2));
	}
}

void Cvar_Seta_f (void) // jitconfig
{
	int		c;
	int		flags = CVAR_ARCHIVE;
	cvar_t	*var;

	c = Cmd_Argc();

	if (c != 3 && c != 4)
	{
		Com_Printf("Usage: seta <variable> <value> [u / s]\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));

	if (var)
		flags |= var->flags;

	if (c == 4)
	{
		if (Q_streq(Cmd_Argv(3), "u"))
			flags |= CVAR_USERINFO;
		else if (Q_streq(Cmd_Argv(3), "s"))
			flags |= CVAR_SERVERINFO;
		else
		{
			Com_Printf("Flags can only be 'u' or 's'.\n");
			return;
		}

		Cvar_FullSet(Cmd_Argv(1), Cmd_Argv(2), flags, false);
	}
	else
	{
		Cvar_FullSet(Cmd_Argv(1), Cmd_Argv(2), flags, false);
	}
}


void Cvar_Unset_f (void)
{
	Cvar_FullSet(Cmd_Argv(1), "", 0, false);
}


void Cvar_Clear_f (void)
{
	Cvar_Set(Cmd_Argv(1), "");
}


/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (char *path)
{
	cvar_t	*var;
	char	buffer[1024];
	FILE	*f;

	f = fopen (path, "a");

	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			Com_sprintf (buffer, sizeof(buffer), "seta %s \"%s\"\n", var->name, var->string); // jitconfig
			fprintf (f, "%s", buffer);
		}
	}

	fclose (f);
}

/*
============
Cvar_List_f

============
*/
void Cvar_List_f (void)
{
	cvar_t	*var;
	int		i;

	for (i = 0, var = cvar_vars; var; var = var->next, i++)
	{
		if (var->flags & CVAR_ARCHIVE)
			Com_Printf("*");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_USERINFO)
			Com_Printf("U");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_SERVERINFO)
			Com_Printf("S");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_NOSET)
			Com_Printf("-");
		else if (var->flags & CVAR_LATCH)
			Com_Printf("L");
		else
			Com_Printf(" ");

		Com_Printf(" %s \"%s\"\n", var->name, var->string);
	}

	Com_Printf("%i cvars.\n", i);
}


qboolean userinfo_modified;


char *Cvar_BitInfo (int bit)
{
	static char	info[MAX_INFO_STRING];
	cvar_t	*var;

	info[0] = 0;

	for (var = cvar_vars; var; var = var->next)
	{
		if (var->flags & bit)
			Info_SetValueForKey(info, var->name, var->string);
	}

	return info;
}

// returns an info string containing all the CVAR_USERINFO cvars
char	*Cvar_Userinfo (void)
{
	return Cvar_BitInfo(CVAR_USERINFO);
}

// returns an info string containing all the CVAR_SERVERINFO cvars
char	*Cvar_Serverinfo (void)
{
	return Cvar_BitInfo(CVAR_SERVERINFO);
}

/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init (void)
{
	Cmd_AddCommand("set", Cvar_Set_f);
	Cmd_AddCommand("seta", Cvar_Seta_f); // jitconfig
	Cmd_AddCommand("unset", Cvar_Unset_f);
	Cmd_AddCommand("cvar_clear", Cvar_Clear_f);
	Cmd_AddCommand("cvarlist", Cvar_List_f);
}
