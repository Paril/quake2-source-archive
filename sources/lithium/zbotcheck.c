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

#define ZBOT_JITTERMAX	4
#define ZBOT_JITTERTIME	10
#define ZBOT_JITTERMOVE 500

char *_ = "\n\n    ZbotCheck v1.01 for Quake 2 by\
 Matt \"WhiteFang\" Ayres (matt@lithium.com)\n\n";

#ifdef _DEBUG
void LogJitter(edict_t *ent) {
	FILE *file = fopen(file_gamedir("jitter.log"), "at");
	client_respawn_t *resp = &ent->client->resp;
	fprintf(file, "  [%d] %s (%d, %d) (%d, %d)\n", level.framenum, ent->client->pers.netname,
		resp->angles[0][0], resp->angles[0][1],
		resp->angles[1][0], resp->angles[1][1]);
	fclose(file);
}

void LogJitter2(edict_t *ent) {
	FILE *file = fopen(file_gamedir("jitter.log"), "at");
	client_respawn_t *resp = &ent->client->resp;
	fprintf(file, "* [%d] %s (%d, %d) (%d, %d)\n", level.framenum, ent->client->pers.netname,
		resp->angles[0][0], resp->angles[0][1],
		resp->angles[1][0], resp->angles[1][1]);
	fclose(file);
}
#endif

qboolean ZbotCheck(edict_t *ent, usercmd_t *ucmd) {
	int tog0, tog1;
	client_respawn_t *resp = &ent->client->resp;

	tog0 = resp->tog;
	resp->tog ^= 1;
	tog1 = resp->tog;

	if(ucmd->angles[0] == resp->angles[tog1][0] &&
		ucmd->angles[1] == resp->angles[tog1][1] &&
		ucmd->angles[0] != resp->angles[tog0][0] &&
		ucmd->angles[1] != resp->angles[tog0][1] &&
		abs(ucmd->angles[0] - resp->angles[tog0][0]) +
		abs(ucmd->angles[1] - resp->angles[tog0][1]) >= ZBOT_JITTERMOVE) {
		
			if(level.time <= resp->jitter_last + 0.1) {
				if(!resp->jitter)
					resp->jitter_time = level.time;
				resp->jitter++;

#ifdef _DEBUG
				LogJitter2(ent);
#endif

				if(resp->jitter == ZBOT_JITTERMAX)
					return true;
			}
#ifdef _DEBUG
		else
			LogJitter(ent);
#endif

		resp->jitter_last = level.time;
	}

	resp->angles[tog1][0] = ucmd->angles[0];
	resp->angles[tog1][1] = ucmd->angles[1];

	if(level.time > resp->jitter_time + ZBOT_JITTERTIME)
		resp->jitter = 0;

	return false;
}
