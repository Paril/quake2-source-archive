/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"

#define MENU_X 32
#define MENU_Y 32

void Menu_Create(edict_t *ent, int startline, int endline) {
	if(ent->menu)
		Menu_Destroy(ent);

	ent->menu = (menu_t *)malloc(sizeof(menu_t));
	ent->menu->firstline = NULL;
	ent->menu->lastline = NULL;
	ent->menu->sel = NULL;
	ent->menu->startline = startline;
	ent->menu->endline = endline;
	ent->menu->page = 0;
	ent->menu->changed = true;
	ent->menu->editing = false;
	ent->menu->cancel_func = NULL;
	ent->menu->xoff = 15;
	ent->menu->yoff = 16;

	Lithium_LayoutOn(ent, LAYOUT_MENU);
	Lithium_LayoutOff(ent, LAYOUT_SCORES);
}

void Menu_Offsets(edict_t *ent, int xoff, int yoff) {
	ent->menu->xoff = xoff;
	ent->menu->yoff = yoff;
}

void Menu_Title(edict_t *ent, char *title) {
	Menu_AddLine(ent, MENU_TEXT, 1, lithium_modname, "mc");
	Menu_AddLine(ent, MENU_TEXT, 2, title, "m");
}

void Menu_AddLine(edict_t *ent, int type, int line, char *text, void *data) {
	menuline_t *menuline;

	menuline = (menuline_t *)malloc(sizeof(menuline_t));
	menuline->prev = NULL;
	menuline->next = NULL;

	if(!ent->menu->firstline) {
		ent->menu->firstline = menuline;
		ent->menu->lastline = menuline;
	}
	else {
		menuline->prev = ent->menu->lastline;
		ent->menu->lastline->next = menuline;
		ent->menu->lastline = menuline;
	}

	menuline->type = type;
	menuline->line = line;
	menuline->text = text;
	menuline->data = data;
	menuline->selectable = type != MENU_TEXT;
	menuline->textp = false;

	if(!ent->menu->sel && menuline->selectable)
		ent->menu->sel = menuline;
}

void Menu_AddText(edict_t *ent, int line, char *format, ...) {
	va_list argptr;
	char *text;
	char buf[80];

	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);

	text = (char *)strdup(buf);

	Menu_AddLine(ent, MENU_TEXT, line, text, "l");
	ent->menu->lastline->textp = true;
}

void Menu_CancelFunc(edict_t *ent, void (*func)(edict_t *ent)) {
	if(ent->menu)
		ent->menu->cancel_func = func;
}

void Menu_Destroy(edict_t *ent) {
	menuline_t *next, *menuline;
	if(!ent->menu)
		return;

	menuline = ent->menu->firstline;
	while(menuline) {
		next = menuline->next;
		if(menuline->textp)
			free(menuline->text);
		free(menuline);
		menuline = next;
	}
	free(ent->menu);
	ent->menu = NULL;

	Lithium_LayoutOff(ent, LAYOUT_MENU);

	ent->lithium_flags |= LITHIUM_STATUSBAR;

	gi.WriteByte(svc_layout);
	gi.WriteString("");
	gi.unicast(ent, true);
}

#define MARGIN_X 16

