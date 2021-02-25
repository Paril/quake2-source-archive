// CUSTOM SOURCE FILE

#include "g_local.h"

void gsutil_centerprint(edict_t *ent, char *format, char *text)
{
	if (!ent->client)
		return;

	if (ent->client->showmenu)
		return;

	if (strcmp(ent->classname, "player"))
		return;
	else
		gi.centerprintf (ent, format, text);		
}

void embolden( char *string ) 
{
   int i, len = strlen( string );

   for (i = 0; i < len-1; i++)
      string[i] += 128;
}
