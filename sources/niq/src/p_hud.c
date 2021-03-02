#include "g_local.h"

#ifdef NIQ
#include  "g_niq.h"
// prototype for player id in regular DM mode
void CTFSetIDView(edict_t *ent);
#endif

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	gclient_t	*client;

	client = ent->client;

	if (deathmatch->value || coop->value)
		{
		ent->client->showscores = true;
		}

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
	client->quad_framenum = 0;
	client->invincible_framenum = 0;
	client->breather_framenum = 0;
	client->enviro_framenum = 0;
	client->grenade_blew_up = false;
	client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (!ent->bot_client && (deathmatch->value || coop->value))
	{
//#ifdef NIQ
		if(niq_enable->value)
		{
			niq_deathmatchscoreboardmessage (ent, NULL, true);
		}
//#else
		else
		{
			DeathmatchScoreboardMessage (ent, NULL);
			gi.unicast (ent, true);
		}
//#endif
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

//ZOID
	if (deathmatch->value && ctf->value)
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

#ifdef EBOTS
void TeamplayScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k, t;
	int		sorted[MAX_CLIENTS];

	bot_team_t	*sortedteams[MAX_TEAMS];
	int			doneteam[MAX_TEAMS];
	int			numteams, best, bestscore;

	int		total;

#ifndef NIQ
	int		sortedscores[MAX_CLIENTS];
	int		score;
#else
	float	sortedscores[MAX_CLIENTS];
	float	score;
	int		iScore;
#endif

	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;

	// clear the doneteam flags
	memset(doneteam, 0, sizeof(int) * MAX_TEAMS);

	numteams = 0;	// incremented each time we add a team to the list

	// sort the teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame)
			continue;

		bestscore = -999999;
		best = -1;

		// find the highest scoring team
		for (j=0; j<MAX_TEAMS; j++)
		{
			if (!bot_teams[j])
				break;

			if (doneteam[j])
				continue;
			if (!bot_teams[j]->ingame)
				continue;

			if (bot_teams[j]->score > bestscore)
			{
				best = j;
				bestscore = bot_teams[j]->score;
			}
		}

		if (best > -1)
		{
			doneteam[best] = true;
			sortedteams[numteams] = bot_teams[best];
			numteams++;
		}
		else	// must be done
		{
			break;
		}
	}

	string[0] = 0;
	stringlength = strlen(string);

	for (t=0; t<numteams; t++)
	{	// print each team/player entry, maximum of 4 players per team
		if (t > 3)	// only print the top 4 teams
			break;

		// sort the clients by score
		total = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			if (cl_ent->client->team != sortedteams[t])
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

		if (total > 4)
			total = 4;

		x = 160;
		y = (t * 48);

		if (ent->client->team == sortedteams[t])
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn tag1 ", 32, y);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// Send team info
		Com_sprintf (entry, sizeof(entry),
			"xv %i yv %i string \"%s\" ", 70, y+6, sortedteams[t]->teamname);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;

		// Send team score info
#ifndef NIQ
		Com_sprintf (entry, sizeof(entry),
			"xv %i yv %i string \"%i\" ", 80, y + 20, sortedteams[t]->score);
#else
		if(sortedteams[t]->score >= 0)
			iScore = (int)(sortedteams[t]->score + 0.5);
		else
			iScore = (int)(sortedteams[t]->score - 0.5);

		Com_sprintf (entry, sizeof(entry),
			"xv %i yv %i string \"%i\" ", 80, y + 20, iScore);
#endif

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			y = (t * 48) + (i * 10);

#ifndef NIQ
			if (ent == cl_ent)
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string \"%3i %s\" ",
					x, y, cl->resp.score, cl->pers.netname);
			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string2 \"%3i %s\" ",
					x, y, cl->resp.score, cl->pers.netname);
			}
#else
			if(cl->resp.score >= 0)
				iScore = (int)(cl->resp.score + 0.5);
			else
				iScore = (int)(cl->resp.score - 0.5);

			if (ent == cl_ent)
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string \"%3i %s\" ",
					x, y, iScore, cl->pers.netname);
			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string2 \"%3i %s\" ",
					x, y, iScore, cl->pers.netname);
			}
#endif

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;

			strcpy (string + stringlength, entry);
			stringlength += j;
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
#endif

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		total;
#ifndef NIQ
	int		sortedscores[MAX_CLIENTS];
	int		score;
#else
	float	sortedscores[MAX_CLIENTS];
	float	score;
	int		iScore;
#endif

	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

//ZOID
    // niq: niq_sbmode != NIQ_MAXSCOREBOARD ==> regular deathmatch scoreboard (even in ctf mode)
	if (ctf->value && (!niq_enable->value || ent->client->pers.niq_sbmode == NIQ_MAXSCOREBOARD)) {
		CTFScoreboardMessage (ent, killer);
		return;
	}
//ZOID

#ifdef EBOTS
	if (niq_ebots->value && teamplay->value && ent->client->team)
	{
		TeamplayScoreboardMessage(ent, killer);
		return;
	}
#endif

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
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

		if (cl_ent->bot_client)
		{
			cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
			if (cl->ping < 0)
				cl->ping = 0;
		}

#ifndef NIQ
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping + (int)(cl->latency + (random() * 25)), (level.framenum - cl->resp.enterframe)/600);
#else
		// send the layout
        if(cl->resp.score < 0)
            iScore = (int)(cl->resp.score - 0.5);
        else
            iScore = (int)(cl->resp.score + 0.5);

		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], iScore, cl->ping + (int)(cl->latency + (random() * 25)), (level.framenum - cl->resp.enterframe)/600);
#endif

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

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
//#ifndef NIQ
	if(!niq_enable->value)
	{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
	}
//#else
	else
	{
		niq_deathmatchscoreboardmessage (ent, NULL, true);
	}
//#endif
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;

		if(niq_enable->value)
			niq_updatescreen(ent);

		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);

	// niq: makes SB appear quicker when hitting F1 with CTF join menu up
	if(ctf->value)
		niq_updatescreen(ent);
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
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory	= false;
	ent->client->showscores		= false;

#ifdef NIQ
	ent->client->resp.niq_helpmode = 0;
#endif

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
//#ifdef NIQ
	if(niq_enable->value)
		niq_setstats(ent);
//#else
	else
	{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

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
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

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
			|| ent->client->showscores || ent->client->resp.niq_helpmode)
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
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

#ifdef NIQ
	if(!ctf->value)
		{
		// If i use 0, name is the name of the map, so CTF must clear something 
		// somewhere. 255 seems to work OK and we should never have this many clients?
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 255;

		// niq: don't allow player id if a scoreboard/menu etc. is up
		if(ent->client->resp.id_state && !ent->client->showhelp && !ent->client->showscores && (!ent->client->resp.niq_helpmode || (ent->client->resp.niq_helpmode == NIQ_MAXHELPSCREEN)))
			CTFSetIDView(ent);
		}
	else
		SetCTFStats(ent);
#else
//ZOID
	if (ctf->value)
		SetCTFStats(ent);
//ZOID
#endif
	}
//#endif
}

