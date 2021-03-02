/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"

lvar_t *use_mapvote;
lvar_t *mapvote_list;
lvar_t *mapvote_min;
lvar_t *mapvote_max;
lvar_t *mapvote_time;
lvar_t *mapvote_need;
lvar_t *mapvote_pass;
lvar_t *mapvote_tries;
lvar_t *mapvote_instant;

qboolean voting = false;
edict_t *vote_ent = NULL;
edict_t *decide_ent = NULL;
float vote_time;
char vote_map[32];
int vote_need, vote_pass;
int vote_yes, vote_no;
qboolean vote_cancel = false;
qboolean vote_instant = false;

// all clients printf (excludes console)
void acprintf(int level, char *format, ...) {
	int i;
	edict_t *ent;

	va_list argptr;
	char text[1024];

	va_start(argptr, format);
	vsnprintf(text, sizeof(text), format, argptr);
	va_end(argptr);

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		gi.cprintf(ent, level, text);
	}
}

qboolean Vote_IsPlaying(edict_t *ent) {
	if(!ent->inuse)
		return false;
	if(ent->lithium_flags & LITHIUM_OBSERVER)
		return false;
	if(ent->client->chase_target != NULL)
		return false;

	return true;
}

int Vote_CountPlaying() {
	int playing = 0;
	int i;
	edict_t *cl_ent;
	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(Vote_IsPlaying(cl_ent))
			playing++;
	}
	return playing;
}

void Vote_InitGame(void) {
	use_mapvote = lvar("use_mapvote", "1", "^", VAR_USE);
	mapvote_list = lvar("mapvote_list", "mapvote.lst", "str", VAR_NONE);
	mapvote_min = lvar("mapvote_min", "0", "##", VAR_VOTE);
	mapvote_max = lvar("mapvote_max", "8", "##", VAR_VOTE);
	mapvote_time = lvar("mapvote_time", "45", "##", VAR_VOTE);
	mapvote_need = lvar("mapvote_need", "50", "##", VAR_VOTE);
	mapvote_pass = lvar("mapvote_pass", "51", "##", VAR_VOTE);
	mapvote_tries = lvar("mapvote_tries", "3", "##", VAR_VOTE);
	mapvote_instant = lvar("mapvote_instant", "1", "##", VAR_VOTE);
}

extern qboolean admin_override;

qboolean Vote_Allow(edict_t *ent) {
	if(!use_mapvote->value) {
		gi.cprintf(ent, PRINT_HIGH, "Map voting disabled on this server.\n");
		return false;
	}

	if(Vote_CountPlaying() < mapvote_min->value || Vote_CountPlaying() > mapvote_max->value) {
		gi.cprintf(ent, PRINT_HIGH, "Map voting only allowed when %d to %d clients are playing.\n", (int)mapvote_min->value, (int)mapvote_max->value);
		return false;
	}

	if(ent->lclient->vote_tries >= mapvote_tries->value) {
		gi.cprintf(ent, PRINT_HIGH, "Only allowed %d map vote tries (reset each map).\n", (int)mapvote_tries->value);
		return false;
	}

	if(admin_override) {
		gi.cprintf(ent, PRINT_HIGH, "An admin has already set the next map.\n");
		return false;
	}

	return true;
}

