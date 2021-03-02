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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

/*

  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  cinematics
  full screen image for quit and victory

  end of unit intermissions

  */

#include "client.h"

float		scr_con_current;	// aproaches scr_conlines at scr_conspeed
float		scr_conlines;		// 0.0 to 1.0 lines of console to display

qboolean	scr_initialized;		// ready to draw

int			scr_draw_loading;

extern cvar_t	*cl_hudscale; //jithudscale
extern cvar_t	*cl_consoleheight; // T3RR0R15T: console height

vrect_t		scr_vrect;		// position of render window on screen


cvar_t		*scr_viewsize;
cvar_t		*scr_conspeed;
cvar_t		*scr_centertime;
cvar_t		*scr_showturtle;
cvar_t		*scr_showpause;
cvar_t		*scr_printspeed;

cvar_t		*scr_netgraph;
cvar_t		*scr_timegraph;
cvar_t		*scr_debuggraph;
cvar_t		*scr_graphheight;
cvar_t		*scr_graphscale;
cvar_t		*scr_graphshift;
cvar_t		*scr_drawall;

typedef struct
{
	int		x1, y1, x2, y2;
} dirty_t;

dirty_t		scr_dirty, scr_old_dirty[2];

char		crosshair_pic[MAX_QPATH];
float		crosshair_width, crosshair_height;

void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);


/*
===============================================================================

BAR GRAPHS

===============================================================================
*/

/*
==============
CL_AddNetgraph

A new packet was just parsed
==============
*/
void CL_AddNetgraph (void)
{
	int		i;
	int		in;
	int		ping;

	// if using the debuggraph for something else, don't
	// add the net lines
	if (scr_debuggraph->value || scr_timegraph->value)
		return;

	for (i = 0; i < cls.netchan.dropped; i++)
		SCR_DebugGraph(30, 0x40);

	for (i = 0; i < cl.surpressCount; i++)
		SCR_DebugGraph(30, 0xdf);

	// see what the latency was on this packet
	in = cls.netchan.incoming_acknowledged & (CMD_BACKUP-1);
	ping = cls.realtime - cl.cmd_time[in];
	ping /= 30;

	if (ping > 30)
		ping = 30;

	SCR_DebugGraph(ping, 0xd0);
}


typedef struct
{
	float	value;
	int		color;
} graphsamp_t;

static	int			current;
#define MAX_GRAPH_VALUES 2048 // jit - so netgraph doesn't wrap around at high res. must be a power of 2
static	graphsamp_t	values[MAX_GRAPH_VALUES];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (float value, int color)
{
	values[current & (MAX_GRAPH_VALUES - 1)].value = value;
	values[current & (MAX_GRAPH_VALUES - 1)].color = color;
	current++;
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	int		a, x, y, w, i, h;
	float	v;
	int		color;

	w = scr_vrect.width;
	x = scr_vrect.x;
	y = scr_vrect.y + scr_vrect.height;
	//re.DrawFill(x, y - scr_graphheight->value,
	//	w, scr_graphheight->value, 8);

	for (a = 0; a < w; a++)
	{
		i = (current - 1 - a + MAX_GRAPH_VALUES) & (MAX_GRAPH_VALUES - 1);
		v = values[i].value;
		color = values[i].color;
		v = v * scr_graphscale->value + scr_graphshift->value;
		
		if (v < 0)
			v += scr_graphheight->value * (1+(int)(-v/scr_graphheight->value));

		h = (int)v % (int)scr_graphheight->value;
		re.DrawFill(x + w-1-a, y - h, 1, h, color);
	}
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str)
{
	char	*s;
	char	line[64];
	int		i, j, l;

	Q_strncpyz(scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime->value;
	scr_centertime_start = cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s)
	{
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	s = str;
	do	
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (s[l] == '\n' || !s[l])
				break;
		for (i=0 ; i<(40-l)*0.5 ; i++)
			line[i] = ' ';

		for (j=0 ; j<l ; j++)
		{
			line[i++] = s[j];
		}

		line[i] = '\n';
		line[i+1] = 0;

		Com_Printf ("%s", line);

		while (*s && *s != '\n')
			s++;

		if (!*s)
			break;
		s++;		// skip the \n
	} while (1);
	Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_ClearNotify ();
}


void SCR_DrawCenterString (void)
{
	char	*start, line[40];
	int		l;
	int		j = 0;
	int		x, y;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = viddef.height * 0.35f;
	else
		y = 48;

	do	
	{
		// scan the width of the line
		for (l = 0; l < sizeof(line); l++)
		{
			if (start[l] == '\n' || !start[l])
				break;

			// jittext - basically like strlen_noformat, except we haave to take into account the \n above.
			switch (start[l])
			{
			case SCHAR_COLOR:
				j += 2;
				break;
			case SCHAR_UNDERLINE:
			case SCHAR_ITALICS:
			case SCHAR_ENDFORMAT:
				j++;
				break;
			}
		}

		x = (viddef.width - (l - j) * CHARWIDTH * hudscale) * 0.5f; // jithudscale
		SCR_AddDirtyPoint(x, y);

		if (l)
		{
			Q_strncpyzna(line, start, min(l + 1, sizeof(line))); // We use l + 1 because we need 1 char for the null
			re.DrawString(x, y, line);
			SCR_AddDirtyPoint(x + l * CHARWIDTH * hudscale, y + CHARHEIGHT * hudscale); // jithudscale	
		}

		y += CHARHEIGHT * hudscale; // jithudscale

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;

		start++;		// skip the \n
	} while (1);
}

void SCR_CheckDrawCenterString (void)
{
	scr_centertime_off -= cl.frametime; // jitnetfps
	
	if (scr_centertime_off <= 0.0f)
		return;

	SCR_DrawCenterString();
}


/*
===============================================================================

POPUP PRINTING - used for tutorials and whatnot - jitspoe

===============================================================================
*/

