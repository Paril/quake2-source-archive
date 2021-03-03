#include "g_local.h"
#include "m_player.h"

#ifdef BOT
#include "bl_cmd.h"
#include "bl_main.h"
#include "bl_redirgi.h"
#endif //BOT

#ifdef OBSERVER
#include "p_observer.h"
#endif //OBSERVER

#ifdef CLIENTLAG
#include "p_lag.h"
#endif //CLIENTLAG


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

	//if one of the entities isn't a client
	if (!ent1->client || !ent2->client) return false;
#ifdef ZOID
	if (ctf->value)
	{
		return ent1->client->resp.ctf_team == ent2->client->resp.ctf_team;
	} //end if
#endif //ZOID
#ifdef AQ2
	if (aq2->value && teamplay->value)
	{
		return ent1->client->resp.team == ent2->client->resp.team;
	} //endif
#endif //AQ2

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

#ifdef ZOID
	if (ctf->value && cl->menu)
	{
		PMenu_Next(ent);
		return;
	} //end if
	else
#endif //ZOID
		if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	} //end if

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

#ifdef ZOID
	if (ctf->value && cl->menu)
	{
		PMenu_Prev(ent);
		return;
	} //end if
	else
#endif //ZOID
		if (cl->chase_target)
	{
		ChasePrev(ent);
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
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
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

#ifdef BOT
	if (give_all || Q_stricmp(name, "Power Screen") == 0)
	{
		it = FindItem("Power Screen");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}
#endif //BOT

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
#ifdef ROGUE
			if (it->flags & IT_NOT_GIVEABLE)					// ROGUE
				continue;										// ROGUE
#endif //ROGUE
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
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

#ifdef ROGUE
	if (it->flags & IT_NOT_GIVEABLE)		
	{
		gi.dprintf ("item cannot be given\n");
		return;							
	}
#endif //ROGUE

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
#ifdef ROGUE
		// PMM - since some items don't actually spawn when you say to ..
		if (!it_ent->inuse)
			return;
		// pmm
#endif //ROGUE
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
#ifdef AQ2
	if (aq2->value)
	{
		//zucc - check for "special"
		if ( stricmp(s, "special") == 0 )
		{
			ReadySpecialWeapon( ent );
			return;
		} //end if
		//zucc - alias names
		if (!stricmp(s, "blaster") || !stricmp(s, "mark 23 pistol")) s = MK23_NAME;
		if (!stricmp(s, "A 2nd pistol") || !stricmp(s, "railgun")) s = DUAL_NAME;
		if (!stricmp(s, "shotgun")) s = M3_NAME;
		if (!stricmp(s, "machinegun")) s = HC_NAME;
		if (!stricmp(s, "super shotgun")) s = MP5_NAME;
		if (!stricmp(s, "chaingun")) s = SNIPER_NAME;
		if (!stricmp(s, "bfg10k")) s = KNIFE_NAME;
		if (!stricmp(s, "grenade launcher")) s = M4_NAME;
		if (!stricmp(s, "grenades")) s = GRENADE_NAME;
	} //end if
#endif //AQ2
	it = FindItem (s);
	if (!it
#ifdef AQ2
		|| (aq2->value && ent->solid == SOLID_NOT && ent->deadflag != DEAD_DEAD)
#endif //AQ2
		)
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
#ifdef XATRIX
		// RAFAEL
		if (strcmp (it->pickup_name, "HyperBlaster") == 0)
		{
			it = FindItem ("Ionripper");
			if (!it)
			{
				gi.cprintf (ent, PRINT_HIGH, "unknown item: Ionripper\n");
				return;
			}
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		// RAFAEL
		else if (strcmp (it->pickup_name, "Railgun") == 0)
		{
			it = FindItem ("Phalanx");
			if (!it)
			{
				gi.cprintf (ent, PRINT_HIGH, "unknown item: Phalanx");
				return;
			}
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
#endif //XATRIX
		{
			gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
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

#ifdef ZOID //--special case for tech powerups
	if (ctf->value)
	{
		if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL)
		{
			it->drop (ent, it);
			return;
		} //end if
	} //end if
#endif //ZOID

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
#ifdef XATRIX
		// RAFAEL
		if (strcmp (it->pickup_name, "HyperBlaster") == 0)
		{
			it = FindItem ("Ionripper");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		// RAFAEL
		else if (strcmp (it->pickup_name, "Railgun") == 0)
		{
			it = FindItem ("Phalanx");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
#endif //XATRIX
		{
			gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
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

	cl->showscores = false;
	cl->showhelp = false;

#ifdef ZOID
	if (ctf->value)
	{
		if (ent->client->menu)
		{
			PMenu_Close(ent);
			ent->client->update_chase = true;
			return;
		}
	} //end if
#endif //ZOID
#ifdef BOT
	if (ent->client->showmenu)
	{
		ent->client->menustate.showmenu = false;
		ent->client->showmenu = false;
		ent->client->menustate.redrawmenu = false;
		ent->client->menustate.removemenu = true;
	} //end if
#endif //BOT

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

#ifdef ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM)
	{
		CTFOpenJoinMenu(ent);
		return;
	} //end if
#endif //ZOID

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

#ifdef ZOID
	if (ctf->value)
	{
		if (ent->client->menu)
		{
			PMenu_Select(ent);
			return;
		}
	} //end if
#endif //ZOID

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

#ifdef ZOID
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
#endif //ZOID

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
#ifdef ROGUE
		// PMM - prevent scrolling through ALL weapons
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
#else //ROGUE
		index = (selected_weapon + i)%MAX_ITEMS;
#endif //ROGUE
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
#ifdef ROGUE
		// PMM - prevent scrolling through ALL weapons
		if (cl->newweapon == it)
			return;
#else //ROGUE
		if (cl->pers.weapon == it)
			return;	// successful
#endif //ROGUE
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
#ifdef ROGUE
		// PMM - prevent scrolling through ALL weapons
		index = (selected_weapon + i)%MAX_ITEMS;
#else //ROGUE
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
#endif //ROGUE
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
#ifdef ROGUE
		// PMM - prevent scrolling through ALL weapons
		if (cl->newweapon == it)
			return;
#else
		if (cl->pers.weapon == it)
			return;	// successful
#endif //ROGUE
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

#ifdef ZOID //for the observer mode
	if (ent->solid == SOLID_NOT)
		return;
#endif //ZOID

	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
#ifdef ROGUE
	// make sure no trackers are still hurting us.
	if(ent->client->tracker_pain_framenum)
		RemoveAttackingPainDaemons (ent);

	if (ent->client->owned_sphere)
	{
		G_FreeEdict(ent->client->owned_sphere);
		ent->client->owned_sphere = NULL;
	}
#endif //ROGUE
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
#ifdef ZOID
	if (ctf->value)
	{
		if (ent->client->menu) PMenu_Close(ent);
		ent->client->update_chase = true;
	} //end if
#endif //ZOID
#ifdef BOT
	if (ent->client->showmenu)
	{
		ent->client->menustate.showmenu = false;
		ent->client->showmenu = false;
		ent->client->menustate.redrawmenu = false;
		ent->client->menustate.removemenu = true;
	} //end if
#endif //BOT
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
	char		dsmall[64];
	char		large[1280];
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
		Com_sprintf (dsmall, sizeof(dsmall), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (dsmall) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, dsmall);
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
	char	outtext[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

#ifdef AQ2
	if (aq2->value && !teamplay->value)
#endif //AQ2
	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
#ifdef ZOID
		&& !ctf->value
#endif //ZOID
		)
		team = false;

#ifdef AQ2
	if (aq2->value)
	{
		if (team)
			Com_sprintf (text, sizeof(text), "%s(%s): ", 
				(teamplay->value && ent->solid == SOLID_NOT) ? "[DEAD] " : "",
				ent->client->pers.netname);
		else if (partner_msg)
			Com_sprintf (text, sizeof(text), "[%sPARTNER] %s: ", 
				(ent->solid == SOLID_NOT) ? "DEAD " : "",
				ent->client->pers.netname);
		else
			Com_sprintf (text, sizeof(text), "%s%s: ", 
				(teamplay->value && ent->solid == SOLID_NOT) ? "[DEAD] " : "",
				ent->client->pers.netname);
	} //end if
	else
#endif //AQ2
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

#ifdef AQ2
	if (aq2->value)
	{
		// don't let text be too long for malicious reasons
		// ...doubled this limit for Axshun -FB
		if (strlen(text) > 300)
			text[300] = 0;
		if (ent->solid != SOLID_NOT && ent->deadflag != DEAD_DEAD)
			ParseSayText(ent, text); // this will parse the % variables, and again check 300 limit afterwards -FB
	} //end if
	else
#endif //AQ2
	{
		// don't let text be too long for malicious reasons
		if (strlen(text) > 150)
			text[150] = 0;

#ifdef ZOID
		if (team)
		{
			CTFSay_Team(ent, text, outtext);
			strcpy(text, outtext);
		} //end if
	} //end else
#endif //CTF

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
		if (!other->client)
			continue;
		if (team)
		{
#ifdef OBSERVER
			if (!(ent->flags & FL_OBSERVER) && (other->flags & FL_OBSERVER)) continue;
			else if ((ent->flags & FL_OBSERVER) && !(other->flags & FL_OBSERVER)) continue;
			else if (!((ent->flags & FL_OBSERVER) && (other->flags & FL_OBSERVER)))
#endif //OBSERVER
			if (!OnSameTeam(ent, other))
				continue;
		}
#ifdef AQ2
		if (aq2->value)
		{
			if (partner_msg)
			{
				if (other != ent->client->resp.radio_partner && other != ent)
					continue;
			}
			if (teamplay->value && team_round_going)
			{
				if (ent->solid == SOLID_NOT && other->solid != SOLID_NOT)
					continue;
			}
		} //end if
#endif //AQ2
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

#ifdef ROGUE
void Cmd_Ent_Count_f (edict_t *ent)
{
	int		x;
	edict_t	*e;

	x=0;

	for (e=g_edicts;e < &g_edicts[globals.num_edicts] ; e++)
	{
		if(e->inuse)
			x++;
	}

	gi.dprintf("%d entites active\n", x);
}
#endif //ROGUE

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
	if (Q_stricmp (cmd, "say_team") == 0)
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
	{
#ifdef BOT
		if (!MenuDown(ent))
#endif //BOT
			SelectNextItem (ent, -1);
	} //end else if
	else if (Q_stricmp (cmd, "invprev") == 0)
	{
#ifdef BOT
		if (!MenuUp(ent))
#endif //BOT
			SelectPrevItem (ent, -1);
	} //end else if
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
	{
#ifdef BOT
		if (!MenuForward(ent))
#endif //BOT
				Cmd_InvUse_f(ent);
	}
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
#ifdef CLIENTLAG
	else if (Q_stricmp(cmd, "lag") == 0)
		Lag_SetClientLag(ent, atoi(gi.argv(1)));
	else if (Q_stricmp(cmd, "lagvariance") == 0)
		Lag_SetClientLagVariance(ent, atoi(gi.argv(1)));
#endif //CLIENTLAG
#ifdef AQ2
	else if (aq2->value && Q_stricmp (cmd, "reload") == 0)
		Cmd_Reload_f(ent);
	else if (aq2->value && Q_stricmp (cmd, "weapon") == 0)
		Cmd_Weapon_f(ent);
	else if (aq2->value && Q_stricmp (cmd, "opendoor") == 0)
		Cmd_OpenDoor_f(ent);
	else if (aq2->value && Q_stricmp (cmd, "bandage") == 0)
		Cmd_Bandage_f(ent);
	else if (aq2->value && Q_stricmp (cmd, "id") == 0)
		Cmd_ID_f (ent);
	else if (aq2->value && Q_stricmp (cmd, "irvision") == 0)
		Cmd_IR_f (ent);
	else if (aq2->value && Q_stricmp(cmd, "team") == 0 && teamplay->value)
		Team_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "radio") == 0)
		Cmd_Radio_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "radiogender") == 0)
		Cmd_Radiogender_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "radio_power") == 0)
		Cmd_Radio_power_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "radiopartner") == 0)
		Cmd_Radiopartner_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "radioteam") == 0)
		Cmd_Radioteam_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "channel") == 0)
		Cmd_Channel_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "say_partner") == 0)
		Cmd_Say_partner_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "partner") == 0)
		Cmd_Partner_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "unpartner") == 0)
		Cmd_Unpartner_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "motd") == 0)
		PrintMOTD(ent);
	else if (aq2->value && Q_stricmp(cmd, "deny") == 0)
		Cmd_Deny_f(ent);
	else if (aq2->value && Q_stricmp(cmd, "choose") == 0)
		Cmd_Choose_f(ent);
