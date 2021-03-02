/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"
#include <stdio.h>
#include <time.h>


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

	if (OnSameIWMTeam(ent1, ent2))
		return true;

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

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

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

	if (Q_stricmp(name, "MP Weapons") == 0)
	{
		it = FindItem ("Ionripper");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Phalanx");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Trap");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 50;
		it = FindItem ("ETF Rifle");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Plasma Beam");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Prox Launcher");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Disruptor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 1;
		it = FindItem ("Tesla");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 50;

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
			if (it->flags & IT_TECH)
				continue;
			if (it->classname && (it == flag2_item))
				continue;
			if (it->classname && (it == flag1_item))
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
			safe_cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		safe_cprintf (ent, PRINT_HIGH, "non-pickup item\n");
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

	if (ent->is_bot)
		return;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

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
		// PMM - prevent scrolling through ALL weapons
//		index = (selected_weapon + i)%MAX_ITEMS;
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		// PMM - prevent scrolling through ALL weapons
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it)
			return;
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
		// PMM - prevent scrolling through ALL weapons
//		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		// PMM - prevent scrolling through ALL weapons
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it)
			return;
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
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
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
}

/*
=================
Cmd_Gib_f
=================
*/
void Cmd_Gib_f (edict_t *ent, edict_t *attacker)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = -50;
	meansOfDeath = MOD_SUICIDE;
	if (attacker != ent)
	{
		meansOfDeath = MOD_BLASTER; // MOD FIXME
		player_die (ent, attacker, attacker, 100000, vec3_origin);
		return;
	}
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
	char	a[64];
	char	b[1280];
	int		c[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			c[count] = i;
			count++;
		}

	// sort by frags
	qsort (c, count, sizeof(c[0]), PlayerSort);

	// print information
	b[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (a, sizeof(a), "%3i %s\n",
			game.clients[c[i]].ps.stats[STAT_FRAGS],
			game.clients[c[i]].pers.netname);
		if (strlen (a) + strlen(b) > sizeof(b) - 100 )
		{	// can't print all of them in one packet
			strcat (b, "...\n");
			break;
		}
		strcat (b, a);
	}

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", b, count);
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

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_CHAT, "%s", text);

		Log_Print (text);
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
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	safe_cprintf(ent, PRINT_HIGH, "%s", text);
}

void SP_monster_rc_car (edict_t *self);

//======================================================
//================= GLADIATOR ==========================
//======================================================
//======================================================
void Spawn_Gladiator(edict_t *ent) {
	edict_t *gladiator;
	vec3_t torigin;
	int temp;
	vec3_t forward = {64, 0, 0};
	
	VectorCopy(ent->s.origin, torigin);
	VectorAdd (torigin, forward, torigin);

	
	G_Spawn_Explosion(TE_EXPLOSION2, torigin, torigin);

	// Create basic entity stuff.
	gladiator=G_Spawn();
	gladiator->classname = "XTransformer"; // Used for Killed()
	gladiator->transformer = ent; // Link back to Owner.
	//VectorCopy(torigin, gladiator->s.origin);
	VectorCopy(ent->s.origin, gladiator->s.origin);
	gi.linkentity(gladiator);
	VectorCopy (ent->s.angles, gladiator->s.angles);
	gladiator->s.angles[2] = 0;
	gladiator->s.angles[0] = 0;

	//------------------------------
	// Change some Global States
	//------------------------------
	
	skill->value=3; // Toggle Advanced AI.
	temp=deathmatch->value; // Temp storage..
	deathmatch->value=0; // Must = 0 to bypass quick-exit in Monster Code
	
	//------------------------------
	// Activate the gladiator Entity
	//------------------------------
	
	SP_monster_rc_car(gladiator);
	
	// Restore to previous value.
	deathmatch->value=temp;
	
	// Reset some stuff on this Holographic Image
	gladiator->monsterinfo.aiflags &= AI_BRUTAL;
	
	ent->is_transformed=true;
	ent->vehicle=gladiator; // Link to this Vehicle.

	ent->old_health = ent->health;
	ent->client->old_gunindex = ent->client->ps.gunindex;
	ent->client->ps.gunindex = 0;
	VectorCopy (ent->s.origin, ent->client->old_origin);

	ent->chasetarget = gladiator;

	ent->client->temp_oldweapon = ent->client->pers.weapon;
	ent->client->old_gunframe = ent->client->ps.gunframe;
	ent->client->pers.weapon = FindItem("Railgun");

	ent->vehicle->weapnum = 1;
	
	//VectorCopy (gladiator->s.angles, ent->client->ps.pmove.delta_angles);
	ent->client->ps.pmove.delta_angles[0] += gladiator->s.angles[0];

	VectorCopy (ent->s.origin, ent->client->rc_stay_here_origin);
	ent->s.modelindex = 0;
}

