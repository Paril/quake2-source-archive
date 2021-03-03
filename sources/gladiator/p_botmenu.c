//===========================================================================
//
// Name:				p_menu.c
// Function:		menu
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

#include "g_local.h"
#include "bl_spawn.h"
#include "bl_main.h"
#include "bl_redirgi.h"
#ifdef OBSERVER
#include "p_observer.h"
#endif //OBSERVER
#include "bl_botcfg.h"

#define MAX_DISPLAYEDMENUITEMS	16
#define MENUCHANGE_MOVE				1
//menu IDs
#define MID_MAIN						0
#define MID_GAME						1
#define MID_GAME_TIMELIMIT			2
#define MID_GAME_FRAGLIMIT			3
#define MID_BOT						4
#define MID_BOT_ADD					5
#define MID_BOT_ADDRANDOM			6
#define MID_BOT_REMOVE				7
#define MID_BOT_REMOVEALL			8
#define MID_BOT_ADD_FIRST			256
#define MID_BOT_ADD_LAST			511
#define MID_BOT_REMOVE_FIRST		512
#define MID_BOT_REMOVE_LAST		1024
#define MID_DM							9
#define MID_DM_TEAMPLAY				10
#define MID_DM_NO_HEALTH			11
#define MID_DM_NO_ITEMS				12
#define MID_DM_WEAPONS_STAY		13
#define MID_DM_NO_FALLING			14
#define MID_DM_INSTANT_ITEMS		15
#define MID_DM_SAME_LEVEL			16
#define MID_DM_NO_FRIENDLY_FIRE	17
#define MID_DM_SPAWN_FARTHEST		18
#define MID_DM_FORCE_RESPAWN		19
#define MID_DM_NO_ARMOR				20
#define MID_DM_ALLOW_EXIT			21
#define MID_DM_INFINITE_AMMO		22
#define MID_DM_QUAD_DROP			23
#define MID_DM_FIXED_FOV			24
#define MID_DM_QUADFIRE_DROP		25
#define MID_DM_NO_MINES				26
#define MID_DM_NO_STACK_DOUBLE	27
#define MID_DM_NO_NUKES				28
#define MID_DM_NO_SPHERES			29
#define MID_CTF						30
#define MID_CTF_BOTTEAM				31
#define MID_CTF_FORCEJOIN			32
#define MID_CTF_ARMOR_PROTECT		33
#define MID_CTF_NO_TECH				34
#define MID_RA2						35
#define MID_RA2_BOTARENA			36
#define MID_RA2_PLAYERCYCLE		37
#define MID_RA2_BOTCYCLE			38
#define MID_RA2_SELFDAMAGE			39
#define MID_RA2_HEALTHPROTECT		40
#define MID_RA2_ARMORPROTECT		41
#define MID_RA2_SHOTGUN				42
#define MID_RA2_SUPERSHOTGUN		43
#define MID_RA2_MACHINEGUN			44
#define MID_RA2_CHAINGUN			45
#define MID_RA2_GRENADELAUNCHER	46
#define MID_RA2_ROCKETLAUNCHER	47
#define MID_RA2_HYPERBLASTER		48
#define MID_RA2_RAILGUN				49
#define MID_RA2_BFG					50
#define MID_HELP						51
#define MID_CREDITS					52
#define MID_EXIT						53
#define MID_BACK						54

menu_t *mainmenu;

