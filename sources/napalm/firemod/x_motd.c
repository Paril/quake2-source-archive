/*==========================================================================
//  x_motd.c -- by Patrick Martin               Last updated:  3-7-1999
//--------------------------------------------------------------------------
//  This file contains code for adjustable MOTD.
//========================================================================*/

#include "g_local.h"
#include "x_motd.h"

cvar_t  *gamedir;
cvar_t  *file_motd;


/*===============================/  MOTD  /===============================*/

static char     motd[MAX_MOTD];

/*------------------------------------------------------/ New Code /--------
//  This checks if the character value is useful and desired.
//------------------------------------------------------------------------*/
qboolean PBM_LegalChar (char c)
{
        int     i = (int)c;

/* Newline character (ascii: 10) is legal, but handled elsewhere. */

/* If value is negative (signed), convert it to positive (unsigned). */
        if (i < 0)
                i += 256;

/* Any character whose value is or between 32 and 254 is legal. */
        if ((i > 31) && (i < 255))
                return true;

/* All others are forbidden.  (Value 255 will CRASH Quake!) */
        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This opens the "motd.ini" file if available, then puts the file's
//  contents into a buffer for later use.
//------------------------------------------------------------------------*/
void PBM_GetMOTD (void)
{
        FILE    *infile;
        char    filename[MAX_OSPATH];
        char    c;
        int     i = 0;
        int     filepos = 0;
        int     linepos = 0;
        int     no_of_lines = 0;
        int     textlimit = MAX_MOTD - 1;

/* Initialize MOTD. */
        motd[0] = 0;

/* Get path of game directory, and the name of the motd file. */
        sprintf (filename, "%s/%s.ini", gamedir->string, file_motd->string);

/* If MOTD file does not exist, exit function.
   NOTE:  File is opened in binary mode because the file may contain
   displayable characters whose value is above the ASCII value 127.
*/
        if ((infile = fopen(filename, "rb")) == NULL)
                return;

/* Read file until EOF or end of MOTD buffer or many lines are read,
   whichever comes first.
*/
        while ( (filepos < textlimit) && (no_of_lines < 16) &&
                ((c = fgetc(infile)) != EOF) )
        {
                if (c == '\n')
                {       motd[i] = c;
                        i++;
                        filepos++;
                        linepos = 0;
                        no_of_lines++;
                }
                else if ( (PBM_LegalChar (c)) && (linepos < 36) )
                {       motd[i] = c;
                        i++;
                        filepos++;
                        linepos++;
                }
                else
                        filepos++;
        }

/* We don't need the file anymore, so close it! */
        fclose (infile);

/* If no message was read, exit function. */
        if (i < 1)
                return;

/* Make sure the rest of the buffer has no garbage. */
        motd[i] = 0;
}

/*------------------------------------------------------/ New Code /--------
//  This centerprints the MOTD if any.
//------------------------------------------------------------------------*/
void PBM_ShowMOTD (edict_t *ent)
{
/* If no MOTD, nothing to show. */
        if (!motd[0])
                return;

/* Display the MOTD. */
        gi.centerprintf (ent, motd);
}


/*==========================/  Initialization  /==========================*/

/*------------------------------------------------------/ New Code /--------
//  This initializes some variables and the MOTD.
//------------------------------------------------------------------------*/
void PBM_ConfigInit (void)
{
        gamedir = gi.cvar ("game", "", CVAR_SERVERINFO | CVAR_LATCH);
        file_motd = gi.cvar ("file_motd", "motd", CVAR_SERVERINFO);

        PBM_GetMOTD ();
}


/*===========================/  END OF FILE  /===========================*/