//======================================================
void Cmd_Gladiator_f(edict_t *ent) {
	
	// Don't allow dead/respawning players to have this!
	if (!G_ClientInGame(ent)) return;
	
	// Already transformed?
	if (ent->is_transformed) return;
	
	// Try to see if we can Spawn the gladiator
	//if (ent->client->resp.score < 2){
	//	safe_centerprintf(ent, "Gladiator costs 2 Frags!\n");
	//	return; }
	//else {
	//	ent->client->resp.score -= 2;
		Spawn_Gladiator(ent);// }

		//ent->classname = "remotecar";
}

/*
8===============>
FL_make
make the dang thing
<===============8
*/

void FL_think (edict_t *self)
{
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;
	trace_t tr;
	
	if (self->owner->health < 1)
	{
		G_FreeEdict (self);
		self->nextthink = 0;
		return;
	}

	AngleVectors (self->owner->client->v_angle, forward, right, up);
	
	VectorSet(offset,24 , 6, self->owner->viewheight-7);
	G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
	VectorMA(start,8192,forward,end);
	
	tr = gi.trace (start,NULL,NULL, end,self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	
	if (tr.fraction != 1)
	{
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);
	}

	
	vectoangles(tr.plane.normal,self->s.angles);
	VectorCopy(tr.endpos,self->s.origin);
	
	gi.linkentity (self);
	self->nextthink = level.time + 0.1;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLASHLIGHT);
	gi.WritePosition (self->s.origin);
	gi.WriteShort (self - g_edicts);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

/*
8===============>
FL_make
make the dang thing
<===============8
*/

void FL_make(edict_t *self) 
{
	vec3_t  start,forward,right,end;

	if (CheckBan(feature_ban, FEATUREBAN_FLASHLIGHT))
	{
		safe_cprintf (self, PRINT_HIGH, "Sorry, Flashlight is banned.\n");
		return;
	}

	if (self->movetype == MOVETYPE_NOCLIP)
		return;
	
	if ( self->flashlight )
	{
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
		self->has_flashlight = 0;
		return;
	}
	
	AngleVectors (self->client->v_angle, forward, right, NULL);
	
	VectorSet(end,100 , 0, 0);
	G_ProjectSource (self->s.origin, end, forward, right, start);
	
	self->flashlight = G_Spawn ();
	self->flashlight->owner = self;
	self->flashlight->movetype = MOVETYPE_NOCLIP;
	self->flashlight->solid = SOLID_NOT;
	self->flashlight->classname = "flashlight";
	//self->flashlight->s.effects |= EF_HYPERBLASTER;
	self->flashlight->s.modelindex = ModelIndex ("sprites/null.sp2");  // HEY KIDDYS NOTE THIS
	self->flashlight->s.skinnum = 0;
	// dull. Try stuff and tell me if you find anything cool
	self->flashlight->think = FL_think;
	self->flashlight->nextthink = level.time + 0.1;
	self->has_flashlight = 1;
}


/*
=================
ClientCommand
=================
*/
void Cmd_Hook_f (edict_t *ent);
void CreateClones (edict_t *ent);
void TestMessage (edict_t *ent);
#define PI		3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

int InvertNumberTest (int in)
{
	int h;

	h = 90 - in;

	return h;
}

void SubtractToOwner (edict_t *ent)
{
	vec3_t start, end;
	vec3_t sub;
	float dist;
	float disti;
	float speed = 800;
	float grav = sv_gravity->value;
	float angle;
	float deg2rad;
	int u;

	if (ent->owner->client->resp.artillery_dest)
		return;

	VectorSubtract (ent->owner->s.origin, ent->s.origin, start);
	vectoangles (start, end);

	ent->s.angles[1] = end[1];

	VectorSubtract (ent->owner->s.origin, ent->s.origin, sub);
	dist = VectorLength(sub);

	//gi.dprintf ("%f\n", dist);

	/*if (dist > 5000)
		ent->owner->client->resp.artillery->s.frame = 0;
	else if (dist > 4000)
		ent->owner->client->resp.artillery->s.frame = 1;	
	else if (dist > 3000)
		ent->owner->client->resp.artillery->s.frame = 2;
	else if (dist > 2000)
		ent->owner->client->resp.artillery->s.frame = 3;
	else if (dist > 1000)
		ent->owner->client->resp.artillery->s.frame = 4;
	else if (dist > 50)
		ent->owner->client->resp.artillery->s.frame = 5;	
	else
		ent->owner->client->resp.artillery->s.frame = 6;*/


	disti = dist;

	deg2rad = (((grav*disti) / pow(speed, 2))/180)* PI;

	angle = asin(deg2rad) * 1000;
	angle = angle / 2;
	angle = 90 - angle;
//	safe_cprintf(ent->owner, PRINT_HIGH, "Angle : %f\n", angle);

	if (ent->owner->client->resp.artillery->s.frame > InvertNumberTest(68))
		ent->owner->client->resp.artillery->s.frame = InvertNumberTest(68);

	u = InvertNumberTest((int)angle);

	ent->owner->client->resp.artillery->s.frame = u;

	//safe_cprintf(ent->owner, PRINT_HIGH, "Angle : %i\n", u);

	ent->nextthink = level.time + .1;
}