//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void SetupRemoveBotMenu(void)
{
	int i, n;
	edict_t *cl_ent;
	menuitem_t *menuitem;
	menu_t *removemenu, *parent = NULL;

	menuitem = GetMenuItemWithId(mainmenu, MID_BOT_REMOVE);
	if (menuitem->submenu)
	{
		parent = menuitem->submenu->parent;
		QuakeDeleteMenu(menuitem->submenu);
	} //end if
	//
	removemenu = QuakeCreateMenu(MID_BOT_REMOVE, "", "m_remove"); //"Bot remove menu"
	n = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse) continue;
		if (!(cl_ent->flags & FL_BOT)) continue;
		QuakeAppendMenu(removemenu, MI_ITEM, MID_BOT_REMOVE_FIRST + n,
			NULL, cl_ent->client->pers.netname, NULL);
		n++;
	} //end for
	//if there are no bots in the game
	if (n == 0) QuakeAppendMenu(removemenu, MI_SEPERATOR, -1, NULL, "- no bots loaded -", NULL);
	QuakeAppendMenu(removemenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(removemenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	removemenu->parent = parent;
	menuitem->submenu = removemenu;
} //end of the function SetupRemoveBotMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void SetupAddBotMenu(void)
{
	int i, n;
	edict_t *cl_ent;
	menuitem_t *menuitem;
	menu_t *addmenu, *parent = NULL;
	bot_t *bot;

	CheckForNewBotFile();
	//
	menuitem = GetMenuItemWithId(mainmenu, MID_BOT_ADD);
	if (menuitem->submenu)
	{
		parent = menuitem->submenu->parent;
		QuakeDeleteMenu(menuitem->submenu);
	} //end if
	//
	addmenu = QuakeCreateMenu(MID_BOT_ADD, "", "m_add"); //"Bot add menu"
	n = 0;
	for (bot = botlist; bot; bot = bot->next)
	{
		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse) continue;
			if (!(cl_ent->flags & FL_BOT)) continue;
			if (!strcmp(bot->name, cl_ent->client->pers.netname)) break;
		} //end for
		if (i < game.maxclients) continue;
		QuakeAppendMenu(addmenu, MI_ITEM, MID_BOT_ADD_FIRST + n, NULL, bot->name, NULL);
		n++;
	} //end for
	QuakeAppendMenu(addmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(addmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	addmenu->parent = parent;
	menuitem->submenu = addmenu;
} //end of the function SetupAddBotMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
char *OnOffString(char *name, int value)
{
	static char buf[128];

	if (value) sprintf(buf, "%-18son", name);
	else sprintf(buf, "%-18soff", name);
	return buf;
} //end of the function OnOffString
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void ToggleMenuCVarBoolean(cvar_t *cvar, char *name, int id)
{
	char buf[32];

	cvar->value = !cvar->value;
	sprintf(buf, "%d", (int)cvar->value);
	gi.cvar_set(name, buf);
	ChangeMenuItemName(mainmenu, id, OnOffString(name, (int) cvar->value));
} //end of the function ToggleMenuCVarBoolean
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void ToggleDMFlag(char *name, int flag, int flip, int id)
{
	char buf[32];

	dmflags->value = ((int) dmflags->value) ^ flag;
	sprintf(buf, "%d", (int) dmflags->value);
	gi.cvar_set("dmflags", buf);
	ChangeMenuItemName(mainmenu, id, OnOffString(name, ((((int) dmflags->value & flag) != 0) ^ flip)));
} //end of the function ToggleDMFlag
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
char *TeamPlayMenuString(void)
{
	if (((int)dmflags->value) & DF_MODELTEAMS)
		return "teamplay          by model";
	else if (((int)dmflags->value) & DF_SKINTEAMS)
		return "teamplay          by skin";
	else
		return "teamplay          disabled";
} //end of the function TeamPlayMenuString
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
char *BotCTFTeamString(void)
{
	if (botctfteam->value == 1)
		return "bot team          red";
	else if (botctfteam->value == 2)
		return "bot team          blue";
	else
		return "bot team          auto assign";
} //end of the function BotCTFTeam
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void MenuProc(edict_t *ent, int id)
{
	int i;
	bot_t *bot;
	edict_t *cl_ent;
	char buf[1024], *str;

	if (id >= MID_BOT_ADD_FIRST && id <= MID_BOT_ADD_LAST)
	{
		str = MenuItemName(mainmenu, id);
		if (!str) return;
		bot = FindBotWithName(str);
		if (!bot) return;
		BotServerCommand("sv", "addbot", bot->name, bot->skin, bot->charfile, bot->charname, NULL);
		QuakeRemoveMenuItem(mainmenu, id);
		return;
	} //end if
	if (id >= MID_BOT_REMOVE_FIRST && id <= MID_BOT_REMOVE_LAST)
	{
		str = MenuItemName(mainmenu, id);
		if (!str) return;
		BotServerCommand("sv", "removebot", str, NULL);
		QuakeRemoveMenuItem(mainmenu, id);
		return;
	} //end if
	switch(id)
	{
		case MID_BOT_ADD: //entering the add bot menu
		{
			SetupAddBotMenu();
			break;
		} //end case
		case MID_BOT_ADDRANDOM: //add a random bot
		{
			AddRandomBot(ent);
			break;
		} //end case
		case MID_BOT_REMOVE: //entering the bot remove menu
		{
			SetupRemoveBotMenu();
			break;
		} //end case
		case MID_BOT_REMOVEALL: //remove all the bots
		{
			for (i = 0; i < game.maxclients; i++)
			{
				cl_ent = g_edicts + 1 + i;
				if (!cl_ent->inuse) continue;
				if (!(cl_ent->flags & FL_BOT)) continue;
				BotServerCommand("sv", "removebot", cl_ent->client->pers.netname, NULL);
			} //end for
			break;
		} //end case
		case MID_DM_TEAMPLAY:
		{
			if (!(((int)dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
			{
				sprintf(buf, "%d", ((int) dmflags->value & ~DF_SKINTEAMS) | DF_MODELTEAMS);
			} //end if
			else if (((int)dmflags->value) & DF_MODELTEAMS)
			{
				sprintf(buf, "%d", ((int) dmflags->value & ~DF_MODELTEAMS) | DF_SKINTEAMS);
			} //end else if
			else if (((int)dmflags->value) & DF_SKINTEAMS)
			{
				sprintf(buf, "%d", (int) dmflags->value & ~(DF_MODELTEAMS | DF_SKINTEAMS));
			} //end else if
			gi.cvar_set("dmflags", buf);
			ChangeMenuItemName(mainmenu, id, TeamPlayMenuString());
			//gi.dprintf("\"dmflags\" = \"%s\"\n", dmflags->string);
			break;
		} //end case
		case MID_DM_NO_HEALTH: ToggleDMFlag("allow health", DF_NO_HEALTH, 1, id); break;
		case MID_DM_NO_ITEMS: ToggleDMFlag("allow powerups", DF_NO_ITEMS, 1, id); break;
		case MID_DM_WEAPONS_STAY: ToggleDMFlag("weapons stay", DF_WEAPONS_STAY, 0, id); break;
		case MID_DM_NO_FALLING: ToggleDMFlag("falling damage", DF_NO_FALLING, 1, id); break;
		case MID_DM_INSTANT_ITEMS: ToggleDMFlag("instant items", DF_INSTANT_ITEMS, 0, id); break;
		case MID_DM_SAME_LEVEL: ToggleDMFlag("same map", DF_SAME_LEVEL, 0, id); break;
		case MID_DM_NO_FRIENDLY_FIRE: ToggleDMFlag("friendly fire", DF_NO_FRIENDLY_FIRE, 1, id); break;
		case MID_DM_SPAWN_FARTHEST: ToggleDMFlag("spawn farthest", DF_SPAWN_FARTHEST, 0, id); break;
		case MID_DM_FORCE_RESPAWN: ToggleDMFlag("force respawn", DF_FORCE_RESPAWN, 0, id); break;
		case MID_DM_NO_ARMOR: ToggleDMFlag("allow armor", DF_NO_ARMOR, 1, id); break;
		case MID_DM_ALLOW_EXIT: ToggleDMFlag("allow exit", DF_ALLOW_EXIT, 0, id); break;
		case MID_DM_INFINITE_AMMO: ToggleDMFlag("infinite ammo", DF_INFINITE_AMMO, 0, id); break;
		case MID_DM_QUAD_DROP: ToggleDMFlag("quad drop", DF_QUAD_DROP, 0, id); break;
		case MID_DM_FIXED_FOV: ToggleDMFlag("fixed FOV", DF_FIXED_FOV, 0, id); break;
#ifdef XATRIX
		case MID_DM_QUADFIRE_DROP: ToggleDMFlag("quad fire drop", DF_QUADFIRE_DROP, 0, id); break;
#endif //XATRIX
#ifdef ROGUE
		case MID_DM_NO_MINES: ToggleDMFlag("allow mines", DF_NO_MINES, 1, id); break;
		case MID_DM_NO_STACK_DOUBLE: ToggleDMFlag("allow stack double", DF_NO_STACK_DOUBLE, 1, id); break;
		case MID_DM_NO_NUKES: ToggleDMFlag("allow nukes", DF_NO_NUKES, 1, id); break;
		case MID_DM_NO_SPHERES: ToggleDMFlag("allow spheres", DF_NO_SPHERES, 1, id); break;
#endif //ROGUE
		case MID_CTF_BOTTEAM:
		{
			if (botctfteam->value == 1) gi.cvar_set("botctfteam", "2");
			else if (botctfteam->value == 2) gi.cvar_set("botctfteam", "0");
			else gi.cvar_set("botctfteam", "1");
			ChangeMenuItemName(mainmenu, id, BotCTFTeamString());
			break;
		} //end case
		case MID_CTF_FORCEJOIN: ToggleDMFlag("force join", DF_CTF_FORCEJOIN, 0, id); break;
		case MID_CTF_ARMOR_PROTECT: ToggleDMFlag("armor protect", DF_ARMOR_PROTECT, 0, id); break;
		case MID_CTF_NO_TECH: ToggleDMFlag("allow techs", DF_CTF_NO_TECH, 1, id); break;

		case MID_RA2_BOTARENA:
		{
			if (arena->value < 1) arena->value = 1;
			else arena->value = floor(arena->value) + 1;
			if (arena->value > RA2_NumArenas()) arena->value = 1;
			sprintf(buf, "%d", (int) arena->value);
			gi.cvar_set("arena", buf);
			sprintf(buf, "%-18s%d", "bot arena", (int) arena->value);
			ChangeMenuItemName(mainmenu, MID_RA2_BOTARENA, buf);
			break;
		} //end case
		case MID_RA2_PLAYERCYCLE: ToggleMenuCVarBoolean(gi.cvar("ra_playercycle", "", 0), "player cycle", id); break;
		case MID_RA2_BOTCYCLE: ToggleMenuCVarBoolean(gi.cvar("ra_botcycle", "", 0), "bot cycle", id); break;
		case MID_RA2_SELFDAMAGE: ToggleMenuCVarBoolean(gi.cvar("selfdamage", "", 0), "self damage", id); break;
		case MID_RA2_HEALTHPROTECT: ToggleMenuCVarBoolean(gi.cvar("healthprotect", "", 0), "health protect", id); break;
		case MID_RA2_ARMORPROTECT: ToggleMenuCVarBoolean(gi.cvar("armorprotect", "", 0), "armor protect", id); break;
		case MID_RA2_SHOTGUN: ToggleMenuCVarBoolean(gi.cvar("shotgun", "", 0), "Shotgun", id); break;
		case MID_RA2_SUPERSHOTGUN: ToggleMenuCVarBoolean(gi.cvar("supershotgun", "", 0), "Super Shotgun", id); break;
		case MID_RA2_MACHINEGUN: ToggleMenuCVarBoolean(gi.cvar("machinegun", "", 0), "Machinegun", id); break;
		case MID_RA2_CHAINGUN: ToggleMenuCVarBoolean(gi.cvar("chaingun", "", 0), "Chaingun", id); break;
		case MID_RA2_GRENADELAUNCHER: ToggleMenuCVarBoolean(gi.cvar("grenadelauncher", "", 0), "Grenade Launcher", id); break;
		case MID_RA2_ROCKETLAUNCHER: ToggleMenuCVarBoolean(gi.cvar("rocketlauncher", "", 0), "Rocket Launcher", id); break;
		case MID_RA2_HYPERBLASTER: ToggleMenuCVarBoolean(gi.cvar("hyperblaster", "", 0), "Hyperblaster", id); break;
		case MID_RA2_RAILGUN: ToggleMenuCVarBoolean(gi.cvar("railgun", "", 0), "Railgun", id); break;
		case MID_RA2_BFG: ToggleMenuCVarBoolean(gi.cvar("bfg", "", 0), "BFG10K", id); break;
		case MID_BACK: //back to the parent menu
		{
			MenuBack(ent);
			break;
		} //end case
		case MID_EXIT:
		{
			ToggleBotMenu(ent);
			return;
		} //end case
	} //end switch
} //end of the function MenuProc
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void OpenBotMenu(edict_t *ent)
{
	menustate_t *menustate;

	menustate = &ent->client->menustate;
	menustate->showmenu = true;
	ent->client->showmenu = true;
	ent->client->showscores = true;
	//
	menustate->mainmenu = mainmenu;
	menustate->menuid = MID_MAIN;
	menustate->highlighteditem = 0;
	menustate->menuproc = MenuProc;
	menustate->redrawmenu = true;
	menustate->removemenu = false;
} //end of the function OpenBotMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void CloseBotMenu(edict_t *ent)
{
	menustate_t *menustate;

	menustate = &ent->client->menustate;
	menustate->showmenu = false;
	ent->client->showmenu = false;
	ent->client->showscores = false;

	menustate->redrawmenu = false;
	menustate->removemenu = true;
} //end of the function CloseBotMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void ToggleBotMenu(edict_t *ent)
{
	menustate_t *menustate;
	cvar_t *rcon_password;

	if (!ent)
	{
		gi.dprintf("only clients can open the menu\n");
		return;
	} //end if
	menustate = &ent->client->menustate;
	if (menustate->showmenu)
	{
		CloseBotMenu(ent);
	} //end if
	else
	{
		if ((gi.cvar("serveronlybotcmds", "0", 0))->value)
		{
			rcon_password = gi.cvar("rcon_password", "", 0);
			if (	strcmp(rcon_password->string, "none") &&
					(gi.argc() <= 1 || strcmp(rcon_password->string, gi.argv(1)))
					)
			{
				gi.cprintf(ent, PRINT_HIGH, "need rcon password to open the menu\n");
				return;
			} //end if
		} //end if
		OpenBotMenu(ent);
	} //end else
} //end of the function ToggleBotMenu
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void CreateBotMenu(void)
{
	menu_t *helpmenu, *botmenu, *addmenu, *removemenu, *dmmenu, *ra2menu, *ctfmenu;
	menu_t *creditsmenu;
	char buf[128];

	helpmenu = QuakeCreateMenu(MID_HELP, "", "m_help"); //"Menu help"
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "Use your forward and", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "backward keys (usually ", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "the arrow keys) to move", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "the cursor,", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "your inventory key", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "(usually Enter) to", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "select,", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "Esc to exit the menu", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(helpmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(helpmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//
	creditsmenu = QuakeCreateMenu(MID_CREDITS, "", "m_credits"); //"Credits"
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Allan (Strider) Kivlin", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Philip Niewold", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Info-Zip Team", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Adrian (Mr Pink) Finol", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Dominic (Cube) Rutter", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Matt (Genocyde) Freitas", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Timm Stokke", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Rhea", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "Raven", NULL);
	QuakeAppendMenu(creditsmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(creditsmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//
	addmenu = QuakeCreateMenu(MID_BOT_ADD, "", "m_add"); //"Bot add menu"
	removemenu = QuakeCreateMenu(MID_BOT_REMOVE, "", "m_remove"); //"Bot remove menu"
	//
 	botmenu = QuakeCreateMenu(MID_BOT, "", "m_bots"); //"Bot menu"
	QuakeAppendMenu(botmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(botmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(botmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(botmenu, MI_SUBMENU, MID_BOT_ADD, addmenu, "add bot", NULL);
	QuakeAppendMenu(botmenu, MI_ITEM, MID_BOT_ADDRANDOM, NULL, "add random", NULL);
	QuakeAppendMenu(botmenu, MI_SUBMENU, MID_BOT_REMOVE, removemenu, "remove bot", NULL);
	QuakeAppendMenu(botmenu, MI_ITEM, MID_BOT_REMOVEALL, NULL, "remove all", NULL);
	QuakeAppendMenu(botmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(botmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//Deathmatch
	dmmenu = QuakeCreateMenu(MID_DM, "", "m_dm");
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_TEAMPLAY, NULL, TeamPlayMenuString(), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_HEALTH, NULL, OnOffString("allow health", !((int) dmflags->value & DF_NO_HEALTH)), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_ITEMS, NULL, OnOffString("allow powerups", !((int) dmflags->value & DF_NO_ITEMS)), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_ARMOR, NULL, OnOffString("allow armor", !((int) dmflags->value & DF_NO_ARMOR)), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_WEAPONS_STAY, NULL, OnOffString("weapons stay", (int) dmflags->value & DF_WEAPONS_STAY), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_FALLING, NULL, OnOffString("falling damage", !((int) dmflags->value & DF_NO_FALLING)), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_INSTANT_ITEMS, NULL, OnOffString("instant items", (int) dmflags->value & DF_INSTANT_ITEMS), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_SAME_LEVEL, NULL, OnOffString("same map", (int) dmflags->value & DF_SAME_LEVEL), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_FRIENDLY_FIRE, NULL, OnOffString("friendly fire", !((int) dmflags->value & DF_NO_FRIENDLY_FIRE)), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_SPAWN_FARTHEST, NULL, OnOffString("spawn farthest", (int) dmflags->value & DF_SPAWN_FARTHEST), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_FORCE_RESPAWN, NULL, OnOffString("force respawn", (int) dmflags->value & DF_FORCE_RESPAWN), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_ALLOW_EXIT, NULL, OnOffString("allow exit", (int) dmflags->value & DF_ALLOW_EXIT), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_INFINITE_AMMO, NULL, OnOffString("infinite ammo", (int) dmflags->value & DF_INFINITE_AMMO), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_QUAD_DROP, NULL, OnOffString("quad drop", (int) dmflags->value & DF_QUAD_DROP), NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_FIXED_FOV, NULL, OnOffString("fixed FOV", (int) dmflags->value & DF_FIXED_FOV), NULL);
	//Xatrix mission pack 1
	if (xatrix->value)
	{
		QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_QUADFIRE_DROP, NULL, OnOffString("quad fire drop", (int) dmflags->value & DF_QUADFIRE_DROP), NULL);
	} //end if
	//Rogue mission pack 2
	if (rogue->value)
	{
		QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_MINES, NULL, OnOffString("allow mines", !((int) dmflags->value & DF_NO_MINES)), NULL);
		QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_STACK_DOUBLE, NULL, OnOffString("allow stack double", !((int) dmflags->value & DF_NO_STACK_DOUBLE)), NULL);
		QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_NUKES, NULL, OnOffString("allow nukes", !((int) dmflags->value & DF_NO_NUKES)), NULL);
		QuakeAppendMenu(dmmenu, MI_ITEM, MID_DM_NO_SPHERES, NULL, OnOffString("allow spheres", !((int) dmflags->value & DF_NO_SPHERES)), NULL);
	} //end if
	QuakeAppendMenu(dmmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(dmmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//Capture The Flag
	ctfmenu = QuakeCreateMenu(MID_CTF, "", "m_ctf");
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_CTF_BOTTEAM, NULL, BotCTFTeamString(), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_CTF_FORCEJOIN, NULL, OnOffString("force join", (int) dmflags->value & DF_CTF_FORCEJOIN), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_CTF_ARMOR_PROTECT, NULL, OnOffString("armor protect", (int) dmflags->value & DF_ARMOR_PROTECT), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_CTF_NO_TECH, NULL, OnOffString("allow techs", !((int) dmflags->value & DF_CTF_NO_TECH)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_NO_HEALTH, NULL, OnOffString("allow health", !((int) dmflags->value & DF_NO_HEALTH)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_NO_ITEMS, NULL, OnOffString("allow powerups", !((int) dmflags->value & DF_NO_ITEMS)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_NO_ARMOR, NULL, OnOffString("allow armor", !((int) dmflags->value & DF_NO_ARMOR)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_WEAPONS_STAY, NULL, OnOffString("weapons stay", (int) dmflags->value & DF_WEAPONS_STAY), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_NO_FALLING, NULL, OnOffString("falling damage", !((int) dmflags->value & DF_NO_FALLING)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_INSTANT_ITEMS, NULL, OnOffString("instant items", (int) dmflags->value & DF_INSTANT_ITEMS), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_SAME_LEVEL, NULL, OnOffString("same map", (int) dmflags->value & DF_SAME_LEVEL), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_NO_FRIENDLY_FIRE, NULL, OnOffString("friendly fire", !((int) dmflags->value & DF_NO_FRIENDLY_FIRE)), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_SPAWN_FARTHEST, NULL, OnOffString("spawn farthest", (int) dmflags->value & DF_SPAWN_FARTHEST), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_FORCE_RESPAWN, NULL, OnOffString("force respawn", (int) dmflags->value & DF_FORCE_RESPAWN), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_ALLOW_EXIT, NULL, OnOffString("allow exit", (int) dmflags->value & DF_ALLOW_EXIT), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_INFINITE_AMMO, NULL, OnOffString("infinite ammo", (int) dmflags->value & DF_INFINITE_AMMO), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_QUAD_DROP, NULL, OnOffString("quad drop", (int) dmflags->value & DF_QUAD_DROP), NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_DM_FIXED_FOV, NULL, OnOffString("fixed FOV", (int) dmflags->value & DF_FIXED_FOV), NULL);
	QuakeAppendMenu(ctfmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(ctfmenu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//Rocket Arena 2
	ra2menu = QuakeCreateMenu(MID_RA2, "", "m_ra2");
	sprintf(buf, "%-18s%d", "bot arena", (int) arena->value);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_BOTARENA, NULL, buf, NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_DM_TEAMPLAY, NULL, TeamPlayMenuString(), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_PLAYERCYCLE, NULL, OnOffString("player cycle", (int) (gi.cvar("ra_playercycle", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_BOTCYCLE, NULL, OnOffString("bot cycle", (int) (gi.cvar("ra_botcycle", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_SELFDAMAGE, NULL, OnOffString("self damage", (int) (gi.cvar("selfdamage", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_HEALTHPROTECT, NULL, OnOffString("health protect", (int) (gi.cvar("healthprotect", "0", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_ARMORPROTECT, NULL, OnOffString("armor protect", (int) (gi.cvar("armorprotect", "0", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_SHOTGUN, NULL, OnOffString("Shotgun", (int) (gi.cvar("shotgun", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_SUPERSHOTGUN, NULL, OnOffString("Super Shotgun", (int) (gi.cvar("supershotgun", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_MACHINEGUN, NULL, OnOffString("Machinegun", (int) (gi.cvar("machinegun", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_CHAINGUN, NULL, OnOffString("Chaingun", (int) (gi.cvar("chaingun", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_GRENADELAUNCHER, NULL, OnOffString("Grenade Launcher", (int) (gi.cvar("grenadelauncher", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_ROCKETLAUNCHER, NULL, OnOffString("Rocket Launcher", (int) (gi.cvar("rocketlauncher", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_HYPERBLASTER, NULL, OnOffString("Hyperblaster", (int) (gi.cvar("hyperblaster", "1", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_RAILGUN, NULL, OnOffString("Railgun", (int) (gi.cvar("railgun", "0", 0))->value), NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_RA2_BFG, NULL, OnOffString("BFG", (int) (gi.cvar("bfg", "0", 0))->value), NULL);
	/*
	"armor", "200", CVAR_SERVERINFO);
	"health", "100", CVAR_SERVERINFO);
	"shells", "100", CVAR_SERVERINFO);
	"bullets", "200" ,CVAR_SERVERINFO);
	"slugs", "50", CVAR_SERVERINFO);
	"grenades", "50", CVAR_SERVERINFO);
	"rockets", "50", CVAR_SERVERINFO);
	"cells", "150", CVAR_SERVERINFO);
	*/
	QuakeAppendMenu(ra2menu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(ra2menu, MI_ITEM, MID_BACK, NULL, "back", NULL);
	//
	mainmenu = QuakeCreateMenu(MID_MAIN, "", "m_main"); //"Main menu"
	QuakeAppendMenu(mainmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(mainmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(mainmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(mainmenu, MI_SEPERATOR, -1, NULL, "", NULL);
	QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_BOT, botmenu, "Bots", NULL);
	//
	if (!ctf->value && !ra->value) QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_DM, dmmenu, "DM", NULL);
	if (ctf->value) QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_CTF, ctfmenu, "CTF", NULL);
	if (ra->value) QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_RA2, ra2menu, "RA2", NULL);
	//
	QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_HELP, helpmenu, "Help", NULL);
	QuakeAppendMenu(mainmenu, MI_SUBMENU, MID_HELP, creditsmenu, "Credits", NULL);
	QuakeAppendMenu(mainmenu, MI_SEPERATOR, -1, NULL, "-----------", NULL);
	QuakeAppendMenu(mainmenu, MI_ITEM, MID_EXIT, NULL, "Exit", NULL);
} //end of the function CreateBotMenu
