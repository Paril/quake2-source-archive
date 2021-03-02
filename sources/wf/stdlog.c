#include "g_local.h"
#include "stdlog.h"
#include <time.h>

struct mod_to_weapon
{
	char *wname;
	int wid;
};

struct mod_to_weapon m2w[] = {

	{"Unknown",      0},
	{"Blaster", 1},
	{"Shotgun", 2},
	{"Super Shotgun", 3},
	{"Machine Gun", 4},
	{"Chain Gun", 5},
	{"Grenade", 6},
	{"Grenade", 7},
	{"Rocket", 8},
	{"Rocket", 9},
	{"Hyperblaster", 10},
	{"Rail Gun", 11},
	{"BFG", 12},
	{"BFG", 13},
	{"BFG", 14},
	{"Hand Grenade", 15},
	{"Hand Grenade", 16},
	{"Water", 17},
	{"Slime", 18},
	{"Lava", 19},
	{"Crush", 20},
	{"Telefrag", 21},
	{"Falling", 22},
	{"Suicide", 23},
	{"Held Grenade", 24},
	{"Explosive", 25},
	{"Barrel", 26},
	{"Bomb", 27},
	{"Exit", 28},
	{"Splash", 29},
	{"Target Laser", 30},
	{"Trigger Hurt", 31},
	{"Hit", 32},
	{"Target Blaster", 33},
	{"Grapple", 34},
	{"Laserball", 35},
	{"Goodyear Grenade", 36},
	{"Proximity Grenade", 37},
	{"Cluster Grenade", 38},
	{"Pipe Bomb", 39},
	{"Earth Quake", 40},
	{"Reverse Telefrag", 41},
	{"Turret Grenade", 42},
	{"Flame", 43},
	{"NAG Rifle", 44},
	{"Pulse Cannon", 45},
	{"Shrapnel Grenade", 46},
	{"Cluster Rocket", 47},
	{"Plasma Bomb", 48},
	{"Disease", 49},
	{"Sniper Rifle", 50},
	{"Nail Gun", 51},
	{"Spon Human Comb", 52},
	{"Needler", 53},
	{"Concusion Grenade", 54},
	{"Armor Piercing Dart", 55},
	{"Infected Dart", 56},
	{"Napalm Rocket", 57},
	{"Lightning Gun", 58},
	{"Telsa Coil", 59},
	{"Magnotron", 60},
	{"Shock Grenade", 61},
	{"Pellet Grenade", 62},
	{"Flare Gun", 64},
	{"Tranquilizer", 65},
	{"Bolted Blaster", 66},
	{"Sentry", 67},
	{"Long Range Projectile", 68},
	{"Flare", 69},
	{"Kamikazi", 70},
	{"Depot", 71},
	{"Sentry Killer", 72},
	{"Mega Chaingun", 73},
	{"Homing Rocket", 74},
	{"Tranquilizer Dart", 75},
	{"Sniper Rifle - Leg Shot", 76},
	{"Sniper Rifle - Head Shot", 77},
	{"Sentry Rocket", 78},
	{"Knife",79},
	{"Knife In The Back",80},
	{"Healing Depot", 81},
	{"Laser Cutter",82},
	{"Napalm Grenade", 83},
	{"Flamethrower",84},
	{"Bio Sentry",85},
	{"AK47", 86},
	{"Pistol", 87},
	{"Ion Ripper",88},
	{"Phalanx",89},
	{"ETF Rifle",90},
	{"Tesla",91},
	{"Heat Beam",92},
	{"Defender Sphere",93},
	{"Blaster2",94},
	{"Tracker",95},
	{"Nuke",96},
	{"Laser Defense",97},
	{"Gas Grenade", 98},
	{"Stinger", 99},
	{"Missile Launcher", 100},
	{"Camera", 101},
	{"Feign", 102},
	{"Freezer", 103},//acrid 3/99
	{"",9999},
};

void wf_strdate(char *s)
{
	time_t t;
	struct tm *tmptr;

	time(&t);	//get the date/time
	tmptr = localtime(&t);

	//format the date
	sprintf(s, "%2d/%2d/%2d",  tmptr->tm_mon + 1, tmptr->tm_mday, tmptr->tm_year);
}

void wf_strtime(char *s)
{
	time_t t;
	struct tm *tmptr;

	time(&t);	//get the date/time
	tmptr = localtime(&t);

	//format the time
	sprintf(s, "%2d:%2d:%2d",  tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);
}

char *sl_FindWeapon(int mod)
{
	int i;
	int len;

	if (mod < 0) mod = 0;

	len = sizeof(m2w);
	for (i = 0; i < len; ++i)
	{
		if (mod == m2w[i].wid)
			return (m2w[i].wname);
	}
	//return ("UNKNOWN");
	return (NULL);
}

FILE *logfile = NULL;

char *strip9(char *str)
{
    char *p = str;
    if (str == 0) return "";
    while(*p)
    {
        if(*p==9)
            *p = '_';
        p++;
    }   
    return str;
}

