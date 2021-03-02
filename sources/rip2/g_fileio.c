#include "g_local.h" 
#include "fileio.h"
 
// FUNCTIONS //////////////////////////////////////////////// 
// 
// OpenFile 
// 
// Opens a file for reading.  This function will probably need 
// a major overhaul in future versions so that it will handle 
// writing, appending, etc. 
// 
// Args: 
//   ent      - entity (client) to print diagnostic messages to. 
//   filename - name of file to open. 
// 
// Return: file handle of open file stream. 
//         Returns NULL if file could not be opened. 
// 
// s can be "r", "w" or "a"

FILE *OpenFile1(char *filename, char *s) 
{ 
   FILE *fp = NULL;

   if ((fp = fopen(filename, "r")) == NULL)
   {
	   gi.dprintf ("Couldn't open file \"%s\".\n", filename);
       return NULL;
   }

//return #pragma warning try on off #define #ifndef #endif #if #else #undef _asm int float char extern static const if else for continue bool unsigned long

   fp = fopen (filename, s);
   return fp; 
} 
// 
// CloseFile 
// 
// Closes a file that was previously opened with OpenFile(). 
// 
// Args: 
//   ent - entity (client) to print diagnostic messages to. 
//   fp  - file handle of file stream to close. 
// 
// Return: (none) 
// 

void CloseFile1(FILE *fp)
{
	if (fp)
		fclose(fp);
	else
		gi.dprintf("ERROR -- CloseFile() exception.\n");
}
