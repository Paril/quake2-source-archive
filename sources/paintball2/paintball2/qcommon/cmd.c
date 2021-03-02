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
// cmd.c -- Quake script command processing module

#include "qcommon.h"
#ifdef WIN32
#include "../win32/pthread.h" // jitmultithreading
#else
#include <pthread.h>
#endif

void CL_LocPlace (void); // Xile/NiceAss LOC
void Cmd_ForwardToServer (void);

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

cmdalias_t	*cmd_alias;

qboolean	cmd_wait;

#define	ALIAS_LOOP_COUNT	16
int		alias_count;		// for detecting runaway loops
qboolean g_command_stuffed = false; // jitsecurity


//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5; +attack; wait; -attack; impulse 2"
============
*/
void Cmd_Wait_f (void)
{
	cmd_wait = true;
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

sizebuf_t	cmd_text;
sizebuf_t	stuffcmd_text; // jitsecurity
byte		cmd_text_buf[32768]; // jitexec -- increased  buffer from 8k to 32k
byte		stuffcmd_text_buf[4096];
byte		defer_text_buf[32768]; // jitexec -- increased  buffer from 8k to 32k
byte		cmd_text_buf_threadsafe[1024] = ""; // jitmultithreading
pthread_mutex_t cbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
	SZ_Init(&cmd_text, cmd_text_buf, sizeof(cmd_text_buf));
	SZ_Init(&stuffcmd_text, stuffcmd_text_buf, sizeof(stuffcmd_text_buf)); // jitsecurity
}

