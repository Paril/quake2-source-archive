#include "g_local.h" 
#include "g_genmaps.h"

/*
Thanks to Allan Campbell, and Mike Davies for releasing their
map rotation code, on which the following is loosely based
*/

#define DM		1
#define TEAM	2
#define CTF		3

#define MAXMAPS	64

mapinfo maplist[MAXMAPS];

//static 
int ReadMapEntry(FILE *fp, mapinfo *entry);

void PrintMapList(void)
{
	int i=0;
	
	gi.dprintf("#: filename - game - type - times\n");
	while(strlen(maplist[i].filename))
	{
		gi.dprintf("%d: %s - %d - %d - %d \n",
			(i+1),maplist[i].filename,
			maplist[i].game,
			maplist[i].type, maplist[i].timesVisited);
		i++;
	}
}

/*
Setup the Maps Linked List
and open the maps.lst file for reading
*/

void InitMapList(void)
{
	FILE *fp;
	int index =0;

	memset(&maplist, 0, sizeof(maplist));

	if(!deathmatch->value)
		return;

	gi.dprintf("==== Loading Maplist ====\n");

	//open it the file
	if(gen_ctf->value ||
	   ((int)genflags->value & GEN_CTF_ONLY))
		fp = fopen("./gen/ctfmaps.lst", "r" );
	else
		fp = fopen("./gen/dmmaps.lst", "r" );
	      
			
	if(fp)
    {          
		int fields_read =0;
		mapinfo Temp;
				
		do
		{
			Temp.game =0;
			Temp.type=0;
			Temp.timesVisited =0;
			fields_read = ReadMapEntry(fp, &Temp);

			if(fields_read >=1)
			{
				maplist[index] = Temp;
				index++;
			}
		
		}while(fields_read >= 2);

	}
	else
	{
		gi.dprintf("==== Error Loading Maplist ====\n");
		return;
	}
	
	if(fclose(fp))
		gi.dprintf("=== ERROR Closing Maplist File ====\n");
}


//static 
int ReadMapEntry(FILE *fp, mapinfo *entry)
{
	char buffer[32]  = {0}; 
	int c;
	int i=0;
	int fInQuotes =0;
	int FieldsRead = 0;
		
	char temp[8];
	int	 num=0;


	do    
	{
        c = fgetc(fp); 
		
		//Use buffer 
		if((i > 0) && 
		   ((((' ' == c) || ('\t' == c)) && !fInQuotes) ||
           (EOF == c) || ('\n' == c)))        
		{
            buffer[i] = '\0';            
			
			switch(FieldsRead)           
			{
                case 0:                
					{
						//strncpy(entry->filename, buffer, 10 ); 
						sscanf(buffer,"%s",entry->filename);
						break;                
					}                
				case 1:
					{
						//strncpy (temp,buffer,8);
						sscanf(buffer,"%s",temp);

						if(Q_stricmp(temp,"q1")==0)
							entry->game = CLASS_Q1;
						else if (Q_stricmp(temp,"doom")==0)
							entry->game = CLASS_DOOM;
						else if (Q_stricmp(temp,"wolf")==0)
							entry->game = CLASS_WOLF;
						else if (Q_stricmp(temp,"q2")==0)
							entry->game = CLASS_Q2;
						else 
							entry->game = CLASS_Q2;
						break;
					}
				case 2:
					{                    
						strncpy (temp, buffer,8);
						if(Q_stricmp(temp,"dm")==0)
							entry->type = DM;
						else if (Q_stricmp(temp,"team")==0)
							entry->type = TEAM;
						else if (Q_stricmp(temp,"ctf")==0)
							entry->type = CTF;
						else 
							entry->type = DM;
						break;
					}
			}            
			
			i = 0;
            FieldsRead++;        
		}       
		else        
		{            
			switch( c )
            {    
				case '\"':                
					{
						fInQuotes = 1 - fInQuotes;                    
						break;
					}                
				case '\t':
                case ' ':
					{                    
						if( !fInQuotes )
                        break;                
					} 
// fallthrough 
                default:                
					{
						if( i < (MAX_QPATH-1) )                    
						{
							buffer[i] = c;                        
							i++;
						}       
						break;
					}            
			}
        }    
	} while( (c != EOF) && (c != '\n') );    
	return FieldsRead;
}


//UTILITY FUNCTIONS



qboolean GenMapListAlive(void)
{
	if(strlen(maplist[0].filename)) 
	{
	return true;
	}
return false;
}


int FindCurrentIndex()
{
	int i=0;
	qboolean found=false;

	while(strlen(maplist[i].filename) ) 
	{
		if(strcmp(maplist[i].filename,level.mapname)==0)
		{
			found = true;
			break;
		}
		if(i >= MAXMAPS)
			break;
		i++;
	}

	if(found==true)
	{
		return i;
	}
	return 0;
}


char * FindCurrentMap()
{
	int i=0;
	qboolean found=false;

	while(strlen(maplist[i].filename) ) 
	{
		if(strcmp(maplist[i].filename,level.mapname)==0)
		{
			found = true;
			break;
		}
		if(i >= MAXMAPS)
			break;
		i++;
	}

	if(found==true)
	{
		return maplist[i].filename;
	}
	return " ";
}

int	FindNextMap(int i, int type)
{
	int index;
	qboolean found=false;
	index = i+1;
	
	while(strlen(maplist[index].filename) 
			&& maplist[index].type)
	{
		if(maplist[index].type == type)
		{ 
			found = true;
			break;
		}
		index++;
	}

	if(found)
		return index;
	else
		return 0;
}

char * GenNextMap(void)
{
	int i=0;
	int index=0;

	i= FindCurrentIndex();

	if(i >= 0)
	{
		if(((int)genflags->value & GEN_DM_ONLY)
			|| gen_team->value)
			index =FindNextMap(i,DM);
		else if((int)genflags->value & GEN_CTF_ONLY)
			index = FindNextMap(i,CTF);
		else 
			index = i +1; //next map
	}

//GPrint("Next map :%s\n", maplist[index].filename);
	if(strlen(maplist[index].filename))
		return maplist[index].filename;
	else
	    return maplist[0].filename;
}