void Artillery_PlaceBase (edict_t *ent)
{
	edict_t *tmp = ent->client->resp.artillery_base;

	tmp = G_Spawn();
	VectorCopy (ent->s.origin, tmp->s.origin);
	tmp->movetype = MOVETYPE_TOSS;
	tmp->classname = "artillery_base";
	tmp->owner = ent;
	tmp->s.modelindex = ModelIndex("models/other/artillery/tris_base.md2");

	gi.linkentity (tmp);

	// Paril: Do I need to do this?
	ent->client->resp.artillery_base = tmp;
}

void Artillery_PlaceGun (edict_t *ent)
{
	edict_t *tmp = ent->client->resp.artillery;
//	vec3_t mins = {12, 12, 12};
//	vec3_t maxs = {-12, -12, -12};

	tmp = G_Spawn();
	VectorCopy (ent->s.origin, tmp->s.origin);
	tmp->movetype = MOVETYPE_TOSS;
//	tmp->solid = SOLID_BBOX;
	//VectorCopy (mins, tmp->mins);
	//VectorCopy (maxs, tmp->maxs);
	tmp->classname = "artillery_gun";
	tmp->owner = ent;
	tmp->s.modelindex = ModelIndex("models/other/artillery/tris.md2");
	tmp->think = SubtractToOwner;
	tmp->nextthink = level.time + .1;

	gi.linkentity (tmp);

	ent->client->resp.artillery_placed = 1;

	ent->client->resp.artillery = tmp;

	Artillery_PlaceBase (ent);
}

void SpawnTheExplosion (vec3_t or)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (or);
	gi.multicast (or, MULTICAST_PVS);
}

void BlowUpThisJoint (edict_t *ent)
{
	vec3_t or;
	int j;
	float i, g, f, h;
//	float sinh;
	vec3_t d;
	float dist;

	g = rand()%25;
	i = rand()%25;
	h = rand()%25;
	f = rand()%25;

	VectorCopy (ent->s.origin, or);

	T_RadiusDamage (ent, ent->owner, 700, NULL, 700, MOD_ARTILLERY); // MOD FIXME

	//G_FreeEdict (tmp);

	VectorCopy (ent->s.origin, or);

	//BecomeExplosion1 (ent);
	SpawnTheExplosion(ent->s.origin);

	for (j = 0; j < 3+rand()%3; j++)
	{
		or[0] -= (g - f + i);
		or[1] -= (g - f);
		or[2] += 10;
		SpawnTheExplosion(or);
		//G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
	}

	VectorSubtract (ent->owner->client->resp.artillery->s.origin, ent->owner->client->resp.artillery_dest->s.origin, d);
	dist = VectorLength (d);
	
	if (ent->tempint < 3)
	{
		ent->nextthink = level.time + 2;
		SpawnTheExplosion(ent->s.origin);
	}
	else
	{
		G_FreeEdict (ent->owner->client->resp.artillery);
		G_FreeEdict (ent->owner->client->resp.artillery_base);
		ent->owner->client->resp.artillery_placed = 0;
		ent->owner->client->resp.artillery = NULL;
		ent->owner->client->resp.artillery_dest = NULL;
		ent->owner->client->resp.artillery_base = NULL;
		BecomeExplosion1(ent);
	}
}

void WooshSound (edict_t *ent)
{
	gi.positioned_sound (ent->s.origin, ent, CHAN_AUTO, SoundIndex ("weapons/art_woosh.wav"), 1, ATTN_NORM, 0);

	ent->tempint ++;

	if (ent->tempint < 3)
		ent->nextthink2 = level.time + 2;
}

void SoundShootOne (edict_t *ent)
{
	gi.positioned_sound (ent->tempent->s.origin, ent->tempent, CHAN_AUTO, SoundIndex ("weapons/art.wav"), 1, ATTN_NORM, 0);
	gi.positioned_sound (ent->tempent->s.origin, ent->tempent, CHAN_AUTO, SoundIndex ("weapons/art.wav"), 1, ATTN_NORM, 0);
	gi.positioned_sound (ent->tempent->s.origin, ent->tempent, CHAN_AUTO, SoundIndex ("weapons/art.wav"), 1, ATTN_NORM, 0);

	ent->tempint2++;

	if (ent->tempint2 < 3)
		ent->nextthink3 = level.time + 2;
}


