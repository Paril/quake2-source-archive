#include "g_local.h"

#define VOTE_MAP	1
#define	VOTE_MODE	2

qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);
void check_for_levelup(edict_t *ent);
void Cmd_Armory_f(edict_t*ent, int selection);
//Function prototypes required for this .c file:
void OpenDOMJoinMenu (edict_t *ent);

void ChaseCam(edict_t *ent)
{
	if (ent->client->menustorage.menu_active)
		closemenu(ent);

	if (ent->client->chase_target) 
	{
		ent->client->chase_target = NULL;
		ent->client->ps.gunindex = 0;
		return;
	}

	GetChaseTarget(ent);
}

int ClassNum(edict_t *ent, int team); //GHz
int HighestLevelPlayer(void); //GHz

void player_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);
}

void StartGame (edict_t *ent)
{
	int		i;
	gitem_t *item=itemlist;

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.spectator = false;
	ent->client->pers.spectator = false;
	ent->client->ps.stats[STAT_SPECTATOR] = 0;
	PutClientInServer (ent);

	average_player_level = AveragePlayerLevel();
	ent->health = ent->myskills.current_health;

	if(savemethod->value == 1) // binary .vrx style saving
		for (i=0; i<game.num_items; i++, item++) // reload inventory.
			ent->client->pers.inventory[ITEM_INDEX(item)] = ent->myskills.inventory[ITEM_INDEX(item)];

	ent->client->pers.inventory[flag_index] = 0; // NO FLAG FOR YOU!!!
	ent->client->pers.inventory[red_flag_index] = 0;
	ent->client->pers.inventory[blue_flag_index] = 0;
	//4.2 remove techs
	ent->client->pers.inventory[resistance_index] = 0;
	ent->client->pers.inventory[strength_index] = 0;
	ent->client->pers.inventory[haste_index] = 0;
	ent->client->pers.inventory[regeneration_index] = 0;
	ent->touch = player_touch;
	modify_max(ent);

	Give_respawnitems(ent);
		
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	if (ent->myskills.boss > 0)
	{
		gi.bprintf(PRINT_HIGH, "A level %d boss known as %s starts their reign.\n", 
			ent->myskills.boss, ent->client->pers.netname);
	}
	else if (IsNewbieBasher(ent) && !ptr->value && !domination->value)
	{
		gi.bprintf(PRINT_HIGH, "A level %d mini-boss known as %s begins their domination.\n", 
		ent->myskills.level, ent->client->pers.netname);
		gi.bprintf(PRINT_HIGH, "Kill %s, and every non-boss gets a reward!\n", ent->client->pers.netname);
		safe_cprintf(ent, PRINT_HIGH, "You are currently a mini-boss. You will receive no point loss, but cannot war.\n");
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "%s starts their reign.\n", ent->client->pers.netname);
	}

	V_UpdatePlayerAbilities(ent);

	//Set the player's name
	strcpy(ent->myskills.player_name, ent->client->pers.netname);

	if (level.time < pregame_time->value && !trading->value) {
		gi.centerprintf(ent, "This map is currently in pre-game\nPlease warm up, upgrade and\naccess the Armory now\n");
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	}

	if (trading->value) // Red shell for trading mode.
	{
		gi.centerprintf(ent, "Welcome to trading mode\nBuy stuff and trade runes freely.\nVote for another mode to start playing.\n");

		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}
	WriteToLogfile(ent, "Logged in.\n");

#ifndef GDS_NOMULTITHREADING
	ent->ThreadStatus = 0;
#endif

	SavePlayer(ent); // Do we need to?
	V_AssignClassSkin(ent, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
}

