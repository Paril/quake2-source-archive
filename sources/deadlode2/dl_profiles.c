#include "g_local.h"

float GetProfileWeight(IniFile *IHnd, int max) {
float totWeight = 0.0;
int		itemtype, num, i;
char	*itemname, *value;
weap_t	*weap;
ammo_t	*ammo;
supply_t *supply;

	for (i = 0; i < IHnd->number_of_sections; i++) {
		itemname = IHnd->section_names[i];

		if (!DL_strcmp(itemname, "PROFILE", -1, false))
			continue;

		num = Ini_ReadInteger(IHnd, itemname, "quantity", 1);
		if (max) num = Ini_ReadInteger(IHnd, itemname, "maximum", num);
		value = Ini_ReadString(IHnd, itemname, "type", NULL);

		if (value) {
			DL_GetIdentValue(value, &itemtype);
			if (itemtype == ITEM_WEAPON) {
				weap = GetWeaponByName(itemname);
				if (!weap) gi.dprintf("PROFILE: Invalid object: %s.\n", itemname);
				else totWeight += weap->weight * num;
			} else if (itemtype == ITEM_AMMO) {
				ammo = GetAmmoByName(itemname);
				if (!ammo) gi.dprintf("PROFILE: Invalid object: %s.\n", itemname);
				else totWeight += ammo->weight * num;
			} else if (itemtype == ITEM_SUPPLY) {
				supply = GetSupplyByName(itemname);
				if (!supply) gi.dprintf("PROFILE: Invalid object: %s.\n", itemname);
				else totWeight += supply->weight * num;
			} else {
				gi.dprintf("PROFILE: Object %s references an invalid object type.\n", itemname);
			}
		} else {
			gi.dprintf("PROFILE: Object %s does not specify an object type.\n", itemname);
		}
	}
	return totWeight;
}

char *ValidateProfileName(char *in, char *out) {
	char mapin[] = {"\\/:*?\"<>|"};
	char mapout[] = {"#########"};
	int mappos;
	char *o;

	o = out;
	while (*in) {
		for (mappos = 0; mapin[mappos]; mappos++)
			if (*in == mapin[mappos])
				break;

		if (mapout[mappos])
			*out++ = mapout[mappos];
		else
			*out++ = *in;
		in++;
	}
	*out = 0;

	return o;
}

void LoadProfile(edict_t *ent, IniFile *IHnd, int flags, float factor) {
int		itemtype, itemnum, itempriority, i, itemmax;
char	*itemname, *altcmd, *link, *value;
item_t	*item;

	for (i = 0; i < IHnd->number_of_sections; i++) {
		itemname = IHnd->section_names[i];

		if (!DL_strcmp(itemname, "PROFILE", -1, false))
			continue;

		itemnum = Ini_ReadInteger(IHnd, itemname, "quantity", 1);
		itemmax = Ini_ReadInteger(IHnd, itemname, "maximum", itemnum);
		itempriority = Ini_ReadInteger(IHnd, itemname, "priority", 0);
		altcmd = Ini_ReadString(IHnd, itemname, "altcmd", NULL);
		link = Ini_ReadString(IHnd, itemname, "link", NULL);

		itemnum *= factor;	// figure in factor AFTER we check max value...


		value = Ini_ReadString(IHnd, itemname, "type", NULL);
		if (value) {
			DL_GetIdentValue(value, &itemtype);
			switch (itemtype) {
			case ITEM_WEAPON:
				if (!flags || (flags & 0x01))
					item = AddToInventory(ent, itemtype, (void *)GetWeaponByName(itemname), itempriority, itemnum, itemmax, NULL, altcmd);
				break;
			case ITEM_AMMO:
				if (!flags || (flags & 0x02))
					item = AddToInventory(ent, itemtype, (void *)GetAmmoByName(itemname), itempriority, itemnum, itemmax, NULL, altcmd);
				break;
			case ITEM_SUPPLY:
				if (!flags || (flags & 0x04))
					item = AddToInventory(ent, itemtype, (void *)GetSupplyByName(itemname), itempriority, itemnum, itemmax, NULL, altcmd);
				break;
			default:
				gi.dprintf("PROFILE: Object %s references an invalid object type.\n", itemname);
			}

			if (!item)
				gi.dprintf("PROFILE: Unknown object: %s\n", itemname);

		} else {
			gi.dprintf("PROFILE: Object %s does not specify an object type.\n", itemname);
		}
	}
}

