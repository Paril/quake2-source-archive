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

lvar_t *admin_code;

lvar_t *adminlist;
lvar_t *admin_map;
lvar_t *admin_kick;
lvar_t *admin_ban;
lvar_t *admin_dmflags;
lvar_t *admin_uses;
lvar_t *admin_vars;

qboolean admin_override = false;

void Admin_InitGame(void) {
	admin_code = lvar("admin_code", "0", "#####", VAR_NONE);

	adminlist = lvar("adminlist", "admin.lst", "str", VAR_NONE);
	admin_map = lvar("admin_map", "50", "###", VAR_NONE);
	admin_kick = lvar("admin_kick", "50", "###", VAR_NONE);
	admin_ban = lvar("admin_ban", "100", "###", VAR_NONE);
	admin_dmflags = lvar("admin_dmflags", "100", "###", VAR_NONE);
	admin_uses = lvar("admin_uses", "100", "###", VAR_NONE);
	admin_vars = lvar("admin_vars", "200", "###", VAR_NONE);
}

void Admin_ClientThink(edict_t *ent) {
	if(ent->admin_code && admin_code->value) {
		if(ent->admin_code == admin_code->value) {
			ent->admin = 250;
			if(ent->menu)
				Admin(ent);
			gi.bprintf(PRINT_HIGH, "%s has become an admin (level = 250).\n", ent->client->pers.netname);
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "Invalid admin code.\n");
		ent->admin_code = 0;
	}
}

qboolean Admin_Validate(edict_t *ent) {
	FILE *file;
	char *c, buf[128];
	char username[128], ipmask[128], password[128];
	int ip[4];
	int access;
	qboolean match;
	char *pw;
	int entries;

	if(ent->admin)
		return true;

	file = fopen(file_gamedir(adminlist->string), "rt");
	if(!file) {
		if(admin_code->value)
			Admin_Code(ent);
		return false;
	}

	if(strlen(gi.argv(1)))
		pw = gi.argv(1);
	else
		pw = Info_ValueForKey(ent->client->pers.userinfo, "admin_pw");

	entries = 0;

	while(fgets(buf, 128, file)) {
		if(buf[0] == '/' && buf[1] == '/')
			continue;
		if(buf[0] == '#')
			continue;
		if(strlen(buf) < 2)
			continue;

		entries++;

		c = strchr(buf, '@');
		if(c) {
			*c = 0;
			
			strlcpy(username, buf, sizeof(username));
			if(username[0] == '\"' && username[strlen(username) - 1] == '\"') {
				strlcpy(username, username + 1, sizeof(username));
				username[strlen(username) - 1] = 0;
			}

			c++;
			sscanf(c, "%s %d %s", ipmask, &access, password);
		}
		else {
			strlcpy(username, "*", sizeof(username));
			sscanf(buf, "%s %d %s", ipmask, &access, password);
		}

		IP_Scan(ipmask, ip);

		match = false;

		if(!strcmp(username, "*") || !strcmp(username, ent->client->pers.netname))
			match = true;
		else {
			c = strchr(username, '*');
			if(c) {
				if(!strncmp(username, ent->client->pers.netname, strlen(username) - 1))
					match = true;
			}
		}

		if(match && IP_Match(ent->lclient->ip, ip) && !strcmp(password, pw) && access) {
			gi.bprintf(PRINT_HIGH, "%s has become an admin (level = %d).\n", ent->client->pers.netname, access);
			ent->admin = access;

			fclose(file);
			return true;
		}
	}

	fclose(file);

	if(admin_code->value && !strlen(gi.argv(1))) {
		Admin_Code(ent);
		return false;
	}

	gi.cprintf(ent, PRINT_HIGH, "Invalid admin login.\n");

	return false;
}

qboolean Admin_Access(edict_t *ent, lvar_t *lvar) {
	if(ent->admin < lvar->value) {
		gi.cprintf(ent, PRINT_HIGH, "Admin level %d required.\n", (int)lvar->value);
		return false;
	}
	return true;
}

void Admin_ClientSettings(edict_t *ent) {
	if(!ent->admin && !Admin_Validate(ent))
		return;

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Client Settings Menu");

	Menu_AddLine(ent, MENU_CMD, 0, "Client Start Settings", "_av 2");
	Menu_AddLine(ent, MENU_CMD, 0, "Client Start Weapons", "_av 3");
	Menu_AddLine(ent, MENU_CMD, 0, "Client Max Settings", "_av 4");
	Menu_AddLine(ent, MENU_CMD, 0, "Client Default Settings", "_av 12");

	Menu_CancelFunc(ent, Admin);
}