qboolean CanJoinGame(edict_t *ent, int returned)
{
	switch(returned)
	{
	case -1:	//bad password
		safe_cprintf(ent, PRINT_HIGH, "Access denied. Incorrect password.\n");
		return false;
	case -2:	//below minimum level
		safe_cprintf(ent, PRINT_HIGH, "You have to be at least level %d to play on this server.\n", ((int)min_level->value));
		return false;
	case -3:	//above maximum level
		safe_cprintf(ent, PRINT_HIGH, "You have to be level %d or below to play here.\n", ((int)max_level->value));
		if (strcmp(reconnect_ip->string, "0") != 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "You are being sent to an alternate server where you can play.\n");
			stuffcmd(ent, va("connect %s\n", reconnect_ip->string));
		}
		return false;
	case -4:	//invalid player name
		safe_cprintf(ent, PRINT_HIGH, "Your name must be greater than 2 characters long.\n");
		return false;
	//case -5:	//playing too much
	//	safe_cprintf(ent, PRINT_HIGH, "Can't join: %d hour play-time limit reached.\n", MAX_HOURS);
	//	safe_cprintf(ent, PRINT_HIGH, "Please try a different character, or try again tommorow.\n");
	//	return false;
/*	case -6:	//newbie basher can't play
		safe_cprintf(ent, PRINT_HIGH, "Unable to join: The current maximum level is %d.\n", NEWBIE_BASHER_MAX);
		safe_cprintf(ent, PRINT_HIGH, "Please return at a later time, or try a different character.\n");
		gi.dprintf("INFO: %s exceeds maximum level allowed by server (level %d)!", ent->client->pers.netname, NEWBIE_BASHER_MAX);
		return false;
*/
	//case -7:	//boss can't play
	//	safe_cprintf(ent, PRINT_HIGH, "Unable to join: Bosses are not allowed unless the server is at least half capacity.\n");
	//	safe_cprintf(ent, PRINT_HIGH, "Please come back at a later time, or try a different character.\n");
	//	return false;
	case -8: // stupid name
		safe_cprintf(ent, PRINT_HIGH, "Unable to join with default name \"Player\" - Don't use this name. We won't be able to know who are you.\n");
		return false;
	default:	//passed every check
		return true;
	}
}

void OpenModeMenu(edict_t *ent)
{
	if (ptr->value)
	{
		OpenPTRJoinMenu(ent);
		return;
	}

	if (domination->value)
	{
		OpenDOMJoinMenu(ent);
		return;
	}

	if (ctf->value)
	{
		CTF_OpenJoinMenu(ent);
		return;
	}

	if (ent->myskills.class_num == 0)
	{
		OpenClassMenu(ent, 1); //GHz
		return;
	}

	StartGame(ent);
}

void JoinTheGame (edict_t *ent)
{
	int		returned;

	if (ent->client->menustorage.menu_active)
	{
		closemenu(ent);
		return;
	}

	if (savemethod->value != 2)
		returned = OpenConfigFile(ent);
	else
	{
		// We can't use this function.
		// We instead do it in somewhere else.
		gi.dprintf("Warning: JoinTheGame Called on MYSQL mode!\n");
		return; 
	}

	if (!CanJoinGame(ent, returned))
		return;

	OpenModeMenu(ent); // This implies the game will start.
}

qboolean StartClient(edict_t *ent)
{
	if (ent->client->pers.spectator && !level.intermissiontime) 
	{
		OpenJoinMenu(ent);
		return true;
	}
	return false;
}

void joinmenu_handler (edict_t *ent, int option)
{
	switch (option)
	{
	case 1:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("owintro.wav"), 1, ATTN_NORM, 0);
		//If no GDS is running, join the game right away.
#ifndef NO_GDS
		if(savemethod->value != 2)
#endif
			JoinTheGame(ent);
#ifndef NO_GDS
		else
		{
			gi.centerprintf(ent, "You have been queued for Loading.\n Please wait.\n");
			V_GDS_Queue_Add(ent, GDS_LOAD);
		}
#endif
		break;
	case 2: 
		ChaseCam(ent); break;
	case 3: 
		closemenu(ent); break;
	}
}

void OpenJoinMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)

	addlinetomenu(ent, va("Vortex", VRX_VERSION), MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Welcome to Vortex ", 0);
	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Start playing", 1);
	addlinetomenu(ent, "View Players", 2);
	addlinetomenu(ent, "End", 3);

	setmenuhandler(ent, joinmenu_handler);
	ent->client->menustorage.currentline = 6;
	showmenu(ent);
}

void myinfo_handler (edict_t *ent, int option)
{
	closemenu(ent);
}

void OpenMyinfoMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, va("%s (%s)", ent->client->pers.netname, 
		GetClassString(ent->myskills.class_num)), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("Level:        %d", ent->myskills.level), 0);
	addlinetomenu(ent, va("Experience:   %d", ent->myskills.experience), 0);
	addlinetomenu(ent, va("Next Level:   %d", (ent->myskills.next_level-ent->myskills.experience)+ent->myskills.nerfme), 0);
	addlinetomenu(ent, va("Credits:      %d", ent->myskills.credits), 0);
	if (ent->myskills.shots > 0)
		addlinetomenu(ent, va("Hit :  %d%c", (int)(100*((float)ent->myskills.shots_hit/ent->myskills.shots)), '%'), 0);
	else
		addlinetomenu(ent, "Hit :  --", 0);
	addlinetomenu(ent, va("Frags:        %d", ent->myskills.frags), 0);
	addlinetomenu(ent, va("Deaths:      %d", ent->myskills.fragged), 0);
	if (ent->myskills.fragged > 0)
		addlinetomenu(ent, va("Frag Percent: %d%c", (int)(100*((float)ent->myskills.frags/ent->myskills.fragged)), '%'), 0);
	else
		addlinetomenu(ent, "Frag Percent: --", 0);
	addlinetomenu(ent, va("Played Hrs:   %.1f", (float)ent->myskills.playingtime/3600), 0);
	addlinetomenu(ent, va("Respawns: %d", ent->myskills.weapon_respawns), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 1);

	setmenuhandler(ent, myinfo_handler);
		ent->client->menustorage.currentline = 13;
	showmenu(ent);
}






void respawnmenu_handler (edict_t *ent, int option)
{
	if (option == 99)
	{
		closemenu(ent);
		return;
	}

	ent->myskills.respawn_weapon = option;
}

char *GetRespawnString (edict_t *ent)
{
	switch (ent->myskills.respawn_weapon)
	{
	case 1: return "Sword";
	case 2: return "Shotgun";
	case 3: return "Super Shotgun";
	case 4: return "Machinegun";
	case 5: return "Chaingun";
	case 6: return "Grenade Launcher";
	case 7: return "Rocket Launcher";
	case 8: return "Hyperblaster";
	case 9: return "Railgun";
	case 10: return "BFG10k";
	case 11: return "Hand Grenades";
	case 12: return "20mm Cannon";
	case 13: return "Blaster";
	default: return "Unknown";
	}
}

void OpenRespawnWeapMenu(edict_t *ent)
{
	if (!ShowMenu(ent))
        return;

	if(ent->myskills.class_num == CLASS_PALADIN)
	{
		safe_cprintf(ent, PRINT_HIGH, "Archons are restricted to a sabre.\n");
		return;
	}

	if(isMorphingPolt(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't pick a respawn weapon.\n");
		return;
	}

	clearmenu(ent);

	//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Pick a respawn weapon.", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Sword", 1);
	addlinetomenu(ent, "Shotgun", 2);
	addlinetomenu(ent, "Super Shotgun", 3);
	addlinetomenu(ent, "Machinegun", 4);
	addlinetomenu(ent, "Chaingun", 5);
	addlinetomenu(ent, "Hand Grenades", 11);
	addlinetomenu(ent, "Grenade Launcher", 6);
	addlinetomenu(ent, "Rocket Launcher", 7);
	addlinetomenu(ent, "Hyperblaster", 8);
	addlinetomenu(ent, "Railgun", 9);
	addlinetomenu(ent, "BFG10k", 10);
	addlinetomenu(ent, "20mm Cannon", 12);
	addlinetomenu(ent, "Blaster", 13);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("Respawn: %s", GetRespawnString(ent)), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 99);

	setmenuhandler(ent, respawnmenu_handler);
		ent->client->menustorage.currentline = 3;
	showmenu(ent);
}

