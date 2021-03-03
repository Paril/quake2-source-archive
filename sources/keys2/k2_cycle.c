/*
	Level cycling via a text file in the keys2 directory
	
	levels.txt
		- File begins with [maplist]
		- File ends with [end]
		- each level has to be on a single line
	
*/
#include "g_local.h"

#define MAX_MAPS  256
#define MAX_MAPNAME_LEN  16 

char *K2_GetDMLevelSettings(char *level);
char *K2_GetCTFLevelSettings(char *level);

// ******** LEVELCYCLE globals ***********
char Levels[MAX_MAPS][MAX_MAPNAME_LEN];
char CTFLevels[MAX_MAPS][MAX_MAPNAME_LEN];
int iCurrLevel;
int iCurrCTFLevel;
int iTotalLevels;
int iTotalCTFLevels;
int iLevelsDone[MAX_MAPS];
int iCTFLevelsDone[MAX_MAPS];

// ******** LEVELCYCLE ***********

char *GetNextLevel ( void )
{
	if ( iCurrLevel == iTotalLevels  ) {
		iCurrLevel = 0;
	}
	else {
		iCurrLevel++;
	}
	return K2_GetDMLevelSettings(Levels[iCurrLevel]);
	
}

char *GetNextCTFLevel ( void )
{
	if ( iCurrCTFLevel == iTotalCTFLevels  ) {
		iCurrCTFLevel = 0;
	}
	else {
		iCurrCTFLevel++;
	}
	return K2_GetCTFLevelSettings(CTFLevels[iCurrCTFLevel]);
	
}


// add this to the EndDMLevel function in g_main.c
qboolean K2_CheckLevelCycle ( void ) 
{
	edict_t		*ent = NULL;
	char	*nextmap = NULL;

	if (!levelcycle->value)
		return false;
	else 
	{ 
		//Check resetlevels var
		if(resetlevels->value)
		{
			gi.dprintf("Reading new map list.\n");
			if(ctf->value)
				K2_ReadCTFLevelCycleFile();
			else
				K2_ReadDMLevelCycleFile();
			gi.cvar_set("resetlevels","0");
		}
				
		if(ctf->value)
			nextmap = GetNextCTFLevel();
		else
			nextmap = GetNextLevel();
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = nextmap; 
		BeginIntermission (ent);
		return true;
	}
	
}

