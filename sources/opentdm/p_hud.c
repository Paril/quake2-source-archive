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
#include "g_tdm.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	PMenu_Close (ent);

	//MM_SCOREBOARD will set the layout
	ent->client->showoldscores = false;
	ent->client->showscores = false;
	ent->client->showmotd = false;

	VectorCopy (level.intermission_origin, ent->s.origin);

	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	VectorCopy (level.intermission_angle, ent->client->v_angle);

	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_state = GRENADE_NONE;
	ent->client->grenade_time = 0;

	ent->waterlevel = 0;
	ent->watertype = 0;

	//simulate player height
	ent->viewheight = 22;

	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->s.event = 0;
	ent->solid = SOLID_NOT;
	ent->client->weapon = NULL;
	ent->client->weapon_sound = 0;

	// add the layout
	gi.WriteByte (svc_layout);
	gi.WriteString (TDM_ScoreBoardString (ent));
	gi.unicast (ent, true);
}

void BeginIntermission (edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissionframe)
		return;		// already activated

	// respawn any dead clients
	for (i=0 ; i<game.maxclients ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	tdm_match_status = MM_SCOREBOARD;

	level.intermissionframe = level.framenum;
	level.changemap = targ->map;
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
		i = genrand_int32() & 3;
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
	for (i=0 ; i<game.maxclients ; i++)
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
		if (!cl_ent->inuse || game.clients[i].pers.team == TEAM_SPEC)
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
	//DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.WriteByte (svc_layout);
	gi.WriteString (TDM_ScoreBoardString (ent));
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
	//ent->client->showinventory = false;
	//ent->client->showhelp = false;

	if (ent->client->pers.menu.active)
	{
		PMenu_Close (ent);
		return;
	}

	ent->client->showmotd = false;

	// wision: switch between showing oldscore and current score during warmup
	if (ent->client->showoldscores)
	{
		ent->client->showoldscores = false;
		return;
	}

	if (tdm_match_status < MM_COUNTDOWN && ent->client->showscores && old_matchinfo.scoreboard_string[0])
	{
		TDM_OldScores_f (ent);
		return;
	}

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
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	Cmd_Score_f (ent);
}


//=======================================================================

