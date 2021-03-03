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
#include "g_tdm_votemenu.h"


/*char *ClientTeam (edict_t *ent)
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
}*/

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	if (ent1->client && ent2->client)
	{
		return (ent1->client->pers.team == ent2->client->pers.team);
	}
	else
	{
		if (ent1->client || ent2->client)
			return false;
		else
			return true;
	}

	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t		*cl;
	int				i, index;
	const gitem_t	*it;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	cl = ent->client;

	if (cl->pers.menu.active)
	{
		PMenu_Next(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->selected_item + i)%MAX_ITEMS;
		if (!cl->inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->selected_item = index;
		return;
	}

	cl->selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t		*cl;
	int				i, index;
	const gitem_t	*it;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	cl = ent->client;

	if (cl->pers.menu.active)
	{
		PMenu_Prev(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->selected_item = index;
		return;
	}

	cl->selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->selected_item == -1)
		return;		// still invalid!

	if (cl->inventory[cl->selected_item])
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
	char			*name;
	const gitem_t	*it;
	int				index;
	int				i;
	qboolean		give_all;
	edict_t			*it_ent;

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (!ent->client->pers.team)
		return;

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
			ent->client->inventory[i] += 1;
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

		ent->client->inventory[ITEM_ITEM_ARMOR_JACKET] = 0;
		ent->client->inventory[ITEM_ITEM_ARMOR_COMBAT] = 0;

		it = GETITEM(ITEM_ITEM_ARMOR_BODY);
		info = (gitem_armor_t *)it->info;
		ent->client->inventory[ITEM_ITEM_ARMOR_BODY] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = GETITEM(ITEM_ITEM_POWER_SHIELD);
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
			ent->client->inventory[i] = 1;
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
			ent->client->inventory[index] = atoi(gi.argv(2));
		else
			ent->client->inventory[index] += it->quantity;
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

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, "%s", msg);
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

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, "%s", msg);
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

	if (!sv_cheats->value)
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

	gi.cprintf (ent, PRINT_HIGH, "%s", msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int				index;
	const gitem_t	*it;
	char			*s;

	//spectators can't use things!
	if (!ent->client->pers.team)
		return;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD || (int)g_gamemode->value == GAMEMODE_ITDM)
		return;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "Unknown item: %s\n", s);
		return;
	}

	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->inventory[index])
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
	int				index;
	const gitem_t	*it;
	char			*s;

	//spectators can't drop!
	if (!ent->client->pers.team)
		return;

	//wision: drop item during warmup (instagib) is soooo ugly
	if ((tdm_match_status < MM_PLAYING || tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD) ||
		(int)g_gamemode->value == GAMEMODE_ITDM)
	{
		return;
	}

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "Unknown item: %s\n", s);
		return;
	}

	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->inventory[index])
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
/*void Cmd_Inven_f (edict_t *ent)
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

	if (cl->menu.active)
	{
		PMenu_Close (ent);
		return;
	}
	else if (cl->pers.team == TEAM_SPEC)
	{
		TDM_ShowTeamMenu (ent);
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->inventory[i]);
	}
	gi.unicast (ent, true);
}*/

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	const gitem_t		*it;

	if (ent->client->pers.menu.active)
	{
		PMenu_Select (ent);
		return;
	}

	if (tdm_match_status == MM_TIMEOUT)
		return;

	//spectators can swap chase mode only
	if (!ent->client->pers.team)
	{
		if (ent->client->chase_target)
			NextChaseMode (ent);
		return;
	}
			

	if (ent->health <= 0)
		return;

	ValidateSelectedItem (ent);

	if (ent->client->selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->selected_item];
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
	gclient_t		*cl;
	int				i, index;
	const gitem_t	*it;
	int				selected_weapon;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	cl = ent->client;

	if (!cl->weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->weapon == it)
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
	gclient_t		*cl;
	int				i, index;
	const gitem_t	*it;
	int				selected_weapon;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	cl = ent->client;

	if (!cl->weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->weapon == it)
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
	gclient_t			*cl;
	int					index;
	const gitem_t		*it;

	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	cl = ent->client;

	if (!cl->weapon || !cl->lastweapon)
		return;

	index = ITEM_INDEX(cl->lastweapon);
	if (!cl->inventory[index])
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
	const gitem_t		*it;

	// wision: decrease a value in voting menu if it's up
	if (ent->client->pers.votemenu_values.show)
	{
		VoteMenuDecreaseValue (ent);
		return;
	}

	//spectators can't drop!
	if (!ent->client->pers.team)
		return;

	//wision: drop item during warmup (instagib) is soooo ugly
	if ((tdm_match_status < MM_PLAYING || tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD) ||
		(int)g_gamemode->value == GAMEMODE_ITDM)
	{
		return;
	}

	ValidateSelectedItem (ent);

	if (ent->client->selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->selected_item];
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
	if (!ent->client->pers.team)
		return;

	if ((level.framenum - ent->client->respawn_framenum) < SECS_TO_FRAMES (5))
		return;

	if (tdm_match_status == MM_COUNTDOWN || tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
		return;

	if (ent->health <= 0)
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
	ent->client->showmotd = false;
	ent->client->showscores = false;
	ent->client->showoldscores = false;

	//ent->client->showhelp = false;
	//ent->client->showinventory = false;

	if (ent->client->pers.menu.active)
		PMenu_Close (ent);
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
/*void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < game.maxclients; i++)
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
}*/

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;
	gclient_t *cl;

	if (tdm_match_status == MM_TIMEOUT)
		return;

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	// can't wave when dead !
	if (ent->health <= 0)
		return;

	//wision: wave flood protection (yeah.. 'some' players are so annoying)
	if (flood_waves->value)
	{
		cl = ent->client;

		if (level.framenum < cl->resp.flood_waves_locktill)
		{
//			gi.cprintf(ent, PRINT_HIGH, "You can't use waves for %d more minutes\n",
//				(int)((cl->resp.flood_waves_locktill - level.framenum)/(60 * SERVER_FPS)));
			return;
		}
		i = cl->resp.flood_waves_whenhead - flood_waves->value + 1;
		if (i < 0)
			i = (sizeof(cl->resp.flood_waves_when)/sizeof(cl->resp.flood_waves_when[0])) + i;
		if (cl->resp.flood_waves_when[i] && (level.framenum - cl->resp.flood_waves_when[i]) < flood_waves_waitdelay->value * (60 * SERVER_FPS))
		{
			cl->resp.flood_waves_locktill = level.framenum + (flood_waves_waitdelay->value * (60 * SERVER_FPS));
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't use waves for %d minutes.\n", (int)(flood_waves_waitdelay->value));
			return;
		}
		cl->resp.flood_waves_whenhead = (cl->resp.flood_waves_whenhead + 1) %	(sizeof(cl->resp.flood_waves_when)/sizeof(cl->resp.flood_waves_when[0]));
		cl->resp.flood_waves_when[cl->resp.flood_waves_whenhead] = level.framenum;
	}

	i = atoi (gi.argv(1));

	ent->client->anim_priority = ANIM_WAVE;

	//FIXME: adjusting s.frame outside of allowed 10hz interval here, will skip frames at non-default server FPS

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
	int			i, j;
	edict_t		*other;
	char		*p;
	char		text[2048];
	gclient_t	*cl;
	int			expandpoint;

	if (gi.argc () < 2 && !arg0)
		return;

	// wision: don't allow spectators to talk during shutup mode
	if (!ent->client->pers.team && !ent->client->pers.admin && g_chat_mode->value == 2)
		return;
	

	// disable mm1 and mm2 for spectators. for players in team disable only mm1
	if (ent->client->pers.mute_frame > level.framenum && (!ent->client->pers.team || (!team && ent->client->pers.team)))
	{
		gi.cprintf(ent, PRINT_HIGH, "You are muted for %d more seconds\n", FRAMES_TO_SECS (ent->client->pers.mute_frame - level.framenum));
		return;
	}

	if (tdm_match_status != MM_TIMEOUT && tdm_match_status > MM_COUNTDOWN && tdm_match_status < MM_SCOREBOARD &&
		!ent->client->pers.team && !ent->client->pers.admin && g_chat_mode->value == 1)
	{
		//Observers can talk only to each other during the match.
		team = true;
	}

	if (team)
		expandpoint = Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		expandpoint = Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

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
		strcat (text, p);
	}

	// don't let text be too long for malicious reasons
	text[256] = 0;

	// no message!
	if (!*(text + expandpoint))
		return;

	if (ent->client->pers.team)
		TDM_MacroExpand (ent, text + expandpoint, sizeof(text) - expandpoint - 1);

	// macro expansion possibly removed the text?
	if (!*(text + expandpoint))
		return;

	strcat(text, "\n");

	//wision: fixed.. but still dunno how it does work :x
	//wision: we don't want to block say_team
	if (flood_msgs->value && !team)
	{
		cl = ent->client;

		if (level.realframenum < cl->resp.flood_locktill)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)((cl->resp.flood_locktill - level.realframenum) * FRAMETIME));
			return;
		}
		i = cl->resp.flood_whenhead - flood_msgs->value + 1;
		if (i < 0)
			i = (sizeof(cl->resp.flood_when)/sizeof(cl->resp.flood_when[0])) + i;
		if (cl->resp.flood_when[i] && (level.realframenum - cl->resp.flood_when[i]) < flood_waitdelay->value * SERVER_FPS)
		{
			cl->resp.flood_locktill = level.realframenum + (flood_waitdelay->value * SERVER_FPS);
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n", (int)(flood_waitdelay->value));
			return;
		}
		cl->resp.flood_whenhead = (cl->resp.flood_whenhead + 1) %	(sizeof(cl->resp.flood_when)/sizeof(cl->resp.flood_when[0]));
		cl->resp.flood_when[cl->resp.flood_whenhead] = level.realframenum;
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

