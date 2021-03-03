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
#include "client.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256
char		key_lines[32][MAXCMDLINE]; // jittext
int			key_linepos;
int			shift_down = false;
int			ctrl_down = false; // jitkeyboard
int			alt_down = false; // jitkeyboard
int			anykeydown;

int			edit_line=0;
int			history_line=0;

int			key_waiting;
char		*keybindings[256]; // jittext
qboolean	consolekeys[256];	// if true, can't be rebound while in console
byte		keyshift[256];		// key to map to if shift held down in console
int			key_repeats[256];	// if > 1, it is autorepeating
qboolean	keydown[256];
qboolean	gamekeydown[256]; // jitmenu
qboolean	key_insert = true; // pooy / jitmenu

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4}, // jitmouse
	{"MOUSE5", K_MOUSE5},
	{"MOUSE6", K_MOUSE6},
	{"MOUSE7", K_MOUSE7},
	{"MOUSE8", K_MOUSE8},
	{"MOUSE9", K_MOUSE9},

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },

	{"MWHEELUP", K_MWHEELUP },
	{"MWHEELDOWN", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

qboolean Cmd_IsComplete (char *cmd);

void CompleteCommand (void)
{
	char	*cmd, *s;

	s = key_lines[edit_line] + 1;

	if (*s == '\\' || *s == '/')
		s++;

	cmd = Cmd_CompleteCommand(s);

	if (cmd)
	{
		key_lines[edit_line][1] = '/';
		strcpy (key_lines[edit_line] + 2, cmd);
		key_linepos = strlen(cmd) + 2;

		if (Cmd_IsComplete(cmd))
		{
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
		}
		else
		{
			key_lines[edit_line][key_linepos] = 0;
		}

		return;
	}
}

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/

int KeyPadKey (int key) // jitmenu
{
	// jitodo -- check numlock
	switch (key)
	{
	case K_KP_SLASH:
		return '/';
	case K_KP_MINUS:
		return '-';
	case K_KP_PLUS:
		return '+';
	case K_KP_HOME:
		return '7';
	case K_KP_UPARROW:
		return '8';
	case K_KP_PGUP:
		return '9';
	case K_KP_LEFTARROW:
		return '4';
	case K_KP_5:
		return '5';
	case K_KP_RIGHTARROW:
		return '6';
	case K_KP_END:
		return '1';
	case K_KP_DOWNARROW:
		return '2';
	case K_KP_PGDN:
		return '3';
	case K_KP_INS:
		return '0';
	case K_KP_DEL:
		return '.';
	default:
		return key;
	}
}

keydest_t Key_GetDest (void) // jitmenu
{
	if (M_MenuActive())
		return key_menu;

	if (cls.state == ca_active)
		return key_game;

	return key_console;
}

void Key_Console (int key) // pooy -- rewritten for text insert mode.
{
	key = KeyPadKey(key); // jit

	// Now that we've added support for foreign keyboards, holding down
	// ctrl+key gives the key a different value.  For the old code below
	// to work, we need to translate that value back to what it originally
	// was.  For example, ctrl+c makes the key 'c' appear as 3 an 'a' as 1.
	// It looks like simply adding 'a' - 1 should fix it.
	if (keydown[K_CTRL] && key <= 26)
		key += 'a' - 1;

	if ((toupper(key) == 'V' && keydown[K_CTRL]) ||
		 (((key == K_INS) || (key == K_KP_INS)) && keydown[K_SHIFT]))
	{
		char *cbd;
		
		if ((cbd = Sys_GetClipboardData()) != 0)
		{
			int i;

			strtok(cbd, "\n\r\b");
			i = strlen(cbd);

			if (i + key_linepos >= MAXCMDLINE)
				i= MAXCMDLINE - key_linepos;

			if (i > 0)
			{
				cbd[i] = 0;
				strcat(key_lines[edit_line], cbd);
				key_linepos += i;
			}

			free(cbd);
		}

		return;
	}

	if (key == 'l') 
	{
		if (keydown[K_CTRL])
		{
			Cbuf_AddText("clear\n");
			return;
		}
	}

	if (key == K_ENTER || key == K_KP_ENTER)
	{
		// backslash text are commands, else chat
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText(key_lines[edit_line] + 2);	// skip the >
		else
			Cbuf_AddText(key_lines[edit_line] + 1);	// valid command

		Cbuf_AddText("\n");
		Com_Printf("%s\n", key_lines[edit_line]);

		if (!Q_streq(key_lines[edit_line], key_lines[(edit_line - 1) & 31])) // jit - don't add a line to the history if it's exactly the same as the last one
		{
			edit_line = (edit_line + 1) & 31;
		}

		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_lines[edit_line][1] = 0;
		key_linepos = 1;

		if (cls.state == ca_disconnected)
			SCR_UpdateScreen();		// force an update, because the command
									// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		CompleteCommand();
		return;
	}
	
	if ((key == K_LEFTARROW) || (key == K_KP_LEFTARROW) || ((key == 'h') && (keydown[K_CTRL])))
	{
		int charcount;

		// jump over invisible color sequences
		charcount = key_linepos;

		if (charcount > 1)
			key_linepos = CharOffset (key_lines[edit_line], charcount - 1);

		return;
	}

	if (key == K_BACKSPACE)
	{
		if (key_linepos > 1)
		{
			// skip to the end of color sequence

			strcpy(key_lines[edit_line] + key_linepos - 1, key_lines[edit_line] + key_linepos);
			key_linepos--;
		}

		return;
	}
	
	if (key == K_DEL)
	{
		if (key_linepos < strlen(key_lines[edit_line]))
			strcpy(key_lines[edit_line] + key_linepos, key_lines[edit_line] + key_linepos + 1);
		return;
	}
	
	if (key == K_INS)
	{ // toggle insert mode
		key_insert = !key_insert;
		return;
	}

	if (key == K_RIGHTARROW)
	{
		if (strlen(key_lines[edit_line]) == key_linepos)
		{
			if (strlen(key_lines[(edit_line + 31) & 31]) <= key_linepos)
				return;

			key_lines[edit_line][key_linepos] = key_lines[(edit_line + 31) & 31][key_linepos];
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
		}
		else
		{
			int charcount;
			// jump over invisible color sequences
			charcount = key_linepos;
			key_linepos = CharOffset(key_lines[edit_line], charcount + 1);
		}
		return;
	}

	if ((key == K_UPARROW) || (key == K_KP_UPARROW) || ((key == 'p') && keydown[K_CTRL]))
	{
		do {
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line && !key_lines[history_line][1]);

		if (history_line == edit_line)
			history_line = (edit_line + 1) & 31;

		Q_strncpyz(key_lines[edit_line], key_lines[history_line], sizeof(key_lines[edit_line]));
		key_linepos = strlen(key_lines[edit_line]);
		return;
	}

	if ((key == K_DOWNARROW) || (key == K_KP_DOWNARROW) ||
		 ((key == 'n') && keydown[K_CTRL]))
	{
		if (history_line == edit_line)
			return;

		do {
			history_line = (history_line + 1) & 31;
		} while (history_line != edit_line
			&& !key_lines[history_line][1]);

		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = ']';
			key_linepos = 1;
		}
		else
		{
			strcpy(key_lines[edit_line], key_lines[history_line]);
			key_linepos = strlen(key_lines[edit_line]);
		}

		return;
	}

	if (key == K_PGUP || key == K_KP_PGUP || key == K_MWHEELUP) // jitscroll
	{
		con.display -= 2;
		return;
	}

	if (key == K_PGDN || key == K_KP_PGDN || key == K_MWHEELDOWN) // jitscroll 
	{
		con.display += 2;

		if (con.display > con.current)
			con.display = con.current;

		return;
	}

	if (key == K_HOME || key == K_KP_HOME )
	{
		if (keydown[K_CTRL])
			con.display = con.current - con.totallines + 10;
		else
			key_linepos = 1;

		return;
	}

	if (key == K_END || key == K_KP_END )
	{
		if (keydown[K_CTRL])
			con.display = con.current;
		else
			key_linepos = strlen(key_lines[edit_line]);

		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable

	if (keydown[K_CTRL]) // jitconsole / jittext
	{
		if (toupper(key) == 'K')
			key = SCHAR_COLOR;
		else if (toupper(key) == 'U')
			key = SCHAR_UNDERLINE;
		else if (toupper(key) == 'I')
			key = SCHAR_ITALICS;
		else if (toupper(key) == 'O')
			key = SCHAR_ENDFORMAT;
	}

	if (key_linepos < MAXCMDLINE-1)
	{
		int i;

		// check insert mode
		if (key_insert)
		{
			// can't do strcpy to move string to right
			//i = strlen(key_lines[edit_line]) - 1;
			i = strlen(key_lines[edit_line]);

			if (i == 254) 
				i--;

			for (; i >= key_linepos; i--)
				key_lines[edit_line][i + 1] = key_lines[edit_line][i];
		}

		// only null terminate if at the end
		i = key_lines[edit_line][key_linepos];
		key_lines[edit_line][key_linepos] = key;
		key_linepos++;

		if (!i)
			key_lines[edit_line][key_linepos] = 0;	
	}
}

//============================================================================

int			chat_team;
char		chat_buffer[MAXCMDLINE];
int			chat_bufferlen = 0;

void Key_Message (int key)
{

	if (key == K_ENTER || key == K_KP_ENTER)
	{
		if (2 == chat_team) // jitlogin
			Cbuf_AddText("login \"");
		else if (1 == chat_team) // jitlogin
			Cbuf_AddText("say_team \"");
		else
			Cbuf_AddText("say \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

		cls.key_dest = Key_GetDest(); // jitmenu
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		cls.key_dest = Key_GetDest(); // jitmenu
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}

	if (chat_bufferlen == sizeof(chat_buffer)-1)
		return; // all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;

	// Null string
	if (!str || !str[0])
		return -1;

	// Single character
	if (!str[1])
		return tolower(str[0]); // jit - case insensitive key binding.

	// Named key (SPACE, MOUSE1, etc.)
	for (kn = keynames; kn->name; kn++)
	{
		if (Q_strcaseeq(str, kn->name))
			return kn->keynum;
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;	
	static char	tinystr[2];
	
	if (keynum == -1)
		return "<KEY NOT FOUND>";

	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = toupper(keynum); // jit - display key bindings as uppercase
		tinystr[1] = 0;
		return tinystr;
	}
	
	for (kn = keynames; kn->name; ++kn)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, const char *binding)
{
	if (keynum == -1)
		return;

	// free old bindings
	if (keybindings[keynum])
	{
		Z_Free(keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
			
	// allocate memory for new binding
	keybindings[keynum] = CopyString(binding);	
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	// jit - Don't let malicious servers force clients to mess up their key bindings.
	if (g_command_stuffed)
		return;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: unbind <key> : Remove commands from a key.\n");
		return;
	}
	
	b = Key_StringToKeynum(Cmd_Argv(1));

	if (b == -1)
	{
		Com_Printf ("\"%s\" isn't a valid key.\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding(b, "");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	// jit - Don't let malicious servers force clients to mess up their key bindings.
	if (g_command_stuffed)
		return;

	for (i = 0; i < 256; ++i)
		if (keybindings[i])
			Key_SetBinding(i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];

	// jit - Don't let malicious servers force clients to mess up their key bindings.
	if (g_command_stuffed)
		return;

	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf("Usage: bind <key> [command] : Attach a command to a key.\n");
		return;
	}

	b = Key_StringToKeynum(Cmd_Argv(1));

	if (b == -1)
	{
		Com_Printf("\"%s\" isn't a valid key.\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Com_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b]);
		else
			Com_Printf("\"%s\" is not bound.\n", Cmd_Argv(1));

		return;
	}
	
	// copy the rest of the command line
	cmd[0] = 0; // start out with a null string

	for (i = 2; i < c; ++i)
	{
		Q_strncatz(cmd, Cmd_Argv(i), sizeof(cmd)); // jitsecurity

		if (i != (c-1))
			Q_strncatz(cmd, " ", sizeof(cmd)); // jitsecurity
	}

	Key_SetBinding(b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]); // jit, added quotes
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			Com_Printf ("%s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;
	char	*keyhistory;

	for (i = 0; i < 32; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}

	key_linepos = 1;

	/*
	// init ascii characters in console mode
	for (i = 32; i < 128; i++)
		consolekeys[i] = true;

	consolekeys[K_ENTER] = true;
	consolekeys[K_KP_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_KP_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_KP_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_KP_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_KP_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_KP_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_KP_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_KP_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_KP_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_KP_INS] = true;
	consolekeys[K_KP_DEL] = true;
	consolekeys[K_KP_SLASH] = true;
	consolekeys[K_KP_PLUS] = true;
	consolekeys[K_KP_MINUS] = true;
	consolekeys[K_KP_5] = true;
	consolekeys[K_MWHEELUP] = true; // jitscroll
	consolekeys[K_MWHEELDOWN] = true; // jitscroll
	consolekeys[K_DEL] = true; // pooy / jit
	*/

	for (i = 1; i < 256; i++) // jitkey - support lots of keys for foreign layouts.
		consolekeys[i] = true;

	consolekeys['`'] = false;
	consolekeys['~'] = false;

	for (i = K_F1; i <= K_F12; i++)
		consolekeys[i] = false; // allow binds for F-keys

	for (i = 0; i < 256; i++)
		keyshift[i] = i;

	for (i = 'a'; i <= 'z'; i++)
		keyshift[i] = i - 'a' + 'A';

	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';
	Cmd_AddCommand("bind", Key_Bind_f);
	Cmd_AddCommand("unbind", Key_Unbind_f);
	Cmd_AddCommand("unbindall", Key_Unbindall_f);
	Cmd_AddCommand("bindlist", Key_Bindlist_f);

	// Load the key line history - jit
	if (FS_LoadFile("configs/keyhistory.dat", (void **)&keyhistory) > 0)
	{
		const char *p;
		size_t size = *(size_t *)keyhistory;

		if (size == sizeof(key_lines) + sizeof(edit_line) + sizeof(history_line))
		{
			p = keyhistory + sizeof(size);
			memcpy(key_lines, p, sizeof(key_lines));
			p += sizeof(key_lines);
			memcpy(&history_line, p, sizeof(history_line));
			p += sizeof(history_line);
			memcpy(&edit_line, p, sizeof(edit_line));
		}
	}
}


void Key_Shutdown (void) // jit
{
	char filename[MAX_QPATH];
	FILE *fp;

	Com_sprintf(filename, sizeof(filename), "%s/configs/keyhistory.dat", FS_Gamedir());

	if ((fp = fopen(filename, "wb")))
	{
		size_t size = sizeof(key_lines) + sizeof(history_line) + sizeof(edit_line);
		fwrite(&size, sizeof(size_t), 1, fp);
		fwrite(key_lines, sizeof(key_lines), 1, fp);
		fwrite(&history_line, sizeof(history_line), 1, fp);
		fwrite(&edit_line, sizeof(edit_line), 1, fp);
		fclose(fp);
	}
}


static void GameKeyup (int key) // jitscores
{
	char *kb;
	char cmd[1024];

	kb = keybindings[key];
	gamekeydown[key] = false;

	if (kb && kb[0] == '+')
	{
		Com_sprintf(cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
		Cbuf_AddText(cmd);
	}

	if (keyshift[key] != key)
	{
		kb = keybindings[keyshift[key]];

		if (kb && kb[0] == '+')
		{
			Com_sprintf(cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
			Cbuf_AddText(cmd);
		}
	}
}

static void GameKeyDown (int key) // jitscores
{
	char *kb;
	char cmd[1024];

	kb = keybindings[key];
	gamekeydown[key] = true;

	if (kb)
	{
		if (kb[0] == '+')
		{	// button commands add keynum and time as a parm
			Com_sprintf(cmd, sizeof(cmd), "%s %i %i\n", kb, key, time);
			Cbuf_AddText(cmd);
		}
		else
		{
			Cbuf_AddText(kb);
			Cbuf_AddText("\n");
		}
	}
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event (int key, qboolean down, unsigned time)
{
	// hack for modal presses
	if (key_waiting == -1)
	{
		if (down)
			key_waiting = key;
		return;
	}

	// update auto-repeat status
	if (down)
	{
		key_repeats[key]++;

		if (key != K_BACKSPACE 
			&& key != K_PAUSE 
			&& key != K_PGUP 
			&& key != K_KP_PGUP 
			&& key != K_PGDN
			&& key != K_KP_PGDN
			&& key != K_MOUSEMOVE // ARTHUR [9/04/03] Allow auto-repeat on mousemove
			&& key != K_LEFTARROW && key != K_RIGHTARROW // pooy
			&& key_repeats[key] > 1)
			return;	// ignore most autorepeats
	}
	else
	{
		key_repeats[key] = 0;
	}

	switch (key) // jitkey
	{
	case K_SHIFT:
		shift_down = down;
		break;
	case K_CTRL:
		ctrl_down =  down;
		break;
	case K_ALT:
		alt_down = down;
		break;
	}

	// console key is hardcoded, so the user can never unbind it
	if (key == '`' || key == '~')
	{
		if (!down)
			return;

		Con_ToggleConsole_f();
		return;
	}

	// menu key is hardcoded, so the user can never unbind it
	if (key == K_ESCAPE)
	{
		if (!down)
			return;

		switch (cls.key_dest)
		{
		case key_message:
			Key_Message(key);
			break;
		case key_menu:
			M_Keydown(key);
			break;
		case key_game:
		case key_console:
			M_Menu_Main_f();
			break;
		default:
			Com_Error(ERR_FATAL, "Bad cls.key_dest.");
		}

		return;
	}

	// track if any key is down for BUTTON_ANY
	keydown[key] = down;

	if (down)
	{
		if (key_repeats[key] == 1)
			anykeydown++;
	}
	else
	{
		anykeydown--;
		if (anykeydown < 0)
			anykeydown = 0;
	}

	if (down)
	{
		int unmodified_key = key;

		// if not a consolekey, send to the interpreter no matter what mode is
		if ((cls.key_dest == key_console && !consolekeys[key]) || // jitmenu
			(cls.key_dest == key_game && (cls.state == ca_active || !consolekeys[key])))
		{
			GameKeyDown(key); // jitmenu
			return;
		}

		// jitkey -- handle upper case and whacky stuff for international keyboards.
		// Note that this is done AFTER checking if it's a game key because we don't
		// want capital letters and whatnot to be bound differently.
		if (shift_down || ctrl_down || alt_down)
			key = keyshift[key];

		switch (cls.key_dest)
		{
		case key_message:
			Key_Message(key);
			break;
		case key_menu:
			if (!M_Keydown(key))
				GameKeyDown(unmodified_key);
			break;
		case key_game:
		case key_console:
			Key_Console(key);
			break;
		default:
			Com_Error (ERR_FATAL, "Bad cls.key_dest.");
		}
	}
	else
	{
		switch (cls.key_dest) // jitmenu -- we want to activate things when the key goes UP!
		{
		case key_menu:
			if (!M_Keyup(key))
				GameKeyup(key);
			break;
		case key_console:
			if (consolekeys[key]) // only send key release to game if it's not a valid console key
				break; // jitbind -- fix calls of keyup binds while typing in console!
		case key_game:
		default:
			GameKeyup(key);
			break;
		}
	}
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	anykeydown = false;

	for (i = 0; i < 256; i++)
	{
		if (keydown[i] || key_repeats[i])
			Key_Event(i, false, 0);

		keydown[i] = 0;
		key_repeats[i] = 0;
	}
}


/*
===================
Key_GetKey
===================
*/
int Key_GetKey (void)
{
	key_waiting = -1;

	while (key_waiting == -1)
		Sys_SendKeyEvents();

	return key_waiting;
}

