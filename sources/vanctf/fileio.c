#include "g_local.h"
#include "fileio.h"

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
	int			ch;

	while (true) {
		fp = fopen(filename, "rb");
		if (!fp) break;

		i = ReadFromFile(fp, NULL);
		filestring = gi.TagMalloc(i, TAG_LEVEL);
		if (!filestring)
			break;

		fseek(fp, 0, SEEK_SET);
		ReadFromFile(fp, filestring);

		break;
	}

	if (fp) fclose(fp);

	return(filestring);	// return new text
}
