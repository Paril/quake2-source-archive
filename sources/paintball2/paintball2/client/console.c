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
// console.c

#include "client.h"

console_t	con;

cvar_t		*con_notifytime;


#define		MAXCMDLINE	256
extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
		

void Draw_StringLen (float x, float y, char *str, int len) // pooy
{
	char saved_byte;

	if (len < 0)
		re.DrawString(x, y, str);

	saved_byte = str[len];
	str[len] = 0;
	re.DrawString(x, y, str);
	str[len] = saved_byte;
}

int CharOffset (const char *s, int charcount) // pooy
{
	const char *start = s;

	for ( ; *s && charcount; ++s)
	{
		--charcount;
	}

	return s - start;


/* jit (this doesn't work!!!)
	char *start = s;

	while(*s && charcount)
	{
		if (*(s+1) && (*s == CHAR_UNDERLINE || *s == CHAR_ITALICS))
		{
			// don't count character
		}
		else if (*(s+1) && *s == CHAR_COLOR)
		{
			s++; // skip two characters.
		}
		else
			charcount--;

		s++;
	}

	return s - start;
/*
	// jittext / jitconsole:
	char temp;
	int offset;

	temp = s[charcount];
	s[charcount] = 0;
	offset = strlen_noformat(s);
	s[charcount] = temp;

	return offset;*/
}

int Con_GetLinePosNoFormat () // jittext / jitconsole
{
	char temp;
	int colorlinepos;

	temp = key_lines[edit_line][key_linepos];
	key_lines[edit_line][key_linepos] = 0;
	colorlinepos = strlen_noformat(key_lines[edit_line]);
	key_lines[edit_line][key_linepos] = temp;

	return colorlinepos;
}

void DrawAltString (int x, int y, char *s)
{
	while (*s)
	{
		re.DrawChar(x, y, *s ^ 0x80);
		x += CHARWIDTH * hudscale;
		s++;
	}
}


void Key_ClearTyping (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
}


/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	SCR_EndLoadingPlaque();	// get rid of loading plaque

	// jitdemo, removed, don't stop demos
/*
	if (cl.attractloop)
	{
		Cbuf_AddText ("killserver\n"); 
		return;
	}

	if (cls.state == ca_disconnected)
	{	// start the demo loop again
		Cbuf_AddText("d1\n");
		return;
	}
*/
	Key_ClearTyping();
	Con_ClearNotify();

	if (cls.key_dest == key_console)
	{
		M_ForceMenuOff();
		//Cvar_Set("paused", "0");
	}
	else
	{
		M_ForceMenuOff ();
		cls.key_dest = key_console;	

		//if (Cvar_VariableValue("maxclients") == 1 
		//	&& Com_ServerState())
		//	Cvar_Set("paused", "1");
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Con_ToggleChat_f (void)
{
	Key_ClearTyping();

	if (cls.key_dest == key_console)
	{
		if (cls.state == ca_active)
		{
			M_ForceMenuOff();
			cls.key_dest = key_game;
		}
	}
	else
		cls.key_dest = key_console;
	
	Con_ClearNotify();
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	memset (con.text, ' ', CON_TEXTSIZE);
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];
	char	filename[MAX_QPATH]; // jitsecurity
	char	*s;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: condump <filename>\n");
		return;
	}

	// === jitsecurity - don't allow .'s in filename
	Q_strncpyz(filename, Cmd_Argv(1), sizeof(filename));
	s = filename;
	while (s = strchr(s, '.'))
	{
		*s = '_';
	}
	// jitsecurity ===

	Com_sprintf(name, sizeof(name), "%s/%s.txt", FS_Gamedir(), filename);
	Com_Printf("Dumped console text to %s.\n", name);
	FS_CreatePath(name);
	f = fopen(name, "w");

	if (!f)
	{
		Com_Printf("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = con.current - con.totallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;

		for (x = 0; x < con.linewidth; x++)
			if (line[x] != ' ')
				break;

		if (x != con.linewidth)
			break;
	}

	// write the remaining lines
	buffer[min(con.linewidth, sizeof(buffer) - 1)] = 0;

	for ( ; l <= con.current; l++)
	{
		line = con.text + (l % con.totallines) * con.linewidth;
		strncpy(buffer, line, min(con.linewidth, sizeof(buffer) - 1));

		for (x = con.linewidth - 1; x >= 0; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}

		strip_garbage(buffer, buffer, sizeof(buffer));
		fprintf(f, "%s\n", buffer);
	}

	fclose(f);
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;
	
	for (i = 0; i < NUM_CON_TIMES; ++i)
		con.times[i] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void)
{
	chat_team = 0;
	cls.key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	chat_team = 1;
	cls.key_dest = key_message;
}

