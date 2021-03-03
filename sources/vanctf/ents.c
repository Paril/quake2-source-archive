#include "g_local.h"
#include "fileio.h"
#include "ents.h"

char *LoadEntities(char *mapname, char *entities)
{
	char	entfilename[MAX_QPATH] = "";
	char	*newentities;
	int		i; //, islefn;
	
	sprintf(entfilename, "%s/maps/%s.ent", gamecvar->string, mapname);
	// convert string to all lowercase (for Linux)
	for (i = 0; entfilename[i]; i++)
		entfilename[i] = tolower(entfilename[i]);

	newentities = ReadTextFile(entfilename);

	if (newentities)
		return(newentities);	// reassign the ents
	else
		return(entities);

}