#define MAX_POPUPS 4
#define POPUP_DISPLAY_TIME 3.0f
#define POPUP_FADE_TIME 0.5f
#define POPUP_POS_Y 20.0f
#define POPUP_WIDTH 256.0f // 80% at 640px, hudscale 2.
#define POPUP_PADDING_Y 10.0f // space between border and text
#define POPUP_PADDING_X 8.0f // space between border and text
#define POPUP_SPACING_Y 8.0f // space between one popup and the next

float scr_popup_time_left[MAX_POPUPS];
char scr_popup_text[MAX_POPUPS][1024];
qboolean scr_popup_behindmenu;

int SCR_WordWrapText (const char *text_in, float width, char *text_out, size_t size_out)
{
	register const char *in = text_in;
	register char *out = text_out;
	register int i_out = 0;
	register char c = *in;
	register const char *wrappable_in = NULL;
	register int wrappable_out = 0;
	register int linecount = 1;
	register int charsize = CHARWIDTH * hudscale; // this will vary depending on character if/when variable sized fonts are supported.
	register int current_line_width = 0;
	qboolean colorchar = false;
	int wrappable_line_width = 0;

	assert(size_out > 0);

	while (c && i_out < size_out)
	{
		switch (c)
		{
		case ' ':
		case '[':
		case ']':
		case '-':
		case '(':
		case ')':
		case '|':
		case ':':
		case ';':
		case '/':
		case '\\':
		case '+':
			wrappable_in = in;
			wrappable_out = i_out;
			wrappable_line_width = current_line_width;
			break;
		case '\r':
			// Probably won't happen, but just in case we do something like try to wordwrap a windows text file, ignore cr's.
			continue;
		}

		// Don't count formatting toward the size.
		if (c == SCHAR_COLOR)
		{
			colorchar = true;
		}
		else if (colorchar)
		{
			colorchar = false;
		}
		else if (c == SCHAR_ITALICS || c == SCHAR_ENDFORMAT || c == SCHAR_UNDERLINE)
		{
			// don't count these.
		}
		else if (c == '\n') // text already has newlines.
		{
			current_line_width = 0;
			++linecount;
			wrappable_in = NULL;
		}
		else
		{
			current_line_width += charsize;
		}

		if (current_line_width > width)
		{
			// Wrap the line at a whitespace, if we have one.
			if (wrappable_in)
			{
				in = wrappable_in;

				// Get rid of any multi-spaces so they don't wrap to the next line.
				while (*in == ' ')
					++in;

				// b42 - prefer to keep these files on the previous line if possible.
				if (wrappable_line_width <= width && wrappable_out < size_out)
				{
					switch (*in)
					{
					case ')':
					case '|':
					case ';':
					case ']':
					case ':':
					case '-':
					case '+':
					case '/':
					case '\\':
						text_out[wrappable_out++] = *in;
						++in;
					}
				}
				// todo: check if wrappable can be left on the first line.
				text_out[wrappable_out] = '\n';
				i_out = wrappable_out + 1;
				wrappable_in = NULL;

				// b42 - Move ahead 1 character if we can so we don't get into an infinite loop of trying to wordwrap on the same character if we have a really long string of non-breakable characters after this.
				if (i_out < size_out)
				{
					text_out[i_out] = *in;
					++in;
					++i_out;
					++wrappable_out;
				}
			}
			else
			{
				text_out[i_out++] = '\n';
			}

			current_line_width = 0;
			++linecount;
		}
		else
		{
			text_out[i_out++] = c;
			++in;
		}

		c = *in;
	}

	assert(i_out < size_out); // b42
	text_out[min(i_out, (size_out - 1))] = 0; // null terminate
	return linecount;
}

// Returns the offset of the popup
float SCR_DrawPopup (int popup_index, float offset)
{
	float alpha = scr_popup_time_left[popup_index] / POPUP_FADE_TIME;
	int linecount = 1;

	if (alpha > 1.0f)
		alpha = 1.0f;

	if (alpha > 0.0f)
	{
		float xpos = viddef.width - (POPUP_WIDTH + POPUP_PADDING_X) * hudscale;
		float ypos = POPUP_POS_Y * hudscale + offset;
		char wordwrappedtext[1024];

		linecount = SCR_WordWrapText(scr_popup_text[popup_index], (POPUP_WIDTH - POPUP_PADDING_X * 2.0f) * hudscale, wordwrappedtext, sizeof(wordwrappedtext));
		re.DrawBorderedPic(&bpdata_popup1, xpos, ypos, POPUP_WIDTH * hudscale, (POPUP_PADDING_Y * 2.0f + linecount * CHARHEIGHT) * hudscale, hudscale / 2.0f, alpha);
		re.DrawStringAlpha(xpos + POPUP_PADDING_X * hudscale, ypos + POPUP_PADDING_Y * hudscale, wordwrappedtext, alpha);

		return hudscale * (CHARHEIGHT * linecount + POPUP_PADDING_Y * 2.0f + POPUP_SPACING_Y) * alpha;
	}

	return 0.0f;
}


void SCR_CheckDrawPopups (void) // jitpopup
{
	int i;
	float offset = 0.0f;

	for (i = 0; i < MAX_POPUPS; ++i)
	{
		if (scr_popup_time_left[i] > 0.0f)
		{
			scr_popup_time_left[i] -= cl.frametime;
			offset += SCR_DrawPopup(i, offset);
		}
	}
}


