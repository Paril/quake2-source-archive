#include "g_local.h"
#include "m_player.h"
#include "x_radio.h"
#include "p_dead.h"
#include "p_c4.h"

void Cmd_Reload_f (edict_t *ent);
void Grenade_Explode (edict_t *ent);
void    PlaceLaser (edict_t *ent);
void Cmd_LaserFence_f(edict_t *ent);

int ClientTeam (edict_t *ent)
{
	if (!CVAR_DEATHMATCH)
		return 0;

	if (team_dm->value)
	{
		if  (ent->client->resp.team < 5 && ent->client->resp.team > 0)
		    return ent->client->resp.team;
	}
	else if (ctf->value)
	{
		if (ent->client->resp.ctf_team < CTF_TEAM2 && ent->client->resp.team > CTF_NOTEAM)
			return ent->client->resp.ctf_team;
	}

    return 0;
}
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	if (ctf->value || team_dm->value)
	{
	    if (!ent1->client || !ent2->client)
		    return false;
    	if (ClientTeam(ent1) == ClientTeam(ent2))
    	    return true;
		return false;
	}
	else
    	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//PSY: CTF
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	}
//PSY: CTF

	if (cl->chase_target) {
		ChaseNext(ent);
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

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//PSY: CTF
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	}
//PSY: CTF

	if (cl->chase_target) {
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

	if (CVAR_DEATHMATCH && !CHEATS_FLAG)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_strcasecmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_strcasecmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "weapons") == 0)
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

	if (give_all || Q_strcasecmp(name, "ammo") == 0)
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

	if (give_all || Q_strcasecmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = item_jacketarmor;
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = item_combatarmor;
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = item_bodyarmor;
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "Power Shield") == 0)
	{
		it = item_powershield;
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
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
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

	if (CVAR_DEATHMATCH && !CHEATS_FLAG)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!ENT_SET_FOR_GODMODE )
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

	if (CVAR_DEATHMATCH && !CHEATS_FLAG)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!ENT_SET_FOR_NOTARGET )
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

	if (CVAR_DEATHMATCH && !CHEATS_FLAG)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if ENT_HAS_NOCLIP_ON
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
    else if (!Q_strcasecmp(s, ent->client->pers.weapon->pickup_name)) // we're trying to toggle, or we're a dumb@$$
    {
		if (ent->client->resp.class == LIGHT_ASSAULT) // burst fire toggles and shrapnel
		{
	        if (!Q_strcasecmp(s, "Machinegun"))
		    {
				if (ent->client->pers.fire_mode == 0) // if we're in here, and in auto mode
				{
					ent->client->pers.fire_mode = 1;
					gi.cprintf(ent, PRINT_HIGH, "Burst Fire.\n");
				}
				else // we're in burst
				{
					ent->client->pers.fire_mode = 0;
					gi.cprintf(ent, PRINT_HIGH, "Fully Auto.\n");
				}
			}
			else if (!Q_strcasecmp(s, "grenades"))
			{
				if (ent->client->shrapnel_toggle == 0) // in normal
				{
					ent->client->shrapnel_toggle = 1;
					gi.cprintf(ent, PRINT_HIGH, "Shrapnel Grenades Selected.\n");
				}
				else // in shrapnel
				{
					ent->client->shrapnel_toggle = 0;
					gi.cprintf(ent, PRINT_HIGH, "Hand Grenades Selected.\n");
				}
			}
        }
		else if (ent->client->resp.class == INFANTRY) // concussion grenade toggles
		{
			if (!Q_strcasecmp(s, "grenades"))
			{
				if (ent->client->concussion_toggle == 0) // we're in normal, switch it
				{
					ent->client->concussion_toggle = 1; // switch
					gi.cprintf(ent, PRINT_HIGH, "Concussion Grenades Selected.\n");
				}
				else // we're using concussion, or we've screwed up, so go to normal
				{
					ent->client->concussion_toggle = 0;
					gi.cprintf(ent, PRINT_HIGH, "Hand Grenades Selected.\n");
				}
			}
		}
		else if (ent->client->resp.class == HEAVY_DEFENSE) // pipe toggle
		{
			if (!Q_strcasecmp(s, "grenades"))
			{
				if (ent->client->pipe_toggle == 0)
				{
					ent->client->pipe_toggle = 1;
					gi.cprintf(ent, PRINT_HIGH, "Pipebombs Selected.\n");
				}
				else
				{
					ent->client->pipe_toggle = 0;
					gi.cprintf(ent, PRINT_HIGH, "Hand Grenades Selected.\n");
				}
			}
		}
		else if (ent->client->resp.class == SEAL) // they're a seal, and trying to re-use
		{
			if (!Q_strcasecmp(s, "machinegun")) // a machinegun, so make
			{
				s = "Dual Machineguns"; // them use the
				it = FindItem(s); // dual machineguns
			}
			// no need for an else, they will be using dual, and asking for single.
		}
		else if (ent->client->resp.class == RECON) // lotsa toggles here
		{
			if (!Q_strcasecmp(s, "grenades"))
			{
				if (ent->client->recon_grenade_toggle == 0) // we're on normal, tick to flash
				{
					ent->client->recon_grenade_toggle = 1; // set to flash
					gi.cprintf(ent, PRINT_HIGH, "Flash Grenades Selected.\n");
				}
				else if (ent->client->recon_grenade_toggle == 1) // we're on flash, tick to gas
				{
					ent->client->recon_grenade_toggle = 2; // set to gas
					gi.cprintf(ent, PRINT_HIGH, "Gas Grenades Selected.\n");
				}
				else // on gas, return to normal grenades
				{
					ent->client->recon_grenade_toggle = 0; // set to reg.
					gi.cprintf(ent, PRINT_HIGH, "Hand Grenades Selected.\n");
				}
			}
		}
		else if (ent->client->resp.class == DEMOLITIONS) // guided rocket toggle
		{
			if (!Q_strcasecmp(s, "rocket launcher"))
			{
				if (ent->client->rocket_toggle == 0) // go to guided
				{
					ent->client->rocket_toggle = 1;
					gi.cprintf(ent, PRINT_HIGH, "Guided Missiles Selected.\n");
				}
				else if (ent->client->rocket_toggle == 1) // on guided, go to homing
				{
					ent->client->rocket_toggle = 2;
					gi.cprintf(ent, PRINT_HIGH, "Homing Missiles Selected.\n");
				}
				else // back to normal
				{
					ent->client->rocket_toggle = 0;
					gi.cprintf(ent, PRINT_HIGH, "Rockets Selected.\n");
				}
			}
		}
		else if (ent->client->resp.class == MEDIC)
		{
			if (!Q_strcasecmp(s, "grenades"))
			{
				if (ent->client->teleport_toggle == 0) // go to telly grenades
				{
					ent->client->teleport_toggle = 1;
					gi.cprintf(ent, PRINT_HIGH, "Teleport Grenades Selected.\n");
				}
				else // go to normal
				{
					ent->client->teleport_toggle = 0;
					gi.cprintf(ent, PRINT_HIGH, "Hand Grenades Selected.\n");
				}
			}
		}
    }
	else if (!Q_strcasecmp(s, "Machinegun"))
	{
		ent->client->pers.fire_mode = 0; // put us in auto mode, gun is just coming up.
	}
	else if (!Q_strcasecmp(s, "grenades"))
	{ // reset grenade vars, its coming up for the first time.
		ent->client->concussion_toggle = 0;
		ent->client->pipe_toggle = 0;
		ent->client->shrapnel_toggle = 0;
		ent->client->teleport_toggle = 0;
	}
	else if (!Q_strcasecmp(s, "rocket launcher"))
	{
		ent->client->rocket_toggle = 0;
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
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

//PSY: CTF
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
		CTFOpenJoinMenu(ent);
		return;
	}

	if (ent->client->menu) {
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}
//PSY: CTF

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

