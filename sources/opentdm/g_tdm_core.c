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

//OpenTDM core. The functions here are mainly 'behind the scenes' things - match
//setup, timer checks, managing state, configstrings, etc...

#include "g_local.h"
#include "g_tdm.h"
#include "g_svcmds.h"

//dynamic FRAMETIME, oh my.
float	FRAMETIME;
int		SERVER_FPS;

teaminfo_t	teaminfo[MAX_TEAMS];
matchmode_t	tdm_match_status;

int soundcache[MAX_SOUNDS];

typedef struct
{
	const char	*variable_name;
	char		*default_string;
} cvarsave_t;

//anything in this list is modified by voting
//so the default values are saved on startup
//and reset when everyone leaves the server.
cvarsave_t preserved_vars[] = 
{
	{"g_team_a_name", NULL},
	{"g_team_b_name", NULL},
	{"g_team_a_skin", NULL},
	{"g_team_b_skin", NULL},
	{"g_match_time", NULL},
	{"g_itemflags", NULL},
	{"g_powerupflags", NULL},
	{"g_gamemode", NULL},
	{"g_tie_mode", NULL},
	{"g_teleporter_nofreeze", NULL},
	{"g_fast_weap_switch", NULL},
	{"g_overtime", NULL},
	{"g_bugs", NULL},
	{"g_chat_mode", NULL},
	{"g_tdm_spawn_mode", NULL},
	{"g_1v1_spawn_mode", NULL},
	{NULL, NULL},
};

const char *soundnames[17] = {
	"death1.wav",
	"death2.wav",
	"death3.wav",
	"death4.wav",
	"fall1.wav",
	"fall2.wav",
	"gurp1.wav",
	"gurp2.wav",
	"jump1.wav",
	"pain25_1.wav",
	"pain25_2.wav",
	"pain50_1.wav",
	"pain50_2.wav",
	"pain75_1.wav",
	"pain75_2.wav",
	"pain100_1.wav",
	"pain100_2.wav",
};

char		**tdm_maplist;

//a note regarding matchinfo and teamplayers.
//matchinfo is a structure containing information about a single match - from "Fight!" to a team winning.
//it is not used for warmup or anything else like that, only initialized when the match actually begins.
//current_matchinfo describes the current match - game mode, timelimit, map, etc. in the matchinfo_t
//structure is a link to a teamplayer_t * - this is a dynamically allocated array that contains info about
//every player in the match for both teams. teamplayer_t also contains a link back to the matchinfo_t for
//time saving purposes.

//at the end of a match, old_matchinfo becomes current_matchinfo and current_matchinfo teamplayers is NULLed
//to prevent certain things from happening. a few extra pieces of glue are needed here - since matchinfo
//structures are static, pointers in the teamplayer_t need updating to point to the old_matchinfo instead
//of current_matchinfo. if old_matchinfo.teamplayers already exists, it is freed and data about that match
//is gone.

matchinfo_t	current_matchinfo;
matchinfo_t	old_matchinfo;

static char teamStatus[MAX_TEAMS][MAX_TEAMNAME_LENGTH];

//static char teamJoinText[MAX_TEAMS][32];

//copy of last game scoreboard for oldscores command
char		old_scoreboard_string[1400];

//static char	last_player_model[MAX_TEAMS][32];

static const pmenu_t joinmenu[] =
{
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Spectate",			PMENU_ALIGN_LEFT, NULL, ToggleChaseCam },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Voting Menu",		PMENU_ALIGN_LEFT, NULL, OpenVoteMenu },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "ENTER select, ESC exit",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*" OPENTDM_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

/*const pmenu_t helpmenu[][] =
{
	{ "*Quake II - OpenTDM",PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Console Commands",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*menu",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Open the OpenTDM menu",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*team <name>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join the team <name>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote <vote command>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Start a vote to change the",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "server settings (see p.2)",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*ready",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle your ready status",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*forceready",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Force all your team ready",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*timeout",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Request a time out",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*(Continued...)",	PMENU_ALIGN_RIGHT, NULL, SelectNextHelpPage },
},
{
	{ "*Quake II - OpenTDM",PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Voting Commands",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote kick <player>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Kick <player>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote map <map>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Change to <map>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote timelimit <x>",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Set timelimit to x",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote overtime <mode>",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Set overtime mode, one of",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "sd or extend",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote weapons",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Show weapon vote menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*vote powerups",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Show powerups vote menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*(Continued...)",	PMENU_ALIGN_RIGHT, NULL, SelectNextHelpPage },
},
{
	{0}
};*/

/*
==============
TDM_SetColorText
==============
Converts a string to color text (high ASCII)
*/
char *TDM_SetColorText (char *buffer)
{
	size_t	len;
	int		i;

	len = strlen (buffer);
	for (i = 0; i < len; i++)
	{
		if (buffer[i] != '\n')
			buffer[i] |= 0x80;
	}

	return buffer;
}

void TDM_SetFrameTime (void)
{
	cvar_t	*sv_fps;

	sv_fps = gi.cvar ("sv_fps", NULL, 0);
	if (!sv_fps)
	{
		FRAMETIME = 0.1f;
		SERVER_FPS = 10;
	}
	else
	{
		FRAMETIME = 1.0f / sv_fps->value;
		SERVER_FPS = (int)sv_fps->value;
	}

	if ((int)(0.1f / FRAMETIME) == 0)
		gi.error ("Invalid server FPS");
}

/*
==============
TDM_SaveDefaultCvars
==============
Save whatever the server admin set so we can restore later.
*/
void TDM_SaveDefaultCvars (void)
{
	cvarsave_t	*preserved;
	cvar_t		*var;

	preserved = preserved_vars;

	while (preserved->variable_name)
	{
		var = gi.cvar (preserved->variable_name, NULL, 0);
		if (!var)
			TDM_Error ("TDM_SaveDefaultCvars: Couldn't preserve %s", preserved->variable_name);

		//note, G_CopyString resets on map change - have to do this one ourselves
		preserved->default_string = gi.TagMalloc (strlen(var->string)+1, TAG_GAME);
		strcpy (preserved->default_string, var->string);

		preserved++;
	}
}

/*
==============
TDM_ResetLevel
==============
Resets the items and triggers / funcs / etc in the level
in preparation for a match.
*/
void TDM_ResetLevel (void)
{
	int i;
	edict_t	*ent;

	//map hasn't loaded yet!
	if (!level.entity_string)
		return;

	//free up any stray ents
	for (ent = g_edicts + 1 + game.maxclients; ent < g_edicts + globals.num_edicts; ent++)
	{
		if (!ent->inuse)
			continue;

		//handle body que specially, just remove effects and unlink it
		if (ent->enttype == ENT_BODYQUE)
		{
			//it could still be an ungibbed body, make sure nothing can happen to it
			ent->takedamage = DAMAGE_NO;
			ent->solid = SOLID_NOT;
			ent->s.modelindex = ent->s.effects = ent->s.sound = ent->s.renderfx = 0;
			gi.unlinkentity (ent);
			continue;
		}

		if (
			(ent->enttype == ENT_DOOR_TRIGGER || ent->enttype == ENT_PLAT_TRIGGER || ent->enttype == ENT_GIB)
			||
			(ent->owner >= (g_edicts + 1) && ent->owner <= (g_edicts + game.maxclients))
			)
			G_FreeEdict (ent);
	}

	///rerun the level entity string
	ParseEntityString (true);

	//immediately droptofloor and setup removed items
	for (ent = g_edicts + 1 + game.maxclients; ent < g_edicts + globals.num_edicts; ent++)
	{
		if (!ent->inuse)
			continue;

		//wision: add/remove items
		if (ent->item)
		{
			// some items will be prevented in deathmatch
			if ( (int)dmflags->value & DF_NO_ARMOR )
			{
				if (ITEM_INDEX(ent->item) == ITEM_ITEM_ARMOR_BODY || ITEM_INDEX(ent->item) == ITEM_ITEM_ARMOR_COMBAT || ITEM_INDEX(ent->item) == ITEM_ITEM_ARMOR_JACKET ||
					ITEM_INDEX(ent->item) == ITEM_ITEM_ARMOR_SHARD || ITEM_INDEX(ent->item) == ITEM_ITEM_POWER_SCREEN || ITEM_INDEX(ent->item) == ITEM_ITEM_POWER_SHIELD)
				{
					G_FreeEdict (ent);
					continue;
				}
			}

			if ( (int)dmflags->value & DF_NO_ITEMS )
			{
				if (ITEM_INDEX(ent->item) == ITEM_ITEM_QUAD || ITEM_INDEX(ent->item) == ITEM_ITEM_INVULNERABILITY || ITEM_INDEX(ent->item) == ITEM_ITEM_SILENCER ||
					ITEM_INDEX(ent->item) == ITEM_ITEM_BREATHER || ITEM_INDEX(ent->item) == ITEM_ITEM_ENVIRO || ITEM_INDEX(ent->item) == ITEM_ITEM_ADRENALINE ||
					ITEM_INDEX(ent->item) == ITEM_ITEM_BANDOLIER || ITEM_INDEX(ent->item) == ITEM_ITEM_PACK)
				{
					G_FreeEdict (ent);
					continue;
				}
			}

			if ( (int)dmflags->value & DF_NO_HEALTH )
			{
				if (ITEM_INDEX(ent->item) == ITEM_ITEM_HEALTH || ITEM_INDEX(ent->item) == ITEM_ITEM_ANCIENT_HEAD)
				{
					G_FreeEdict (ent);
					continue;
				}
			}

			if ( (int)dmflags->value & DF_INFINITE_AMMO )
			{
				if (ent->item->flags & (IT_AMMO|IT_WEAPON))
				{
					G_FreeEdict (ent);
					continue;
				}
			}

			for (i = 0; i < sizeof(weaponvotes) / sizeof(weaponinfo_t); i++)
			{
				//this item isn't removed
				if (!((int)g_itemflags->value & weaponvotes[i].value))
					continue;

				//this is a weapon that should be removed
				if (ITEM_INDEX (ent->item) == weaponvotes[i].itemindex)
				{
					G_FreeEdict (ent);
					break;
				}

				//this is ammo for a weapon that should be removed
				if (ITEM_INDEX (ent->item) == GETITEM (weaponvotes[i].itemindex)->ammoindex)
				{
					//special case: cells, grenades, shells
					if ((int)g_itemflags->value & GETITEM(GETITEM(weaponvotes[i].itemindex)->ammoindex)->tag)
					{
						G_FreeEdict (ent);
						break;
					}
				}
			}

			//was removed
			if (!ent->inuse)
				continue;

			//wision: add/remove powerups
			for (i = 0; i < sizeof(powerupvotes) / sizeof(powerupinfo_t); i++)
			{
				//this powerup isn't removed
				if (!((int)g_powerupflags->value & powerupvotes[i].value))
					continue;

				if (ITEM_INDEX (ent->item) == powerupvotes[i].itemindex)
				{
					G_FreeEdict (ent);
					break;
				}
			}

			//was removed
			if (!ent->inuse)
				continue;

			//track how many items have spawned for stats
			TDM_ItemSpawned (ent);
		}

		if (ent->think == droptofloor)
		{
			ent->nextthink = 0;
			droptofloor (ent);
		}
	}
}

