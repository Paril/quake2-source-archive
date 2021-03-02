#include "g_local.h"



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->medikit_framenum = 0;
	ent->client->glowlamp_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->fog_fog = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

//ZOID
	if (ctf->value)
		CTFCalcScores();
//ZOID

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	if(ent->is_bot)
		return;

//ZOID
	if (ctf->value)
	{
		CTFScoreboardMessage (ent, killer);
		return;
	}
//ZOID
	
	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

// RipVTide

	if(holdthephone == 1)
	{
		its_me = ent;
		strcpy(string, ConversationRedraw());
	}

//	if(showingit == 1);
//	{
//		strcpy(string, Credits());
//	}

	ent->client->showinventory = false;
	ent->client->showhelp = false;


//	RipVTide

// gi.error("%s\n", string);

//	gi.dprintf ("%s\n", string);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	if(ent->is_bot)
		return;
	
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	if(ent->is_bot)
		return;

//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if(ent->is_bot)
		return;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	if(ent->is_bot)
		return;
	
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
//gi.configstring (CS_GENERAL+playernum, va("%s", ent->client->pers.netname));

void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
//	int			cells;
//	int			power_armor_type;
	int			poolmax;

	if(ent->is_bot)
		return;

//CaRRaC
//Force/Weapon menu stuff
	if(ent->client->force_user)
	{
		if(ent->client->pers.force.top_level)
		{
			poolmax = ceil(ent->client->pers.force.top_level*2);
			if(deathmatch->value)
			{
				if(poolmax <= 100)
				{
					if(ent->client->pers.force.pool < 100)
					{
						ent->client->pers.force.pool += 0.5;
						if(ent->client->pers.force.pool > 100)
						{
							ent->client->pers.force.pool = 100;
						}
					}
				}
				else if(ent->client->pers.force.pool < poolmax)
				{
					ent->client->pers.force.pool += 0.5;
				}
				else if(ent->client->pers.force.pool > poolmax)
				{
					ent->client->pers.force.pool = poolmax;
				}
			}
			else
			{
				if(ent->client->pers.force.pool < poolmax)
				{
					ent->client->pers.force.pool += 0.5;
					if(ent->client->pers.force.pool > poolmax)
						ent->client->pers.force.pool += poolmax;
				}
			}
		}
		ent->client->ps.stats[STAT_FORCE_ICON] = gi.imageindex(ent->client->pers.force_power->icon);
		ent->client->ps.stats[STAT_FORCE_POOL] = (short)ent->client->pers.force.pool;
	}
	else
	{
		ent->client->ps.stats[STAT_FORCE_ICON] = 0;
		ent->client->ps.stats[STAT_FORCE_POOL] = 0;
	}

	ent->client->ps.stats[STAT_FMENU] = 0;

	ent->client->ps.stats[STAT_FPOW1] = 0;
	ent->client->ps.stats[STAT_FPOW2] = 0;
	ent->client->ps.stats[STAT_FPOW3] = 0;
	ent->client->ps.stats[STAT_FPOW4] = 0;
	ent->client->ps.stats[STAT_FPOW5] = 0;

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->icon_menu_active == 2)
		{
			set_sub_menu1_weap(ent);
			set_sub_menu2_weap(ent);
			set_sub_menu3_weap(ent);
			set_sub_menu4_weap(ent);
		}
		else if (ent->client->force_user)
		{
			set_sub_menu1(ent);
			set_sub_menu2(ent);
			set_sub_menu3(ent);
			set_sub_menu4(ent);
			set_sub_menu5(ent);
//			set_sub_menu6(ent);
		}
	}
	else
	{
		ent->client->icon_menu_active = 0;
	}

	if(ent->client->in_snipe)
	{
//		ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("snipe2");
//RipVTide
//
		if(ent->client->mouse_s)
		{
			if (ent->client->ps.fov == 90)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic1");
				_stuffcmd(ent, "sensitivity \"%f\"\n", ent->client->mouse_s);
			}
			else if (ent->client->ps.fov > 84)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic2");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.1) );
			}
			else if (ent->client->ps.fov > 78)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic3");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.2) );
			}
			else if (ent->client->ps.fov > 72)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic4");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.3) );
			}
			else if (ent->client->ps.fov > 66)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic5");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.4) );
			}
			else if (ent->client->ps.fov > 60)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic6");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.5) );
			}
			else if (ent->client->ps.fov > 54)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic7");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.6) );
			}
			else if (ent->client->ps.fov > 48)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic8");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 1.8) );
			}
			else if (ent->client->ps.fov > 42)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic9");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 2) );
			}
			else if (ent->client->ps.fov > 36)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic10");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 2.2) );
			}
			else if (ent->client->ps.fov > 30)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic11");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 2.5) );
			}
			else if (ent->client->ps.fov > 26)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic12");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 2.7) );
			}
			else if (ent->client->ps.fov > 22)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic13");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 3) );
			}
			else if (ent->client->ps.fov > 18)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic14");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 4) );
			}
			else if (ent->client->ps.fov > 14)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic15");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 6) );
			}
			else if (ent->client->ps.fov > 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic16");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 8) );
			}
			else if (ent->client->ps.fov == 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic17");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 10) );
			}
			else if (ent->client->ps.fov < 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic18");
				_stuffcmd(ent, "sensitivity \"%f\"\n", (ent->client->mouse_s / 12) );
			}
			else
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic1");
			}
		}
		else
		{
			if (ent->client->ps.fov == 90)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic1");
			}
			else if (ent->client->ps.fov > 84)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic2");
			}
			else if (ent->client->ps.fov > 78)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic3");
			}
			else if (ent->client->ps.fov > 72)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic4");
			}
			else if (ent->client->ps.fov > 66)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic5");
			}
			else if (ent->client->ps.fov > 60)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic6");
			}
			else if (ent->client->ps.fov > 54)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic7");
			}
			else if (ent->client->ps.fov > 48)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic8");
			}
			else if (ent->client->ps.fov > 42)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic9");
			}
			else if (ent->client->ps.fov > 36)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic10");
			}
			else if (ent->client->zoom_factor > 30)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic11");
			}
			else if (ent->client->zoom_factor > 26)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic12");
			}
			else if (ent->client->zoom_factor > 22)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic13");
			}
			else if (ent->client->zoom_factor > 18)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic14");
			}
			else if (ent->client->zoom_factor > 14)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic15");
			}
			else if (ent->client->zoom_factor > 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic16");
			}
			else if (ent->client->zoom_factor == 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic17");
			}
			else if (ent->client->zoom_factor < 10)
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic18");
			}
			else
			{
				ent->client->ps.stats[STAT_SNIPEVIEW] = gi.imageindex("sniper/pic1");
			}
		}