/*
================
Con_MessageModeLogin_f
================
*/
void Con_MessageModeLogin_f (void)
{
	chat_team = 2;
	cls.key_dest = key_message;
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/

void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	if (!hudscale)
		hudscale = 1;

	//width = (viddef.width/hudscale >> 3) - 2;
	width = (int)(viddef.width / (hudscale * CHARWIDTH)) - 2;

	if (width == con.linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		memset (con.text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if (con.totallines < numlines)
			numlines = con.totallines;

		numchars = oldwidth;
	
		if (con.linewidth < numchars)
			numchars = con.linewidth;

		memcpy (tbuf, con.text, CON_TEXTSIZE);
		memset (con.text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con.text[(con.totallines - 1 - i) * con.linewidth + j] =
						tbuf[((con.current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con.linewidth = -1;
	Con_CheckResize();
	Com_Printf("Console initialized.\n");
	con_notifytime = Cvar_Get("con_notifytime", "3", CVAR_ARCHIVE); // jittext (was 3, and no archive)
	Cmd_AddCommand("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand("messagemode", Con_MessageMode_f);
	Cmd_AddCommand("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand("messagemodelogin", Con_MessageModeLogin_f);
	Cmd_AddCommand("clear", Con_Clear_f);
	Cmd_AddCommand("condump", Con_Dump_f);
	con.initialized = true;
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	con.x = 0;
	if (con.display == con.current)
		con.display++;
	con.current++;
	memset (&con.text[(con.current%con.totallines)*con.linewidth]
	, ' ', con.linewidth);
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void Con_Print (const char *txt) // jittext
{
	int		y;
	int		l;
	char c; // jittext
	static int	cr;
	qboolean isitalics = false;
	qboolean isunderlined = false;
	qboolean iscolored = false;
	char	color;

	if (!con.initialized)
		return;

	while ((c = *txt))
	{
		// count word length
		for (l = 0; l < con.linewidth; l++)
			if (txt[l] <= ' ')
				break;

		// word wrap
		if (l != con.linewidth && (con.x + l > con.linewidth))
			con.x = 0;

		txt++;

		if (cr)
		{
			con.current--;
			cr = false;
		}

		if (!con.x)
		{
			Con_Linefeed();

			if (isunderlined) // jittext, continue formatting or wordwrapped lines
			{
				y = con.current % con.totallines;
				con.text[y * con.linewidth + con.x] = SCHAR_UNDERLINE;
				con.x++;
			}

			if (isitalics)
			{
				y = con.current % con.totallines;
				con.text[y * con.linewidth + con.x] = SCHAR_ITALICS;
				con.x++;
			}

			if (iscolored)
			{
				y = con.current % con.totallines;
				con.text[y * con.linewidth + con.x] = SCHAR_COLOR;
				con.text[y * con.linewidth + con.x + 1] = color;
				con.x += 2;
			}

			// mark time for transparent overlay
			if (con.current >= 0)
				con.times[con.current % NUM_CON_TIMES] = cls.realtime;
		}

		if (c == SCHAR_COLOR) // jittext
		{
			iscolored = true;
			color = *txt;
		}
		else if (c == SCHAR_UNDERLINE)
		{
			isunderlined = !isunderlined;
		}
		else if (c == SCHAR_ITALICS)
		{
			isitalics = !isitalics;
		}

		switch (c)
		{
		case '\n':
			con.x = 0;
			break;
		case '\r':
			con.x = 0;
			cr = 1;
			break;
		default:	// display character and advance
			y = con.current % con.totallines;
			con.text[y * con.linewidth + con.x] = c; // jittext | mask | con.ormask;
			con.x++;

			if (con.x >= con.linewidth)
				con.x = 0;

			break;
		}
	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/




/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawCursor (int x, int y) // jittext / jitmenu
{

	if ((int)(cls.realtime>>8)&1)
		//re.DrawChar ( 8+colorlinepos*8, con.vislines-18, key_insert ? '_' : 11);
		re.DrawChar(x, y + hudscale, key_insert ? '_' : 11);
}

void Con_DrawInput (void) // pooy, jittext
{
	char	*text;
	int		colorlinepos;
	int		byteofs;
	int		bytelen;


	if (cls.key_dest == key_menu)
		return;

	if (cls.key_dest != key_console && cls.state == ca_active)
		return;		// don't draw anything (always draw if not active)

	text = key_lines[edit_line];

	// convert byte offset to visible character count
	colorlinepos = Con_GetLinePosNoFormat();

	// prestep if horizontally scrolling
	if (colorlinepos >= con.linewidth + 1)
	{
		byteofs = CharOffset(text, colorlinepos - con.linewidth);
		text += byteofs;
		colorlinepos = con.linewidth;
	}

	// draw it
	bytelen = CharOffset(text, con.linewidth);	
		
	Draw_StringLen(CHARWIDTH * hudscale, con.vislines - 22 * hudscale, text, bytelen);

	// add the cursor frame
	Con_DrawCursor(CHARWIDTH * hudscale + colorlinepos * CHARWIDTH * hudscale, con.vislines - 22 * hudscale);
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		v;
	char	*text;
	int		i;
	int		time;
	char	*s;
	int		skip;

	v = 0;

	for (i = con.current - NUM_CON_TIMES + 1; i <= con.current; i++)
	{
		if (i < 0)
			continue;

		time = con.times[i % NUM_CON_TIMES];

		if (time == 0)
			continue;

		time = cls.realtime - time;

		if (time > con_notifytime->value * 1000.0f)
			continue;

		text = con.text + (i % con.totallines) * (con.linewidth);
		Draw_StringLen(CHARWIDTH * hudscale, v, text, con.linewidth); // jit, draw whole line at once
		v += CHARWIDTH * hudscale; // jithudscale
	}

	if (cls.key_dest == key_message)
	{
		if (2 == chat_team) // jitlogin
		{
			re.DrawString(CHARWIDTH*hudscale, v, "Login:");
			skip = 8*hudscale;
		}
		else if (1 == chat_team) // jitlogin
		{
			re.DrawString (CHARWIDTH*hudscale, v, "Say_team:");
			skip = 11*hudscale;
		}
		else
		{
			re.DrawString (CHARWIDTH*hudscale, v, "Say:");
			skip = 6*hudscale;
		}

		s = chat_buffer;

		if (chat_bufferlen > ((viddef.width / (hudscale * CHARWIDTH))) - (skip + 1))
			s += chat_bufferlen - ((int)(viddef.width / (hudscale * CHARWIDTH)) - (skip + 1));

		re.DrawString(skip << 3, v, s);
		re.DrawChar((strlen(s) * hudscale + skip) * 8, v, 10 + ((cls.realtime >> 8) & 1));
		v += CHARWIDTH * hudscale;
	}

	if (v)
	{
		SCR_AddDirtyPoint(0,0);
		SCR_AddDirtyPoint(viddef.width - 1, v);
	}
}

/*
================
Con_DrawConsole

Draws the console with the solid background
Also draws menu background (for now).
================
*/
void Con_DrawConsole (float frac)
{
	int				i, x, y;
	int				rows;
	char			*text;
	int				row;
	float			lines;
	char			version[64];
	float			height = viddef.height;
	float			width = viddef.width;

	lines = height * frac;

	if (lines <= 0.0f)
		return;

	if (lines > height)
		lines = height;

	// draw the background
	re.DrawStretchPic(0.0f, lines - height, width, height, cl_conback->string);
	SCR_AddDirtyPoint(0,0);
	SCR_AddDirtyPoint(viddef.width-1, lines-1);

	Com_sprintf(version, sizeof(version), "%c]v%4.2f Alpha (build %d)", SCHAR_COLOR, VERSION, BUILD); // jit 
	re.DrawString(width - 176.0f * hudscale, lines - 12.0f * hudscale, version);

	if (cls.key_dest == key_menu)
		return; // jitmenu

// draw the text
	con.vislines = lines;
	rows = (lines - 22 * hudscale) / CHARWIDTH;		// rows of text to draw
	y = lines - 30 * hudscale;

// draw from the bottom up
	if (con.display != con.current)
	{
	// draw arrows to show the buffer is backscrolled
		for (x = 0; x < con.linewidth; x += 4)
			re.DrawChar((x * hudscale + 1.0f) * CHARWIDTH, y, '^');
	
		y -= 8 * hudscale;
		rows--;
	}
	
	row = con.display;

	for (i=0 ; i<rows ; i++, y-=8*hudscale, row--)
	{
		if (row < 0)
			break;
		if (con.current - row >= con.totallines)
			break;		// past scrollback wrap point
			
		text = con.text + (row % con.totallines)*con.linewidth;

		Draw_StringLen(CHARWIDTH * hudscale, y, text, con.linewidth); // jit, draw whole line at once
	}

	Con_DrawDownloadBar(true);
// draw the input prompt, user text, and cursor if desired
	Con_DrawInput();
}

void Con_DrawDownloadBar (qboolean inConsole)
{
	int x, y, i, j, n;
	char			*text;
	char			dlbar[1024];
	char			dlbar_fill[1024]; // jittext
	char			translated_text[64];
	//ZOID
	// draw the download bar
	// figure out width
	if (cls.download)
	{
		cvar_t *cs_loadingbarback;
		cvar_t *cs_loadingbarfront;
		cvar_t *cs_downloadspeed;
		int len;

		if (!inConsole)
		{
			cs_loadingbarback = Cvar_Get("cs_loadingbarback", "0", CVAR_NOSET);
			cs_loadingbarfront = Cvar_Get("cs_loadingbarfront", "0", CVAR_NOSET);
			cs_downloadspeed = Cvar_Get("cs_downloadspeed", "0", CVAR_NOSET);
		}

		if ((text = strrchr(cls.downloadname, '/')) != NULL)
			++text;
		else
			text = cls.downloadname;

		if (inConsole)
		{
			x = con.linewidth - ((con.linewidth * 7) * 0.025);
			y = x - strlen(text) - 8;
			i = con.linewidth * 0.3333333333;
		}
		else
		{
			x = 34; // let the compiler bother with that
			y = x;
		}
		
		dlbar[0] = 0;
		
		if (inConsole)
		{
			if (strlen(text) > i)
			{
				y = x - i - 11;
				Q_strncpyzna(dlbar, text, i);
				dlbar[i] = 0;
				strcat(dlbar, "...");
			}
			else
			{
				strcpy(dlbar, text);
			}

			strcat(dlbar, ": ");
		}
		else
		{
			translate_string(translated_text, sizeof(translated_text), "Downloading");
			Cvar_ForceSet("cs_loadingstatus", va("%s %s", translated_text, text));
		}
		
		i = strlen(dlbar);
		dlbar[i++] = '\x80';

		// where's the dot go?
		if (cls.downloadpercent == 0)
			n = 0;
		else
			n = ((float)(y+2) * (float)(cls.downloadpercent) / 100.0f + 0.5f); // jit (round properly)
			
		// === jittext -- new download bar:
		dlbar_fill[0] = SCHAR_COLOR;
		dlbar_fill[1] = 'Q'; // jittext

		for (j = 2; j <= i; ++j) // jittext
			dlbar_fill[j] = ' ';

		for (j = 0; j < n; ++j) // jittext
		{
			if (j == n-1)
				dlbar_fill[j + i + 1] = '\x1f'; // end bar
			else
				dlbar_fill[j + i + 1] = '\x1e'; // middle bar

			if (j == 0)
				dlbar_fill[j + i + 1] = '\x1d'; // start bar
		}

		dlbar_fill[j + i + 1] = '\0';

		for (j = 0; j < y; ++j)
			dlbar[i++] = '\x81';

		dlbar[i++] = '\x82';
		dlbar[i] = 0;
		len = strlen(dlbar);

#ifdef USE_DOWNLOAD3
		if (cls.download3rate)
		{
			if (cls.download3rate > 1048576.0f)
			{
				if (inConsole)
					Com_sprintf(dlbar + len, sizeof(dlbar) - len, " %02d%% %1.2fMB/s", cls.downloadpercent, cls.download3rate / 1048576.0f);
				else
					Cvar_ForceSet("cs_downloadspeed", va("%02d%% %1.2fMB/s", cls.downloadpercent, cls.download3rate / 1048576.0f));
			}
			else if (cls.download3rate > 1024.0f)
			{
				if (inConsole)
					Com_sprintf(dlbar + len, sizeof(dlbar) - len, " %02d%% %1.2fKB/s", cls.downloadpercent, cls.download3rate / 1024.0f);
				else
					Cvar_ForceSet("cs_downloadspeed", va("%02d%% %1.2fKB/s", cls.downloadpercent, cls.download3rate / 1024.0f));
			}
			else
			{
				if	(inConsole)
					Com_sprintf(dlbar + len, sizeof(dlbar) - len, " %02d%% %1.2fB/s", cls.downloadpercent, cls.download3rate);
				else
					Cvar_ForceSet("cs_downloadspeed", va("%02d%% %1.2fB/s", cls.downloadpercent, cls.download3rate));
			}
		}
		else
#endif
		{
			if (inConsole)
				Com_sprintf(dlbar + len, sizeof(dlbar) - len, " %02d%%", cls.downloadpercent);
			else
				Cvar_ForceSet("cs_downloadspeed",va("%02d%%", cls.downloadpercent));
		}

		// draw it
		y = con.vislines - 12 * hudscale;
		if(!inConsole)
		{
			Cvar_ForceSet("cs_loadingbarback", dlbar_fill);
			Cvar_ForceSet("cs_loadingbarfront", dlbar);
		}
		else
		{
			re.DrawString(CHARWIDTH * hudscale, y, dlbar_fill);  // jit
			re.DrawString(CHARWIDTH * hudscale, y, dlbar); // jit, draw whole line at once
		}
		// jittext ===
	}
//ZOID
}