/*
==============
TDM_BeginMatch
==============
A match has just started (end of countdown)
*/
void TDM_BeginMatch (void)
{
	edict_t		*ent;

	//level.match_start_framenum = 0;
	level.match_end_framenum = level.framenum + (int)(g_match_time->value * SERVER_FPS);
	tdm_match_status = MM_PLAYING;

	//must setup teamplayers before level, or we lose item spawn stats
	TDM_SetupMatchInfoAndTeamPlayers ();

	//respawn the map
	TDM_ResetLevel ();

	gi.bprintf (PRINT_HIGH, "Fight!\n");

	//should already be 0, check this is needed
	teaminfo[TEAM_A].score = teaminfo[TEAM_B].score = 0;

	//psuedo-kill everyone first to prevent the spawnspot selection from being able to be controlled
	//by positioning in warmup and also possible telefrag conditions
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		//reset invites on players who were invite during warmup, to prevent them joining mid-game
		ent->client->resp.last_invited_by = NULL;

		if (ent->client->pers.team)
		{
			//fake kill them first so the spawnpoint code doesn't consider them standing on a spot
			//if they were on one during warmup
			ent->health = 0;
			gi.unlinkentity (ent);
		}
	}

	//put everyone in the server and go!
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (ent->client->pers.team)
		{
			respawn (ent);
		}
	}

	TDM_UpdateConfigStrings (false);
}

/*
==============
TDM_ScoreBoardString
==============
Display TDM scoreboard. Must be unicast or multicast after calling this
function.
*/
char *TDM_ScoreBoardString (edict_t *ent)
{
	char		entry[1024];
	char		tmpstr[60];
	static char	string[1024];
	int			len;
	int			i, j, k;
	int			sorted[2][MAX_CLIENTS];
	int			sortedscores[2][MAX_CLIENTS];
	int			score, total[2], totalscore[2];
	float		averageping[2];
	int			last[2];
	int			width[2];
	int			maxplayers;
	int			offset;
	static int	firstteam = TEAM_A;
	static int	secondteam = TEAM_B;
	cvar_t		*hostname;
	char		serverinfo[51];
	struct tm	*ts;
	time_t		t;
	qboolean	drawn_header;

	gclient_t		*cl;
	teamplayer_t	*tmpl;
	edict_t			*cl_ent;
	int				team;
	const int		maxsize = 1000;

	t = time (NULL);
	ts = localtime (&t);

	hostname = gi.cvar ("hostname", NULL, 0);

	serverinfo[sizeof(serverinfo)-1] = '\0';

	if (hostname)
		strncpy (serverinfo, hostname->string, sizeof(serverinfo)-1);
	else
		strcpy (serverinfo, "unnamed server");

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	averageping[0] = averageping[1] = 0.0;

	//init string
	*string = 0;
	len = 0;

	// team info bars
	sprintf (string,
		"xv 72 yv 0 string2 \" Team          Frags\" "
		"yv 8 string \"%-15.15s %4d\" "
		"yv 16 string \"%-15.15s %4d\" ",
		teaminfo[firstteam].name,
		teaminfo[firstteam].score,
		teaminfo[secondteam].name,
		teaminfo[secondteam].score
		);

	// time info
	sprintf (string + strlen(string),
		"yv 32 string \" [%d-%02d-%02d %02d:%02d]\" ",
		ts->tm_year + 1900,
		ts->tm_mon + 1,
		ts->tm_mday,
		ts->tm_hour,
		ts->tm_min
		);


	// wision: match scoreboard
	if (current_matchinfo.teamplayers)
	{
		int		ping;

		for (i = 0; i < current_matchinfo.num_teamplayers; i++)
		{
			// skip players which are not supposed to be drawn during the match (disconnected, moved to observer)
			if ((current_matchinfo.teamplayers[i].client == NULL || current_matchinfo.teamplayers[i].client->client->pers.team == TEAM_SPEC) &&
					tdm_match_status != MM_SCOREBOARD)
				continue;

			if (current_matchinfo.teamplayers[i].team == TEAM_A)
				team = 0;
			else if (current_matchinfo.teamplayers[i].team == TEAM_B)
				team = 1;
			else
				continue; // unknown team?

			score = current_matchinfo.teamplayers[i].enemy_kills
						- current_matchinfo.teamplayers[i].team_kills
						- current_matchinfo.teamplayers[i].suicides;

			for (j = 0; j < total[team]; j++)
			{
				if (score > sortedscores[team][j])
					break;
			}

			for (k = total[team]; k > j; k--)
			{
				sorted[team][k] = sorted[team][k-1];
				sortedscores[team][k] = sortedscores[team][k-1];
			}

			sorted[team][j] = i;
			sortedscores[team][j] = score;

			totalscore[team] += score;
			total[team]++;

			if (current_matchinfo.teamplayers[i].client)
				ping = current_matchinfo.teamplayers[i].client->client->ping;
			else
				ping = current_matchinfo.teamplayers[i].ping;

			if (ping > 999)
				averageping[team] += 999;
			else
				averageping[team] += (float)ping;
		}

		if (total[0] > 0)
			averageping[0] = averageping[0] / (float)total[0];

		if (total[1] > 0)
			averageping[1] = averageping[1] / (float)total[1];

		// wision: set which team should be on the top
		if (teaminfo[TEAM_A].score < teaminfo[TEAM_B].score)
		{
			firstteam = TEAM_B;
			secondteam = TEAM_A;
		}
		else if (teaminfo[TEAM_A].score > teaminfo[TEAM_B].score)
		{
			firstteam = TEAM_A;
			secondteam = TEAM_B;
		}

		maxplayers = total[firstteam-1];

		for (i = 0; i < MAX_TEAMS - 1; i++)
		{
			//determine how wide to draw team score
			len = abs(teaminfo[i+1].score);

			if (len > 999)
				width[i] = 4;
			else if (len > 99)
				width[i] = 3;
			else if (len > 9)
				width[i] = 2;
			else
				width[i] = 1;

			//for negative sign
			if (teaminfo[i+1].score < 0)
				width[i]++;
		}

		//figure out how far the other team needs to be drawn below
		offset = maxplayers * 8 + 24;

		// print more info into old scoreboard
		// ent is NULL only when we request the scoreboard for preserving
		if (!ent)
		{
			sprintf (tmpstr, "oldscoreboard: %s", level.mapname);
			sprintf (string + strlen(string), "xv %d yv 24 string2 \"%s\" ",
					(int)((36-strlen(tmpstr))/2)*8 + 8,
					tmpstr
					);
		}

		// headers
		sprintf (tmpstr, "%s:%.f(%s)", teaminfo[firstteam].name, averageping[firstteam-1], teaminfo[firstteam].skin);
		sprintf (string + strlen(string),
			"xv %d yv 40 string \"%s\" ",
			(int)((36-strlen(tmpstr))/2)*8 + 8,
			tmpstr
 			);
 
		sprintf (tmpstr, "%s:%.f(%s)", teaminfo[secondteam].name, averageping[secondteam-1], teaminfo[secondteam].skin);
 		sprintf (string + strlen(string),
			"xv %d yv %d string \"%s\" ",
			(int)((36-strlen(tmpstr))/2)*8 + 8,
			offset + 40, tmpstr
			);

		sprintf (string + strlen(string),
			"xv 8 yv 48 string2 \" Name            Frags Dths Net Ping\" "
 			);
 
 		sprintf (string + strlen(string),
			"yv %d string2 \" Name            Frags Dths Net Ping\" ",
			offset + 48
			);

		len = strlen(string);

		//now the players
		for (i = 0; i < current_matchinfo.num_teamplayers; i++)
		{
			if (i >= total[firstteam-1] && i >= total[secondteam-1])
				break; // we're done

			// top (winning team)
			if (i < total[firstteam-1])
			{
				tmpl = &current_matchinfo.teamplayers[sorted[firstteam-1][i]];
				cl_ent = tmpl->client;

				if (!cl_ent)
					ping = tmpl->ping;
				else
					ping = cl_ent->client->ping;

				// calculate player's score
				j = tmpl->enemy_kills - tmpl->team_kills - tmpl->suicides;
				sprintf (entry,
					"yv %d string \"%-15.15s   %4d  %3d %3d  %3d\" ",
					i * 8 + 56,
					tmpl->name,
					j, 
					tmpl->deaths,
					tmpl->enemy_kills - tmpl->team_kills - tmpl->deaths,
					(ping > 999) ? 999 : ping);

				if (maxsize - len > strlen(entry))
				{
					strcat (string, entry);
					len = strlen(string);
					last[firstteam-1] = i;
				}
			}

			// bottom (losing team)
			if (i < total[secondteam-1])
			{
				tmpl = &current_matchinfo.teamplayers[sorted[secondteam-1][i]];

				cl_ent = tmpl->client;

				if (!cl_ent)
					ping = tmpl->ping;
				else
					ping = cl_ent->client->ping;

				// calculate player's score
				j = tmpl->enemy_kills - tmpl->team_kills - tmpl->suicides;
				sprintf (entry,
					"yv %d string \"%-15.15s   %4d  %3d %3d  %3d\" ",
					i * 8 + 56 + offset,
					tmpl->name,
					j, 
					tmpl->deaths,
					tmpl->enemy_kills - tmpl->team_kills - tmpl->deaths,
					(ping > 999) ? 999 : ping);

				if (maxsize - len > strlen(entry))
				{
					strcat (string, entry);
					len = strlen(string);
					last[secondteam-1] = i;
				}
			}
		}
	}
	// wision: warmup scoreboard
	else
	{
		for (i=0 ; i < game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;

			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team == TEAM_A)
				team = 0;
			else if (game.clients[i].pers.team == TEAM_B)
				team = 1;
			else
				continue; // unknown team?

			score = game.clients[i].resp.score;
			for (j = 0; j < total[team]; j++)
			{
				if (score > sortedscores[team][j])
					break;
			}

			for (k = total[team]; k > j; k--)
			{
				sorted[team][k] = sorted[team][k-1];
				sortedscores[team][k] = sortedscores[team][k-1];
			}
			sorted[team][j] = i;
			sortedscores[team][j] = score;
			totalscore[team] += score;
			total[team]++;

			if (game.clients[i].ping > 999)
				averageping[team] += 999;
			else
				averageping[team] += (float)game.clients[i].ping;
		}

		// calculate average ping
		if (total[1] > 0)
			averageping[1] = averageping[1] / (float)total[1];
		if (total[0] > 0)
			averageping[0] = averageping[0] / (float)total[0];

		// set TEAM_B as 1st team if TEAM_A is without players
		if (total[0] < total[1])
		{
			firstteam = TEAM_B;
			secondteam = TEAM_A;
			maxplayers = total[1];
		}
		else
		{
			firstteam = TEAM_A;
			secondteam = TEAM_B;
			maxplayers = total[0];
		}
	
		for (i = 0; i < MAX_TEAMS - 1; i++)
		{
			//determine how wide to draw team score
			len = abs(teaminfo[i+1].score);

			if (len > 999)
				width[i] = 4;
			else if (len > 99)
				width[i] = 3;
			else if (len > 9)
				width[i] = 2;
			else
				width[i] = 1;

			//for negative sign
			if (teaminfo[i+1].score < 0)
				width[i]++;
		}

		//figure out how far the other team needs to be drawn below
		offset = maxplayers * 8 + 24;

		// headers
		if (total[firstteam-1] > 0)
		{
			sprintf (tmpstr, "%s:%.f(%s)", teaminfo[firstteam].name, averageping[firstteam-1], teaminfo[firstteam].skin);
			sprintf (string + strlen(string),
				"xv %d yv 40 string \"%s\" "
				// draw name on X=0 later, so we don't have to set it for all the players below
				"xv 264 yv 48 string2 \"Ping\" xv 8 string2 \" Name\" ",
				(int)((36-strlen(tmpstr))/2)*8 + 8,
				tmpstr
				);
		}
		if (total[secondteam-1] > 0)
		{
			sprintf (tmpstr, "%s:%.f(%s)", teaminfo[secondteam].name, averageping[secondteam-1], teaminfo[secondteam].skin);
			sprintf (string + strlen(string),
				"xv %d yv %d string \"%s\" "
				// draw name on X=0 later, so we don't have to set it for all the players below
				"xv 264 yv %d string2 \"Ping\" xv 8 string2 \" Name\" ",
				(int)((36-strlen(tmpstr))/2)*8 + 8, offset + 40,
				tmpstr,	offset + 48
				);
		}

		len = strlen(string);

		//now the players
		for (i = 0; i < 16; i++)
		{
			if (i >= total[firstteam-1] && i >= total[secondteam-1])
				break; // we're done

			// top
			if (i < total[firstteam-1])
			{
				cl = &game.clients[sorted[firstteam-1][i]];
				cl_ent = g_edicts + 1 + sorted[firstteam-1][i];

				sprintf (entry,
					"yv %d string \"%-15.15s   %13.13s  %3d\" ",
					i * 8 + 56,
 					cl->pers.netname,
					cl->resp.ready ? "[READY]    " : "",
					(cl->ping > 999) ? 999 : cl->ping
					);

				if (maxsize - len > strlen(entry))
				{
					strcat (string, entry);
					len = strlen(string);
					last[firstteam-1] = i;
				}
			}

			// bottom
			if (i < total[secondteam-1])
			{
				cl = &game.clients[sorted[secondteam-1][i]];
				cl_ent = g_edicts + 1 + sorted[secondteam-1][i];

				sprintf (entry,
					"yv %d string \"%-15.15s   %13.13s  %3d\" ",
					i * 8 + 56 + offset,
 					cl->pers.netname,
					cl->resp.ready ? "[READY]    " : "",
					(cl->ping > 999) ? 999 : cl->ping
					);

				if (maxsize - len > strlen(entry))
				{
					strcat (string, entry);
					len = strlen(string);
					last[secondteam-1] = i;
				}
			}
		}
	}

	// put server info on the bottom of screen
	if (maxsize - len > (strlen(serverinfo) + 25))
	{
		sprintf (string + strlen(string), "xl 8 yb -37 string2 \"%s\" ", serverinfo);
		len = strlen(string);
	}

	// put in spectators if we have enough room
	j = total[0] * 8 + total[1] * 8 + 88;
	
	drawn_header = false;
	
	if (maxsize - len > 50)
	{
		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->pers.team != TEAM_SPEC ||
				cl_ent->client->pers.mvdclient)
				continue;

			if (!drawn_header)
			{
				drawn_header = true;
				sprintf (entry, "xv 64 yv %d string2 \" Spectators\" ", j);
				strcat (string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf (entry,
				"yv %d string \"%s:%d%s\" ",
				//0, // x
				j+8, // y
				cl->pers.netname,
				cl->ping > 999 ? 999 : cl->ping,
				cl->chase_target ? va("->%-12.12s", cl->chase_target->client->pers.netname) : "");

			if (maxsize - len > strlen(entry))
			{
				strcat (string, entry);
				len = strlen(string);
			}
			
			j += 8;
		}
	}

	if (maxsize - len > 80)
	{
		if (total[firstteam-1] - last[firstteam-1] > 1) // couldn't fit everyone
			sprintf (string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
				offset + 56 + (last[firstteam-1]+1)*8, total[firstteam-1] - last[firstteam-1] - 1);

		if (total[secondteam-1] - last[secondteam-1] > 1) // couldn't fit everyone
			sprintf (string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
				offset + 56 + (last[secondteam-1]+1)*8, total[secondteam-1] - last[secondteam-1] - 1);
	}

	return string;
}

