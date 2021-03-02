#include "g_local.h" 

char *ReadEntFile(char *filename) 
{ 
	FILE      *fp; 
	char      *filestring = NULL; 
	long int   i = 0; 
	int         ch; 
	
	for (;;) 
	{ 
		fp = fopen(filename, "r"); 
		if (!fp) break; 
		
		for (i=0; (ch = fgetc(fp)) != EOF; i++); 
		
		filestring = gi.TagMalloc(i+1, TAG_LEVEL); 
		if (!filestring) break; 
		
		fseek(fp, 0, SEEK_SET); 
		for (i=0; (ch = fgetc(fp)) != EOF; i++) 
			filestring[i] = (char) ch; 
		filestring[i] = '\0'; 
		break; 
	} 
	
	if (fp) fclose(fp); 
	return(filestring); 
} 

char *LoadEntFileOld(char *mapname, char *entities) 
{ 
	char   entfilename[MAX_QPATH] = ""; 
	char   *newentities; 
	int      i; 
    
	sprintf(entfilename, "iwm/ent/%s.ent", mapname); 
	// convert string to all lowercase (for Linux) 
	for (i = 0; entfilename[i]; i++) 
		entfilename[i] = (char) tolower(entfilename[i]); 
	
	newentities = ReadEntFile(entfilename); 
	
	if (newentities) 
	{   //leave these dprints active they show up in the server init console section 
		gi.dprintf(".ent file found\n", mapname); 
		return(newentities);   // reassign the ents 
	}
	else 
	{ 
		gi.dprintf("No .ent File for %s.bsp\n", mapname); 
		return(entities); 
	} 
} 

char *LoadEntFile(char *mapname, char *entities) 
{ 
	char   entfilename[MAX_QPATH] = ""; 
	char   *newentities; 
	int      i; 
//    mkdir ("iwm/cmo/");

	sprintf(entfilename, "iwm/cmo/%s.cmo", mapname); 
	// convert string to all lowercase (for Linux) 
	for (i = 0; entfilename[i]; i++) 
		entfilename[i] = (char) tolower(entfilename[i]); 
	
	newentities = ReadEntFile(entfilename); 
	
	if (newentities) 
	{   //leave these dprints active they show up in the server init console section 
		gi.dprintf("Complete Map Overwrite file %s.cmo found\n", mapname); 
		return newentities;   // reassign the ents 
	}
	else 
	{ 
		gi.dprintf("No .cmo File for %s.bsp\n", mapname); 
		return entities; 
	} 
} 
