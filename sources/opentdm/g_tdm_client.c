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

//TDM client stuff. Commands that are issued by or somehow affect a client are mostly
//here.

#include "g_local.h"
#include "g_tdm.h"

const int	teamJoinEntries[MAX_TEAMS] = {9, 3, 6};

/*
==============
TDM_AddPlayerToMatchinfo
==============
A new player joined mid-match (invite / etc), update teamplayerinfo to reflect this.
*/
void TDM_AddPlayerToMatchinfo (edict_t *ent)
{
	int				i;
	teamplayer_t	*new_teamplayers;

	if (!current_matchinfo.teamplayers)
		TDM_Error ("TDM_AddPlayerToMatchinfo: no teamplayers");

	//allocate a new teamplayers array
	new_teamplayers = gi.TagMalloc (sizeof(teamplayer_t) * (current_matchinfo.num_teamplayers + 1), TAG_GAME);
	memcpy (new_teamplayers, current_matchinfo.teamplayers, sizeof(teamplayer_t) * current_matchinfo.num_teamplayers);

	//setup this client
	TDM_SetupTeamInfoForPlayer (ent, new_teamplayers + current_matchinfo.num_teamplayers);

	//fix up pointers on existing clients and other things that reference teamplayers. NASTY!
	for (i = 0; i < current_matchinfo.num_teamplayers; i++)
	{
		if (current_matchinfo.captains[TEAM_A] == current_matchinfo.teamplayers + i)
			current_matchinfo.captains[TEAM_A] = new_teamplayers + i;
		else if (current_matchinfo.captains[TEAM_B] == current_matchinfo.teamplayers + i)
			current_matchinfo.captains[TEAM_B] = new_teamplayers + i;

		if (level.tdm_timeout_caller == current_matchinfo.teamplayers + i)
			level.tdm_timeout_caller = new_teamplayers + i;

		//might not have a current client (disconnected)
		if (current_matchinfo.teamplayers[i].client)
			current_matchinfo.teamplayers[i].client->client->resp.teamplayerinfo = new_teamplayers + i;
	}

	//remove old memory
	gi.TagFree (current_matchinfo.teamplayers);

	//move everything to new
	current_matchinfo.teamplayers = new_teamplayers;
	current_matchinfo.num_teamplayers++;
}

/*
==============
JoinedTeam
==============
A player just joined a team, so do things.
*/
void JoinedTeam (edict_t *ent, qboolean reconnected, qboolean notify)
{
	PMenu_Close (ent);

	if (notify)
	{
		if (g_gamemode->value != GAMEMODE_1V1)
			gi.bprintf (PRINT_HIGH, "%s %sjoined team '%s'\n", ent->client->pers.netname, reconnected ? "re" : "", teaminfo[ent->client->pers.team].name);
		else
			gi.bprintf (PRINT_HIGH, "%s %sjoined the game.\n", ent->client->pers.netname, reconnected ? "re" : "");
	}

	ent->client->resp.ready = false;

	//joining a team with no captain by default assigns.
	//FIXME: should this still assign even if the team has existing players?
	if (!teaminfo[ent->client->pers.team].captain)
		TDM_SetCaptain (ent->client->pers.team, ent);

	//nasty hack for setting team names for 1v1 mode
	TDM_UpdateTeamNames ();

	//if we were invited mid-game, reallocate and insert into teamplayers
	// wision: do not add if a player reconnected and used his joincode
	// skuller: force add if player was picked/invited to the different team
	if (tdm_match_status != MM_WARMUP && !reconnected && (!ent->client->resp.teamplayerinfo || ent->client->resp.teamplayerinfo->team != ent->client->pers.team))
		TDM_AddPlayerToMatchinfo (ent);

	//wision: set skin for new player
	gi.configstring (CS_PLAYERSKINS + (ent - g_edicts) - 1, va("%s\\%s", ent->client->pers.netname, teaminfo[ent->client->pers.team].skin));

	//set everyone elses teamskin for this player based on what team he is on
	TDM_SetAllTeamSkins (ent);

	//set this players teamskins based on his team
	TDM_SetTeamSkins (ent, NULL);

	if (g_gamemode->value != GAMEMODE_1V1)
		gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, va ("%s (%s)", ent->client->pers.netname, teaminfo[ent->client->pers.team].name));
	else
		gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, ent->client->pers.netname);

	TDM_TeamsChanged ();
	respawn (ent);
}