/*
==============
TDM_Is1V1
==============
Return true if it's 1v1, regardless of game mode. Used to avoid printing
team name when teamname = playername due to the 'pseudo 1v1' mode.
*/
qboolean TDM_Is1V1 (void)
{
	if (level.tdm_pseudo_1v1mode || g_gamemode->value == GAMEMODE_1V1)
		return true;

	return false;
}

/*
==============
TDM_MakeDemoName
==============
All players are ready so start the countdown
*/
const char *TDM_MakeDemoName (edict_t *ent)
{
	int			i;
	int			len;
	struct tm	*ts;
	time_t		t;
	cvar_t		*hostname;
	char		*servername;
	static char	string[1400];

	hostname = gi.cvar ("hostname", NULL, 0);
	
	if (hostname)
		servername = hostname->string;
	else
		servername = "unnamed_server";

	t = time (NULL);
	ts = localtime (&t);

	// current format: playername-team_a-team_b-servername-map-year-mon-day-hour-min-sec
	Com_sprintf (string, sizeof(string), "%s-%s-%s-%s-%s_%d-%02d-%02d_%02d-%02d-%02d",
			ent->client->pers.netname,
			teaminfo[ent->client->pers.team].name,
			teaminfo[(ent->client->pers.team%2)+1].name,
			servername,
			level.mapname,
			ts->tm_year + 1900,
			ts->tm_mon + 1,
			ts->tm_mday,
			ts->tm_hour,
			ts->tm_min,
			ts->tm_sec
			);

	// filter not allowed characters
	len = strlen(string);

	for (i = 0; i < len; i++)
	{
		if ((string[i] < '!' && string[i] > '~') || string[i] == '\\' || string[i] == '\"' || 
				string[i] == ':' || string[i] == '*' || string[i] == '/' || string[i] == '?' ||
				string[i] == '>' || string[i] == '<' || string[i] == '|' || string[i] == ' ')
			string[i] = '_';
	}

	return string;
}

/*
==============
TDM_BeginCountdown
==============
All players are ready so start the countdown
*/
void TDM_BeginCountdown (void)
{
	edict_t *client;

	//set this here so settingsstring knows about it
	if (teaminfo[TEAM_A].players == 1 && teaminfo[TEAM_B].players == 1)
		level.tdm_pseudo_1v1mode = true;
	else
		level.tdm_pseudo_1v1mode = false;

	gi.bprintf (PRINT_HIGH, "Match Settings:\n%s", TDM_SettingsString ());

	gi.bprintf (PRINT_HIGH, "All players ready! Starting countdown (%g secs)...\n", g_match_countdown->value);

	//remove any vote so it doesn't change settings mid-match :D
	if (vote.active)
		TDM_RemoveVote ();

	tdm_match_status = MM_COUNTDOWN;

	//reset these, in case voting code called us from random match state
	level.tdm_timeout_caller = NULL;
	level.timeout_end_framenum = 0;
	level.match_resume_framenum = 0;
	level.match_end_framenum = 0;

	//called to apply a temporary hack for people who do 1v1 on tdm mode
	TDM_UpdateTeamNames ();
	
	level.match_start_framenum = level.framenum + (int)(g_match_countdown->value * SERVER_FPS);

	// wision: force players to record
	for (client = g_edicts + 1; client <= g_edicts + game.maxclients; client++)
	{
		if (client->inuse && client->client->pers.team && (g_force_record->value == 1 || client->client->pers.config.auto_record))
			G_StuffCmd (client, "record \"%s\"\n", TDM_MakeDemoName (client));
	}
}

/*
==============
TDM_EndIntermission
==============
Intermission timer expired or all clients are ready. Reset for another game.
*/
void TDM_EndIntermission (void)
{
	edict_t		*client;
	int			i;

	//for test server
	gi.bprintf (PRINT_CHAT, "Please report any bugs at www.opentdm.net.\n");

	// wision: stop demo recording if we enforce it
	for (client = g_edicts + 1; client <= g_edicts + game.maxclients; client++)
	{
		if (!client->inuse)
			continue;

		if (client->client->pers.team && (g_force_record->value == 1 || client->client->pers.config.auto_record))
			G_StuffCmd (client, "stop\n");
	}


	//shuffle current stats to old and cleanup any players who never reconnected
	if (current_matchinfo.teamplayers)
	{
		if (old_matchinfo.teamplayers)
			gi.TagFree (old_matchinfo.teamplayers);

		current_matchinfo.scores[TEAM_A] = teaminfo[TEAM_A].score;
		current_matchinfo.scores[TEAM_B] = teaminfo[TEAM_B].score;

		old_matchinfo = current_matchinfo;

		current_matchinfo.teamplayers = NULL;
		current_matchinfo.num_teamplayers = 0;

		for (i = 0; i < old_matchinfo.num_teamplayers; i++)
		{
			if (old_matchinfo.teamplayers[i].saved_client)
			{
				gi.TagFree (old_matchinfo.teamplayers[i].saved_client);
				gi.TagFree (old_matchinfo.teamplayers[i].saved_entity);

				old_matchinfo.teamplayers[i].saved_entity = NULL;
				old_matchinfo.teamplayers[i].saved_client = NULL;
			}

			//update matchinfo pointer
			old_matchinfo.teamplayers[i].matchinfo = &old_matchinfo;
		}

		memset (&current_matchinfo, 0, sizeof(current_matchinfo));
	}
	else
		TDM_Error ("TDM_EndIntermission: We should have current_teamplayers but we don't!");

	level.match_score_end_framenum = 0;
	TDM_ResetGameState ();

	TDM_FixDeltaAngles ();
}

/*
==============
TDM_BeginIntermission
==============
Match has ended, move all clients to spectator mode and set origin, note this
is not the same as EndDMLevel since we aren't changing maps.
*/
void TDM_BeginIntermission (void)
{
	int		i;
	edict_t	*ent, *client;

	level.match_score_end_framenum = level.framenum + SECS_TO_FRAMES (g_intermission_time->value);

	//remove any weapons or similar stuff still in flight
	for (ent = g_edicts + game.maxclients + 1; ent < g_edicts + globals.num_edicts; ent++)
	{
		if ((ent->owner && ent->owner->client) || ent->enttype == ENT_GHOST)
			G_FreeEdict (ent);
	}

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	
		// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	
		// chose one of four spots
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

	//take a copy of the scoreboard, have to do this before we destroy
	//teamplayers or no scores are written!
	strcpy (current_matchinfo.scoreboard_string, TDM_ScoreBoardString(NULL));

	// move all clients to the intermission point
	for (client = g_edicts + 1; client <= g_edicts + game.maxclients; client++)
	{
		if (!client->inuse)
			continue;

		MoveClientToIntermission (client);
	}

	//disable chasecams after moving clients to intermission, that way the scoreboard
	//will remember who was chasing who.
	for (client = g_edicts + 1; client <= g_edicts + game.maxclients; client++)
	{
		if (!client->inuse)
			continue;

		//reset any invites
		client->client->resp.last_invited_by = NULL;

		if (client->client->chase_target)
			DisableChaseCam (client);
	}
}

