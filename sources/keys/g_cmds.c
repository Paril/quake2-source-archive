#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "p_trail.h"

//ERASER
void botRemovePlayer(edict_t *self);
//ERASER

char *ClientTeam (edict_t *ent)
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
	char	ent1Team [512];
	char	ent2Team [512];

	//ERASER
	// ERASER teams
	if (ent1->client && ent2->client)
		if (ent1->client->team && (ent1->client->team == ent2->client->team))
			return true;
	//ERASER

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	} else if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
//ZOID

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
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

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
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

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
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
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
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
			//K2:Begin - Don't give certain weapons and powerups if they are turned off
			if (Q_stricmp(it->pickup_name, "BFG10K") == 0)
			{
				if( nobfg->value )
				{
					continue;
				}

				//This is another id Software bug
				//If infinite ammo is on and cheats are on, and you give yourself the BFG
				//The server will crash
				if ( (int)dmflags->value & DF_INFINITE_AMMO )
				{
					continue;
				}

			}
			if ((Q_stricmp(it->pickup_name, "Shotgun") == 0) &&
				(noshotgun->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Super Shotgun") == 0) &&
				(nosupershotgun->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Machinegun") == 0) &&
				(nomachinegun->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Chaingun") == 0) &&
				(nochaingun->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Grenade Launcher") == 0) &&
				(nogrenadelauncher->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Rocket Launcher") == 0) &&
				(norocketlauncher->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Hyperblaster") == 0) &&
				(nohyperblaster->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Railgun") == 0) &&
				(norailgun->value) )
				continue;
			
			//K2:End

			ent->client->pers.inventory[i] += 1;
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
		
		//K2:Begin - Fix DF_NO_ARMOR bug with Power Shield
		//           This is an id Software bug
		//           If not allowing armor, give all or give power shield results in crash
		if ( ((int)dmflags->value & DF_NO_ARMOR) == 0)
		{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
		}
		else
			gi.cprintf (ent,PRINT_HIGH,"You can't have the Power Shield\n");
		//K2:End


		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			
			//K2:Begin
			if ((Q_stricmp(it->pickup_name, "Quad Damage") == 0) &&
				(noquad->value) )
				continue;
			if ((Q_stricmp(it->pickup_name, "Invulnerability") == 0) &&
				(noinvulnerability->value) )
				continue;
			//K2:End
			
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
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf(ent,PRINT_HIGH,"non-pickup item\n");
		return;
	}

	//K2:Begin - Don't give weapons if not included in game
	if (strcmp(it->pickup_name, "BFG10K") == 0)
	{
		if( nobfg->value )
		{
			gi.cprintf(ent,PRINT_HIGH,"You can't have the BFG\n");
			return;
		}

		//This is another id Software bug
		//If infinite ammo is on and you give yourself the BFG
		//The server will crash
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			gi.cprintf(ent,PRINT_HIGH,"You can't have the BFG\n");
			return;
		}
	}
	if ((Q_stricmp(it->pickup_name, "Shotgun") == 0) &&
				(noshotgun->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Super Shotgun") == 0) &&
				(nosupershotgun->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Machinegun") == 0) &&
				(nomachinegun->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Chaingun") == 0) &&
				(nochaingun->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Grenade Launcher") == 0) &&
				(nogrenadelauncher->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Rocket Launcher") == 0) &&
				(norocketlauncher->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Hyperblaster") == 0) &&
				(nohyperblaster->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Railgun") == 0) &&
				(norailgun->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Quad Damage") == 0) &&
				(noquad->value) )
				return;
	if ((Q_stricmp(it->pickup_name, "Invulnerability") == 0) &&
				(noinvulnerability->value) )
				return;

	//K2:End

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
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
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
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
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
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
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

	safe_cprintf (ent, PRINT_HIGH, msg);
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
/*
// ERASER, only enable grapple if calc_nodes = 0
	if (bot_calc_nodes->value && !strcmp(s, "grapple"))
	{
		safe_cprintf (ent, PRINT_HIGH, "Grapple not available while bot_calc_nodes = 1\n");
		return;
	}
*/
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
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

//ZOID--special case for tech powerups
	if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL) {
		it->drop (ent, it);
		return;
	}
//ZOID

	s = gi.args();
	
	//K2:Begin - Drop Key command
	if (!strcmp(s,"key"))
	{
		if(!droppable->value)
		{
			safe_cprintf(ent,PRINT_HIGH,"This server does not allow dropping of keys\n");
			return;
		}

		K2_DropKeyCommand(ent);
		K2_ResetClientKeyVars(ent);
		return;
				
	}
	//K2:End

	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
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

//ZOID
	if (ent->client->menu) {
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}
//ZOID

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

//ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID

	//K2:Begin Menu
	if(!ctf->value && !ent->client->resp.inServer)
	{
		K2_OpenJoinMenu(ent);
		return;
	}
	//K2:End


	cl->showinventory = true;
	cl->showscores = false;

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

//ZOID
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
//ZOID

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
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
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		
		//K2:Begin - Substitute Cycle_Weapon() for Use_Weapon()
		//it->use (ent, it);
		Cycle_Weapon(ent,it);
		//K2:End
		
		if (cl->pers.weapon == it)
			return;	// successful
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
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		
		//K2:Begin - Substitute Cycle_Weapon() for Use_Weapon()
		//it->use (ent, it);
		Cycle_Weapon(ent,it);
		//K2:End
		
		if (cl->pers.weapon == it)
			return;	// successful
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
	
	//K2:Begin - Substitute Cycle_Weapon() for Use_Weapon()
	//it->use (ent, it);
	Cycle_Weapon(ent,it);
	//K2:End
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
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	
	//K2:Begin - Allow normal inventory dropping of keys as well as "drop key"
	if( droppable->value &&
		( strstr(it->pickup_name,"Key") || strstr(it->pickup_name,"BFK"))
      )
	{
		K2_DropKeyCommand(ent);
		K2_ResetClientKeyVars(ent);
		SelectPrevItem(ent, IT_POWERUP);
		return;
	}
	//K2:End
	
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
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
	//ent->deadflag = DEAD_DEAD;
	//respawn (ent);
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
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
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

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
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
		safe_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}


//ERASER?
/*
=================
Cmd_Join_f
=================
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void Cmd_Join_f(edict_t *ent, char *teamname)
{
	int i;
	char	userinfo[MAX_INFO_STRING];

	//K2:Begin - Single player join command
	if (!ctf->value && !teamplay->value)
	{
		if (!ent->client->resp.inServer)
			K2EnterGame(ent,NULL);
		else if (ent->client->resp.spectator)
			safe_centerprintf(ent,"You are in Spectator mode.\nType \"spectator 0\" at the console\nto rejoin the game");
		return;

	}
	//K2:End
	
	if (ctf->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv bluebots <name1> <name2> ..\" to spawn groups of bots in CTF.\n");
		return;
	}

	if (ent->client->team)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nYou are already a member of a team.\nYou must disconnect to change teams.\n\n");
		return;
	}

	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
		{
			safe_cprintf(ent, PRINT_HIGH, "Team \"%s\" does not exist.\n", teamname);
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
				safe_cprintf(ent, PRINT_HIGH, "Team \"%s\" is full.\n", bot_teams[i]->teamname);
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
			safe_cprintf(ent, PRINT_HIGH, "Latency set to %i\n", i);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "lag must be lower than 1000\n");
		}
	}
	else
	{
		safe_cprintf(ent, PRINT_HIGH, "lag must be higher than 0\n");
	}
}

void Cmd_Teams_f (edict_t *ent)
{
	char str[256];
	int i, j;

	if (ctf->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv redbots <name1> <name2> ..\" to spawn groups of bots in CTF.\n\n");
		return;
	}

	safe_cprintf(ent, PRINT_HIGH, "\n=====================================\nAvailable teams:\n\n");

	// list all available teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;	// don't show teams that haven't been created, when in dedicated server mode

		// print the team name
		safe_cprintf(ent, PRINT_HIGH, "%s ", bot_teams[i]->teamname);

		for (j=0; j<(15-strlen(bot_teams[i]->teamname)); j++)
			str[j] = ' ';
		str[j] = 0;

		safe_cprintf(ent, PRINT_HIGH, "%s(%s)", str, bot_teams[i]->abbrev);

		for (j=0; j<(4-strlen(bot_teams[i]->abbrev)); j++)
			str[j] = ' ';
		str[j] = 0;

		safe_cprintf(ent, PRINT_HIGH, str);

		if (bot_teams[i]->ingame)
		{
			safe_cprintf(ent, PRINT_HIGH, "%i plyrs", bot_teams[i]->num_players);
			if (bot_teams[i]->num_bots)
				safe_cprintf(ent, PRINT_HIGH, " (%i bots)\n", bot_teams[i]->num_bots);

			safe_cprintf(ent, PRINT_HIGH, "\n");
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "[none]\n");

	}

	safe_cprintf(ent, PRINT_HIGH, "\n=====================================\n");
}

void Cmd_BotCommands_f	(edict_t	*ent)
{	// show bot info
	gi.dprintf("\n=================================\nSERVER ONLY COMMANDS:\n\n \"bot_num <n>\" - sets the maximum number of bots at once to <n>\n\n \"bot_name <name>\" - spawns a specific bot\n\n \"bot_free_clients <n>\" - makes sure there are always <n> free client spots\n\n \"bot_calc_nodes 0/1\" - Enable/Disable dynamic node-table calculation\n\n \"bot_allow_client_commands <n>\" - set to 1 to allow clients to spawn bots via \"cmd bots <n>\"\n=================================\n\n");
}

void Cmd_Tips_f (edict_t *ent)
{	
	safe_cprintf(ent, PRINT_HIGH, "\nERASER TIPS:\n\n * Set \"skill 0-3\" to vary the difficulty of your opponents (1 is default)\n\n * You can create your own bots by editing the file BOTS.CFG in the Eraser directory\n\n * Set \"maxclients 32\" to allow play against more bots\n\n");
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

		gi.dprintf("Showing path for %s\n", tr.ent->client->pers.netname);
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

//ERASER

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j,i;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
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
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client || other->bot_client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		safe_cprintf(other, PRINT_CHAT, "%s", text);
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
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


//ERASER
//==========================================================
// CTF flagpath hack, provide alternate routes for bots to return
// the flag to base
void FlagPathTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->bot_client)
		return;

	if (ent->last_goal)	// this is a source flagpath
	{
		if (other->client->resp.ctf_team != ent->skill_level)
			return;
		if (other->flagpath_goal)	// already heading for a destination
			return;
	}
	else	// this is a destination
	{
		if (other->flagpath_goal == ent)		// reached destination, so clear it
			other->flagpath_goal = NULL;

		return;
	}

	if (!CarryingFlag(other))
		return;

	// carring flag, if this is a src path, send them on an alternate (safest) route
	if (ent->last_goal)
	{
		float count1, count2;
		int i;

		if (!other->flagpath_goal)
		{
			count1 = count2 = 0;

			for (i=0; i<num_players; i++)
			{
				if (players[i]->client->resp.ctf_team && (players[i]->client->resp.ctf_team != other->client->resp.ctf_team))
				{
					count1 += entdist(ent->last_goal, players[i]);
					count2 += entdist(ent->target_ent, players[i]);
				}
			}

			if (count1 > count2)
				other->flagpath_goal = ent->last_goal;
			else
				other->flagpath_goal = ent->target_ent;
		}

	}
	else	// this is a destination path
	{
		other->flagpath_goal = NULL;
	}
}

extern int dropped_trail;

edict_t *flagpaths[3] = {NULL, NULL, NULL};
void FlagPath(edict_t *ent, int ctf_team)
{
	int i;
	static int flagpath_type=0;

	if (flagpath_type == 0)
	{
		// new source
		if (flagpaths[flagpath_type])
		{
			if (ent->client)
			{
				safe_cprintf(ent, PRINT_HIGH, "Incomplete FlagPath, starting a new path.\n");

				for (i=0; i<3; i++)
				{
					G_FreeEdict(flagpaths[i]);
					flagpaths[i] = NULL;
				}
			}
		}

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_src";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->skill_level = ctf_team;

		if (ent->client)
			safe_cprintf(ent, PRINT_HIGH, "Flagpath SOURCE dropped.\n");
	}
	else if (flagpath_type == 1)
	{

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->last_goal = flagpaths[flagpath_type];

		if (ent->client)
			safe_cprintf(ent, PRINT_HIGH, "Flagpath DEST 1 dropped.\n");
	}
	else if (flagpath_type == 2)
	{
		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->target_ent = flagpaths[flagpath_type];

		// completed paths, so make them triggers, and clear
		for (i=0; i<3; i++)
		{
			flagpaths[i]->solid = SOLID_TRIGGER;
			VectorSet(flagpaths[i]->mins, -16, -16, -16);
			VectorSet(flagpaths[i]->maxs,  16,  16,  4);
			flagpaths[i]->touch = FlagPathTouch;
			gi.linkentity(flagpaths[i]);

			flagpaths[i] = NULL;
		}

		flagpath_type = -1;

		if (ent->client)
		{
			safe_cprintf(ent, PRINT_HIGH, "Flagpath DEST 2 dropped.\nSequence complete.\n\n");
			dropped_trail = true;
		}
	}

	flagpath_type++;
}

//ERASER

//K2:Begin
//Take Key
void Cmd_TakeKey_f (edict_t *ent)
{
	
	if (K2_IsAnti(ent))
		K2_TakePlayerKey(ent);
	else
		safe_cprintf(ent,PRINT_HIGH,"You need the Anti-Key to take someone's key\n");
	
}

//Feign
void Cmd_Feign_f (edict_t *ent)
{
	if( allowfeigning->value )
	{
		if (ent->client->is_feigning)
			Client_EndFeign (ent);
		else
			Client_BeginFeign (ent);	
	}
	else
	{
		safe_cprintf(ent,PRINT_HIGH,"Feigning is not allowed\n");
	
	}
	
	
}

//Flash Grenade
void Cmd_FlashGrenade_f(edict_t *ent)
{
	int			ammo;
	gitem_t		*it;
	
	it=FindItem("Grenades");
	//TODO:Check grenade ammo
	ammo = ent->client->pers.inventory[ITEM_INDEX(it)];
	if (!ammo)
	{
		safe_cprintf(ent,PRINT_HIGH,"No Grenades\n");
		return;
	}
		
	if ( (ent->client->grenadeType == GRENADE_NORMAL) || (ent->client->grenadeType == GRENADE_FREEZE))
	{
		if( flashgrenades->value)
		{
			ent->client->grenadeType = GRENADE_FLASH;
			safe_centerprintf(ent,"Flash Grenades Selected\n");
		}
	}
	else
	{
		ent->client->grenadeType = GRENADE_NORMAL;
		safe_centerprintf(ent,"Standard Grenades Selected\n");
	}

	it->use(ent,it);
}

//Flash Grenade
void Cmd_FreezeGrenade_f(edict_t *ent)
{
	int			ammo;
	gitem_t		*it;
	
	it=FindItem("Grenades");
	//TODO:Check grenade ammo
	ammo = ent->client->pers.inventory[ITEM_INDEX(it)];
	if (!ammo)
	{
		safe_cprintf(ent,PRINT_HIGH,"No Grenades\n");
		return;
	}
		
	if ( (ent->client->grenadeType == GRENADE_NORMAL) || (ent->client->grenadeType == GRENADE_FLASH))
	{
		if( freezegrenades->value)
		{
			ent->client->grenadeType = GRENADE_FREEZE;
			safe_centerprintf(ent,"Freeze Grenades Selected\n");
		}
	}
	else
	{
		ent->client->grenadeType = GRENADE_NORMAL;
		safe_centerprintf(ent,"Standard Grenades Selected\n");
	}

	it->use(ent,it);
}

void Cmd_DrunkRocket_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Rocket Launcher");
	if (!it)
	{
		it=FindItem("Rocket Launcher");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Rocket Launcher\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->rocketType = ROCKET_FIRE;
	Use_Weapon (ent, it);
	
}

void Cmd_FreezeGrenadeLauncher_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Grenade Launcher");
	if (!it)
	{
		it=FindItem("Grenade Launcher");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Grenade Launcher\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->grenadeType = GRENADE_FIRE;
	Use_Weapon (ent, it);
	
}

void Cmd_FlashGrenadeLauncher_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Grenade Launcher");
	if (!it)
	{
		it=FindItem("Grenade Launcher");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Grenade Launcher\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->grenadeType = GRENADE_NORMAL;
	Use_Weapon (ent, it);
	
}

void Cmd_FireGrenadeLauncher_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Grenade Launcher");
	if (!it)
	{
		it=FindItem("Grenade Launcher");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Grenade Launcher\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->grenadeType = GRENADE_FLASH;
	Use_Weapon (ent, it);
	
}

void Cmd_FireRocketLauncher_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Rocket Launcher");
	if (!it)
	{
		it=FindItem("Rocket Launcher");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Rocket Launcher\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->rocketType = ROCKET_NORMAL;
	Use_Weapon (ent, it);
	
}

void Cmd_GibGun_f(edict_t *ent)
{
	gitem_t		*it;
	
	it=FindItem("Blaster");
	if (!it)
	{
		it=FindItem("Blaster");
		if (!it)
		{
			gi.cprintf(ent,PRINT_HIGH,"unknown item\n");
			return;
		}
	}
	
	if (!ent->client->pers.inventory[ITEM_INDEX(it)])
	{
		safe_centerprintf(ent,"You don't have the Blaster\n");
		return;
	}

	//Hack for Use_Weapon
	ent->client->k2cmd = true;	
	
	ent->client->blasterType = BLASTER_NORMAL;
	Use_Weapon (ent, it);
	
}

//Keys2_ commands
void Cmd_Keys2_f (edict_t *ent)
{
	char	*s;

	s = gi.args();

	if (Q_stricmp(s,"help") == 0)
	{
		safe_cprintf(ent,PRINT_HIGH,"Now close the console to view the help menu...\n");
		K2_OpenHelpMenu(ent, NULL);
		return;
	}
			
}

void Cmd_Bot_f (edict_t *ent)
{
	int		i, num_to_remove;

	//Check allow
	if (!bot_allow_client_commands->value)
		{
			gi.cprintf(ent,PRINT_HIGH,"Server has disabled bot commands\n");
			return;
		}

	
	//Check last command time
	if(!ent->client->next_botnum_command || (ent->client->next_botnum_command < level.time))
	{

		if (Q_stricmp(gi.argv(1),"") == 0)
		{
			gi.cprintf(ent,PRINT_HIGH,"Usage: botnum <n> where n is the number of bots you want in the game.");
			return;
		}
	
		i = atoi(gi.argv(1));

		if (i < 0)
			return;

		if (i > (int)maxbots->value)
		{
			gi.cprintf(ent,PRINT_HIGH,"Only %i bots allowed in game\n",(int)maxbots->value);
			return;
		}

	
		num_to_remove = (int)bot_num->value - i;
		
		gi.cvar_set("bot_num",gi.argv(1));

		//Remove bot's if needed
		if (num_to_remove > 0)
		{
			while ( num_to_remove > 0)
			{
				if ((bot_count > 0)) // && (num_players > (maxclients->value - bot_free_clients->value)))
				{	// drop a bot to free a client spot
					edict_t	*trav, *lowest=NULL;
					int	i;

					// drop the lowest scoring bot
					for (i=0; i < num_players; i++)
					{
						trav = players[i];

						if (!trav->bot_client)
							continue;

						if (!lowest || (trav->client->resp.score < lowest->client->resp.score))
						{
							lowest = trav;
						}
					}

					if (lowest)		// if NULL, then must be full of real players
					{
						botDisconnect(lowest);
					}
				}

				num_to_remove--;
			}
		}
		
		ent->client->next_botnum_command = level.time + 60;
	
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"Try this command again later\n");
	}
	
	
}

//K2:End


/*
=================
ClientCommand
=================
*/
//ERASER
extern float	team1_rushbase_time, team2_rushbase_time;	// used by RUSHBASE command
extern float	team1_defendbase_time, team2_defendbase_time;
#define	RUSHBASE_OVERRIDE_TIME	180.0
//ERASER

void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client /*|| ent->bot_client*/)
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
		if (ctf->value)
			CTFSay_Team(ent, gi.args());
		else
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

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
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
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	//ERASER
	else if (Q_stricmp (cmd, "botname") == 0) {
		//if (!bot_allow_client_commands->value)
		//{
		//	gi.dprintf("Server has disabled bot commands\n");
		//	return;
		//}
	
		//spawn_bot (gi.argv(1));
		
		safe_cprintf(ent, PRINT_HIGH, "\nThis command is not used anymore.\nUse botnum # to spawn some bots.\n\n");
	}
	else if (Q_stricmp (cmd, "bots") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nThis command is not used anymore.\nUse bot_num # to spawn some bots.\n\n");
	}
	else if (Q_stricmp (cmd, "servcmd") == 0)
	{
		Cmd_BotCommands_f(ent);
	}
	else if (Q_stricmp (cmd, "tips") == 0)
	{
		Cmd_Tips_f(ent);
	}
	else if (Q_stricmp (cmd, "addmd2skin") == 0)
	{
		AddModelSkin(gi.argv(1), gi.argv(2));
	}
	else if (Q_stricmp (cmd, "join") == 0)
	{
		Cmd_Join_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "lag") == 0)
	{
		Cmd_Lag_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "teams") == 0)
	{
		Cmd_Teams_f(ent);
	}
	else if (Q_stricmp (cmd, "botpath") == 0)
	{
		Cmd_Botpath_f(ent);
	}
	else if (Q_stricmp (cmd, "showpath") == 0)
	{
		Cmd_Showpath_f(ent);
	}
	else if (Q_stricmp (cmd, "group") == 0)
	{
		TeamGroup(ent);
	}
	else if (Q_stricmp (cmd, "disperse") == 0)
	{
		TeamDisperse(ent);
	}
	else if (Q_stricmp (cmd, "rushbase") == 0)
	{

		edict_t	*plyr;
		int	i;
		edict_t *flag, *enemy_flag;

		if (!ctf->value)
		{
			safe_cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			team1_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team1_defendbase_time = 0;
		}
		else
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			team2_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team2_defendbase_time = 0;
		}

		safe_centerprintf(ent, "All available units RUSH BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];

			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;

			if (plyr->bot_client)
			{
				plyr->movetarget = enemy_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				safe_cprintf(plyr, PRINT_CHAT, "<%s> Rushing base!\n", ent->client->pers.netname);
			}
		}
	}

	else if (Q_stricmp (cmd, "defendbase") == 0)
	{

		edict_t	*plyr;
		int	i;
		edict_t *flag, *enemy_flag;

		if (!ctf->value)
		{
			safe_cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			team1_rushbase_time = 0;
			team1_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}
		else
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			team2_rushbase_time = 0;
			team2_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}

		safe_centerprintf(ent, "All available units DEFEND BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];

			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;

			if (plyr->bot_client)
			{
				plyr->movetarget = flag;
				plyr->target_ent = flag;
				plyr->movetarget_want = WANT_SHITYEAH;
			}
			else if (plyr != ent)
			{
				safe_cprintf(plyr, PRINT_CHAT, "<%s> Defending base!\n", ent->client->pers.netname);
			}
		}
	}

	else if (Q_stricmp (cmd, "freestyle") == 0)
	{
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			team1_rushbase_time = 0;
			team1_defendbase_time = 0;
		}
		else
		{
			team2_rushbase_time = 0;
			team2_defendbase_time = 0;
		}

		safe_cprintf(ent, PRINT_HIGH, "Returning bots to Freestyle mode.\n");
	}

	else if (Q_stricmp (cmd, "flagpath") == 0)
	{
		FlagPath(ent, ent->client->resp.ctf_team);
	}
	else if (Q_stricmp (cmd, "clear_flagpaths") == 0)
	{
		edict_t *trav=NULL, *last=NULL;
		int count=0;

		while (trav = G_Find(last, FOFS(classname), "flag_path_src"))
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
		}

		last = NULL;
		while (trav = G_Find(last, FOFS(classname), "flag_path_dest"))
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
		}

		if (count)
			safe_cprintf(ent, PRINT_HIGH, "\nSuccessfully cleared all flagpaths\n\n");
	}
	else if (Q_stricmp (cmd, "botpause") == 0)
	{
		paused = !paused;
		
		if (!paused)
		{	// just resumed play
			int i;

			for (i=0; i<num_players; i++)
			{
				if (!players[i]->bot_client && !ent->client->ctf_grapple)
				{
					players[i]->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
			}

			safe_bprintf(PRINT_HIGH, "%s unpaused the game\n", ent->client->pers.netname);
		}
	}
	else if ((Q_stricmp (cmd, "redflag") == 0) || (Q_stricmp (cmd, "blueflag") == 0))
	{	// spawn a red flag, which gets saved as node data to make DM levels into CTF levels
		edict_t *flag;

		flag = G_Spawn();
		flag->classname = gi.TagMalloc(16, TAG_LEVEL);
		strcpy(flag->classname, cmd);
		VectorCopy(ent->s.origin, flag->s.origin);
		VectorCopy(ent->s.angles, flag->s.angles);

		safe_cprintf(ent, PRINT_HIGH, "Dropped \"%s\" node\n", cmd);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clearflags") == 0)
	{
		edict_t *flag;

		flag = NULL;
		while (flag = G_Find( flag, FOFS(classname), "blueflag" ) )
		{
			G_FreeEdict( flag );
		}

		flag = NULL;
		while (flag = G_Find( flag, FOFS(classname), "redflag" ) )
		{
			G_FreeEdict( flag );
		}

		safe_cprintf( ent, PRINT_HIGH, "Cleared user created CTF flags.\n");
	}
	else if (Q_stricmp (cmd, "ctf_item") == 0)
	{
		ctf_item_t *old_ctf_item_head;

		cmd = gi.argv(1);

		if (strlen(cmd) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "No classname specified, ignored.\n");
			return;
		}

		old_ctf_item_head = ctf_item_head;

		ctf_item_head = gi.TagMalloc(sizeof(ctf_item_t), TAG_LEVEL);
		memset(ctf_item_head, 0, sizeof(ctf_item_t));

		strcpy(ctf_item_head->classname, cmd);
		VectorCopy(ent->s.origin, ctf_item_head->origin);
		VectorCopy(ent->s.angles, ctf_item_head->angles);
		ctf_item_head->next = old_ctf_item_head;

		safe_cprintf(ent, PRINT_HIGH, "Successfully placed %s at (%i %i %i)\nThis item will appear upon reloading the current map\n", cmd, (int)ent->s.origin[0], (int)ent->s.origin[1], (int)ent->s.origin[2]);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clear_items") == 0)
	{
		ctf_item_head = NULL;
		safe_cprintf(ent, PRINT_HIGH, "Cleared CTF_ITEM data\n");
		dropped_trail = true;
	}

	else if (Q_stricmp (cmd, "toggle_flagpaths") == 0)
	{
		if (ent->flags & FL_SHOW_FLAGPATHS)
		{
			ent->flags &= ~FL_SHOW_FLAGPATHS;

			// turn off the models
			{
				edict_t *trav;

				// show lines between alternate routes
				trav = NULL;
				while (trav = G_Find(trav, FOFS(classname), "flag_path_src"))
					trav->s.modelindex = 0;
			}
		}
		else
			ent->flags |= FL_SHOW_FLAGPATHS;
	}
	//ERASER

//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	} else if (Q_stricmp(cmd, "id") == 0) {
		CTFID_f (ent);
	}
//ZOID

    ///Q2 Camera Begin
	else if (Q_stricmp (cmd, "cam") == 0)
    {
        //K2: Just do the command...we'll do the botRemovePlayer in CameraCmd();
		//if (CameraCmd(ent))
		//	botRemovePlayer(ent);	// Ridah, must remove from list first (make sure to add to list, if ability to change back to player is added)
		CameraCmd(ent,gi.argv(1));
    }
    ///Q2 Camera End

	//K2:Begin - Feign
	else if (Q_stricmp (cmd, "feign") == 0)
		Cmd_Feign_f (ent);
	//Flash Grenade
	else if (Q_stricmp (cmd, "flash") == 0)
        Cmd_FlashGrenade_f (ent);
	else if (Q_stricmp (cmd, "freeze") == 0)
        Cmd_FreezeGrenade_f (ent);
	else if (Q_stricmp (cmd, "gib") == 0)
        Cmd_GibGun_f (ent);
	else if (Q_stricmp (cmd, "firerl") == 0)
        Cmd_FireRocketLauncher_f (ent);
	else if (Q_stricmp (cmd, "firegl") == 0)
        Cmd_FireGrenadeLauncher_f (ent);
	else if (Q_stricmp (cmd, "flashgl") == 0)
        Cmd_FlashGrenadeLauncher_f (ent);
	else if (Q_stricmp (cmd, "freezegl") == 0)
        Cmd_FreezeGrenadeLauncher_f (ent);
	else if (Q_stricmp (cmd, "drunk") == 0)
        Cmd_DrunkRocket_f (ent);
	//keys2_ command
	else if (Q_stricmp (cmd, "keys2") == 0)
        Cmd_Keys2_f (ent);
	//take key
	else if (Q_stricmp (cmd, "take") == 0)
        Cmd_TakeKey_f (ent);
	//else if (Q_stricmp(cmd, "_version") == 0) 
	//Cmd_ClientVersion_f (ent);
	//K2:Zbot Scan
	else if (Q_stricmp(cmd, "!zbot") == 0)
		ent->client->resp.bot_end = 0; // not a bot.
	else if (Q_stricmp(cmd, "#zbot") == 0)
        ent->client->resp.std_resp = 1; // replied.
	//Client Bot Commands
	else if (Q_stricmp (cmd, "botnum") == 0)
		Cmd_Bot_f(ent);
	//K2:end
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}