/*
==============
TDM_LeftTeam
==============
A player just left a team, so do things. Remember to call TeamsChanged afterwards!
*/
void TDM_LeftTeam (edict_t *ent, qboolean notify)
{
	int	oldteam;

	if (notify)
		gi.bprintf (PRINT_HIGH, "%s left team '%s'\n", ent->client->pers.netname, teaminfo[ent->client->pers.team].name);

	oldteam = ent->client->pers.team;

	//wision: remove player from the team!
	ent->client->pers.team = TEAM_SPEC;

	//assign a new captain
	if (teaminfo[oldteam].captain == ent)
		TDM_SetCaptain (oldteam, TDM_FindPlayerForTeam (oldteam));

	//resume play if this guy called time and game isn't already resuming?
	if (tdm_match_status == MM_TIMEOUT && level.tdm_timeout_caller && level.tdm_timeout_caller->client == ent && level.timeout_end_framenum)
		TDM_ResumeGame ();
}

qboolean CanJoin (edict_t *ent, unsigned team)
{
	if (tdm_match_status == MM_COUNTDOWN)
	{
		gi.cprintf (ent, PRINT_HIGH, "Teams are locked during countdown.\n");
		return false;
	}

	if (tdm_match_status != MM_WARMUP)
	{
		gi.cprintf (ent, PRINT_HIGH, "Match already in progress.\n");
		return false;
	}

	if (g_gamemode->value == GAMEMODE_1V1 && teaminfo[team].captain)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only one player is allowed on each team in 1v1 mode.\n");
		return false;
	}

	if (ent->client->pers.mvdclient)
	{
		gi.cprintf (ent, PRINT_HIGH, "MVD clients cannot join a team.\n");
		return false;
	}

	//wision: forbid rejoining the team
	if (ent->client->pers.team == team)
	{
		gi.cprintf (ent, PRINT_HIGH, "You are already on team '%s'.\n", teaminfo[ent->client->pers.team].name);
		return false;
	}

	//wision: forbid joining locked team
	if (teaminfo[team].locked)
	{
		//being invited to a team bypasses a lock
		if (!(ent->client->resp.last_invited_by && ent->client->resp.last_invited_by->inuse &&
			teaminfo[ent->client->resp.last_invited_by->client->pers.team].captain == ent->client->resp.last_invited_by))
		{
			gi.cprintf (ent, PRINT_HIGH, "Team '%s' is locked.\n", teaminfo[team].name);
			return false;
		}
	}

	//r1: cap at max allowed per-team (default 4)
	if (g_max_players_per_team->value && teaminfo[team].players >= g_max_players_per_team->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "Team '%s' is full.\n", teaminfo[team].name);
		return false;
	}

	return true;
}


/*
==============
JoinTeam1
==============
Player joined Team A via menu
*/
void JoinTeam1 (edict_t *ent)
{
	if (!CanJoin (ent, TEAM_A))
		return;

	if (ent->client->pers.team)
		TDM_LeftTeam (ent, true);

	ent->client->pers.team = TEAM_A;
	JoinedTeam (ent, false, true);
}
//merge those together?
/*
==============
JoinTeam2
==============
Player joined Team B via menu
*/
void JoinTeam2 (edict_t *ent)
{
	if (!CanJoin (ent, TEAM_B))
		return;

	if (ent->client->pers.team)
		TDM_LeftTeam (ent, true);

	ent->client->pers.team = TEAM_B;
	JoinedTeam (ent, false, true);
}

/*
==============
ToggleChaseCam
==============
Player hit Spectator menu option or used
chase command.
*/
void ToggleChaseCam (edict_t *ent)
{
	if (ent->client->pers.team)
	{
		TDM_LeftTeam (ent, true);
		TDM_TeamsChanged ();
		if (tdm_match_status == MM_TIMEOUT && teaminfo[TEAM_A].players == 0 && teaminfo[TEAM_B].players == 0)
			TDM_ResumeGame ();
		respawn (ent);
	}

	if (ent->client->chase_target)
		DisableChaseCam (ent);
	else
		GetChaseTarget(ent);

	PMenu_Close (ent);
}

/*
==============
SelectNextHelpPage
==============
Select the next page of help from the help menu. If help
menu is not open, opens on the first page of help.
*/
void SelectNextHelpPage (edict_t *ent)
{
/*	int		i;

	if (!ent->client->pers.menu.active)
	{
		PMenu_Open (ent, helpmenu[0], 0, 0, false);
		return;
	}

	for (i = 0; i < 3; i++)
	{
		if (ent->client->pers.menu.entries = helpmenu[i])
		{
			PMenu_Close (ent);
			PMenu_Open (ent, helpmenu[i+1], 0, 0, false);
			return;
		}
	}

	PMenu_Close (ent);
	*/
}

