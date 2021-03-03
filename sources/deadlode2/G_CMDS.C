#include "g_local.h"

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	DL_strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"), -1);
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

void SelectNextItem (edict_t *ent, int itflags){
	if (ent->client->menu[ent->client->curmenu])
		PMenu_Next(ent);
}

void SelectPrevItem (edict_t *ent, int itflags) {
	if (ent->client->menu[ent->client->curmenu])
		PMenu_Prev(ent);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}

void Cmd_CloseMenu_f (edict_t *ent) {
	PMenu_Close (ent, true);
}

/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	item_t		*useMe;
	int			index=0, i, num=0;
	pmenuhnd_t	*hnd;
	pmenu_t		*p;
	edict_t		*g=NULL;
	char		*s;
	vec3_t		start;

	ent->client->showscores = false;
	ent->client->showhelp = false;

	s = gi.args();
	if (!s[0]) {
		// Don't let observers use items
		if (!(ent->movetype != MOVETYPE_NOCLIP && ent->deadflag != DEAD_DEAD))
		return;

		// If viewing a camera - Don't
		if (ent->client->dummy) {
			Camera_toggle(ent);
			return;
		}

		// look for nearby objects that can be used
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		g = FindInFOV(start, ent->client->v_angle, 90, 64);
		// Is it a usable buildign?
		if (g && g->building && g->use) {
			// Release the previously used item
			if (ent->use_this)
				ent->use_this->use_this = NULL;
			ent->use_this = g;
			g->use(g, ent, ent);
			return;
		}

		// What about a trigger we can reclaim?
		if (g && g->supply && (g->supply->type & (SUP_TIMER|SUP_TRIPWIRE|SUP_PROXY|SUP_VIDSENSOR)) ) {
			ReclaimTrigger(ent, g);
			return;
		}
		return;
	}

	// Check for hotkeys
	if (!DL_strcmp(s, "Blaster", -1, false))
		index = 1;
	else if (!DL_strcmp(s, "Shotgun", -1, false))
		index = 2;
	else if (!DL_strcmp(s, "Super Shotgun", -1, false))
		index = 3;
	else if (!DL_strcmp(s, "Machinegun", -1, false))
		index = 4;
	else if (!DL_strcmp(s, "Chaingun", -1, false))
		index = 5;
	else if (!DL_strcmp(s, "Grenade Launcher", -1, false))
		index = 6;
	else if (!DL_strcmp(s, "Rocket Launcher", -1, false))
		index = 7;
	else if (!DL_strcmp(s, "Hyperblaster", -1, false))
		index = 8;
	else if (!DL_strcmp(s, "Railgun", -1, false))
		index = 9;
	else if (!DL_strcmp(s, "BFG10K", -1, false))
		index = 10;

	if (index)
	{
		hnd = ent->client->menu[ent->client->curmenu];
		if (ent->client->prefs.inmenu && hnd)
		{		// menu hotkeys
			for (i=hnd->oldstart, p = hnd->entries+hnd->oldstart; i < min(hnd->oldstart+16, hnd->num); i++, p++)
			{
				if (!p->SelectFunc && !p->target)
					continue;

				num++;
				if (num == index)
				{
					hnd->cur = i;
					PMenu_Select(ent);
					return;
				}
			}
			gi.cprintf(ent, PRINT_HIGH, "Unknown menu item: %d\n", index);
			return;
		}
	}

	// Don't let observers use items
	if (!(ent->movetype != MOVETYPE_NOCLIP && ent->deadflag != DEAD_DEAD))
	return;

	useMe = FindItemByName(ent, s, ITEM_ANY);
	if (useMe) {
		if (useMe->itemtype == ITEM_WEAPON) {
			ent->client->nextweap = useMe;
			return;
		} else if (useMe->itemtype == ITEM_AMMO) {
			ent->client->nextammo = ITEMAMMO(useMe);
			return;
		} else if (useMe->itemtype == ITEM_SUPPLY) {
			UseSupply(ent, s);
			return;
		}
	}

	useMe = FindItemByAltCmd(ent, s, ITEM_ANY);
	if (useMe) {
		if (useMe->itemtype == ITEM_WEAPON) {
			ent->client->nextweap = useMe;
			return;
		} else if (useMe->itemtype == ITEM_AMMO) {
			ent->client->nextammo = ITEMAMMO(useMe);
			return;
		} else if (useMe->itemtype == ITEM_SUPPLY) {
			UseSupply(ent, GetItemName(useMe));
			return;
		}
	}
	gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
}


/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
//ZOID
	if (ent->client->menu[ent->client->curmenu]) {
		PMenu_Select(ent);
		return;
	}
//ZOID