#endif //AQ2
#ifdef ROGUE
	else if (Q_stricmp (cmd, "entcount") == 0)		// PGM
		Cmd_Ent_Count_f (ent);						// PGM
	else if (Q_stricmp (cmd, "disguise") == 0)		// PGM
	{
		ent->flags |= FL_DISGUISED;
	}
#endif //ROGUE
#ifdef ROCKETARENA
	else if (Q_stricmp (cmd, "toarena") == 0)
		Cmd_toarena_f(ent, atoi(gi.argv(1)));
	else if (Q_stricmp (cmd, "mstart") == 0)
		Cmd_start_match_f(ent, ent->client->resp.context);
	else if (Q_stricmp (cmd, "mstop") == 0)
		Cmd_stop_match_f(ent, ent->client->resp.context, 0);
#endif //ROCKETARENA
#ifdef OBSERVER
	else if (ClientObserverCmd(cmd, ent))
	{
	} //end else if
#endif //OBSERVER

#ifdef BOT
	//bot commands
	else if (BotCmd(cmd, ent, false))
	{
	} //end else if
#endif //BOT

#ifdef ZOID
	else if (ctf->value && Q_stricmp(cmd, "team") == 0)
	{
		CTFTeam_f(ent);
	} //end else if
	else if (ctf->value && Q_stricmp(cmd, "id") == 0)
	{
		CTFID_f(ent);
	} //end else if
#endif //ZOID
#ifdef CTF_HOOK
	else if (ctf->value && Q_stricmp(cmd, "hookon") == 0)
	{
		ent->client->ctf_hookstate = CTF_HOOK_STATE_ON;
	} //end else if
	else if (ctf->value && Q_stricmp(cmd, "hookoff") == 0)
	{
		if (ent->client->ctf_hookstate & CTF_HOOK_STATE_ON)
				ent->client->ctf_hookstate |= CTF_HOOK_STATE_TURNOFF;
	} //end else if
#endif //CTF_HOOK
	else
	{
#ifdef BOT
		//assume team message if in a teamplay game
		if (((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)) || ctf->value)
		{
			Cmd_Say_f(ent, true, true);
		} //end if
		else
#endif //BOT
			// anything that doesn't match a command will be a chat
			Cmd_Say_f(ent, false, true);
	} //end else
#ifdef BOT
	BotClearCommandArguments();
#endif //BOT
}