/*
==============
TDM_EndMatch
==============
A match has ended through some means.
Overtime / SD is handled in CheckTimes.
*/
void TDM_EndMatch (void)
{
	qboolean	forfeit;
	int			winner, loser;
	edict_t		*ent;

	//cancel any mid-game vote (restart, etc)
	TDM_RemoveVote ();

	//have to set this here so the stats system doesn't think we're still playing
	tdm_match_status = MM_SCOREBOARD;

	//show stats
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (ent->inuse && ent->client->pers.team)
		{
			if (!ent->client->resp.teamplayerinfo)
				TDM_Error ("TDM_EndMatch: Missing teamplayerinfo for client %d", ent - g_edicts - 1);

			TDM_Stats_f (ent, ent->client->resp.teamplayerinfo->matchinfo);
		}
	}

	winner = 0;
	loser = 0;
	forfeit = false;

	if (teaminfo[TEAM_A].players == 0 && teaminfo[TEAM_B].players == 0)
	{
		winner = TEAM_SPEC;
		loser = TEAM_SPEC;
		gi.bprintf (PRINT_HIGH, "Match canceled, no players remaining.\n");
	}
	else if (teaminfo[TEAM_A].players == 0)
	{
		winner = TEAM_B;
		loser = TEAM_A;
		forfeit = true;
	}
	else if (teaminfo[TEAM_B].players == 0)
	{
		winner = TEAM_A;
		loser = TEAM_B;
		forfeit = true;
	}
	else if (teaminfo[TEAM_A].score > teaminfo[TEAM_B].score)
	{
		winner = TEAM_A;
		loser = TEAM_B;
	}
	else if (teaminfo[TEAM_B].score > teaminfo[TEAM_A].score)
	{
		winner = TEAM_B;
		loser = TEAM_A;
	}
	else
	{
		gi.bprintf (PRINT_HIGH, "Tie game, %d to %d.\n", teaminfo[TEAM_A].score, teaminfo[TEAM_A].score);
	}

	if (winner)
	{
		if (forfeit)
		{
			gi.bprintf (PRINT_HIGH, "Match ended.\n");
			gi.bprintf (PRINT_HIGH, "%s wins by forfeit!\n", teaminfo[winner].name);
		}
		else
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit. Match ended.\n");
			gi.bprintf (PRINT_HIGH, "%s wins, %d to %d.\n", teaminfo[winner].name, teaminfo[winner].score, teaminfo[loser].score);
		}
	}

	current_matchinfo.winning_team = winner;

	level.timeout_end_framenum = 0;
	level.match_resume_framenum = 0;
	level.match_end_framenum = 0;

	TDM_BeginIntermission ();
}

void TDM_Overtime (void)
{
	level.match_end_framenum = level.framenum + SECS_TO_FRAMES ((int)g_overtime->value);

	gi.bprintf (PRINT_HIGH, "Scores are tied %d - %d, adding %g minute%s overtime.\n",
		teaminfo[TEAM_A].score, teaminfo[TEAM_A].score, g_overtime->value / 60, g_overtime->value / 60 == 1 ? "" : "s");

	tdm_match_status = MM_OVERTIME;
}

void TDM_SuddenDeath (void)
{
	gi.bprintf (PRINT_HIGH, "Scores are tied %d - %d, entering Sudden Death!\n", teaminfo[TEAM_A].score, teaminfo[TEAM_A].score);

	tdm_match_status = MM_SUDDEN_DEATH;
}

/*
==============
TDM_NagUnreadyPlayers
==============
Show who isn't ready, only called if >= 50% of other players are ready.
*/
void TDM_NagUnreadyPlayers (void)
{
	char	message[1000];
	edict_t	*ent;
	int		len;

	len = 0;

	message[0] = '\0';

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (ent->client->pers.team == TEAM_SPEC)
			continue;

		if (ent->client->resp.ready)
			continue;

		if (message[0])
		{
			len += 2;
			strcat (message, ", ");
		}

		len += strlen (ent->client->pers.netname);
		strcat (message, ent->client->pers.netname);

		if (len >= sizeof(message)-20)
			break;
	}

	gi.bprintf (PRINT_CHAT, "Waiting on %s\n", message);
}

/*
==============
TDM_FixDeltaAngles
==============
Set angles after a restart of a match otherwise movement done during frozen
will screw things up.
*/
void TDM_FixDeltaAngles (void)
{
	int		i;
	edict_t	*ent;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		//set the delta angle
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
	}
}

/*
==============
TDM_UpdateServerInfo
==============
Update server info visible to browsers.
*/
void TDM_UpdateServerInfo (void)
{
	char		buff[12];

	if (tdm_match_status < MM_PLAYING)
	{
		gi.cvar_forceset("time_remaining", "WARMUP");
		gi.cvar_forceset("Score_B", "WARMUP");
		gi.cvar_forceset("Score_A", "WARMUP");
	}
	else
	{
		if (level.match_end_framenum - level.framenum >= 0)
			gi.cvar_forceset("time_remaining", TDM_SecsToString (FRAMES_TO_SECS(level.match_end_framenum - level.framenum)));
		else
			gi.cvar_forceset("time_remaining", "N/A");

		sprintf(buff, "%d", teaminfo[TEAM_B].score);
		gi.cvar_forceset("Score_B", buff);
		sprintf(buff, "%d", teaminfo[TEAM_A].score);
		gi.cvar_forceset("Score_A", buff);
	}

	if (g_gamemode->value == GAMEMODE_TDM && !(level.tdm_pseudo_1v1mode))
		gi.cvar_forceset("match_type", "TDM");
	else if (level.tdm_pseudo_1v1mode || g_gamemode->value == GAMEMODE_1V1)
		gi.cvar_forceset("match_type", "Duel");
	else if (g_gamemode->value == GAMEMODE_ITDM)
		gi.cvar_forceset("match_type", "ITDM");
	else
		gi.cvar_forceset("match_type", "N/A");
}

/*
==============
TDM_CheckTimes
==============
Check miscellaneous timers, eg match start countdown
*/
void TDM_CheckTimes (void)
{
	edict_t		*ent;

	if (tdm_match_status < MM_PLAYING && level.match_start_framenum)
	{
		int		remaining;

		remaining = level.match_start_framenum - level.framenum;

		if (remaining == (int)(10.4f * SERVER_FPS))
		{
			gi.sound (world, 0, gi.soundindex ("world/10_0.wav"), 1, ATTN_NONE, 0);
		}
		else if (remaining > 0 && remaining <= SECS_TO_FRAMES(5) && remaining % SECS_TO_FRAMES(1) == 0)
		{
			gi.bprintf (PRINT_HIGH, "%d\n", (int)(remaining / SERVER_FPS));
		}
		else if (remaining == 0)
		{
			TDM_BeginMatch ();
		}
	}

	if (level.match_resume_framenum)
	{
		int		remaining;

		remaining = level.match_resume_framenum - level.realframenum;		
		
		if (remaining > 0 && remaining <= SECS_TO_FRAMES(5) && remaining % SECS_TO_FRAMES(1) == 0)
		{
			gi.bprintf (PRINT_HIGH, "%d\n", (int)(remaining / SERVER_FPS));
		}
		else if (remaining == 0)
		{
			TDM_FixDeltaAngles ();
			gi.bprintf (PRINT_HIGH, "Fight!\n");

			level.tdm_timeout_caller = NULL;
			level.match_resume_framenum = 0;
			tdm_match_status = level.last_tdm_match_status;

			UpdateMatchStatus ();
		}
	}

	if (level.match_end_framenum)
	{
		int		remaining;

		if (tdm_match_status == MM_SUDDEN_DEATH)
		{
			if (teaminfo[TEAM_A].score != teaminfo[TEAM_B].score)
				TDM_EndMatch ();
		}
		else
		{
			remaining = level.match_end_framenum - level.framenum;

			if (remaining == (int)(10.4f * SERVER_FPS))
			{
				gi.sound (world, 0, gi.soundindex ("world/10_0.wav"), 1, ATTN_NONE, 0);
			}
			/*else if (remaining > 0 && remaining <= (int)(5.0f / FRAMETIME) && remaining % (int)(1.0f / FRAMETIME) == 0)
			{
				gi.bprintf (PRINT_HIGH, "%d\n", (int)(remaining * FRAMETIME));
			}*/
			else if (remaining == 0)
			{
				if (teaminfo[TEAM_A].score == teaminfo[TEAM_B].score)
				{
					if (g_tie_mode->value == 1)
						TDM_Overtime ();
					else if (g_tie_mode->value == 2)
						TDM_SuddenDeath ();
					else
						TDM_EndMatch ();
				}
				else
					TDM_EndMatch ();
			}
		}
	}

	//end timeout, regardless of what's happening. prevents single player from
	//destroying server should they disconnect eg in 1v1 and never reconnect
	if (level.timeout_end_framenum)
	{
		int		remaining;

		remaining = level.timeout_end_framenum - level.realframenum;

		if (remaining == SECS_TO_FRAMES(60))
		{
			gi.bprintf (PRINT_HIGH, "Automatically resuming in 1 minute.\n");
		}

		if (remaining == 0)
			TDM_ResumeGame ();
	}

	//end of map intermission - same as regular dm style
	if (level.intermissionframe && level.framenum - level.intermissionframe == 0)
	{
		level.exitintermission = 1;
	}

	if (level.next_ready_nag_framenum && level.next_ready_nag_framenum == level.framenum)
	{
		TDM_NagUnreadyPlayers ();
		level.next_ready_nag_framenum = level.framenum + SECS_TO_FRAMES(20);
	}

	//between match intermission - scores only
	if (level.match_score_end_framenum)
	{
		int	remaining;

		remaining = level.match_score_end_framenum - level.framenum;

		if (remaining <= 0)
			TDM_EndIntermission ();

		// take screenshot 5 frames after the intermission began
		if (remaining == SECS_TO_FRAMES (g_intermission_time->value) - 5)
		{
			edict_t *client;

			for (client = g_edicts + 1; client <= g_edicts + game.maxclients; client++)
			{
				if (!client->inuse)
					continue;

				if (client->client->pers.team && (g_force_screenshot->value == 1 || client->client->pers.config.auto_screenshot))
					G_StuffCmd (client, "screenshot\n");
			}
		}
	}

	if (vote.active)
	{
		// update vote string every second since we display the timer
		if ((vote.end_frame - level.framenum) % SERVER_FPS == 0)
		{
			TDM_UpdateVoteConfigString ();
		}

		if (level.framenum == vote.end_frame)
		{
			gi.bprintf (PRINT_HIGH, "Vote failed.\n");
			TDM_RemoveVote ();
		}
	}

	if (tdm_match_status == MM_WARMUP && tdm_settings_not_default && level.framenum >= SECS_TO_FRAMES(300) &&
		teaminfo[TEAM_A].players == 0 && teaminfo[TEAM_B].players == 0)
	{
		qboolean	reset = true;

		for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
		{
			if (!ent->client)
				continue;

			if (FRAMES_TO_SECS (level.framenum - ent->client->last_activity_frame) < 300)
			{
				reset = false;
				break;
			}
		}

		if (reset)
		{
			gi.bprintf (PRINT_HIGH, "No active players for five minutes, restoring default match settings.\n");
			TDM_ResetVotableVariables ();
		}
	}

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		//r1: only show motd once per connect, not on level change
		if (!ent->client->pers.shown_motd && !ent->client->showmotd && level.framenum - ent->client->resp.enterframe == SECS_TO_FRAMES(10))
		{
			TDM_Motd_f (ent);
			ent->client->pers.shown_motd = true;
		}
	}

#ifdef _DEBUG
	TDM_SetFrameTime ();
#endif
}

