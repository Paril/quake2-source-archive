//begin fraglogfile modification
#include "g_local.h"
#include "s_readconfig.h" // has directory info

void logFrag( char *frager, char *fragee )
{

	if( !(logfile->value) )		//if fraglogfile is set to 0 check to see if logfile
	{						//is open, then get out of here
		if( fragFile )
		{
			fclose( fragFile );
			fragFile = NULL;
		}
		return;
	}
		
	if( !fragFile )   //fraglogfile is set to 1, make sure logfile is open
	{
		FILE *f;
		int i, j, flag = 0;
		char logFileName[ 80 ];

		// properly initialize variable
		for (j=0; j<80; j++)
			logFileName[j]='\0';

		for( i = 0; i <= 999; i++ )  //find next "unused" frag log file
		{
			sprintf( logFileName, "%sfrag_%i.log", directory, i );
			
			if( ( f = fopen( logFileName, "r" ) ) == NULL )
			{
				if( ( fragFile = fopen( logFileName, "w" ) ) != NULL )  flag = 1;
				break;
			}
			else fclose( f );
		}

		if( !flag )
		{
			gi.cvar_set("logfile", "0"); // was fraglogfile, if all 999 log files are used or 
			return;  //couldn't open new log file then disable logging
		}
	}

	fprintf( fragFile, "\\%s\\%s\\\n", frager, fragee );  
	fflush(fragFile); // fixed by John Penman 
	//write \frager\fragee\ followed by a \n to log file
	//NOTE:  this is the same format as quakeworld frag log files :)
}
//end fraglogfile modification
