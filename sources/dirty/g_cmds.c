#include "g_local.h"
#include "m_player.h"

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

// GRIM
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
        if ((!max_teams) && (!coop->value))
                return false;

        if ((!ent1->client) || (!ent2->client))
                return false;

        if ((ent1->client->resp.team == NULL) || (ent2->client->resp.team == NULL))
                return false;

        if (ent1->client->resp.team != ent2->client->resp.team)
                return false;

        return true;
}
// GRIM


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

        // GRIM
	if (cl->menu)
        {
                if (level.time - ent->menu_time > 0.1)
                        PMenu_Next(ent);
		return;
	}
        else if (cl->chase_target) // GRIM
        {
                if (level.time - ent->menu_time > 0.1)
                        ChaseNext(ent);
		return;
	}

// GRIM
        if (ent->deadflag)
		return;
// GRIM

// GRIM
         //Searching a corpse.
        if (ent->client->pers.grabbing == 2 && ent->corpse && (!ent->corpse->client))
        {
                SelectNextCorpseItem (ent, itflags);
                return;
        }
// GRIM

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
                if (cl->pers.inventory[index] < 1)
			continue;
                it = &itemlist[index];
// GRIM
                if ((!it->drop) && (!it->use))
			continue;
// GRIM

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

        // GRIM
	if (cl->menu)
        {
                if (level.time - ent->menu_time > 0.1)
                        PMenu_Prev(ent);
		return;
	}
        else if (cl->chase_target) // GRIM
        {
                if (level.time - ent->menu_time > 0.1)
                        ChasePrev(ent);
		return;
	}

// GRIM
        if (ent->deadflag)
		return;
// GRIM

// GRIM
         //Searching a corpse.
        if (ent->client->pers.grabbing == 2 && ent->corpse && (!ent->corpse->client))
        {
                SelectPrevCorpseItem (ent, itflags);
                return;
        }
// GRIM

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
                if (cl->pers.inventory[index] < 1)
			continue;
		it = &itemlist[index];
// GRIM
                if ((!it->drop) && (!it->use))
			continue;
// GRIM

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

// GRIM
        if (ent->deadflag)
		return;
// GRIM

        // GRIM - HACK! Problems with grenades and shit.
                // Just prevent exploiting them
        if (cl->pers.inventory[cl->pers.selected_item] < 1)
        {
                cl->pers.inventory[cl->pers.selected_item] = 0; // NO FUCKING NEGATIVES!!!
                SelectNextItem (ent, -1);
                return;         // NOT valid
        }
        // GRIM
}


//=================================================================================

