/*
Copyright (C) 2003-2004 Nathan Wulf (jitspoe)

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

// ===
// jitmenu / jitscores
// cl_scores.c -- Client side scoreboard.

#include "client.h"

#define SCORE_LISTVIEW_COLUMN_COUNT 9

typedef struct cl_score_s {
	int ping;
	int kills;
	int deaths;
	int grabs;
	int caps;
	int starttime;
	char team;
	qboolean isalive;
	qboolean hasflag;
	// jitodo -- shots fired w/each gun, etc.
	qboolean inuse;
} cl_score_t;

static cl_score_t cl_scores[MAX_CLIENTS];
static qboolean cl_scores_modified;

char **cl_scores_nums;
char **cl_scores_info;
char ***cl_scores_listview_info;
int cl_scores_listview_column_count;
int cl_scores_count;

int splat(int teamnum) 
{
	if (teamnum < 1 || teamnum > 4)
		return 154; // white

	switch (cl.configstrings[CS_TEAMINDEXES][teamnum-1]) 
	{
	case 1: // red
		return 15;
	case 2: // blue
		return 14;
	case 3: // purple
		return 5;
	case 4: // yellow
		return 28;
	default:
		return ' ';
	}
}

int cl_scores_get_team_splat (int client)
{
	return splat(cl_scores[client].team);
}

unsigned char cl_scores_get_team_textcolor (int client)
{
	int teamnum;
	teamnum = cl_scores[client].team;

	if (teamnum < 1 || teamnum > 4)
		return COLOR_CHAT;

	switch (cl.configstrings[CS_TEAMINDEXES][teamnum-1]) 
	{
	case 1: // red
		return 158;
	case 2: // blue
		return 159;
	case 3: // purple
		return 141;
	case 4: // yellow
		return 140;
	default:
		return COLOR_CHAT;
	}
}

int cl_scores_get_team (int client)
{
	return cl_scores[client].team;
}

int cl_scores_get_isalive (int client)
{
	return (cls.gametype == GAMETYPE_DM) || cl_scores[client].isalive; // todo - move dm check to set_isalive
}

void init_cl_scores (void)
{
	int i;

	// allocating memory for the scoreboard...
	cl_scores_nums = Z_Malloc(sizeof(char*)*MAX_CLIENTS);
	cl_scores_info = Z_Malloc(sizeof(char*)*MAX_CLIENTS);
	cl_scores_listview_info = Z_Malloc(sizeof(char**)*MAX_CLIENTS);
	cl_scores_listview_column_count = SCORE_LISTVIEW_COLUMN_COUNT;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		cl_scores_nums[i] = Z_Malloc(sizeof(char) * 4);
		cl_scores_info[i] = Z_Malloc(sizeof(char) * MAX_SCOREBOARD_STRING);
	}

	memset(cl_scores, 0, sizeof(cl_scores));
	memset(cl_scores_listview_info, 0, sizeof(char**)*MAX_CLIENTS);

	cl_scores_modified = true;
}

void shutdown_cl_scores (void) // jitodo
{
	int i, j;

	if (cl_scores_nums)
		Z_Free(cl_scores_nums);

	if (cl_scores_info)
		Z_Free(cl_scores_info); //xrichardx: isn't there a memory leak here?

	if (cl_scores_listview_info)
	{
		for (i = 0; i < MAX_CLIENTS && cl_scores_listview_info[i]; i++)
		{
			for(j = 0; j < cl_scores_listview_column_count && cl_scores_listview_info[i][j]; j++)
				Z_Free(cl_scores_listview_info[i][j]);

			Z_Free(cl_scores_listview_info[i]);
		}
		Z_Free(cl_scores_listview_info);
	}
}

void cl_scores_refresh (void)
{
	cl_scores_modified = true;
	M_RefreshWidget("scores", true);
}

void cl_scores_setping (int client, int ping)
{
	cl_scores[client].ping = ping;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setstarttime (int client, int time)
{
	cl_scores[client].starttime = time;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setkills (int client, int kills)
{
	cl_scores[client].kills = kills;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setdeaths (int client, int deaths)
{
	cl_scores[client].deaths = deaths;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setgrabs (int client, int grabs)
{
	cl_scores[client].grabs = grabs;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setcaps (int client, int caps)
{
	cl_scores[client].caps = caps;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setteam (int client, char team)
{
	cl_scores[client].team = team;
	cl_scores[client].inuse = true;
	cl_scores_refresh();
}

void cl_scores_setisalive (int client, qboolean alive)
{
	cl_scores[client].isalive = alive;
	cl_scores[client].inuse = true;

	if (!alive)
		cl_scores_sethasflag(client, false);

	cl_scores_refresh();
}

void cl_scores_setisalive_all (qboolean alive)
{
	int i;

	for (i=0; i<MAX_CLIENTS; i++)
		if (cl_scores[i].inuse)
			cl_scores[i].isalive = alive;

	cl_scores_refresh();
}

void cl_scores_sethasflag_all (qboolean hasflag)
{
	int i;

	for (i=0; i<MAX_CLIENTS; i++)
		if (cl_scores[i].inuse)
			cl_scores[i].hasflag = hasflag;

	cl_scores_refresh();
}

void cl_scores_sethasflag (int client, qboolean hasflag)
{
	cl_scores[client].hasflag = hasflag;
	cl_scores_refresh();
}

void cl_scores_setinuse (int client, qboolean inuse)
{
	cl_scores[client].inuse = inuse;
	cl_scores_refresh();
}

void cl_scores_setinuse_all (qboolean inuse)
{
	int i;

	for (i=0; i<MAX_CLIENTS; i++)
		cl_scores[i].inuse = inuse;

	cl_scores_refresh();
}

void cl_scores_clear (int client)
{
	memset(&cl_scores[client], 0, sizeof(cl_score_t));
	cl_scores_refresh();
}


static void RequestPings()
{
	static int lastrequest = 0;

	if (cls.realtime - lastrequest > 2000) // update at a max of 2 seconds
	{
		if (cls.server_gamebuild >= 126) // 1.80+ server version
			Cbuf_AddText("cmd getpings 0\n");

		lastrequest = cls.realtime;
	}
}

// sort the scoreboard.
static int cl_sorted_scorelist[MAX_CLIENTS];
static void SortScores (void)
{
	int i, j, k, score;
	int sortedscores[MAX_CLIENTS];

	cl_scores_count = 0;

	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!cl_scores[i].inuse)
			continue;

		if (cls.gametype != GAMETYPE_DM)
		{
			// put player's own team at top of list:
			if (cl_scores[cl.playernum].team && cl_scores[i].team == cl_scores[cl.playernum].team)
				score = 5;
			else
				score = cl_scores[i].team;
		}
		else
		{
			score = 0;
		}

		switch ((int)(cl_scoreboard_sorting->value))
		{
		case 1:															// default until build 41
			score = ((score)*1000) + cl_scores[i].kills;
			break;
		case 2:															// default since build 41
			score = (score * 10000) + cl_scores[i].caps * 8 + cl_scores[i].grabs * 4 + cl_scores[i].kills;
			break;
		case 3:															// T3RR0R15T's favorite :) ---> need some tests
			score = (score * 10000) + cl_scores[i].caps * 8 + cl_scores[i].grabs * 4 + cl_scores[i].kills - cl_scores[i].deaths / 2 + (600 - (cl.frame.servertime/1000 - cl_scores[i].starttime)/60);
			break;
		default:
			score = (score * 10000) + cl_scores[i].caps * 8 + cl_scores[i].grabs * 4 + cl_scores[i].kills;
			break;
		}

		for (j = 0; j < cl_scores_count; ++j)
		{
			if (score > sortedscores[j] ||
				(score == sortedscores[j] &&
				cl_scores[i].deaths < cl_scores[cl_sorted_scorelist[j]].deaths))
			{
				break;
			}
		}

		for (k=cl_scores_count; k>j; k--) // Move it down the list
		{
			cl_sorted_scorelist[k] = cl_sorted_scorelist[k-1];
			sortedscores[k] = sortedscores[k-1];
		}

		cl_sorted_scorelist[j] = i;
		sortedscores[j] = score;
		cl_scores_count++;
	}
}

int strpos_noformat (const char *in_str, int pos)
{
	int count = 0;
	const char *s;

	s = in_str;

	while (*s)
	{
		if (*(s + 1) && (*s == SCHAR_UNDERLINE || *s == SCHAR_ENDFORMAT || *s == SCHAR_ITALICS))
			{ } // don't count character
		else if (*(s + 1) && *s == SCHAR_COLOR)
			++s; // skip two characters.
		else
			++count;

		if (count >= pos)
			return (s - in_str);

		++s;
	}

	return -1;
}

// put the scores into readable strings for the widget to draw
#define MAX_NAME_WIDTH 19
#define MAX_NAME_WIDTH_S "19"
qboolean cl_scores_prep_select_widget (void)
{
	int i, j, len_noformat, len, format_diff;

	RequestPings();

	if (!cl_scores_modified)
		return false;

	SortScores();

	for (i = 0; i < cl_scores_count; i++)
	{
		j = cl_sorted_scorelist[i];
		Com_sprintf(cl_scores_nums[i], 4, "%d", j);
		Com_sprintf(cl_scores_info[i], MAX_SCOREBOARD_STRING, "%c%c%s",
			cl_scores[j].hasflag ? 25 : cl_scores[j].isalive ? 26 : ' ',
			splat(cl_scores[j].team), name_from_index(j));
		len_noformat = strlen_noformat(cl_scores_info[i]);
		len = strlen(cl_scores_info[i]);
		format_diff = len - len_noformat;
		
		if (len_noformat > MAX_NAME_WIDTH) // name too long
		{
			if (format_diff) // colored name
			{
				int pos;

				pos = strpos_noformat(cl_scores_info[i], MAX_NAME_WIDTH+1);
				cl_scores_info[i][pos] = SCHAR_ENDFORMAT;
				cl_scores_info[i][pos + 1] = '\0';
			}
			else
			{
				cl_scores_info[i][MAX_NAME_WIDTH] = '\0';
			}
		}
		else if (format_diff)
		{
			// add spaces to compensate for format codes
			memset(cl_scores_info[i]+len, ' ', MAX_NAME_WIDTH - len_noformat);
			cl_scores_info[i][MAX_NAME_WIDTH+format_diff] = '\0';
		}

		Com_sprintf(cl_scores_info[i], MAX_SCOREBOARD_STRING, "%-" MAX_NAME_WIDTH_S "s%4d%3d%3d%3d%2d%3d",
			cl_scores_info[i], cl_scores[j].ping, cl_scores[j].kills,
			cl_scores[j].deaths, cl_scores[j].grabs, cl_scores[j].caps,
 			(cl.frame.servertime/1000 - cl_scores[j].starttime)/60);
	}

	cl_scores_modified = false;
	return true;
}

qboolean cl_scores_prep_listview_widget (void)
{
	int i, j, k;
	char buffer[MAX_SCOREBOARD_STRING];

	RequestPings();

	if (!cl_scores_modified)
		return false;

	SortScores();

	for (i = 0; i < cl_scores_count; i++)
	{
		j = cl_sorted_scorelist[i];
		Com_sprintf(cl_scores_nums[i], 4, "%d", j);

		for(k = 0; k < cl_scores_listview_column_count && cl_scores_listview_info[i] && cl_scores_listview_info[i][k]; k++)
			Z_Free(cl_scores_listview_info[i][k]);

		if (!cl_scores_listview_info[i])
			cl_scores_listview_info[i] = Z_Malloc(sizeof(char**) * cl_scores_listview_column_count);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%c",
			cl_scores[j].hasflag ? 25 : cl_scores[j].isalive ? 26 : ' ');
		cl_scores_listview_info[i][0] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%c", splat(cl_scores[j].team));
		cl_scores_listview_info[i][1] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%s", name_from_index(j));
		cl_scores_listview_info[i][2] = CopyString(buffer);
		
		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", cl_scores[j].ping);
		cl_scores_listview_info[i][3] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", cl_scores[j].kills);
		cl_scores_listview_info[i][4] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", cl_scores[j].deaths);
		cl_scores_listview_info[i][5] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", cl_scores[j].grabs);
		cl_scores_listview_info[i][6] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", cl_scores[j].caps);
		cl_scores_listview_info[i][7] = CopyString(buffer);

		Com_sprintf(buffer, MAX_SCOREBOARD_STRING, "%d", (cl.frame.servertime/1000 - cl_scores[j].starttime)/60);
		cl_scores_listview_info[i][8] = CopyString(buffer);
	}

	cl_scores_modified = false;
	return true;
}


void CL_Score_f (void) // jitodo jitscores -- client-side scoreboard
{
	int i;

	cl_scores_prep_select_widget();

	for (i=0; i<cl_scores_count; i++)
	{
		Com_Printf("%s\n", cl_scores_info[i]);
	}
}

void CL_Scoreboard_f (void)
{
	static qboolean show = true;

	if (cls.state != ca_active)
		return;

	// scoreboard disabled via console or esc, so force it to show again.
	if (!M_MenuActive())
		show = true;

	if (cls.server_gamebuild < 126)
	{
		Cbuf_AddText("cmd score\n");
		return;
	}

	if (show)
		CL_ScoreboardShow_f();
	else
		CL_ScoreboardHide_f();

	show = !show;
}


void CL_ScoreboardShow_f (void)
{
	if (cls.state != ca_active)
		return;

	if (cls.server_gamebuild < 126)
		Cbuf_AddText("cmd scoreson\n");
	else
		Cbuf_AddText("menu scores\n");
}


void CL_ScoreboardHide_f (void)
{
	if (cls.state != ca_active)
		return;

	if (cls.server_gamebuild < 126)
		Cbuf_AddText("cmd scoresoff\n");
	else
		Cbuf_AddText("menu pop scores\n");
}


#define MAX_DECODE_ARRAY 256
#define SCORESIZE 10
static unsigned int temp_array[MAX_DECODE_ARRAY];
// client index, alive, flag, team, ping, kills, deaths, grabs, caps, start time
void CL_ParseScoreData (const unsigned char *data)
{
	unsigned int i, j=0, idx, count;

	count = decode_unsigned(data, temp_array, MAX_DECODE_ARRAY) / SCORESIZE;

	for (i = 0; i < count; i++)
	{
		idx = temp_array[j++];

		if (idx > 255)
			return;

		cl_scores_setinuse(idx, true);
		cl_scores_setisalive(idx, temp_array[j++]);
		cl_scores_sethasflag(idx, temp_array[j++]);
		cl_scores_setteam(idx, (char)temp_array[j++]);
		cl_scores_setping(idx, temp_array[j++]);
		cl_scores_setkills(idx, temp_array[j++]);
		cl_scores_setdeaths(idx, temp_array[j++]);
		cl_scores_setgrabs(idx, temp_array[j++]);
		cl_scores_setcaps(idx, temp_array[j++]);
		cl_scores_setstarttime(idx, temp_array[j++]);
	}
}

#define PINGSIZE 2
void CL_ParsePingData (const unsigned char *data)
{
	unsigned int i, j=0, idx, count;

	count = decode_unsigned(data, temp_array, MAX_DECODE_ARRAY) / PINGSIZE;

	for (i = 0; i < count; i++)
	{
		idx = temp_array[j++];

		if (idx > 255)
			return;

		cl_scores_setping(idx, temp_array[j++]);
	}
}


#define MAX_SCOREDATA_SEND 32
#define SCORESIZE 10 // client index, alive, flag, team, ping, kills, deaths, grabs, caps, start time
static unsigned int scoredata[SCORESIZE*MAX_SCOREDATA_SEND];
static byte scorestr[SCORESIZE*MAX_SCOREDATA_SEND*6];

// returns true if there's more data
int CL_ScoresDemoData (int startindex, unsigned char **sptr)
{
	register int count = 0, i, j=0;
	int retcode = 0;

	for (i = startindex; i < MAX_CLIENTS; i++)
	{
		if (count >= MAX_SCOREDATA_SEND)
		{
			retcode = i;
			break;
		}

		if (cl_scores[i].inuse)
		{
			 // client index, alive, flag, team, ping, kills, deaths, grabs, caps, start time
			scoredata[j++] = i;
			scoredata[j++] = cl_scores[i].isalive;
			scoredata[j++] = cl_scores[i].hasflag;
			scoredata[j++] = cl_scores[i].team;
			scoredata[j++] = cl_scores[i].ping;
			scoredata[j++] = cl_scores[i].kills;
			scoredata[j++] = cl_scores[i].deaths;
			scoredata[j++] = cl_scores[i].grabs;
			scoredata[j++] = cl_scores[i].caps;
			scoredata[j++] = cl_scores[i].starttime;
			count++;
		}
	}

	if (j)
	{
		encode_unsigned(j, scoredata, scorestr);
		*sptr = scorestr;
	}
	else
	{
		*sptr = NULL;
	}

	return retcode;
}
// jitscores
// ===
