#include "g_local.h"
#include "m_player.h"
#include "z_gq.h"

/*//Eraser
#include "bot_procs.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "p_trail.h"
// Eraser end*/

void spectator_respawn (edict_t *ent);
void botRemovePlayer(edict_t *self);

char *ClientTeam (edict_t *ent)		// Not used! -- Stone
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
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

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
//	debugmsg("Running OnSameTeam\n");
// Modified by Stone
	if (!ent1->client || !ent2->client)	// Non-players can't be on a team
		return false;
//	debugmsg("Both clients\n");
	if ((teamplay->value || ((int)playmode->value == PM_BADGE_WARS))
		&& (ent1->client->team == ent2->client->team) 
		&& (ent1->client->team>0)) {
//		debugmsg("On same team!\n");
		return true;
	}
	return false;
/*	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
*/
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// See if the menu is visible, and if it is, handle it accordingly.
	
	if (ent->client->menu_data.showmenu)
	{
		RPS_MenuNext(ent);
//		Menu_Dn(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use && !it->drop)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// ++QMENU++
	//
	// See if the menu is visible, and if it is, handle it accordingly.
	
	if (ent->client->menu_data.showmenu)
	{
		RPS_MenuPrev(ent);
//		Menu_Up(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use && !it->drop)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item]>0)
		return;		// valid

	SelectNextItem (ent, -1);
}

void Cmd_Settings_f(edict_t *ent) {
	if (ent->client->menu_data.showmenu) {
		RPS_MenuClose(ent);
	} else {
		RPS_MenuOpen(ent, GQ_SettingsMenu1, GQ_SettingsChoice, false, 0);
	}
	return;
}

void Cmd_Team_Menu(edict_t *ent) {
	if (force_join->value)
		return;
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe < level.framenum)) {
		if (ent->client->team)
			gi_cprintf(ent, PRINT_MEDIUM, "You can switch teams after this match\n");
		else
			gi_centerprintf(ent, "You'll have to wait until\nthe next match to play\n");
		return;
	}
	if (!ent->client->pers.spectator && ent->client->team) {
		teamdata[ent->client->team-1].players--;
		GQ_CountTeamPlayers();
		ent->client->pers.spectator=1;
		ent->client->team=0;
		ent->client->resp.spectator=1;
		ent->s.modelindex3 = 0;
		ent->s.modelindex4 = 0;
		spectator_respawn (ent);
		debugmsg("Respawining\n");
	} else {
		PutClientInServer(ent);
	}
}

void Cmd_Class_Menu(edict_t *ent) {
	if ((int)playmode->value != PM_LAST_MAN_STANDING)
		return;
	if (preptimeframe < level.framenum) {
		gi_centerprintf(ent, "You'll have to wait until the\nnext match to change your class\n");
		return;
	}
	if (use_classes->value)
		RPS_MenuOpen(ent, GQ_ClassMenu, GQ_ChooseClass, false, 0);
	else
		RPS_MenuOpen(ent, GQ_WeaponsMenu, GQ_ChooseWeapon, false, 0);
}

void Cmd_Weapon_Menu(edict_t *ent) {
	if ((int)playmode->value != PM_LAST_MAN_STANDING)
		return;
	if (preptimeframe < level.framenum) {
		gi_centerprintf(ent, "You'll have to wait until the\nnext match to change your weapons\n");
		return;
	}
	if (use_classes->value)
		RPS_MenuOpen(ent, GQ_ClassMenu, GQ_ChooseClass, false, 0);
	else
		RPS_MenuOpen(ent, GQ_WeaponsMenu, GQ_ChooseWeapon, false, 0);
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
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] = 1;
			ent->client->pers.carried[i] = it->capacity;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}
/*
	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}
*/
	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (it->flags & IT_ARMOR)
				continue;
			if (!it->pickup)
				continue;