char *Menu_GetLine(edict_t *ent, menuline_t *menuline, qboolean sel) {
	static char line[80];
	char right[20] = "";
	char string[40];
	char format[40];
	qboolean colored = false;

	int x = 16;
	menu_t *menu = ent->menu;

	if(menuline->type == MENU_TEXT) {
		if(strchr(menuline->data, 'l'))
			x = 8;
		else if(strchr(menuline->data, 'm'))
			x = ((256 - MARGIN_X * 2) - strlen(menuline->text) * 8) / 2;
		else if (strchr(menuline->data, 'r'))
			x = (256 - MARGIN_X * 2) - strlen(menuline->text) * 8;
	}

	if(menuline->selectable) {
		if(menuline->type == MENU_LVAR || menuline->type == MENU_PVAR) {
			qboolean show = false;
			char *edit = NULL;
			float value = 0;

			if(menuline->type == MENU_LVAR) {
				lvar_t *lvar = (lvar_t *)menuline->data;
				value = lvar->value;
				edit = lvar->edit;
			}
			if(menuline->type == MENU_PVAR) {
				pvar_t *pvar = pvar_find(ent, menuline->data);
				value = (float)*pvar->value;
				edit = pvar->edit;
			}

			if(menu->editing && sel)
				strlcpy(right, menu->edit, sizeof(right));
			else if(!menu->editing)
				show = true;
			else if(menu->editing && !sel && menuline != menu->sel)
				show = true;

			if(strchr(edit, '#') && show) {
				if(strchr(edit, '.')) {
					int before = 0, after = 0;
					char *c = edit;
					while(*c++ != '.')
						before++;
					while(*c++)
						after++;
					snprintf(right, sizeof(right), "%*.*f", before, after, value);
				}
				else
					snprintf(right, sizeof(right), "%*d", (int)strlen(edit), (int)value);
			}
			else if(strchr(edit, ':')) {
				int f = 0, field = (int)value;
				char *c = edit, *d;
				while(c) {
					if(*c == ':')
						c++;
					if(f == field) {
						d = strchr(c, ':');
						if(d)
							strlcpy_s(right, c, sizeof(right), d - c);
						else
							strlcpy(right, c, sizeof(right));
					}
					c = strchr(c, ':');
					f++;
				}
			}
			else if(edit[0] == '^') {
				if(value)
					strlcpy(right, "On", sizeof(right));
				else
					strlcpy(right, "Off", sizeof(right));
			}
		}
	}
	else if(strchr(menuline->data, 'c'))
		colored = true;

	if(strlen(right))
		snprintf(string, sizeof(string), "%-*s%s", 24 - (int)strlen(right), menuline->text, right);
	else
		strlcpy(string, menuline->text, sizeof(string));

	if(sel) {
		strlcpy(format, string, sizeof(format));
		snprintf(string, sizeof(string), "> %-24s <", format);
		x -= 16;
		colored = true;
	}

	snprintf(line, sizeof(line), "xv %d yv %d string%s \"%s\" ", MENU_X + ent->menu->xoff + x + 2, 
		MENU_Y + ent->menu->yoff + menuline->line * 8, colored ? "2" : "", string);

	return line;
}

qboolean Menu_IsLineUsed(menu_t *menu, int num) {
	menuline_t *menuline;
	menuline = menu->firstline;
	while(menuline) {
		if(!menuline->selectable && menuline->line == num)
			return true;
		menuline = menuline->next;
	}
	return false;
}

int Menu_Draw(edict_t *ent) {
	static char string[1400];
	menuline_t *menuline;
	int page = 0, num = 0;

	ent->menu->changed = false;

	menuline = ent->menu->firstline;
	while(menuline) {
		if(menuline->selectable) {
			if(page == ent->menu->page)
				menuline->line = ent->menu->startline + num;
			else
				menuline->line = 0;

			do num++; while(Menu_IsLineUsed(ent->menu, ent->menu->startline + num));

			if(num == ent->menu->endline - ent->menu->startline && menuline->next && menuline->next->next) {
				page++;
				num = 0;
			}
		}
		menuline = menuline->next;
	}

	snprintf(string, sizeof(string), "xv %d yv %d picn inventory ", MENU_X, MENU_Y);

	menuline = ent->menu->firstline;
	while(menuline) {
		if(menuline->line) {
			strlcat(string, Menu_GetLine(ent, menuline, false), sizeof(string));
			if(ent->menu->sel && !ent->menu->sel->line && menuline->selectable)
				ent->menu->sel = menuline;
		}
		menuline = menuline->next;
	}

	if(page > ent->menu->page)
		snprintf(string + strlen(string), sizeof(string)-strlen(string), "xv %d yv %d string ... ", 
			MENU_X + ent->menu->xoff + 16 + 2, MENU_Y + ent->menu->yoff + ent->menu->endline * 8);

	gi.WriteByte(0x0D);
	gi.WriteShort(5);
	gi.WriteString(string);
	gi.unicast(ent, true);

	return strlen(string);
}

int Menu_UpdateLine(edict_t *ent) {
	char *string;

	if(!ent->menu)
		return 0;
	if(ent->menu->changed)
		return 0;

	string = Menu_GetLine(ent, ent->menu->sel, true);

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);

	return strlen(string);
}

int Menu_Update(edict_t *ent) {
	int len = 0;

	if(!ent->menu)
		return 0;
	if(ent->menu->changed)
		len = Menu_Draw(ent);
	if(ent->menu->sel)
		len += Menu_UpdateLine(ent);

	return len;
}

void Menu_Prev(edict_t *ent) {
	if(!ent->menu->sel)
		return;

	Menu_EditEnd(ent);

	if(!ent->menu->sel->prev || !ent->menu->sel->prev->selectable) {
		while(ent->menu->sel->next && ent->menu->sel->next->selectable && ent->menu->sel->next->line)
			ent->menu->sel = ent->menu->sel->next;
		Menu_UpdateLine(ent);
		return;
	}

	ent->menu->sel = ent->menu->sel->prev;
	if(ent->menu->sel->type == MENU_BLANK)
		Menu_Prev(ent);

	if(ent->menu->sel->line == 0) {
		ent->menu->page--;
		ent->menu->changed = true;
		ent->layout_update = true;
	}
	Menu_UpdateLine(ent);
}