//PSY: CTF
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
//PSY: CTF

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
//PSY: CTF
	if (ent->solid == SOLID_NOT)
		return;
//PSY: CTF

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

//PSY: CTF
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
//PSY: CTF
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
	for_each_client
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
	if ENT_CURRENTLY_IS_CROUCHED
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

	if (!ENABLE_TEAMPLAY)
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

	if (CVAR_DEDICATED)
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
	for (i = 0, e2 = g_edicts + 1; i < CVAR_MAXCLIENTS; i++, e2++) {
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

void Cmd_Infrared_f (edict_t *ent)
{
	if (ent->client->goggles) // we're on
	{
		ent->client->goggles = 0;
		ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	}
	else // we're off
	{
		ent->client->goggles = 1;
		ent->client->ps.rdflags |= RDF_IRGOGGLES;
	}
}

void Cmd_DetPipes_f (edict_t *ent)
{
	edict_t *blip = NULL;

    while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
    {
	    if (!strcmp(blip->classname, "detpipe") && blip->owner == ent)
        {
            blip->nextthink = level.time + .1;
        }
	}
}

void Cmd_Reload_f (edict_t *ent)
{
	int rds_left;

    if(ent->deadflag == DEAD_DEAD)
    {
	    gi.centerprintf(ent, "Cannot reload while dead.\n");
        return;
    }

    //First, grab the current magazine max count...
    if(strcmp(ent->client->pers.weapon->pickup_name, "Pistol") == 0)
	    rds_left = 12;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Flare Gun") == 0)
	    rds_left = 4;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Shotgun") == 0)
	    rds_left = 6;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Super Shotgun") == 0)
	    rds_left = 10;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Machinegun") == 0)
	    rds_left = 30;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Dual Machineguns") == 0)
	    rds_left = 60;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Chaingun") == 0)
	    rds_left = 75;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Grenade Launcher") == 0)
	    rds_left = 6;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Rocket Launcher") == 0)
	    rds_left = 6;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "HyperBlaster") == 0)
	    rds_left = 40;
    else if(strcmp(ent->client->pers.weapon->pickup_name, "Railgun") == 0)
	    rds_left = 3;
    else
    {
        gi.centerprintf(ent,"You can't reload that.\n");
        return;
    }

    if(ent->client->pers.inventory[ent->client->ammo_index])
    {
	    if((ent->client->weaponstate != WEAPON_END_MAG) && (ent->client->pers.inventory[ent->client->ammo_index] < rds_left))
        {
	        gi.centerprintf(ent,"You're on your last magazine!\n");
        }
        else
		{
			ent->client->reload_time = 10;
            ent->client->weaponstate = WEAPON_START_RELOADING;
		}
    }
    else
        gi.centerprintf(ent,"Cannot reload with no ammo.\n");
}