//			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
			if (it->flags & (IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi_cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi_cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		if (!it_ent->inuse)
			return;
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
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
		gi_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi_cprintf (ent, PRINT_HIGH, msg);
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
		gi_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi_cprintf (ent, PRINT_HIGH, msg);
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
		gi_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
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

	gi_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
	it->use (ent, it);
}

void Cmd_Id_f (edict_t *ent)
{
	if (!deathmatch->value && !coop->value) {
		gi_cprintf(ent, PRINT_MEDIUM, "Player ID is not available in single-player\n");
		return;
	}
	if (ent->client->idon) {
		ent->client->idon=false;
		gi_cprintf(ent, PRINT_MEDIUM, "Player ID is now off\n");
		if (ent->client->showscores && ent->client->showid) {
			ent->client->showscores=false;
			ent->client->showid=false;
			ent->client->idplayer=NULL;
		}
	} else {
		ent->client->idon=true;
		gi_cprintf(ent, PRINT_MEDIUM, "Player ID is now on\n");
	}
}

void Cmd_Ready_f (edict_t *ent)
{
	if (!deathmatch->value) {
		gi_cprintf(ent, PRINT_MEDIUM, "You cannot use the \"ready\" command in single-player games.\n");
		return;
	}
	if (((int)playmode->value != PM_LAST_MAN_STANDING) || !ready_all->value) {
		gi_cprintf(ent, PRINT_MEDIUM, "The \"ready\" command only works in Last Man Standing games with the READY_ALL flag set.\n");
		return;
	}
	if (preptimeframe<level.framenum) {
		gi_cprintf(ent, PRINT_MEDIUM, "The game has already started!\n");
		return;
	}
	if (preptimeframe<level.framenum+150) {
		gi_cprintf(ent, PRINT_MEDIUM, "Too late to change your mind now.  The match is about to start.\n");
		return;
	}
	if (ent->flags & FL_READY) {
		gi_bprintf(PRINT_HIGH, "%s is NO LONGER ready.\n", ent->client->pers.netname);
		preptimeframe=level.framenum+9999;
		ent->flags &= ~FL_READY;
	} else {
		int			i;
		edict_t		*player;
		qboolean	allset=true;
		gi_bprintf(PRINT_HIGH, "%s is now ready.\n", ent->client->pers.netname);
		ent->flags |= FL_READY;
		for (i=0 ; i<maxclients->value ; i++) {
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client)
				continue;
			if (player->client->resp.spectator)
				continue;
			if (!player->client->team)
				continue;
			if (player->deadflag)
				continue;
			if (!(player->flags & FL_READY)) {
				allset=false;
			}
		}
		if (allset) {
			gi_bprintf(PRINT_HIGH, "Game starts in 15 seconds\n");
			preptimeframe=level.framenum+151;
		}
	}
}



/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!Q_strcasecmp(s,"weapon"))	// Drop current weapon
		it=ent->client->pers.weapon;
	if (!Q_strcasecmp(s,"last") && ent->item)	// Drop last item picked up
		it=ent->item;
	if (!Q_strcasecmp(s,"artifact") || !Q_strcasecmp(s,"tech") || !Q_strcasecmp(s,"tech")) {	// Drop artifact
		if (!ent->client->artifact) {
			gi_cprintf (ent, PRINT_HIGH, "You're not carrying an artifact\n");
			return;
		} else {
			it=ent->client->artifact;
		}
	}
	if (!it)
	{
		gi_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index] && !(it->flags & IT_ARTIFACT))
	{
		gi_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
	CalcWeight(ent);
}


void Cmd_DropArtifact_f (edict_t *ent)
{
	if (!ent->client->artifact)
	{
		gi_cprintf (ent, PRINT_HIGH, "Not carrying an artifact\n");
		return;
	}
	if (!ent->client->artifact->drop)
	{
		gi_cprintf (ent, PRINT_HIGH, "Item is not dropable??\n");
		return;
	}
	ent->client->artifact->drop (ent, ent->client->artifact);
	CalcWeight(ent);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	// ++QMENU++
	//
	// if we are in menu mode, call the menu handler.
	
	if (ent->client->menu_data.showmenu)
	{
		RPS_MenuSelect(ent);
		return;
	}
	
	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon - i);
		if (index<0)
			index = MAX_ITEMS + index;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->newweapon == it) {
			return;	// successful
		}
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i) % MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->newweapon == it) {
			return;	// successful
		}
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}