/*
==============
TDM_ShowTeamMenu
==============
Show/hide the join team menu
*/
void TDM_ShowTeamMenu (edict_t *ent)
{
	if (ent->client->pers.menu.active)
	{
		// if the votemenu is open, go back to normal menu
		// close the menu after this condition!
		if (ent->client->pers.votemenu_values.show)
		{
			PMenu_Close (ent);
			PMenu_Open (ent, ent->client->pers.joinmenu, teamJoinEntries[ent->client->pers.team], MENUSIZE_JOINMENU, false);
		}
		else
			PMenu_Close (ent);
	}
	else
		PMenu_Open (ent, ent->client->pers.joinmenu, teamJoinEntries[ent->client->pers.team], MENUSIZE_JOINMENU, false);
}

/*
==============
TDM_ProcessJoinCode
==============
Returns true if a player was set back on a team as a result of rejoining.
*/
qboolean TDM_ProcessJoinCode (edict_t *ent, unsigned value)
{
	int				i;
	const char		*code;
	teamplayer_t	*t;
	edict_t			*ghost;

	edict_t			new_entity;
	gclient_t		new_client;

	gclient_t		*saved_client;

	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return false;

	//if no value was passed, look up userinfo
	if (!value)
	{
		code = Info_ValueForKey (ent->client->pers.userinfo, "joincode");
		if (!code[0])
			return false;

		value = strtoul (code, NULL, 10);
	}

	//must be a disconnected client for this to work
	t = TDM_FindTeamplayerForJoinCode (value);
	if (!t || t->client || !t->saved_client || !t->saved_entity)
		return false;

	//0000201: Joincode rejoin. don't allow bypassing of max players setting.
	if (teaminfo[t->team].players >= current_matchinfo.max_players_per_team)
		return false;

	//could be using a joincode while chasing, need to fix it here before we possibly overwrite entity
	if (ent->client->chase_target)
		DisableChaseCam (ent);

	gi.unlinkentity (ent);

	ent->client->pers.team = t->team;
	JoinedTeam (ent, true, true);

	//we only preserve the whole client state in 1v1 mode, in TDM we simply respawn the player
	if (TDM_Is1V1())
	{
		//remove ghost model
		for (ghost = g_edicts + game.maxclients + 1; ghost < g_edicts + globals.num_edicts; ghost++)
		{
			if (!ghost->inuse)
				continue;

			if (ghost->enttype != ENT_GHOST)
				continue;

			if (ghost->count == t->saved_entity->s.number)
			{
				G_FreeEdict (ghost);
				break;
			}
		}

		//take copies of the current (new) structures
		new_entity = *ent;
		new_client = *ent->client;

		//preserve new client pointer
		saved_client = ent->client;

		//restore all edict fields and preserve new client pointer
		*ent = *t->saved_entity;
		ent->client = saved_client;

		//restore all client fields
		*ent->client = *t->saved_client;

		//restore stuff that shouldn't be overwritten
		ent->inuse = true;
		ent->s.number = new_entity.s.number;
		ent->client->resp.vote = VOTE_HOLD;
		VectorCopy (new_client.resp.cmd_angles, ent->client->resp.cmd_angles);

		strcpy (ent->client->pers.userinfo, new_client.pers.userinfo);

		//aiee :E :E
		ent->linkcount = new_entity.linkcount;
		ent->area = new_entity.area;
		ent->num_clusters = new_entity.num_clusters;
		memcpy (ent->clusternums, new_entity.clusternums, sizeof(ent->clusternums));
		ent->headnode = new_entity.headnode;
		ent->areanum = new_entity.areanum;
		ent->areanum2 = new_entity.areanum2;

		//set the delta angle
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
	}
	else
	{
		ent->client->resp.score = t->saved_client->resp.score;
		ent->client->resp.enterframe = t->saved_client->resp.enterframe;
		ent->client->resp.ready = t->saved_client->resp.ready;

		ent->client->pers.joinstate = t->saved_client->pers.joinstate;
		ent->client->pers.admin = t->saved_client->pers.admin;
	}

	//free the teamplayer saved info
	gi.TagFree (t->saved_entity);
	gi.TagFree (t->saved_client);

	t->saved_entity = NULL;
	t->saved_client = NULL;

	// wision: restore player's name
	G_StuffCmd (ent, "set name \"%s\"\n", t->name);

	//restore teamplayer links
	ent->client->resp.teamplayerinfo = t;
	t->client = ent;

	//no linkentity, we are called as part of PutClientInServer, linking here would telefrag ourselves!
	gi.unlinkentity (ent);

	return true;
}