qboolean OpenPlayerProfile(edict_t *ent) {
char	filename[MAX_QPATH], profile[32], *lastProfile, *player_key, *profile_key;

	if (!profile)
		return false;

	player_key = Info_ValueForKey(ent->client->pers.userinfo, "passkey");
	sprintf(filename, "%s/%s/profiles/players/%s.ini", dlsys.gamedir->string, dlsys.inidir->string, ValidateProfileName(ent->client->pers.netname, profile));

	if ((ent->client->prefs.IHnd = Ini_ReadIniFile(filename)) == NULL) {
		// return true, because no profile exists, but that player can create a new one.
		return true;
	}

	profile_key = Ini_ReadString(ent->client->prefs.IHnd, "PROFILE", "key", NULL);

	if (profile_key && profile_key[0]) {
		if (!player_key) {
			// Automatic false, because a the profile is locked (true with the about line),
			// but the player doesn't have a key, so it can't match. (Saves the time of running DL_strcmp)
			Ini_FreeIniFile(ent->client->prefs.IHnd);
			ent->client->prefs.IHnd = NULL;
			return false;
		}

		if (DL_strcmp(player_key, profile_key, -1, true)) {
			// They don't match, so don't let the player access this profile
			Ini_FreeIniFile(ent->client->prefs.IHnd);
			ent->client->prefs.IHnd = NULL;
			return false;
		}
	}

	ent->client->prefs.adv_menus = Ini_ReadInteger(ent->client->prefs.IHnd, "PROFILE", "AdvancedMenus", false);
	ent->client->prefs.no_menu_background = Ini_ReadInteger(ent->client->prefs.IHnd, "PROFILE", "NoMenuBackground", false);
	ent->client->prefs.menu_pos = Ini_ReadInteger(ent->client->prefs.IHnd, "PROFILE", "MenuPosition", 0);
	ent->client->prefs.auto_profile = Ini_ReadInteger(ent->client->prefs.IHnd, "PROFILE", "AutoProfile", false);

	if (ent->client->prefs.auto_profile) {
		lastProfile = Ini_ReadString(ent->client->prefs.IHnd, "PROFILE", "LastProfile", NULL);
		if (!lastProfile || !lastProfile[0] || (lastProfile[0] == '*'))
			memset(ent->client->resp.next_profile, 255, 32);
		else
			DL_strcpy(ent->client->resp.next_profile, lastProfile, -1);
	}

	return true;
}

void LoadSystemProfile(edict_t *ent, char *profile, int flags, float factor) {
char	filename[MAX_QPATH];
IniFile	*IHnd;

	if (!ent || !ent->client || !profile)
		return;

	sprintf(filename, "%s/%s/profiles/%s.ini", dlsys.gamedir->string, dlsys.inidir->string, profile);
	if ((IHnd = Ini_ReadIniFile(filename)) == NULL){
		gi.dprintf("Error: Unable to read from: %s\n", filename);
		return;
	}

	LoadProfile(ent, IHnd, flags, factor);

	Ini_FreeIniFile(IHnd);

/*	if (profile) {
		// convert spaces to underscores for cfg files..
		sprintf(filename, "exec configs/%s.cfg\n", profile);
		for (i=5; filename[i]; i++)
			if (filename[i] == ' ')
				filename[i] = '_';
		stuffcmd(ent, filename);
	}
*/
}

void AddItemToProfile(edict_t *ent, char *itemName, int type, int count, int max, int priority, char *depend, char *altcmd) {
char *typeName[5] = {"ANY","WEAPON","AMMO","SUPPLY","EQUIPMENT"};

	Ini_WriteString(ent->client->prefs.IHnd, itemName, "Type", typeName[type]);
	if (count >= 1)
		Ini_WriteInteger(ent->client->prefs.IHnd, itemName, "Quantity", count);
	if (max && (max != count))
		Ini_WriteInteger(ent->client->prefs.IHnd, itemName, "Maximum", max);
	if (priority)
		Ini_WriteInteger(ent->client->prefs.IHnd, itemName, "Priority", priority);
	if (depend)
		Ini_WriteString(ent->client->prefs.IHnd, itemName, "Link", depend);
	if (altcmd)
		Ini_WriteString(ent->client->prefs.IHnd, itemName, "AltCmd", altcmd);

	// DEADLODE
	// Lets not forget to write any changes to our profile
	// if ent->lastwrite < level.time)
		Ini_WriteIniFile(ent->client->prefs.IHnd);
}

