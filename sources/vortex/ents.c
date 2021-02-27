#include "g_local.h"
#include <sys/stat.h>
#include <ctype.h>

static int ReadFromFile(FILE *fp, char *buffer)
{
	int i, ch;

	for (i = 0; (ch = fgetc(fp)) != EOF; i++)
	{
		if (ch == '\r')
		{
			ch = fgetc(fp);
			if (ch != EOF && ch != '\n')
				ungetc(ch, fp);
			ch = '\n';
		}
		if (buffer)
			buffer[i] = ch;
	}
	if (buffer)
		buffer[i] = '\0';
	return (i+1);
}


// return is only good thru end of level.
// using fgetc() since it auto-converts CRLF pairs
char *ReadTextFile(char *filename) {

	FILE		*fp;
	char		*filestring = NULL;
	long int	i = 0;
	struct stat mstat;

	if (stat(filename, &mstat) != 0)
		return NULL;

	while (true) {
		fp = fopen(filename, "rb");
		if (!fp) break;

		i = ReadFromFile(fp, NULL);
		filestring = V_Malloc(i, TAG_LEVEL);
		if (!filestring)
			break;

		fseek(fp, 0, SEEK_SET);
		ReadFromFile(fp, filestring);

		break;
	}

	if (fp) fclose(fp);

	return(filestring);	// return new text
}

void doentityfilename(char* line, char* mapname, qboolean stuff)
{
	cvar_t	*game_dir;
	int i;

	game_dir = gi.cvar ("game", "", 0);

	if (invasion->value > 1)
	{
		if (!stuff)
			sprintf(line, "%s/maps/%s.ent", game_dir->string, mapname);
		else
			sprintf(line, "%s/maps/%s.hnt", game_dir->string, mapname); // invasion hard mode ent
	}else
		sprintf(line, "%s/maps/%s.ent", game_dir->string, mapname); // by default only use this ent

	// convert string to all lowercase (for Linux)
	for (i = 0; line[i]; i++)
		line[i] = tolower(line[i]);
}

char *LoadEntities(char *mapname, char *entities)
{
	char	entfilename[MAX_QPATH] = "";
	char	*newentities;
	
	doentityfilename(entfilename, mapname, true); // try giving me a inv. hard mode entity.

	newentities = ReadTextFile(entfilename);

	if (newentities)
		return(newentities);	// reassign the ents
	else
	{
		doentityfilename(entfilename, mapname, false); // try giving me a normal ent
		newentities = ReadTextFile(entfilename);

		if (newentities)
			return(newentities); // not found yet? k go back
		else
			return (entities);
	}

}