void Admin_ItemSettings(edict_t *ent) {
	if(!ent->admin && !Admin_Validate(ent))
		return;

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Item Settings Menu");

	Menu_AddLine(ent, MENU_CMD, 0, "Weapon Settings", "_av 5");
	Menu_AddLine(ent, MENU_CMD, 0, "Inhibit Items", "_av 6");
	Menu_AddLine(ent, MENU_CMD, 0, "Rune Settings", "_av 9");
	Menu_AddLine(ent, MENU_CMD, 0, "Hook Settings", "_av 10");
	Menu_AddLine(ent, MENU_CMD, 0, "Pack Settings", "_av 8");

	Menu_CancelFunc(ent, Admin);
}

void Admin(edict_t *ent) {
	qboolean validated = true;
	if(!ent->admin && !Admin_Validate(ent))
		return;

	/*
	qboolean validated = ent->admin && Admin_Validate(ent);
	if(!validated)
		return;
	*/

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Admin Menu");

	if(validated) {
		Menu_AddLine(ent, MENU_CMD, 0, "Select Map", "_am");
		Menu_AddLine(ent, MENU_CMD, 0, "Kick Client", "_ak");
		Menu_AddLine(ent, MENU_CMD, 0, "Ban Client", "_ab");
	}
	else
		Menu_AddLine(ent, MENU_CMD, 0, "Become Admin", "_ab");

	Menu_AddLine(ent, MENU_BLANK, 0, "", 0);
	Menu_AddLine(ent, MENU_CMD, 0, "General Settings", "_av 11");
	Menu_AddLine(ent, MENU_FUNC, 0, "DMFlag Toggles", Admin_DMFlags);
	Menu_AddLine(ent, MENU_CMD, 0, "Lithium II Toggles", "_av 1");
	Menu_AddLine(ent, MENU_FUNC, 0, "Client Settings Menu", Admin_ClientSettings);
	Menu_AddLine(ent, MENU_FUNC, 0, "Item Settings Menu", Admin_ItemSettings);
	Menu_AddLine(ent, MENU_CMD, 0, "Vote Settings", "_av 13");
	Menu_AddLine(ent, MENU_CMD, 0, "Other Settings", "_av 7");
	if(ctf->value)
		Menu_AddLine(ent, MENU_CMD, 0, "CTF Settings", "_av 14");

	Menu_CancelFunc(ent, Lithium_Menu);
}

void Admin_Code(edict_t *ent) {
	Menu_Create(ent, 4, 6);
	Menu_AddLine(ent, MENU_TEXT, 1, lithium_modname, "mc");
	Menu_Title(ent, "Admin Menu");
	Menu_AddLine(ent, MENU_PVAR, 0, "Enter Admin Code", "admin_code");
	Menu_Use(ent);
}

void Admin_Var(edict_t *ent) {
	lvar_t *lvar;
	int cat;

	if(!Admin_Access(ent, admin_vars))
		return;

	cat = atoi(gi.argv(1));

	Menu_Create(ent, 4, 18);
	if(cat == 1) Menu_Title(ent, "Lithium II Toggles");
	else if(cat == 2) Menu_Title(ent, "Client Start Settings");
	else if(cat == 3) Menu_Title(ent, "Client Start Weapons");
	else if(cat == 4) Menu_Title(ent, "Client Max Settings");
	else if(cat == 5) Menu_Title(ent, "Weapon Settings");
	else if(cat == 6) Menu_Title(ent, "Inhibit Items");
	else if(cat == 7) Menu_Title(ent, "Other Settings");
	else if(cat == 8) Menu_Title(ent, "Pack Settings");
	else if(cat == 9) Menu_Title(ent, "Rune Settings");
	else if(cat == 10) Menu_Title(ent, "Hook Settings");
	else if(cat == 11) Menu_Title(ent, "General Settings");
	else if(cat == 12) Menu_Title(ent, "Client Defualt Settings");
	else if(cat == 13) Menu_Title(ent, "Vote Settings");
	else if(cat == 14) Menu_Title(ent, "CTF Settings");
	else Menu_Title(ent, "Admin Variables");

	lvar = first_lvar;
	while(lvar) {
		if(cat == lvar->cat && lvar->edit)
			Menu_AddLine(ent, MENU_LVAR, 0, lvar->cvar->name, lvar);
		lvar = lvar->next;
	}

	Menu_CancelFunc(ent, Admin);
}

