#include "g_local.h"
#include "p_sogmusic.h"

#define MAXSONGS	64

#define MUSIC_OFF		0
#define MUSIC_READY		1
#define MUSIC_PLAYING	2
#define MUSIC_PAUSE		3

cvar_t *mp3_music;

char CurrentMP3[MAX_QPATH] = ""; 

songinfo songlist[MAXSONGS];

int Music_State = MUSIC_OFF;

int ReadSongEntry(FILE *fp, songinfo *entry);

void InitSongList(void)
{
	FILE *fp;
	int index =0;

	memset(&songlist, 0, sizeof(songlist));

	if(deathmatch->value)
		return;

	//open it the file
	fp = fopen("./sog/music.cfg", "r" );
	      
			
	if(fp)
    {          
		int fields_read =0;
		songinfo Temp;
				
		do
		{
			fields_read = ReadSongEntry(fp, &Temp);

			if(fields_read >=1)
			{
			//	if (strcmp(Temp.mapname, "\\") != 0)
			//	{
					songlist[index] = Temp;
					index++;
			//	}
			}
		
		}while(fields_read >= 1);

	}
	else
	{
		gi.dprintf("ERROR Loading songlist from music.cfg\n");
		return;
	}
	
	if(fclose(fp))
		gi.dprintf("ERROR Closing music.cfg\n");
}



int ReadSongEntry(FILE *fp, songinfo *entry)
{
	char buffer[MAX_QPATH]  = {0}; 
	int c;
	int i=0;
	int fInQuotes =0;
	int FieldsRead = 0;
		
//	char temp[8];
//	int	 num=0;


	do    
	{
        c = fgetc(fp); 
		
		//Use buffer 
/*		
		if (c == '/')
		{
			do
				c = fgetc(fp);
				
			while ((c != EOF) && (c != '\n'));
			if (c != EOF) c = fgetc(fp);
		}
*/		
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
						sscanf(buffer,"%s",entry->mapname);
						break;                
					}                
				case 1:
					{
						sscanf(buffer,"%s",entry->filename);
					}
	
			}            
			
			if ((strcmp(entry->mapname,"//") == 0) || (strcmp(entry->mapname,"//") == 0))
			{
				do
					c = fgetc(fp);
				while ((c != EOF) && (c != '\n'));
				i= 0;
				FieldsRead = 0;
			
			}
			else
			{
				i = 0;
            	FieldsRead++;        
			}
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
	} while((c != EOF) && (FieldsRead <= 1));    
	return FieldsRead;
}


//UTILITY FUNCTIONS

void PrintSongList(void)
{
	int i=0;
	
	gi.dprintf("#: mapname - filename\n");
	while(strlen(songlist[i].mapname))
	{
		gi.dprintf("%d: %s - %s\n",
			(i+1),songlist[i].mapname,
			songlist[i].filename);
		i++;
	}
}

qboolean SongListAlive(void)
{
	if(strlen(songlist[0].filename)) 
	{
	return true;
	}
return false;
}


int FindCurrentSongIndex(void)
{
	int i=0;
	qboolean found=false;

	while(strlen(songlist[i].mapname) ) 
	{
		if(strcmp(songlist[i].mapname,level.mapname)==0)
		{
			found = true;
			break;
		}
		if(i >= MAXSONGS)
			break;
		i++;
	}

	if(found==true)
	{
		return i;
	}
	return MAXSONGS;
}

void music_init(void)
{	
	if (Music_State == MUSIC_OFF)
	{
	
		mp3_music = gi.cvar("mp3_music", "0", 0);
		
		if (!mp3_music) 
			return;
		
		if (dedicated->value)
			return;
		
		gi.dprintf("==== Initializing SOG MP3 Drivers ====\n");
		
		
		// Loading playlist
	
		gi.dprintf("Loading music.cfg\n");
		
		InitSongList();
		
		if (SongListAlive())
		{
			// Loading MP3 drivers from FMOD
	
			gi.dprintf("Loading fmod.dll\n");

			if (FSOUND_GetVersion() != FMOD_VERSION)
			{
				gi.dprintf("Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
			}
			else
				// ==========================================================================================
				// INITIALIZE
				// ==========================================================================================
				if (!FSOUND_Init(44100, 32, 0))
			
					//safe_bprintf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
					gi.dprintf("Error while initialing sound\n");
				else
					Music_State = MUSIC_READY;
		}
	}
}

void music_play_song(void)
{
	int CurrentIndex;
	
//	if (dedicated)
//		return;

	if ((Music_State == MUSIC_READY) || (Music_State == MUSIC_PLAYING))
	{
	
		CurrentIndex = FindCurrentSongIndex();

		if (CurrentIndex == MAXSONGS)
		{
			music_pause_song();
			return;
		}
		
		if (strcmp(songlist[CurrentIndex].filename, CurrentMP3) != 0)
		{
	
			if (stream) FSOUND_Stream_Close(stream);
			stream = FSOUND_Stream_OpenMpeg(songlist[CurrentIndex].filename, FSOUND_NORMAL | FSOUND_LOOP_NORMAL);
			//tell the computer what the stream is (and it loops it too!)

			FSOUND_Stream_Play(FSOUND_FREE, stream);
			
			// Play the stream

			if (!stream)
				gi.dprintf("ERROR : music didn't load\n");
			else
			{
				Music_State = MUSIC_PLAYING;
				strncpy(CurrentMP3,songlist[CurrentIndex].filename,MAX_QPATH);
			}
		}
	}
}

void music_pause_song(void)
{
	if ((Music_State == MUSIC_PLAYING) || (Music_State == MUSIC_READY))
		if (stream)
			if (FSOUND_Stream_Stop (stream))
			{
				gi.dprintf("Music paused");
				Music_State = MUSIC_PAUSE;
			}
}

void music_stop_song(void)
{	
	if (Music_State != MUSIC_OFF)
	{	 
		if (stream) 
		{
			if (Music_State == MUSIC_PLAYING)
				FSOUND_Stream_Stop(stream);
			FSOUND_Stream_Close(stream);
		}
		FSOUND_Close();
		Music_State = MUSIC_OFF;
	}
}

void music_sogmov1_hack(void)
{
	int SongIndex=0;

	qboolean found=false;
	
	if ((Music_State == MUSIC_READY) || (Music_State == MUSIC_PLAYING))
	{
		while(strlen(songlist[SongIndex].mapname) ) 
		{
			if(strcmp(songlist[SongIndex].mapname,"sogmov1")==0)
			{
				found = true;
				break;
			}
			if(SongIndex >= MAXSONGS)
				break;
			SongIndex++;
		}

		if(found != true)
		{
			return;
		}
		else
		{	
			if (strcmp(songlist[SongIndex].filename, CurrentMP3) != 0)
			{
				if (stream) FSOUND_Stream_Close(stream);
				stream = FSOUND_Stream_OpenMpeg(songlist[SongIndex].filename, FSOUND_NORMAL | FSOUND_LOOP_NORMAL);
				//tell the computer what the stream is (and it loops it too!)

				FSOUND_Stream_Play(FSOUND_FREE, stream);
				// Play the stream

				if (!stream)
					gi.dprintf("ERROR : music didn't load\n");
				else
				{
					Music_State = MUSIC_PLAYING;
					strncpy(CurrentMP3,songlist[SongIndex].filename,MAX_QPATH);
				}
			}
		}
	}
}