void Cmd_Chasecam_Viewlock(edict_t *ent);
void Cmd_Chasecam_Zoom(edict_t *ent, char *opt);
void Cmd_Chasecam_Toggle (edict_t *ent);
void Cmd_Airstrike_f(edict_t *ent);

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

	if (Q_strcasecmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_strcasecmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_strcasecmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_strcasecmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_strcasecmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_strcasecmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_strcasecmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_strcasecmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_strcasecmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_strcasecmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_strcasecmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_strcasecmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_strcasecmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_strcasecmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_strcasecmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_strcasecmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_strcasecmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_strcasecmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_strcasecmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
//PSY: CTF
	else if (Q_strcasecmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	} else if (Q_strcasecmp(cmd, "id") == 0) {
		CTFID_f (ent);
	}
//PSY: CTF
	else if (Q_strcasecmp (cmd, "chasecam") == 0)
		Cmd_Chasecam_Toggle (ent);
	else if (Q_strcasecmp (cmd, "camzoomout") == 0)
		Cmd_Chasecam_Zoom(ent, "out");
	else if (Q_strcasecmp (cmd, "camzoomin") == 0)
		Cmd_Chasecam_Zoom(ent, "in");
	else if (Q_strcasecmp (cmd, "camviewlock") == 0)
		Cmd_Chasecam_Viewlock(ent);
	else if (Q_strcasecmp (cmd, "camreset") == 0){ 
		if (ent->client->chasetoggle != 3 &&
			ent->client->chasetoggle != 0){ 
			ent->client->chasecam->chaseAngle = 0; 
		} 
	} 
    else if (Q_strcasecmp (cmd, "radio_power") == 0)   //Radio Power Switch
        X_Radio_Power_f(ent, gi.argv(1));
    else if (Q_strcasecmp (cmd, "radio") == 0)         //Radio to All Players
        X_Radio_f(ent, "ALL", gi.argv(1));
    else if (Q_strcasecmp (cmd, "tradio") == 0)                //Radio to Team
        X_Radio_f(ent, "TEAM", gi.argv(1));
	else if (Q_strcasecmp (cmd, "infrared") == 0)
		Cmd_Infrared_f (ent);
    else if (Q_strcasecmp (cmd, "flashlight") == 0)
        FL_make (ent);
    else if (Q_strcasecmp (cmd, "reload") == 0)
        Cmd_Reload_f (ent);
    else if (Q_strcasecmp (cmd, "detpipe") == 0)
        Cmd_DetPipes_f (ent);
	else if (Q_strcasecmp (cmd, "playdead") == 0)
		PlayDead (ent);
    else if (Q_strcasecmp (cmd, "laser") == 0)
        PlaceLaser (ent);
	else if (Q_strcasecmp(cmd, "fence") == 0)
        Cmd_LaserFence_f(ent);
	else if (Q_strcasecmp (cmd, "c4") == 0)
		PlaceC4(ent);
    else if (Q_strcasecmp (cmd, "sight") == 0)
        SP_LaserSight (ent);
    else if (Q_strcasecmp (cmd, "zoom") == 0)
    {
    	int zoomtype=atoi(gi.argv(1));

		if (ent->client->resp.class != SNIPER)
			return;

        if (zoomtype==0)
        {
        	ent->client->ps.fov = 90;
        }
        else if (zoomtype==1)
        {
            if (ent->client->ps.fov == 90) ent->client->ps.fov = 40;
            else if (ent->client->ps.fov == 40) ent->client->ps.fov = 20;
            else if (ent->client->ps.fov == 20) ent->client->ps.fov = 10;
            else ent->client->ps.fov = 90;
        }
    }
	else if (Q_strcasecmp(cmd, "airstrike1") == 0) {
		ent->airstrike_type=1;
		Cmd_Airstrike_f(ent); }
	else if (Q_strcasecmp(cmd, "airstrike2") == 0) {
		ent->airstrike_type=2;
		Cmd_Airstrike_f(ent); }
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