void Artillery_PlaceDest (edict_t *ent)
{
	edict_t *tmp = ent->client->resp.artillery_dest;
	vec3_t  star;
	vec3_t  st;
	vec3_t  end, start;
	trace_t tr;
	vec3_t up = {0, 0, -90};
	vec3_t vec;
	vec3_t sub;
	float dist;
	float disti;
	float speed = 800;
	float grav = sv_gravity->value;
	float angle;
	float deg2rad;
	int u;

	if (!ent->client->resp.artillery)
	{
//		gi.dprintf ("It happened.\n");
		Artillery_PlaceGun(ent);
		return;
	}

	VectorCopy(ent->s.origin, star);

	VectorMA(star, 8192, up, end);
	tr = gi.trace(star, NULL, NULL, end, ent, MASK_SOLID);

	if ( gi.pointcontents(tr.endpos) == (CONTENTS_SOLID || CONTENTS_WINDOW || CONTENTS_TRANSLUCENT || CONTENTS_LADDER))
	{
		gi.dprintf ("Somehow, the destination went inside a wall. Tell Paril to fix it.\n");
		return;
	}

	VectorCopy (tr.endpos, st);

	tmp = G_Spawn();
	VectorCopy (st, tmp->s.origin);
	tmp->movetype = MOVETYPE_NONE;
	tmp->solid = SOLID_NOT;
	tmp->classname = "artillery_dest";
	tmp->owner = ent;
	tmp->tempent = ent->client->resp.artillery;
	ent->client->resp.artillery->tempent = tmp;
	ent->client->resp.artillery_dest = tmp;
	tmp->think = BlowUpThisJoint;
	tmp->nextthink = level.time + 5.0;
	tmp->think2 = WooshSound;
	tmp->nextthink2 = level.time + 4.07;
	tmp->think3 = SoundShootOne;
	tmp->nextthink3 = level.time + .1;
	tmp->s.modelindex = ModelIndex ("models/proj/ecks/tris.md2");

	gi.linkentity (tmp);

	VectorCopy (ent->client->resp.artillery->s.origin, start);
	VectorCopy (tmp->s.origin, vec);

	VectorSubtract (vec, start, vec);
	vectoangles (vec, vec);

	//VectorCopy (vec, ent->client->resp.artillery->s.angles);
	ent->client->resp.artillery->s.angles[1] = vec[1];

	VectorSubtract (tmp->s.origin, ent->client->resp.artillery->s.origin, sub);
	dist = VectorLength(sub);

	//gi.dprintf ("%f\n", dist);

	/*if (dist > 5000)
		ent->client->resp.artillery->s.frame = 0;
	else if (dist > 4000)
		ent->client->resp.artillery->s.frame = 1;	
	else if (dist > 3000)
		ent->client->resp.artillery->s.frame = 2;
	else if (dist > 2000)
		ent->client->resp.artillery->s.frame = 3;
	else if (dist > 1000)
		ent->client->resp.artillery->s.frame = 4;
	else if (dist > 50)
		ent->client->resp.artillery->s.frame = 5;	
	else
		ent->client->resp.artillery->s.frame = 6;*/

	disti = dist;

	deg2rad = (((grav*disti) / pow(speed, 2))/180)* PI;

	angle = asin(deg2rad) * 1000;
	angle = angle / 2;
	angle = 90 - angle;

	if (ent->client->resp.artillery->s.frame > InvertNumberTest(68))
		ent->client->resp.artillery->s.frame = InvertNumberTest(68);

	u = InvertNumberTest((int)angle);

	ent->client->resp.artillery->s.frame = u;

//	safe_cprintf(ent, PRINT_HIGH, "Angle : %i\n", u);
}

void Cmd_Artillery (edict_t *ent)
{
	if (ent->deadflag || ent->health < 1)
		return;

	if (CheckBan(feature_ban, FEATUREBAN_ARTILLERY))
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, Artillery is banned.\n");
		return;
	}

	if (!ent->client->resp.artillery_placed)
	{
		if (ent->client->resp.artillery)
		{
			safe_cprintf (ent, PRINT_HIGH, "You already have your artillery placed!\n");
			return;
		}
		Artillery_PlaceGun(ent);
		safe_cprintf (ent, PRINT_HIGH, "Artillery gun placed, go place a destination with the same command.\n");
	}
	else
	{
		if (ent->client->resp.artillery_dest)
		{
			safe_cprintf (ent, PRINT_HIGH, "You already have your artillery placed!\n");
			return;
		}
		Artillery_PlaceDest(ent);
		safe_cprintf (ent, PRINT_HIGH, "Destination placed; run!\n");
	}
}

/*
==================
Cmd_AGMPull_f

Shortens the AG Manipulator beam.
==================
*/
void Cmd_AGMPull_f(edict_t *ent)
{
	if (ent->client->agm_target == NULL)
	{
		safe_centerprintf(ent, "No AG Manipulator target to pull\n");
		return;
	}

	ent->client->agm_pull = true;
	ent->client->agm_push = false;
}

