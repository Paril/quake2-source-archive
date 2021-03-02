#include "g_local.h"

qboolean Jet_Active( edict_t *ent );

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

        //BEGIN! - Turret! --> look for intermissions...
        ent->client->onturret = 0;

	// clean up powerup info
  /*ATTILA begin*/
  ent->client->Jet_framenum = 0;
  /*ATTILA end*/
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->client->ps.rdflags &= ~RDF_IRGOGGLES;		// PGM
	ent->client->ir_framenum = 0;					// PGM
        ent->client->steroids_framenum = 0;
        ent->client->cloak_framenum = 0;
        ent->client->beans_framenum = 0;

/*-----/ PM /-----/ NEW:  Kill any flames /-----*/
	ent->burnout = 0;
/*----------------------------------------------*/

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

	if (ent -> client -> showscores || ent -> client -> showinventory)
		if (ent->client->pers.scanner_active)
			ent->client->pers.scanner_active = 2;

if (ent -> client -> showscores)
{

//ZOID
	if (ctf->value) {
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
// added ...
  }
else
*string = 0;

 // Scanner active ?
if (ent->client->pers.scanner_active & 1) 
        ShowScanner(ent,string);

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

//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

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
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;
//Wyrm: scanner
        ent->client->pers.scanner_active = 0;

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
int CTF_GetWinner();
void SetTechStat(edict_t *ent);

void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

        if ((ent->client->pers.hud == 2)||(ent->movetype == MOVETYPE_NOCLIP))
        {
                ent->client->ps.stats[STAT_SHOWCTF] = 0;
                ent->client->ps.stats[STAT_SHOWAMMO] = 0;
        }
        else if (!ent->client->pers.hud)
        {
                ent->client->ps.stats[STAT_SHOWCTF] = 0;
                ent->client->ps.stats[STAT_SHOWAMMO] = 1;
        }
        else
        {
                ent->client->ps.stats[STAT_SHOWCTF] = 1;
                ent->client->ps.stats[STAT_SHOWAMMO] = 0;
        }

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;
#if 0
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
#endif
        //Wyrm: weapon hud
        if (ent->client->pers.weapon)
                ent->client->ps.stats[STAT_WEAPON_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
        else
        {
                ent->client->ps.stats[STAT_WEAPON_ICON] = 0;
                ent->client->ps.stats[STAT_WEAPON_STRING] = 0;
        }

        //Wyrm: clear marks
        if (ent->client->ps.stats[STAT_SHOWAMMO])
        {
        ent->client->ps.stats[STAT_USEAMMO_MARK] = 0;
        ent->client->ps.stats[STAT_USEAMMO2_MARK] = 0;
                //Wyrm: ammo hud

                //Wyrm: ammo valors! :)
                index = ITEM_INDEX(FindItem("Shells"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc1");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc1");

                ent->client->ps.stats[STAT_AMMO_SHELLS] =  ent->client->pers.inventory[index];


                index = ITEM_INDEX(FindItem("Bullets"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc2");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc2");

                ent->client->ps.stats[STAT_AMMO_BULLETS] = ent->client->pers.inventory[index];

                index = ITEM_INDEX(FindItem("Grenades"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc3");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc3");

                ent->client->ps.stats[STAT_AMMO_GRENADES] = ent->client->pers.inventory[index];;

                index = ITEM_INDEX(FindItem("Rockets"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc4");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc4");

                ent->client->ps.stats[STAT_AMMO_ROCKETS] = ent->client->pers.inventory[index];;

                index = ITEM_INDEX(FindItem("Slugs"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc5");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc5");

                ent->client->ps.stats[STAT_AMMO_SLUGS] = ent->client->pers.inventory[index];;

                index = ITEM_INDEX(FindItem("Cells"));

                if (index == ent->client->ammo_index)
                        ent->client->ps.stats[STAT_USEAMMO_MARK] = gi.imageindex("marc6");
                else if (index == ent->client->ammo2_index)
                        ent->client->ps.stats[STAT_USEAMMO2_MARK] = gi.imageindex("marc6");

                ent->client->ps.stats[STAT_AMMO_CELLS] = ent->client->pers.inventory[index];;
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
                        ent->flags &= ~FL_POWER_SCREEN;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
                if (power_armor_type == POWER_ARMOR_SHIELD)
                        ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
                else
                        ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powerscreen");

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
  /*ATTILA begin*/
  if ( Jet_Active(ent) )
  {
    ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_jetpack");
    ent->client->ps.stats[STAT_TIMER] = ent->client->pers.inventory[ITEM_INDEX(FindItem("Jetpack"))] / 10;
  } 
  else
  /*ATTILA end*/
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
//Wyrm: prova
        else if (ent->client->cloak_framenum > level.framenum)
	{
                ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_cloak");
                ent->client->ps.stats[STAT_TIMER] = (ent->client->cloak_framenum - level.framenum)/10;
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
	else if (ent->client->ir_framenum > level.framenum)
	{
                ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_goggles");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->ir_framenum - level.framenum)/10;
	}
//Wyrm: Steroids
        else if (ent->client->steroids_framenum > level.framenum)
	{
                ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_adrenaline");
                ent->client->ps.stats[STAT_TIMER] = (ent->client->steroids_framenum - level.framenum)/10;
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
                        || ent->client->showscores || ent->client->pers.scanner_active)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
                if (ent->client->showscores || ent->client->showhelp || ent->client->pers.scanner_active)
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
        if (ctf->value)
        {
                if (level.intermissiontime)
                { // blink 1/8th second
                        int winner = CTF_GetWinner();

                        if (winner)
                        {
                                if (level.framenum & 8)
                                        ent->client->ps.stats[STAT_HELPICON]=0;
                                else if (winner == 1)
                                        ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("ctfsb1");
                                else if (winner == 2)
                                        ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("ctfsb2");
                        }
                }
                
        }
        else
        {
                if (ent->client->pers.helpchanged && (level.framenum&8) )
                        ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
                else
                        ent->client->ps.stats[STAT_HELPICON] = 0;
        }
	ent->client->ps.stats[STAT_SPECTATOR] = 0;

        SetTechStat(ent);

//ZOID
        if (ent->client->ps.stats[STAT_SHOWCTF])
                SetCTFStats(ent);
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

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