void SCR_PrintPopup (const char *str, qboolean behindmenu)
{
	int min_index = 0;
	float min_time = 9999.9f;
	int i;
	qboolean do_shift = true;

	scr_popup_behindmenu = behindmenu; // a bit of a hack, but for the tutorial, we want the popups to appear behind the menu so they don't get in the way.  Normally we want them in front (I think...)

	// Shift existing ones up if ones above have timed out
	while (do_shift) // crappy bubble sort type thing, but it doesn't need to be fancy
	{
		do_shift = false;

		for (i = 0; i < MAX_POPUPS; ++i)
		{
			// Shift existing popups up as the one above may have faded out.
			if (scr_popup_time_left[i] <= 0.0f && i + 1 < MAX_POPUPS && scr_popup_time_left[i + 1] > 0.0f)
			{
				scr_popup_time_left[i] = scr_popup_time_left[i + 1];
				Q_strncpyz(scr_popup_text[i], scr_popup_text[i + 1], sizeof(scr_popup_text[i]));

				scr_popup_time_left[i + 1] = 0.0f;
				scr_popup_text[i + 1][0] = 0;
				do_shift = true;
			}
		}
	}

	for (i = 0; i < MAX_POPUPS; ++i)
	{
		// If we get the same string multiple times, just reset the timer on it so it continues to display.
		if (Q_streq(str, scr_popup_text[i]))
		{
			scr_popup_time_left[i] = POPUP_DISPLAY_TIME + POPUP_FADE_TIME;
			return;
		}

		if (scr_popup_time_left[i] < min_time)
		{
			min_index = i;
			min_time = scr_popup_time_left[i];
		}
	}

	Q_strncpyz(scr_popup_text[min_index], str, sizeof(scr_popup_text[min_index]));
	scr_popup_time_left[min_index] = POPUP_DISPLAY_TIME + POPUP_FADE_TIME; // Display for 3 seconds for now.
}

//=============================================================================

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void SCR_CalcVrect (void)
{
#if 0 // jit - we really probably don't need this anymore.
	int		size;

	// bound viewsize
	if (scr_viewsize->value < 40)
		Cvar_Set ("viewsize","40");
	if (scr_viewsize->value > 100)
		Cvar_Set ("viewsize","100");

	size = scr_viewsize->value;

	scr_vrect.width = viddef.width*size/100;
	scr_vrect.width &= ~7;

	scr_vrect.height = viddef.height*size/100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (viddef.width - scr_vrect.width)*0.5;
	scr_vrect.y = (viddef.height - scr_vrect.height)*0.5;
#else
	scr_vrect.width = viddef.width;
	scr_vrect.width &= ~7; // not sure what this does
	scr_vrect.height = viddef.height;
	scr_vrect.height &= ~1; // or this
	scr_vrect.x = scr_vrect.y = 0;
#endif
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value+10);
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value-10);
}

/*
=================
SCR_Sky_f

Set a specific sky and rotation speed
=================
*/
void SCR_Sky_f (void)
{
	float	rotate;
	vec3_t	axis;

	if (Cmd_Argc() < 2)
	{
		Com_Printf ("Usage: sky <basename> <rotate> <axis x y z>\n");
		return;
	}
	if (Cmd_Argc() > 2)
		rotate = atof(Cmd_Argv(2));
	else
		rotate = 0;
	if (Cmd_Argc() == 6)
	{
		axis[0] = atof(Cmd_Argv(3));
		axis[1] = atof(Cmd_Argv(4));
		axis[2] = atof(Cmd_Argv(5));
	}
	else
	{
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;
	}

	re.SetSky (Cmd_Argv(1), rotate, axis);
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
	scr_conspeed = Cvar_Get("scr_conspeed", "3", 0);
	scr_showturtle = Cvar_Get("scr_showturtle", "0", 0);
	scr_showpause = Cvar_Get("scr_showpause", "1", 0);
	scr_centertime = Cvar_Get("scr_centertime", "2.5", 0);
	scr_printspeed = Cvar_Get("scr_printspeed", "8", 0);
	scr_netgraph = Cvar_Get("netgraph", "0", 0);
	scr_timegraph = Cvar_Get("timegraph", "0", 0);
	scr_debuggraph = Cvar_Get("debuggraph", "0", 0);
	scr_graphheight = Cvar_Get("graphheight", "32", 0);
	scr_graphscale = Cvar_Get("graphscale", "1", 0);
	scr_graphshift = Cvar_Get("graphshift", "0", 0);
	scr_drawall = Cvar_Get("scr_drawall", "0", 0);

	// register our commands
	Cmd_AddCommand("timerefresh",SCR_TimeRefresh_f);
	Cmd_AddCommand("loading",SCR_Loading_f);
	Cmd_AddCommand("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand("sizedown",SCR_SizeDown_f);
	Cmd_AddCommand("sky",SCR_Sky_f);

	scr_initialized = true;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged 
		< CMD_BACKUP-1)
		return;

	re.DrawPic2 (scr_vrect.x+64*hudscale, scr_vrect.y, i_net);
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void)
{
	if (!scr_showpause->value)		// turn off for screenshots
		return;

	if (!cl_paused->value)
		return;

	re.DrawPic2((viddef.width - i_pause->width * hudscale) * 0.5f, viddef.height * 0.5f + 8.0f, i_pause);
}

/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	if (!scr_draw_loading)
		return;

	scr_draw_loading = false;
	//re.DrawPic2((viddef.width - i_loading->width * hudscale) * 0.5f, (viddef.height - i_loading->height * hudscale) * 0.5f, i_loading);
	Cbuf_ExecuteText(EXEC_NOW, "menu loading\n");
	M_Draw();
}

//=============================================================================