/*
==============
TDM_Disconnected
==============
A player disconnected, do things.
*/
void TDM_Disconnected (edict_t *ent)
{
	qboolean	removeTimeout;

	removeTimeout = false;

	//have to check this right up here since we nuke the teamplayer just below!
	if (tdm_match_status == MM_TIMEOUT && level.tdm_timeout_caller && level.tdm_timeout_caller->client == ent)
		removeTimeout = true;

	//we remove this up here so TDM_LeftTeam doesn't try to resume if we become implicit timeout caller
	TDM_RemoveStatsLink (ent);

	if (ent->client->pers.team)
	{
		if (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD)
		{
			//do joincode stuff if a team player disconnects - save all their client info
			ent->client->resp.teamplayerinfo->saved_client = gi.TagMalloc (sizeof(gclient_t), TAG_GAME);
			*ent->client->resp.teamplayerinfo->saved_client = *ent->client;

			ent->client->resp.teamplayerinfo->saved_entity = gi.TagMalloc (sizeof(edict_t), TAG_GAME);
			*ent->client->resp.teamplayerinfo->saved_entity = *ent;

			if (TDM_Is1V1() && g_1v1_timeout->value > 0)
			{
				edict_t		*ghost;

				//may have already been set by a player or previous client disconnect
				if (tdm_match_status != MM_TIMEOUT)
				{	
					edict_t		*opponent;

					//timeout is called implicitly in 1v1 games or the other player would auto win
					level.timeout_end_framenum = level.realframenum + SECS_TO_FRAMES(g_1v1_timeout->value);
					level.last_tdm_match_status = tdm_match_status;
					tdm_match_status = MM_TIMEOUT;

					level.tdm_timeout_caller = ent->client->resp.teamplayerinfo;
					gi.bprintf (PRINT_CHAT, "%s disconnected and has %s to reconnect.\n", level.tdm_timeout_caller->name, TDM_SecsToString (g_1v1_timeout->value));

					//show the opponent their options
					opponent = TDM_FindPlayerForTeam (TEAM_A);
					if (opponent)
						gi.cprintf (opponent, PRINT_HIGH, "Your opponent has disconnected. You can allow them %s to reconnect, or you can force a forfeit by typing 'win' in the console.\n", TDM_SecsToString (g_1v1_timeout->value));

					opponent = TDM_FindPlayerForTeam (TEAM_B);
					if (opponent)
						gi.cprintf (opponent, PRINT_HIGH, "Your opponent has disconnected. You can allow them %s to reconnect, or you can force a forfeit by typing 'win' in the console.\n", TDM_SecsToString (g_1v1_timeout->value));
				}

				//show a "ghost" player where the player was
				ghost = G_Spawn ();
				VectorCopy (ent->s.origin, ghost->s.origin);
				VectorCopy (ent->s.angles, ghost->s.angles);
				ghost->s.effects = EF_SPHERETRANS;
				ghost->s.modelindex = 255;
				ghost->s.modelindex2 = 255;
				ghost->s.skinnum = ent - g_edicts - 1;
				ghost->s.frame = ent->s.frame;
				ghost->count = ent->s.number;
				ghost->classname = "ghost";
				ghost->target_ent = ent;
				ghost->enttype = ENT_GHOST;
				gi.linkentity (ghost);
			}
		}

		if (removeTimeout)
			TDM_ResumeGame ();

		TDM_LeftTeam (ent, false);
	}

	TDM_TeamsChanged ();

	if (tdm_match_status == MM_WARMUP && teaminfo[TEAM_SPEC].players == 0 && teaminfo[TEAM_A].players == 0 && teaminfo[TEAM_B].players == 0)
	{
		if (vote.active)
			TDM_RemoveVote ();

		//reset the map if it's been running for over 7 days to workaround time precision bugs in the engine, fixes 0000208
		if (time (NULL) - level.spawntime > (86400 * 7))
		{
			char	command[256];
			Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.mapname);
			gi.AddCommandString (command);
		}
	}
	else
		TDM_CheckVote ();

	//zero for connecting clients on server browsers
	ent->client->ps.stats[STAT_FRAGS] = 0;
}