void Vote_Menu(edict_t *ent) {

	if(!voting) {
		if(!Vote_Allow(ent))
			return;
		Menu_Create(ent, 4, 18);
		Menu_Title(ent, "Map Vote");
		Mapqueue_Menu(ent, "vote map");
		return;
	}

	Menu_Create(ent, 16, 18);
	Menu_Title(ent, "Map Vote");

	Menu_AddLine(ent, MENU_TEXT, 4, "Voting in progress", "l");
	Menu_AddText(ent, 6, "Map name..... %s", vote_map);
	Menu_AddText(ent, 7, "When......... %s", vote_instant ? "instant" : "map over");
	Menu_AddText(ent, 8, "Request by... %s", vote_ent->client->pers.netname);
	Menu_AddText(ent, 9, "Playing...... %d", Vote_CountPlaying());
	Menu_AddText(ent, 10, "Need votes... %d%c (%d)", (int)mapvote_need->value, '%', vote_need);
	Menu_AddText(ent, 11, "Passes with.. %d%c (%d)", (int)mapvote_pass->value, '%', vote_pass);
	Menu_AddText(ent, 12, "Yes votes.... %d%c (%d)", vote_yes * 100 / MAX(Vote_CountPlaying(), 1), '%', vote_yes);
	Menu_AddText(ent, 13, "No votes..... %d%c (%d)", vote_no * 100 / MAX(Vote_CountPlaying(), 1), '%', vote_no);
	Menu_AddText(ent, 14, "Time left.... %d", (int)(mapvote_time->value - level.time + vote_time + 0.5));

	if(ent == vote_ent) {
		Menu_AddLine(ent, MENU_CMD, 0, "Continue playing", "menu");
		Menu_AddLine(ent, MENU_CMD, 0, "Cancel map vote", "vote cancel");
	}
	else if(ent->lclient->vote == -1) {
		Menu_AddLine(ent, MENU_CMD, 0, "Vote yes", "vote yes");
		Menu_AddLine(ent, MENU_CMD, 0, "Vote no", "vote no");
	}
}

void Vote_RunFrame(void) {
	int i;
	edict_t *cl_ent;
	static int last_time;
	qboolean stop = false;
	qboolean pass = false;
	qboolean timeup = false;

	if(!voting) {
		last_time = 0;
		return;
	}

	if(level.time - last_time < 1)
		return;

	last_time = level.time;

	vote_need = mapvote_need->value * Vote_CountPlaying() / 100 - 0.001;
	vote_pass = mapvote_pass->value * Vote_CountPlaying() / 100 - 0.001;

	vote_yes = 0;
	vote_no = 0;

	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(!cl_ent->inuse)
			continue;
		if(!cl_ent->lclient)
			continue;
		if(!Vote_IsPlaying(cl_ent))
			continue;
		if(cl_ent->lclient->vote == 0)
			vote_no++;
		else if(cl_ent->lclient->vote == 1)
			vote_yes++;
	}

	pass = vote_yes + vote_no > vote_need && vote_yes > vote_pass;

	if(mapvote_time->value - level.time + vote_time < 0)
		timeup = true;

	if(pass || vote_cancel || timeup || level.intermissiontime)
		stop = true;

	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(!cl_ent->inuse)
			continue;
		if(stop)
			Menu_Destroy(cl_ent);
		else {
			// semi-hacky stuff here...
			if(cl_ent->menu && cl_ent->menu->firstline && cl_ent->menu->firstline->next && !strcmp(cl_ent->menu->firstline->next->text, "Map Vote")) {
				qboolean lastline;
				if(cl_ent->menu->sel == cl_ent->menu->lastline)
					lastline = 1;
				else
					lastline = 0;
				Vote_Menu(cl_ent);
				if(cl_ent->lclient->vote == -1 || cl_ent == vote_ent) {
					if(lastline)
						cl_ent->menu->sel = cl_ent->menu->lastline;
					else
						cl_ent->menu->sel = cl_ent->menu->lastline->prev;
				}
			}
		}
	}

	if(stop) {
		if(vote_cancel)
			gi.bprintf(PRINT_HIGH, "%s has canceled the map vote.\n", vote_ent->client->pers.netname);
		else if(timeup)
			gi.bprintf(PRINT_HIGH, "Voting time up, no map change.\n");
		else if(pass) {
			gi.bprintf(PRINT_HIGH, "Voting passed, new map set: %s.\n", vote_map);
			Mapqueue_Override(vote_map);
			if(vote_instant)
				end_time = level.time + 0.5;
		}
		voting = false;
		vote_ent = NULL;
		vote_cancel = false;
	}
}

