/*
	e_gbstat.c

	Implementation of GibStat logging specification for Expert Server

	GibStat page at: http://www.planetquake.com/gibstat
*/

#include <stdlib.h>
#include <stdio.h>
#include "g_local.h"

// Only these functions need to have access to the file pointer.
static FILE		*gsFile;

// Translation table from "cause of death" index to official Gibstats 1.2
// names of weapons and other causes of death
char gsCauseTable[35][20] = 
{
"Unknown",
"Blaster",
"Shotgun",
"Super Shotgun",
"Machinegun",
"Chaingun",
"Grenade Launcher",
"Grenade Launcher",
"Rocket Launcher",
"Rocket Launcher",
"Hyperblaster",
"Railgun",
"BFG10K",
"BFG10K",
"BFG10K",
"Hand Grenade",
"Hand Grenade",
"Drowned",
"Slime",
"Lava",
"Squished",
"Telefrag",
"Fell",
"Kill Command",
"Hand Grenade",
"Blown Up",
"Blown Up",
"Blown Up",
"Exit",
"Blown Up",
"Laser Trap",
"Trap",
"Trap",
"Blaster Trap",
"Hook"
};

//
// Start/Stop logging functions
// Note gsStartLogging is safe to call repeatedly
//
qboolean gsStartLogging()
{
	char filename[1000];
	time_t curTime;
	struct tm *now;

	// File is open
	if (gsFile != NULL)
		return true;

	if (strlen(sv_giblog->string) == 0) {
		gi.dprintf("The cvar \"giblog\", which tells the server the name of the file "
				"in which to write the gibstats log, is blank.  Disabling Gibstats\n", 
				GS_VERSION);
		return false;
	}

	time (&curTime);			// Get current date/time
	now = localtime(&curTime);	// Convert it to local time

	// Create the filename.
	strcpy(filename, gamedir->string);
	strcat(filename, "/");
	strcat(filename, sv_giblog->string);
	if (utilflags & EXPERT_GIBSTAT_PER_GAME) {
		// add the date, mapname, and current name to the filename 
		strcat(filename, va(".%02u.%02u.%02u.%s.%02u.%02u.%02u", now->tm_year, now->tm_mon + 1,
				now->tm_mday, level.mapname, now->tm_hour, now->tm_min, now->tm_sec));
		// wipe out any prexisting files
		gsFile = fopen(filename, "w");
	} else {
		// We are writing to the same log as last level, so open in 
		// append mode so as not to delete any previous logs.
		gsFile = fopen(filename, "a+");
	}

	if (gsFile == NULL)
	{
		gi.dprintf("***************************\n");
		gi.dprintf("ERROR: Couldn't open %s (GibStat).\n", filename);
		gi.dprintf("***************************\n");
		return false;
	}

	gi.dprintf("Started GibStats %s Logging...\n", GS_VERSION);
	return true;
}

qboolean gsStopLogging()
{

	if (gsFile == NULL)
		return true;

	if (fclose(gsFile)) {
		return false;
	}

	gsFile = NULL;
	gi.dprintf("Stopped GibStats %s Logging...\n", GS_VERSION);

	return true;
}

void gsLogDate()
{
	time_t curTime;
	struct tm *now;

	if (gsFile == NULL)
		return;

	time (&curTime);			// Get current date/time
	now = localtime(&curTime);	// Convert it to local time

	// Create the time/date log entries
	fprintf(gsFile, GS_LOG_DATE,	now->tm_mday, 
									now->tm_mon + 1,
									now->tm_year);
	fprintf(gsFile, GS_LOG_TIME,	now->tm_hour,
									now->tm_min,
									now->tm_sec);
}

//
// Game Start/Game End logs
//

