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
#include "fragtrak.h"

#if 1
void FragTrak_InitLevel(void) { }
void FragTrak_Frag(edict_t *killer, edict_t *killee) { }
void FragTrak_Save(void) { }
void FragTrak_ClientBegin(edict_t *ent) { }
void FragTrak_ClientDisconnect(edict_t *ent) { }
#else

edict_t *trak_ent[256];

trak_header_t trak_header;
trak_t trak[256];
frag_t frag[2048];

trak_time_t GetTime(void) {
	trak_time_t time;
	char strtime[16];
	char strdate[16];

#ifdef _WINDOWS
	_strtime(strtime);
	sscanf(strtime, "%d:%d:%d", &time.hour, &time.minute, &time.second);
	_strdate(strdate);
	sscanf(strdate, "%d/%d/%d", &time.month, &time.day, &time.year);
#endif

	return time;
}

void FragTrak_InitLevel(void) {
	strlcpy(trak_header.magic, "TRAK", 5);
	trak_header.version = 1;
	strlcpy(trak_header.mapname, level.mapname, sizeof(trak_header.mapname));
	trak_header.begin = GetTime();
	trak_header.traks = 0;
	trak_header.frags = 0;
}

void FragTrak_Frag(edict_t *killer, edict_t *killee) {
	// if killed by unknown, set killer to -1
	if(killer->client)
		frag[trak_header.frags].killer = killer->trak_num;
	else
		frag[trak_header.frags].killer = -1;

	// if killed by self by camping, set killee to -1
	if(killer == killee && killer->client->decamp_count && killer->client->decamp_fire)
		frag[trak_header.frags].killee = -1;
	else
		frag[trak_header.frags].killee = killee->trak_num;

	trak_header.frags++;
}

void FragTrak_Save(void) {
	FILE *file;
	char filename[32];
	int i;

	trak_header.end = GetTime();

	file = fopen("fragtrak\\doit", "rb");
	if(!file) return;
	fclose(file);

	for(i = 0; i < trak_header.traks; i++) {
		if(!trak[i].end) {
			trak[i].end = level.time;
			//trak_ent[i] can be NULL here BUG
			trak[trak_ent[i]->trak_num].ping_total = trak_ent[i]->ping_total;
			trak[trak_ent[i]->trak_num].ping_count = trak_ent[i]->ping_count;
		}
	}

	snprintf(filename, sizeof(filename), "fragtrak\\%02d%02d%02d.trk", trak_header.begin.hour,
		trak_header.begin.minute, trak_header.begin.second);

	file = fopen(filename, "wb");
	fwrite(&trak_header, sizeof(trak_header_t), 1, file);
	fwrite(trak, sizeof(trak_t), trak_header.traks, file);
	fwrite(frag, sizeof(frag_t), trak_header.frags, file);
	fclose(file);
}

void FragTrak_ClientBegin(edict_t *ent) {
	strlcpy(trak[trak_header.traks].netname, ent->client->pers.netname, sizeof(trak[trak_header.traks].netname));
	trak[trak_header.traks].begin = level.time;
	trak[trak_header.traks].end = 0;
	trak_ent[trak_header.traks] = ent;
	ent->trak_num = trak_header.traks++;
}

void FragTrak_ClientDisconnect(edict_t *ent) {
	trak[ent->trak_num].end = level.time;
	trak[ent->trak_num].ping_total = ent->ping_total;
	trak[ent->trak_num].ping_count = ent->ping_count;
}

#endif
