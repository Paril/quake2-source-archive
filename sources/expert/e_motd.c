#include <stdio.h>
#include "g_local.h"

char *motd;

// InitMOTD: Initializes the MOTD on each level start
void InitMOTD(void)
{
	FILE *motdfile;

	// Open the MOTD
	motdfile = OpenGamedirFile(gamedir->string, va("%s/%s", levelCycle->string, EXPERT_MOTD_FILENAME), "r");

	// If a file was found, copy it over
	if (motdfile)
	{
		char line[256];				// more columns than ever possible
		char *motdbuf = NULL;
		int motdlen = 1, linelen = 0;

//		gi.dprintf("Reading %s..\n", MOTD_FILENAME);

		motdbuf = calloc(1, 1);
		if (motdbuf == NULL)			// Ran out of memory... uh oh
		{
			gi.dprintf("ERROR: Ran out of memory while initializing MOTD. Continuting execution.\n");
			fclose(motdfile);
			return;
		}

		// FIXME: This is probably a terribly inefficient way to set this up, but it works
		while (fgets(line, 256, motdfile))
		{
			linelen = strlen(line);
			motdlen += linelen;
			motdbuf = realloc(motdbuf, motdlen);		// Tagged allocation doesn't have realloc
			if (motdbuf == NULL)			// Ran out of memory... uh oh
			{
				gi.dprintf("ERROR: Ran out of memory while initializing MOTD. Continuting execution.\n");
				if (motdbuf)
					free(motdbuf);
				fclose(motdfile);
				return;
			}
			strncat(motdbuf, line, linelen);
		}
		
		motd = gi.TagMalloc(motdlen, TAG_LEVEL);
		strncpy(motd, motdbuf, motdlen);

		fclose(motdfile);
		free(motdbuf);
	} else {
		// Didn't find the file

//		gi.dprintf("Didn't find %s\n", MOTD_FILENAME);
		motd = NULL;
	}
}

// DisplayMOTD: Display the MOTD to the client
void DisplayMOTD(edict_t *client)
{
	if (motd)
		gi.centerprintf(client, "Expert Quake2 v%s\n"
			"http://www.planetquake.com/expert/\n\n"
			"\"settings\" for settings\n\n%s",
			EXPERT_VERSION, motd);
	else
		gi.centerprintf(client, "Expert Quake2 v%s\n"
			"http://www.planetquake.com/expert/\n",
			EXPERT_VERSION);
}

// DisplayRespawnLine: Displays an info line on respawn
void DisplayRespawnLine(edict_t *client)
{
	gi.cprintf(client, PRINT_MEDIUM, "Expert %s - \"help\" for info\n", GAMESTRING);
}

void DisplaySettings(edict_t *client)
{
	int i;
	qboolean firstPrint = true;

	gi.cprintf(client, PRINT_MEDIUM, "The following options are in effect: ");
	
	for (i = 0; i < NUM_SETTINGS; i++) {
		if (expflags & (1 << i)) {
			if (firstPrint) {
				gi.cprintf(client, PRINT_MEDIUM, "%s", e_bits[i]);
				firstPrint = false;
			} else {
				gi.cprintf(client, PRINT_MEDIUM, ", %s", e_bits[i]);
			}
		}
	}

	gi.cprintf(client, PRINT_MEDIUM, "\n");

	gi.cprintf(client, PRINT_MEDIUM, "Pace multiplier is %.2f\n", pace);
	gi.cprintf(client, PRINT_MEDIUM, "Lethality multiplier is %.2f\n", lethality);
}