/*
==============
TDM_CreatePlayerDmStatusBar
==============
Create player's own customized dm_statusbar.
*/
const char *TDM_CreatePlayerDmStatusBar (playerconfig_t *c)
{
	static char	*dm_statusbar;
	int			id_x, id_y, id_highlight;

	// opentdm default
	id_highlight = 0;
	id_x = -100;
	id_y = -80;

	id_x += c->id_x;
	id_y += c->id_y;
	id_highlight = c->id_highlight;

	dm_statusbar = va (
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer (quad, rebreather, envirosuit)
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

// timer (pent)
"if 29 "
"   yb  -80 "
"	xv	246 "
"	num	2	30 "
"	xv	296 "
"	pic	29 "
"endif "

// First team name
"xr -250 "
"yb -96 "
"stat_string 18 "

// First team score / status
"xr -66 "
"yb -120 "
"num 4 23 "

// Second team name
"xr -250 "
"yb -48 "
"stat_string 19 "

// Second team score / status
"xr -66 "
"yb -72 "
"num 4 24 "

// Time
"xv 175 "
"yb -48 "
"stat_string 26 "

// Time value
"xv 175 "
"yb -39 "
"stat_string 22 "

// Timeout message
"if 25 "
	"xr -58 "
	"yt 50 "
	"string \"Timeout\" "

	// Timeout value
	"xr -42 "
	"yt 58 "
	"stat_string 25 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 31 "

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

// player id view
"if 27 "
  "xv %d "
  "yb %d "
  "stat_string 27 "
"endif "

// vote notice
"if 28 "
  "xl 10 "
  "yb -180 "
  "stat_string 28 "
"endif ", id_x, id_y);

	return dm_statusbar;
}

/*
==============
TDM_SendStatusBarCS
==============
Send status bar config string.
*/
void TDM_SendStatusBarCS (edict_t *ent)
{
	gi.WriteByte (svc_configstring);
	gi.WriteShort (CS_STATUSBAR);
	gi.WriteString (TDM_CreatePlayerDmStatusBar (&ent->client->pers.config));
	gi.unicast (ent, true);
}

qboolean TDM_ParsePlayerConfigLine (char *line, int line_number, void *param)
{
	playerconfig_t	*c;
	char			*p, *variable;

	c = (playerconfig_t *)param;

	p = strchr (line, '\t');
	if (!p)
	{
		gi.dprintf ("WARNING: Malformed line %d '%s'\n", line_number, line);
		return false;
	}

	p[0] = 0;
	p++;

	variable = line;

	//no validation is done here to keep things small - these should be validated serverside
	//already. yes, we're trusting the server not to lie to us :).
	if (!strcmp (variable, "teamskin"))
		Q_strncpy (c->teamskin, p, sizeof(c->teamskin)-1);
	else if (!strcmp (variable, "enemyskin"))
		Q_strncpy (c->enemyskin, p, sizeof(c->enemyskin)-1);
	else if (!strcmp (variable, "autorecord"))
		c->auto_record = atoi (p);
	else if (!strcmp (variable, "autoscreenshot"))
		c->auto_screenshot = atoi (p);
	else if (!strcmp (variable, "id_highlight"))
		c->id_highlight = atoi (p);
	else if (!strcmp (variable, "id_x"))
		c->id_x = atoi (p);
	else if (!strcmp (variable, "id_y"))
		c->id_y = atoi (p);
	else
		gi.dprintf ("Unknown player config variable '%s'. Check you are using the latest version of OpenTDM.\n", variable);

	return true;
}

void TDM_PlayerConfigDownloaded (tdm_download_t *download, int code, byte *buff, int len)
{
	playerconfig_t	config;

	if (!download->initiator)
		return;

	if (buff)
	{
		if (!TDM_ProcessText ((char *)buff, len, TDM_ParsePlayerConfigLine, &config))
		{
			gi.dprintf ("TDM_PlayerConfigDownloaded: Parse failed.\n");
		}
		else
		{
			config.loaded = true;
			download->initiator->client->pers.config = config;
			gi.cprintf (download->initiator, PRINT_HIGH, "Your opentdm.net player config was loaded successfully.\n");
			TDM_SetTeamSkins (download->initiator, NULL);
		}
	}
	else
	{
		if (code == 404)
			gi.cprintf (download->initiator, PRINT_HIGH, "No player config for your stats_id was found. Please check your stats_id cvar is set properly.\n");
		else if (code == 400)
			gi.cprintf (download->initiator, PRINT_HIGH, "A malformed stats_id was found while trying to download your player config. Please check your stats_id cvar is set properly.\n");
		else
			gi.cprintf (download->initiator, PRINT_HIGH, "An unspecified error %d occured while trying to download your player config.\n", code);
	}

	//wision: set up the dm_statusbar according the config and send it to the client
	TDM_SendStatusBarCS (download->initiator);
}

