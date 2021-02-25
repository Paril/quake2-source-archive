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
	// ent->s.sound = 0; // ***** NH change *****
	if(last_beat)
	{
		ent->s.sound = gi.soundindex("misc/beat2.wav"); // ***** NH change *****
	}
	else
	{	
		ent->s.sound = gi.soundindex("misc/beat1.wav"); // ***** NH change *****
	}
	// ***** Start of NH changes *****
	// Added light effects fading in and out 
	gi.configstring(CS_LIGHTS+0, "abcccbaaaaaaabaaaaaaaa");
	// ***** End of NH changes *****

	ent->solid = SOLID_NOT;


	if (deathmatch->value || coop->value) // ***** NH change *****
	{
	        // ***** Start of NH changes ***** Night Hunter scoreboard
	        if (use_NH_scoreboard->value )
		  NHScoreboardMessage(ent, NULL) ;
		else 
	        // ***** End of NH changes *****
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

	// ***** NH change *****  so nobody becomes predator while in intermission
	level.pred_check_restart = level.time + 999; // ***** NH change *****

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

	// ***** Start of NH changes ***** CTF-like menu.
	// If the menu is showing, close it.
	if (ent->client->menu)
	  PMenu_Close(ent);
	// ***** End of NH changes *****
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

	        // // ***** Start of NH changes ***** Night Hunters scoreboard.
	        if (use_NH_scoreboard->value )
		  Cmd_NHScore_f(ent) ;
		else 
		// ***** End of NH changes *****

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
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	// ***** Start of NH changes ***** Panic mode.
	static int panic_end = 0 ;

	//
	// health
	//
	// STAT_HEALTH_ICON = 0
	// STAT_HEALTH = 1
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	// STAT_AMMO_ICON = 2
	// STAT_AMMO = 3
	if (!ent->client->ammo_index
	    /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
	  ent->client->ps.stats[STAT_AMMO_ICON] = 0;
	  ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
	  // ***** NH changes *****
	  // Overload icon for predator.
	  if ((ent->isPredator) &&
	      (enable_predator_overload->value ) &&
	      (ent->client->weapon_overload) &&
	      (ent->client->pers.weapon == FindItem("Rocket Launcher"))) 
	    ent->client->ps.stats[STAT_AMMO_ICON] = 
	      gi.imageindex("h_over") ;
	  else {
	  // ***** End of NH changes *****
	    item = &itemlist[ent->client->ammo_index];
	    ent->client->ps.stats[STAT_AMMO_ICON] =
	      gi.imageindex (item->icon);
	  }
	  ent->client->ps.stats[STAT_AMMO] =
	    ent->client->pers.inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	// STAT_ARMOR_ICON = 4
	// STAT_ARMOR = 5
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
	  cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
	  if (cells == 0)
	    {	// ran out of cells for power armor
	      ent->flags &= ~FL_POWER_ARMOR;
	      gi.sound(ent, CHAN_ITEM,
		       gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	      power_armor_type = 0;;
	    }
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	  {	// flash between power armor and other armor icon
	    ent->client->ps.stats[STAT_ARMOR_ICON] =
	      gi.imageindex ("i_powershield");
	    ent->client->ps.stats[STAT_ARMOR] = cells;
	  }
	else if (index)
	  {
	    item = GetItemByIndex (index);
	    ent->client->ps.stats[STAT_ARMOR_ICON] =
	      gi.imageindex (item->icon);
	    ent->client->ps.stats[STAT_ARMOR] =
	      ent->client->pers.inventory[index];
	}
	else
	{
	  ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
	  ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	// STAT_PICKUP_ICON = 7
	// STAT_PICKUP_STRING = 8
	if (level.time > ent->client->pickup_msg_time)  {
	    ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
	    ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	// STAT_TIMER_ICON = 9
	// STAT_TIMER = 10 
	if (ent->client->quad_framenum > level.framenum) {
	  ent->client->ps.stats[STAT_TIMER_ICON] =
	    gi.imageindex ("nhir"); // ***** NH change *****
	  ent->client->ps.stats[STAT_TIMER] =
	    (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum) {
	  ent->client->ps.stats[STAT_TIMER_ICON] =
	    gi.imageindex ("nhir"); // ***** NH change ***** 
	  ent->client->ps.stats[STAT_TIMER] =
	    (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)	{
	  ent->client->ps.stats[STAT_TIMER_ICON] =
	    gi.imageindex ("p_envirosuit");
	  ent->client->ps.stats[STAT_TIMER] =
	    (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum) {
	  ent->client->ps.stats[STAT_TIMER_ICON] =
	    gi.imageindex ("p_rebreather");
	  ent->client->ps.stats[STAT_TIMER] =
	    (ent->client->breather_framenum - level.framenum)/10;
	}
	// ***** Start of NH changes ***** IR goggles.
	else if (ent->client->IRgoggles_framenum > level.framenum) {

	  ent->client->ps.stats[STAT_TIMER_ICON] =
	    gi.imageindex ("nhir");	// NH IR
	  ent->client->ps.stats[STAT_TIMER] =
	    (ent->client->IRgoggles_framenum - level.framenum)/10;
	}
	// ***** End of NH changes *****
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// help icon / current weapon if not shown
	//
	// STAT_HELPICON = 11
	if ((ent->client->pers.hand == CENTER_HANDED || 
	     ent->client->ps.fov > 91) &&
	    ent->client->pers.weapon) 
	  ent->client->ps.stats[STAT_HELPICON] =
	    gi.imageindex (ent->client->pers.weapon->icon);
        else 
	  ent->client->ps.stats[STAT_HELPICON] = 0 ;

	//
	// selected item
	//
	// STAT_SELECTED_ITEM = 12
	if (ent->client->pers.selected_item == -1)
	  ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
	  ent->client->ps.stats[STAT_SELECTED_ICON] =
	    gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] =
	  ent->client->pers.selected_item;

	//
	// layouts
	//
	// STAT_LAYOUTS = 13
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)	{
	  if (ent->client->pers.health <= 0 ||
	      level.intermissiontime ||
	      ent->client->showscores ||
	      (ent->ShowMOTD > (int)level.time)) {
	    // ***** NH Change *****
	    ent->client->ps.stats[STAT_LAYOUTS] |= 1;

	  }
		
	  if ((ent->client->showinventory) && 
	      (ent->client->pers.health > 0)) {
	    ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	  }

	}
	else {
	  if (ent->client->showscores || ent->client->showhelp) { 
	    ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	  }	    

	  if ((ent->client->showinventory) &&
	      (ent->client->pers.health > 0)) {
	    ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	  }
	}

	//
	// frags
	//
	// STAT_FRAGS = 14
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	// ***** Start of NH changes *****	
	// 
	// Spectator
	// 
	// STAT_SPECTATOR = 17
	ent->client->ps.stats[STAT_SPECTATOR] = 0;

	//
	// Player icon
	//
	// STAT_PREDATOR = 19
	ent->client->ps.stats[STAT_PREDATOR] = gi.imageindex ("h_marine");
	if (ent->isPredator)
	  ent->client->ps.stats[STAT_PREDATOR] = gi.imageindex ("h_pred");
	// ***** End of NH changes *****


	//
	// Teleport panic timer.
	//
	// STAT_PANIC = 18
	// STAT_TELEPORT = 20
	ent->client->ps.stats[STAT_TELEPORT] = 0 ;
	ent->client->ps.stats[STAT_PANIC] = 0 ;

	if (ent->isPredator) {

	  // Panic mode.
	  if ((ent->client->teleport_panic_end > (int) level.time) &&
	      (!ent->client->teleport_stored)) {

	    ent->client->ps.stats[STAT_PANIC] =
	      ent->client->teleport_panic_end - (int) level.time ;
	  }

	  // Teleport.
	  ent->client->ps.stats[STAT_TELEPORT] =
	    ent->client->pers.teleport_shots  ;
	}

	// 
	// Flashlight
	//
	// STAT_NHFLASHLIGHT = 21
	if (ent->flashlight) 
	  ent->client->ps.stats[STAT_NHFLASHLIGHT] = gi.imageindex("nhflash") ;
	else
	  ent->client->ps.stats[STAT_NHFLASHLIGHT] = 0 ;

	// 
	// Safety mode.
	//
	// STAT_SAFETY = 23
	if (ent->safety) {

	//  gi.dprintf("STAT_SAFETY = %d\n", ent->client->ps.stats[STAT_SAFETY]);
	  if (ent->safety_end < (int) level.time) 
	    clearSafetyMode(ent) ;
	}

 	//
	// Predator start countdown.
	//
	// STAT_COUNT = 24
	// STAT_COUNT_TIME = 25
	//	if ((ent->isPredator) &&
	//	    (ent->inWaiting) &&
	if ((ent->isPredator) &&
	    (ent->inWaiting) &&
	    (level.be_pred_time > (int) level.time)) {

	  ent->client->ps.stats[STAT_COUNT] = 1 ;
	  ent->client->ps.stats[STAT_COUNT_TIME] = 
	    level.be_pred_time - (int) level.time ;
	}
	else
	  ent->client->ps.stats[STAT_COUNT] = 0 ;
	// ***** End of NH changes *****

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

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;

	// ***** NH Change *****
	if ((cl->pers.health <= 0) ||
	    level.intermissiontime ||
	    cl->showscores || 
	    (ent->ShowMOTD > (int)level.time))
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	
	if ((cl->showinventory) &&
	    (cl->pers.health > 0))
	  cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
		  (cl->chase_target - g_edicts) - 1;
	else
	{ // ***** NH change *****
		cl->ps.stats[STAT_CHASE] = 0;
		cl->ps.stats[STAT_NHFLASHLIGHT] = 0; // ***** NH change *****
		cl->ps.stats[STAT_OVERLOAD] = 0 ; // ***** NH change *****
	} // ***** NH change *****

}



