/*
==================
SCR_RunConsole

Scroll it up or down
==================
*/
void SCR_RunConsole (void)
{
	// decide on the height of the console
	if (cl_consoleheight->value > 1.0)			// not more than 1 (= 100% of the screen)
		Cvar_Set("cl_consoleheight", "1.0");
	if (cl_consoleheight->value < 0.5)			// not less than 0.5 (= 50% of the screen)
		Cvar_Set("cl_consoleheight", "0.5");
	consoleheight = cl_consoleheight->value;

	if (cls.key_dest == key_console)
		scr_conlines = consoleheight;		// half screen --> T3RR0R15T: setting cl_consoleheight, default 0.5
	else
		scr_conlines = 0.0f;		// none visible
	
	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed->value * cl.frametime; // jitnetfps

		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;
	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed->value * cl.frametime; // jitnetfps

		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}
}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	Con_CheckResize();
	
	if (cls.key_dest != key_menu) // jitmenu
	{
		if (cls.state == ca_disconnected || cls.state == ca_connecting)
		{	// forced full screen console
			Con_DrawConsole(1.0);
			return;
		}

		if (cls.state != ca_active || !cl.refresh_prepped)
		{	// connected, but can't render
			if (cl_consoleheight->value > 1.0)			// not more than 1 (= 100% of the screen)
				Cvar_Set("cl_consoleheight", "1.0");
			if (cl_consoleheight->value < 0.5)			// not less than 0.5 (= 50% of the screen)
				Cvar_Set("cl_consoleheight", "0.5");
			consoleheight = cl_consoleheight->value;
			Con_DrawConsole(consoleheight); // T3RR0R15T: was 0.5
			re.DrawFill(0, viddef.height*consoleheight, viddef.width, viddef.height*consoleheight, 0);
			return;
		}
	}

	if (scr_con_current)
	{
		Con_DrawConsole(scr_con_current);
	}
	else
	{
		if (cls.key_dest == key_game || cls.key_dest == key_message || cls.key_dest == key_menu) // jitmenu
			Con_DrawNotify();	// only draw notify in game
	}
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque (const char *mapname)
{
	char translated_text[1024];

	S_StopAllSounds();
	cl.sound_prepped = false;		// don't play ambients
	CDAudio_Stop();

	if (mapname)
		Cvar_Set("menu_mapname", mapname);
	else
		Cvar_Set("menu_mapname", "");

	if (cls.disable_screen || cls.loading_screen)
		return;

	if (developer->value)
		return;

	//if (cls.state == ca_disconnected)
	//	return;	// if at console, don't bring up the plaque

	if (cls.key_dest == key_console)
		return;

	if (cl.cinematictime > 0)
		scr_draw_loading = 2;	// clear to black first
	else
		scr_draw_loading = 1;

	SCR_UpdateScreen();
	cls.disable_screen = Sys_Milliseconds();
	cls.loading_screen = true;
	cls.disable_servercount = cl.servercount;

	translate_string(translated_text, sizeof(translated_text), "Loading...");
	Cvar_ForceSet("cs_loadingstatus", translated_text);
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void)
{
	if (cls.key_dest == key_console)
		return;
	cls.disable_screen = 0;
	cls.loading_screen = false;
	Con_ClearNotify();
	M_PopMenu("loading");
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void)
{
	SCR_BeginLoadingPlaque(NULL);
}

/*
================
SCR_TimeRefresh_f
================
*/
int entitycmpfnc (const entity_t *a, const entity_t *b)
{
	// all other models are sorted by model then skin
	if (a->model == b->model)
	{
		 // todo - for multi-skin meshes, this probably isn't going to buy us anything...
		return ((int)a->skins[0] - (int)b->skins[0]);
	}
	else
	{
		return ((int)a->model - (int)b->model);
	}
}

void SCR_TimeRefresh_f (void)
{
	int		i;
	int		start, stop;
	float	time;

	if (cls.state != ca_active)
		return;

	start = Sys_Milliseconds();

	if (Cmd_Argc() == 3) // jitest
	{
		re.BeginFrame(0);

		for (i = 0; i < 128; i++)
		{
			cl.refdef.viewangles[1] = (float)i / 128.0f * 360.0f;
			re.RenderFrame(&cl.refdef);
			re.EndFrame();
		}

		re.EndFrame();
	}
	else if (Cmd_Argc() == 2)
	{	// run without page flipping
		re.BeginFrame(0);

		for (i = 0; i < 128; i++)
		{
			cl.refdef.viewangles[1] = (float)i / 128.0f * 360.0f;
			re.RenderFrame(&cl.refdef);
		}

		re.EndFrame();
	}
	else
	{
		for (i = 0; i < 128; i++)
		{
			cl.refdef.viewangles[1] = (float)i / 128.0f * 360.0f;
			re.BeginFrame(0);
			re.RenderFrame(&cl.refdef);
			re.EndFrame();
		}
	}

	stop = Sys_Milliseconds();
	time = (float)(stop - start) / 1000.0f;
	Com_Printf("%f seconds (%f fps)\n", time, 128.0f / time);
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint (int x, int y)
{
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}

void SCR_DirtyScreen (void)
{
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width-1, viddef.height-1);
}

/*
==============
SCR_TileClear

Clear any parts of the tiled background that were drawn on last frame
==============
*/
void SCR_TileClear (void)
{
	int		i;
	int		top, bottom, left, right;
	dirty_t	clear;

	if (scr_drawall->value)
		SCR_DirtyScreen ();	// for power vr or broken page flippers...

	if (scr_con_current == 1.0)
		return;		// full screen console
	if (scr_viewsize->value == 100)
		return;		// full screen rendering
	if (cl.cinematictime > 0)
		return;		// full screen cinematic

	// erase rect will be the union of the past three frames
	// so tripple buffering works properly
	clear = scr_dirty;
	for (i=0 ; i<2 ; i++)
	{
		if (scr_old_dirty[i].x1 < clear.x1)
			clear.x1 = scr_old_dirty[i].x1;
		if (scr_old_dirty[i].x2 > clear.x2)
			clear.x2 = scr_old_dirty[i].x2;
		if (scr_old_dirty[i].y1 < clear.y1)
			clear.y1 = scr_old_dirty[i].y1;
		if (scr_old_dirty[i].y2 > clear.y2)
			clear.y2 = scr_old_dirty[i].y2;
	}

	scr_old_dirty[1] = scr_old_dirty[0];
	scr_old_dirty[0] = scr_dirty;

	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;

	// don't bother with anything convered by the console)
	top = scr_con_current*viddef.height;
	if (top >= clear.y1)
		clear.y1 = top;

	if (clear.y2 <= clear.y1)
		return;		// nothing disturbed

	top = scr_vrect.y;
	bottom = top + scr_vrect.height-1;
	left = scr_vrect.x;
	right = left + scr_vrect.width-1;

	if (clear.y1 < top)
	{	// clear above view screen
		i = clear.y2 < top-1 ? clear.y2 : top-1;
		re.DrawTileClear2 (clear.x1 , clear.y1,
			clear.x2 - clear.x1 + 1, i - clear.y1+1, i_backtile);
		clear.y1 = top;
	}
	if (clear.y2 > bottom)
	{	// clear below view screen
		i = clear.y1 > bottom+1 ? clear.y1 : bottom+1;
		re.DrawTileClear2 (clear.x1, i,
			clear.x2-clear.x1+1, clear.y2-i+1, i_backtile);
		clear.y2 = bottom;
	}
	if (clear.x1 < left)
	{	// clear left of view screen
		i = clear.x2 < left-1 ? clear.x2 : left-1;
		re.DrawTileClear2 (clear.x1, clear.y1,
			i-clear.x1+1, clear.y2 - clear.y1 + 1, i_backtile);
		clear.x1 = left;
	}
	if (clear.x2 > right)
	{	// clear left of view screen
		i = clear.x1 > right+1 ? clear.x1 : right+1;
		re.DrawTileClear2 (i, clear.y1,
			clear.x2-i+1, clear.y2 - clear.y1 + 1, i_backtile);
		clear.x2 = right;
	}
}


