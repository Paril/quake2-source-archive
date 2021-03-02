// SIMPLE CLIENT SIDE STATS SYSTEM
// FIRST DRAFT 28 DECEMBER 2008 VICIOUZ

#include "client.h"

#include "cl_stats.h"

// vars/pointers
FILE * statsfile;
char profilename[128];
time_t time_start, time_end;

stats_t stats;

void Stats_Init()
{
	char filename[256];

	strcpy(profilename, Cvar_Get("menu_profile_file", "unnamed", 0)->string);

	Com_sprintf(filename, sizeof(filename), "%s/profiles/%s.stat", FS_Gamedir(), profilename);

	statsfile = fopen(filename,"r+");

	if(statsfile)
	{
		Stats_LoadFromFile();
		Com_Printf("Loaded stats file %s.stat.\n", profilename);
	}
	else
	{
		statsfile = fopen(filename,"w+");

		if (statsfile)
		{
			Com_Printf("Created local statistics file %s.stat.\n", profilename);
			fprintf(statsfile, "0 0 0 0 0 eof");
		}
		else
		{
			Com_Printf("Error accessing stats file %s.stat.\n", profilename);
			return;
		}
	}
	
	Cmd_AddCommand("localstats", Stats_Query);
	Cmd_AddCommand("clearlocalstats", Stats_Clear);

	time_start = time(NULL);
}

void Stats_Clear(void)
{
	time_start = time(NULL);
	rewind(statsfile);
	fprintf(statsfile, "0 0 0 0 0 eof");
	Stats_LoadFromFile();
}

void Stats_LoadFromFile()
{
	rewind(statsfile);
	fscanf(statsfile, "%d %d %d %d %d eof", &stats.kills, &stats.deaths, &stats.grabs, &stats.caps, &stats.playtime);
}

void Stats_WriteToFile()
{
	rewind(statsfile);
	fprintf(statsfile, "%d %d %d %d %d eof", stats.kills, stats.deaths, stats.grabs, stats.caps, stats.playtime);
}

void Stats_UpdateTime()
{
	time_end = time(NULL);
	stats.playtime = stats.playtime + (time_end - time_start);
	time_start = time(NULL);
}

void Stats_AddEvent(int type)
{
	// this should be better
	if (cl.attractloop)
		return;

	switch (type)
	{
		case STATS_KILL:
			stats.kills++;
			break;
		case STATS_DEATH:
			stats.deaths++;
			break;
		case STATS_GRAB:
			stats.grabs++;
			break;
		case STATS_CAP:
			stats.caps++;
			break;
		default:
			break;
	}
}

void Stats_Query(void)
{
	Stats_UpdateTime();

	// ifs to fix division by zero

	stats.kdratio = (float)stats.kills/(float)stats.deaths;
	if (stats.deaths == 0)
		stats.kdratio = stats.kills;

	stats.gcratio = (float)stats.grabs/(float)stats.caps;
	if (stats.caps == 0)
		stats.gcratio = stats.grabs;

	// this is split up to make it easier to read and change
	Com_Printf("Local Profile Statistics:\n");
	Com_Printf("Profile Name: %s\n", profilename);
	Com_Printf("Kills: %d\n", stats.kills);
	Com_Printf("Deaths: %d\n", stats.deaths);
	Com_Printf("K/D Ratio: %.2f:1\n", stats.kdratio);
	Com_Printf("Flags Grabbed: %d\n", stats.grabs);
	Com_Printf("Flags Captured: %d\n", stats.caps);
	Com_Printf("G/C Ratio: %.2f:1\n", stats.gcratio);
	Com_Printf("Total Play Time: %dh %dm %ds\n", stats.playtime/3600, (stats.playtime%3600)/60, ((stats.playtime%3600)%60));

	Stats_WriteToFile();
}

void Stats_Shutdown()
{
	if (!statsfile)
		return;

	Stats_UpdateTime();
	Stats_WriteToFile();

	stats.kills = 0;
	stats.deaths = 0;
	stats.grabs = 0;
	stats.caps = 0;
	stats.playtime = 0;
	
	fclose(statsfile);
	Cmd_RemoveCommand("localstats");
	Cmd_RemoveCommand("clearlocalstats");
}
