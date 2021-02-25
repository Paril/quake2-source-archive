#include "g_local.h"
#include "m_player.h"

//Skid - new Header
#include "g_genutil.h"
#include "g_genmidi.h"


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

// Skid - Dont think this needed
	if(gen_ctf->value || gen_team->value)
	{
		if(CheckTeamDamage(ent1,ent2));
			return true;
	}
	else
	{
// End Skid

		if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
			return false;

		strcpy (ent1Team, ClientTeam (ent1));
		strcpy (ent2Team, ClientTeam (ent2));

		if (strcmp(ent1Team, ent2Team) == 0)
			return true;
	}
	return false;
}

void Cmd_WeapNext_f (edict_t *ent);
void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//CurMo
	if (ent->client->showmenu)
	{
		MenuNext(ent);
		return;
	}
//CurMo-END 
	else if (cl->chase_target) 
	{
		ChaseNext(ent);
		return;
	}
	else if(ent->client->resp.player_class != CLASS_Q2)
	{
		Cmd_WeapNext_f (ent);
		return;
	}

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

void Cmd_WeapPrev_f (edict_t *ent);
void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//CurMo
	if (ent->client->showmenu)
	{
		MenuPrev(ent);
		return;
	}
//CurMo-END
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}
	else if(ent->client->resp.player_class != CLASS_Q2)
	{
		Cmd_WeapPrev_f (ent);
		return;
	}


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
void Cmd_Give_f (edict_t *ent, qboolean idkfa)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;
	int			player_class;

	if ((deathmatch->value||coop->value) && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

//Skid added
	player_class = ent->client->resp.player_class;

	name = gi.args();

	if ((Q_stricmp(name, "all") == 0) ||
	    (ent->client->resp.player_class == CLASS_DOOM && idkfa))
	{
		if(ent->client->resp.player_class == CLASS_DOOM && !idkfa)
		{
			gi.cprintf (ent, PRINT_HIGH, "Sorry Doomguy, you'll have to do better than that\n");
			return;
		}
		give_all = true;
	}
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
//Skid - only give him items in his class
			if(player_class == CLASS_Q1 && !(it->flags & IT_Q1))
				continue;
			if(player_class == CLASS_DOOM && !(it->flags & IT_DOOM))
				continue;
			if(player_class == CLASS_WOLF && !(it->flags & IT_WOLF))
				continue;
			if(player_class == CLASS_Q2 && (it->flags & (IT_Q1|IT_DOOM|IT_WOLF)))
				continue;
//end Skid
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

		//Skid added
		switch(player_class)
		{
			case CLASS_Q1:
				{
					ent->client->pers.inventory[Q1_GREEN] = 0;
					ent->client->pers.inventory[Q1_YELLOW] = 0;
					ent->client->pers.inventory[Q1_RED] = 200;
					break;
				}
			case CLASS_DOOM:
				{
					ent->client->pers.inventory[D_GREEN] = 0;
					ent->client->pers.inventory[D_BLUE] = 200;
					break;
				}
			case CLASS_WOLF:
				{

					ent->client->pers.inventory[W_GREEN] =0;
					ent->client->pers.inventory[W_BLUE] = 200;
					break;
				}
			case CLASS_Q2:
				{

					it = FindItem("Jacket Armor");
					ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

					it = FindItem("Combat Armor");
					ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

					it = FindItem("Body Armor");
					info = (gitem_armor_t *)it->info;
					ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;
					break;
				}
		}

		if (!give_all)
			return;
	}

	if ((give_all || Q_stricmp(name, "Power Shield") == 0)
//Skid added
		&& player_class == CLASS_Q2)
//end Skid
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

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
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
			gi.cprintf (ent, PRINT_HIGH, "unknown item %s\n", name);
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
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

	if ((deathmatch->value||coop->value) && !sv_cheats->value)
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

	if ((deathmatch->value||coop->value) && !sv_cheats->value)
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

	if ((deathmatch->value||coop->value) && !sv_cheats->value)
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