/*
==============
TDM_DownloadPlayerConfig
==============
Queue up a player config download if possible.
*/
void TDM_DownloadPlayerConfig (edict_t *ent)
{
	const char	*stats_id;

	if (ent->client->pers.config.loaded)
		return;

	stats_id = Info_ValueForKey (ent->client->pers.userinfo, "stats_id");
	if (!stats_id[0])
	{
		//FIXME: is this necessary with the global CS back in place?
		TDM_SendStatusBarCS (ent);
		return;
	}

	//prevent multiple calls from overwriting old request
	if (ent->client->pers.download.inuse)
	{
		//FIXME: shouldn't get here.
		return;
	}

	Com_sprintf (ent->client->pers.download.path , sizeof(ent->client->pers.download.path ), "playerconfigs/%s", stats_id);
	ent->client->pers.download.initiator = ent;
	ent->client->pers.download.type = DL_PLAYER_CONFIG;
	strncpy (ent->client->pers.download.name, stats_id, sizeof(ent->client->pers.download.name)-1);
	ent->client->pers.download.onFinish = TDM_PlayerConfigDownloaded;
	ent->client->pers.download.inuse = true;
	ent->client->pers.download.unique_id = ent->client->pers.uniqueid;

	HTTP_QueueDownload (&ent->client->pers.download);
}

/*
==============
TDM_SetupClient
==============
Setup the client after an initial connection. Called on first spawn
only, not every map. Returns true if a join code was used to respawn
a full player, so that PutClientInServer knows about it.
*/
qboolean TDM_SetupClient (edict_t *ent)
{
	ent->client->pers.team = TEAM_SPEC;
	TDM_TeamsChanged ();

	//handled in userinfo updates now
	//TDM_DownloadPlayerConfig (ent);

	if (!TDM_ProcessJoinCode (ent, 0))
	{
		if (tdm_match_status == MM_TIMEOUT)
		{
			gi.cprintf (ent, PRINT_CHAT, "\nMatch is currently paused and will auto resume in %s.\n", TDM_SecsToString(FRAMES_TO_SECS(level.timeout_end_framenum - level.realframenum)));
		}
		else
		{
			TDM_ShowTeamMenu (ent);
			gi.cprintf (ent, PRINT_CHAT, "\nWelcome to OpenTDM, an open source OSP/Battle replacement. Please report any bugs at www.opentdm.net. Type 'commands' in the console for a brief command guide.\n\n");
		}
	}
	else
	{
		if (tdm_match_status == MM_TIMEOUT)
		{
			if (TDM_Is1V1())
			{
				//only resume if we have 2 players - it's possible both players dropped, and that is a situation
				//we need to be able to handle.
				if (teaminfo[TEAM_A].players == 1 && teaminfo[TEAM_B].players == 1)
					TDM_ResumeGame ();
				return true;
			}
		}
	}

	return false;
}

/*
==============
TDM_FindPlayerForTeam
==============
Returns first matching player of team. Used in situations where
team is without captain so we can't use teaminfo.captain.
*/
edict_t *TDM_FindPlayerForTeam (unsigned team)
{
	edict_t	*ent;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (ent->client->pers.team == team)
			return ent;
	}

	return NULL;
}


/*
==============
TDM_PlayerNameChanged
==============
Someone changed their name, update configstrings for spectators and such.
*/
void TDM_PlayerNameChanged (edict_t *ent)
{
	if (ent->client->pers.team)
	{
		if (TDM_Is1V1())
			gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, ent->client->pers.netname);
		else
			gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, va("%s (%s)", ent->client->pers.netname, teaminfo[ent->client->pers.team].name));

		TDM_SetAllTeamSkins (ent);
		TDM_UpdateTeamNames ();
	}
}