/*
==============
TDM_CheckMatchStart
==============
See if everyone is ready and there are enough players to start a countdown
*/
void TDM_CheckMatchStart (void)
{
	edict_t	*ent;
	int		ready[MAX_TEAMS];
	int		total_players, total_ready;

	if (tdm_match_status >= MM_PLAYING)
		return;

	ready[TEAM_A] = ready[TEAM_B] = 0;
	total_ready = total_players = 0;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (ent->client->pers.team == TEAM_SPEC)
			continue;

		if (ent->client->resp.ready)
		{
			total_ready++;
			ready[ent->client->pers.team]++;
		}

		total_players++;
	}

	if (teaminfo[TEAM_A].players && ready[TEAM_A] == teaminfo[TEAM_A].players)
		teaminfo[TEAM_A].ready = true;
	else
		teaminfo[TEAM_A].ready = false;

	if (teaminfo[TEAM_B].players && ready[TEAM_B] == teaminfo[TEAM_B].players)
		teaminfo[TEAM_B].ready = true;
	else
		teaminfo[TEAM_B].ready = false;

	if (teaminfo[TEAM_A].ready && teaminfo[TEAM_B].ready)
	{
		level.next_ready_nag_framenum = 0;

		//wision: do NOT restart match during the match
		//r1: under what conditions can/did this happen? late joining shouldn't be possible?
		if (tdm_match_status < MM_COUNTDOWN)
			TDM_BeginCountdown ();
	}
	else
	{
		if (teaminfo[TEAM_A].players && teaminfo[TEAM_B].players &&
			total_players >= 2 && total_ready >= total_players / 2.0f)
		{
			if (!level.next_ready_nag_framenum)
				level.next_ready_nag_framenum = level.framenum + SECS_TO_FRAMES(20);
		}
		else
			level.next_ready_nag_framenum = 0;

		if (level.match_start_framenum)
		{
			//stop the .wav file, icky.
			G_StuffCmd (NULL, "stopsound\n");
			gi.bprintf (PRINT_CHAT, "Countdown aborted!\n");
			level.match_start_framenum = 0;
			level.tdm_pseudo_1v1mode = false;
			tdm_match_status = MM_WARMUP;

			//reset teamnames if we entered pseudo-1v1 mode
			TDM_UpdateTeamNames ();
		}
	}
}

/*
==============
LookupPlayer
==============
Look up a player by partial subname, full name or client id. If multiple
matches, show a list. Return 0 on failure. Case insensitive.
*/
int LookupPlayer (const char *match, edict_t **out, edict_t *ent)
{
	int			matchcount;
	int			numericMatch;
	edict_t		*p;
	char		lowered[32];
	char		lowermatch[32];
	const char	*m;

	if (!match[0])
		return 0;

	matchcount = 0;
	numericMatch = 0;

	m = match;

	while (m[0])
	{
		if (!isdigit (m[0]))
		{
			numericMatch = -1;
			break;
		}
		m++;
	}

	if (numericMatch == 0)
	{
		numericMatch = strtoul (match, NULL, 10);

		if (numericMatch < 0 || numericMatch >= game.maxclients)
		{
			if (ent)
				gi.cprintf (ent, PRINT_HIGH, "Invalid client id %d.\n", numericMatch);
			return 0;
		}
	}

	if (numericMatch == -1)
	{
		Q_strncpy (lowermatch, match, sizeof(lowermatch)-1);
		Q_strlwr (lowermatch);

		for (p = g_edicts + 1; p <= g_edicts + game.maxclients; p++)
		{
			if (!p->inuse)
				continue;

			if (p->client->pers.mvdclient)
				continue;

			Q_strncpy (lowered, p->client->pers.netname, sizeof(lowered)-1);
			Q_strlwr (lowered);

			if (!strcmp (lowered, lowermatch))
			{
				*out = p;
				return 1;
			}

			if (strstr (lowered, lowermatch))
			{
				matchcount++;
				*out = p;
				continue;
			}
		}

		if (matchcount == 1)
		{
			return 1;
		}
		else if (matchcount > 1)
		{
			if (ent)
				gi.cprintf (ent, PRINT_HIGH, "'%s' matches multiple players.\n", match);
			return 0;
		}
	}
	else
	{
		p = g_edicts + 1 + numericMatch;

		if (!p->inuse || p->client->pers.mvdclient)
		{
			if (ent)
				gi.cprintf (ent, PRINT_HIGH, "Client %d is not active.\n", numericMatch);
			return 0;
		}

		*out = p;
		return 1;
	}

	if (ent)
		gi.cprintf (ent, PRINT_HIGH, "No player match found for '%s'\n", match);
	return 0;
}

/*
==============
TDM_UpdateTeamNames
==============
A rather messy function to handle team names in TDM and 1v1. Assigns "dynamic" team names
if two players start a 1v1 game while in game mode TDM.
*/
void TDM_UpdateTeamNames (void)
{
	edict_t	*ent;

	if (g_gamemode->value == GAMEMODE_1V1)
	{
		if (teaminfo[TEAM_A].captain)
		{
			if (strcmp (teaminfo[TEAM_A].name, teaminfo[TEAM_A].captain->client->pers.netname))
			{
				strncpy (teaminfo[TEAM_A].name, teaminfo[TEAM_A].captain->client->pers.netname, sizeof(teaminfo[TEAM_A].name)-1);
				g_team_a_name->modified = true;
			}
		}
		else
		{
			if (strcmp (teaminfo[TEAM_A].name, "Player 1"))
			{
				strcpy (teaminfo[TEAM_A].name, "Player 1");
				g_team_a_name->modified = true;
			}
		}	

		if (teaminfo[TEAM_B].captain)
		{
			if (strcmp (teaminfo[TEAM_B].name, teaminfo[TEAM_B].captain->client->pers.netname))
			{
				strncpy (teaminfo[TEAM_B].name, teaminfo[TEAM_B].captain->client->pers.netname, sizeof(teaminfo[TEAM_B].name)-1);
				g_team_b_name->modified = true;
			}
		}
		else
		{
			if (strcmp (teaminfo[TEAM_B].name, "Player 2"))
			{
				strcpy (teaminfo[TEAM_B].name, "Player 2");
				g_team_b_name->modified = true;
			}
		}	
	}
	else
	{
		if (TDM_Is1V1())
		{
			edict_t	*p1, *p2;

			p1 = TDM_FindPlayerForTeam (TEAM_A);
			p2 = TDM_FindPlayerForTeam (TEAM_B);

			//one of the players might have disappeared due to a disconnect
			if (p1 && p2)
			{
				if (strcmp (teaminfo[TEAM_A].name, p1->client->pers.netname))
				{
					strncpy (teaminfo[TEAM_A].name, p1->client->pers.netname, sizeof(teaminfo[TEAM_A].name)-1);
					g_team_a_name->modified = true;
				}

				if (strcmp (teaminfo[TEAM_B].name, p2->client->pers.netname))
				{
					strncpy (teaminfo[TEAM_B].name, p2->client->pers.netname, sizeof(teaminfo[TEAM_B].name)-1);
					g_team_b_name->modified = true;
				}
			}
		}
		else
		{
			if (strcmp (teaminfo[TEAM_A].name, g_team_a_name->string))
			{
				strncpy (teaminfo[TEAM_A].name, g_team_a_name->string, sizeof(teaminfo[TEAM_A].name)-1);
				g_team_a_name->modified = true;
			}

			if (strcmp (teaminfo[TEAM_B].name, g_team_b_name->string))
			{
				strncpy (teaminfo[TEAM_B].name, g_team_b_name->string, sizeof(teaminfo[TEAM_B].name)-1);
				g_team_b_name->modified = true;
			}
		}
	}

	if (!g_team_a_name->modified && !g_team_b_name->modified)
		return;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (ent->client->pers.team == TEAM_A && g_team_a_name->modified)
		{
			if (TDM_Is1V1())
				gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, ent->client->pers.netname);
			else
				gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, va("%s (%s)", ent->client->pers.netname, teaminfo[TEAM_A].name));
		}
		else if (ent->client->pers.team == TEAM_B && g_team_b_name->modified)
		{
			if (TDM_Is1V1())
				gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, ent->client->pers.netname);
			else
				gi.configstring (CS_TDM_SPECTATOR_STRINGS + (ent - g_edicts) - 1, va("%s (%s)", ent->client->pers.netname, teaminfo[TEAM_B].name));
		}
	}
}

/*
==============
TDM_ResetVotableVariables
==============
Everyone has left the server, so reset anything they voted back to defaults
*/
void TDM_ResetVotableVariables (void)
{
	cvarsave_t	*var;

	gi.dprintf ("Resetting votable variables to defaults.\n");

	var = preserved_vars;

	while (var->variable_name)
	{
		if (!var->default_string)
			TDM_Error ("TDM_ResetVotableVariables: Preserved variable %s with no default", var->variable_name);
		gi.cvar_forceset (var->variable_name, var->default_string);
		var++;
	}

	tdm_settings_not_default = false;

	if (g_gamemode->value == GAMEMODE_ITDM)
		dmflags = gi.cvar_set ("dmflags", g_itdmflags->string);
	else if (g_gamemode->value == GAMEMODE_TDM)
		dmflags = gi.cvar_set ("dmflags", g_tdmflags->string);
	else if (g_gamemode->value == GAMEMODE_1V1)
		dmflags = gi.cvar_set ("dmflags", g_1v1flags->string);

	gi.AddCommandString ("exec defaults.cfg\nsv applysettings\n");

	TDM_ResetLevel ();
	TDM_UpdateConfigStrings (true);
}	

void TDM_ResumeGame (void)
{
	if (tdm_match_status != MM_TIMEOUT)
		TDM_Error ("TDM_ResumeGame called with match state %d", tdm_match_status);

	if (teaminfo[TEAM_A].players == 0 && teaminfo[TEAM_B].players == 0)
	{
		TDM_EndMatch();
		return;
	}

	gi.sound (world, 0, gi.soundindex ("world/10_0.wav"), 1, ATTN_NONE, 0);
	gi.bprintf (PRINT_CHAT, "Game resuming in 10 seconds. Match time remaining: %s\n", TDM_SecsToString(FRAMES_TO_SECS(level.match_end_framenum - level.framenum)));

	level.timeout_end_framenum = 0;
	level.match_resume_framenum = level.realframenum + SECS_TO_FRAMES(10.4);
}

/*
==============
TDM_SetCaptain
==============
Set ent to be a captain of team, ent can be NULL to remove captain
*/
void TDM_SetCaptain (int team, edict_t *ent)
{
	teaminfo[team].captain = ent;

	//no announce in 1v1, but captain is still silently used.
	if (ent && g_gamemode->value != GAMEMODE_1V1)
	{
		gi.bprintf (PRINT_HIGH, "%s became captain of '%s'\n", ent->client->pers.netname, teaminfo[team].name);
		//gi.cprintf (ent, PRINT_CHAT, "You are the captain of '%s'\n", teaminfo[team].name);
	}
}

/*
==============
TDM_SetupSounds
==============
First time setup of sound paths, caches commonly used client sounds indexes.
*/
void TDM_SetupSounds (void)
{
	int		i;
	char	path[MAX_QPATH];

	strcpy (path, "*");

	for (i = 0; i < SND_MAX; i++)
	{
		strcpy (path + 1, soundnames[i]);
		soundcache[i] = gi.soundindex (path);
	}
}

/*
==============
CountPlayers
==============
Count how many players each team has.
*/
void CountPlayers (void)
{
	edict_t	*ent;
	int		i;
	int		total;

	for (i = 0; i < MAX_TEAMS; i++)
		teaminfo[i].players = 0;

	total = 0;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (ent->inuse)
		{
			if (!ent->client->pers.mvdclient)
			{
				teaminfo[ent->client->pers.team].players++;
				total++;
			}
		}
	}
}

/*
==============
UpdateMatchStatus
==============
Update match status (end match when whole team leaves i.e.)
*/
void UpdateMatchStatus (void)
{
	int team;

	//unlock if a team emptied out
	for (team = 1; team < MAX_TEAMS; team++)
		if (teaminfo[team].players == 0)
		{
			teaminfo[team].locked = false;
			teaminfo[team].speclocked = false;
		}

	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	if (tdm_match_status == MM_TIMEOUT)
	{
		if (teaminfo[TEAM_A].players + teaminfo[TEAM_B].players == 0)
			TDM_EndMatch ();
		return;
	}

	for (team = 1; team < MAX_TEAMS; team++)
	{
		if (teaminfo[team].players < 1)
		{
			TDM_EndMatch ();
			break;
		}
	}
}

/*
==============
UpdatePlayerTeamMenu
==============
Update the join menu to reflect team names / player counts
*/
static char	openTDMBanner[32];

