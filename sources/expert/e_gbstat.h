/*
	e_gbstat.h

	Definitions specific to e_gbstat.c file

	Credit is due to:
		GibStat page at: http://www.planetquake.com/gibstat
*/


// giblog Default filename
#define EXPERT_GIB_FILENAME			"gibstats.log"

/* 
	Version 1.2 GibStats Standard
*/

#define GS_VERSION						"1.2"				// GibStats Version

/*	Log Start/Stop Constants		*/

#define GS_LOG_START					"\t\tStdLog\t%s\n"
#define GS_GAME_START					"\t\tGameStart\t\t\t%s\n"
#define GS_GAME_END						"\t\tGameEnd\t\t\t%s\n"
#define GS_PATCH_NAME					"\t\tPatchName\t%s\n"
#define GS_LOG_DATE						"\t\tLogDate\t%02u.%02u.%02u\n"
#define GS_LOG_TIME						"\t\tLogTime\t%02u:%02u:%02u\n"
#define GS_DM_SETTINGS					"\t\tLogDeathFlags\t%u\n"
#define GS_EXPERT_SETTINGS				"\t\tLogExpertFlags\t%u\n"
#define GS_MAP_NAME						"\t\tMap\t%s\n"

/*	Player Info						*/
#define GS_PLAYER_INFO					"\t\tPlayer\t%s\t%s\t%u\n"

#define GS_PLAYER_DROP					"\t\tPlayerLeft\t%s\t\t%u\n"
#define GS_PLAYER_CONNECT				"\t\tPlayerConnect\t%s\t%s\t%u\n"

#define GS_PLAYER_TEAM_CHANGE			"\t\tPlayerTeamChange\t%s\t%s\t%u\n"
#define GS_PLAYER_NAME_CHANGE			"\t\tPlayerRename\t%s\t%s\t%u\n"

#define GS_SCORE						"%s\t%s\t%s\t%s\t%i\t%u\t%u\n"

#define GS_TYPE_KILL					"Kill"
#define GS_TYPE_TEAMKILL				"TeamKill"
#define GS_TYPE_SUICIDE					"Suicide"

#define GS_EMPTY_STRING					""

#define GS_OBSERVER_TEAM				"Observer"

/*	CTF */

#define GS_TYPE_CTF_CAPTURE				"Flag Capture"
#define GS_TYPE_CTF_RETURN				"Flag Return"
#define GS_TYPE_CTF_FLAG_PICKUP			"Flag Pickup"

#define GS_TYPE_CTF_DEFENDER_KILL		"Defender Kill"

#define GS_TYPE_CTF_RETURN_FLAG_ASSIST	"Return Assist"
#define GS_TYPE_CTF_CARRIER_KILL_ASSIST	"Kill Assist"

#define GS_TYPE_CTF_BASE_DEFENSE		"Base Defense"
#define GS_TYPE_CTF_FLAG_HOLDING		"Flag Holding"
#define GS_TYPE_CTF_FLAG_DEFENSE		"Flag Defense"

#define GS_TYPE_CTF_CARRIER_DEFENSE		"Carrier Defense"
#define GS_TYPE_CTF_CARRIER_KILL		"Carrier Kill"
#define GS_TYPE_CTF_CARRIER_SAVE		"Carrier Save"

qboolean gsStartLogging();
qboolean gsStopLogging();

void gsEnumConnectedClients();
void gsPlayerNameChange(char* szOldName, char* szNewName);
void gsTeamChange(char* szOldName, char* szNewName);
void gsLogLevelStart();
void gsLogDate();
void gsGameStart();
void gsGameEnd();
void gsLogClientConnect(edict_t *ent);
void gsLogClientDisconnect(edict_t *ent);
void gsLogFrag(edict_t *vict, edict_t *attk, int cod);
void gsLogKillSelf(edict_t *vict, int cod);
void gsLogScore(edict_t *player, char *scoreName, int scoreAmount);
void gsLogMisc(char *logEvent);