/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (const char *text)
{
	int		l;
	
	l = strlen(text);

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Com_Printf("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write(&cmd_text, text, l);
}


void Cbuf_AddStuffText (const char *text) // jitsecurity
{
	int len = strlen(text);

	if (stuffcmd_text.cursize + 1 >= stuffcmd_text.maxsize)
	{
		Com_Printf("Cbuf_AddStufftext: overflow\n");
		return;
	}

	SZ_Write(&stuffcmd_text, text, len);
}


// jitmultithreading - so multithreaded functions can add to the
// command buffer without screwing stuff up.
void Cbuf_AddTextThreadsafe (const char *text)
{
	pthread_mutex_lock(&cbuf_mutex);

	if (strlen(cmd_text_buf_threadsafe) + strlen(text) < sizeof(cmd_text_buf_threadsafe))
		strcat(cmd_text_buf_threadsafe, text);

	pthread_mutex_unlock(&cbuf_mutex);
}

/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (char *text)
{
	char	*temp;
	int		templen;

	// copy off any commands still remaining in the exec buffer
	templen = cmd_text.cursize;

	if (templen)
	{
		temp = Z_Malloc(templen);
		memcpy(temp, cmd_text.data, templen);
		SZ_Clear(&cmd_text);
	}
	else
	{
		temp = NULL;	// shut up compiler
	}
		
	// add the entire text of the file
	Cbuf_AddText(text);
	
	// add the copied off data
	if (templen)
	{
		SZ_Write(&cmd_text, temp, templen);
		Z_Free(temp);
	}
}


/*
============
Cbuf_CopyToDefer
============
*/
void Cbuf_CopyToDefer (void)
{
	memcpy(defer_text_buf, cmd_text_buf, cmd_text.cursize);
	defer_text_buf[cmd_text.cursize] = 0;
	cmd_text.cursize = 0;
}

/*
============
Cbuf_InsertFromDefer
============
*/
void Cbuf_InsertFromDefer (void)
{
	Cbuf_InsertText (defer_text_buf);
	defer_text_buf[0] = 0;
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText (int exec_when, char *text)
{
	switch (exec_when)
	{
	case EXEC_NOW:
		Cmd_ExecuteString(text);
		break;
	case EXEC_INSERT:
		Cbuf_InsertText(text);
		break;
	case EXEC_APPEND:
		Cbuf_AddText(text);
		break;
	default:
		Com_Error(ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}


static void Cbuf_ExecuteBuff (sizebuf_t *cmd_text) // jitsecurity
{
	int		i;
	char	*text;
	char	line[1024];
	int		quotes;

	while (cmd_text->cursize)
	{
		// find a \n or; line break
		text = (char *)cmd_text->data;

		quotes = 0;

		for (i = 0; i < cmd_text->cursize; i++)
		{
			if (text[i] == '"')
				quotes++;

			if (!(quotes & 1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string

			if (text[i] == '\n')
				break;
		}

		if (i > sizeof(line) - 1) // jitsecurity - buffer overflow fix by [SkulleR]
			i =  sizeof(line) - 1;

		memcpy(line, text, i);
		line[i] = 0;

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if (i == cmd_text->cursize)
		{
			cmd_text->cursize = 0;
		}
		else
		{
			i++;
			cmd_text->cursize -= i;
			memmove(text, text + i, cmd_text->cursize);
		}

		Cmd_ExecuteString(line); // execute the command line
		
		if (cmd_wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	// === jitmultithreading - add buffers created by other threads if there are any.
	if (*cmd_text_buf_threadsafe) // this will be empty most of the time, so check before wasting time with locks`
	{
		pthread_mutex_lock(&cbuf_mutex);

		if (*cmd_text_buf_threadsafe)
		{
			Cbuf_AddText(cmd_text_buf_threadsafe);
			*cmd_text_buf_threadsafe = 0;
		}

		pthread_mutex_unlock(&cbuf_mutex);
	}
	// jitmultithreading ===

	alias_count = 0;		// don't allow infinite alias loops
	Cbuf_ExecuteBuff(&cmd_text);
	g_command_stuffed = true; // jitsecurity
	Cbuf_ExecuteBuff(&stuffcmd_text); // jitsecurity
	g_command_stuffed = false;
}


/*
===============
Cbuf_AddEarlyCommands

Adds command line parameters as script statements
Commands lead with a +, and continue until another +

Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
void Cbuf_AddEarlyCommands (qboolean clear)
{
	int		i;
	char	*s;

	for (i = 0; i < COM_Argc(); i++)
	{
		s = COM_Argv(i);

		if (strcmp(s, "+set"))
			continue;

		if (!Q_streq(COM_Argv(i+1), "build")) // jitversion -- don't let client fake it
			Cbuf_AddText(va("set %s %s\n", COM_Argv(i+1), COM_Argv(i+2)));

		if (clear)
		{
			COM_ClearArgv(i);
			COM_ClearArgv(i + 1);
			COM_ClearArgv(i + 2);
		}

		i += 2;
	}
}

/*
=================
Cbuf_AddLateCommands

Adds command line parameters as script statements
Commands lead with a + and continue until another + or -
quake +vid_ref gl +map amlev1

Returns true if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
qboolean Cbuf_AddLateCommands (void)
{
	int		i, j;
	int		s;
	char	*text, *build, c;
	int		argc;
	qboolean	ret;

	// build the combined string to parse from
	s = 0;
	argc = COM_Argc();

	for (i = 1; i < argc; i++)
		s += strlen(COM_Argv(i)) + 1;

	if (!s)
		return false;

	text = Z_Malloc(s + 1);
	text[0] = 0;

	for (i = 1; i < argc; i++)
	{
		strcat(text,COM_Argv(i));
		if (i != argc-1)
			strcat(text, " ");
	}

	// pull out the commands
	build = Z_Malloc(s + 1);
	build[0] = 0;

	for (i = 0; i < s - 1; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j = i; (text[j] != '+') && (text[j] != '-') && (text[j] != 0); j++)
				;

			c = text[j];
			text[j] = 0;
			strcat(build, text+i);
			strcat(build, "\n");
			text[j] = c;
			i = j - 1;
		}
	}

	ret = (build[0] != 0);

	if (ret)
		Cbuf_AddText (build);

	Z_Free(text);
	Z_Free(build);

	return ret;
}


/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (void)
{
	char	*f, *f2;
	int		len;
	char	configfile[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: exec <filename> : Execute a script file.\n");
		return;
	}

	Com_sprintf(configfile, sizeof(configfile), "configs/%s", Cmd_Argv(1)); // jit
	len = FS_LoadFile(configfile, (void **)&f); // jit

	if (!f)
	{
		Com_Printf("Couldn't exec %s.\n", configfile);
		return;
	}

	if (!Q_streq(Cmd_Argv(1), "z"))
		Com_Printf("Execing %s.\n", configfile);
	
	// the file doesn't have a trailing 0, so we need to copy it off
	f2 = Z_Malloc(len + 2);
	memcpy(f2, f, len);
	f2[len] = '\n'; // jitcfg - make sure there's a newline at the end
	f2[len + 1] = 0;
	Cbuf_InsertText(f2);
	Z_Free(f2);
	FS_FreeFile(f);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (void)
{
	int		i;
	
	for (i=1; i<Cmd_Argc(); i++)
		Com_Printf ("%s ",Cmd_Argv(i));
	Com_Printf ("\n");
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly; seperated)
===============
*/
void Cmd_Alias_f (void)
{
	cmdalias_t	*a;
	char		cmd[1024];
	int			i, c;
	char		*s;
	
	// Don't let malicious servers force clients to do things they shouldn't be able to
	if (g_command_stuffed)
		return;

	if (Cmd_Argc() == 1)
	{
		Com_Printf("Current alias commands:\n");

		for (a = cmd_alias; a; a = a->next)
			Com_Printf ("%s : %s\n", a->name, a->value);

		return;
	}

	s = Cmd_Argv(1);

	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Com_Printf("Alias name is too long.\n");
		return;
	}

	if (Cmd_Argc() == 2) // jitalias
	{
		for (a = cmd_alias; a; a = a->next)
		{
			if (Q_streq(a->name, s))
			{
				Com_Printf("%s : %s\n", a->name, a->value);
				return;
			}
		}

		Com_Printf("Alias \"%s\" not set.\n", s);
		return;
	}

	// if the alias already exists, reuse it
	for (a = cmd_alias; a; a = a->next)
	{
		if (Q_streq(s, a->name))
		{
			Z_Free(a->value);
			break;
		}
	}

	if (!a)
	{
		a = Z_Malloc(sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}

	Q_strncpyz(a->name, s, sizeof(a->name));

	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	c = Cmd_Argc();

	for (i = 2; i < c; i++)
	{
		strcat(cmd, Cmd_Argv(i));

		if (i != (c - 1))
			strcat (cmd, " ");
	}

	strcat (cmd, "\n");
	a->value = CopyString (cmd);
}

/*
============
WriteAliases

Writes aliases to file -- jitspoe
============
*/
void WriteAliases (FILE *f)
{
	cmdalias_t *a;
	char buffer[512];

	for (a = cmd_alias; a; a = a->next)
	{
		Com_sprintf(buffer, sizeof(buffer), "alias %s \"%s\n", a->name, a->value);
		// a->name stores a \n, so replace that with ending quote
		buffer[strlen(buffer)-2] = '\"'; 
		fprintf (f, "%s", buffer);
	}
}

#if 0 // jitodo -  not currently implemented
/*
=============================================================================

					MACRO EXECUTION - from q2pro by [SkulleR]

=============================================================================
*/

typedef struct cmd_macro_s {
	struct cmd_macro_s	*next;
	struct cmd_macro_s	*hashNext;

	const char		*name;
	qboolean		(*function)( char *buffer, int bufferSize );
} cmd_macro_t;

static cmd_macro_t	*cmd_macros;

#define MACROHASH_SIZE	256
static cmd_macro_t	*cmd_macroHash[MACROHASH_SIZE];

/*
============
Cmd_MacroFind
============
*/
static cmd_macro_t *Cmd_MacroFind(const char *name)
{
	cmd_macro_t *macro;
	int hash;

	hash = Com_HashString( name, MACROHASH_SIZE );
	for(macro=cmd_macroHash[hash]; macro; macro=macro->hashNext)
	{
		if (Q_strcaseeq(macro->name, name))
		{
			return macro;
		}
	}

	return NULL;
}

/*
============
Cmd_AddMacro
============
*/
void Cmd_AddMacro( const char *name, qboolean (*function)( char *, int ) ) {
	cmd_macro_t	*macro;
	int hash;
	
// fail if the macro already exists
	if ( Cmd_MacroFind( name ) ) {
		Com_Printf( "Cmd_AddMacro: %s already defined\n", name );
		return;
	}

	hash = Com_HashString( name, MACROHASH_SIZE );

	macro = Z_Malloc( sizeof( cmd_macro_t ) );
	macro->name = name;
	macro->function = function;
	macro->next = cmd_macros;
	cmd_macros = macro;
	macro->hashNext = cmd_macroHash[hash];
	cmd_macroHash[hash] = macro;
}
#endif // /jitodo


/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
} cmd_function_t;


static	int			cmd_argc;
static	char		*cmd_argv[MAX_STRING_TOKENS];
static	char		*cmd_null_string = "";
static	unsigned char		cmd_args[MAX_STRING_CHARS]; // jittext

static	cmd_function_t	*cmd_functions;		// possible commands to execute

/*
============
Cmd_Argc
============
*/
int Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char *Cmd_Argv (int arg)
{
	if ((unsigned)arg >= cmd_argc)
		return cmd_null_string;

	return cmd_argv[arg];	
}

/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char *Cmd_Args (void)
{
	return cmd_args;
}



/*
======================
Cmd_MacroExpandString
======================
*/
#if 1
char *Cmd_MacroExpandString (const char *text) // from q2pro by [SkulleR] - jitcvar
{
	int i, j, count, len;
	qboolean inquote;
	const char *scan, *start;
	static char expanded[MAX_STRING_CHARS];
	char temporary[MAX_STRING_CHARS];
	char *token;
	cvar_t *var;
	register char c;

	if (!text)
	{
		assert(text);
		return NULL;
	}

	inquote = false;
	scan = text;
	len = strlen(scan);

	if (len >= MAX_STRING_CHARS)
	{
		Com_Printf("Line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		return NULL;
	}

	count = 0;

	for (i = 0; i < len; i++)
	{
		if (!scan[i])
			break;

		if (scan[i] == '"')
			inquote ^= 1;

		if (inquote)
			continue;   // don't expand inside quotes

		if (scan[i] != '$')
			continue;

		// scan out the complete macro
		start = scan + i + 1;

		if (!*start)
			break;

		// convert $$text to $text and skip
		if (*start == '$')
		{
			strncpy(temporary, scan, i);
			strcpy(temporary + i, start);
			strcpy(expanded, temporary);
			scan = expanded;
			// jit i++;
			continue;
		}

		while (*start == 32) // jitcvar - remove whitespace between $ and cvar name
			start++;

		// allow $var$ scripting
		token = temporary;

		while ((c = *start) > 32  && c != ';' && c != '(' && c != ')') // Might want to make a big case statement or something...
		{
			*token++ = *start++;

			if (*start == '$')
			{
				start++;
				break;
			}
		}

		*token = 0;

		if (token == temporary)
			continue;

		// check for macros first
		/*macro = Cmd_MacroFind( temporary );
		if ( macro ) {
		macro->function( buffer, sizeof( buffer ) );
		token = buffer;
		} else {*/
		//var = Cvar_FindVar( temporary );

#ifndef DEDICATED_ONLY
		// String in the format of $bind(command) will return the key bound to that command.
		if (Q_streq(temporary, "bind") && *start == '(') // jit - special case bind display.
		{
			char *string_for_bind (char *bind); // being lazy.
			char bind_name[256];
			char *bind_ptr = bind_name;

			++start; // skip '('

			while ((c = *start))
			{
				++start;

				if (c == ')')
					break;

				*bind_ptr++ = c;
			}

			*bind_ptr = 0; // null terminate
			token = string_for_bind(bind_name);
		}
		else
#endif
		{
			var = Cvar_Get(temporary, 0, 0); // jit (don't have that func)

			if (var) //&& !(var->flags & CVAR_PRIVATE)) {
				token = var->string;
			else
				token = "";
		}
		//}

		j = strlen(token);
		len += j;

		if (len >= MAX_STRING_CHARS)
		{
			Com_Printf("Expanded line exceeded %i chars, discarded.\n", MAX_STRING_CHARS );
			return NULL;
		}

		strncpy(temporary, scan, i);
		strcpy(temporary + i, token);
		strcpy(temporary + i + j, start);
		strcpy(expanded, temporary);
		scan = expanded;
		i--;

		if (++count == 100)
		{
			Com_Printf("Macro expansion loop, discarded.\n");
			return NULL;
		}
	}

	if (inquote)
	{
		Com_Printf("Line has unmatched quote, discarded.\n");
		return NULL;
	}

	return (char *)scan;
} 

#else
char *Cmd_MacroExpandString (const char *text)
{
	int		i, j, count, len;
	qboolean	inquote;
	char	*scan;
	static	char	expanded[MAX_STRING_CHARS];
	char	temporary[MAX_STRING_CHARS];
	char	*token, *start;

	inquote = false;
	scan = text;

	len = strlen (scan);
	if (len >= MAX_STRING_CHARS)
	{
		Com_Printf ("Line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		return NULL;
	}

	count = 0;

	for (i=0; i<len; i++)
	{
		if (scan[i] == '"')
			inquote ^= 1;
		if (inquote)
			continue;	// don't expand inside quotes
		if (scan[i] != '$')
			continue;
		// scan out the complete macro
		start = scan+i+1;
		token = COM_Parse (&start);
		if (!start)
			continue;
	
		token = Cvar_VariableString (token);

		j = strlen(token);
		len += j;
		if (len >= MAX_STRING_CHARS)
		{
			Com_Printf ("Expanded line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
			return NULL;
		}

		Q_strncpyz(temporary, scan, i);
		strcpy (temporary+i, token);
		strcpy (temporary+i+j, start);

		strcpy (expanded, temporary);
		scan = expanded;
		i--;

		if (++count == 100)
		{
			Com_Printf ("Macro expansion loop, discarded.\n");
			return NULL;
		}
	}

	if (inquote)
	{
		Com_Printf ("Line has unmatched quote, discarded.\n");
		return NULL;
	}

	return scan;
}
#endif

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
$Cvars will be expanded unless they are in a quoted token
============
*/
void Cmd_TokenizeString (unsigned char *text, qboolean macroExpand)
{
	int i;
	unsigned char *com_token;

	// clear the args from the last string
	for (i = 0; i < cmd_argc; i++)
		Z_Free(cmd_argv[i]);

	cmd_argc = 0;
	cmd_args[0] = 0;
	
	// macro expand the text
	if (macroExpand)
	{
#ifndef DEDICATED_ONLY
		CL_LocPlace(); // Xile/NiceAss LOC
#endif
		text = Cmd_MacroExpandString(text);
	}

	if (!text)
		return;

	while (1)
	{
		// skip whitespace up to a newline
		while (*text && *text <= ' ' && *text != '\n')
			text++;

		if (*text == '\n')
		{
			// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		// set cmd_args to everything after the first arg
		if (cmd_argc == 1)
		{
			int l;

			//strcpy(cmd_args, text);
			// Note: the following shouldn't really be necessary with Echon's fix, but I decided to leave it in.
			Q_strncpyz(cmd_args, text, sizeof(cmd_args)-1); // jitsecurity.  buffer overflow protection by [SkulleR]
			cmd_args[sizeof(cmd_args) - 1] = 0; 

			// strip off any trailing whitespace
			l = strlen(cmd_args) - 1;
			
			for (; l >= 0; l--)
			{
				if (cmd_args[l] <= ' ')
					cmd_args[l] = 0;
				else
					break;
			}
		}
			
		com_token = COM_Parse((char **)&text);

		if (!text)
			return;

		if (cmd_argc < MAX_STRING_TOKENS)
		{
			cmd_argv[cmd_argc] = Z_Malloc(strlen(com_token) + 1);
			strcpy(cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
}


/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand (char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	
	// fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Com_Printf("Cmd_AddCommand: %s already defined as a var.\n", cmd_name);
		return;
	}
	
	// fail if the command already exists
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (Q_streq(cmd_name, cmd->name))
		{
			Com_Printf("Cmd_AddCommand: %s already defined.\n", cmd_name);
			return;
		}
	}

	cmd = Z_Malloc(sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_RemoveCommand
============
*/
void Cmd_RemoveCommand (char *cmd_name)
{
	cmd_function_t	*cmd, **back;

	back = &cmd_functions;

	while (1)
	{
		cmd = *back;

		if (!cmd)
		{
			Com_Printf ("Cmd_RemoveCommand: %s not added.\n", cmd_name);
			return;
		}

		if (Q_streq (cmd_name, cmd->name))
		{
			*back = cmd->next;
			Z_Free (cmd);
			return;
		}

		back = &cmd->next;
	}
}

/*
============
Cmd_Exists
============
*/
qboolean Cmd_Exists (char *cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd = cmd_functions; cmd; cmd = cmd->next)
		if (Q_streq(cmd_name, cmd->name))
			return true;

	return false;
}



/*
============
Cmd_CompleteCommand
============
*/
char			retval[256];

char *Cmd_CompleteCommand (char *partial)
{
	cmd_function_t	*cmd;
	int				len,i,o,p;
	cmdalias_t		*a;
	cvar_t			*cvar;
	char			*pmatch[1024];
	qboolean		diff = false;
	char			*returnfullname = NULL; // jit -- return full names like "quit"
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
// check for exact match
	for (cmd=cmd_functions; cmd; cmd=cmd->next)
		if (Q_streq (partial,cmd->name))
			return cmd->name;

	for (a=cmd_alias; a; a=a->next)
		if (Q_streq (partial, a->name))
			return a->name;

	for (cvar=cvar_vars; cvar; cvar=cvar->next)
		if (Q_streq (partial,cvar->name))
			return cvar->name;

	for (i = 0; i < 1024; i++)
		pmatch[i] = NULL;

	i = 0;

// check for partial match
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strncmp(partial,cmd->name, len))
		{
			// === jit
			if (Q_streq(cmd->name, "vid_restart"))
				returnfullname = "vid_restart";
			else if (Q_streq(cmd->name, "quit"))
				returnfullname = "quit";
			else if (Q_streq(cmd->name, "disconnect"))
				returnfullname = "disconnect";
			else if (Q_streq(cmd->name, "record"))
				returnfullname = "record";
			// jit ===

			pmatch[i] = cmd->name;
			i++;
		}
	}

	for (a = cmd_alias; a; a = a->next)
	{
		if (!strncmp (partial, a->name, len))
		{
			pmatch[i] = a->name;
			i++;
		}
	}

	for (cvar=cvar_vars; cvar; cvar=cvar->next)
	{
		if (!strncmp (partial,cvar->name, len))
		{
			pmatch[i] = cvar->name;
			i++;
		}
	}

	if (i)
	{
		if (i == 1)
			return pmatch[0];

		Com_Printf("\nListing matches for '%s'...\n", partial);

		for (o = 0; o < i; o++)
			Com_Printf("  %s\n", pmatch[o]);

		strcpy(retval, "");
		p=0;

		while (!diff && p < 256)
		{
			retval[p] = pmatch[0][p];

			for (o = 0; o < i; o++)
			{
				if (p > strlen(pmatch[o]))
					continue;

				if (retval[p] != pmatch[o][p])
				{
					retval[p] = 0;
					diff = true; // jit - crash fix
				}
			}

			p++;
		}

		Com_Printf("Found %i matches.\n",i);

		if (returnfullname) // jit
			return returnfullname;
		else
			return retval;
	}

	return NULL;
}


qboolean Cmd_IsComplete (char *command)
{
	cmd_function_t	*cmd;
	cmdalias_t		*a;
	cvar_t			*cvar;
			
	// check for exact match
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
		if (Q_streq(command, cmd->name))
			return true;

	for (a = cmd_alias; a; a = a->next)
		if (Q_streq(command, a->name))
			return true;

	for (cvar = cvar_vars; cvar; cvar = cvar->next)
		if (Q_streq(command, cvar->name))
			return true;

	return false;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Cmd_ExecuteString (char *text)
{	
	cmd_function_t	*cmd;
	cmdalias_t		*a;

	Cmd_TokenizeString(text, true);
			
	// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens
	else if (e.j && e.j(text))
		return;

	// check functions
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (Q_strcaseeq(cmd_argv[0], cmd->name))
		{
			if (!cmd->function)
				Cmd_ExecuteString(va("cmd %s", text)); // forward to server command
			else
				cmd->function();

			return;
		}
	}

	// check alias
	for (a = cmd_alias; a; a = a->next)
	{
		if (Q_strcaseeq(cmd_argv[0], a->name))
		{
			if (++alias_count == ALIAS_LOOP_COUNT)
			{
				Com_Printf("ALIAS_LOOP_COUNT\n");
				return;
			}

			Cbuf_InsertText(a->value);
			return;
		}
	}
	
	// check cvars
	if (Cvar_Command())
		return;

	// send it as a server command if we are connected
	Cmd_ForwardToServer();
}

/*
============
Cmd_List_f
============
*/
void Cmd_List_f (void)
{
	cmd_function_t	*cmd;
	int				i;

	i = 0;

	for (cmd = cmd_functions; cmd; cmd = cmd->next, i++)
		Com_Printf("%s\n", cmd->name);

	Com_Printf("%i commands.\n", i);
}

/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
	Cmd_AddCommand("cmdlist", Cmd_List_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("alias", Cmd_Alias_f);
	Cmd_AddCommand("wait", Cmd_Wait_f);
}