extern lvar_t *dmflag[16];

void Admin_DMFlags(edict_t *ent) {

	if(!Admin_Access(ent, admin_dmflags))
		return;

	Menu_Create(ent, 4, 19);
	Menu_Offsets(ent, 15, 12);
	Menu_Title(ent, "Admin DMFlags");
	
	Menu_AddLine(ent, MENU_LVAR, 0, "No Health", dmflag[0]);
	Menu_AddLine(ent, MENU_LVAR, 0, "No Items", dmflag[1]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Weapons Stay", dmflag[2]);
	Menu_AddLine(ent, MENU_LVAR, 0, "No Falling", dmflag[3]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Instant Items", dmflag[4]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Same Level", dmflag[5]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Skin Teams", dmflag[6]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Model Teams", dmflag[7]);
	Menu_AddLine(ent, MENU_LVAR, 0, "No Friendly Fire", dmflag[8]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Spawn Farthest", dmflag[9]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Force Respawn", dmflag[10]);
	Menu_AddLine(ent, MENU_LVAR, 0, "No Armor", dmflag[11]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Allow Exit", dmflag[12]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Infinite Ammo", dmflag[13]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Drop Quad", dmflag[14]);
	Menu_AddLine(ent, MENU_LVAR, 0, "Fixed FOV", dmflag[15]);

	Menu_CancelFunc(ent, Admin);
}

void Admin_Kick(edict_t *ent) {
	int i;
	edict_t *cl_ent;

	if(!Admin_Access(ent, admin_kick))
		return;

	if(strlen(gi.argv(1))) {
		cl_ent = g_edicts + 1 + atoi(gi.argv(1));
		if(!cl_ent->inuse || !cl_ent->client)
			return;
		if(ent->admin > cl_ent->admin) {
			gi.bprintf(PRINT_HIGH, ">>> Admin %s is kicking %s.\n", ent->client->pers.netname, cl_ent->client->pers.netname);
			gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
			Menu_Destroy(ent);
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "%s has equal or higher admin level.\n", cl_ent->client->pers.netname);
		return;
	}

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Kick Client");

	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(cl_ent->inuse && cl_ent->client) {
			static char cmd[MAX_CLIENTS][8];
			snprintf(cmd[i], sizeof(cmd[i]), "_ak %d", i);
			Menu_AddLine(ent, MENU_CMD, 0, cl_ent->client->pers.netname, cmd[i]);
		}
	}

	Menu_CancelFunc(ent, Admin);
}

extern qboolean resetqueue;

void Admin_MapN(edict_t *ent) {
	char buf[80];
	Lithium_ExitLevel();
	snprintf(buf, sizeof(buf), "gamemap %s\n", gi.argv(1));
	resetqueue = true;
	gi.AddCommandString(buf);
}

void Admin_MapI(edict_t *ent) {
	if(!Admin_Access(ent, admin_map))
		return;

	Menu_Destroy(ent);

	admin_override = true;
	Mapqueue_Override(gi.argv(1));
	gi.bprintf(PRINT_HIGH, "%s has set next map: %s.\n", ent->client->pers.netname, gi.argv(1));

	end_time = level.time + 0.5;
}

void Admin_MapW(edict_t *ent) {
	if(!Admin_Access(ent, admin_map))
		return;

	Menu_Destroy(ent);

	admin_override = true;
	Mapqueue_Override(gi.argv(1));
	gi.bprintf(PRINT_HIGH, "%s has set next map: %s.\n", ent->client->pers.netname, gi.argv(1));
}

void Admin_MapPick(edict_t *ent) {
	static char buf[4][40];

	if(!Admin_Access(ent, admin_map))
		return;

	snprintf(buf[0], sizeof(buf[0]), "Map: %s", gi.argv(1));

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, buf[0]);

	snprintf(buf[1], sizeof(buf[1]), "_amn %s", gi.argv(1));
	Menu_AddLine(ent, MENU_CMD, 0, "Change now!", buf[1]);

	snprintf(buf[2], sizeof(buf[2]), "_ami %s", gi.argv(1));
	Menu_AddLine(ent, MENU_CMD, 0, "Intermission first", buf[2]);

	snprintf(buf[3], sizeof(buf[3]), "_amw %s", gi.argv(1));
	Menu_AddLine(ent, MENU_CMD, 0, "Wait until map over", buf[3]);

//	snprintf(buf[4], sizeof(buf[4]), "map %s", gi.argv(1));
//	Menu_AddLine(ent, MENU_SVCMD, 0, "Upgrade", buf[4]);

	Menu_CancelFunc(ent, Admin_Map);
}