//RipVTide
	}
	else
	{
		ent->client->ps.stats[STAT_SNIPEVIEW] = 0;
	}
	
	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
		ent->client->ps.stats[STAT_CLIPAMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		if (!strcmp("Thermals", item->pickup_name))
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
			ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
			ent->client->ps.stats[STAT_CLIPAMMO] = 0;
		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
			ent->client->ps.stats[STAT_CLIPAMMO] = ent->client->pers.inventory[ent->client->ammo_index];
			ent->client->ps.stats[STAT_AMMO] = ent->client->pers.clipammo[ITEM_INDEX(ent->client->pers.weapon)];
		}
	}
	
	//
	// armor
	//
//	power_armor_type = PowerArmorType (ent);
//	if (power_armor_type)
//	{
//		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
//		if (cells == 0)
//		{	// ran out of cells for power armor
//			ent->flags &= ~FL_POWER_ARMOR;
//			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
//			power_armor_type = 0;;
//		}
//	}

//	index = ArmorIndex (ent);
//	if (power_armor_type && (!index || (level.framenum & 8) ) )
//	{	// flash between power armor and other armor icon
//		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
//		ent->client->ps.stats[STAT_ARMOR] = cells;
//	}
//	else if (index)
//	{
//		item = GetItemByIndex (index);
//		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
//		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
//	}
//	else
//	{
//		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
//		ent->client->ps.stats[STAT_ARMOR] = 0;
//	}

	//
	// pickup message
	//
//	if (level.time > ent->client->pickup_msg_time)
//	{
//		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
//		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
//	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
/*	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;*/

//ZOID
	if(ctf->value)
	{
		SetCTFStats(ent);
	}
	else
	{
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
		if (ent->client->resp.id_state)
			CTFSetIDView(ent);
	}

//ZOID
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

//	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

//	if (cl->chase_target && cl->chase_target->inuse)
//		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
//			(cl->chase_target - g_edicts) - 1;
//	else
//		cl->ps.stats[STAT_CHASE] = 0;

//ZOID
	if(ctf->value)
	{
		SetCTFStats(ent);
	}
	else
	{
		cl->ps.stats[STAT_CTF_ID_VIEW] = 0;
		if (ent->client->resp.id_state)
			CTFSetIDView(ent);
	}

//ZOID
}


char *Credits()
{
	static m_code		draw[30];

	char				string[1024];

	int					charsup;

	strcpy(string, "xv 0 yv 0 picn back_b xv 0 yv 0 overlay ");

	//if(charsup < 30)
	//{
	//	while


}