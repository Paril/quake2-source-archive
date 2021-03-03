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
// cl_inv.c -- client inventory screen

#include "client.h"

/*
================
CL_ParseInventory
================
*/
void CL_ParseInventory (void)
{
	int		i;

	for (i=0 ; i<MAX_ITEMS ; i++)
		cl.inventory[i] = MSG_ReadShort (&net_message);
}


/*
================
Inv_DrawString
================
*/
void Hud_DrawString (int x, int y, const char *string, int alpha);
void Inv_DrawString (int x, int y, char *string)
{
	Hud_DrawString(x, y, string, 255);
}

void SetStringHighBit (char *s)
{
	while (*s)
		*s++ |= 128;
}

/*
================
CL_DrawInventory
================
*/
#define	DISPLAY_ITEMS	17

void CL_DrawInventory (void)
{
	int		i, j;
	int		num, selected_num, item;
	int		index[MAX_ITEMS];
	char	string[1024];
	int		x, y;
	char	binding[1024];
	char	*bind;
	int		selected;
	int		top;

	selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];

	num = 0;
	selected_num = 0;
	for (i=0; i<MAX_ITEMS; i++)
	{
		if (i==selected)
			selected_num = num;
		if (cl.inventory[i])
		{
			index[num] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - DISPLAY_ITEMS/2;
	if (num - top < DISPLAY_ITEMS)
		top = num - DISPLAY_ITEMS;
	if (top < 0)
		top = 0;

	x = viddef.width/2 - scaledHud(128);
	y = viddef.height/2 - scaledHud(120);
	//x = (viddef.width-256)/2;
	//y = (viddef.height-240)/2;

	// repaint everything next frame
	SCR_DirtyScreen ();

	R_DrawScaledPic (x, y+scaledHud(8), HudScale(), hud_alpha->value, "inventory");

	y += scaledHud(24);
	x += scaledHud(24);
	Inv_DrawString (x, y, S_COLOR_BOLD"hotkey ### item");
	Inv_DrawString (x, y+scaledHud(8), S_COLOR_BOLD"------ --- ----");
	y += scaledHud(16);
	for (i=top; i<num && i < top+DISPLAY_ITEMS; i++)
	{
		item = index[i];
		// search for a binding

		// Knightmare- BIG UGLY HACK for connected to server using old protocol
		// Changed config strings require different parsing
		if ( LegacyProtocol() )
			Com_sprintf (binding, sizeof(binding), "use %s", cl.configstrings[OLD_CS_ITEMS+item]);
		else
			Com_sprintf (binding, sizeof(binding), "use %s", cl.configstrings[CS_ITEMS+item]);

		bind = "";
		for (j=0; j<256; j++)
			if (keybindings[j] && !Q_stricmp (keybindings[j], binding))
			{
				bind = Key_KeynumToString(j);
				break;
			}

		// Knightmare- BIG UGLY HACK for connected to server using old protocol
		// Changed config strings require different parsing
		if ( LegacyProtocol() )
		{
			if (item != selected)
			{
				Com_sprintf (string, sizeof(string), " "S_COLOR_BOLD S_COLOR_ALT"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[OLD_CS_ITEMS+item] );
			}
			else	// draw a blinky cursor by the selected item
			{
				Com_sprintf (string, sizeof(string), S_COLOR_BOLD">"S_COLOR_ITALIC"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[OLD_CS_ITEMS+item] );
			}
		}
		else
		{
			if (item != selected)
			{
				Com_sprintf (string, sizeof(string), " "S_COLOR_BOLD S_COLOR_ALT"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[CS_ITEMS+item] );
			}
			else	// draw a blinky cursor by the selected item
			{
				Com_sprintf (string, sizeof(string), S_COLOR_BOLD">"S_COLOR_ITALIC"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[CS_ITEMS+item] );
			}
		}

		Inv_DrawString (x, y, string);
		y += scaledHud(8);
	}


}


