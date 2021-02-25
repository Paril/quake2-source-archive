// CUSTOM SOURCE FILE

#include "g_local.h"
#include "botsfile.h"

// return is only good thru end of level.
// need to recode this thing.  its a mess
char *ReadTextFile(char *filename) {

	FILE	*fp;
	char	*filestring = NULL;
	int		i, ch;

	while (true) {
		fp = fopen(filename, "r");
		if (!fp) break;

		for (i=0; (ch = fgetc(fp)) != EOF; i++)
		;

		filestring = gi.TagMalloc(i+1, TAG_LEVEL);
		if (!filestring) break;

		fseek(fp, 0, 0);
		for (i=0; (ch = fgetc(fp)) != EOF; i++)
			filestring[i] = ch;
		filestring[i] = '\0'; // just in case

		break;
	}

	if (fp) fclose(fp);

	return(filestring);	// return new text

}