//displays either current or previous team scores depending on which scoreboard is up
static void G_SetTeamScoreStats (edict_t *ent)
{
	int		first_team;

	if (ent->client->showoldscores)
	{
		if (old_matchinfo.scores[TEAM_A] < old_matchinfo.scores[TEAM_B])
			first_team = TEAM_B;
		else
			first_team = TEAM_A;

		ent->client->ps.stats[STAT_FIRST_TEAM_SCORE] = old_matchinfo.scores[first_team];
		ent->client->ps.stats[STAT_SECOND_TEAM_SCORE] = old_matchinfo.scores[(first_team % 2) + 1];
	}
	else
	{
		if (teaminfo[TEAM_A].score < teaminfo[TEAM_B].score)
			first_team = TEAM_B;
		else
			first_team = TEAM_A;

		ent->client->ps.stats[STAT_FIRST_TEAM_SCORE] = teaminfo[first_team].score;
		ent->client->ps.stats[STAT_SECOND_TEAM_SCORE] = teaminfo[(first_team % 2) + 1].score;
	}

	ent->client->ps.stats[STAT_FIRST_TEAM_NAME_INDEX] = CS_TDM_TEAM_A_NAME + first_team - 1;
	ent->client->ps.stats[STAT_SECOND_TEAM_NAME_INDEX] = CS_TDM_TEAM_A_NAME + (first_team % 2);

	ent->client->ps.stats[STAT_FIRST_TEAM_STATUS_INDEX] = CS_TDM_TEAM_A_STATUS + first_team - 1;
	ent->client->ps.stats[STAT_SECOND_TEAM_STATUS_INDEX] = CS_TDM_TEAM_A_STATUS + (first_team % 2);
}

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	const gitem_t	*item;
	int				index, cells;
	int				power_armor_type;

	cells = 0;

	ent->client->ps.stats[STAT_ID_VIEW_INDEX] = 0;

	if (!ent->client->pers.disable_id_view)
		if (TDM_GetPlayerIdView (ent))
			ent->client->ps.stats[STAT_ID_VIEW_INDEX] = CS_TDM_ID_VIEW;
	
	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = (ent->health & 0xFFFF);

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->inventory[ITEM_AMMO_CELLS];
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
		ent->client->ps.stats[STAT_ARMOR] = ent->client->inventory[index];
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
		ent->client->ps.stats[STAT_TIMER] = FRAMES_TO_SECS((ent->client->quad_framenum - level.framenum));
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = FRAMES_TO_SECS(ent->client->enviro_framenum - level.framenum);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = FRAMES_TO_SECS(ent->client->breather_framenum - level.framenum);
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	if (ent->client->invincible_framenum > level.framenum)
	{
		//is the usual timer in use?
		if (ent->client->ps.stats[STAT_TIMER])
		{
			//yes, show new timer for invuln
			ent->client->ps.stats[STAT_TIMER_PENT_ICON] = gi.imageindex ("p_invulnerability");
			ent->client->ps.stats[STAT_TIMER_PENT] = FRAMES_TO_SECS(ent->client->invincible_framenum - level.framenum);
		}
		else
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
			ent->client->ps.stats[STAT_TIMER] = FRAMES_TO_SECS(ent->client->invincible_framenum - level.framenum);

			// clear previous timer if there was something
			if (ent->client->ps.stats[STAT_TIMER_PENT])
			{
				ent->client->ps.stats[STAT_TIMER_PENT_ICON] = 0;
				ent->client->ps.stats[STAT_TIMER_PENT] = 0;
			}
		}
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_PENT_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER_PENT] = 0;
	}

	//
	// selected item
	//
	if (ent->client->selected_item == -1 || itemlist[ent->client->selected_item].icon == NULL)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (ent->health <= 0 || tdm_match_status == MM_SCOREBOARD || ent->client->showscores ||
			ent->client->pers.menu.active || ent->client->showoldscores || ent->client->showmotd)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;

	ent->client->ps.stats[STAT_GAME_STATUS_STRING_INDEX] = CS_TDM_GAME_STATUS;

	if (vote.active)
		ent->client->ps.stats[STAT_VOTE_STRING_INDEX] = CS_TDM_VOTE_STRING;
	else
		ent->client->ps.stats[STAT_VOTE_STRING_INDEX] = 0;
	//if (ent->client->showinventory && ent->health > 0)
	//	ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	//
	// frags
	//
	G_SetTeamScoreStats (ent);

	// frags for server browser
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
	// frags for ingame hud
	ent->client->ps.stats[STAT_SCORE] = ent->client->resp.score;

	ent->client->ps.stats[STAT_TIME_REMAINING] = CS_TDM_TIMELIMIT_STRING;

	if (tdm_match_status == MM_TIMEOUT)
		ent->client->ps.stats[STAT_TIMEOUT_STRING_INDEX] = CS_TDM_TIMEOUT_STRING;
	else
		ent->client->ps.stats[STAT_TIMEOUT_STRING_INDEX] = 0;

	//
	// help icon / current weapon if not shown
	//
	if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 90)
		&& ent->client->weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;
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
	else
	{
		memcpy (cl->ps.stats, cl->chase_target->client->ps.stats, sizeof(cl->ps.stats));

		//copy gun if in-eyes mode
		if (cl->chase_mode == CHASE_EYES)
		{
			cl->ps.gunindex = cl->chase_target->client->ps.gunindex;
			cl->ps.gunframe = cl->chase_target->client->ps.gunframe;
			VectorCopy (cl->chase_target->client->ps.gunangles, cl->ps.gunangles);

			//copy kickangles so hits/etc look realistic
			VectorCopy (cl->chase_target->client->ps.kick_angles, cl->ps.kick_angles);
		}

		//if our target player has the id stat up, we need to set configstrings for ourself.
		if (cl->ps.stats[STAT_ID_VIEW_INDEX])
			TDM_GetPlayerIdView (ent);

		//team scores are independent in spectator
		G_SetTeamScoreStats (ent);

		// wision: observers show 0 frags in server browser
		cl->ps.stats[STAT_FRAGS] = 0;
	}

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independent in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;

	if (tdm_match_status == MM_SCOREBOARD || cl->pers.menu.active || ent->client->showscores ||
			ent->client->showoldscores || ent->client->showmotd)
		cl->ps.stats[STAT_LAYOUTS] |= 1;

	//if (cl->showinventory && ent->health > 0)
	//	cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_TDM_SPECTATOR_STRINGS + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