// DEADLODE
	Cmd_Use_f(ent);
// DEADLODE
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent) {
item_t	*weap;

	if (!ent->client->nextweap)
		ent->client->nextweap = ent->client->curweap;

	weap = FindPrevWeapon(ent, ent->client->nextweap);
	if (weap)
		ent->client->nextweap = weap;
	else if (weap = FindPrevWeapon(ent, NULL))
		ent->client->nextweap = weap;
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent) {
item_t	*weap;

	if (!ent->client->nextweap)
		ent->client->nextweap = ent->client->curweap;

	weap = FindNextWeapon(ent, ent->client->nextweap);
	if (weap)
		ent->client->nextweap = weap;
	else if (weap = FindNextWeapon(ent, NULL))
		ent->client->nextweap = weap;

}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID

	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
//	respawn (ent);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu[ent->client->curmenu])
		PMenu_CloseAll(ent);
//ZOID
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

void Cmd_Team_f (edict_t *ent)
{
	char *t, *s;
	team_t *desired_team = NULL;
	int i;

	t = gi.args();
	if (!*t) {
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n", ent->client->resp.team->team_name);
		return;
	}

	for (i = 0; i < MAX_TEAMS; i++)
		if (!DL_strcmp(t, dlsys.teams[i].team_name, -1, false))
			desired_team = &dlsys.teams[i];

	if (!desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "Unknown team name: %s\n", t);
		return;
	}

	if (ent->client->resp.team == desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n", desired_team->team_name);
		return;
	}

	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
// DEADLODE
	if (ent->client->resp.team)
		ent->client->resp.team->players--;

	// Players lose thier buildings when switchign teams...
	ReleaseBuildings(ent);

	ent->team = ent->client->resp.team = desired_team;
	ent->client->resp.team->players++;
// DEADLODE

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	DL_AssignSkin(ent, s);

	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, ent->client->resp.team->team_name);
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);

	ent->client->resp.score = 0;

	gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n", ent->client->pers.netname, ent->client->resp.team->team_name);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent) {
	// Dl2 doesn't support waves
	return;
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];

	if (gi.argc () < 2 && !arg0)
		return;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team) {
			if (ent->client->resp.team != other->client->resp.team)
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_Menu_f (edict_t *ent) {
edict_t	*g = NULL;
char	*menu;
vec3_t	start;
	
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->oldmenu[0] = 0;

	if (gi.argc() < 2) {
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		g = FindInFOV(start, ent->client->v_angle, 90, 64);
		if (g && g->building && g->team == ent->client->resp.team) {
			PMenu_Open(ent, ID_BUILDINGINFO, NULL, g);
			return;
		}
		if (g && g->supply && g->team == ent->client->resp.team)
		{
			PMenu_Open(ent, ID_TRIGGERINFO, NULL, g);
			return;
		}

		if (ent->client->menu[ent->client->curmenu]) {
			PMenu_CloseAll (ent);
		} else {
			PMenu_CloseAll (ent);
			PMenu_Open(ent, ID_MAIN, NULL, NULL);
		}
		return;
	}
	
	menu = gi.argv(1);
	if (DL_strcmp(menu, "on", -1, false) == 0) {
		gi.cprintf (ent, PRINT_HIGH, "Menus enabled.\n");
		ent->client->prefs.inmenu = true;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "off", -1, false) == 0) {
		gi.cprintf (ent, PRINT_HIGH, "Menus disabled.\n");
		ent->client->prefs.inmenu = false;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "alt", -1, false) == 0) {
		PMenu_AltSelect(ent);
		return;
	} else if (DL_strcmp(menu, "trans", -1, false) == 0) {
		ent->client->prefs.no_menu_background = !(ent->client->prefs.no_menu_background);
		if (ent->client->prefs.no_menu_background)
			gi.cprintf (ent, PRINT_HIGH, "Menus Background: OFF.\n");
		else
			gi.cprintf (ent, PRINT_HIGH, "Menus Background: ON.\n");

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "topleft", -1, false) == 0)	{
		ent->client->prefs.menu_pos = MENU_POS_TOP|MENU_POS_LEFT;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if ((DL_strcmp(menu, "top", -1, false) == 0) || (DL_strcmp(menu, "topcenter", -1, false) == 0))	{
		ent->client->prefs.menu_pos = MENU_POS_TOP;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "topright", -1, false) == 0)	{
		ent->client->prefs.menu_pos = MENU_POS_TOP|MENU_POS_RIGHT;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "left", -1, false) == 0)	{
		ent->client->prefs.menu_pos = MENU_POS_LEFT;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "center", -1, false) == 0)	{
		ent->client->prefs.menu_pos = 0;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	} else if (DL_strcmp(menu, "right", -1, false) == 0)	{
		ent->client->prefs.menu_pos = MENU_POS_RIGHT;

		if (ent->client->menu[ent->client->curmenu])
			PMenu_Update (ent);
		return;
	}
	if (DL_strcmp(menu, "team", -1, false) == 0) {
		PMenu_CloseAll (ent);
		PMenu_Open(ent, ID_TEAMOBJECTS, 0, NULL);
	} else if (DL_strcmp(menu, "main", -1, false) == 0) {
		PMenu_CloseAll (ent);
		PMenu_Open(ent, ID_MAIN, 0, NULL);
	} else if ((DL_strcmp(menu, "shop", -1, false) == 0) || (DL_strcmp(menu, "armoury", -1, false) == 0)) {
		PMenu_CloseAll (ent);
		PMenu_Open(ent, ID_SHOP, 0, NULL);
	} else if (DL_strcmp(menu, "close", -1, false) == 0)
		PMenu_Close (ent, true);
	else if (DL_strcmp(menu, "closeall", -1, false) == 0)
		PMenu_CloseAll (ent);
	else
		gi.cprintf (ent, PRINT_HIGH, va("Unknown menu: %s\n", menu));
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client || !ent->inuse)
		return;		// not fully in game yet

	cmd = gi.argv(0);