//===============================================================


#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[2][11] = 
{
#ifdef QUAKE2
	{"num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
	"num_6", "num_7", "num_8", "num_9", "num_minus"},
#else
	{"anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5", // jit -- used the wrong files, these were just "num" too lazy to fix the images :P
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"},
#endif
	{"anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"}
};

struct image_s *sb_nums_img[2][11]; // jitopt

#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16
#define	ICON_SPACE	8



/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void SizeHUDString (char *string, int *w, int *h)
{
	int		lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string)
	{
		if (*string == '\n')
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8;
	*h = lines * 8;
}

void DrawHUDString (int x, int y, int centerwidth, int xor, unsigned char *string, ...)
{
	int		margin;
	char	line[1024];
	int		width;
	va_list	argptr;
	char	msg[2048], *strp = msg;
	int		formatwidth;

	va_start(argptr, string);
	_vsnprintf(msg, sizeof(msg), string, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(msg); // jitsecurity -- make sure string is null terminated.

	margin = x;

	while (*strp)
	{
		// scan out one line of text from the string
		width = 0;
		formatwidth = 0;

		while (*strp && *strp != '\n')
		{
			if (*strp == SCHAR_COLOR) // jittext
				formatwidth += 2;
			else if (*strp == SCHAR_UNDERLINE || *strp == SCHAR_ITALICS)
				formatwidth ++;
			
			line[width++] = *strp++;
		}

		line[width] = 0;

		if (centerwidth)
			x = margin + (centerwidth*hudscale - strlen_noformat(line)*8*hudscale)*0.5;
		else
			x = margin;

		re.DrawString(x, y, line); // jittext

		if (*strp)
		{
			strp++;	// skip the \n
			x = margin;
			y += 8*hudscale;
		}
	}
}


/*
==============
SCR_DrawField
==============
*/
void SCR_DrawField (int x, int y, int color, int width, int value)
{
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint(x, y);
	SCR_AddDirtyPoint(x + width * CHAR_WIDTH * hudscale + 2, y + 23 * hudscale);

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);

	if (l > width)
		l = width;

	x += 2 + CHAR_WIDTH * (width - l) * hudscale;

	ptr = num;

	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		//re.DrawPic(x, y, sb_nums[color][frame]);
		re.DrawPic2(x, y, sb_nums_img[color][frame]);
		x += CHAR_WIDTH * hudscale;
		ptr++;
		l--;
	}
}


