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

#include <sys/stat.h>
#include <sys/types.h>
#include "g_local.h"
#ifdef WIN32
#include "direct.h"
#endif

#define WRITE_HIGHSCORES 10
#define MAX_HIGHSCORES 20

place_t *first_place;

char *Highscores_File(void) {
	static char buf[64];
	snprintf(buf, sizeof(buf), "%s/hiscores/%s.%02dm", gi.cvar("gamedir", 0, 0)->string,
		level.mapname, (int)timelimit->value);
	return buf;
}

void Highscores_Read(void) {
	int p = 0;
	char buf[64];
	FILE *file = fopen(Highscores_File(), "rt");

	place_t *place = NULL;
	place_t *prev = NULL;

	first_place = NULL;

	if(file) {
		while(fgets(buf, 64, file)) {
			place = (place_t *)gi.TagMalloc(sizeof(place_t), TAG_LEVEL);
			sscanf(buf, "%d;%[^';'];%s\r\n", &place->score, place->name, place->date);

			if(p == 0)
				first_place = place;

			place->prev = prev;
			if(prev)
				prev->next = place;
			prev = place;

			p++;
			if(p == WRITE_HIGHSCORES)
				break;
		}
		fclose(file);
	}

	if(place)
		place->next = NULL;
}

void Highscores_Update(void) {
	int i;
	edict_t *ent;
	FILE *file;
	char buf[64];
	place_t *place, *new_place = NULL;
	
	time_t t;
	struct tm *tm;
	time(&t);
	tm = localtime(&t);

	Highscores_Read();

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;

		if(ent->client->resp.score <= 0)
			continue;

		place = first_place;
		while(place) {
			if(ent->client->resp.score > place->score) {
				new_place = (place_t *)gi.TagMalloc(sizeof(place_t), TAG_LEVEL);

				new_place->score = ent->client->resp.score;
				strlcpy(new_place->name, ent->client->pers.netname, sizeof(new_place->name));
				snprintf(new_place->date, sizeof(new_place->date), "%02d/%02d/%04d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year);

				new_place->next = place;
				new_place->prev = place->prev;
				if(place->prev)
					place->prev->next = new_place;
				place->prev = new_place;

				if(place == first_place)
					first_place = new_place;

				break;
			}

			if(!place->next)
				break;

			place = place->next;
		}

		if(!new_place) {
			new_place = (place_t *)gi.TagMalloc(sizeof(place_t), TAG_LEVEL);
			new_place->prev = place;
			new_place->next = NULL;

			new_place->score = ent->client->resp.score;
			strlcpy(new_place->name, ent->client->pers.netname, sizeof(new_place->name));
			snprintf(new_place->date, sizeof(new_place->date), "%02d/%02d/%04d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year);

			if(!place)
				first_place = new_place;
			else
				place->next = new_place;
		}

		place = first_place;
		while(place) {
			if(place != new_place && !strcmp(new_place->name, place->name)) {
				if(place->prev)
					place->prev->next = place->next;
				if(place->next)
					place->next->prev = place->prev;
			}

			place = place->next;
		}
	}

	place = first_place;
	if(!place)
		return;

#ifdef WIN32
	_mkdir(file_gamedir("hiscores"));
#else
	mkdir(file_gamedir("hiscores"), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
	file = fopen(Highscores_File(), "wt");
	if(!file)
		return;

	i = 0;
	while(place && i < 10) {
		snprintf(buf, sizeof(buf), "%d;%s;%s\r\n", place->score, place->name, place->date);
		fputs(buf, file);

		place = place->next;

		i++;
	}

	fclose(file);
}

void Highscores_List(edict_t *ent) {
	int i = 0;
	place_t *place = first_place;

	if(!use_highscores->value) {
		gi.cprintf(ent, PRINT_HIGH, "This server isn't using the high scores feature.\n");
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "High scores for %s (in %d min)\n", level.mapname, (int)timelimit->value);

	while(place) {
		gi.cprintf(ent, PRINT_HIGH, " %2d %-16s %3d on %s\n", i++ + 1, place->name, place->score, place->date);
		place = place->next;
	}
}

void Highscores_Scoreboard(char *string, unsigned int strlen, int *down) {
	place_t *place = first_place;
	int sec = (int)(timelimit->value * 60 + empty_time - level.time);

	*down -= 12;

	if(!level.intermissiontime) {
		*down -= 4;
		strlcat(string, va("yv %d xv 0 cstring \"Time left is %d:%02d\" ", *down, sec / 60, sec % 60), strlen);
		*down += 8;
	}

	if(place && place->score > 0) {
		strlcat(string, va("yv %d xv 0 cstring \"High score is %d (in %d min)\" ", *down, place->score, (int)timelimit->value), strlen);
		strlcat(string, va("yv %d xv 0 cstring \"by %s on %s\" ", *down + 8, place->name, place->date), strlen);
	}
	else
		strlcat(string, va("yv %d xv 0 cstring \"No high score for this map\" ", *down), strlen);

	*down += 24;
}

int Highscores_FullScoreboard(edict_t *ent) {
	char string[1024] = "";
	int i = 0;

	place_t *place = first_place;

	strlcat(string, va("xv 0 yv 40 cstring \"High scores for %s (in %d min)\" ", level.mapname, (int)timelimit->value), sizeof(string));
	while(place) {
		i++;
		strlcat(string, va("xv -4 yv %d cstring \"%2d %-16s %3d on %s\" ", 56 + i * 8, i, place->name, place->score, place->date), sizeof(string));
		place = place->next;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);

	return strlen(string);
}