//Skid - Command parsing to allow similar weapon bindings 
	if(ent->client->resp.player_class == CLASS_Q1)
	{
		if (!Q_stricmp(s, "Blaster"))    
		{            
			s ="Axe";
		}  
		else if (!Q_stricmp(s, "Shotgun"))    
		{
			s ="Boomstick";       
		} 
		else if (!Q_stricmp(s, "Super Shotgun"))    
		{
			s ="Double Barrelled Shotgun";       
		} 
		else if (!Q_stricmp(s, "Machinegun"))    
		{
			s = "Nailgun";       
		} 
		else if (!Q_stricmp(s, "Chaingun"))    
		{
			s = "Super Nailgun";       
		} 
		else if (!Q_stricmp(s, "Grenade Launcher"))    
		{
			s = "GrenadeLauncher";       
		} 
		else if (!Q_stricmp(s, "Rocket Launcher"))    
		{
			s = "RocketLauncher";       
		} 
		else if (!Q_stricmp(s, "HyperBlaster"))    
		{
			s = "Thunderbolt";       
		}
		else if (!Q_stricmp(s, "Grapple"))
		{
			s = "Morning Star";
		}
	}
	else if(ent->client->resp.player_class == CLASS_DOOM)
	{
		if (!Q_stricmp(s, "Blaster"))    
		{            
			if(ent->client->pers.weapon == FindItem("Chainsaw"))
				s ="Fists";
			else 
			{
				if(ent->client->pers.inventory[D_CSAW])
					s ="Chainsaw";
				else
					s ="Fists";
			}
		}  
		else if (!Q_stricmp(s, "Shotgun"))    
		{
			s ="Pistol";       
		} 
		else if (!Q_stricmp(s, "Super Shotgun"))    
		{
			if(ent->client->pers.weapon == FindItem("Doom Super Shotgun"))
				s ="Doom Shotgun";
			else if(ent->client->pers.weapon == FindItem("Doom Shotgun"))
				s ="Doom Super Shotgun";
			else
			{
				if (ent->client->pers.inventory[D_SSG])
					s ="Doom Super Shotgun";
				else if(ent->client->pers.inventory[D_SG])
					s ="Doom Shotgun";
				else
					s ="Doom Shotgun";
			}
		} 
		else if (!Q_stricmp(s, "Machinegun"))    
		{
			s ="Doom Chaingun"; 
		} 
		else if (!Q_stricmp(s, "Chaingun"))    
		{
			s = "Doom Rocket Launcher"; 
		} 
		else if (!Q_stricmp(s, "Grenade Launcher"))    
		{
			s ="Plasma Rifle";
		} 
		else if (!Q_stricmp(s, "Rocket Launcher"))    
		{
			s = "BFG9000";    
		} 
		else if (!Q_stricmp(s, "HyperBlaster"))    
		{
			s = "Chainsaw";
		}
	}
	else if(ent->client->resp.player_class == CLASS_WOLF)
	{
		if (!Q_stricmp(s, "Blaster"))    
		{            
			s ="Knife";
		}  
		else if (!Q_stricmp(s, "Shotgun"))    
		{
			s ="W Pistol";       
		} 
		else if (!Q_stricmp(s, "Super Shotgun"))    
		{
			s ="W Machinegun";       
		} 
		else if (!Q_stricmp(s, "Machinegun"))    
		{
			s = "Gatling Gun";       
		} 
		else if (!Q_stricmp(s, "Chaingun"))    
		{
			s = "Gatling Guns";
		} 
		else if (!Q_stricmp(s, "Grenade Launcher"))    
		{
			s = "Flamethrower";       
		} 
		else if (!Q_stricmp(s, "Rocket Launcher"))    
		{
			s = "Bazooka";       
		}
	}
//========================================


	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
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
	if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL)
	{
		it->drop (ent, it);
		return;
	}
//ZOID

//Skid added
	if((ent->client->resp.player_class == CLASS_Q1)
		&& (gen_ctf->value || gen_team->value))
	{
		Q1DropAmmoPack(ent);
		return;
	}
	
	if(ent->client->resp.player_class != CLASS_Q2)
		return;
//end Skid

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
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
	//int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

//Skid added
	if (ent->client->showmenu)
	{
		CloseCurMoMenu(ent);
		ent->client->update_chase = true;
		return;
	}
	else if(!((int)genflags->value & (GEN_Q1_ONLY|GEN_DOOM_ONLY|GEN_RANDOM_CLASS)))
	{
		if(gen_ctf->value || 
		  (gen_team->value && ((int)genflags->value & GEN_REDBLUE_TEAMS)))
			GenOpenJoinClassMenu(ent);
		else
			GenOpenJoinMenu(ent);
		return;
	}
//end
	
/*	if (cl->showinventory)
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
	gi.unicast (ent, true);*/
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

//CurMo
	if (ent->client->showmenu)
	{
		UseMenu(ent, 1);
		return;
	}  
//CurMo-END  

//Skid added
	if(ent->client->resp.player_class == CLASS_Q2)
	{
//end Skid
			ValidateSelectedItem (ent);

			if (ent->client->pers.selected_item == -1)
			{
				gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
				return;
			}
	
			it = &itemlist[ent->client->pers.selected_item];
			if (!it->use)
			{
				gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
				return;
			}
			it->use (ent, it);
	}
}

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
		it->use (ent, it);
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
		it->use (ent, it);
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

//CurMo
	if (ent->client->showmenu)
	{
		UseMenu(ent, 0);
		return;
	}
//CurMo-END

	ValidateSelectedItem (ent);

//Skid added
	if((ent->client->resp.player_class == CLASS_Q1)
		&& (gen_ctf->value || gen_team->value))
	{
		Q1DropAmmoPack(ent);
		return;
	}

	if(ent->client->resp.player_class != CLASS_Q2)
		return;
//end Skid

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
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
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);

/*
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);
*/
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

//CurMo
	if (ent->client->showmenu)
	{
		UseMenu(ent, 0);
		ent->client->update_chase = true;
		return;
	}
