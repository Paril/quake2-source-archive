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

        // GRIM
        if (max_teams)
        {
                TeamScoreBoard (ent);
		return;
	}
        // GRIM

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
        //ent->client->showhelp = false;

        // GRIM
	if (ent->client->menu)
		PMenu_Close(ent);
        // GRIM

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
                Cmd_MeGoal_f (ent); // Dirty
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
                level.found_goals[0], level.total_goals[0], // GRIM
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

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
}

// GRIM
//=======================================================================
// HUD SHIT
//=======================================================================

/*
===============
SetWoundsHud - Call when ever your wound status changes...
        NOTE : Wound icons go opposite to wound side.
                The icon names are confusing at the moment,
                coz originally, it wasn't like that :(
===============
*/
void SetWoundsHud (edict_t *ent)
{
        int             icon;

        // just been wounded, flash wound icons to get players attention...
        if ((level.time < (ent->client->wound_flash_time - 0.3)) && (level.framenum & 4))
        {
                ent->client->ps.stats[STAT_HEAD] = 0;
                ent->client->ps.stats[STAT_CHEST] = 0;
                ent->client->ps.stats[STAT_L_LEG] = 0;
                ent->client->ps.stats[STAT_R_LEG] = 0;
                ent->client->ps.stats[STAT_L_ARM] = 0;
                ent->client->ps.stats[STAT_R_ARM] = 0;
                return;
        }

        if (ent->client->pers.wounds[W_RARM] > 6)
                icon = gi.imageindex ("w_la_r");
        else if (ent->client->pers.wounds[W_RARM] > 3)
                icon = gi.imageindex ("w_la_y");
        else if (ent->client->pers.wounds[W_RARM] > 0)
                icon = gi.imageindex ("w_la_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_R_ARM] = icon;

        if (ent->client->pers.wounds[W_LARM] > 6)
                icon = gi.imageindex ("w_ra_r");
        else if (ent->client->pers.wounds[W_LARM] > 3)
                icon = gi.imageindex ("w_ra_y");
        else if (ent->client->pers.wounds[W_LARM] > 0)
                icon = gi.imageindex ("w_ra_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_L_ARM] = icon;

        if (ent->client->pers.wounds[W_RLEG] > 6)
                icon = gi.imageindex ("w_ll_r");
        else if (ent->client->pers.wounds[W_RLEG] > 3)
                icon = gi.imageindex ("w_ll_y");
        else if (ent->client->pers.wounds[W_RLEG] > 0)
                icon = gi.imageindex ("w_ll_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_R_LEG] = icon;

        if (ent->client->pers.wounds[W_LLEG] > 6)
                icon = gi.imageindex ("w_rl_r");
        else if (ent->client->pers.wounds[W_LLEG] > 3)
                icon = gi.imageindex ("w_rl_y");
        else if (ent->client->pers.wounds[W_LLEG] > 0)
                icon = gi.imageindex ("w_rl_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_L_LEG] = icon;

        if (ent->client->pers.wounds[W_HEAD] > 6)
                icon = gi.imageindex ("w_h_r");
        else if (ent->client->pers.wounds[W_HEAD] > 3)
                icon = gi.imageindex ("w_h_y");
        else if (ent->client->pers.wounds[W_HEAD] > 0)
                icon = gi.imageindex ("w_h_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_HEAD] = icon;

        if (ent->client->pers.wounds[W_CHEST] > 6)
                icon = gi.imageindex ("w_ch_r");
        else if (ent->client->pers.wounds[W_CHEST] > 3)
                icon = gi.imageindex ("w_ch_y");
        else if (ent->client->pers.wounds[W_CHEST] > 0)
                icon = gi.imageindex ("w_ch_g");
        else 
                icon = 0;

        ent->client->ps.stats[STAT_CHEST] = icon;
}


/*
===============
SetAmmoHud - Call when ever you fire a weapon, reload a weapon etc
===============
*/
void A2HudOff (edict_t *ent)
{
        ent->client->ps.stats[STAT_AMMO_ICON2] = 0;
        ent->client->ps.stats[STAT_AMMO2] = 0;
}

void A1HudOff (edict_t *ent)
{
        ent->client->ps.stats[STAT_AMMO_ICON] = 0;
        ent->client->ps.stats[STAT_AMMO] = 0;
}

void SetAmmoHud (edict_t *ent)
{
	gitem_t		*item;
        weapons_t       *wep;
        int             i;

        if (ent->client->ammo_index2 > 0)
        {
                i = 0;
                wep = ent->client->pers.offhand;
                item = &itemlist[ent->client->ammo_index2];
                if (ent->client->pers.weapon2->flags & IT_RELOAD)
                        i = wep->quantity;
                else
                        i = ent->client->pers.inventory[ent->client->ammo_index2];

                if (i >= 0)
                {
                        ent->client->ps.stats[STAT_AMMO_ICON2] = gi.imageindex (item->icon);
                        ent->client->ps.stats[STAT_AMMO2] = i;
                }
        }
        else
        {
                ent->client->ps.stats[STAT_AMMO_ICON2] = 0;
                ent->client->ps.stats[STAT_AMMO2] = 0;
        }


        if (ent->client->ammo_index > 0)
        {
                i = 0;
                wep = ent->client->pers.goodhand;
                item = &itemlist[ent->client->ammo_index];
                if (ent->client->pers.weapon->flags & IT_RELOAD)
                        i = wep->quantity;
                else
                        i = ent->client->pers.inventory[ent->client->ammo_index];
                
                if (i >= 0)
                {
                        ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
                        ent->client->ps.stats[STAT_AMMO] = i;
                }
        }
        else
        {
                ent->client->ps.stats[STAT_AMMO_ICON] = 0;
                ent->client->ps.stats[STAT_AMMO] = 0;
        }
}


/*
===============
G_SetStats
===============
*/

void G_SetSpectatorStats (edict_t *ent);

void G_SetStats (edict_t *ent)
{
        qboolean        hud_off = false;

        // Check for chase target...
        if (ent->client->chase_target && ent->client->chase_target->inuse && (ent->movetype == MOVETYPE_NOCLIP))
        {
                G_SetSpectatorStats (ent);
                return;
        }
        else if (ent->deadflag || (ent->movetype == MOVETYPE_NOCLIP))
        {
                hud_off = true;
                ent->client->ps.stats[STAT_WEIGHT] = gi.imageindex ("wt_10");
        }

        //
	// health
	//
        ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
        // mostly done elsewhere...
        if (hud_off)
        {
                ent->client->ps.stats[STAT_AMMO_ICON2] = 0;
                ent->client->ps.stats[STAT_AMMO2] = 0;
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
        }

	//
	// armor
	//
        // Just if hud off, remove armor icon...
        if (hud_off || (!ent->client->pers.armor))
                ent->client->ps.stats[STAT_ARMOR_ICON] = 0;

	//
        // weight
	//
        // Done elsewhere...

	//
        // wounds
	//
        // mostly done elsewhere...
        if (ent->client->wound_flash_time > level.time)
                SetWoundsHud (ent);


	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// timers
	//
        /*
	{
                ent->client->ps.stats[STAT_GEN_ICON1] = 0;
                ent->client->ps.stats[STAT_GEN_COUNT1] = 0;
	}
        */

	//
	// selected item
	//
        if (hud_off)
                ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
        else
        {
                if (ent->client->pers.selected_item == -1)
                        ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
                else
                        ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);
        }

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

        if (level.intermissiontime || ent->client->showscores)
                ent->client->ps.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
                ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
        if (hud_off)
		ent->client->ps.stats[STAT_HELPICON] = 0;
        else if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
        // Dirty
        else if (level.framenum & 4)
                ent->client->ps.stats[STAT_HELPICON] = 0;
        // Dirty

        // no longer do this every frame... do once every 4 (8?)
        if ((!hud_off) && ent->client->resp.id_state)
        {
                if (level.framenum & 4)
                {
                        ent->client->ps.stats[STAT_ID_VIEW] = 0;
                        SetIDView(ent);
                }
        }
        else
                ent->client->ps.stats[STAT_ID_VIEW] = 0;

        if ((!ent->goal) || hud_off)
                ent->client->ps.stats[STAT_GOAL1] = 0;
        else
                SetGoal1 (ent);

        if ((!ent->req && !ent->audio) || hud_off)
                ent->client->ps.stats[STAT_GOAL2] = 0;
        else
                SetGoal2 (ent);
}
// GRIM

/*
===============
G_CheckChaseStats
===============
*/
// GRIM - This needed anymore?
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++)
        {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
                G_SetSpectatorStats (g_edicts + i);
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

        // GRIM
	if (!cl->chase_target)
        {
		G_SetStats (ent);
                return;
        }

        memcpy (cl->ps.stats, cl->chase_target->client->ps.stats, sizeof(cl->ps.stats));
        // GRIM

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;

        if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;
        
        // GRIM STAT_ID_VIEW instead of STAT_CHASE
	if (cl->chase_target && cl->chase_target->inuse)
                cl->ps.stats[STAT_ID_VIEW] = CS_PLAYERNAMES + (cl->chase_target - g_edicts) - 1;
	else
                cl->ps.stats[STAT_ID_VIEW] = 0;

        if ((cl->chase_target->client->ps.fov != cl->ps.fov) && (cl->chase_mode == CHASE_POV))
                cl->ps.fov = cl->chase_target->client->ps.fov;
        // GRIM
}