void classmenu_handler (edict_t *ent, int option)
{
	int points_needed;
	int page_num = (option / 1000);
	int page_choice = (option % 1000);
	int i;

	if ((page_num == 1) && (page_choice == 1))
	{
		OpenJoinMenu(ent);
		return;
	}
	else if (page_num > 0)
	{
		if (page_choice == 2)	//next
            OpenClassMenu (ent, page_num+1);
		else if (page_choice == 1)	//prev
			OpenClassMenu (ent, page_num-1);
		return;
	}
	//don't cause an invalid class to be created (option 99 is checked as well)
	else if ((option > CLASS_MAX) || (option < 1))
	{
		closemenu(ent);
		return;
	}

	// assign new players a team if the start level is low
	// otherwise, deny them access until the next game
	if (ctf->value && (level.time > pregame_time->value))
	{
		if (start_level->value > 1)
		{
			safe_cprintf(ent, PRINT_HIGH, "Can't join in the middle of a CTF game.\n");
			return;
		}

		// assign a random team
		ent->teamnum = GetRandom(1, 2);
	}

	for (i = 0; i < start_level->value; ++i)
	{
		points_needed = start_nextlevel->value * pow(nextlevel_mult->value, i);
		if (points_needed > 50000)
			points_needed = 50000;
		ent->myskills.experience += points_needed;
	}

	ent->myskills.class_num = option;
	AssignAbilities(ent);
	setTalents(ent);
	ent->myskills.weapon_respawns = 250;
	gi.dprintf("INFO: %s created a new %s!\n", ent->client->pers.netname, GetClassString(ent->myskills.class_num));
	WriteToLogfile(ent, va("%s created a %s.\n", ent->client->pers.netname, GetClassString(ent->myskills.class_num)));
	check_for_levelup(ent);
	modify_max(ent);
	Give_respawnweapon(ent, ent->myskills.respawn_weapon);
	Give_respawnitems(ent);
	//StartGame(ent);
	resetWeapons(ent);

	// FIXME: we should do a better job of selecting a team OR block them from joining (temp make non-spec to make savechar() happy)
	// we need to select a team if we're past pre-game time in CTF or Domination modes
	if ((ctf->value || domination->value) && (level.time > pregame_time->value))
		ent->teamnum = GetRandom(1, 2);

	StartGame(ent);
	SaveCharacter(ent);
}

void OpenClassMenu (edict_t *ent, int page_num)
{
	int i;

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Please select your", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "character class:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = ((page_num-1)*11); i < (page_num*11); ++i)
	{
		if (i < CLASS_MAX)
			addlinetomenu(ent, va("%s", GetClassString(i+1)), i+1);
		else addlinetomenu(ent, " ", 0);
	}

	addlinetomenu(ent, " ", 0);
	if (i < CLASS_MAX) addlinetomenu(ent, "Next", (page_num*1000)+2);
	addlinetomenu(ent, "Back", (page_num*1000)+1);
	addlinetomenu(ent, "Exit", 99);

	setmenuhandler(ent, classmenu_handler);
	if (CLASS_MAX > 11)
		ent->client->menustorage.currentline = 15;
	else ent->client->menustorage.currentline = 5 + i;
	showmenu(ent);
}

void masterpw_handler (edict_t *ent, int option)
{
	closemenu(ent);
}

void OpenMasterPasswordMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Master Password", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	if (strcmp(ent->myskills.email, ""))
	{
		addlinetomenu(ent, "A master password has", 0);
		addlinetomenu(ent, "already been set and can't", 0);
		addlinetomenu(ent, "be changed!", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
	}
	else
	{
		// open prompt for password
		stuffcmd(ent, "messagemode\n");

		addlinetomenu(ent, "Please enter a master", 0);
		addlinetomenu(ent, "password above. If you", 0);
		addlinetomenu(ent, "forget your normal", 0);
		addlinetomenu(ent, "password, you can use", 0);
		addlinetomenu(ent, "this one to recover your", 0);
		addlinetomenu(ent, "character.", 0);
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Close", 1);

	setmenuhandler(ent, masterpw_handler);
	ent->client->menustorage.currentline = 10;
	ent->client->menustorage.menu_index = MENU_MASTER_PASSWORD;
	showmenu(ent);
}

void generalmenu_handler (edict_t *ent, int option)
{
	switch (option)
	{
	case 1: OpenUpgradeMenu(ent); break;
	case 2: OpenWeaponUpgradeMenu(ent, 0); break;
	case 3: OpenTalentUpgradeMenu(ent, 0); break;
	case 4: OpenRespawnWeapMenu(ent); break;
	case 5: OpenMasterPasswordMenu(ent); break;
	case 6: OpenMyinfoMenu(ent); break;
	case 7: OpenArmoryMenu(ent); break;
	case 8: ShowInventoryMenu(ent, 0, false); break;
	case 9: ShowAllyMenu(ent); break;
	case 10: ShowTradeMenu(ent); break;
	case 11: ShowVoteModeMenu(ent); break;
	case 12: ShowHelpMenu(ent, 0); break;
	case 13: Cmd_Armory_f(ent, 31); break;
	default: closemenu(ent);
	}
}

void OpenGeneralMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

		//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Welcome to Vortex 5.0!", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Please choose a sub-menu.", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	ent->client->menustorage.currentline = 4;
	if (!ent->myskills.speciality_points)
		addlinetomenu(ent, "Upgrade abilities", 1);
	else
		addlinetomenu(ent, va("Upgrade abilities (%d)", ent->myskills.speciality_points), 1);
	
	if (!isMorphingPolt(ent))
	{
		if (!ent->myskills.weapon_points)
			addlinetomenu(ent, "Upgrade weapons", 2);
		else
			addlinetomenu(ent, va("Upgrade weapons (%d)", ent->myskills.weapon_points), 2);
	}
	if (!ent->myskills.talents.talentPoints)
		addlinetomenu(ent, "Upgrade talents", 3);
	else
		addlinetomenu(ent, va("Upgrade talents (%d)",ent->myskills.talents.talentPoints), 3);

	addlinetomenu(ent, " ", 0);
	if (!isMorphingPolt(ent) && 
		ent->myskills.class_num != CLASS_PALADIN)
			addlinetomenu(ent, "Set respawn weapon", 4);
	addlinetomenu(ent, "Set master password", 5);
	addlinetomenu(ent, "Show character info", 6);
	addlinetomenu(ent, "Access the armory", 7);
	addlinetomenu(ent, "Access your items", 8);
	addlinetomenu(ent, "Form alliance", 9);
	addlinetomenu(ent, "Trade items", 10);
	addlinetomenu(ent, "Vote for map/mode", 11);
	addlinetomenu(ent, "Help", 12);
#ifndef REMOVE_RESPAWNS
	if (pregame_time->value > level.time || trading->value) // we in pregame? you can buy respawns
	{
		if (ent->myskills.class_num != CLASS_PALADIN && !isMorphingPolt(ent)) // A class that needs respawns?
			addlinetomenu(ent, va("Buy Respawns (%d)", ent->myskills.weapon_respawns), 13);
	}
#endif
	setmenuhandler(ent, generalmenu_handler);
	showmenu(ent);
}