void Menu_Next(edict_t *ent) {
	if(!ent->menu->sel)
		return;

	Menu_EditEnd(ent);

	if(!ent->menu->sel->next || !ent->menu->sel->next->selectable) {
		while(ent->menu->sel->prev && ent->menu->sel->prev->selectable && ent->menu->sel->prev->line)
			ent->menu->sel = ent->menu->sel->prev;
		Menu_UpdateLine(ent);
		return;
	}

	ent->menu->sel = ent->menu->sel->next;
	if(ent->menu->sel->type == MENU_BLANK)
		Menu_Next(ent);

	if(ent->menu->sel->line == 0) {
		ent->menu->page++;
		ent->menu->changed = true;
		ent->layout_update = true;
	}
	Menu_UpdateLine(ent);
}

/*
void Menu_PrevPage(edict_t *ent) {
	menuline_t *menuline;
	if(!ent->menu->sel)
		return;

	Menu_EditEnd(ent);

	menuline = ent->menu->sel->prev;
	while(menuline) {
		if(menuline->selectable && menuline->line == 0) {
			ent->menu->sel = menuline;
			ent->menu->page--;
			ent->menu->changed = true;
			ent->layout_update = true;
			return;
		}
		menuline = menuline->prev;
	}
}

void Menu_NextPage(edict_t *ent) {
	menuline_t *menuline;
	if(!ent->menu->sel)
		return;

	Menu_EditEnd(ent);

	menuline = ent->menu->sel->prev;
	while(menuline) {
		if(menuline->selectable && menuline->line == 0) {
			ent->menu->sel = menuline;
			ent->menu->page++;
			ent->menu->changed = true;
			ent->layout_update = true;
			return;
		}
		menuline = menuline->next;
	}
}
*/

int countfields(char *edit) {
	char *c = edit;
	int fields = 0;
	while(c) {
		c = strchr(c + 1, ':');
		fields++;
	}
	return fields;
}

void Menu_Use(edict_t *ent) {
	menuline_t *menuline;
	void (*func)(edict_t *ent);
	lvar_t *lvar;
	pvar_t *pvar;

	if(!ent->inuse)
		return;
	if(!ent->menu->sel)
		return;

	menuline = ent->menu->sel;

	switch(menuline->type) {
		case MENU_LVAR:
			lvar = (lvar_t *)menuline->data;
			if(strchr(lvar->edit, '#')) {
				if(!ent->menu->editing)
					Menu_EditBegin(ent);
				else
					Menu_EditEnd(ent);
			}
			else if(strchr(lvar->edit, ':')) {
				int fields = countfields(lvar->edit);
				char buf[8];
				int x = lvar->value + 1;
				if(x >= fields)
					x = 0;
				snprintf(buf, sizeof(buf), "%d", x);
				gi.cvar_set(lvar->cvar->name, buf);
			}
			else if(lvar->edit[0] == '^') {
				if(lvar->value)
					gi.cvar_set(lvar->cvar->name, "0");
				else
					gi.cvar_set(lvar->cvar->name, "1");
			}
			break;
		case MENU_FUNC:
			func = menuline->data;
			func(ent);
			break;
		case MENU_CMD:
			stuffcmd(ent, menuline->data);
			break;
		case MENU_SVCMD:
			gi.AddCommandString(menuline->data);
			break;
		case MENU_PVAR:
			pvar = pvar_find(ent, menuline->data);
			if(strchr(pvar->edit, '#')) {
				if(!ent->menu->editing)
					Menu_EditBegin(ent);
				else
					Menu_EditEnd(ent);
			}
			else if(strchr(pvar->edit, ':')) {
				int fields = countfields(pvar->edit);
				(*pvar->value)++;
				if(*pvar->value >= fields)
					*pvar->value = 0;
			}
			else if(pvar->edit[0] == '^')
				*pvar->value ^= 1;
			break;
	}
}

void Menu_Key(edict_t *ent, int key) {
	menuline_t *menuline;
	int count;

	if(!ent->menu)
		return;

	if(ent->menu->editing) {
		char *c;
		c = strchr(ent->menu->edit, '_');
		if(c)
			*c = '0' + key;
		c = strchr(ent->menu->edit, ' ');
		if(c)
			*c = '_';
		else
			Menu_EditEnd(ent);
		Menu_UpdateLine(ent);
		return;
	}

	if(key == 0) {
		Menu_Destroy(ent);
		return;
	}

	// make selection number based on key
	menuline = ent->menu->firstline;
	count = 1;
	while(menuline) {
		if(menuline->selectable && menuline->type != MENU_BLANK && count++ == key) {
			ent->menu->sel = menuline;
			Menu_UpdateLine(ent);
			Menu_Use(ent);
			return;
		}
		menuline = menuline->next;
	}
}