void sl_LogPlayerName( game_import_t  *gi,
                  char           *pPlayerName,
                   char           *pTeamName)
{}
                             
void  sl_Logging( game_import_t  *gi, char *pPatchName )
{
    char path[100];
    char str[20];
	char fname[60];
	char fullpath[100];

	logfile = NULL;

	//if (wfflags == NULL) return;

	//Is frag logging on?
    if (((int)wfflags->value & WF_FRAG_LOGGING) == 0)
        return;

    strcpy(path, gamedir->string);

    strcpy(fname, wf_game.stdlog_name);

	if (fname == NULL || fname[0] == 0) strcpy(fname, "std.log");

#if defined(_WIN32) || defined(WIN32)
    //strcat(path,"\\std.log");
	sprintf(fullpath, "%s\\%s", path, fname);
#else
    //strcat(path,"/std.log");
	sprintf(fullpath, "%s/%s", path, fname);
#endif

    if( logfile = fopen(fullpath,"a") )
    {
        fprintf(logfile,"\x9\x9StdLog\x9 1.22\n");
        fprintf(logfile,"\x9\x9PatchName\x9%s\n",strip9(pPatchName));
        wf_strdate( str );
        fprintf(logfile,"\x9\x9LogDate\x9%s\n",str);
        wf_strtime( str );
        fprintf(logfile,"\x9\x9LogTime\x9%s\n",str);
		fflush(logfile);
    }
}


void sl_GameStart( game_import_t    *gi,level_locals_t    level )
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9LogDeathFlags\x9%d\n",dmflags);
        fprintf(logfile,"\x9\x9Map\x9%s\n",strip9(level.level_name));
        fprintf(logfile,"\x9\x9GameStart\x9\x9\x9%f\n",level.time);
		fflush(logfile);
    }
}

void sl_GameEnd( game_import_t    *gi,level_locals_t    level )
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9GameEnd\x9\x9\x9%f\n",level.time);
        fflush(logfile);
    }
}

void sl_LogPlayerConnect( game_import_t *gi,level_locals_t level, edict_t *ent )
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9PlayerConnect\x9%s\x9%s\x9%f\n",
			strip9(ent->client->pers.netname),
			CTFTeamName(ent->client->resp.ctf_team),
			level.time);
		fflush(logfile);
    }
}

void sl_LogPlayerDisconnect( game_import_t *gi,level_locals_t level, edict_t *ent )
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9PlayerDisonnect\x9%s\x9\x9%f\n",
			strip9(ent->client->pers.netname),
			level.time);
		fflush(logfile);
    }
}

void sl_LogPlayerTeamChange( game_import_t *gi, char *pPlayerName, char *pTeamName)
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9PlayerTeamChange\x9%s\x9%s\x9%f\n",
			strip9(pPlayerName),
			strip9(pTeamName),
			level.time);
		fflush(logfile);
    }
}

void sl_LogPlayerRename( game_import_t  *gi, char *pPlayerName, char *pNewName)
{
    if( logfile )
    {
        fprintf(logfile,"\x9\x9PlayerRename\x9%s\x9%s\x9%f\n",
			strip9(pPlayerName),
			strip9(pNewName),
			level.time);
		fflush(logfile);
    }
}



void sl_LogScore( game_import_t  *gi,
                         char           *pKillerName,
                         char           *pTargetName,
                         char           *pScoreType,
                         int            mod,
                         int             iScore)
{
	char *pWeaponName;

    if( !logfile ) return;

	//Check for suicide
	if (strcmp(pScoreType, "Suicide") == 0)
	{
		pWeaponName = sl_FindWeapon(mod);	//Start with these
		switch (mod)
		{
			case MOD_FALLING:
				pWeaponName = "Fell";
				break;
			case MOD_CRUSH:
				pWeaponName = "Crushed";
				break;
			case MOD_WATER:
				pWeaponName = "Drowned";
				break;
			case MOD_SLIME:
				pWeaponName = "Melted";
				break;
			case MOD_LAVA:
				pWeaponName = "Lava";
				break;
			case MOD_BOMB:
			case MOD_EXPLOSIVE:
			case MOD_BARREL:
				pWeaponName = "Explosion";
				break;
			case MOD_TARGET_LASER:
				pWeaponName = "Lasered";
				break;
			case MOD_TARGET_BLASTER:
				pWeaponName = "Blasted";
				break;
		}
	    fprintf(logfile,"%s\x9%s\x9%s\x9%s\x9%d\x9%f\n",
			strip9(pKillerName),
			strip9(pTargetName),
			strip9(pScoreType),
			pWeaponName,
			iScore,
			level.time );
		fflush(logfile);
		return;
	}


	
	//Not suicide - do normal kill 
    if( logfile )
    {
    fprintf(logfile,"%s\x9%s\x9%s\x9%s\x9%d\x9%f\n",
		strip9(pKillerName),
		strip9(pTargetName),
		strip9(pScoreType),
		strip9(sl_FindWeapon(mod)),
		iScore,
		level.time );
	fflush(logfile);
    }
}
