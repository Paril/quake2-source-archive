//===========================================================================
//
// Name:         g_log.c
// Function:     log file stuff
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1997-12-31
// Tab Size:     3
//===========================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "g_local.h"

#define MAX_LOGFILENAMESIZE		1024

char logfilename[MAX_LOGFILENAMESIZE];
FILE *logfp;
int lognumwrites;

void Log_Open(char *filename)
{
	if (!filename || !strlen(filename))
	{
		gi.dprintf("openlog <filename>\n");
		return;
	} //end if
	if (logfp)
	{
		gi.dprintf("The log file %s is already opened\n", logfilename);
		return;
	} //end if
	logfp = fopen(filename, "wb");
	if (!logfp)
	{
		gi.dprintf("Error opening log file %s\n", filename);
		return;
	} //end if
	strncpy(logfilename, filename, MAX_LOGFILENAMESIZE);
	gi.dprintf("Opened log %s\n", logfilename);
} //end of the function Log_Create

void Log_Close(void)
{
	if (!logfp)
	{
		gi.dprintf("no log file to close\n");
		return;
	} //end if
	if (fclose(logfp))
	{
		gi.dprintf("Error closing log file %s\n", logfilename);
		return;
	} //end if
	logfp = NULL;
	gi.dprintf("Closed log %s\n", logfilename);
} //end of the function Log_Close

void Log_ShutDown(void)
{
	if (logfp) Log_Close();
} //end of the function Log_ShutDown

void Log_Write(char *fmt, ...)
{
	va_list ap;

	if (!logfp) return;
	va_start(ap, fmt);
	vfprintf(logfp, fmt, ap);
	va_end(ap);
	fprintf(logfp, "\r\n");
	fflush(logfp);
} //end of the function Log_Write

void Log_WriteTimeStamped(char *fmt, ...)
{
	va_list ap;

	if (!logfp) return;
	fprintf(logfp, "%d   %02d:%02d:%02d:%02d   ",
					lognumwrites,
					(int) (level.time / 60 / 60),
					(int) (level.time / 60),
					(int) (level.time),
					(int) ((int) (level.time * 100)) - ((int) level.time) * 100);
	va_start(ap, fmt);
	vfprintf(logfp, fmt, ap);
	va_end(ap);
	fprintf(logfp, "\r\n");
	lognumwrites++;
	fflush(logfp);
} //end of the function Log_Write

qboolean LogCmd(char *cmd)
{
	if (Q_stricmp (cmd, "openlog") == 0)
	{
		//NOTE: 2 because it's an "sv" command
		Log_Open(gi.argv(2));
	} //end if
	else if (Q_stricmp (cmd, "closelog") == 0)
	{
		Log_Close();
	} //end else if
	else if (Q_stricmp (cmd, "writelog") == 0)
	{
		//NOTE: 2 because it's an "sv" command
		Log_WriteTimeStamped(gi.argv(2));
	} //end else if
	else
	{
		return false;
	} //end else
	return true;
} //end of the function LogCmd