//CurMo-END
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
	char	lsmall[64];
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
		Com_sprintf (lsmall, sizeof(lsmall), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (lsmall) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, lsmall);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
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
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

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

	if (flood_msgs->value) 
	{
		cl = ent->client;

        if (level.time < cl->flood_locktill) 
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) 
		{
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
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
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
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) 
	{
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) 
		{
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
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
//Zoid	
	else if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
		if(gen_ctf->value || gen_team->value)
			CTFSay_Team(ent, gi.args());
		else
			Cmd_Say_f (ent, true, false);
		return;
	}
//Zoid
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
		Cmd_Give_f (ent,false);
	else if (Q_stricmp (cmd, "god") == 0)
	{
//SKid added
		if(ent->client->resp.player_class != CLASS_DOOM)
			Cmd_God_f (ent);
		else
			gi.cprintf (ent, PRINT_HIGH, "Sorry Doomguy, you'll have to do better than that\n");
	}
	else if((Q_stricmp (cmd, "iddqd")==0) && ent->client->resp.player_class == CLASS_DOOM)
		Cmd_God_f(ent);
	else if((Q_stricmp (cmd, "idkfa")== 0) && ent->client->resp.player_class == CLASS_DOOM)
		Cmd_Give_f (ent,true);
//end Skid
	else if (Q_stricmp (cmd, "notarget") == 0)
			Cmd_Notarget_f (ent);
	else if  (Q_stricmp (cmd, "noclip") ==0)
	{
//SKid added
		if(ent->client->resp.player_class != CLASS_DOOM)
			Cmd_Noclip_f (ent);
		else
			gi.cprintf (ent, PRINT_HIGH, "Sorry Doomguy, you'll have to do better than that\n");
	}
	else if((Q_stricmp (cmd, "idclip") ==0) && ent->client->resp.player_class == CLASS_DOOM)
			Cmd_Noclip_f (ent);
//end Skid
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

//Skid - new client commands	
	else if (Q_stricmp (cmd, "team") == 0)
		 CTFTeam_f (ent);
	else if (Q_stricmp (cmd, "class") == 0)
		 Gen_Class_f(ent);
//Use key
	else if(Q_stricmp (cmd, "buse") == 0)
		OldDudesUseState(ent);
//CTF
	else if (Q_stricmp (cmd, "hook") == 0)
		 Offhand_Grapple_Fire (ent);
	else if (Q_stricmp (cmd, "unhook") == 0)
		 Offhand_Grapple_Drop (ent);
	else if (Q_stricmp (cmd, "id") ==0 )
		 CTFID_f(ent);
//CHASECAM
	else if (Q_stricmp (cmd, "chasecam") == 0)
            Cmd_Chasecam_Toggle (ent);
//MIDI Stuff
#ifdef _WIN32	
	else if(Q_stricmp(cmd, "mplay") == 0)
			//GenMidiCmd(ent,MCMD_PLAY);
			GemMidiCommand(ent,MCMD_PLAY);
	else if(Q_stricmp(cmd, "mstop") == 0)
			GenMidiCmd(ent,MCMD_STOP);
	else if(Q_stricmp(cmd, "mresume") == 0)
			GenMidiCmd(ent,MCMD_RESUME);
	else if(Q_stricmp(cmd, "mpause") == 0)
			GenMidiCmd(ent,MCMD_PAUSE);
#endif
	
	else if(Q_stricmp(cmd, "myip") == 0)
	{
		gi.dprintf("IP addr :%s\n",ent->client->sock.ipaddr);
	}
	else if(Q_stricmp(cmd,"showorg")== 0)
	{
		gi.dprintf("X :%f\nY :%f\nZ :%f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
	}
/*	else if(Q_stricmp(cmd,"d_ir")==0)
	{
		if(ent->client->ps.rdflags & RDF_IRGOGGLES)
		{
gi.dprintf("Toggle IR OFF\n");
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
		}
		else
		{
gi.dprintf("Toggle IR ON\n");
			ent->client->ps.rdflags |= RDF_IRGOGGLES;
		}
	}
	else if(Q_stricmp(cmd,"d_uv")==0)
	{
		if(ent->client->ps.rdflags & RDF_UVGOGGLES)
		{
gi.dprintf("Toggle UV Off\n");
			ent->client->ps.rdflags &= ~RDF_UVGOGGLES;
		}
		else
		{
gi.dprintf("Toggle UV On\n");
			ent->client->ps.rdflags |= RDF_UVGOGGLES;
		}
	}
*/
	else
	{
		// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
	}
}


//ROGUE
/*
#define RF_IR_VISIBLE		0x00008000		// 32768
#define	RF_SHELL_DOUBLE		0x00010000		// 65536
#define	RF_SHELL_HALF_DAM	0x00020000
#define RF_USE_DISGUISE		0x00040000
//ROGUE

//ROGUE
#define	RDF_IRGOGGLES		4
#define RDF_UVGOGGLES		8
*/