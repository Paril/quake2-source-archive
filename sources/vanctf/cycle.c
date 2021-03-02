#include "g_local.h"
#include "fileio.h"
#include "cycle.h"

char *cycle_get_next_map(char *currentmap)
{
	char	*linebegin, *lineend;
	map_t	*newmapt;
	map_t	*walk;
	map_t	*maphead = NULL;
	map_t	*maptail = NULL;

	// load up the map list
	linebegin = ReadTextFile(va("%s/maps.lst", gamecvar->string));
	if (!linebegin)
		return (NULL);						// couldn't load file

	while (1)
	{
		// find end of line and terminate string there
		lineend = strchr(linebegin, '\n');
		if (lineend)
			*lineend = '\0';
		if (linebegin[0] == '\0')			// stop on blank line
			break;

		// make new linked list entry
		// (mem is freed on level change automatically)
		newmapt = gi.TagMalloc(sizeof(map_t), TAG_LEVEL);
		newmapt->next = NULL;
		newmapt->mapname = linebegin;

		// link it into the list
		if (!maphead)
		{
			maphead = newmapt;
		}
		else
		{
			maptail->next = newmapt;
		}
		maptail = newmapt;
		
		// if we truncated the string, then the start of the
		// next line is one beyond that.  otherwise, we
		// just processed the last line, so we are done
		if (lineend)
			linebegin = lineend + 1;
		else
			break;						// stop if last line
	}


/*
	fp = fopen("vanctf/maplist.txt", "r");
	if (!fp)
		return (NULL);
	
	while (!feof(fp))
	{
		newmapt = gi.TagMalloc(sizeof(map_t), TAG_LEVEL);
		newmapt->mapname = gi.TagMalloc(MAX_QPATH, TAG_LEVEL);
		newmapt->next = NULL;
		fscanf(fp, " %s", newmapt->mapname);
		if (!newmapt->mapname[0])
			break;		// stop on blank line
		if (!maphead)
		{
			maphead = newmapt;
		}
		else
		{
			maptail->next = newmapt;
		}
		maptail = newmapt;
	}
	fclose (fp);
*/

	// look for entry of current level
	for (walk = maphead; walk; walk = walk->next)
	{
		if (Q_strcasecmp(walk->mapname, currentmap) == 0)
			break;
	}
	
	// if we found an entry and it is not the last one
	// then return the name of the next level
	if (walk && walk->next)
		return (walk->next->mapname);
	
	// either we were on a level not in the list or the
	// last level in the list.  either way, just start at
	// the beginning of the list
	if (maphead)
		return(maphead->mapname);
	
	return(NULL);		// couldn't load file?
}
