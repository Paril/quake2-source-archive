// 
// fileio.c -- file access routines 
// 
// 1/98 - L. Allan Campbell (Geist) 
// 

// INCLUDES ///////////////////////////////////////////////// 

#include "g_local.h" 

     

// FUNCTIONS //////////////////////////////////////////////// 

// 
// WFOpenFile 
// 
// Opens a file for reading.  This function will probably need 
// a major overhaul in future versions so that it will handle 
// writing, appending, etc. 
// 
// Args: 
//   ent      - entity (client) to print diagnostic messages to. 
//   filename - name of file to open. 
// 
// Return: file handle of open file stream. 
//         Returns NULL if file could not be opened. 
// 
FILE *WFOpenFile(edict_t *ent, char *filename) 
{ 
        FILE *fp = NULL; 

        if ((fp = fopen(filename, "r")) == NULL)       // test to see if file opened 
        { 
           // file did not load 
           gi.dprintf ("Could not open file \"%s\".\n", filename); 
           return NULL; 
        } 

        return fp; 
} 
       

// 
// WFCloseFile 
// 
// Closes a file that was previously opened with WFOpenFile(). 
// 
// Args: 
//   ent - entity (client) to print diagnostic messages to. 
//   fp  - file handle of file stream to close. 
// 
// Return: (none) 
// 
void WFCloseFile(edict_t *ent, FILE *fp) 
{ 
        if (fp)        // if the file is open 
        { 
           fclose(fp); 
        } 
        else    // no file is opened 
           gi.dprintf("ERROR -- WFCloseFile() exception.\n"); 
}


// FUNCTIONS //////////////////////////////////////////////// 

void fixline(char *str)
{
	int i;
	i = 0;
	while (str[i] != 0)
	{
		if (str[i] >=128) str[i] = 0;
		if (str[i] < 32 && str[i] != '[' && str[i] != ']') str[i] = 0;
//gi.dprintf("<%d>", str[i]);
		++i;
	}
}

//Convert string to lower case
void lcase(char *str)
{
	int i;
	i = 0;
	while (str[i] != 0)
	{
		
		str[i] = tolower(str[i]);
		++i;
	}
}

//Read a line from a file
void readline(FILE *file, char *str, int max)
{
	int i;
	int enough;
	unsigned char c;

	i = 0;
	enough = 0;
	while (i < max && !feof(file) && enough == 0)
	{
		c = fgetc(file);
		if (c >= ' ') str[i++] = c;
		if (c == 13 || c == 10) enough = 1;
//gi.dprintf("{%d}",c);

	}
	str[i] = 0;

	--i;
	//now trim back the white space
	while (i >= 0 && str[i] == ' ')
	{
		str[i] = 0;
		--i;
	}
//gi.dprintf("ReadLine: [%s]\n", str);
}

void OldReadLine( FILE *fp, char *buffer, int maxlen)
{
	int curlen;
	char c;

	curlen = 0;

	//Read two lines from file
	while (!feof(fp))
	{
		c = fgetc(fp);
//gi.dprintf("{%d}",c);

		if ((c == 13) || (c == 10)) break;
		if (curlen > maxlen) break;

		*buffer = c;
		++buffer;
		++curlen;
	}
	*buffer = '\0';
}
