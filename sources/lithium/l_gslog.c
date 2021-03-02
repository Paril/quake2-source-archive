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

FILE *gslog_file = NULL;

lvar_t *use_gslog;
lvar_t *gslog;
lvar_t *gslog_flush;

void GSLog(char *format, ...) {
	va_list argptr;
	char buf[256];

	if(!gslog_file)
		return;

	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);

#ifdef WIN32
	strlcat(buf, "\n", sizeof(buf));
#else
	strlcat(buf, "\r\n", sizeof(buf));
#endif

	fprintf(gslog_file, "%s", buf);

	if(gslog_flush->value)
		fflush(gslog_file);
}

void GSLog_InitGame(void) {
	use_gslog = lvar("use_gslog", "0", "^", VAR_USE);
	gslog = lvar("gslog", "gslog.log", "str", VAR_NONE);
	gslog_flush = lvar("gslog_flush", "0", "^", VAR_OTHER);
}

#include "time.h"

char *monthname[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void GSLog_InitLevel(void) {
	time_t t;
	struct tm *tm;

	if(!use_gslog->value)
		return;

	gslog_file = fopen(file_gamedir(gslog->string), "at");

	GSLog("\t\tStdLog\t1.2");
	GSLog("\t\tPatchName\tLithium II");

	time(&t);
	tm = localtime(&t);
	GSLog("\t\tLogDate\t%d %s %04d", tm->tm_mday, monthname[MIN(MAX(tm->tm_mon, 0), 12)], 1900 + tm->tm_year);
	GSLog("\t\tLogTime\t%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

	GSLog("\t\tLogDeathFlags\t%d", (int)dmflags->value);

	GSLog("\t\tMap\t%s", level.level_name);
	GSLog("\t\tGameStart\t\t\t%d", (int)level.time);
}

void GSLog_ExitLevel(void) {
	if(!gslog_file)
		return;
	GSLog("\t\tGameEnd\t\t\t%d", (int)level.time);
	fclose(gslog_file);
	gslog_file = NULL;
}

void GSLog_Shutdown(void) {
	GSLog_ExitLevel();
}

void GSLog_ClientBegin(edict_t *ent) {
	GSLog("\t\tPlayerConnect\t%s\t\t%d", ent->client->pers.netname, (int)level.time);
}

void GSLog_ClientDisconnect(edict_t *ent) {
	if(ent->client)
		GSLog("\t\tPlayerLeft\t%s\t\t%d", ent->client->pers.netname, (int)level.time);
}

void GSLog_PlayerRename(char *oldname, char *newname) {
	if(!strcmp(oldname, newname) || !strlen(oldname))
		return;

	GSLog("\t\tPlayerRename\t%s\t%s\t%d", oldname, newname, (int)level.time);
}

void GSLog_Score(char *attacker, char *target, char *type, char *weapon, int score, int time, int ping) {
	GSLog("%s\t%s\t%s\t%s\t%d\t%d\t%d", attacker, target, type, weapon, score, time, ping);
}

void GSLog_Bonus(edict_t *ent, char *type, int score) {
	GSLog_Score(ent->client->pers.netname, "", type, "", score, (int)level.time, ent->client->ping);
}
