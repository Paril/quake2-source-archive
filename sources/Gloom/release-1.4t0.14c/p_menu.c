/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "g_local.h"
#include "menus.h"
#include <stdio.h>

void PMenu_Update(edict_t *ent)
        {
        char string[1400]="";
        int i;
        pmenu_t *p;
        int x, lastx = 0, len=0;
        pmenuhnd_t *hnd;
        char *t;
        qboolean alt = false;


        if (ent->client->menu.entries == engineer_menu && sv_extended && !(ent->client->pers.uflags & UFLAG_NO_PREVIEWS))
            ViewStructurePreview(ent, ent->client->menu.cur);


        hnd = &ent->client->menu;

        if (hnd->cur != -1)
        { 
                len = sprintf(string, "xv 54 yv %d string2 \"\x0d\" ", 32 + hnd->cur * 8); // 56
        }

        for (i = 0, p = hnd->entries; ; i++, p++)
        {
                if (!p->text)
                        break;

                if (!p->text[0]) {
                        continue; // blank line
                }

                //if (!(*p->text))
                        //break;

                t = p->text;
                if (*t == '*') {
                        alt = true;
                        t++;
                }

                x = 64;

                if (p->align == PMENU_ALIGN_CENTER)
                        x = 196/2 - (int)strlen(t)*4 + 64;
                else if (p->align == PMENU_ALIGN_RIGHT)
                        x = 64 + (196 - (int)strlen(t)*8);

                if (lastx != x)
                {
                        len += sprintf(string + len, "xv %d ", x);
                        lastx = x;
                }

                len += sprintf(string + len, "yv %d string%s \"%s\" ", 32 + i * 8, alt ? "2" : "", t);

                alt = false;
        }

        if (AppendPlayerQueueLayout(ent, string+len, sizeof(string)-len) || len)
        {
                gi.WriteByte (svc_layout);
                gi.WriteString (string);
                gi.unicast(ent,true);
        }
}

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, uint16_t flags)
{
        pmenuhnd_t *hnd;
        pmenu_t *p;
        int i;

        hnd = &ent->client->menu;

        hnd->entries = entries;
        entries->menuflags = flags;

        if (!(entries->menuflags & MENU_HIDE)) {
            if (entries==alien_menu || entries==alien_menu_exit) {
                    ent->client->ps.stats[STAT_MENU] = imagecache[alienmenu];
            } else if (entries==human_menu || entries==human_menu_exit) {
                    ent->client->ps.stats[STAT_MENU] = imagecache[humanmenu];
            } else if (entries==team_menu) {
                    ent->client->ps.stats[STAT_MENU] = imagecache[teammenu];
            } else if (entries==breeder_menu) {
                    ent->client->ps.stats[STAT_MENU] = imagecache[breedermenu];
            } else if (entries==engineer_menu) {
                if (ent->client->pers.uflags & UFLAG_NO_ENGIMENU)
                    entries->menuflags |= MENU_HIDE;
                else
                    ent->client->ps.stats[STAT_MENU] = imagecache[engiemenu];
            } else
                    ent->client->ps.stats[STAT_MENU] = imagecache[inventory];
        }
        
        if (cur < 0 || !entries[cur].SelectFunc) {
                for (i = 0, p = entries; ; i++, p++) {
                        if (!p->text) { // && !(*p->text)) {
                                i = -1;
                                break;
                        }
                        if (p->SelectFunc)
                                break;
                }
        } else
                i = cur;

        hnd->cur = i;

        if (!(hnd->entries->menuflags & MENU_HIDE))
            ent->client->showscores = 1;

        if (ent->client->preview_ent)
            ViewStructurePreview(ent, 0);

        PMenu_Update(ent);
}

void PMenu_Close(edict_t *ent)
{
        ent->client->ps.stats[STAT_MENU] = 0;

        ent->client->menu.entries = NULL;
        ent->client->showscores = 0;

        if (ent->client->preview_ent)
            ViewStructurePreview(ent, 0);
}

void PMenu_Next(edict_t *ent)
{
        pmenuhnd_t *hnd;
        int i;
        pmenu_t *p;

        hnd = &ent->client->menu;

        if (hnd->entries->menuflags & MENU_NOBROWSE) {
            PMenu_Close(ent);
            return;
        }
        
        if (hnd->cur < 0)
                return; // no selectable entries

        i = hnd->cur;
        p = hnd->entries + hnd->cur;
        do {
                i++, p++;
                if (!p->text)// && !(*p->text))
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

        hnd = &ent->client->menu;

        if (hnd->entries->menuflags & MENU_NOBROWSE) {
            PMenu_Close(ent);
            return;
        }
        
        if (hnd->cur < 0)
                return; // no selectable entries

        i = hnd->cur;
        p = hnd->entries + hnd->cur;
        do
        {
                if (i == 0)
                {
                        for (p = hnd->entries; ; p++)
                        {
                                if (!p->text)// && !(*p->text))
                                        break;
                        }
                        p--;
                        i = p - hnd->entries;
                }
                else
                {
                        i--, p--;
                }

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

        hnd = &ent->client->menu;

        if (hnd->cur < 0)
                return; // no selectable entries

        p = hnd->entries + hnd->cur;

        if (p->SelectFunc)
                p->SelectFunc(ent);
}