void DL_Cmd_AddItem(edict_t *ent) {
char *s, *itemName=NULL, *altcmd=NULL;
int i = 1, type = 1, count=0, max=0, maxCnt;
weap_t	*weap;
ammo_t	*ammo;
supply_t *supl;
float curWeight, itemWeight;

	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "usage: AddItem <weapon|ammo|supply> <name> [quantity [maximum]] [altcmd]\n");
		return;
	}

	s = gi.argv(i);
	while (s[0]) {
		if (i == 1) {
			if (!DL_strcmp(s, "weapon", -1, false))
				type = ITEM_WEAPON;
			else if (!DL_strcmp(s, "ammo", -1, false))
				type = ITEM_AMMO;
			else if (!DL_strcmp(s, "supply", -1, false))
				type = ITEM_SUPPLY;
			else {
				gi.cprintf(ent, PRINT_HIGH, "usage: AddItem <weapon|ammo|supply> <name> [quantity [maximum]] [altcmd]\n");
				return;
			}
		} else if (i == 2) {
			switch (type) {
			case ITEM_WEAPON:	if (!(weap = GetWeaponByName(s))) {
									gi.cprintf(ent, PRINT_HIGH, "Unknown Weapon: %s\n", s);
									return;
								}
								itemWeight = weap->weight;
				break;
			case ITEM_AMMO:		if (!(ammo = GetAmmoByName(s))) {
									gi.cprintf(ent, PRINT_HIGH, "Unknown Ammo: %s\n", s);
									return;
								}
								itemWeight = ammo->weight;
				break;
			case ITEM_SUPPLY:	if (!(supl = GetSupplyByName(s)) || (supl->type & SUP_GOAL)) {
									gi.cprintf(ent, PRINT_HIGH, "Unknown Supply: %s\n", s);
									return;
								}
								itemWeight = supl->weight;
				break;
			}
			itemName = s;
		} else if (i == 3) {
			if (isdigit(s[0]))
				count = atoi(s);
			else
				altcmd = s;
		} else if (i == 4) {
			if (isdigit(s[0]))
				max = atoi(s);
			else
				altcmd = s;
		} else if (i == 5)
			altcmd = s;

		s = gi.argv(++i);
	}

	maxCnt = count;
	if (!maxCnt || (max > maxCnt)) maxCnt = max;
	if (!maxCnt) maxCnt = 1; 

	curWeight = GetProfileWeight(ent->client->prefs.IHnd, 1);
	if (curWeight+(itemWeight * maxCnt) >= 300.0) {
		gi.cprintf(ent, PRINT_HIGH, "This item exceeds you maximum weight limit.\nThe item has not be added.\n");
		return;
	}

	AddItemToProfile(ent, itemName, type, count, max, 0, NULL, altcmd);
	gi.cprintf(ent, PRINT_HIGH, "%s: %0.1f/%0.1f/%0.1fm\n", itemName, count * itemWeight, GetProfileWeight(ent->client->prefs.IHnd, 0), GetProfileWeight(ent->client->prefs.IHnd, 1));
}

void DL_Cmd_DropItem(edict_t *ent) {
int num = 0, cur = 0;
char *s;

	if (gi.argc() <= 1) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: dropitem <item name> [quantity]\n");
		return;
	}

	s = gi.argv(1);
	if (FindSection(ent->client->prefs.IHnd, s) == -1) {
		gi.cprintf(ent, PRINT_HIGH, "%s not found in profile.\n", s);
		return;
	}

	num = cur = Ini_ReadInteger(ent->client->prefs.IHnd, s, "Quantity", 1);
	if (gi.argc() > 2)
		num = atoi(gi.argv(2));

	if (num < cur)
		Ini_WriteInteger(ent->client->prefs.IHnd, s, "Quantity", cur - num);
	else
		Ini_RemoveSection(ent->client->prefs.IHnd, s);

	gi.cprintf(ent, PRINT_HIGH, "New Inventory Weight: %0.1f (%0.1f max)\n", GetProfileWeight(ent->client->prefs.IHnd, 0), GetProfileWeight(ent->client->prefs.IHnd, 1));

	// if ent->lastwrite < level.time)
		Ini_WriteIniFile(ent->client->prefs.IHnd);
}		