void Menu_EditBegin(edict_t *ent) {
	char *c;

	if(ent->menu->lastedit == ent->menu->sel && level.time < ent->menu->lastedit_time + 1.0)
		return;

	if(ent->menu->sel->type == MENU_LVAR) {
		lvar_t *lvar = (lvar_t *)ent->menu->sel->data;
		strlcpy(ent->menu->edit, lvar->edit, sizeof(ent->menu->edit));
	}
	else if(ent->menu->sel->type == MENU_PVAR) {
		pvar_t *pvar = (pvar_t *)pvar_find(ent, ent->menu->sel->data);
		strlcpy(ent->menu->edit, pvar->edit, sizeof(ent->menu->edit));
	}
	else
		return;

	ent->menu->editing = true;
	c = strchr(ent->menu->edit, '#');
	*c = '_';
	c = ent->menu->edit;
	while(*c) {
		if(*c == '#')
			*c = ' ';
		c++;
	}
	ent->menu->changed = true;
	ent->layout_update = true;
}

void Menu_EditEnd(edict_t *ent) {
	char *c;
	int val;
	if(!ent->menu->editing)
		return;
	ent->menu->editing = false;
	if((ent->menu->edit[0] >= '0' && ent->menu->edit[0] <= '9') ||
		(ent->menu->edit[1] >= '0' && ent->menu->edit[1] <= '9')) {
		c = strchr(ent->menu->edit, '_');
		if(c)
			*c = 0;

		if(ent->menu->sel->type == MENU_LVAR) {
			lvar_t *lvar = (lvar_t *)ent->menu->sel->data;
			val = lvar->value;
			gi.cvar_set(lvar->cvar->name, ent->menu->edit);
		}
		else if(ent->menu->sel->type == MENU_PVAR) {
			pvar_t *pvar = (pvar_t *)pvar_find(ent, ent->menu->sel->data);
			val = *pvar->value;
			*pvar->value = atoi(ent->menu->edit);
		}
		else
			return;

		ent->menu->lastedit = ent->menu->sel;
		ent->menu->lastedit_time = level.time;

		if(val != atoi(ent->menu->edit))
			return;
	}

	ent->menu->changed = true;
	ent->layout_update = true;
}

void Menu_Cancel(edict_t *ent) {
	if(!ent->menu)
		return;
	if(ent->menu->editing)
		return;
	if(ent->menu->page) {
		ent->menu->page--;
		ent->menu->changed = true;
		ent->layout_update = true;
	}
	else if(ent->menu->cancel_func)
		ent->menu->cancel_func(ent);
	else
		Menu_Destroy(ent);
}

void Menu_LVarChanged(lvar_t *lvar) {
	edict_t *ent;
	menuline_t *menuline;
	int i;
	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse || !ent->menu || !ent->menu->firstline)
			continue;
		menuline = ent->menu->firstline;
		while(menuline) {
			if(menuline->type == MENU_LVAR && menuline->data == lvar && menuline->line) {
				ent->menu->changed = true;
				ent->layout_update = true;
				return;
			}
			menuline = menuline->next;
		}
	}
}


void Menu_ClientFrame(edict_t *ent) {
	menuline_t *menuline;
	pvar_t *pvar;

	if(level.intermissiontime)
		return;

	if(!ent->menu || !ent->menu->firstline)
		return;

	pvar = ent->pvar;
	while(pvar) {
		if(*pvar->value != pvar->old) {
			pvar->old = *pvar->value;

			menuline = ent->menu->firstline;
			while(menuline) {
				if(menuline->type == MENU_PVAR && !strcmp(menuline->data, pvar->name) && menuline->line) {
					ent->menu->changed = true;
					ent->layout_update = true;
					return;
				}
				menuline = menuline->next;
			}
		}

		pvar = pvar->next;
	}

}

qboolean Menu_ClientCommand(edict_t *ent) {
	char *cmd = gi.argv(0);

	if(!ent->menu)
		return false;

	if(Q_stricmp (cmd, "invuse") == 0)
		Menu_Use(ent);
/*
	else if(Q_stricmp (cmd, "weapprev") == 0)
		Menu_PrevPage(ent);
	else if(Q_stricmp (cmd, "weapnext") == 0)
		Menu_NextPage(ent);
*/
	else if(Q_stricmp (cmd, "invdrop") == 0)
		Menu_Cancel(ent);
	else if(Q_stricmp(cmd, "invnext") == 0)
		Menu_Next(ent);
	else if (Q_stricmp(cmd, "invprev") == 0)
		Menu_Prev(ent);
	else
		return false;

	return true;
}