void K2_ReadDMLevelCycleFile(void) 
{ 
    FILE *fp; 
    int  i=0, j=0, k=0, length=0;
	fpos_t pos;
    char szLineIn[80]="";
	char filename[50]="";
    cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
		
	gi.dprintf("DM Level cycling is ON\n");

	sprintf ( filename, "%s/k2maps.txt", gamed->string );

	fp = fopen(filename,"rt"); 

	if (fp)  // opened successfully? 
    {
		
		//Check for DMBEGIN
        do 
        { 
             fscanf(fp, "%s", szLineIn); 
			
        } while (!feof(fp) && Q_stricmp(szLineIn,"[dmbegin]"));
		
		if (feof(fp)) 
        { 
             // no DM section
             gi.dprintf ("-------------------------------------\n"); 
             gi.dprintf ("ERROR - No maps listed in DM Section in \"%s\".\n", filename); 
			 gi.dprintf ("Level Cycling DISABLED\n\n");
			 gi.cvar_forceset("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 
			 fclose(fp); 
			  
			 return;
		         
        } 

		fgetpos(fp,&pos);
        
		
		// scan for a colon
        do 
        { 
             fscanf(fp, "%s", szLineIn); 
        } while (!feof(fp) && !strchr(szLineIn,':') && Q_stricmp(szLineIn,"[dmend]"));

        if (feof(fp)) 
        { 
             // no colon = no maps!
             gi.dprintf ("-------------------------------------\n"); 
             gi.dprintf ("ERROR - No maps listed in \"%s\".\n", filename); 
			 gi.dprintf ("Level Cycling DISABLED\n\n");
			 gi.cvar_forceset("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 
			 fclose(fp); 
			  
			 return;
		         
        } 
        else 
        { 
            fsetpos(fp,&pos);

			gi.dprintf ("-------------------------------------\n"); 
      
             // read map names into array 
             while ((!feof(fp)) && (i<MAX_MAPS) && Q_stricmp(szLineIn,"[dmend]") ) 
             { 
                fscanf(fp, "%s", szLineIn); 

                if (strchr(szLineIn,'#'))
					continue;

				if(!strchr(szLineIn,':'))
					continue;

				if(!Q_stricmp(szLineIn,"[dmend]"))
					break;
                            
				//Split out mapname
				length = strlen(szLineIn);
				
		
				for (j=1;j<length;j++)
					Levels[i][j-1] = szLineIn[j];
				
				Levels[i][j+1] = '\0';

				//No colons in map names and no blank maps
				if( (strchr(Levels[i],':')) || (strlen(Levels[i]) <= 1) )
				{
					gi.dprintf("Bogus map...not added\n");
					strcpy(Levels[i],"\0");
					continue;
				}

				// TODO: check that maps exist before adding to list 
				
				gi.dprintf("Map:%s\tAdded\n", Levels[i]); 
                i++; 
             }
			
			fclose(fp);
			
         } 
		
         if (i == 0) 
         { 
             gi.dprintf ("No DM maps listed in %s\n", filename); 
             gi.cvar_set("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 return;
		 }
		           
         gi.dprintf ("-------------------------------------\n"); 
		 gi.dprintf ("%i map(s) loaded.\n\n", i); 
		
	
		iTotalLevels = i-1; 
		iCurrLevel = iTotalLevels;

		// init levels array for duplicate avoidance
		for ( i = 0; i < MAX_MAPS; i++ ) iLevelsDone[i] = 0;
          
	}
	else
	{
		gi.dprintf("Couldn't open %s for level cycling\nLevel cycling not on\n", filename);
		gi.cvar_set("levelcycle","0");
					 
		return;
	}

	
	
} 

void K2_ReadCTFLevelCycleFile(void) 
{ 
    FILE *fp; 
    int  i=0, j=0, k=0, length=0;
	fpos_t pos;
	char szLineIn[80]="";
	char filename[50]="";
    cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
		
	gi.dprintf("CTF Level cycling is ON\n");
	sprintf ( filename, "%s/k2maps.txt", gamed->string );

	fp = fopen(filename,"rt"); 

	if (fp)  // opened successfully? 
    {
		//Check for CTFBEGIN
        do 
        { 
             fscanf(fp, "%s", szLineIn); 
        } while (!feof(fp) && Q_stricmp(szLineIn,"[ctfbegin]"));
		
		if (feof(fp)) 
        { 
             // no CTF section
             gi.dprintf ("-------------------------------------\n"); 
             gi.dprintf ("ERROR - No maps listed in CTF Section in \"%s\".\n", filename); 
			 gi.dprintf ("Level Cycling DISABLED\n\n");
			 gi.cvar_forceset("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 
			 fclose(fp); 
			  
			 return;
		         
        } 

		fgetpos(fp,&pos);
        
		// scan for a colon
        do 
        { 
             fscanf(fp, "%s", szLineIn); 
        } while (!feof(fp) && !strchr(szLineIn,':') && Q_stricmp(szLineIn,"[ctfend]"));

        if (feof(fp)) 
        { 
             // no colon = no maps!
             gi.dprintf ("-------------------------------------\n"); 
             gi.dprintf ("ERROR - No maps listed in \"%s\".\n", filename); 
			 gi.dprintf ("Level Cycling DISABLED\n\n");
			 gi.cvar_forceset("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 
			 fclose(fp); 
			  
			 return;
		         
        } 
        else 
        { 
            fsetpos(fp,&pos);

			gi.dprintf ("-------------------------------------\n"); 
      
             // read map names into array 
             while ((!feof(fp)) && (i<MAX_MAPS) && Q_stricmp(szLineIn,"[ctfend]")) 
             { 
                fscanf(fp, "%s", szLineIn); 

                if (strchr(szLineIn,'#'))
					continue;

				if(!strchr(szLineIn,':'))
					continue;
				
				if(!Q_stricmp(szLineIn,"[ctfend]"))
					break;
            
				//Split out mapname
				length = strlen(szLineIn);
				
				for (j=1;j<length;j++)
					CTFLevels[i][j-1] = szLineIn[j];
				
				CTFLevels[i][j+1] = '\0';

				//No colons in map names and no blank maps
				if( (strchr(CTFLevels[i],':')) || (strlen(CTFLevels[i]) <= 1) )
				{
					gi.dprintf("Bogus map...not added\n");
					strcpy(CTFLevels[i],"\0");
					continue;
				}

				// TODO: check that maps exist before adding to list 
				
				gi.dprintf("Map:%s\tAdded\n", CTFLevels[i]); 
                i++; 
             }
			
			fclose(fp);
			
         } 
		
         if (i == 0) 
         { 
             gi.dprintf ("No CTF maps listed in %s\n", filename); 
             gi.cvar_set("levelcycle","0");
			 gi.dprintf ("-------------------------------------\n"); 
			 return;
		 }
		           
         gi.dprintf ("-------------------------------------\n"); 
		 gi.dprintf ("%i map(s) loaded.\n\n", i); 
			
		iTotalCTFLevels = i-1; 
		iCurrCTFLevel = iTotalCTFLevels;

		// init levels array for duplicate avoidance
		for ( i = 0; i < MAX_MAPS; i++ ) iCTFLevelsDone[i] = 0;
          
	}
	else
	{
		gi.dprintf("Couldn't open %s for level cycling\nLevel cycling not on\n", filename);
		gi.cvar_set("levelcycle","0");
					 
		return;
	}

	
	
} 

char *K2_GetDMLevelSettings(char *level) 
{ 
    FILE *fp; 
    int  i=0, j=0, k=0, length=0;
    char szLineIn[80]="";
	char filename[30]="";
    cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
	char	variable[50] = "" ;
	char	value[10] = "" ;
		
	sprintf ( filename, "%s/k2maps.txt", gamed->string );

	fp = fopen(filename,"rt"); 

	if (!fp) return level;
	
    //Find DMBEGIN
	do 
    { 
       fgets(szLineIn,sizeof(szLineIn),fp);
	   
    
	} while (!feof(fp) && !strstr(szLineIn,"[dmbegin]"));

	if(feof(fp))
	{
		fclose(fp);
		gi.dprintf("No [dmbegin] section, level settings not changed!\n");
		return level;
	}
	
	//Find the level
	do 
    { 
       fgets(szLineIn,sizeof(szLineIn),fp);
    
	} while (!feof(fp) && !strstr(szLineIn,level) && Q_stricmp(szLineIn,"[dmend]"));

    
	 //Level not listed...must have been changed after server init
	if (feof(fp))
	{
		if (fclose(fp))
			gi.dprintf("Couldn't close %s\n",filename);
		else
			gi.dprintf("Closed %s\n",filename);

		gi.dprintf("Level not listed.  Level settings not changed.\n");
		
		return level;
	}
    
    
	//Make sure there is a var to read
	fgets(szLineIn,sizeof(szLineIn),fp);
		
    if(strchr(szLineIn,':') || strchr(szLineIn,'['))
	{
		fclose(fp);
		return level;
	}
	
	gi.dprintf("===========================\n");
	gi.dprintf("Changes for the next level:\n");
	
	//Set the vars
	while (!feof(fp)) 
    { 
         
			//Get variable
			length = strlen(szLineIn);
			i=0;	
			for (j=4 ;j<length; j++)
			{
				if (szLineIn[j] == 0x0020)
					break;

				variable[i] = szLineIn[j];
				i++; 
			}
			variable[i]='\0';
			
			//Get value
			i=0;
			for (j+=1; j<length;j++)
			{
				if (szLineIn[j] == '"')
					continue;
				value[i] = szLineIn[j];
				i++;
			}
			value[i]='\0';

			//Set it
			gi.cvar_set(variable,value);
			gi.dprintf("%s set to %s",variable,value);

			sprintf(variable,"");
			sprintf(value,"");
			
			//Get next line and check for no more vars
			fgets(szLineIn,sizeof(szLineIn),fp);

			if(strchr(szLineIn,':') || strchr(szLineIn,'['))
			{
				fclose(fp);
				gi.dprintf("===========================\n");
				return level;
			}
                  
    } 
	gi.dprintf("===========================\n");

    fclose(fp);
		
	return level;

} 

char *K2_GetCTFLevelSettings(char *level) 
{ 
    FILE *fp; 
    int  i=0, j=0, k=0, length=0;
    char szLineIn[80]="";
	char filename[30]="";
    cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
	char	variable[50] = "" ;
	char	value[10] = "" ;
		
	sprintf ( filename, "%s/k2maps.txt", gamed->string );

	fp = fopen(filename,"rt"); 

	if (!fp) return level;
	
    //Find CTFBEGIN
	do 
    { 
       fgets(szLineIn,sizeof(szLineIn),fp);
	     
	} while (!feof(fp) && !strstr(szLineIn,"[ctfbegin]"));

	if(feof(fp))
	{
		fclose(fp);
		gi.dprintf("No CTFBEGIN section, level settings not changed!\n");
		return level;
	}
	
	//Find the level
	do 
    { 
       fgets(szLineIn,sizeof(szLineIn),fp);
    
	} while (!feof(fp) && !strstr(szLineIn,level) && !strstr(szLineIn,"[ctfend]"));

    
	 //Level not listed...must have been changed after server init
	if (feof(fp))
	{
		if (fclose(fp))
			gi.dprintf("Couldn't close %s\n",filename);
		else
			gi.dprintf("Closed %s\n",filename);

		gi.dprintf("Level not listed.  Level settings not changed.\n");
		
		return level;
	}
        
    //Make sure there is a var to read
	fgets(szLineIn,sizeof(szLineIn),fp);
		
    if(strchr(szLineIn,':') || strchr(szLineIn,'['))
	{
		fclose(fp);
		return level;
	}
	
	gi.dprintf("===========================\n");
	gi.dprintf("Changes for the next level:\n");
	
	//Set the vars
	while (!feof(fp)) 
    { 
    		//Get variable
			length = strlen(szLineIn);
			i=0;	
			for (j=4 ;j<length; j++)
			{
				if (szLineIn[j] == 0x0020)
					break;

				variable[i] = szLineIn[j];
				i++; 
			}
			variable[i]='\0';
			
			//Get value
			i=0;
			for (j+=1; j<length;j++)
			{
				if (szLineIn[j] == '"')
					continue;
				value[i] = szLineIn[j];
				i++;
			}
			value[i]='\0';

			//Set it
			gi.cvar_set(variable,value);
			gi.dprintf("%s set to %s",variable,value);

			sprintf(variable,"");
			sprintf(value,"");

			//Get next line and Check for no more vars
			fgets(szLineIn,sizeof(szLineIn),fp);
		
			if(strchr(szLineIn,':') || strchr(szLineIn,'['))
			{
				fclose(fp);
				gi.dprintf("===========================\n");
				return level;
			}
    } 

    gi.dprintf("===========================\n");
	fclose(fp);
		
	return level;

} 