void Cmd_PlayerList_f (edict_t *ent)
{
	char	st[128];
	char	text[1024];
	char	ip[32];
	int		players = 0;
	cvar_t	*sv_reserved_slots;
	edict_t	*e2;

	// find out how many reserved slots there are
	sv_reserved_slots = gi.cvar ("sv_reserved_slots", NULL, 0);

	// show players' ips for admin
	if (ent->client->pers.admin)
		strcpy (text, "id    time  ping   score                ip  name          team\n"
					"----------------------------------------------------------------\n");
	else
		strcpy (text, "id    time  ping   score  name          team\n"
					"----------------------------------------------\n");

	ip[0] = '[';

	for (e2 = g_edicts + 1; e2 <= g_edicts + game.maxclients; e2++)
	{
		if (!e2->inuse)
			continue;

		if (e2->client->pers.mvdclient)
			continue;

		if (ent->client->pers.admin)
		{
			char	*p;

			strcpy (ip + 1, e2->client->pers.ip);

			p = strchr (ip, ':');
			if (p)
			{
				p[0] = ']';
				p[1] = '\0';
			}
		}

		Com_sprintf (st, sizeof(st), "%2d  %3d:%02d  %4d     %3d %s %-13s %s%s\n",
			(int)(e2 - g_edicts - 1),
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			ent->client->pers.admin ? va ("%17s ", ip) : "",
			e2->client->pers.netname,
			teaminfo[e2->client->pers.team].name,
			e2->client->pers.team == TEAM_SPEC && e2->client->chase_target ? 
				va ("->%s", e2->client->chase_target->client->pers.netname) : "");

		if (strlen(text) + 1 + strlen(st) > sizeof(text) - 50)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s", text);
			text[0] = 0;
		}

		strcat (text, st);
		players++;
	}

	//force flush just to be safe
	gi.cprintf (ent, PRINT_HIGH, "%s", text);
	text[0] = 0;

	strcat (text, va ("\nServer status: "));
	if (sv_reserved_slots)
	{
		if (players >= game.maxclients - (int)sv_reserved_slots->value)
			strcat (text, TDM_SetColorText (va ("%d/%d\n", players, game.maxclients - (int)sv_reserved_slots->value)));
		else
			strcat (text, va("%d/%d\n", players, game.maxclients - (int)sv_reserved_slots->value));
	}
	else
	{
		if (players >= game.maxclients)
			strcat (text, TDM_SetColorText (va ("%d/%d\n", players, game.maxclients)));
		else
			strcat (text, va("%d/%d\n", players, game.maxclients));
	}

	gi.cprintf (ent, PRINT_HIGH, "%s", text);
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

	ent->client->last_activity_frame = level.framenum;

	/*if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}*/
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
	if (Q_stricmp(cmd, "playerlist") == 0 || Q_stricmp (cmd, "players") == 0 || Q_stricmp (cmd, "details") == 0)
	{
		Cmd_PlayerList_f(ent);
		return;
	}

	if (level.intermissionframe)
		return;

	if (TDM_Command (cmd, ent))
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
	//else if (Q_stricmp (cmd, "inven") == 0)
	//	Cmd_Inven_f (ent);
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
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
