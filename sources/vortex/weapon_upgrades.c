#include "g_local.h"

void OpenGeneralWeaponMenu (edict_t *ent, int lastline);

//************************************************************************************************

int V_WeaponUpgradeVal(edict_t *ent, int weapnum)
{
    //Returns an integer value, usualy from 0-100. Used as a % of maximum upgrades statistic

	int	i;
	float iMax, iCount;
	float val;
	
	iMax = iCount = 0.0f;

	if ((weapnum >= MAX_WEAPONS) || (weapnum < 0))
		return -666;	//BAD weapon number

	for (i=0; i<MAX_WEAPONMODS;++i)
	{
		iCount += ent->myskills.weapons[weapnum].mods[i].current_level;
		iMax += ent->myskills.weapons[weapnum].mods[i].soft_max;
	}

	if (iMax == 0)
		return 0;

	val = (iCount / iMax) * 100;
	
	return (int)val;
}

//************************************************************************************************
//************************************************************************************************
/*
			This is how the weapon upgrades menu works.

				Each weapon has an index (defined in weapon_def.h)

				The current menu system requires an option that is not one of it's default
				formatting options, like MENU_GREEN_CENTERED. In order to bypass that, and
				still allow for the menu system to know which weapon it is meant to access,
				A code was used to index the option number. Here it is:


	example:	Blaster	=	1100
				Shotgun =	1200
				Railgun =	1800
				etc...

				Each mod is added to the base weapon number, and the function looks at that.

	example:	Blaster Damage =	1100
				Shotgun Spread =	1204
				Railgun Flash  =	1805


				Using this method, each weapon upgrade is indexed so only one menu is needed
				to load ANY of the weapons. This saves code space, and should be easier to modify.
*/
//************************************************************************************************
//************************************************************************************************

//************************************************************************************************
//	Secondary menu
//************************************************************************************************

void generalWeaponMenu_handler(edict_t *ent, int option)
{
	int WeaponIndex	= (option / 100)-10;
	int ModIndex	= (option % 100) - 1;
	
	//Are we just navigating?
	if (option == 6666)
	{
		closemenu(ent);
		return;
	}
	else if (option >= 7777)
	{
        int LastWeapon = option - 7777;
		OpenWeaponUpgradeMenu(ent, LastWeapon+1);
		return;
	}

    if (!(ent->myskills.weapons[WeaponIndex].mods[ModIndex].level < ent->myskills.weapons[WeaponIndex].mods[ModIndex].soft_max))
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum upgrade level in %s %s (%d).\n",
			GetWeaponString(WeaponIndex), GetModString(WeaponIndex,ModIndex), 
			ent->myskills.weapons[WeaponIndex].mods[ModIndex].soft_max);
		OpenGeneralWeaponMenu(ent, option);
		return;
	}

	if (ent->myskills.weapon_points < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You are out of weapon points.\n");
		closemenu(ent);
		return;
	}

	ent->myskills.weapons[WeaponIndex].mods[ModIndex].current_level++;
	ent->myskills.weapons[WeaponIndex].mods[ModIndex].level++;
	ent->myskills.weapon_points--;

	safe_cprintf(ent, PRINT_HIGH, "%s %s upgraded to level %d.\n",	GetWeaponString(WeaponIndex),GetModString(WeaponIndex,ModIndex), 
		ent->myskills.weapons[WeaponIndex].mods[ModIndex].current_level);
    
    //Refresh the menu
	OpenGeneralWeaponMenu(ent, option);
}

//************************************************************************************************

void OpenGeneralWeaponMenu (edict_t *ent, int lastline)
{
	int WeaponIndex = (lastline / 100) - 10;
	int modIndex = (lastline % 100);
	int i;

	if (!ShowMenu(ent))
		return;
	clearmenu(ent);

	addlinetomenu(ent, va("%s\n(%d%c)", GetWeaponString(WeaponIndex), V_WeaponUpgradeVal(ent, WeaponIndex), '%'), MENU_GREEN_CENTERED);
	addlinetomenu(ent, va("Weapon points left: %d", ent->myskills.weapon_points), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Select the weapon", 0);
	addlinetomenu(ent, " attribute you want to", 0);
	addlinetomenu(ent, " improve upon.", 0);
	addlinetomenu(ent, " ", 0);

	for (i = 0; i < MAX_WEAPONMODS; ++i)
	{
		char sMod[30];
		int level, cur;
		level = ent->myskills.weapons[WeaponIndex].mods[i].level;
		cur = ent->myskills.weapons[WeaponIndex].mods[i].current_level;
		strcpy(sMod, GetModString(WeaponIndex, i));
		padRight(sMod, 15);
		strcat(sMod, va("%d", level));
		padRight(sMod, 18);
		addlinetomenu(ent, va("%s[%d]", sMod, cur), ((WeaponIndex+10)*100) + i + 1);
	}
		
	setmenuhandler(ent, generalWeaponMenu_handler);

	if (modIndex)
		ent->client->menustorage.currentline = modIndex + 7;
	else
		ent->client->menustorage.currentline = 14;

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Previous Menu", 7777 + WeaponIndex);
	addlinetomenu(ent, "Exit", 6666);

	showmenu(ent);
}

//************************************************************************************************
//	Main weapon upgrades menu
//************************************************************************************************

void weaponmenu_handler (edict_t *ent, int option)
{
	if ((option/100)-10 > MAX_WEAPONS)
	{
		closemenu(ent);
		return;
	}

	OpenGeneralWeaponMenu(ent, option);
}

//************************************************************************************************

void OpenWeaponUpgradeMenu (edict_t *ent, int lastline)
{
	int i;

	if (isMorphingPolt(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't upgrade weapons.\n");
		return;
	}

	if (!ShowMenu(ent))
		return;
	clearmenu(ent);

	addlinetomenu(ent, "Weapon Upgrades", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Select the weapon you", 0);
	addlinetomenu(ent, "want to upgrade:", 0);
	addlinetomenu(ent, " ", 0);

	// todo: unuglyfy
	if (ent->myskills.class_num == CLASS_PALADIN) // only add the sword
	{
		for (i = 0; i < MAX_WEAPONS; ++i)
		{
			char weaponString[24];
			strcpy(weaponString, GetWeaponString(i));
			if (!strcmp(weaponString, "Sword")) // only add the sword
			{
				padRight(weaponString, 18);
				addlinetomenu(ent, va("%s%d%c", weaponString, V_WeaponUpgradeVal(ent, i),'%'), (i+10)*100);
			}
		}
	}else
	{
		for (i = 0; i < MAX_WEAPONS; ++i)
		{
			char weaponString[24];
			strcpy(weaponString, GetWeaponString(i));
			padRight(weaponString, 18);
			addlinetomenu(ent, va("%s%d%c", weaponString, V_WeaponUpgradeVal(ent, i),'%'), (i+10)*100);

		}
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 6666);
	setmenuhandler(ent, weaponmenu_handler);
	if (lastline)
		ent->client->menustorage.currentline = lastline + 5;
	else
		ent->client->menustorage.currentline = MAX_WEAPONS + 7;
	showmenu(ent);

	// try to shortcut to chat-protect mode
	if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
		ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
}