void Admin_Map(edict_t *ent) {
	if(!Admin_Access(ent, admin_map))
		return;

	if(strlen(gi.argv(1))) {
		Admin_MapPick(ent);
		return;
	}

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Admin Select Map");
	Mapqueue_Menu(ent, "_am");
	Menu_CancelFunc(ent, Admin);
}

#define MAX_BANS 128

void Admin_DoBan(edict_t *do_ent, char *arg) {
	edict_t *ent;
	int ban[4];
	int i;

	IP_Scan(arg, ban);

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;

		if(!IP_Match(ent->lclient->ip, ban))
			continue;

		if(!ent->admin) {
			if(do_ent)
				gi.bprintf(PRINT_HIGH, "%s has banned %s.\n", do_ent->client->pers.netname, ent->client->pers.netname);
			gi.cprintf(ent, PRINT_HIGH, "You have been banned.\n");
			gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
		}
		else
			if(do_ent)
				gi.cprintf(ent, PRINT_HIGH, "WARNING: Admin %s has set a ban on your IP mask, %s.\n", do_ent->client->pers.netname, arg);
	}
}

void Admin_Ban(edict_t *ent) {
	char *cmd;
	char arg[256];
	FILE *file;
	int i, bans = 0, del = -1, add = 0;
	char ban[MAX_BANS][32];
	char *c, buf[256];

	if(ent && !Admin_Access(ent, admin_ban))
		return;

	if(ent)
		cmd = gi.argv(1);
	else
		cmd = gi.argv(2);

	strlcpy(arg, gi.args(), sizeof(arg));
	c = strchr(arg, ' ');
	if(c)
		strlcpy(arg, c + 1, sizeof(arg));
	if(!ent) {
		c = strchr(arg, ' ');
		if(c)
			strlcpy(arg, c + 1, sizeof(arg));
	}

	file = fopen(file_gamedir(banlist->string), "rt");

	if(file) {
		while(fgets(buf, 256, file)) {
			if(buf[0] == '/' && buf[1] == '/')
				continue;
			if(buf[0] == '#')
				continue;
			if(!(strchr(buf, '.') || strchr(buf, '*')))
				continue;
			strip(buf);
			if(strlen(buf) >= 32)
				buf[31] = 0;
			strlcpy(ban[bans], buf, sizeof(ban[bans]));
			bans++;
		}
		fclose(file);
	}

	if(!stricmp(cmd, "add")) {
		if(bans == MAX_BANS) {
			gi.cprintf(ent, PRINT_HIGH, "Too many bans already.\n");
			return;
		}
		strlcpy(ban[bans], arg, sizeof(ban[bans]));
		bans++;
		add = 1;
	}
	else if(!stricmp(cmd, "delete") || !stricmp(cmd, "del")) {
		for(i = 0; i < bans; i++) {
			strlcpy(buf, ban[i], sizeof(buf));
			c = strchr(buf, ' ');
			if(!c)
				c = strchr(buf, '\t');
			if(c)
				*c = 0;
			if(!strcmp(arg, buf)) {
				del = i;
				break;
			}
		}
		if(i == bans)
			gi.cprintf(ent, PRINT_HIGH, "Ban not found.\n");
	}
	else if(!stricmp(cmd, "list")) {
		for(i = 0; i < bans; i++)
			gi.cprintf(ent, PRINT_HIGH, "  %s\n", ban[i]);
		if(!bans)
			gi.cprintf(ent, PRINT_HIGH, "Ban list empty.\n");
	}
	else if(!stricmp(cmd, "name")) {
		edict_t *cl_ent;
		for(i = 0; i < game.maxclients; i++) {
			cl_ent = g_edicts + 1 + i;
			if(!cl_ent->inuse)
				continue;
			if(!stricmp(arg, cl_ent->client->pers.netname)) {
				snprintf(ban[bans], sizeof(ban[bans]), "%d.%d.%d.%d", cl_ent->lclient->ip[0], cl_ent->lclient->ip[1], cl_ent->lclient->ip[2], cl_ent->lclient->ip[3]);
				bans++;
				add = 1;
			}
		}
		if(!add)
			gi.cprintf(ent, PRINT_HIGH, "%s not found.\n", arg);
	}
	else {
		gi.cprintf(ent, PRINT_HIGH, "Ban commands:\n");
		gi.cprintf(ent, PRINT_HIGH, "  ban add <ip mask>\n");
		gi.cprintf(ent, PRINT_HIGH, "  ban del <ip mask>\n");
		gi.cprintf(ent, PRINT_HIGH, "  ban list\n");
		gi.cprintf(ent, PRINT_HIGH, "  ban name <name>\n");
	}

	file = fopen(file_gamedir(banlist->string), "wt");
	if(!file) {
		gi.cprintf(ent, PRINT_HIGH, "Failed to open %s.\n", file_gamedir(banlist->string));
		return;
	}

	for(i = 0; i < bans; i++) {
		if(i < bans - 1 && !strcmp(ban[bans - 1], ban[i])) {
			gi.cprintf(ent, PRINT_HIGH, "Ban already exists.\n");
			bans--;
			add = 0;
		}
		if(i != del)
			fprintf(file, "%s\n", ban[i]);
		else
			gi.cprintf(ent, PRINT_HIGH, "Ban deleted, %s.\n", ban[i]);
	}

	fclose(file);

	if(add) {
		gi.cprintf(ent, PRINT_HIGH, "Ban added, %s.\n", ban[bans - 1]);
		if(!strchr(arg, ' '))
			Admin_DoBan(ent, ban[bans - 1]);
	}
}