// Intermission Commands
	if (DL_strcmp (cmd, "players", -1, false) == 0) {
		Cmd_Players_f (ent);
		return;
	} else if (DL_strcmp (cmd, "say", -1, false) == 0) {
		Cmd_Say_f (ent, false, false);
		return;
	} else if ((DL_strcmp (cmd, "say_team", -1, false) == 0) || (DL_strcmp (cmd, "steam", -1, false) == 0)) {
		Radio_Chat(ent, gi.args());
		return;
	} else if (DL_strcmp (cmd, "score", -1, false) == 0) {
		Cmd_Score_f (ent);
		return;
	} else if (DL_strcmp (cmd, "help", -1, false) == 0) {
		Cmd_Help_f (ent);
		return;
	}
// Menu Stuff
	if (DL_strcmp (cmd, "use", -1, false) == 0) {
		Cmd_Use_f (ent);
		return;
	} else if (DL_strcmp (cmd, "inven", -1, false) == 0) {
		Cmd_Menu_f (ent);
		return;
	} else if (DL_strcmp (cmd, "invnext", -1, false) == 0) {
		SelectNextItem (ent, -1);
		return;
	} else if (DL_strcmp (cmd, "invprev", -1, false) == 0) {
		SelectPrevItem (ent, -1); 
		return;
	} else if (DL_strcmp (cmd, "invuse", -1, false) == 0) {
		Cmd_InvUse_f (ent);
		return;
	} else if (DL_strcmp (cmd, "putaway", -1, false) == 0) {
		Cmd_PutAway_f (ent);
		return;
	} else if (DL_strcmp (cmd, "invdrop", -1, false) == 0) {
		Cmd_CloseMenu_f (ent);
		return;
	} else if (DL_strcmp (cmd, "menu", -1, false) == 0) {
		Cmd_Menu_f (ent);
		return;
	}

// Intermission Commands
	if (level.intermissiontime)
		return;