/*
===============
SCR_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/
void SCR_TouchPics (void)
{
	int		i, j;

	for (i = 0; i < 2; ++i)
		for (j = 0; j < 11; ++j)
			sb_nums_img[i][j] = re.RegisterPic(sb_nums[i][j]); // jitopt - cache the img pointer so we don't have to look it up by string later

	if (crosshair->value)
	{
		int crosshair_width_i, crosshair_height_i; // jithudscale / jitcrosshair - handle these as floating point later so we can use non-integer scaling.
		Com_sprintf(crosshair_pic, sizeof(crosshair_pic), "ch%i", (int)(crosshair->value));
		re.DrawGetPicSize(&crosshair_width_i, &crosshair_height_i, crosshair_pic);

		if (!crosshair_width_i)
			crosshair_pic[0] = 0;

		crosshair_width = (float)crosshair_width_i; // jithudscale
		crosshair_height = (float)crosshair_height_i;
	}
}

/*
================
SCR_ExecuteLayoutString 

================
*/
void SCR_ExecuteLayoutString (char *s) // jit: optimized somewhat
{
	int		x, y;
	int		value;
	char	*token;
	int		width;
	int		index;
	clientinfo_t	*ci;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	x = 0;
	y = 0;
	width = 3;

	while (s)
	{
		token = COM_Parse (&s);

		if (token[0]=='a')
		{
			//if (Q_streq(token, "anum"))
			{	// ammo number
				int		color;

				width = 3;
				value = cl.frame.playerstate.stats[STAT_AMMO];

				if (value > 5)
					color = 0;	// green
				else if (value >= 0)
					color = (cl.frame.serverframe>>2) & 1;		// flash
				else
					continue;	// negative number = don't show

				if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
					re.DrawPic (x, y, "field_3");

				SCR_DrawField (x, y, color, width, value);
				continue;
			}
		}
		
		if (token[0]=='c')
		{
			//if (Q_streq(token, "client"))
			if (token[1]=='l')
			{	// draw a deathmatch client block
				int		score, ping, time;

				token = COM_Parse (&s);
				x = viddef.width*0.5 - 160 + atoi(token);
				token = COM_Parse (&s);
				y = viddef.height*0.5 - 120 + atoi(token);
				SCR_AddDirtyPoint (x, y);
				SCR_AddDirtyPoint (x+159, y+31);

				token = COM_Parse (&s);
				value = atoi(token);
				if (value >= MAX_CLIENTS || value < 0)
					Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
				ci = &cl.clientinfo[value];

				token = COM_Parse (&s);
				score = atoi(token);

				token = COM_Parse (&s);
				ping = atoi(token);

				token = COM_Parse (&s);
				time = atoi(token);

				DrawAltString (x+32, y, ci->name);
				re.DrawString (x+32, y+8,  "Score: ");
				DrawAltString (x+32+7*8, y+8,  va("%i", score));
				re.DrawString (x+32, y+16, va("Ping:  %i", ping));
				re.DrawString (x+32, y+24, va("Time:  %i", time));

				if (!ci->icon)
					ci = &cl.baseclientinfo;
				re.DrawPic (x, y, ci->iconname);
				continue;
			}
			//if (Q_streq(token, "cstring2"))
			else if (token[7] == '2')
			{
				//char local_s[MAX_TOKEN_CHARS];
				token = COM_Parse(&s);
				//sprintf(local_s, "%c%c%s", CHAR_COLOR, ']', token); // jittext
				DrawHUDString(x, y, 320, 0, "%c%c%s", SCHAR_COLOR, ']', token); // jittext
				//re.DrawString (x, y, local_s); // jittext
				continue;
			}			
			//if (Q_streq(token, "cstring"))
			else
			{
				token = COM_Parse (&s);
				DrawHUDString (x, y, 320, 0,token);
				continue;
			}
		}

		//if (Q_streq(token, "hnum"))
		if (token[0]=='h')
		{	// health number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_HEALTH];
			if (value > 25)
				color = 0;	// green
			else if (value > 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				color = 1;

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
				re.DrawPic (x, y, "field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}

		if (token[0]=='p')
		{
			//if (Q_streq(token, "picn"))
			if (token[3]=='n')
			{	// draw a pic from a name
				token = COM_Parse (&s);
				SCR_AddDirtyPoint (x, y);
				SCR_AddDirtyPoint (x+23*hudscale, y+23*hudscale);
				re.DrawPic (x, y, token);
				continue;
			}

			//if (Q_streq(token, "pic"))
			else
			{	// draw a pic from a stat number
				token = COM_Parse (&s);
				value = cl.frame.playerstate.stats[atoi(token)];
				if (value >= MAX_IMAGES)
					Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
				if (cl.configstrings[CS_IMAGES+value])
				{
					SCR_AddDirtyPoint (x, y);
					SCR_AddDirtyPoint (x+23*hudscale, y+23*hudscale);
					re.DrawPic (x, y, cl.configstrings[CS_IMAGES+value]);
				}
				continue;
			}
		}

		//if (Q_streq(token, "num"))
		if (token[0]=='n')
		{	// draw a number
			token = COM_Parse (&s);
			width = atoi(token);
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			SCR_DrawField(x, y, 0, width, value);
			continue;
		}

		if (token[0]=='s')
		{
			//if (Q_streq(token, "stat_string"))
			if (token[4]=='_')
			{
				token = COM_Parse (&s);
				index = atoi(token);
				if (index < 0 || index >= MAX_CONFIGSTRINGS)
					Com_Error (ERR_DROP, "Bad stat_string index.");
				index = cl.frame.playerstate.stats[index];
				if (index < 0 || index >= MAX_CONFIGSTRINGS)
					Com_Error (ERR_DROP, "Bad stat_string index.");
				re.DrawString (x, y, cl.configstrings[index]);
				continue;
			}
			//if (Q_streq(token, "string2"))
			else if (token[6]=='2')
			{
				char local_s[MAX_TOKEN_CHARS];
				token = COM_Parse (&s);
				sprintf(local_s, "%c%c%s", SCHAR_COLOR, ']', token); // jittext
				//re.DrawString (x, y, token);
				re.DrawString (x, y, local_s); // jittext
				continue;
			}
			//if (Q_streq(token, "string"))
			else
			{
				token = COM_Parse (&s);
				re.DrawString (x, y, token);
				continue;
			}			
		}

		//if (Q_streq(token, "if"))
		if (token[0]=='i')
		{	// draw a number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (!value)
			{	// skip to endif
				while (s && !Q_streq(token, "endif"))
				{
					token = COM_Parse (&s);
				}
			}
			continue;
		}

		if (token[0]=='x')
		{
			if (token[1]=='l')
			{
				token = COM_Parse (&s);
				x = atoi(token)*hudscale; // jithudscale
			}
			else if (token[1]=='r')
			{
				token = COM_Parse (&s);
				x = viddef.width + atoi(token) * hudscale; // jithudscale
			}
			else if (token[1]=='v')
			{
				token = COM_Parse (&s);
				x = viddef.width*0.5 - 160*hudscale + atoi(token)*hudscale; // jithudscale
			}

			//token = COM_Parse (&s);
			continue;
		}

		if (token[0]=='y')
		{
			if (token[1]=='t')
			{
				token = COM_Parse (&s);
				y = atoi(token) * hudscale; // jithudscale
			}
			else if (token[1]=='b')
			{
				token = COM_Parse (&s);
				y = viddef.height + atoi(token) * hudscale; // jithudscale
			}
			else if (token[1]=='v')
			{
				token = COM_Parse (&s);
				y = viddef.height*0.5 - 120*hudscale + atoi(token)*hudscale; // jithudscale
			}

			//token = COM_Parse (&s);
			continue;
		}
		/*
				if (Q_streq(token, "xl"))
		{
			token = COM_Parse (&s);
			x = atoi(token);
			continue;
		}
		if (Q_streq(token, "xr"))
		{
			token = COM_Parse (&s);
			x = viddef.width + atoi(token);
			continue;
		}
		if (Q_streq(token, "xv"))
		{
			token = COM_Parse (&s);
			x = viddef.width*0.5 - 160 + atoi(token);
			continue;
		}

		if (Q_streq(token, "yt"))
		{
			token = COM_Parse (&s);
			y = atoi(token);
			continue;
		}
		if (Q_streq(token, "yb"))
		{
			token = COM_Parse (&s);
			y = viddef.height + atoi(token);
			continue;
		}
		if (Q_streq(token, "yv"))
		{
			token = COM_Parse (&s);
			y = viddef.height*0.5 - 120 + atoi(token);
			continue;
		}
		  if (Q_streq(token, "rnum"))
		{	// armor number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = 0;	// green

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
				re.DrawPic (x, y, "field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}*/
		/*		if (Q_streq(token, "ctf"))
		{	// draw a ctf client block
			int		score, ping;
			char	block[80];

			token = COM_Parse (&s);
			x = viddef.width*0.5 - 160 + atoi(token);
			token = COM_Parse (&s);
			y = viddef.height*0.5 - 120 + atoi(token);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+159, y+31);

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;

			sprintf(block, "%3d %3d %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				DrawAltString (x, y, block);
			else
				re.DrawString (x, y, block);
			continue;
		}*/
	}
}


/*
================
SCR_DrawStats

The status bar is a small layout program that
is based on the stats array
================
*/
void SCR_DrawStats (void)
{
	SCR_DrawHUD();
}


/*
================
SCR_DrawLayout

================
*/
#define	STAT_LAYOUTS		13

void SCR_DrawLayout (void)
{
	if (!cl.frame.playerstate.stats[STAT_LAYOUTS])
		return;
	SCR_ExecuteLayoutString (cl.layout);
}

//=======================================================

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/

extern cvar_t *cl_drawhud; // jithud
extern cvar_t *cl_menuback;

#define FPS_DISPLAY_X_POS (viddef.width - 64 * hudscale)

void SCR_UpdateScreen (void)
{
	int numframes;
	int i;
	float separation[2] = { 0, 0 };
	extern cvar_t *cl_hudscale;
	extern cvar_t *cl_crosshairscale;

	// if the screen is disabled (vid mode changing)
	// do nothing at all
	if (cls.disable_screen && !cls.loading_screen)
	{
		if (Sys_Milliseconds() - cls.disable_screen > 10000) // jit - dropped from 2 mins to 10 seconds
		{
			cls.disable_screen = 0;
			Com_Printf("Loading plaque timed out.\n");
		}

		return;
	}

	if (!scr_initialized || !con.initialized)
		return;				// not initialized yet


	if (cl_hudscale->value < 1.0 || cl_hudscale->value > viddef.width / 320.0f) // jithudscale
		Cvar_SetValue("cl_hudscale", viddef.width / 320.0f); // jithudscale

	if (cl_crosshairscale->value <= 0.0 || cl_crosshairscale->value > viddef.width / 320.0f)
		Cvar_SetValue("cl_crosshairscale", viddef.width / 320.0f);

	hudscale = cl_hudscale->value;

	if (m_invert->modified || m_pitch->modified) // jitmouse
	{
		// if m_invert is true, set m_pitch to a negative value.
		if (m_invert->value)
			Cvar_SetValue("m_pitch", -fabs(m_pitch->value));
		else
			Cvar_SetValue("m_pitch", fabs(m_pitch->value));

		m_invert->modified = false;
		m_pitch->modified = false;
	}

	/*
	** range check cl_camera_separation so we don't inadvertently fry someone's
	** brain
	*/

	if (cl_stereo_separation->value > 1.0)
		Cvar_SetValue("cl_stereo_separation", 1.0);
	else if (cl_stereo_separation->value < 0)
		Cvar_SetValue("cl_stereo_separation", 0.0);

	if (cl_stereo->value)
	{
		numframes = 2;
		separation[0] = -cl_stereo_separation->value / 2;
		separation[1] =  cl_stereo_separation->value / 2;
	}		
	else
	{
		separation[0] = 0;
		separation[1] = 0;
		numframes = 1;
	}

	if (cl_hudscale->modified) // jithudscale / jitmenu
	{
		M_RefreshMenu();
		cl_hudscale->modified = false;
	}

	if (cl_menuback->modified)
	{
		M_CreateTemporaryBackground();
		M_ReloadMenu();
		cl_menuback->modified = false;
	}

	for (i = 0; i < numframes; i++)
	{
		re.BeginFrame(separation[i]);

		if (scr_draw_loading == 2)
		{	
			//  loading plaque over black screen
			re.CinematicSetPalette(NULL);
			//scr_draw_loading = false;
			//Cbuf_ExecuteText(EXEC_NOW, "menu loading\n");
			//M_Draw();
			//re.DrawPic2((viddef.width - i_loading->width) * 0.5f, (viddef.height - i_loading->height) * 0.5f, i_loading);
//			re.EndFrame();
//			return;
		} 
		// if a cinematic is supposed to be running, handle menus
		// and console specially
		else if (cl.cinematictime > 0)
		{
			if (cls.key_dest == key_menu)
			{
				if (cl.cinematicpalette_active)
				{
					re.CinematicSetPalette(NULL);
					cl.cinematicpalette_active = false;
				}
				M_Draw();
//				re.EndFrame();
//				return;
			}
			else if (cls.key_dest == key_console)
			{
				if (cl.cinematicpalette_active)
				{
					re.CinematicSetPalette(NULL);
					cl.cinematicpalette_active = false;
				}
				SCR_DrawConsole();
//				re.EndFrame();
//				return;
			}
			else
			{
				SCR_DrawCinematic();
//				re.EndFrame();
//				return;
			}
		}
		else 
		{

			// make sure the game palette is active
			if (cl.cinematicpalette_active)
			{
				re.CinematicSetPalette(NULL);
				cl.cinematicpalette_active = false;
			}

			// do 3D refresh drawing, and then update the screen
			SCR_CalcVrect();

			// clear any dirty part of the background
			SCR_TileClear();

			V_RenderView(separation[i]);

			if (cl_drawhud->value)
			{
				// Draw FPS display - MrG - jit, modified
				if (cl_drawfps->value)
				{
					static char s[15];
					static int framecount = 0;
					static int lasttime = 0;

					if ((int)cl_drawfps->value == 1) // average framerate over a few frames
					{
						if (!(framecount & 0xF)) // once every 16 frames
						{
							register float t;
							
							t = curtime - lasttime;

							if (t > 160000.0f)
								t = 160.0f;
							else
								t /= 1000.0f;

							if (t > 0.0f)
								Com_sprintf(s, sizeof(s), "%4.0ffps", framecount / t);
							else
								Q_strncpyz(s, " ???fps", sizeof(s));

							lasttime = curtime;
							framecount = 0;
						}
					}
					else if ((int)cl_drawfps->value == 2) // per-frame display, not averaged
					{
						register float framerate;

						framerate = 1.0f / cls.frametime;
						Com_sprintf(s, sizeof(s), "%4.0ffps", framerate);
					}
					else // Display the lowest framerate every .5 seconds
					{
						static float minframerate = 999999.9f;
						register float framerate;

						framerate = 1.0f / cls.frametime;

						if (curtime - lasttime > 500) // 500ms = .5s
						{
							lasttime = curtime;
							minframerate = framerate;
						}

						if (framerate < minframerate)
							minframerate = framerate;

						Com_sprintf(s, sizeof(s), "%4.0ffps", minframerate);
					}

					framecount++;
					re.DrawString(FPS_DISPLAY_X_POS, 64 * hudscale, s);
				}

				if (cl_drawclock->value) // viciouz - real time clock
				{
					time_t rawtime;
					struct tm * timeinfo;
					char buffer [80];

					time ( &rawtime );
					timeinfo = localtime ( &rawtime );

					strftime(buffer, 80, "%H:%M:%S", timeinfo);

					if(cl_drawclockx->value == -1 && cl_drawclocky->value == -1)
					{
						re.DrawString(viddef.width/2 - 64,1 * hudscale, buffer);			// clock: default position
					}
					else
					{
						re.DrawString(cl_drawclockx->value,cl_drawclocky->value, buffer);	// clock: own position
					}
				}

				if (cl_maptime->value) // T3RR0R15T: elapsed maptime (from AprQ2)
				{
					char	temp[32];
					int		time, hour, mins, secs;
					
					time = cl.time / 1000;
					hour = time/3600;
					mins = (time%3600) /60;
					secs = time%60;
					
					if (hour > 0)
					{
						if(cl_maptime->value == 2)
							Com_sprintf(temp, sizeof(temp), "%i:%02i:%02i h", hour, mins, secs);
						else
							Com_sprintf(temp, sizeof(temp), "  %i:%02i:%02i", hour, mins, secs);
					}
					else
					{
						if (mins > 0)
						{
							if (cl_maptime->value == 2)
								Com_sprintf(temp, sizeof(temp), " %i:%02i min", mins, secs);
							else
								Com_sprintf(temp, sizeof(temp), "     %i:%02i", mins, secs);
						}
						else
						{
							if (cl_maptime->value == 2)
								Com_sprintf(temp, sizeof(temp), "   %02i sec", secs);
							else
								Com_sprintf(temp, sizeof(temp), "       %02i", secs);
						}
					}
				
					if (cl_maptimex->value == -1 && cl_maptimey->value == -1)
					{
						re.DrawString(viddef.width/2 - 64,9 * hudscale, temp);
					}
					else
					{
						re.DrawString(cl_maptimex->value,cl_maptimey->value, temp);
					}
				}
				
				if (cl_drawping->value) // T3RR0R15T: display ping on HUD
				{
					int  ping = 0;
					int  ping2 = 0;
					int  in;
					static int ping_min = 999;
					static int ping_sum = 0;
					static int ping_count = 0;
					static int lasttime = 0;
					static char drawping[16];

					if (curtime - lasttime > 500 && ping_count != 0) // 500ms = .5s
					{
						lasttime = curtime;

						if(cl_drawping->value == 2)
						{
							ping2 = ping_sum / ping_count;		// average
						}
						else
						{
							ping2 = ping_min;					// lowest
						}
						
						if (ping2 > 0) // don't show it on game start
						{
							Com_sprintf(drawping, sizeof(drawping), "%4dping", ping2);
						}

						ping_min = 999;
						ping_sum = 0;
						ping_count = 0;
					}
					else
					{
						in = cls.netchan.incoming_acknowledged & (CMD_BACKUP-1);
						ping = cls.realtime - cl.cmd_time[in];
						
						if (ping > 999)
							ping = 999;

						if (ping < ping_min)
							ping_min = ping;

						ping_sum += ping;
						ping_count++;
					}
					
					re.DrawString(FPS_DISPLAY_X_POS, 80 * hudscale, drawping);
				}

				if (cl_drawpps->value) // jitnetfps
				{
					extern char pps_string[16];

					re.DrawString(FPS_DISPLAY_X_POS, 72 * hudscale, pps_string);
				}

				if (cl_drawtexinfo->value) // jit
				{
					trace_t tr;
					vec3_t end;
					char texinfo[1024];
					vec3_t start;
					extern float g_viewheight;

					VectorCopy(cl.predicted_origin, start);
					start[2] += g_viewheight;
					VectorMA(start, 8192, cl.v_forward, end);
					tr = CM_BoxTrace(start, end, vec3_origin, vec3_origin, 0, cl_drawtexinfo->value > 1.0f ? MASK_ALL : MASK_SOLID);
					Com_sprintf(texinfo, sizeof(texinfo), "%s %d 0x%x 0x%x", tr.surface->name, tr.surface->value, tr.surface->flags, tr.contents);
					re.DrawString(0, viddef.height - 44 * hudscale, texinfo);
				}

				SCR_DrawStats();

				if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 1)
					SCR_DrawLayout();

				//if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 2)
				//	CL_DrawInventory();

				CL_DrawItemPickups(); // jit
				CL_DrawEventStrings(); // jitevents
				// jitodo -- call client scoreboard display here

				if (Cvar_VariableValue("snazbot"))
				{
					re.DrawString(viddef.width / 2 - 76 * hudscale, viddef.height - 8 * hudscale,
						"SnaZ-BoT v3 Enabled");
				}

				SCR_DrawNet();
				SCR_CheckDrawCenterString();

				if (scr_timegraph->value)
					SCR_DebugGraph(cl.frametime * 1000.0f, 0); // jitnetgraph

				if (scr_debuggraph->value || scr_timegraph->value || scr_netgraph->value)
					SCR_DrawDebugGraph();

				SCR_DrawPause();
			}

			if (scr_popup_behindmenu)
				SCR_CheckDrawPopups(); // jit

			SCR_DrawConsole();
			M_Draw();
			SCR_DrawLoading();

			if (!scr_popup_behindmenu)
				SCR_CheckDrawPopups(); // jit
		}
	}

	re.EndFrame();
}