void Vote_Start2(edict_t *ent) {
	int i;
	edict_t *cl_ent;

	voting = true;
	vote_ent = ent;
	vote_time = level.time;
	strlcpy(vote_map, ent->lclient->vote_map, sizeof(vote_map));
	ent->lclient->vote_tries++;

	for(i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(!cl_ent->inuse)
			continue;
		cl_ent->lclient->vote = -1;
	}
		
	ent->lclient->vote = 1;

	acprintf(PRINT_HIGH, "%s has voted for map %s.\nType 'yes' or 'no' to place your vote.\nType 'vote' for more info and stats.\n", ent->client->pers.netname, vote_map);
	gi.dprintf("%s has voted for map %s.\n", ent->client->pers.netname, vote_map);

	Vote_Menu(ent);
}

void Vote_Start(edict_t *ent) {
	if(voting) {
		Vote_Menu(ent);
		return;
	}

	if(!Vote_Allow(ent))
		return;

	if(!strlen(gi.argv(2))) {
		Vote_Menu(ent);
		return;
	}

	if(!Mapqueue_Valid(file_gamedir(mapvote_list->string), gi.argv(2))) {
		gi.cprintf(ent, PRINT_HIGH, "Not a valid map for this server.\n");
		return;
	}

	strlcpy(ent->lclient->vote_map, gi.argv(2), sizeof(ent->lclient->vote_map));

	if(!mapvote_instant->value) {
		vote_instant = false;
		Vote_Start2(ent);
		return;
	}

	decide_ent = ent;

	Menu_Create(ent, 4, 8);
	Menu_Title(ent, "Map Vote");
	Menu_AddLine(ent, MENU_CMD, 0, "Instant map change", "vote instant");
	Menu_AddLine(ent, MENU_CMD, 0, "Change when map over", "vote mapover");
//	Menu_CancelFunc(ent, Lithium_Menu);
}

void Vote_Cmd(edict_t *ent, char *cmd1, char *cmd2) {
	if(!Vote_IsPlaying(ent)) {
		gi.cprintf(ent, PRINT_HIGH, "You must be playing to vote.\n");
		return;
	}

	if(ent == vote_ent) {
		if(!Q_stricmp(cmd1, "cancel"))
			vote_cancel = true;
		if(!ent->menu)
			Vote_Menu(ent);
		return;
	}

	if(!Q_stricmp(cmd1, "map"))
		Vote_Start(ent);
	else if(!Q_stricmp(cmd1, "yes"))
		ent->lclient->vote = 1;
	else if(!Q_stricmp(cmd1, "no"))
		ent->lclient->vote = 0;
	else if(!Q_stricmp(cmd1, "change"))
		ent->lclient->vote ^= 1;
	else if(!Q_stricmp(cmd1, "instant") && ent == decide_ent) {
		if(!mapvote_instant) {
			gi.cprintf(ent, PRINT_HIGH, "Instant map change disabled on this server.\n");
			vote_instant = false;
		}
		else
			vote_instant = true;
		decide_ent = NULL;
		Vote_Start2(ent);
	}
	else if(!Q_stricmp(cmd1, "mapover") && ent == decide_ent) {
		vote_instant = false;
		decide_ent = NULL;
		Vote_Start2(ent);
	}
	else
		Vote_Menu(ent);
}

qboolean Vote_ClientCommand(edict_t *ent) {
	char *cmd = gi.argv(0);
	if(!Q_stricmp(cmd, "vote"))
		Vote_Cmd(ent, gi.argv(1), gi.argv(2));
	else if(!Q_stricmp(cmd, "yes") && voting)
		Vote_Cmd(ent, "yes", "");
	else if(!Q_stricmp(cmd, "no") && voting)
		Vote_Cmd(ent, "no", "");
	else
		return false;
	return true;
}