void UpdatePlayerTeamMenu (edict_t *ent)
{
	void		*teamJoinLeaveFunc[MAX_TEAMS];
	unsigned	i;

	memcpy (ent->client->pers.joinmenu, joinmenu, sizeof(joinmenu));

	for (i = 0; i < MAX_TEAMS; i++)
	{
		if (ent->client->pers.team == i)
		{
			sprintf (ent->client->pers.joinmenu_values.string_teamJoinText[i], "*Leave %.20s", teaminfo[i].name);
			teamJoinLeaveFunc[i] = ToggleChaseCam;
		}
		else
		{
			sprintf (ent->client->pers.joinmenu_values.string_teamJoinText[i], "*Join %.20s", teaminfo[i].name);

			if (i == 1)
				teamJoinLeaveFunc[i] = JoinTeam1;
			else if (i == 2)
				teamJoinLeaveFunc[i] = JoinTeam2;
		}
	}

	ent->client->pers.joinmenu[0].text = openTDMBanner;

	ent->client->pers.joinmenu[3].text = ent->client->pers.joinmenu_values.string_teamJoinText[1];
	ent->client->pers.joinmenu[3].SelectFunc = teamJoinLeaveFunc[1];
	ent->client->pers.joinmenu[4].text = teamStatus[1];

	ent->client->pers.joinmenu[6].text = ent->client->pers.joinmenu_values.string_teamJoinText[2];
	ent->client->pers.joinmenu[6].SelectFunc = teamJoinLeaveFunc[2];
	ent->client->pers.joinmenu[7].text = teamStatus[2];

	ent->client->pers.joinmenu[10].text = teamStatus[0];

	/* we might just update the join/leave message.. no need to update it for the client */
	if (ent->client->pers.menu.active && ent->client->pers.menu.entries == ent->client->pers.joinmenu)
	{
		PMenu_Update (ent);
		gi.unicast (ent, true);
	}
}

/*
==============
UpdateTeamMenu
==============
Update the join menu for all players
*/
void UpdateTeamMenu (void)
{
	unsigned			i;
	edict_t				*ent;
	static const char	*gameString[] = {
		"TDM",
		"ITDM",
		"1v1"
	};

	for (i = 0; i < MAX_TEAMS; i++)
		sprintf (teamStatus[i], "  (%d player%s)", teaminfo[i].players, teaminfo[i].players == 1 ? "" : "s");

	sprintf (openTDMBanner, "*Quake II - OpenTDM (%s)", gameString[(int)g_gamemode->value]);

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
		if (ent->inuse)
			UpdatePlayerTeamMenu (ent);
}

/*
==============
TDM_TeamsChanged
==============
The teams have changed in some way, so check everything out
*/
void TDM_TeamsChanged (void)
{
	CountPlayers ();
	TDM_UpdateTeamNames ();
	UpdateTeamMenu ();
	UpdateMatchStatus ();
	TDM_CheckMatchStart ();
	TDM_CheckVote ();
}

/*
==============
TDM_SetupSpawns
==============
Keeps track of spawn points rather than counting/looping each time.
*/
void TDM_SetupSpawns (void)
{
	int		count;
	edict_t	*spot;

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		level.spawns[count] = spot;
		count++;

		if (count > TDM_MAX_MAP_SPAWNPOINTS)
		{
			TDM_Error ("TDM_SetupSpawns: too many spawn points");
			break;
		}
	}

	level.numspawns = count;
	gi.dprintf ("Map has %d spawn points.\n", level.numspawns);
}

/*
==============
TDM_CheckMap_FileExists
==============
Check map name.
Not used since we don't check if file exists.
*/
qboolean TDM_CheckMapExists (const char *mapname)
{
	cvar_t	*gamedir;      // our mod dir
	cvar_t	*basedir;      // our root dir
	FILE	*mf;
	char	buffer[MAX_QPATH + 1];

	gamedir = gi.cvar ("gamedir", NULL, 0);   // created by engine, we need to expose it for mod
	basedir = gi.cvar ("basedir", NULL, 0);   // created by engine, we need to expose it for mod

	buffer[sizeof(buffer)-1] = '\0';

	if (basedir)
	{
		// check basedir
		snprintf (buffer, sizeof(buffer)-1, "%s/baseq2/maps/%s.bsp", basedir->string, mapname);

		mf = fopen (buffer, "r");
		if (mf != NULL)
		{
			fclose (mf);
			return true;
		}
	}

	if (gamedir)
	{
		// check gamedir
		snprintf (buffer, sizeof(buffer), "%s/maps/%s.bsp", gamedir->string, mapname);
		mf = fopen (buffer, "r");
		if (mf == NULL)
			return false;
		else
		{   
			fclose (mf);
			return true;
		}
	}

	return false;
}

/*
==============
TDM_WriteMaplist
==============
Prints the maplist to the client.
*/
void TDM_WriteMaplist (edict_t *ent)
{
	int				i, j;
	static char		maplist_string[1024];
	static qboolean	short_maplist = false;

	// maplist fits the string, so no need to create new maplist again
	if (short_maplist)
		gi.cprintf (ent, PRINT_HIGH, "%s", maplist_string);
	else
	{
		short_maplist = true;

		maplist_string[0] = 0;
		j = sprintf (maplist_string, "Allowed maplist:\n----------------\n");

		for (i = 0; tdm_maplist[i] != NULL; i++)
		{
			if (strlen(tdm_maplist[i]) + 4 + j >= sizeof(maplist_string) - 1)
			{
				j = 0;
				short_maplist = false;
				gi.cprintf (ent, PRINT_HIGH, "%s", maplist_string);
				maplist_string[j] = 0;
			}
			j += sprintf (maplist_string + j, "  %s\n", tdm_maplist[i]);
		}

		gi.cprintf (ent, PRINT_HIGH, "%s", maplist_string);
	}
}

/*
==============
TDM_CreateMaplist
==============
Return array of allowed maps in maplist.
*/
void TDM_CreateMaplist (void)
{
	qboolean	start = true;
	int			len;
	int			entries_num = 100;
	cvar_t		*gamedir;
	int			i = 0, j = 0;
	FILE		*maplst;
	char		buffer[MAX_QPATH + 1];
	char		*entry;

	if (tdm_maplist)
	{
		gi.TagFree (tdm_maplist);
		tdm_maplist = NULL;
	}

	// maplist is not in use
	if (!g_maplistfile || !g_maplistfile->string[0])
		return;

	// created by engine, we need to expose it for mod
	gamedir = gi.cvar ("gamedir", NULL, 0); 

	// Make sure we can find the game directory.
	if (!gamedir || !gamedir->string[0])
		return;
	
	buffer[sizeof(buffer)-1] = '\0';

	snprintf (buffer, sizeof(buffer)-1, "./%s/%s", gamedir->string, g_maplistfile->string);
	maplst = fopen (buffer, "r");
	if (maplst == NULL)
		return;

	tdm_maplist = gi.TagMalloc (sizeof(char *) * entries_num, TAG_GAME);

	for (;;)
	{
		entry = fgets (buffer, sizeof (buffer), maplst);

		if (entry == NULL)
			break;

		len = strlen (buffer);

		// cut only first column from the line
		for (i = 0; i < len; i++)
		{
			if (buffer[i] == ' ' || (!isalnum (buffer[i]) && buffer[i] != '_' && buffer[i] != '-'))
			{
				buffer[i] = '\0';

				if (start)
					entry++;
			}
			else
				start = false;
		}

		if (!entry[0])
			continue;

		tdm_maplist[j] = gi.TagMalloc (strlen(entry) + 1, TAG_GAME);
		strcpy (tdm_maplist[j], entry);
		j++;

		// realloc
		if (j % entries_num == 0)
		{
			char	**tmp;

			tmp = gi.TagMalloc (sizeof(char *) * (j + entries_num), TAG_GAME);
			memcpy (tmp, tdm_maplist, j * sizeof(char *));

			gi.TagFree (tdm_maplist);
			tdm_maplist = tmp;
		}
	}

	// close the maplist
	if (maplst != NULL)
		fclose (maplst);

	if (j)
		// set NULL so we know where is the end
		tdm_maplist[j] = NULL;
	else
	{
		gi.TagFree (tdm_maplist);
		tdm_maplist = NULL;
	}
}

/*
==============
TDM_CheckMap
==============
Check map name.
Modified code from QwazyWabbit. http://www.clanwos.org/forums/viewtopic.php?t=3983
*/
qboolean TDM_Checkmap (edict_t *ent, const char *mapname)
{
	int		i;
	size_t	len;

	len = strlen (mapname);

	if (len >= MAX_QPATH - 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid map name.\n");
		return false;
	}

	for (i = 0; i < len; i++)
	{
		if (!isalnum (mapname[i]) && mapname[i] != '_' && mapname[i] != '-')
		{
			gi.cprintf (ent, PRINT_HIGH, "Invalid map name.\n");
			return false;
		}
	}

	// allow map on maplist failure
	if (tdm_maplist == NULL)
		return true;

	for (i = 0; tdm_maplist[i] != NULL; i++)
	{
		if (!Q_stricmp (tdm_maplist[i], mapname) /* && TDM_CheckMapExists (buffer)) */)
			return true;
	}

	gi.cprintf (ent, PRINT_HIGH, "Map '%s' was not found\n", mapname);
	return false;
}

/*
==============
TDM_MapChanged
==============
Called when a new level is about to begin.
*/
void TDM_MapChanged (void)
{
	//check console didn't try to set gamemode to invalid value
	if (g_gamemode->value > 2 || g_gamemode->value < 0)
		gi.error ("g_gamemode: invalid value");

	//check for common config error
	if (timelimit->value)
		gi.dprintf ("WARNING: The cvar 'timelimit' is no longer used. Did you intend to set g_match_time?\n");

	//cancel any vote if the map was changed by admin
	TDM_RemoveVote ();
	TDM_ResetGameState ();
	TDM_SetSkins ();
	TDM_SetupSounds ();
	TDM_UpdateConfigStrings (true);
	TDM_SetupSpawns ();
}

/*
==============
TDM_ResetGameState
==============
Reset the game state after a match has completed or a map / mode change.
*/
void TDM_ResetGameState (void)
{
	edict_t		*ent;

	level.tdm_pseudo_1v1mode = false;
	level.match_start_framenum = 0;
	level.warmup_start_framenum = level.framenum;

	tdm_match_status = MM_WARMUP;
	TDM_ResetLevel ();
	TDM_SetFrameTime ();

	//don't memset, since we have info we do actually want to preserve
	teaminfo[TEAM_A].score = teaminfo[TEAM_B].score = 0;
	teaminfo[TEAM_A].players = teaminfo[TEAM_B].players = 0;
	teaminfo[TEAM_A].locked = teaminfo[TEAM_B].locked = false;
	teaminfo[TEAM_A].ready = teaminfo[TEAM_B].ready = false;

	//preserve captains as per bug #0000001
	if (!g_auto_rejoin_match->value)
		teaminfo[TEAM_A].captain = teaminfo[TEAM_B].captain = NULL;

	TDM_UpdateTeamNames ();

	//note, this block of code only runs on a reset from the same map. a map change
	//will have every client ->inuse false until they are reconnected.
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (ent->inuse)
		{
			ent->client->resp.last_command_frame = 0;
			ent->client->resp.last_invited_by = NULL;
			ent->client->resp.score = 0;
			ent->client->resp.teamplayerinfo = NULL;
			ent->client->resp.ready = false;

			ent->client->showmotd = false;
			ent->client->showscores = false;
			ent->client->showoldscores = false;

			ent->viewheight = 0;
			ent->health = 0;

			if (ent->client->pers.team != TEAM_SPEC && g_auto_rejoin_match->value)
			{
				//preserve teams as per bug #0000001
				//ent->client->pers.team = TEAM_SPEC;
				JoinedTeam (ent, false, false);
			}
			else
			{
				ent->client->pers.team = TEAM_SPEC;
				respawn(ent);
			}
		}
	}

	//run these functions only after we've moved everyone to spec mode
	CountPlayers ();
	UpdateTeamMenu ();

	//show menu for players in spec
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (ent->inuse && !ent->client->pers.team)
			TDM_ShowTeamMenu (ent);
	}

	//re-resolve otdm server in case of DNS change
	HTTP_ResolveOTDMServer ();
}

