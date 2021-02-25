/*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* 
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

// Contributed by QwazyWabbit.

#include "g_local.h"

void PMenu_UpdateEntry(pmenu_t *entry, const char *text, int align, SelectFunc_t SelectFunc);

/* Development menu */
// This is the maximum size the selection menu can take.
// 18 lines by 28 characters. Align left and right lose 2 chars for margins.
// Note: line index 
pmenu_t testmenu[] = {
	{ "*StroggDM",	PMENU_ALIGN_CENTER, NULL },
	{ "                            ",	PMENU_ALIGN_CENTER, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL }, 
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_LEFT, NULL },
	{ "                            ",	PMENU_ALIGN_CENTER, NULL }
};

// Development menu command
// Prototype in g_cmds.c:
// void Cmd_DevMenu_f(char *cmd, edict_t *ent);


// Interface in g_local.h gclient_s:
// 	pmenu_t		*devmenu;		// this is a pointer to the menu
// to be assigned in PMenu_Open

// Call in ClientCommand: 
//		if (Q_stricmp (cmd, "devmenu") == 0)
//			Cmd_DevMenu_f(cmd, ent);

// Command format:
// devmenu open | close | reset
// devmenu text "string" line_number justification (l|c|r)
// * at the start of the string makes highlighted text.
// Defaults to left justify.
//
void Cmd_DevMenu_f(char *scmd, edict_t *ent)
{
	int i, align;
	char *p, *text;
	pmenuhnd_t *hnd;
	
	// the command is passed in full (devmenu xxx "xxx" l j)
	scmd = gi.argv(1); // ptr to the second level command
	
	if (Q_stricmp (scmd, "open") == 0)
	{
		// testmenu just a template, copy it to the dynamic menu
		hnd = PMenu_Open (ent, testmenu, 0, sizeof(testmenu) / sizeof(pmenu_t), NULL);
	}
	
	else if (Q_stricmp (scmd, "close") == 0)
	{
		PMenu_CloseIt(ent);
	}
	
	else if (Q_stricmp (scmd, "text") == 0)
	{
		if (ent->client->menu)	// menu must be open
		{
			if (gi.argc() == 4 || gi.argc() == 5)
			{
				i = atoi(gi.argv(3));
				if ( 0 > i || i > 17) // range check line number
				{
					gi.cprintf (ent, PRINT_HIGH, "Line number must be between 0 and 17.\n");
					return;
				}
				p = testmenu[i].text;
				if (strlen(gi.argv(2)) < 29) // 28 chars plus '\0' per line
					text = strdup(gi.argv(2)); // copy at most 28 chars to the text location
				else {
					gi.cprintf (ent, PRINT_HIGH, "Number of characters must be less than 29.\n");
					return;
				}
				
				p = gi.argv(4);
				if (*p == 'c')
					align = PMENU_ALIGN_CENTER;
				else if (*p == 'r')
					align = PMENU_ALIGN_RIGHT;
				else
					align = PMENU_ALIGN_LEFT;
				
				hnd = ent->client->menu; // pointer to the dynamic menu
				hnd->entries[i].text = strdup(text);	// dynamically allocated string
				hnd->entries[i].align = align;
			}
			else
				gi.cprintf (ent, PRINT_HIGH, "Number and text must be supplied (ex: devmenu text \"hello\" 3 l|c|r)\n");
		}
		else
			gi.cprintf (ent, PRINT_HIGH, "Menu must be open first.\n");
	}
	
	else if (Q_stricmp (scmd, "reset") == 0)
	{
		if (ent->client->menu)	// menu must be open
		{
			hnd = ent->client->menu;
			for (i=0; i < (sizeof(testmenu) / sizeof(pmenu_t)); i++) 
			{
				hnd->entries[i].text = strdup(""); // blank the line
				hnd->entries[i].align = PMENU_ALIGN_CENTER;
				hnd->entries[i].SelectFunc = NULL;
				
			}
			gi.cprintf (ent, PRINT_HIGH, "Menu reset\n");
		}
	}
}