/*
==============
TDM_SetInitialItems
==============
Give a client an initial weapon/item loadout depending on match mode
*/
void TDM_SetInitialItems (edict_t *ent)
{
	gclient_t		*client;
	const gitem_t	*item;
	int				i;

	client = ent->client;


	client->max_bullets		= 200;
	client->max_shells		= 100;
	client->max_rockets		= 50;
	client->max_grenades	= 50;
	client->max_cells		= 200;
	client->max_slugs		= 50;

	switch (tdm_match_status)
	{
		case MM_WARMUP:
			//wision: spawn with rail in insta
			if (g_gamemode->value == GAMEMODE_ITDM)
			{
				item = GETITEM (ITEM_WEAPON_RAILGUN);
				Add_Ammo (ent, GETITEM(item->ammoindex), 1000);
				client->weapon = item;
				client->selected_item = ITEM_INDEX(item);
				client->inventory[client->selected_item] = 1;
			}
			else
			{
				for (i = 1; i < game.num_items; i++)
				{
					item = GETITEM (i);
					//wision: BFG sucks in warmup :X
					if ((item->flags & IT_WEAPON) && i != ITEM_WEAPON_BFG)
					{
						client->inventory[i] = 1;
						if (item->ammoindex)
							Add_Ammo (ent, GETITEM(item->ammoindex), 1000);
					}
				}

				//spawn with RL up
				if (!client->pers.last_weapon || client->inventory[ITEM_INDEX (client->pers.last_weapon)] == 0)
				{
					client->selected_item = ITEM_WEAPON_ROCKETLAUNCHER;
					client->weapon = GETITEM (ITEM_WEAPON_ROCKETLAUNCHER);
				}
				else
				{
					client->weapon = client->pers.last_weapon;
					client->selected_item = ITEM_INDEX (client->pers.last_weapon);
				}
			}
			client->inventory[ITEM_ITEM_ARMOR_BODY] = 100;
			break;

		default:
			//wision: spawn with rail in insta
			if (g_gamemode->value == GAMEMODE_ITDM)
			{
				item = GETITEM (ITEM_WEAPON_RAILGUN);
				Add_Ammo (ent, GETITEM(item->ammoindex), 1000);
			}
			else
				item = GETITEM (ITEM_WEAPON_BLASTER);
			client->weapon = item;
			client->selected_item = ITEM_INDEX(item);
			client->inventory[client->selected_item] = 1;
			break;
	}
}

/*
==============
TDM_GetArmorValue
==============
Returns how much armor (any type) the client has.
*/
int TDM_GetArmorValue (edict_t *ent)
{
	int		index;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	return ent->client->inventory[index];
}