/*
==================
Cmd_AGMUnpull_f

Stops shortening the AG Manipulator beam.
==================
*/
void Cmd_AGMUnpull_f(edict_t *ent)
{
	ent->client->agm_pull = false;
}

/*
==================
Cmd_AGMPush_f

Lengthens the AG Manipulator beam.
==================
*/
void Cmd_AGMPush_f(edict_t *ent)
{
	if (ent->client->agm_target == NULL)
	{
		safe_centerprintf(ent, "No AG Manipulator target to push\n");
		return;
	}

	ent->client->agm_pull = false;
	ent->client->agm_push = true;
}

/*
==================
Cmd_AGMUnpush_f

Stops lengthening the AG Manipulator beam.
==================
*/
void Cmd_AGMUnpush_f(edict_t *ent)
{
	ent->client->agm_push = false;
}

/*
==================
Cmd_AGMFling_f

Flings the player held by the AG Manipulator beam.
==================
*/
void Cmd_AGMFling_f(edict_t *ent)
{
	static gitem_t *tech = NULL;
	edict_t	*targ;
	vec3_t	forward;

//	Sanity check.

	if (ent->client->agm_target == NULL)
	{
		safe_centerprintf(ent, "No AG Manipulator target to fling\n");
		return;
	}

	targ = ent->client->agm_target;

//	Scale the target's velocity vector along our viewing angle. The old velocity needs to be
//	set to the same value, otherwise flinging the victim close to a wall won't necessarily
//	register a sufficient delta-v.

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 2000, targ->velocity);
	VectorCopy(targ->velocity, targ->client->oldvelocity);
	gi.linkentity(targ);

	targ->flung_by_agm = true;
	targ->held_by_agm = false;
	targ->thrown_by_agm = false;

//	Reset our AGM stuff.

	ent->client->agm_target = NULL;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->agm_charge = 0;
	ent->client->agm_tripped = true;

//	Play Quad or Tech sounds, if appropriate.

	if (!tech)
		tech = FindItemByClassname("item_tech2");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (ent->client->ctf_techsndtime < level.time)
		{
			ent->client->ctf_techsndtime = level.time + 1.0;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, SoundIndex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, SoundIndex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
		}
	}
	else if (ent->client->quad_framenum > level.framenum)
		gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage3.wav"), 1, ATTN_NORM, 0);
}

void MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (edict_t *owner, vec3_t origin, int Backwards);
void HelpMenu(edict_t *ent, pmenuhnd_t *p);
void CheckModelIndexes (edict_t *ent);
void FixLaserChains (edict_t *ent);
void drawbbox (edict_t *self)
{
	int	lines[4][3] = {
		{1, 2, 4},
		{1, 2, 7},
		{1, 4, 5},
		{2, 4, 7}
	};

	int starts[4] = {0, 3, 5, 6};

	vec3_t pt[8];
	int i, j, k;
	vec3_t coords[2];
	vec3_t newbox;
	vec3_t f,r,u, dir;

	if (!self)
		return;

	VectorCopy (self->absmin, coords[0]);
	VectorCopy (self->absmax, coords[1]);

	for (i=0; i<=1; i++)
	{
		for (j=0; j<=1; j++)
		{
			for (k=0; k<=1; k++)
			{
				pt[4*i+2*j+k][0] = coords[i][0];
				pt[4*i+2*j+k][1] = coords[j][1];
				pt[4*i+2*j+k][2] = coords[k][2];
			}
		}
	}
	
	for (i=0; i<= 3; i++)
	{
		for (j=0; j<= 2; j++)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_DEBUGTRAIL);
			gi.WritePosition (pt[starts[i]]);
			gi.WritePosition (pt[lines[i][j]]);
			gi.multicast (pt[starts[i]], MULTICAST_ALL);	
		}
	}

	vectoangles2 (self->s.angles, dir);
	AngleVectors (dir, f, r, u);

	VectorMA (self->s.origin, 50, f, newbox);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (self->s.origin);
	gi.WritePosition (newbox);
	gi.multicast (self->s.origin, MULTICAST_PVS);	
	VectorClear (newbox);

	VectorMA (self->s.origin, 50, r, newbox);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (self->s.origin);
	gi.WritePosition (newbox);
	gi.multicast (self->s.origin, MULTICAST_PVS);	
	VectorClear (newbox);

	VectorMA (self->s.origin, 50, u, newbox);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (self->s.origin);
	gi.WritePosition (newbox);
	gi.multicast (self->s.origin, MULTICAST_PVS);	
	VectorClear (newbox);
}
//extern int numitems;
//extern edict_t *items[100];
//void EditorCommands (edict_t *ent, char *cmd);
ff12 EditorCommands;