void Cmd_Commands_f (edict_t *ent)
{
	gi_cprintf (ent, PRINT_HIGH, "The following commands are available:\n");
	gi_cprintf (ent, PRINT_HIGH, "MOTD - Display message of the day\n");
	gi_cprintf (ent, PRINT_HIGH, "SETTINGS - View game options\n");
	gi_cprintf (ent, PRINT_HIGH, "MENUHELP - How to use the menu system\n");
	gi_cprintf (ent, PRINT_HIGH, "DROP ARTIFACT - Drop carried artifact\n");
	gi_cprintf (ent, PRINT_HIGH, "ID - Toggle player ID on and off\n");
	gi_cprintf (ent, PRINT_HIGH, "TEAM_MENU - Lets you change teams\n");
	gi_cprintf (ent, PRINT_HIGH, "CLASS_MENU - Choose a different class (Last Man Standing)\n");
	gi_cprintf (ent, PRINT_HIGH, "WEAPON_MENU - Select new weapons and ammo (Last Man Standing)\n");
	gi_cprintf (ent, PRINT_HIGH, "READY - Indicates you are ready to begin the match (Last Man Standing)\n");
	gi_cprintf (ent, PRINT_HIGH, "\n");
	gi_cprintf (ent, PRINT_HIGH, "The following binds are vital:\n");
	gi_cprintf (ent, PRINT_HIGH, "+ALT_FIRE - Secondary firing mode\n");
	gi_cprintf (ent, PRINT_HIGH, "+RELOAD - Reload current weapon\n");
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
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

//	anum = game.clients[anum].ps.stats[STAT_FRAGS];
//	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];
	anum = game.clients[anum].resp.score;
	bnum = game.clients[bnum].resp.score;

	if (anum > bnum)
		return -1;
	if (anum < bnum)
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
//	int		count;
	char	small[64];
	char	large[1280];
/*	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);
*/
	// print information
	large[0] = 0;

	for (i = 0 ; playerlist[i]>-1 ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[playerlist[i]].ps.stats[STAT_FRAGS],
			game.clients[playerlist[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, i);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

// Eraser
/*
=================
Cmd_Join_f
=================
*/
/*
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void Cmd_Join_f(edict_t *ent, char *teamname)
{
	int i;
	char	userinfo[MAX_INFO_STRING];

	if (ctf->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv bluebots <name1> <name2> ..\" to spawn groups of bots in CTF.\n");
		return;
	}

	if (ent->client->team)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nYou are already a member of a team.\nYou must disconnect to change teams.\n\n");
		return;
	}

	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
		{
			gi.cprintf(ent, PRINT_HIGH, "Team \"%s\" does not exist.\n", teamname);
			return;
		}

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;		// only allow joining a team that has been created on the server

#ifdef _WIN32
		if (!_stricmp(bot_teams[i]->teamname, teamname) || !_stricmp(bot_teams[i]->abbrev, teamname))
#else
		if (!strcasecmp(bot_teams[i]->teamname, teamname) || !strcasecmp(bot_teams[i]->abbrev, teamname))
#endif
		{	// match found

			// check team isn't already full
			if ((bot_teams[i]->num_players >= players_per_team->value) && (bot_teams[i]->num_bots == 0))
			{
				gi.cprintf(ent, PRINT_HIGH, "Team \"%s\" is full.\n", bot_teams[i]->teamname);
				return;
			}

			bot_teams[i]->num_players++;

			if (strlen(bot_teams[i]->default_skin) > 0)	// set the team's skin
			{
				// copy userinfo
				strcpy(userinfo, ent->client->pers.userinfo);

				// set skin
				Info_SetValueForKey (userinfo, "skin", bot_teams[i]->default_skin);

				// record change
				ClientUserinfoChanged(ent, userinfo);
			}

			// must set this after skin!
			ent->client->team = bot_teams[i];
			bot_teams[i]->ingame = true;		// make sure we enable the team

			my_bprintf(PRINT_HIGH, "%s has joined team %s\n", ent->client->pers.netname, bot_teams[i]->teamname);
			return;
		}
	}
}

void Cmd_Lag_f (edict_t *ent, char *val)
{
	int i;

	i = atoi(val);

	if (i > 0)
	{
		if (i < 1000)
		{
			ent->client->latency = i;
			gi.cprintf(ent, PRINT_HIGH, "Latency set to %i\n", i);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "lag must be lower than 1000\n");
		}
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "lag must be higher than 0\n");
	}
}

void Cmd_Teams_f (edict_t *ent)
{
	char str[256];
	int i, j;

	if (ctf->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv redbots <name1> <name2> ..\" to spawn groups of bots in CTF.\n\n");
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "\n=====================================\nAvailable teams:\n\n");

	// list all available teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;	// don't show teams that haven't been created, when in dedicated server mode

		// print the team name
		gi.cprintf(ent, PRINT_HIGH, "%s ", bot_teams[i]->teamname);

		for (j=0; j<(15-strlen(bot_teams[i]->teamname)); j++)
			str[j] = ' ';
		str[j] = 0;

		gi.cprintf(ent, PRINT_HIGH, "%s(%s)", str, bot_teams[i]->abbrev);

		for (j=0; j<(4-strlen(bot_teams[i]->abbrev)); j++)
			str[j] = ' ';
		str[j] = 0;

		gi.cprintf(ent, PRINT_HIGH, str);

		if (bot_teams[i]->ingame)
		{
			gi.cprintf(ent, PRINT_HIGH, "%i plyrs", bot_teams[i]->num_players);
			if (bot_teams[i]->num_bots)
				gi.cprintf(ent, PRINT_HIGH, " (%i bots)\n", bot_teams[i]->num_bots);

			gi.cprintf(ent, PRINT_HIGH, "\n");
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "[none]\n");

	}

	gi.cprintf(ent, PRINT_HIGH, "\n=====================================\n");
}

void Cmd_BotCommands_f	(edict_t	*ent)
{	// show bot info
	debugmsg("\n=================================\nSERVER ONLY COMMANDS:\n\n \"bot_num <n>\" - sets the maximum number of bots at once to <n>\n\n \"bot_name <name>\" - spawns a specific bot\n\n \"bot_free_clients <n>\" - makes sure there are always <n> free client spots\n\n \"bot_calc_nodes 0/1\" - Enable/Disable dynamic node-table calculation\n\n \"bot_allow_client_commands <n>\" - set to 1 to allow clients to spawn bots via \"cmd bots <n>\"\n=================================\n\n");
}

void Cmd_Tips_f (edict_t *ent)
{	
	gi.cprintf(ent, PRINT_HIGH, "\nERASER TIPS:\n\n * Set \"skill 0-3\" to vary the difficulty of your opponents (1 is default)\n\n * You can create your own bots by editing the file BOTS.CFG in the Eraser directory\n\n * Set \"maxclients 32\" to allow play against more bots\n\n");
}

void Cmd_Botpath_f (edict_t *ent)
{
	trace_t tr;
	vec3_t	dest;

	AngleVectors(ent->client->v_angle, dest, NULL, NULL);
	VectorScale(dest, 600, dest);
	VectorAdd(ent->s.origin, dest, dest);

	tr = gi.trace(ent->s.origin, VEC_ORIGIN, VEC_ORIGIN, dest, ent, MASK_PLAYERSOLID);

	if (tr.ent && tr.ent->bot_client)
	{
		tr.ent->flags |= FL_SHOWPATH;

		debugmsg("Showing path for %s\n", tr.ent->client->pers.netname);
	}
}

// toggles the debug path for this client
void Cmd_Showpath_f (edict_t *ent)
{
	if (ent->flags & FL_SHOWPATH)
		ent->flags -= FL_SHOWPATH;
	else
		ent->flags |= FL_SHOWPATH;
}
// Eraser end*/

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (ent->flags & FL_ANTIBOT)
		return;
	if (gi.argc () < 2 && !arg0)
		return;

//	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
	if (!(teamplay->value))
		team = false;

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

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi_cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi_cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi_cprintf(ent, PRINT_HIGH, "%s", text);
}

/*
==================
Cmd_Team_f

Join a specific team
==================
*/
void Cmd_Team_f (edict_t *ent)
{
	int			i, j;
	gitem_t		*it;
	char		*s;

	if (((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe < level.framenum)) {
		if (ent->client->team)
			gi_cprintf(ent, PRINT_MEDIUM, "You can switch teams after this match\n");
		else
			gi_centerprintf(ent, "You'll have to wait until\nthe next match to play\n");
		return;
	}

	s = gi.args();
	it = FindItem (s);
	i = atoi(s);

	if (!Q_strcasecmp(s, "red")) {
		i=1;
	} else if (!Q_strcasecmp(s, "blue")) {
		i=2;
	} else if (!Q_strcasecmp(s, "green")) {
		i=3;
	} else if (!Q_strcasecmp(s, "white")) {
		i=4;
	}
	if (i>0) {
		if (i>(int)number_of_teams->value) {
			gi_cprintf(ent, PRINT_HIGH, "TEAM: Invalid team choice.\n");
			return;
		}
		GQ_ChooseTeam(ent, i);
		return;
	} else {
		for (j=0; j<(int)number_of_teams->value; j++) {
			if (!Q_strcasecmp(s, teamdata[j].name->string)) {
				GQ_ChooseTeam(ent, j+1);
				return;
			}
		}
	}
	gi_cprintf(ent, PRINT_HIGH, "TEAM: Invalid team choice.\n");
}


void Cmd_Start_Alt_Fire(edict_t *ent) {
	ent->client->alt_fire_on = true;
	//    gi_cprintf(ent, PRINT_HIGH, "Alt fire on");
}

void Cmd_End_Alt_Fire(edict_t *ent) {
	ent->client->alt_fire_on = false;
	//    gi_cprintf(ent, PRINT_HIGH, "Alt fire off");
}

void Cmd_Start_Reload(edict_t *ent) {
	ent->client->reloading = true;
	//gi_bprintf(PRINT_HIGH, "+reload\n");
}

void Cmd_End_Reload(edict_t *ent) {
	ent->client->reloading = false;
	//gi_bprintf(PRINT_HIGH, "-reload\n");
}


/*
=================
ClientCommand
=================
*/

void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "settings") == 0)
	{
		Cmd_Settings_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "menuhelp") == 0) {
		RPS_MenuHelp(ent);
		return;
	}
	if (Q_stricmp (cmd, "motd") == 0) {
		GQ_MOTD(ent);
		return;
	}
	if ((Q_stricmp (cmd, "cmds") == 0) || (Q_stricmp (cmd, "commands") == 0)) {
		Cmd_Commands_f(ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0) {
		Cmd_Drop_f (ent);
	} else if ((Q_stricmp (cmd, "droprune") == 0) || (Q_stricmp (cmd, "droptech") == 0) || (Q_stricmp (cmd, "dropartifact") == 0)) {
		Cmd_DropArtifact_f (ent);
	} else if (Q_stricmp (cmd, "give") == 0) {
		Cmd_Give_f (ent);
		CalcWeight(ent);
	} else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else if (Q_stricmp (cmd, "+alt_fire") == 0)
		Cmd_Start_Alt_Fire(ent);
	else if (Q_stricmp (cmd, "-alt_fire") == 0)
		Cmd_End_Alt_Fire(ent);
	else if (Q_stricmp (cmd, "+reload") == 0)
		Cmd_Start_Reload(ent);
	else if (Q_stricmp (cmd, "-reload") == 0)
		Cmd_End_Reload(ent);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "team_menu") == 0)
		Cmd_Team_Menu(ent);
	else if (Q_stricmp (cmd, "class_menu") == 0)
		Cmd_Class_Menu(ent);
	else if (Q_stricmp (cmd, "weapon_menu") == 0)
		Cmd_Weapon_Menu(ent);
	else if (Q_stricmp (cmd, "id") == 0)
		Cmd_Id_f(ent);
	else if (Q_stricmp (cmd, "ready") == 0)
		Cmd_Ready_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