/*
==============
TDM_GetPlayerIdView
==============
Find the best player for the id view and return a configstring index that contains their info.
*/
int TDM_GetPlayerIdView (edict_t *ent)
{
	edict_t		*ignore;
	edict_t		*target;
	vec3_t		forward;
	trace_t		tr;
	vec3_t		start;
	vec3_t		mins = {-4,-4,-4};
	vec3_t		maxs = {4,4,4};
	qboolean	ignoreConfigStringUpdate;
	int			i;
	int			powerarmor;
	qboolean	show_health_info;

	ignoreConfigStringUpdate = false;

	if (ent->client->chase_target)
	{
		target = ent->client->chase_target->client->resp.last_id_client;
	}
	else
	{
		int	tracemask;

		VectorCopy (ent->s.origin, start);
		start[2] += ent->viewheight;

		AngleVectors (ent->client->v_angle, forward, NULL, NULL);

		VectorScale (forward, 4096, forward);
		VectorAdd (ent->s.origin, forward, forward);

		ignore = ent;

		target = NULL;

		tracemask = CONTENTS_SOLID|CONTENTS_MONSTER|MASK_WATER;

		//find best player through tracing
		for (i = 0; i < 10; i++)
		{
			tr = gi.trace (start, mins, maxs, forward, ignore, tracemask);

			//hit transparent water
			if (tr.ent == world && tr.surface && (tr.surface->flags & (SURF_TRANS33|SURF_TRANS66)))
			{
				tracemask &= ~MASK_WATER;
				VectorCopy (tr.endpos, start);
				continue;
			}

			if (tr.ent == world || tr.fraction == 1.0f)
				break;

			//we hit something that's a player and it's alive and on our team!
			//note, we trace twice so we hit water planes
			if (tr.ent && tr.ent->client && tr.ent->health > 0 && visible (tr.ent, ent, CONTENTS_SOLID | MASK_WATER))
			{
				target = tr.ent;
				break;
			}
			else
			{
				VectorCopy (tr.endpos, start);
				ignore = tr.ent;
			}
		}

		//if trace was unsuccessful, try guessing based on angles
		if (!target)
		{
			edict_t		*who, *best;
			vec3_t		dir;
			float		distance, bdistance = 0.0f;
			float		bd = 0.0f, d;

			AngleVectors (ent->client->v_angle, forward, NULL, NULL);
			best = NULL;

			for (who = g_edicts + 1; who <= g_edicts + game.maxclients; who++)
			{
				if (!who->inuse)
					continue;

				if (who->health <= 0)
					continue;

				if (who == ent)
					continue;

				VectorSubtract (who->s.origin, ent->s.origin, dir);
				distance = VectorLength (dir);

				VectorNormalize (dir);
				d = DotProduct (forward, dir);

				//note, we trace twice so we hit water planes
				if (d > bd && visible (ent, who, CONTENTS_SOLID | MASK_WATER) && visible (who, ent, CONTENTS_SOLID | MASK_WATER))
				{
					bdistance = distance;
					bd = d;
					best = who;
				}
			}

			if (best)
			{
				if (!best->client->pers.team)
					TDM_Error ("TDM_GetPlayerIdView: Got a spectator via DotProduct with %d health (%s)", best->health, best->client->pers.netname);
				//allow variable slop based on proximity
				if ((bdistance < 150 && bd > 0.50f) || (bdistance < 250 && bd > 0.90f) || (bdistance < 600 && bd > 0.96f) || bd > 0.98f)
					target = best;
			}
		}
		else if (!target->client->pers.team)
			TDM_Error ("TDM_GetPlayerIdView: Got a spectator via trace with %d health (%s)", target->health, target->client->pers.netname);
	}

	if (!target)
		return 0;

	show_health_info = false;

	if (ent->client->pers.team == target->client->pers.team)
	{
		//same team, show health info
		show_health_info = true;
	}
	else if (ent->client->pers.team == TEAM_SPEC)
	{
		/*if (!ent->client->chase_target)
		{
			//free floating camera, DON'T show health info
			show_health_info = false;
		}
		else */
		if (ent->client->chase_target && ent->client->chase_target->client->pers.team == target->client->pers.team)
		{
			//viewing someone on the same team as our chase target, show health info
			show_health_info = true;
		}
	}

	//check for power armor
	if (target->client->inventory[ITEM_ITEM_POWER_SCREEN] > 0 || target->client->inventory[ITEM_ITEM_POWER_SHIELD] > 0)
		powerarmor = target->client->inventory[ITEM_AMMO_CELLS];
	else
		powerarmor = -1;

	//don't spam configstring if they haven't changed since last time
	if (ent->client->resp.last_id_client == target &&
		ent->client->resp.last_id_health == target->health &&
		ent->client->resp.last_id_armor == TDM_GetArmorValue (target) &&
		ent->client->resp.last_id_powerarmor == powerarmor)
		ignoreConfigStringUpdate = true;

	ent->client->resp.last_id_client = target;
	ent->client->resp.last_id_health = target->health;
	ent->client->resp.last_id_armor = TDM_GetArmorValue (target);
	ent->client->resp.last_id_powerarmor = powerarmor;

	if (!ignoreConfigStringUpdate)
	{
		char	*string;

		if (show_health_info)
		{
			char	buff[16];

			//show power armor if they have it
			if (powerarmor != -1)
				sprintf (buff, " P:%d", powerarmor);
			else
				buff[0] = '\0';

			if (ent->client->pers.config.id_highlight)
				string = TDM_SetColorText (va ("%16s H:%d A:%d%s",
						target->client->pers.netname,
						target->health,
						TDM_GetArmorValue (target),
						buff));
			else
				string = va ("%16s H:%d A:%d%s",
						target->client->pers.netname,
						target->health,
						TDM_GetArmorValue (target),
						buff);
		}
		else
		{
			if (ent->client->pers.config.id_highlight)
				string = TDM_SetColorText (va ("%16s", target->client->pers.netname));
			else
				string = va ("%16s", target->client->pers.netname);
		}

		gi.WriteByte (svc_configstring);
		gi.WriteShort (CS_TDM_ID_VIEW);
		gi.WriteString (string);
		gi.unicast (ent, false);
	}

	return target - g_edicts;
}
