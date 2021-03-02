//#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ )
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
#include "g_local.h"

// Note that the pmenu entries are duplicated
// this is so that a static set of pmenu entries can be used
// for multiple clients and changed without interference
// note that arg will be freed when the menu is closed, it must be allocated memory
pmenuhnd_t *PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, void *arg)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (ent->is_bot)
		return NULL;

	if (!ent->client)
		return NULL;

	if (ent->client->menu) {
		gi.dprintf("warning, ent already has a menu\n");
		PMenu_Close(ent);
	}

	hnd = malloc(sizeof(*hnd));

	hnd->arg = arg;
	hnd->entries = malloc(sizeof(pmenu_t) * num);
	memcpy(hnd->entries, entries, sizeof(pmenu_t) * num);
	// duplicate the strings since they may be from static memory
	for (i = 0; i < num; i++)
		if (entries[i].text)
			hnd->entries[i].text = strdup(entries[i].text);

	hnd->num = num;

	if (cur < 0 || !entries[cur].SelectFunc) {
		for (i = 0, p = entries; i < num; i++, p++)
			if (p->SelectFunc)
				break;
	} else
		i = cur;

	if (i >= num)
		hnd->cur = -1;
	else
		hnd->cur = i;

	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->menu = hnd;

	PMenu_Do_Update(ent);
	gi.unicast (ent, true);

	return hnd;
}

void PMenu_Close(edict_t *ent)
{
	int i;
	pmenuhnd_t *hnd;

	if (!ent->client->menu)
		return;

	hnd = ent->client->menu;
	for (i = 0; i < hnd->num; i++)
		if (hnd->entries[i].text)
			free(hnd->entries[i].text);
	free(hnd->entries);
	if (hnd->arg)
		free(hnd->arg);
	free(hnd);
	ent->client->menu = NULL;
	ent->client->showscores = false;
}

// only use on pmenu's that have been called with PMenu_Open
void PMenu_UpdateEntry(pmenu_t *entry, const char *text, int align, SelectFunc_t SelectFunc)
{
	if (entry->text)
		free(entry->text);
	entry->text = strdup(text);
	entry->align = align;
	entry->SelectFunc = SelectFunc;
}

void PMenu_Do_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	strcpy(string, "xv 32 yv 8 picn inventory ");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
		sprintf(string + strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - strlen(t)*8);
		else
			x = 64;

		sprintf(string + strlen(string), "xv %d ",
			x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		else
			sprintf(string + strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void PMenu_Update(edict_t *ent)
{
	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	if (ent->is_bot)
		return;

	if (level.time - ent->client->menutime >= 1.0) {
		// been a second or more since last update, update now
		PMenu_Do_Update(ent);
		gi.unicast (ent, true);
		ent->client->menutime = level.time;
		ent->client->menudirty = false;
	}
	ent->client->menutime = level.time + 0.2;
	ent->client->menudirty = true;
}

void PMenu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		i++, p++;
		if (i == hnd->num)
			i = 0, p = hnd->entries;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
}

void PMenu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		if (i == 0) {
			i = hnd->num - 1;
			p = hnd->entries + i;
		} else
			i--, p--;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
}

void PMenu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;

	if (p->SelectFunc)
		p->SelectFunc(ent, hnd);
}

void CloseMenu (edict_t *ent, pmenuhnd_t *p);
void FirstCommandMenu(edict_t *ent, pmenuhnd_t *p);
void NextCommandMenu(edict_t *ent, pmenuhnd_t *p);
void HelpMenu(edict_t *ent, pmenuhnd_t *p);
void OpenAliasesMenu(edict_t *ent, pmenuhnd_t *p);

pmenu_t commands2menu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*Insane Weapons Mod Commands Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "**Remote Control Car Commands*",								PMENU_ALIGN_LEFT,  },
	{ "rc_create - Creates the car",						PMENU_ALIGN_LEFT, NULL },
	{ "rc_backward - Moves car backwards",					PMENU_ALIGN_LEFT, NULL },
	{ "rc_forward - Moves car forward",						PMENU_ALIGN_LEFT, NULL },
	{ "rc_stop_forward - Stops forward/backward movement",							PMENU_ALIGN_LEFT, NULL },
	{ "rc_left - Turns car left",								PMENU_ALIGN_LEFT, NULL },
	{ "rc_right - Turns car right",								PMENU_ALIGN_LEFT, NULL },
	{ "rc_stop_side - Stops turning movement",								PMENU_ALIGN_LEFT, NULL },
	{ "rc_boom - Blows up the car",								PMENU_ALIGN_LEFT, NULL },
	{ "rc_weapon - Changes car's weapon",								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*Prev Page",								PMENU_ALIGN_LEFT, FirstCommandMenu },
	{ "*Close Menu",								PMENU_ALIGN_LEFT, CloseMenu },
};

