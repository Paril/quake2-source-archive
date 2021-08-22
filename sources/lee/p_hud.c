#include "g_local.h"

char *ClientTeam (edict_t *ent);
int	powerup_frame;

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
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

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

void BeginIntermission (edict_t *ent)
{
	int		i, n;
	edict_t	*client;

	if (level.intermissiontime)
		return;		// already activated
	
	//ctf
	if (ctf->value)
		CTFCalcScores();

	game.autosaved = false;
//	ent->count = 0;

#ifdef MIDI
	//midi
	if (midi->value)
	{
		ShutdownMidi();
//			return;
		miditried = 0;
	}
#endif

	//if observer, make player
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BBOX;
	ent->svflags &= ~SVF_DEADMONSTER;

	level.intermissiontime = level.time;
	level.changemap = ent->map;

	// respawn any dead clients
	if (ctf->value || deathmatch->value || mbm->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			client = g_edicts + 1 + i;
			if (!client->inuse)
				continue;
			make_dead_player(client);	
			
			//		if (client->health <= 0)
			//			respawn(client);
		}
	
	//	make_dead_player(ent);	
	}

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

void TeamScoreboard (edict_t *ent, edict_t *killer)
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
	int		male, female, cyborg;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
  
	if (Q_stricmp(ent->classname, "bot") == 0)
		  return;

  // end if
  
  // sort the clients by score
	total = 0;
	male  = 0;
	female= 0;
	cyborg= 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		
		//remove observers
		if //(cl_ent->solid == SOLID_NOT)
			((cl_ent->client->resp.ctf_team == CTF_NOTEAM) &&
			(strcmp(cl_ent->classname, "player") == 0))
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

	// make a header for the data 
 
	string[0] = 0;

	stringlength = strlen(string);

	// print level name and exit rules
	Com_sprintf(entry, sizeof(entry), 
	//"xv 148 yv 234 
	"xv 108 yv 234 string2 \"  team frags to win : %.0f\" "
	"xv -90 yv 226 string2 \" \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37 \" "
	"xv -90 yv 4 string2 \" \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37 \" "
	"xv 90 yv -16 string2 \" Teamplay scoreboard \" "
	"xv 120 yv -6 string \" on '%s' \" "		//map name
	, teamlimit->value, level.level_name); 

	j = strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} 

	//add timelimit on to score board
	if (timelimit->value > 0)
	{
		float timeleft;

		//work out time left
		timeleft = (timelimit->value*60) - level.time;
		if (timeleft < 0)
			timeleft = 0;

		Com_sprintf(entry, sizeof(entry), 	
		"xv 148 yv 264 string2 \"    Time limit : %.0f mins\" " 
		"xv 148 yv 254 string2 \"Remaining time : %.f secs\" " 
		, timelimit->value, timeleft);
		
		//add to string
		j = strlen(entry); 
		if (stringlength + j < 1024) 
		{ 
			strcpy (string + stringlength, entry); 
			stringlength += j; 
		} 
	}
	else
	{
		//work out how long the game been going
		Com_sprintf(entry, sizeof(entry), 	
		"xv 148 yv 264 string2 \"  running time : %.0f secs\" "  
		, level.time);

		//add to string
		j = strlen(entry); 
		if (stringlength + j < 1024) 
		{ 
			strcpy (string + stringlength, entry); 
			stringlength += j; 
		} 
	}


	// add the clients in sorted order
	if (total > 12)
		total = 12;



	for (i=0 ; i<total ; i++)
	{

		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		//x = (i>=6) ? 160 : 0;
		//gi.dprintf ("%i\n",strcmp(ClientTeam(cl_ent),"male"));
		if (strcmp(ClientTeam(cl_ent),"male") == 0)
		{
			x = -80;
			TP_score_types[0]++;
			y = 24 + 32 * (TP_score_types[0]%6);
			male=male+cl->resp.score;
		}
		else if (strcmp(ClientTeam(cl_ent),"female") == 0)
		{
			TP_score_types[1]++;
			y = 24 + 32 * (TP_score_types[1]%6);
			x = 100;
			female = female + cl->resp.score;
		}
		else 
		{
			TP_score_types[2]++;
			y = 24 + 32 * (TP_score_types[2]%6);
			x = 260;
			cyborg=cyborg+cl->resp.score;
		}

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


	// ADD TEAM and score
	Com_sprintf(entry, sizeof(entry), 
	"xv -60 yv 20 string2 \" Team 1 \" "
	"xv 120 yv 20 string2 \" Team 2 \" "
	"xv 280 yv 20 string2 \" Team 3 \" "

	"xv -70 yv 30 string \" Score : %i\" "		//score
	"xv 110 yv 30 string \" Score : %i\" "
	"xv 270 yv 30 string \" Score : %i\" "
	, male, female, cyborg); 

	j = strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} 

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	TP_score_types[0] = 0;
	TP_score_types[1] = 0;
	TP_score_types[2] = 0;
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
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
  
  if (Q_stricmp(ent->classname, "bot") == 0)
      return;

  //teamplay
  if ((int)(dmflags->value) & (DF_MODELTEAMS))// | DF_SKINTEAMS))
  {
	TeamScoreboard(ent,killer);
	return;
  }
  
  //ctf
  if ((ctf->value) && (strcmp(ent->classname, "player") == 0))
	{
		CTFScoreboardMessage (ent, killer);
		return;
	}
  // end if

 
  // sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if //(cl_ent->solid == SOLID_NOT)
			((cl_ent->client->resp.ctf_team == CTF_NOTEAM) &&
			(strcmp(cl_ent->classname, "player") == 0))
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

	if (darkmatch->value)
		tag="DARKMATCH";
	else
		tag=" ";

	Com_sprintf(entry, sizeof(entry), 
	"xv 148 yv 234 string2 \"  Frags to win : %.0f\" "
	"xv -30 yv 226 string2 \" \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37 \" "
	"xv -30 yv 4 string2 \" \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37 \" "
	"xv 90 yv -16 string2 \" Deathmatch scoreboard \" "
	"xv 120 yv -6 string \" on '%s' \" "		//map name
	"xv 0 yv 234 string \" %s \" "				//darkmatch
	, fraglimit->value, level.level_name, tag); 

	j = strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} 

	//add timelimit on to score board
	if (timelimit->value > 0)
	{
		float timeleft;

		//work out time left
		timeleft = (timelimit->value*60) - level.time;
		if (timeleft < 0)
			timeleft = 0;

		Com_sprintf(entry, sizeof(entry), 	
		"xv 148 yv 264 string2 \"    Time limit : %.0f mins\" " 
		"xv 148 yv 254 string2 \"Remaining time : %.f secs\" " 
		, timelimit->value, timeleft);
		
		//add to string
		j = strlen(entry); 
		if (stringlength + j < 1024) 
		{ 
			strcpy (string + stringlength, entry); 
			stringlength += j; 
		} 
	} else
	{
		//work out how long the game been going
		Com_sprintf(entry, sizeof(entry), 	
		"xv 148 yv 264 string2 \"  running time : %.0f secs\" "  
		, level.time);

		//add to string
		j = strlen(entry); 
		if (stringlength + j < 1024) 
		{ 
			strcpy (string + stringlength, entry); 
			stringlength += j; 
		} 
	}

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{

		char *temp;

		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		//picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 210 : 0;
		y = 16 + 36 * (i%6); 
		//y = 32 + 32 * (i%6); 

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
		if (i == 0)
			temp= "st";
		else if (i == 1)
			temp="nd";
		else if (i == 2)
			temp="rd";
		else
			temp="th";

		//x <>
		//y up

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i "
			"xv %i yv %i string %i%s ",
			x, y, sorted[i], cl->resp.score, cl->ping, ((level.framenum - cl->resp.enterframe)/600),// );
			(x), (y-6), (i+1), temp);
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
	ent->client->showinventory = false;
	ent->client->showhelp = false;
	
	//ctf
	if (ctf->value && ent->client->menu && strcmp(ent->classname, "player") == 0) 
		PMenu_Close(ent);

	if (!deathmatch->value && !coop->value && !ctf->value)
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

	if (skill->value == 0)
		sk = "Easy";
	else if (skill->value == 1)
		sk = "Medium";
	else if (skill->value == 2)
		sk = "Hard";
	else
		sk = "Very hard";

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
  // bot
  if (Q_stricmp(ent->classname, "bot") == 0)
      return;
  // end if
 
  
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

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
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;
	char		*temp;

	//
	// health
	//

	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	if (ent->health > 0)
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
	else
		ent->client->ps.stats[STAT_HEALTH] = 0;

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
	powerup_frame++;
	if (powerup_frame >= 15)
		powerup_frame = 0;

	if (powerup_frame ==0)
		temp="m_cursor0";
	else if (powerup_frame ==1)
		temp="m_cursor1";
	else if (powerup_frame ==2)
		temp="m_cursor2";
	else if (powerup_frame ==3)
		temp="m_cursor3";
	else if (powerup_frame ==4)
		temp="m_cursor4";
	else if (powerup_frame ==5)
		temp="m_cursor5";
	else if (powerup_frame ==6)
		temp="m_cursor6";
	else if (powerup_frame ==7)
		temp="m_cursor7";
	else if (powerup_frame ==8)
		temp="m_cursor8";
	else if (powerup_frame ==9)
		temp="m_cursor9";
	else if (powerup_frame ==10)
		temp="m_cursor10";
	else if (powerup_frame ==11)
		temp="m_cursor11";
	else if (powerup_frame ==12)
		temp="m_cursor12";
	else if (powerup_frame ==13)
		temp="m_cursor13";
	else// if (powerup_frame ==14)
		temp="m_cursor14";

		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex (temp);//"p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
	powerup_frame++;

	if (powerup_frame >= 6)
		powerup_frame = 0;

	if (powerup_frame ==0)
		temp="inv0";
	else if (powerup_frame ==1)
		temp="inv1";
	else if (powerup_frame ==2)
		temp="inv2";
	else if (powerup_frame ==3)
		temp="inv3";
	else if (powerup_frame ==4)
		temp="inv4";
	else// if (powerup_frame ==5)
		temp="inv5";

		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex (temp);
			//"p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
	powerup_frame++;
	if (powerup_frame >= 8)
		powerup_frame = 0;

	if (powerup_frame ==0)
		temp="env0";
	else if (powerup_frame ==1)
		temp="env1";
	else if (powerup_frame ==2)
		temp="env2";
	else if (powerup_frame ==3)
		temp="env3";
	else if (powerup_frame ==4)
		temp="env4";
	else if (powerup_frame ==5)
		temp="env5";
	else if (powerup_frame ==6)
		temp="env6";
	else// if (powerup_frame ==7)
		temp="env7";

		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex (temp);
			//"p_envirosuit");
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
	// frags	or points
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
	if (ent->client->resp.id_state)
		CTFSetIDView(ent);

	if (mbm->value)
	{
		int mins, secs, time;
		div_t div_result;

		//work out remeining
		time = (level.mbm_timelimit * 60) - level.time;
		
		//current time
		div_result = div(time,60);
		mins = div_result.quot;
		secs = (time - (mins*60));

		//the ':'
		ent->client->ps.stats[22] = gi.imageindex ("2dots");

		//secs remining
		ent->client->ps.stats[21] = secs;

		//mins remining
		ent->client->ps.stats[20] = mins;

		if (secs < 10)
		{
			ent->client->ps.stats[23] = 1;
			ent->client->ps.stats[24] = 0;
		} else
			ent->client->ps.stats[23] = 0;
	}

	//work out current postion and rank
	else if ((deathmatch->value) && (strcmp(ent->classname, "player") == 0) && (ent->client->resp.ctf_team != CTF_NOTEAM) &&
		(!((int)(dmflags->value) & (DF_MODELTEAMS))))
	{
		edict_t *cl_ent;
		int players, pos, i, hscore, lscore;

		players=1;
		pos=1;
		hscore=0;
		lscore=0;
		ent->client->ps.stats[17] = 1;
		
		//only go though the clients				
		for (i = 0; i < maxclients->value; i++) 
		{
			cl_ent = g_edicts + 1 + i;

			if (!cl_ent->inuse)
				continue;

			if (cl_ent == ent)			
				continue;

			//don't count observers
//			if (cl_ent->solid == SOLID_NOT)
//				continue;
			if ((cl_ent->client->resp.ctf_team == CTF_NOTEAM) &&
				(strcmp(cl_ent->classname, "player") == 0))
				continue;

			players++;

			//check to see if thier score is higher than yours
			if (cl_ent->client->resp.score > ent->client->resp.score)
			{
				pos++;

				if (cl_ent->client->resp.score > hscore)
					hscore=cl_ent->client->resp.score;
			}

			else if (cl_ent->client->resp.score <= ent->client->resp.score)
					if (lscore<cl_ent->client->resp.score) 
						lscore=cl_ent->client->resp.score;
		}
		//position
		ent->client->ps.stats[20]=pos;
		//out of
		ent->client->ps.stats[21]=players;
		lscore=ent->client->resp.score - lscore;
		hscore=hscore - ent->client->resp.score;

		//gi.dprintf("pos:%i  lscore:%i  hscore:%i\n",pos,lscore,hscore);
		//work out the differents		
		if ((pos == 1) && (lscore> 0))
		{
			//your the highest
			ent->client->ps.stats[22]=lscore;
			ent->client->ps.stats[23]=1; //+
			ent->client->ps.stats[24]=0; //-
		}
		else if (pos == 1)
		{
			//no differents
			ent->client->ps.stats[22]=0;
			ent->client->ps.stats[23]=0; //+
			ent->client->ps.stats[24]=0; //-
		}
		else
		{
			//your are not the best
			ent->client->ps.stats[22]=hscore;
			ent->client->ps.stats[23]=0; //+
			ent->client->ps.stats[24]=1; //-
		}
	} 
	//teamplay score
	else if ((deathmatch->value) && (strcmp(ent->classname, "player") == 0) && (ent->client->resp.ctf_team != CTF_NOTEAM) &&
		(((int)(dmflags->value) & (DF_MODELTEAMS))))
	{
		int male, female, cyborg, i;
		gclient_t	*cl;
		edict_t		*cl_ent;

		ent->client->ps.stats[18] = 1;

		male=0;
		female=0;
		cyborg=0;

		for (i=0 ; i<maxclients->value ; i++)
		{
			
			cl = game.clients + i;
			cl_ent = g_edicts + 1 + i;
			
			if (!g_edicts[i+1].inuse)
				continue;

			//add up the score
			if (Q_stricmp(ClientTeam(cl_ent),"male") == 0)
				male = male+cl->resp.score;

			else if (Q_stricmp(ClientTeam(cl_ent),"female") == 0)
				female = female + cl->resp.score;

			else 		
				cyborg=cyborg+cl->resp.score;

		}

		ent->client->ps.stats[28] = male;
		ent->client->ps.stats[29] = female;
		ent->client->ps.stats[30] = cyborg;

		if (teams->value == 3)
			ent->client->ps.stats[31] = 1;
		else
			ent->client->ps.stats[31] = 0;
			

	}
	else if (ent->client->resp.ctf_team == CTF_NOTEAM)
	{
		ent->client->ps.stats[17] = 0;
		ent->client->ps.stats[18] = 0;
	}

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

	if ((deathmatch->value == 3) || (ctf->value))
		SetCTFStats(ent);
}