char *G_GetTruncatedIP (edict_t *player);

char *GetTeamString (edict_t *ent)
{
	if (ent->teamnum == 1)
		return "Red";
	if (ent->teamnum == 2)
		return "Blue";
	if (numAllies(ent) > 0)
		return "Allied";
	return "None";
}

char *GetStatusString (edict_t *ent)
{
	if (ent->health < 1)
		return "Dead";
	if (ent->flags & FL_CHATPROTECT)
		return "Chat Protect";
	if (ent->client->idle_frames > 300)
		return "Idle";
	return "Normal";
}
	
void OpenWhoisMenu (edict_t *ent)
{
	edict_t *player;

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	if ((player = FindPlayerByName(gi.argv(1))) == NULL)
	{
		safe_cprintf(ent, PRINT_HIGH, "Couldn't find player \"%s\".\n", gi.argv(1));
		return;
	}

	if (G_IsSpectator(player))
		return;

	addlinetomenu(ent, "Whois Information", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "", 0);

	// print name and class
	addlinetomenu(ent, va("%s (%s)", player->client->pers.netname, 
		GetClassString(player->myskills.class_num)), MENU_GREEN_CENTERED);

	// print IP address
	if (ent->myskills.administrator)
		addlinetomenu(ent, player->client->pers.current_ip, MENU_GREEN_CENTERED);
	else
		addlinetomenu(ent, G_GetTruncatedIP(player), MENU_GREEN_CENTERED);

	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, va("Admin:        %s", player->myskills.administrator?"Yes":"No"), 0);
	addlinetomenu(ent, va("Owner:        %s", player->myskills.owner), 0);
	addlinetomenu(ent, va("Status:       %s", GetStatusString(player)), 0);
	addlinetomenu(ent, va("Team:         %s", GetTeamString(player)), 0);
	addlinetomenu(ent, va("Level:        %d", player->myskills.level), 0);
	addlinetomenu(ent, va("Experience:   %d", player->myskills.experience), 0);
	addlinetomenu(ent, va("Hit Percent:  %d%c", (int)(100*((float)player->myskills.shots_hit/player->myskills.shots)), '%'), 0);
	addlinetomenu(ent, va("Frag Percent: %d%c", (int)(100*((float)player->myskills.frags/player->myskills.fragged)), '%'), 0);
	addlinetomenu(ent, va("Played Hours: %.1f", (float)player->myskills.playingtime/3600), 0);


	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 1);

	setmenuhandler(ent, masterpw_handler);//FIXME: is this safe?
		ent->client->menustorage.currentline = 16;
	showmenu(ent);
}