pmenu_t commandsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*Insane Weapons Mod Commands Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "**Main Commands*",								PMENU_ALIGN_LEFT,  },
	{ "flashlight - Turn on your flashlight",						PMENU_ALIGN_LEFT, NULL },
	{ "hook action - Launches hook",					PMENU_ALIGN_LEFT, NULL },
	{ "hook shrink - shorten hook",						PMENU_ALIGN_LEFT, NULL },
	{ "hook grow - makes the hook longer",							PMENU_ALIGN_LEFT, NULL },
	{ "hook stop - retracts the hook",								PMENU_ALIGN_LEFT, NULL },
	{ "clone - enables clone shield (in progress)",								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "*Next Page",								PMENU_ALIGN_LEFT, NextCommandMenu },
	{ "*Close Menu",								PMENU_ALIGN_LEFT, CloseMenu },
};

pmenu_t aliasmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*Insane Weapons Mod Sample Aliases Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "**Main Samples*",								PMENU_ALIGN_LEFT,  },
	{ "alias +hook \"hook action;hook shrink\"",						PMENU_ALIGN_LEFT, NULL },
	{ "alias -hook \"hook stop\"",						PMENU_ALIGN_LEFT, NULL },
	{ "alias +rc_forward \"rc_forward\"",						PMENU_ALIGN_LEFT, NULL },
	{ "alias -rc_forward \"rc_stop_forward\"",							PMENU_ALIGN_LEFT, NULL },
	{ "alias +rc_back \"rc_backward\"",								PMENU_ALIGN_LEFT, NULL },
	{ "alias -rc_back \"rc_stop_forward\"",								PMENU_ALIGN_LEFT, NULL },
	{ "alias +rc_left \"rc_left\"",								PMENU_ALIGN_LEFT, NULL },
	{ "alias -rc_left \"rc_stop_side\"",								PMENU_ALIGN_LEFT, NULL },
	{ "alias +rc_right \"rc_right\"",								PMENU_ALIGN_LEFT, NULL },
	{ "alias -rc_right \"rc_stop_side\"",								PMENU_ALIGN_LEFT, NULL },
	{ "*Close Menu",								PMENU_ALIGN_LEFT, CloseMenu },
};

pmenu_t helpmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*Insane Weapons Mod Help Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER,  },
	{ "Commands",						PMENU_ALIGN_LEFT, FirstCommandMenu },
	{ "Alias Examples",					PMENU_ALIGN_LEFT, OpenAliasesMenu },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ GAMEVERSION,								PMENU_ALIGN_LEFT, NULL },
	{ __DATE__,								PMENU_ALIGN_LEFT, NULL },
	{ __TIME__,								PMENU_ALIGN_LEFT, NULL },
	//{ __LINE__,								PMENU_ALIGN_LEFT, NULL },
	//{ __FILE__,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "A lot of this stuff is incomplete.",								PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] by default",					PMENU_ALIGN_LEFT, NULL },
	{ "to navigate, and enter to",					PMENU_ALIGN_LEFT, NULL },
	{ "select an option.",					PMENU_ALIGN_LEFT, NULL },
};

void CloseMenu (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
}

void FirstCommandMenu(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, commandsmenu, -1, sizeof(commandsmenu) / sizeof(pmenu_t), NULL);
}

void NextCommandMenu(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, commands2menu, -1, sizeof(commands2menu) / sizeof(pmenu_t), NULL);
}

void HelpMenu(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, helpmenu, -1, sizeof(helpmenu) / sizeof(pmenu_t), NULL);
}

void OpenAliasesMenu(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, aliasmenu, -1, sizeof(aliasmenu) / sizeof(pmenu_t), NULL);
}
