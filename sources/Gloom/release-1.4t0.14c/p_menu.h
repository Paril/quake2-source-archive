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

enum {
        PMENU_ALIGN_LEFT,
        PMENU_ALIGN_CENTER,
        PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s {
        struct pmenu_s *entries;
        int cur;
} pmenuhnd_t;

typedef struct pmenu_s {
        char *text;
        int align;
        void (*SelectFunc)(edict_t *ent);
        uint16_t menuflags;
} pmenu_t;

enum menuflags_e {
        MENU_HIDE = 1,
        MENU_NOBROWSE = 2      //Causes invnext/invprev to close the menu. Used by preview impulse commands.
};

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, uint16_t flags);
void PMenu_Close(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);

extern pmenu_t breeder_menu[];
extern pmenu_t engineer_menu[];
extern pmenu_t alien_menu[];
extern pmenu_t human_menu[];
extern pmenu_t alien_menu_exit[];
extern pmenu_t human_menu_exit[];
extern pmenu_t team_menu[];

extern pmenu_t  upgrade_menu_kami[];
extern pmenu_t  upgrade_menu_drone[];
extern pmenu_t  upgrade_menu_commando[];
extern pmenu_t  upgrade_menu_exterm[];
extern pmenu_t  upgrade_menu_mech[];
extern pmenu_t  upgrade_menu_guardian[];
extern pmenu_t  upgrade_menu_stalker[];
extern pmenu_t  upgrade_menu_ht[];
extern pmenu_t  upgrade_menu_st[];
extern pmenu_t  upgrade_menu_hatch[];
extern pmenu_t  upgrade_menu_grunt[];
extern pmenu_t  upgrade_menu_engi[];
extern pmenu_t  upgrade_menu_wraith[];
extern pmenu_t  upgrade_menu_bio[];

extern pmenu_t menu_exitqueue[];
extern pmenu_t menu_join_admin[];