void DL_Cmd_ClearProfile(edict_t *ent) {
IniFile *IHnd;
int i;

	IHnd = ent->client->prefs.IHnd;

	// Clear remove all sections from the INI file, except "PROFILE".
	for (i=0; i<IHnd->number_of_sections; i++)
		if (DL_strcmp(IHnd->section_names[i], "PROFILE", -1, false))
			if (!Ini_RemoveSection(IHnd, IHnd->section_names[i]))
				i--;

	gi.cprintf(ent, PRINT_HIGH, "New Inventory Weight: %0.1f (%0.1f max)\n", GetProfileWeight(ent->client->prefs.IHnd, 0), GetProfileWeight(ent->client->prefs.IHnd, 1));
	// if ent->lastwrite < level.time)
		Ini_WriteIniFile(ent->client->prefs.IHnd);
}

void DL_Cmd_ShowProfile(edict_t *ent) {
IniFile	*IHnd;
int		itemtype, itemnum, itempriority, i, itemmax;
char	*itemname, *altcmd, *link, *value;

	IHnd = ent->client->prefs.IHnd;

	gi.cprintf(ent, PRINT_HIGH, "Name             Type    Qty  Max  Altcmd\n");
	gi.cprintf(ent, PRINT_HIGH, "===============================================\n");

	for (i = 0; i < IHnd->number_of_sections; i++)
	{
		itemname = IHnd->section_names[i];

		if (!DL_strcmp(itemname, "PROFILE", -1, false))
			continue;

		itemnum = Ini_ReadInteger(IHnd, itemname, "quantity", 1);
		itemmax = Ini_ReadInteger(IHnd, itemname, "maximum", itemnum);
		itempriority = Ini_ReadInteger(IHnd, itemname, "priority", 0);
		altcmd = Ini_ReadString(IHnd, itemname, "altcmd", NULL);
		link = Ini_ReadString(IHnd, itemname, "link", NULL);

		value = Ini_ReadString(IHnd, itemname, "type", NULL);
		if (value) {
			DL_GetIdentValue(value, &itemtype);
			switch (itemtype) {
			case ITEM_WEAPON: gi.cprintf(ent, PRINT_HIGH, "%-16.16s Weapon  %3d  %3d  %s\n", itemname, itemnum, itemmax, altcmd); break;
			case ITEM_AMMO:   gi.cprintf(ent, PRINT_HIGH, "%-16.16s Ammo    %3d  %3d  %s\n", itemname, itemnum, itemmax, altcmd); break;
			case ITEM_SUPPLY: gi.cprintf(ent, PRINT_HIGH, "%-16.16s Supply  %3d  %3d  %s\n", itemname, itemnum, itemmax, altcmd); break;
			default:          gi.cprintf(ent, PRINT_HIGH, "%s: Unknown item.\n", itemname);
			}
		} else {
			gi.cprintf(ent, PRINT_HIGH, "%s: Unknown item.\n", itemname);
		}
	}
}

void DL_Cmd_SetAltCmd(edict_t *ent) {
char *s, *cur;

	if (gi.argc() <= 1) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: altcmd <item name> [cmd]\n");
		return;
	}

	s = gi.argv(1);
	if (FindSection(ent->client->prefs.IHnd, s) == -1) {
		gi.cprintf(ent, PRINT_HIGH, "%s not found in profile.\n", s);
		return;
	}

	if (gi.argc() > 2) {
		Ini_WriteString(ent->client->prefs.IHnd, s, "AltCmd", gi.argv(2));
		// if ent->lastwrite < level.time)
			Ini_WriteIniFile(ent->client->prefs.IHnd);
	} else {
		if ((cur = Ini_ReadString(ent->client->prefs.IHnd, s, "AltCmd", NULL))==NULL)
			gi.cprintf(ent, PRINT_HIGH, "%s does not have an altcmd.\n", s);
		else
			gi.cprintf(ent, PRINT_HIGH, "%s = %s\n", s, cur);
	}
}