/*
==============
TDM_Init
==============
Single time initialization stuff.
*/
void TDM_Init (void)
{
	cvar_t		*var;
	const char	*p;
	int			revision;

	HTTP_Init ();

	var = gi.cvar ("game", NULL, 0);
	if (!var)
		gi.error ("Couldn't determine game directory");

	strncpy (game.gamedir, var->string, sizeof(game.gamedir)-1);

	//ensure R1Q2 entflags are available since we use them for all projectiles
	var = gi.cvar ("sv_new_entflags", NULL, 0);
	if (!var)
	{
		//super cheesy notice!
		gi.dprintf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		gi.dprintf ("               W A R N I N G !\n");
		gi.dprintf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		gi.dprintf ("\n");
		gi.dprintf ("  OpenTDM is designed to use some of the new\n");
		gi.dprintf ("  features in R1Q2. Your server does not\n");
		gi.dprintf ("  appear to be running R1Q2, or is out of\n");
		gi.dprintf ("  date. Some features may not work correctly.\n");
		gi.dprintf ("\n");
		gi.dprintf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	}
	else
	{
		gi.cvar_set ("sv_new_entflags", "1");
	}

	TDM_SetFrameTime ();

	strcpy (teaminfo[0].name, "Spectators");
	strcpy (teaminfo[0].skin, "male/grunt");

	//this is used as index in some arrays, must be valid!
	if (g_gamemode->value > 2 || g_gamemode->value < 0)
		gi.error ("g_gamemode: invalid value");

	TDM_SaveDefaultCvars ();

	if (g_gamemode->value == GAMEMODE_ITDM)
		dmflags = gi.cvar_set ("dmflags", g_itdmflags->string);
	else if (g_gamemode->value == GAMEMODE_TDM)
		dmflags = gi.cvar_set ("dmflags", g_tdmflags->string);
	else if (g_gamemode->value == GAMEMODE_1V1)
		dmflags = gi.cvar_set ("dmflags", g_1v1flags->string);

	//show opentdm version to browsers
	p = strchr (OPENTDM_VERSION, ':');
	if (p)
	{
		p += 2;
		revision = atoi (p);
		gi.cvar ("revision", va("%d", revision), CVAR_SERVERINFO|CVAR_NOSET);
	}

	TDM_ResetGameState ();

	TDM_CreateMaplist ();
	TDM_CreateConfiglist ();
}

/*
==============
TDM_SetTeamSkins
==============
Setup teamskin/enemyskin configstrings.
*/
void TDM_SetTeamSkins (edict_t *cl, edict_t *target_to_set_skins_for)
{
	edict_t		*ent;
	const char	*teamskin, *enemyskin;

	//not using teamskins
	if (!cl->client->pers.config.teamskin[0] && !cl->client->pers.config.enemyskin[0])
		return;

	teamskin = cl->client->pers.config.teamskin;
	enemyskin = cl->client->pers.config.enemyskin;

	if (!enemyskin[0])
	{
		//don't care about enemyskin
		if (!strcmp (teamskin, g_team_a_skin->string))
			enemyskin = g_team_b_skin->string;
		else
			enemyskin = g_team_a_skin->string;
	}
	else if (!teamskin[0])
	{
		//don't care about teamskin
		if (!strcmp (enemyskin, g_team_a_skin->string))
			teamskin = g_team_b_skin->string;
		else
			teamskin = g_team_a_skin->string;
	}

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (target_to_set_skins_for && ent != target_to_set_skins_for)
			continue;

		if (!ent->inuse)
			continue;

		if (ent->client->pers.team)
		{
			gi.WriteByte (svc_configstring);
			gi.WriteShort (CS_PLAYERSKINS + (ent - g_edicts) -1);

			//spectators get team A as teamskin, team B as enemyskin
			if (ent->client->pers.team == cl->client->pers.team ||
				(cl->client->pers.team == TEAM_SPEC && ent->client->pers.team == TEAM_A))
				gi.WriteString (va ("%s\\%s", ent->client->pers.netname, teamskin));
			else
				gi.WriteString (va ("%s\\%s", ent->client->pers.netname, enemyskin));

			gi.unicast (cl, true);

			//gi.dprintf ("TS DEBUG: Setting skin of %s for %s.\n", ent->client->pers.netname, cl->client->pers.netname);
		}	
	}
}

void TDM_SetAllTeamSkins (edict_t *target_to_set_skins_for)
{
	edict_t	*ent;

	//now reset anyone who had teamskin/enemyskin set. ew.
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		TDM_SetTeamSkins (ent, target_to_set_skins_for);
	}
}

/*
==============
TDM_SetSkins
==============
Setup skin configstrings.
*/
void TDM_SetSkins (void)
{
	edict_t		*ent;
	const char	*newskin, *oldskin;
	unsigned 	i;
	//int			index;

	for (i = TEAM_A; i <= TEAM_B; i++)
	{
		oldskin = teaminfo[i].skin;

		if (i == TEAM_A)
		{
			//index = CS_TDM_TEAM_A_PIC;
			newskin = g_team_a_skin->string;
		}
		else
		{
			//index = CS_TDM_TEAM_B_PIC;
			newskin = g_team_b_skin->string;
		}

		if (!strcmp (oldskin, newskin))
			continue;

		//gi.configstring (index, va("/players/%s_i.pcx", newskin));

		strncpy (teaminfo[i].skin, newskin, sizeof(teaminfo[i].skin)-1);

		for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
		{
			if (!ent->inuse)
				continue;

			if (ent->client->pers.team == i)
			{
				gi.configstring (CS_PLAYERSKINS + (ent - g_edicts) - 1, va("%s\\%s", ent->client->pers.netname, teaminfo[i].skin));
			}	
		}
	}

	TDM_SetAllTeamSkins (NULL);
}

/*
==============
TDM_SecsToString
==============
Convert secs to mm:ss string.
*/
const char *TDM_SecsToString (int seconds)
{
	static char	time_buffer[32];
	int			mins;

	mins = seconds / 60;
	seconds -= (mins * 60);

	sprintf (time_buffer, "%d:%.2d", mins, seconds);

	return time_buffer;
}

/*
==============
TDM_UpdateConfigStrings
==============
Check any cvar and other changes and update relevant configstrings
*/
void TDM_UpdateConfigStrings (qboolean forceUpdate)
{
	static int			last_timeout_remaining = -1;
	static int			last_time_remaining = -1;
	static int			last_scores[MAX_TEAMS] = {-9999, -9999, -9999};
	static qboolean		last_ready_status[MAX_TEAMS] = {false, false, false};
	static matchmode_t	last_mode = MM_INVALID;
	int					time_remaining;
	int					timeout_remaining;
	qboolean			need_serverinfo_update;

	need_serverinfo_update = false;

	if (g_team_a_name->modified || forceUpdate)
	{
		g_team_a_name->modified = false;
		sprintf (teaminfo[TEAM_A].statname, "%31s", teaminfo[TEAM_A].name);
		gi.configstring (CS_TDM_TEAM_A_NAME, teaminfo[TEAM_A].statname);
	}

	if (g_team_b_name->modified || forceUpdate)
	{
		g_team_b_name->modified = false;
		sprintf (teaminfo[TEAM_B].statname, "%31s", teaminfo[TEAM_B].name);
		gi.configstring (CS_TDM_TEAM_B_NAME, teaminfo[TEAM_B].statname);
	}

	if (g_team_a_skin->modified || g_team_b_skin->modified || forceUpdate)
	{
		g_team_a_skin->modified = g_team_b_skin->modified = false;
		TDM_SetSkins ();
	}

	if (tdm_match_status != last_mode || forceUpdate ||
		last_ready_status[TEAM_A] != teaminfo[TEAM_A].ready ||
		last_ready_status[TEAM_B] != teaminfo[TEAM_B].ready)
	{
		last_mode = tdm_match_status;

		switch (tdm_match_status)
		{
			//force scores to update
			case MM_PLAYING:
				gi.configstring (CS_TDM_GAME_STATUS, "Match");
				last_scores[TEAM_A] = last_scores[TEAM_B] = -9999;
				break;

			//note, we shouldn't need to do anything when we are mm_countdown, but fall through just to be safe
			case MM_COUNTDOWN:
				gi.configstring (CS_TDM_GAME_STATUS, "Countdown");
			case MM_WARMUP:
				if (tdm_match_status == MM_WARMUP)
					gi.configstring (CS_TDM_GAME_STATUS, "Warmup");

				if (teaminfo[TEAM_A].ready != last_ready_status[TEAM_A] || forceUpdate)
				{
					last_ready_status[TEAM_A] = teaminfo[TEAM_A].ready;

					if (teaminfo[TEAM_A].ready)
						sprintf (teaminfo[TEAM_A].statstatus, "%15s", "READY");
					else
						sprintf (teaminfo[TEAM_A].statstatus, "%15s", "WARMUP");

					gi.configstring (CS_TDM_TEAM_A_STATUS, teaminfo[TEAM_A].statstatus);
				}

				if (teaminfo[TEAM_B].ready != last_ready_status[TEAM_B] || forceUpdate)
				{
					last_ready_status[TEAM_B] = teaminfo[TEAM_B].ready;

					if (teaminfo[TEAM_B].ready)
						sprintf (teaminfo[TEAM_B].statstatus, "%15s", "READY");
					else
						sprintf (teaminfo[TEAM_B].statstatus, "%15s", "WARMUP");

					gi.configstring (CS_TDM_TEAM_B_STATUS, teaminfo[TEAM_B].statstatus);
				}
				break;

			case MM_SUDDEN_DEATH:
				gi.configstring (CS_TDM_TIMELIMIT_STRING, "Sudden Death");
				break;

			case MM_OVERTIME:
				gi.configstring (CS_TDM_GAME_STATUS, "Overtime");
				break;

			case MM_SCOREBOARD:
				gi.configstring (CS_TDM_GAME_STATUS, "Match End");
				break;

			default:
				gi.configstring (CS_TDM_GAME_STATUS, "Match");
				//nothing to do!
				break;
		}
	}

	if (tdm_match_status >= MM_PLAYING)
	{
		int		i;

		for (i = TEAM_A; i <= TEAM_B; i++)
		{
			if (last_scores[i] != teaminfo[i].score || forceUpdate)
			{
				last_scores[i] = teaminfo[i].score;
				sprintf (teaminfo[i].statstatus, "%15d", teaminfo[i].score);
				gi.configstring (CS_TDM_TEAM_A_STATUS + (i - TEAM_A), teaminfo[i].statstatus);
				need_serverinfo_update = true;
			}
		}
	}

	switch (tdm_match_status)
	{
		case MM_COUNTDOWN:
			timeout_remaining = 0;
			time_remaining = level.match_start_framenum - level.framenum;
			break;
		case MM_TIMEOUT:
			if (level.match_resume_framenum)
				timeout_remaining = level.match_resume_framenum - level.realframenum;
			else
				timeout_remaining = level.timeout_end_framenum - level.realframenum;
			time_remaining = level.match_end_framenum - level.framenum;
			break;
		case MM_WARMUP:
			timeout_remaining = 0;
			time_remaining = g_match_time->value * (1 * SERVER_FPS) - 1;
			break;
		case MM_SUDDEN_DEATH:
			timeout_remaining = 0;
			time_remaining = 0;
			break;
		default:
			timeout_remaining = 0;
			time_remaining = level.match_end_framenum - level.framenum;
			break;
	}

	if (time_remaining != last_time_remaining || forceUpdate)
	{
		static int	last_secs = -1;
		char		time_buffer[8];
		int			mins, secs;

		last_time_remaining = time_remaining;

		secs = ceil((float)time_remaining * FRAMETIME);

		if (secs < 0)
			secs = 0;

		if (secs != last_secs || forceUpdate)
		{
			last_secs = secs;

			mins = secs / 60;
			secs -= (mins * 60);

			sprintf (time_buffer, "%2d:%.2d", mins, secs);

			if (last_secs < 60)
				TDM_SetColorText (time_buffer);
			else if (last_secs == 60 && tdm_match_status >= MM_PLAYING)
			{
				gi.sound (world, 0, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NONE, 0);
				gi.bprintf (PRINT_HIGH, "1 minute remaining.\n");
			}

			gi.configstring (CS_TDM_TIMELIMIT_STRING, time_buffer);

			need_serverinfo_update = true;
		}
	}

	if (timeout_remaining != last_timeout_remaining)
	{
		static int	last_secs = -1;
		char		time_buffer[8];
		int			mins, secs;

		last_timeout_remaining = timeout_remaining;

		secs = ceil((float)timeout_remaining * FRAMETIME);

		if (secs < 0)
			secs = 0;

		if (secs != last_secs || forceUpdate)
		{
			last_secs = secs;

			mins = secs / 60;
			secs -= (mins * 60);

			sprintf (time_buffer, "%2d:%.2d", mins, secs);

			if (last_secs < 60)
				TDM_SetColorText (time_buffer);

			gi.configstring (CS_TDM_TIMEOUT_STRING, time_buffer);
		}
	}

	if (need_serverinfo_update)
		TDM_UpdateServerInfo ();
}