void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet
	cmd = gi.argv(0);

	// Paril: Couldn't talk or anything!
	// Made SOME commands usable.
	if( ent->frozen ) //acrid 3/19 no cmds if frozen
	{
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

	return;	
	}

	//JABot[start]
	//if(BOT_Commands(ent))
	//	return;
	//[end]


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
	else if (Q_stricmp(cmd, "rc_create") == 0 )
		Cmd_Gladiator_f(ent);
	else if (Q_stricmp(cmd, "hook") == 0)
		Cmd_Hook_f(ent);
	else if (Q_stricmp (cmd, "flashlight") == 0)
		FL_make (ent);
	else if (Q_stricmp (cmd, "clone") == 0)
	{
		if (ent->client->clones == 0)
		{
			if (ent->health < 1)
				return;
			safe_cprintf (ent, PRINT_HIGH, "Clone shield activated!\n");
			CreateClones(ent);
			ent->client->clones = 1;
			return;
		}
		else
		{
			//if (ent->client->clone1->inuse)
			G_FreeEdict (ent->client->clone1);
			//if (ent->client->clone2->inuse)
			G_FreeEdict (ent->client->clone2);
			//if (ent->client->clone3->inuse)
			G_FreeEdict (ent->client->clone3);
			//if (ent->client->clone4->inuse)
			G_FreeEdict (ent->client->clone4);

			ent->client->clone1 = ent->client->clone2 = ent->client->clone3 = ent->client->clone4 = NULL;

			ent->client->clones = 0;

			safe_cprintf (ent, PRINT_HIGH, "Clone shield destroyed!\n");
		}
	}
	else if (Q_stricmp(cmd, "rc_turbo") == 0 || Q_stricmp(cmd, "rc_jump") == 0)
	{
		vec3_t forward;
		vec3_t kvel;

		if (!ent->vehicle)
			return;

		AngleVectors (ent->vehicle->s.angles, forward, NULL, NULL);

		//VectorInverse(forward);
		
		VectorScale (forward, 400.0 * 2, kvel);
		
		VectorAdd (ent->vehicle->velocity, kvel, ent->vehicle->velocity);
	}
	else if (Q_stricmp(cmd, "rc_forward") == 0)
		ent->client->rc_forward = 1;
	else if (Q_stricmp(cmd, "rc_stop_forward") == 0)
		ent->client->rc_forward = 0;
	else if (Q_stricmp(cmd, "rc_stop_side") == 0)
		ent->client->rc_left = 0;
	else if (Q_stricmp(cmd, "rc_backward") == 0)
		ent->client->rc_forward = -1;
	else if (Q_stricmp(cmd, "rc_left") == 0)
		ent->client->rc_left = 1;
	else if (Q_stricmp(cmd, "rc_right") == 0)
		ent->client->rc_left = -1;
	else if (Q_stricmp(cmd, "rc_horn") == 0)
	{
		if (!ent->vehicle)
			return;

		gi.sound (ent, 8, SoundIndex("weapons/car_horn.wav"), 1, ATTN_NORM, 0);
	}
	else if (Q_stricmp(cmd, "rc_boom") == 0)
	{
		if (!ent->vehicle)
			return;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (ent->vehicle->s.origin);
		gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);
		T_RadiusDamage (ent->vehicle, ent, 400, ent->vehicle, 400, MOD_BLASTER); // MOD
		ent->is_transformed=false;
		ent->client->ps.gunindex = ent->client->old_gunindex;
		ent->chasetarget = NULL;
		ent->movetype = MOVETYPE_WALK;
		ent->s.modelindex = 255;

		//BecomeExplosion1(ent->vehicle);
//		G_FreeEdict (ent->vehicle);
		ent->vehicle=NULL;
		ent->client->pers.weapon = ent->client->temp_oldweapon;
		ent->client->ps.gunframe = ent->client->old_gunframe;
	}
	else if (Q_stricmp(cmd, "rc_kamikaze") == 0)
	{
		if (!ent->vehicle)
			return;
	
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < 50)
		{
			safe_cprintf (ent, PRINT_HIGH, "You need 50 rockets to do this!\n");
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (ent->vehicle->s.origin);
			gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);
			T_RadiusDamage (ent->vehicle, ent, 400, ent->vehicle, 400, MOD_BLASTER); // MOD
			ent->is_transformed=false;
			ent->client->ps.gunindex = ent->client->old_gunindex;
			ent->chasetarget = NULL;
			ent->movetype = MOVETYPE_WALK;
			ent->s.modelindex = 255;