// GRIM
/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
        edict_t         *given;
        gitem_t         *item;
	char		*name;
        int             i;

        if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

        if (Q_stricmp(gi.argv(1), "health") == 0)
	{
                if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
                ent->client->concus_time = 0;

                memset(ent->client->pers.wounds, 0, sizeof(ent->client->pers.wounds));
                return;
	}

        if (Q_stricmp(gi.argv(1), "concussion") == 0)
	{
                if (gi.argc() == 3)
                        ent->client->concus_time = level.time + atoi(gi.argv(2));
		else
                        ent->client->concus_time = level.time + ((rand()%5) * 0.5) + 0.5;
                return;
	}

        if (Q_stricmp(gi.argv(1), "wound") == 0)
        {
                if (Q_stricmp(gi.argv(2), "leg") == 0)
                {
                        if (gi.argc() == 4)
                                i = atoi(gi.argv(3));
                        else
                                i = 1;
                        ent->client->pers.wounds[W_RLEG] = i;
                        ent->client->pers.wounds[W_LLEG] = i;
                }
                else if (Q_stricmp(gi.argv(2), "arm") == 0)
                {
                        if (gi.argc() == 4)
                                i = atoi(gi.argv(3));
                        else
                                i = 1;
                        ent->client->pers.wounds[W_RARM] = i;
                        ent->client->pers.wounds[W_LARM] = i;
                }
                else if (Q_stricmp(gi.argv(2), "head") == 0)
                {
                        if (gi.argc() == 4)
                                ent->client->pers.wounds[W_HEAD] = atoi(gi.argv(3));
                        else
                                ent->client->pers.wounds[W_HEAD]++;
                }
                else if (Q_stricmp(gi.argv(2), "chest") == 0)
                {
                        if (gi.argc() == 4)
                                ent->client->pers.wounds[W_CHEST] = atoi(gi.argv(3));
                        else
                                ent->client->pers.wounds[W_CHEST]++;
                }
                else if (gi.argc() == 3)
                        ent->client->pers.wounds[W_CHEST] = atoi(gi.argv(2));
                else
                        ent->client->pers.wounds[W_CHEST]++;
                return;
	}

        item = FindItem (name);
        if (!item)
	{
		name = gi.argv(1);
                item = FindItem (name);
                if (!item)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

        if (!item->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

        given = Drop_Item (ent, item, ITEM_UNDEF);
        given->spawnflags &= ~DROPPED_PLAYER_ITEM;
        given->spawnflags |= DROPPED_ITEM;
}
// GRIM


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

        // GRIM
        //stuffcmd(ent, "cl_sidespeed 200\n cl_forwardspeed 200\n");

        memset(ent->client->pers.wounds, 0, sizeof(ent->client->pers.wounds));
        ent->client->pers.heal_rate = 9;
        // GRIM

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

// GRIM
        if (ent->deadflag)
		return;
// GRIM

        // Dirty
        if ((ent->client->weaponstate == WEAPON_FIRING)
         || (ent->client->weaponstate == WEAPON_FIRING2)
          || (ent->client->weaponstate == WEAPON_FIRING3))
		return;
        // Dirty

        s = gi.args();

        // Dirty
        if (!Q_stricmp (s, "blaster"))
                s = "MK23 Pistol";
        else if (!Q_stricmp (s, "shotgun"))
                s = "M3 Super 90 Assault Shotgun";
        else if (!Q_stricmp (s, "super shotgun"))
                s = "Handcannon";
        else if (!Q_stricmp (s, "machinegun"))
                s = "MP5/10 Submachinegun";
        else if (!Q_stricmp (s, "chaingun"))
                s = "Sniper Rifle";
        else if (!Q_stricmp (s, "grenade launcher"))
                s = "Sniper Rifle";
        else if (!Q_stricmp (s, "rocket launcher"))
                s = "Sniper Rifle";
        else if (!Q_stricmp (s, "grenade"))
                s = "M26 Fragmentation Grenade";
        else if (!Q_stricmp (s, "railgun"))
                s = "Combat Knife";
        else if (!Q_stricmp (s, "hyper blaster"))
                s = "M4 Assault Rifle";
        else if (!Q_stricmp (s, "BFG10K"))
                s = "M4 Assault Rifle";
        // Dirty

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

        // GRIM - No Negative things
        if (ent->client->pers.inventory[index] < 1)
	{
                if (ent->client->pers.inventory[index] < 0)
                {
                        gi.dprintf ("Cmd_Use_f - tried to use a negative item\n");
                        ent->client->pers.inventory[index] = 0;
                }
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
        // GRIM

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
        int     index;
        gitem_t *it;
        char    *s;

// GRIM
        if (ent->client->weaponstate != WEAPON_READY)
		return;

        if (ent->deadflag)
		return;
// GRIM

	s = gi.args();

        // Dirty
        if ( Q_stricmp(s, "weapon") == 0 )
        {
                Cmd_DropWepQuick (ent);
                return;
        }
        // Dirty

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
        if (ent->client->pers.inventory[index] < 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);

        // GRIM
        if (ent->client->showinventory)
        {
                gi.WriteByte (svc_inventory);
                for (index=0 ; index<MAX_ITEMS ; index++)
                        gi.WriteShort (ent->client->pers.inventory[index]);
                gi.unicast (ent, false);
        }
        // GRIM
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

        // DTEMP
        //cl->showscores = false;
        //cl->showhelp = false;
        // DTEMP

        // GRIM
	if (ent->client->menu)
        {
                if (level.time - ent->menu_time > 0.1)
                {
                        cl->showscores = false;
                        PMenu_Close(ent);
                        ent->client->update_chase = true;
                }
                return;
	}
        // GRIM

        // DTEMP
        if (cl->showscores || cl->showhelp)
        {
                cl->showscores = false;
                cl->showhelp = false;
		return;
	}
        // DTEMP

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

// GRIM
        if (deathmatch->value)
        {
                if (cl->resp.state == CS_NOT_READY)
                {
                        if (level.time - ent->menu_time > 0.1)
                        {
                                ent->menu_time = level.time;

                                // If not...if, doesn't make a difference...
                                StartClient(ent);
                        }
                        //OpenChoosingMenu(ent);
                        return;
                }
                else if ((ent->movetype == MOVETYPE_NOCLIP) || (ent->deadflag == DEAD_DEAD))
                {
                        ChangeMenu(ent);
                        return;
                }
        }
 // GRIM

// GRIM
        if (ent->deadflag)
		return;

        if (ent->client->weaponstate != WEAPON_READY)
		return;
// GRIM

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

        // GRIM
        if (ent->client->menu)
        {
                if (level.time - ent->menu_time > 0.1)
                        PMenu_Select(ent);
		return;
	}
        // GRIM

// GRIM
        if (ent->deadflag)
		return;

        // Dirty
        if ((ent->client->weaponstate == WEAPON_FIRING)
         || (ent->client->weaponstate == WEAPON_FIRING2)
          || (ent->client->weaponstate == WEAPON_FIRING3))
		return;
        // Dirty

         //Searching a corpse.
        if (ent->client->pers.grabbing == 2 && ent->corpse && (!ent->corpse->client))
        {
                CorpseDropItem (ent);
                return;
        }
// GRIM

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

//ZOID // GRIM
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
//ZOID // GRIM

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
                if (cl->pers.inventory[index] < 1)
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
                if (!(it->flags & IT_WEAPON))
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
                if (cl->pers.inventory[index] < 1)
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
// GRIM
qboolean ChangeLast (edict_t *ent, gitem_t *weapon, gitem_t *weapon2)
{
        gitem_t *it;
        int     index;

        if (weapon == NULL)
                return false;

        index = ITEM_INDEX(weapon);
        if (ent->client->pers.inventory[index] < 1)
        {
                ent->client->pers.inventory[index] = 0;
                return false;
        }

	it = &itemlist[index];

	if (!it->use)
                return false;

        if (!(it->flags & IT_WEAPON))
                return false;

        if (weapon2 && (weapon->flags & IT_TWIN))
        {
                index = ITEM_INDEX(weapon2);
                if (!Q_stricmp(weapon2->pickup_name, weapon->pickup_name))
                {
                        if (ent->client->pers.inventory[index] > 1)
                                ent->client->newweapon2 = weapon2;
                }
                else if (ent->client->pers.inventory[index] > 0)
                        ent->client->newweapon2 = weapon2;
        }

	it->use (ent, it);
	return true;
}

void SetHands (edict_t *ent);
void Cmd_WeapLast_f (edict_t *ent)
{
        if (ChangeLast (ent, ent->client->pers.lastweapon, ent->client->pers.lastweapon2))
                return;
        else if (ent->client->pers.largeweapon && ent->client->pers.largeweapon->pickup_name)
                ChangeLast (ent, FindItem(ent->client->pers.largeweapon->pickup_name), NULL);
        else if (ChangeLast (ent, ent->client->pers.prev_weapon, ent->client->pers.prev_weapon2))
                return;

        if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
                SetHands (ent);
        else
                Cmd_WeapPrev_f (ent);
}
// GRIM

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
        gitem_t *it;
        int     i; // GRIM

// GRIM
        if (ent->deadflag)
		return;

        if (ent->client->weaponstate != WEAPON_READY)
		return;
// GRIM

         //Searching a corpse.
        if (ent->client->pers.grabbing == 2 && ent->corpse && (!ent->corpse->client))
        {
                CorpseDropItem (ent);
                return;
        }

        ValidateSelectedItem (ent);
// GRIM

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

        // GRIM
        if (ent->client->showinventory)
        {
                gi.WriteByte (svc_inventory);
                for (i=0 ; i<MAX_ITEMS ; i++)
                        gi.WriteShort (ent->client->pers.inventory[i]);
                gi.unicast (ent, false);
        }
        // GRIM
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
        if (ent->deadflag || (ent->movetype == MOVETYPE_NOCLIP))
		return;

        // Dirty
        if (ent->is_a_lame_mofo)
        {       // 3 times as long for a TK'r
                if ((level.time - ent->client->respawn_time) < 15)
                        return;
        }
        else if ((level.time - ent->client->respawn_time) < 5)
                return;
        // Dirty

	ent->health = 0;
	ent->flags &= ~FL_GODMODE;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);

        // don't even bother waiting for death frames
        // and stop holograms being made.
        ent->deadflag = DEAD_DEAD;
        ent->s.frame = FRAME_crdeath5;
        ent->client->anim_end  = ent->s.frame;
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
        // GRIM
	if (ent->client->menu)
		PMenu_Close(ent);

        ent->client->update_chase = true;
        // GRIM

	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
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

// GRIM
/*
=================
Cmd_Wave_f
=================
*/

void wave(edict_t *ent, int waveNum)
{
	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (waveNum)
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

void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	wave(ent, i);
}
// GRIM

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
	qboolean        observers_only = false; // GRIM

	if (gi.argc () < 2 && !arg0)
		return;

// GRIM
//        if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
//                team = false;

        if (!FloodCheck (ent))
                return;

        if (!max_teams)
                team = false;

        if (turns_on)
        {
                if ((!max_teams) && level.living < 3)
                        observers_only = false;
                else if ((level.match_state < MATCH_START) || (level.match_state == MATCH_FINISHED))
                        observers_only = false;
                else if (ent->client->resp.state < CS_PLAYING)
                        observers_only = true;
        }

        if (team && ent->client->resp.team)
        {
                if (observers_only)
                        Com_sprintf (text, sizeof(text), "([NON]%s): ", ent->client->pers.netname);
                else
                        Com_sprintf (text, sizeof(text), "(%s[%s]): ", ent->client->pers.netname, ent->client->resp.team->name);
        }
        else if (observers_only)
                Com_sprintf (text, sizeof(text), "[NON]%s: ", ent->client->pers.netname);
        else if (max_teams && ent->client->resp.team)
                Com_sprintf (text, sizeof(text), "%s[%s]: ", ent->client->pers.netname, ent->client->resp.team->name);
	else
                Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);
