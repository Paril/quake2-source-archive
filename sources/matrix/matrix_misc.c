#include "g_local.h"
#define MOD_FILE				"motd.ini"

void Matrix_MOTD (edict_t *ent)
{
	FILE *motd_file;
	char motd[500];
	char line[80];
	char file_name[256];
	cvar_t *game_dir;

	game_dir = gi.cvar ("game", "", 0);

#ifndef AMIGA
#ifdef UNIX
	sprintf(file_name, "./%s/%s", game_dir->string, MOD_FILE); 
#else
	sprintf(file_name, ".\\%s\\%s", game_dir->string, MOD_FILE);
#endif
#else
	sprintf(file_name,"%s/%s",game_dir->string, MOD_FILE);	
#endif

	if (motd_file = fopen(file_name, "r"))
	{
		// we successfully opened the file "motd.txt"
		if ( fgets(motd, 500, motd_file) )
		{
			// we successfully read a line from "motd.txt" into motd
			// ... read the remaining lines now
			while ( fgets(line, 80, motd_file) )
			{	
				// add each new line to motd, to create a BIG message string.
				// we are using strcat: STRing conCATenation function here.
				strcat(motd, line);
			}
			// print our message.
			gi.centerprintf (ent, motd);
		}
		// be good now ! ... close the file
		fclose(motd_file);
	}
}

void stuffcmd(edict_t *e, char *s) 
{
	gi.WriteByte (11);
	gi.WriteString (s);
	gi.unicast (e, true);
}
	