void gsEnumConnectedClients()
{
	int i = 0;
	edict_t *ent;

	if (gsFile == NULL)
		return;

	ent = g_edicts + 1;
	for (i = 1; i <= maxclients->value; i++) {
		if (ent->inuse && ent->client) {
			fprintf(gsFile, GS_PLAYER_INFO,
							ent->client->pers.netname,
							expflags & EXPERT_ENFORCED_TEAMS ? 
								nameForTeam(ent->client->resp.team) : 
								GS_EMPTY_STRING,
							(int)level.time);
		}
		ent = ent + 1;
	}
}

void gsPlayerNameChange(char* szOldName, char* szNewName)
{
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_PLAYER_NAME_CHANGE,
					szOldName,
					szNewName,
					(int)level.time);
}

void gsTeamChange(char *playerName, char *newTeamName)
{
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_PLAYER_TEAM_CHANGE,
					playerName,
					newTeamName,
					(int)level.time);
}

void gsLogLevelStart()
{
	if (gsFile == NULL)
		return;

	// GS 1.2: Log Start Added.
	fprintf(gsFile, GS_LOG_START, GS_VERSION);
	fprintf(gsFile, GS_PATCH_NAME, va("Expert %s", GAMESTRING));
	// Log the level name
	fprintf(gsFile, GS_MAP_NAME, level.level_name);
	// Log the DM Flags setting
	fprintf(gsFile, GS_DM_SETTINGS, (int)dmflags->value);
	// Log the Expert Flags setting
	fprintf(gsFile, GS_EXPERT_SETTINGS, expflags);
	// Log the Data
	gsLogDate();

}

// gsGameStart/End: Markers to ignore all scoring before the
// official start of the game/ end of the game
/*
void gsGameStart()
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_GAME_START);
}
void gsGameEnd()
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_GAME_END);
}
*/

//
// Map and Client Connect/Disconnect logging.
//

void gsLogClientConnect(edict_t *ent)
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	if (expflags & EXPERT_ENFORCED_TEAMS)
	{	
		fprintf(gsFile, GS_PLAYER_CONNECT, 
						ent->client->pers.netname, 
						nameForTeam(ent->client->resp.team),
						(int)level.time);
	} else {
		fprintf(gsFile, GS_PLAYER_CONNECT, 
						ent->client->pers.netname, 
						GS_EMPTY_STRING, (int)level.time);
	}
}

void gsLogClientDisconnect(edict_t *ent)
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_PLAYER_DROP, 
					ent->client->pers.netname, 
					(int)level.time);
}

//
// Frag Logging
//

void gsLogFrag(edict_t *vict, edict_t *attk, int cod)
{
	char *killType;
	int cause;

	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	if (cod & MOD_FRIENDLY_FIRE) {
		killType = GS_TYPE_TEAMKILL;
		cause = cod & ~MOD_FRIENDLY_FIRE;
	} else {
		killType = GS_TYPE_KILL;
		cause = cod;
	}

	fprintf(gsFile,	GS_SCORE,
			attk->client->pers.netname,
			vict->client->pers.netname,
			killType,
			gsCauseTable[cause],
			1,
			(int)level.time,
			attk->client->ping);
}

void gsLogKillSelf(edict_t *vict, int cod)
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	if (cod & MOD_FRIENDLY_FIRE)
		cod = cod & ~MOD_FRIENDLY_FIRE;

	fprintf(gsFile, GS_SCORE,
			vict->client->pers.netname,
			GS_EMPTY_STRING,
			GS_TYPE_SUICIDE,
			gsCauseTable[cod],
			-1,
			(int)level.time,
			vict->client->ping);
}

// Log bonuses
void gsLogScore(edict_t *player, char *scoreName, int scoreAmount)
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	fprintf(gsFile, GS_SCORE,
			player->client->pers.netname,
			GS_EMPTY_STRING,
			scoreName,
			GS_EMPTY_STRING,
			scoreAmount,
			(int)level.time,
			player->client->ping);
}

//
// Roll your own special stuff!
// USE WITH CAUTION! Improper formatting could totally screw up the log.
//

void gsLogMisc(char *logEvent)
{
	// Don't write to an unopened file.
	if (gsFile == NULL)
		return;

	fprintf(gsFile, logEvent);
}






