#include "g_local.h"
#include <sys/stat.h>

// Geoff roxxxx
void stripem(char *p, char *s)
{
	while (p[0])
	{
		if (p[0] == ':' || p[0] == '/')
			p++;
		else
			*s++ = *p++;
	}
	s[0] = '\0'; // terminate the resulting string
}

#include <stdio.h>
//#include <direct.h>

void InitLogging ()
{
	char *daystring;
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	char *tempst;
	char buffer2[6];
	
	gi.dprintf ("Initializing logging system...\n");

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	strftime (buffer, 80, "%x_%X",timeinfo);

	daystring = gi.TagMalloc (strlen(buffer)*4, TAG_GAME);
	tempst = gi.TagMalloc (strlen(buffer)*4, TAG_GAME);

	sprintf (daystring, "%s_iwmlog.txt", buffer);
	stripem (daystring, tempst);

	strftime (buffer2, 8, "%Y",timeinfo);
	mkdir ("iwm\\logs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir (va("iwm\\logs\\%s", buffer2), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	sprintf (daystring, "iwm\\logs\\%s\\%s", buffer2, tempst);

	filpt = fopen (daystring, "a+");
	if (!filpt)
	{
		gi.dprintf ("Stream initialization failed!\n");
		return;
	}

	gi.dprintf ("Stream initialization complete, writing starting info...\n");
	fprintf (filpt, "== Logging started ==\n", timeinfo);
}

void Log_Print (char *print)
{
	fprintf (filpt, "%s", print);
}