/*
==============
TDM_Error
==============
Die horribly.
*/
void TDM_Error (const char *fmt, ...)
{
	va_list		argptr;
	char		text[512];
	edict_t		*ent;

	va_start (argptr,fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);
	text[sizeof(text)-1] = 0;

	gi.dprintf ("An internal OpenTDM error has occured. Please save the following information and post it on the forums at www.opentdm.net.\n");
	gi.dprintf ("ERROR: %s\n", text);
	gi.dprintf ("Match state: %d\n", tdm_match_status);
	gi.dprintf ("Team status: %d - %d - %d\n", teaminfo[TEAM_SPEC].players, teaminfo[TEAM_A].players, teaminfo[TEAM_B].players);
	gi.dprintf ("Level times: frame: %d, start: %d, end: %d, resume: %d, scores: %d\n", level.framenum, level.match_start_framenum, level.match_end_framenum,
		level.match_resume_framenum, level.match_score_end_framenum);
	gi.dprintf ("Vote status: Active: %d, type: %d, end: %d, initiator %d\n", vote.active, vote.flags, vote.end_frame, vote.initiator ? (int)(vote.initiator - g_edicts) : 0);
	
	gi.dprintf ("Client dump:\n");
	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		gi.dprintf ("%d: %s, connected %d, team %d, info %p\n", (int)(ent - g_edicts - 1), ent->client->pers.netname, ent->client->pers.connected, ent->client->pers.team, ent->client->resp.teamplayerinfo);
	}

	gi.error (text);
}

/*
==============
TDM_ProcessText
==============
Generic function for processing a bunch of text a line at a time. Destroys input!
*/
qboolean TDM_ProcessText (char *buff, int len, qboolean (*func)(char *, int, void *), void *param)
{
	char		line[256];
	char		*q;
	char		*ptr;
	int			line_number;

	ptr = buff;
	q = buff;

	line_number = 1;

	while (len)
	{
		switch (buff[0])
		{
			case '\n':
			case '\r':
				buff[0] = 0;
				if (q)
				{
					qboolean	parse;
					char		*p;

					Q_strncpy (line, q, sizeof(line)-1);
					parse = true;

					p = strchr (line, '\n');
					if (p)
						p[0] = 0;

					p = strchr (line, '\r');
					if (p)
						p[0] = 0;

					if (line[0] == '#' || line[0] == '/' || line[0] == '\0')
						parse = false;
					else if (line[0] == '\\')
					{
						/*if (!strncmp (line + 1, "include ", 8))
						{
							char	*path;

							path = line + 9;

							if (!TDM_ProcessFile (path, func))
								gi.dprintf ("WARNING: Unable to read included file '%s' on line %d\n", path, line_number);
						}
						else*/
						{
							gi.dprintf ("WARNING: Unknown directive '%s' on line %d\n", line + 1, line_number);
						}

						parse = false;
					}

					if (parse)
					{
						if (!func (line, line_number, param))
							return false;
					}
					q = NULL;
					line_number++;
				}
				buff++;
				break;
			case '\0':
				buff++;
				break;
			default:
				if (!q)
					q = buff;
				buff++;
				break;
		}
		len--;
	}

	return true;
}

/*
==============
TDM_HandleDownload
==============
A download we requested for something has finished. Do stuff.
*/
void TDM_HandleDownload (tdm_download_t *download, char *buff, int len, int code)
{
	//handle went invalid (client->pers->download = zeroed), just ignore this download completely.
	if (!download->inuse)
		return;

	//player left before download finished, lame!
	//note on an extremely poor connection it's possible another player since occupied their slot, but
	//for that to happen, the download must take 3+ seconds which should be unrealistic, so i don't
	//deal with it.
	if (!download->initiator->inuse || download->initiator->client->pers.uniqueid != download->unique_id)
		download->initiator = NULL;

	download->onFinish (download, code, (byte *)buff, len);

	/*switch (download->type)
	{
		case DL_CONFIG:
			TDM_ConfigDownloaded (buff, len, code);
			break;

		case DL_PLAYER_CONFIG:
			TDM_

		case DL_POST_STATS:
			break;
		
		default:
			TDM_Error ("TDM_HandleDownload: Unrequested/unknown type");
			break;
	}*/
}

/*
==============
TDM_GetTeamFromArgs
==============
Generic function to return a team index from gi.arg*, used to lookup
team based on user input. -1 on error.
*/
int TDM_GetTeamFromArg (edict_t *ent, const char *value)
{
	if (!Q_stricmp (value, "1") || !Q_stricmp (value, "A") || !Q_stricmp (value, teaminfo[TEAM_A].name))
	{
		return TEAM_A;
	}
	else if (!Q_stricmp (value, "2") || !Q_stricmp (value, "B") || !Q_stricmp (value, teaminfo[TEAM_B].name))
	{
		return TEAM_B;
	}
	else if (!Q_stricmp (value, "0") || !Q_stricmp (value, "S") || !Q_stricmp (value, "O") ||
		!Q_stricmp (value, "spec") || !Q_stricmp (value, "obs"))
	{
		return TEAM_SPEC;
	}

	return -1;
}

/*
==============
TDM_UpdateSpectator
==============
Updates the spectator according to chosen mode.
*/
void TDM_UpdateSpectator (edict_t *ent)
{
	int			score = -999;
	edict_t		*target = NULL, *tmp = NULL;

	for (target = g_edicts + 1; target <= g_edicts + game.maxclients; target++)
	{
		// skip if client is not a player or we are not allowed to spectate his team and we are not an admin
		if (!target->inuse || !target->client->pers.team ||
				(teaminfo[target->client->pers.team].speclocked &&
				 !ent->client->pers.specinvite[target->client->pers.team] &&
				 !ent->client->pers.admin))
			continue;

		// find quadder
		if (ent->client->resp.spec_mode == SPEC_QUAD)
		{
			if (target->client->quad_framenum > level.framenum)
			{
				SetChase (ent, target);
				break;
			}
		}
		// find invul guy
		else if (ent->client->resp.spec_mode == SPEC_INVUL)
		{
			if (target->client->invincible_framenum > level.framenum)
			{
				SetChase (ent, target);
				break;
			}
		}
		// count frags and find top fragger
		else if (ent->client->resp.spec_mode == SPEC_LEADER)
		{
			if (target->client->resp.score > score)
			{
				score = target->client->resp.score;
				tmp = target;
			}
		}
	}

	if (ent->client->resp.spec_mode == SPEC_LEADER && tmp)
			SetChase (ent, tmp);
}

/*
==============
TDM_UpdateSpectatorsOnEvent
==============
Updates all spectators when something happens.
*/
void TDM_UpdateSpectatorsOnEvent (int spec_mode, edict_t *target, edict_t *killer)
{
	int			score_a = -999, score_b = -999;
	edict_t		*e, *top_fragger_a = NULL, *top_fragger_b = NULL, *new_target = NULL;

	// calculate the top fragger
	if (spec_mode == SPEC_KILLER)
	{
		for (e = g_edicts + 1; e <= g_edicts + game.maxclients; e++)
		{
			if (!e->inuse || !e->client->pers.team)
				continue;

			if (e->client->pers.team == TEAM_A)
			{
				if (e->client->resp.score > score_a)
				{
					score_a = e->client->resp.score;
					top_fragger_a = e;
				}
			}
			else if (e->client->pers.team == TEAM_B)
			{
				if (e->client->resp.score > score_b)
				{
					score_b = e->client->resp.score;
					top_fragger_b = e;
				}
			}
		}
	}

	for (e = g_edicts + 1; e <= g_edicts + game.maxclients; e++)
	{
		// we are looking for a spectator who wants an auto-followed POV
		if (!e->inuse || e->client->pers.team || e->client->resp.spec_mode == SPEC_NONE)
			continue;

		// don't bother with spectators who are not allowed to watch anything
		if (teaminfo[TEAM_A].speclocked && teaminfo[TEAM_B].speclocked &&
				e->client->pers.specinvite[TEAM_A] && e->client->pers.specinvite[TEAM_B])
			continue;

		new_target = NULL;

		if (spec_mode == SPEC_KILLER)
		{
			// update spectators who use SPEC_KILLER and spectating the victim
			if (e->client->resp.spec_mode == SPEC_KILLER && e->client->chase_target == target &&
				(!teaminfo[target->client->pers.team].speclocked || e->client->pers.specinvite[target->client->pers.team]))
			{
				//could be worldspawn!
				if (killer->client)
					new_target = killer;
			}
			// check if the killer is not top fragger now and update spectators who us SPEC_LEADER
			else if (e->client->resp.spec_mode == SPEC_LEADER)
			{
				if (score_a > score_b)
				{
					if (!teaminfo[TEAM_A].speclocked || e->client->pers.specinvite[TEAM_A])
						new_target = top_fragger_a;
					else if (e->client->chase_target != top_fragger_b)
						new_target = top_fragger_b;
				}
				else if (score_b > score_a)
				{
					if (!teaminfo[TEAM_B].speclocked || e->client->pers.specinvite[TEAM_B])
						new_target = top_fragger_b;
					else if (e->client->chase_target != top_fragger_a)
						new_target = top_fragger_a;
				}
			}
		}
		// some player took quad or invul, update all spectators with SPEC_QUAD on
		else if ((spec_mode == SPEC_QUAD || spec_mode == SPEC_INVUL) && e->client->resp.spec_mode == spec_mode &&
				(!teaminfo[target->client->pers.team].speclocked || e->client->pers.specinvite[target->client->pers.team]))
		{
			new_target = target;
		}

		if (new_target)
			SetChase (e, new_target);
	}
}