// Menu Stuff
	if (DL_strcmp (cmd, "team", -1, false) == 0) {
		Cmd_Team_f (ent);
		return;
// These commands will work at any time
	} else if (DL_strcmp(cmd, "additem", -1, false) == 0) {
		DL_Cmd_AddItem(ent);
		return;
	} else if (DL_strcmp(cmd, "dropitem", -1, false) == 0) {
		DL_Cmd_DropItem(ent);
		return;
	} else if (DL_strcmp(cmd, "altcmd", -1, false) == 0) {
		DL_Cmd_SetAltCmd(ent);
		return;
	} else if (DL_strcmp(cmd, "profile", -1, false) == 0) {
		if (DL_strcmp(gi.argv(1), "clear", -1, false) == 0)
			DL_Cmd_ClearProfile(ent);
		else if (DL_strcmp(gi.argv(1), "show", -1, false) == 0)
			DL_Cmd_ShowProfile(ent);
		else {
			if (ent->client->resp.profile[0])
				gi.cprintf(ent, PRINT_HIGH, "Current profile: %s\n", ent->client->resp.profile);
			else
				gi.cprintf(ent, PRINT_HIGH, "Current profile: Player Profile\n");
		}
		return;
	} else if (DL_strcmp(cmd, "spectate", -1, false) == 0) {
		ent->team = ent->client->resp.team = NULL;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
	}


	// CHEATS
	if (DL_strcmp (cmd, "give", -1, false) == 0)
		Cmd_Give_f (ent);
	else if (DL_strcmp (cmd, "god", -1, false) == 0)
		Cmd_God_f (ent);
	else if (DL_strcmp (cmd, "notarget", -1, false) == 0)
		Cmd_Notarget_f (ent);
	else if (DL_strcmp (cmd, "noclip", -1, false) == 0)
		Cmd_Noclip_f (ent);
	// CHEATS

	// Dead/Observer Commands
	if ((ent->movetype == MOVETYPE_NOCLIP) || (ent->deadflag == DEAD_DEAD)) {
		// Ignore these commands while dead/observing
		if (DL_strcmp(cmd, "ammoprev", -1, false) == 0);
		else if (DL_strcmp(cmd, "ammonext", -1, false) == 0);
		else if (DL_strcmp(cmd, "weapprev", -1, false) == 0);
		else if (DL_strcmp(cmd, "weapnext", -1, false) == 0);
		else if (DL_strcmp(cmd, "reload", -1, false) == 0);
		else if (DL_strcmp(cmd, "throw", -1, false) == 0);
		else if (DL_strcmp(cmd, "radio", -1, false) == 0);
		else if (DL_strcmp(cmd, "tuneradio", -1, false) == 0);
		else if (DL_strcmp(cmd, "voice", -1, false) == 0);
		// anything that doesn't match a command will be a chat
		else Cmd_Say_f (ent, false, true);
		return;
	}
	// Dead/Observer Commands

	// Old Q2 Commands
	if (DL_strcmp (cmd, "weapprev", -1, false) == 0)
		Cmd_WeapPrev_f (ent);
	else if (DL_strcmp (cmd, "weapnext", -1, false) == 0)
		Cmd_WeapNext_f (ent);
	else if (DL_strcmp (cmd, "kill", -1, false) == 0)
		Cmd_Kill_f (ent);
	else if (DL_strcmp (cmd, "wave", -1, false) == 0)
		Cmd_Wave_f (ent);
	// Old Q2 Commands

// DEADLODE
	else if (DL_strcmp(cmd, "radio", -1, false) == 0)
		Radio_Use(ent, gi.args());
	else if (DL_strcmp(cmd, "tuneradio", -1, false) == 0)
		Radio_Tune(ent, atoi(gi.argv(1)));
	else if (DL_strcmp(cmd, "voice", -1, false) == 0)
		Voice_Use(ent, gi.argv(1));
	else if (DL_strcmp(cmd, "build", -1, false) == 0)
		Building_build(ent, GetBuildingByName(gi.args()));
	else if (DL_strcmp(cmd, "ammoprev", -1, false) == 0) {
		if (ent->client->curweap) 
			ent->client->nextammo = FindPrevCompatibleAmmo(ent, ent->client->curweap->cliptype);
	} else if (DL_strcmp(cmd, "ammonext", -1, false) == 0) {
		if (ent->client->curweap)
			ent->client->nextammo = FindNextCompatibleAmmo(ent, ent->client->curweap->cliptype);
	} else if (DL_strcmp(cmd, "reload", -1, false) == 0) {
		if (ent->client->weaponstate == WEAPON_READY) 
			ent->client->weaponstate = WEAPON_RELOADING;
		else if (ent->client->curweap) {
			if (!ITEMWEAP(ent->client->curweap)->frame_reload_num) {
				ent->client->ps.gunframe = ITEMWEAP(ent->client->curweap)->frame_raise;
				ent->client->weaponstate = WEAPON_ACTIVATING;
			} else {
				ent->client->ps.gunframe = ITEMWEAP(ent->client->curweap)->frame_idle;
				ent->client->weaponstate = WEAPON_READY;
			}
			ent->client->wreload_time = 0.0;
			ent->client->burstcnt = 0;
		}
	}
	else if (DL_strcmp(cmd, "throw", -1, false) == 0)
		Grenade_Use(ent, NULL);
	else if (DL_strcmp(cmd, "nextcam", -1, false) == 0)
		ent->client->camera = Camera_next (ent);
	else if (DL_strcmp(cmd, "viewcam", -1, false) == 0)
		Camera_toggle(ent);

	else if (DL_strcmp(cmd, "testsen", -1, false) == 0)
		ent->show_hostile = !ent->show_hostile;

	else if (DL_strcmp(cmd, "dropgoal", -1, false) == 0)
		DropGoalItem(ent);

	else if (DL_strcmp(cmd, "time", -1, false) == 0)
		gi.configstring(CS_LIGHTS+12, gi.argv(1));
// DEADLODE

	// anything that doesn't match a command will be a chat
	else Cmd_Say_f (ent, false, true);
}