void DL_DoPlayerStats(edict_t *targ, edict_t *attacker, int mod) {
	switch (mod) {
		case MOD_KILLSELF:
		case MOD_BLDG_KILLSELF:
			targ->client->prefs.stats.total_suicides++;
			break;
		case MOD_BLDG_KILLOWNER:
			if (attacker->client)
				attacker->client->prefs.stats.total_kills++;
			targ->client->prefs.stats.total_deaths++;
			break;
		case MOD_KILLTEAM:
		case MOD_BLDG_KILLTEAM:
			targ->client->prefs.stats.total_deaths++;
			break;
		case MOD_KILLOTHER:
		case MOD_BLDG_KILLOTHER:
			if (attacker->client) {
				attacker->client->prefs.stats.total_kills++;
				if (attacker->goalitem)
					attacker->client->prefs.stats.chasers_killed++;
				if (targ->goalitem)
					attacker->client->prefs.stats.carriers_killed++;
			}
			targ->client->prefs.stats.total_deaths++;
			break;
	}
}

void DL_ReadPlayerStats(edict_t *ent) {
IniFile	*IHnd;

	if (!ent || !ent->client || !ent->client->prefs.IHnd)
		return;

	IHnd = ent->client->prefs.IHnd;

	ent->client->prefs.stats.total_kills = Ini_ReadInteger(IHnd, "PROFILE", "TotalKills", 0);
	ent->client->prefs.stats.total_deaths = Ini_ReadInteger(IHnd, "PROFILE", "TotalDeaths", 0);
	ent->client->prefs.stats.total_suicides = Ini_ReadInteger(IHnd, "PROFILE", "TotalSuicides", 0);
	ent->client->prefs.stats.carriers_killed = Ini_ReadInteger(IHnd, "PROFILE", "CarriersKilled", 0);
	ent->client->prefs.stats.chasers_killed = Ini_ReadInteger(IHnd, "PROFILE", "ChasersKilled", 0);

	ent->client->prefs.stats.game_time = Ini_ReadDouble(IHnd, "PROFILE", "GameTime", 0.0);
	ent->client->prefs.stats.carry_time = Ini_ReadDouble(IHnd, "PROFILE", "CarryTime", 0.0);
}


void DL_WritePlayerStats(edict_t *ent) {
IniFile	*IHnd;

	if (!ent || !ent->client || !ent->client->prefs.IHnd)
		return;

	IHnd = ent->client->prefs.IHnd;

	// Player Stats
	Ini_WriteInteger(IHnd, "PROFILE", "TotalKills", ent->client->prefs.stats.total_kills);
	Ini_WriteInteger(IHnd, "PROFILE", "TotalDeaths", ent->client->prefs.stats.total_deaths);
	Ini_WriteInteger(IHnd, "PROFILE", "TotalSuicides", ent->client->prefs.stats.total_suicides);
	Ini_WriteInteger(IHnd, "PROFILE", "CarriersKilled", ent->client->prefs.stats.carriers_killed);
	Ini_WriteInteger(IHnd, "PROFILE", "ChasersKilled", ent->client->prefs.stats.chasers_killed);
	Ini_WriteDouble(IHnd, "PROFILE", "GameTime", ent->client->prefs.stats.game_time);
	Ini_WriteDouble(IHnd, "PROFILE", "CarryTime", ent->client->prefs.stats.carry_time);

	// Player Settings
	Ini_WriteInteger(IHnd, "PROFILE", "AdvancedMenus", ent->client->prefs.adv_menus);
	Ini_WriteInteger(IHnd, "PROFILE", "NoMenuBackground", ent->client->prefs.no_menu_background);
	Ini_WriteInteger(IHnd, "PROFILE", "MenuPosition", ent->client->prefs.menu_pos);
	Ini_WriteInteger(IHnd, "PROFILE", "AutoProfile", ent->client->prefs.auto_profile);

	if (ent->client->resp.profile[0] == -1)
		Ini_WriteString(IHnd, "PROFILE", "LastProfile", "*Custom");
	else
		Ini_WriteString(IHnd, "PROFILE", "LastProfile", ent->client->resp.profile);
}