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

// ===
// jit
#define MAX_ITEM_STRINGS 8
static char item_strings[MAX_ITEM_STRINGS][MAX_QPATH]; // displayed briefly just above hud
static int item_string_time[MAX_ITEM_STRINGS];
static int startpos = 0;

static void item_print (char *s)
{
	startpos--;

	if (startpos<0)
		startpos = MAX_ITEM_STRINGS - 1;

	strcpy(item_strings[startpos], s);
	item_string_time[startpos] = curtime;
}

void CL_ParsePrintItem (char *s) // jit
{
	char buff[MAX_QPATH];

	if (*s == '+') // item pickup
		Com_sprintf(buff, sizeof(buff), "%s", cl.configstrings[CS_ITEMS + s[1]]);
	else if (*s == '-') // item drop
		Com_sprintf(buff, sizeof(buff), "%c%ca%s", SCHAR_ITALICS, SCHAR_COLOR, cl.configstrings[CS_ITEMS + s[1]]);
	else // unknown? (shouldn't happen)
		Com_sprintf(buff, sizeof(buff), "UNKNOWN %s", cl.configstrings[CS_ITEMS + s[1]]);

	item_print(buff);
}

void CL_DrawItemPickups (void)
{
	int i, j;
	float alpha;

	for (i = 0, j = startpos; i < MAX_ITEM_STRINGS; i++, j++, j %= MAX_ITEM_STRINGS)
	{
		if (*item_strings[j])
		{
			alpha = (4000 - (curtime-item_string_time[j])) / 3000.0f;

			if (alpha > 1.0f)
				alpha = 1.0f;

			if (alpha < 0.05f)
			{
				item_strings[j][0] = '\0';
				break;
			}

			// jitodo -- cvars for positioning.
			re.DrawStringAlpha(0 + 8 * hudscale, viddef.height - (40 + i * 8) * hudscale, item_strings[j], alpha);
		}
		else
		{
			break;
		}
	}
}

// jit
// ===

/*
================
CL_ParseInventory
================
*/
void CL_ParseInventory (void)
{
	int i;

	for (i = 0; i < MAX_ITEMS; i++)
		cl.inventory[i] = MSG_ReadShort(&net_message);
}