//			BecomeExplosion1(ent->vehicle);
			//G_FreeEdict (ent->vehicle);
			ent->vehicle=NULL;
			ent->client->pers.weapon = ent->client->temp_oldweapon;
			ent->client->ps.gunframe = ent->client->old_gunframe;
			return;
		}

		ent->s.modelindex = 255;

		MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (ent, ent->vehicle->s.origin, 0);
		gi.sound (ent->vehicle, CHAN_AUTO, SoundIndex ("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);
		ent->is_transformed=false;
		ent->client->ps.gunindex = ent->client->old_gunindex;
		ent->chasetarget = NULL;
		ent->movetype = MOVETYPE_WALK;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= 50;

		BecomeExplosion1(ent->vehicle);
		//G_FreeEdict (ent->vehicle);
		ent->vehicle=NULL;
		ent->client->pers.weapon = ent->client->temp_oldweapon;
		ent->client->ps.gunframe = ent->client->old_gunframe;
	}
	else if (Q_stricmp(cmd, "rc_weapon") == 0)
	{
		if (!ent->vehicle)
			return;

		if (!ent->vehicle->weapnum)
			ent->vehicle->weapnum = 1;

		ent->vehicle->weapnum++;

		if (ent->vehicle->weapnum == 2)
			safe_cprintf (ent, PRINT_HIGH, "Rocket Launcher\n");

		if (ent->vehicle->weapnum == 3)
		{
			safe_cprintf (ent, PRINT_HIGH, "Railgun\n");
			ent->vehicle->weapnum = 1;
		}
	}
	else if (Q_stricmp(cmd, "artillery") == 0)
		Cmd_Artillery(ent);
	else if (Q_stricmp(cmd, "iwmhelp") == 0)
		HelpMenu(ent, NULL);
	else if (Q_stricmp(cmd, "modelindexes") == 0)
		CheckModelIndexes(ent);
	else if (Q_stricmp(cmd, "soundindexes") == 0)
		CheckSoundIndexes(ent);
	else if (Q_stricmp(cmd, "imageindexes") == 0)
		CheckImageIndexes(ent);
	else if (Q_stricmp(cmd, "telepuddle") == 0)
		Cmd_TelePuddle(ent);
	else if (Q_stricmp(cmd, "gib") == 0)
		Cmd_Gib_f(ent, ent);
	else if (Q_stricmp(cmd, "loc") == 0)
		safe_cprintf (ent, PRINT_HIGH, "%s\n", vtos(ent->s.origin));
//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	} else if (Q_stricmp(cmd, "id") == 0) {
		CTFID_f (ent);
	} else if (Q_stricmp(cmd, "yes") == 0) {
		CTFVoteYes(ent);
	} else if (Q_stricmp(cmd, "no") == 0) {
		CTFVoteNo(ent);
	} else if (Q_stricmp(cmd, "ready") == 0) {
		CTFReady(ent);
	} else if (Q_stricmp(cmd, "notready") == 0) {
		CTFNotReady(ent);
	} else if (Q_stricmp(cmd, "ghost") == 0) {
		CTFGhost(ent);
	} else if (Q_stricmp(cmd, "admin") == 0) {
		CTFAdmin(ent);
	} else if (Q_stricmp(cmd, "stats") == 0) {
		CTFStats(ent);
	} else if (Q_stricmp(cmd, "warp") == 0) {
		CTFWarp(ent);
	} else if (Q_stricmp(cmd, "boot") == 0) {
		CTFBoot(ent);
	} else if (Q_stricmp(cmd, "playerlist") == 0) {
		CTFPlayerList(ent);
	} else if (Q_stricmp(cmd, "observer") == 0) {
		CTFObserver(ent);
	}