// GRIM

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

                // GRIM
                if (observers_only && (other->client->resp.state == CS_PLAYING))
                        continue;
                if (team && (!OnSameTeam (other, ent)))
                        continue;
                // GRIM

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

// GRIM
void Cmd_Slot (edict_t *ent)
{
        weapons_t       *wep;
	char		*name;

	name = gi.args();
        wep = FindSlot(ent, name);

        if (wep == NULL)
        {
                gi.cprintf(ent, PRINT_HIGH, "Unknown slot %s\n", name);
                return;
        }

        gi.cprintf(ent, PRINT_HIGH, "Slot Name = %s, Weapon = %s, Ammo = %s, Quantity = %i\n", wep->slot_name, wep->pickup_name, wep->ammo, wep->quantity);
}
// GRIM

// TEMP
void ReSetStuff (edict_t *dude)
{
	edict_t	*ent;
        teams_t *team;
        int     i, living, participants, n;

        if (deathmatch->value && !sv_cheats->value)
	{
                gi.cprintf (dude, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

        UnHideAllSpots();

        if (max_teams)
        {
                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("SetupMatch - No team at index %i.\n", i);
                                break;
                        }
                        team->living = 0;
                        team->score = 0;
                        team->spot = NULL;
                        if (team->members > 0)
                                GiveTeamASpot(team);
                }
        }

        living = 0;
        participants = 0;

        // Go through everything and get rid of what shouldn't be around
        // anymore, respawn that which should come back etc.
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

                if (i > 0)
		{
                        // Start the players...
                        if (i <= maxclients->value)
                        {
                                if (ent->client->resp.state > CS_NOT_READY)
                                {
                                        ent->client->resp.state = CS_PLAYING;
                                        ent->pre_solid = 1;
                                        ent->pre_solid_time = level.time + 3;
                                        if (ent->movetype == MOVETYPE_NOCLIP)
                                                ThrowEmIn(ent);
                                        if (ent->client->resp.team)
                                                ent->client->resp.team->living++;
                                        living++;
                                        participants++;
                                }
                                continue;
                        }
                        // Hide any corpses...
                        else if (i <= (maxclients->value + BODY_QUEUE_SIZE))
                        {
                                ent->solid = SOLID_NOT;
                                ent->svflags |= SVF_NOCLIENT;
                                gi.linkentity (ent);
                                continue;
                        }
		}

                // Remove gibs, skulls etc...
                if (ent->deadflag && (!ent->client))
                {
                        G_FreeEdict (ent);
			continue;
                }

                // Remove any dropped items/techs,
                // respawn flags and other items...
                if (ent->item)
                {
                        if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                        {
                                if (ent->item->pickup == CTFPickup_Flag)
                                        CTFDropFlagThink(ent);
                                else
                                        G_FreeEdict (ent);
                        }
                        else if (ent->item->flags & IT_RAND)
                                G_FreeEdict (ent);
                        else if (ent->solid == SOLID_NOT)
                                DoRespawn (ent);
                }

                // Paranoid
                if (ent->flags & FL_RESPAWN)
                {
                        if (ent->reset)
                        {
                                ent->reset (ent);
                                //if (ent->classname)
                                //        gi.dprintf ("SM : %s at %s.\n", ent->classname, vtos(ent->s.origin));
                        }
                        else
                                ent->flags &= ~FL_RESPAWN;
                }
                // Paranoid
	}

        if (max_teams)
        {
                living = 0;
                participants = 0;
                team = NULL;
                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("SetupMatch - No team at index %i.\n", i);
                                break;
                        }

                        if (team->living > 0)
                        {
                                living++;
                                if (ctf->value)
                                {
                                        n = TeamGoal(team);
                                        participants += n;
                                }
                                else
                                        participants++;
                        }
                }
        }

        level.living = living;
        level.participants = participants;

        HideAllSpots();

        //level.match_state = MATCH_START; // Begin match
        //level.match_time = level.time + 3;

        teamgame.last_score = level.time - 5;
        teamgame.last_score_team = NULL;
        //gi.dprintf ("SetupMatch - participants %i, living %i\n", participants, living);
}
// TEMP


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

        // GRIM
	if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
                Say_Team(ent, gi.args());
                //Cmd_Say_f (ent, true, false);
		return;
	}
        // GRIM

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
// GRIM
        else if (Q_stricmp(cmd, "action_on") == 0)
                Cmd_Action_On(ent);
        else if (Q_stricmp(cmd, "action_off") == 0)
                Cmd_Action_Off(ent);
        else if (Q_stricmp (cmd, "invnexta") == 0)
                SelectNextItem (ent, IT_AMMO);
        else if (Q_stricmp (cmd, "invpreva") == 0)
                SelectPrevItem (ent, IT_AMMO);
        else if (Q_stricmp (cmd, "drop_armor") == 0)
                Drop_ArmorQuick (ent);
        else if (Q_stricmp (cmd, "position") == 0)
                gi.cprintf (ent, PRINT_HIGH, "Position: %s\n", vtos(ent->s.origin));
        else if (Q_stricmp (cmd, "hands") == 0)
                Cmd_Hands_f (ent);
        else if (Q_stricmp(cmd, "goodhand") == 0)
                Cmd_Weapon_GoodHand(ent);
        else if (Q_stricmp(cmd, "offhand") == 0)
                Cmd_Weapon_OffHand(ent);
        // Dirty - AQ compatibility 8)
        else if (Q_stricmp(cmd, "reload") == 0)
                Cmd_Weapon_Reload(ent);
        else if (Q_stricmp(cmd, "opendoor") == 0)
                ent->client->pers.grabbing = -1;
        else if (Q_stricmp(cmd, "bandage") == 0)
                Cmd_Hands_f (ent);
        else if (Q_stricmp(cmd, "weapon") == 0)
                ent->client->latched_buttons |= BUTTON_USE;
        // Dirty
        else if (Q_stricmp(cmd, "setpos") == 0)
        {
                if (ent->goal)
                {
                        gi.cprintf (ent, PRINT_HIGH, "goal off\n");
                        ent->client->ps.stats[STAT_GOAL1] = 0;
                        ent->goal = false;
                }
                else
                {
                        gi.cprintf (ent, PRINT_HIGH, "goal on\n");
                        VectorCopy(ent->s.origin, ent->goal_pos);
                        ent->goal = true;
                }
        }
        else if (Q_stricmp(cmd, "setpos2") == 0)
        {
                if (ent->audio)
                {
                        gi.cprintf (ent, PRINT_HIGH, "goal2 off\n");
                        ent->client->ps.stats[STAT_GOAL2] = 0;
                        ent->audio = false;
                }
                else
                {
                        gi.cprintf (ent, PRINT_HIGH, "goal2 on\n");
                        VectorCopy(ent->s.origin, ent->audio_pos);
                        ent->audio = true;
                }
        }
	else if (Q_stricmp (cmd, "team") == 0)
                Cmd_Team_f (ent);
        else if (Q_stricmp (cmd, "change") == 0)
                ChangeThings(ent, NULL);
        else if (Q_stricmp(cmd, "id") == 0)
                Cmd_IdState_f (ent);
        else if (!Q_stricmp (cmd, "radio")
         || !Q_stricmp (cmd, "audio")
          || !Q_stricmp (cmd, "radioteam"))
                Cmd_Audio_f (ent);
        else if (Q_stricmp (cmd, "request") == 0)
                Cmd_Request_f (ent);
        else if (Q_stricmp (cmd, "reply") == 0)
                Cmd_Reply_f (ent);
        else if (Q_stricmp (cmd, "drop_weapon") == 0)
                Cmd_DropWepQuick(ent);
        /*
        else if (Q_stricmp (cmd, "slot") == 0)
                Cmd_Slot (ent);
        else if (Q_stricmp (cmd, "startmatch") == 0)
        {
                if (!sv_cheats->value)
                        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                else
                {
                        level.match_time = level.time + 1;
                        level.match_state = MATCH_PRE_START;
                }
        }
        else if (Q_stricmp (cmd, "endmatch") == 0)
        {
                if (!sv_cheats->value)
                        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                else
                {
                        level.match_time = level.time + 1;
                        level.match_state = MATCH_FINISHED;
                }
        }
        else if (Q_stricmp (cmd, "reset") == 0)
        {
                if (!sv_cheats->value)
                        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                else
                        ReSetStuff (ent);
        }
        */
        else if (Q_stricmp (cmd, "megoal") == 0)
                Cmd_MeGoal_f (ent);
        else if (Q_stricmp (cmd, "newbie") == 0)
                HelpMenu (ent, NULL);
        /*
        else if (Q_stricmp (cmd, "zu") == 0)
        {
                ent->f_offset[2]++;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "zd") == 0)
        {
                ent->f_offset[2]--;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "yu") == 0)
        {
                ent->f_offset[1]++;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "yd") == 0)
        {
                ent->f_offset[1]--;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "xu") == 0)
        {
                ent->f_offset[0]++;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "xd") == 0)
        {
                ent->f_offset[0]--;
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        else if (Q_stricmp (cmd, "xyz") == 0)
        {
                VectorSet (ent->f_offset, 0, 8, ent->viewheight-8);
                gi.cprintf (ent, PRINT_HIGH, "f_offset: %s\n", vtos(ent->f_offset));
        }
        */
// GRIM
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