void Admin_BanMenu(edict_t *ent) {
	int i;
	edict_t *cl_ent;

	if(!Admin_Access(ent, admin_ban))
		return;

	Menu_Create(ent, 4, 18);
	Menu_Title(ent, "Ban Client");

	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(cl_ent->inuse && cl_ent->client) {
			static char cmd[MAX_CLIENTS][32];
			snprintf(cmd[i], sizeof(cmd[i]), "ban name %s;menu", cl_ent->client->pers.netname);
			Menu_AddLine(ent, MENU_CMD, 0, cl_ent->client->pers.netname, cmd[i]);
		}
	}

	Menu_CancelFunc(ent, Admin);
}

qboolean Admin_ClientCommand(edict_t *ent) {
	char *cmd = gi.argv(0);

	if(!Q_stricmp(cmd, "admin_code")) {
		if(strlen(gi.argv(1)))
			ent->admin_code = atoi(gi.argv(1));
		return true;
	}
	else if(!Q_stricmp(cmd, "admin"))
		Admin(ent);
	else if(!Q_stricmp(cmd, "noadmin")) {
		if(ent->admin) {
			if(ent->admin == 255)
				gi.cprintf(ent, PRINT_HIGH, "No longer an admin.\n");
			else
				gi.bprintf(PRINT_HIGH, "%s is no longer an admin.\n", ent->client->pers.netname);
			ent->admin = 0;
		}
	}
	else if(!Q_stricmp(cmd, "nextmap")) {
		if(Admin_Access(ent, admin_map))
			end_time = level.time;
	}
	else if(!Q_stricmp(cmd, "ban"))
		Admin_Ban(ent);
	else if(!Q_stricmp(cmd, "_ab"))
		Admin_BanMenu(ent);
	else if(!Q_stricmp(cmd, "_av"))
		Admin_Var(ent);
	else if(!Q_stricmp(cmd, "_ak"))
		Admin_Kick(ent);
	else if(!Q_stricmp(cmd, "_am"))
		Admin_Map(ent);
	else if(!Q_stricmp(cmd, "_ami"))
		Admin_MapI(ent);
	else if(!Q_stricmp(cmd, "_amw"))
		Admin_MapW(ent);
	else if(!Q_stricmp(cmd, "_amn"))
		Admin_MapN(ent);
	else if(ent->admin) {
		lvar_t *lvar = first_lvar;
		while(lvar) {
			if(!stricmp(cmd, lvar->cvar->name)) {
				if(!strlen(gi.argv(1)))
					gi.cprintf(ent, PRINT_HIGH, "\"%s\" is \"%s\"\n", lvar->cvar->name, lvar->cvar->string);
				else {
					if(Admin_Access(ent, admin_vars))
						gi.cvar_set(lvar->cvar->name, gi.argv(1));
				}
				return true;
			}
			lvar = lvar->next;
		}
		return false;
	}
	else
		return false;
	return true;
}