//ZOID
	/*else if (Q_stricmp (cmd, "goto") == 0)
	{
		float vec0 = atof(gi.argv(1));
		float vec1 = atof(gi.argv(2));
		float vec2 = atof(gi.argv(3));
		vec3_t start;

		VectorSet (start, vec0, vec1, vec2);
		VectorCopy (start, ent->s.origin);
		gi.linkentity (ent);
	}*/
	/*else if (Q_stricmp(cmd, "line") == 0)
	{
		float dist = atof(gi.argv(1));
		vec3_t  start;

		vec3_t  forward;
		vec3_t  end;
		trace_t tr;
		
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorMA(start, dist, forward, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.multicast (start, MULTICAST_PVS);
	}*/
	//else if (Q_stricmp(cmd, "nn") == 0)
	//	Cmd_SeeWeaponsThingy(ent);
	/*else if (Q_stricmp(cmd, "gibb") == 0)
	{
		int i;
		int num = atoi(gi.argv(1));

		if (!num)
			ThrowGib2 (ent, "models/objects/gibs/chest/tris.md2");
		else
		{
			for (i = 0; i < num; i++)
			{
				ThrowGib2 (ent, "models/objects/gibs/chest/tris.md2");
			}
		}
	}*/
	/*else if (!Q_stricmp(cmd, "pull"))
		Cmd_AGMPull_f(ent);
	else if (!Q_stricmp(cmd, "unpull"))
		Cmd_AGMUnpull_f(ent);
	else if (!Q_stricmp(cmd, "push"))
		Cmd_AGMPush_f(ent);
	else if (!Q_stricmp(cmd, "unpush"))
		Cmd_AGMUnpush_f(ent);
	else if (!Q_stricmp(cmd, "fling"))
		Cmd_AGMFling_f(ent);*/
	//else if (Q_stricmp(cmd, "removeinfests") == 0)
	//	RemoveInfests(ent);
	//else if (Q_stricmp(cmd, "fixlinks") == 0)
	//	FixLaserChains(ent);
	else if (Q_stricmp(cmd, "fixvehicle") == 0)
		ent->laz_vehicle->s.origin[1] += 5;
	else if (Q_stricmp(cmd, "bboxvehicle") == 0)
		drawbbox (ent->laz_vehicle);
	// Handle editor commands all in iwm_editor.c
	// IN ANOTHER DLL! RARAAGGF.
	else if (Q_strncasecmp (cmd, "ed_", 3) == 0)
	{
		if (EditorCommands != NULL)
			EditorCommands (ent, cmd, level.mapname, level.time, level.framenum, entities_glb);
		else
			safe_cprintf (ent, PRINT_HIGH, "Editor DLL is not loaded!\n");
	}
	else if (Q_stricmp(cmd, "magnets_repel") == 0)
		RepelMagnets (ent);
	else if (Q_stricmp(cmd, "magnets_attract") == 0)
		AttractMagnets (ent);
	else if (Q_stricmp(cmd, "telepuddle_retrieve") == 0)
	{
		if (ent->client->resp.puddle_dest)
		{
			G_FreeEdict(ent->client->resp.puddle_dest);
			ent->client->resp.puddle_dest = NULL;
		}
		if (ent->client->resp.puddle_tele)
		{
			G_FreeEdict(ent->client->resp.puddle_tele);
			ent->client->resp.puddle_tele = NULL;
		}
		ent->client->resp.layed_t = 0;
	}
	/*else if (Q_stricmp(cmd, "lol") == 0)
	{
		int lol = atoi(gi.argv(1));

		for (lol = 0; lol < 143; lol++)
			gi.configstring (CS_MODELS+lol, "models/weapons/v_shotg/tris.md2");
	}*/
	else if (Q_stricmp(cmd, "checkmodulate") == 0)
	{
		int modulate_level = atoi(gi.argv(1));
		if (!modulate_level)
			return;

		//if (modulate_level > 3)
			gi.dprintf ("Player %s has a gl_modulate of %i.\n", ent->client->pers.netname, modulate_level);
	}
	/*else if (Q_stricmp(cmd, "forcewall") == 0)
	{
		edict_t *point1;
		edict_t *point2;
		vec3_t start, forward, end, up, right;
		trace_t tr;

		VectorCopy (ent->s.origin, start);
		AngleVectors (ent->client->v_angle, forward, NULL, NULL);
		VectorMA (start, 8192, forward, end);

		tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

		point1 = G_Spawn();
		point2 = G_Spawn();

		VectorCopy (tr.endpos, point1->s.origin);
		AngleVectors (point1->s.angles, NULL, right, up);
	}*/
    // command is gi.argv(0)
	else if (Q_stricmp(cmd, "vote") == 0) 
	{ 
		if (Q_stricmp(gi.argv(1), "map") == 0)   // gi.argv(1) determines election type
            Voting_BeginElection(ent, ELECT_MAP);   // set the vote type flag and who started it.
		else if (Q_stricmp(gi.argv(1), "yes") == 0 || Q_stricmp(gi.argv(1), "no") == 0)   // 'vote yes' command
            Voting_CmdVote_f(ent, Q_stricmp(gi.argv(1), "yes") == 0 ? YES : NO);
		else
			safe_cprintf (ent, PRINT_HIGH, "Unknown voting command: \"%s\"\n", gi.argv(1));
	} 
	else if (Q_stricmp(cmd, "time") == 0)
	{
		char s[30];
		size_t i;
		struct tm tim;
		time_t now;
		char command[500];

		now = time(NULL);

		tim = *(localtime(&now));
		i = strftime(s,40,"%A",&tim);

		sprintf (command, "exec cfglists/%c%s.cfg\n", tolower(s[0]), s+1);
		gi.dprintf ("%s", command);
		gi.AddCommandString (command);
	}
	else if (Q_stricmp(cmd, "lag") == 0)
	{
		ent->lagged = true;
		ent->latency_time = level.time + 8;
		ent->tempint = 5;
	}
	else	// anything that doesn't match a command will be a chat
		OpenTeamMenu(ent, NULL);
		//safe_cprintf (ent, PRINT_HIGH, "Unknown command: \"%s\"\n", cmd);
		//Cmd_Say_f (ent, false, true);
}
