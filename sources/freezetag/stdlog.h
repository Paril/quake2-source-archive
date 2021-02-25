/******************************************************************************
**
**	Standard Logging functions
**
**	Public header file.
**
**	$Id: stdlog.h 1.11 1999/04/25 17:52:27 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/


#ifndef __STDLOG_H__
#define __STDLOG_H__

#define __STDLOG_ID__ "$Id: stdlog.h 1.11 1999/04/25 17:52:27 mdavies Exp $"


/******************************************************************************
**
**	DEFINES
**
******************************************************************************/
#define SL_LOG_STYLE_1_2      0
#define SL_LOG_STYLE_1_2a     1

#define SL_LOG_STYLE_LATEST   SL_LOG_STYLE_1_2a
#define SL_LOG_STYLE_DEFAULT  SL_LOG_STYLE_1_2


/******************************************************************************
**
**	Typedefs
**
******************************************************************************/

/******************************************************************************
**
**	StdLog API
**
******************************************************************************/

extern int  sl_OpenLogFile( game_import_t  *gi );
extern void sl_CloseLogFile( void );
extern void sl_FlushLogFile( void );

extern void sl_LogVers( game_import_t  *gi );

extern void sl_LogPatch( game_import_t  *gi,
                         char           *pPatchName );

extern void sl_LogDate( game_import_t  *gi );
extern void sl_LogTime( game_import_t  *gi );

extern void sl_LogDeathFlags( game_import_t  *gi,
                              unsigned long   dmFlags );

extern void sl_LogDeathFlagsEx( game_import_t  *gi,
                                unsigned long   dmFlags,
                                const char     *pString );

extern void sl_LogMapName( game_import_t  *gi,
                           char           *pMapName );

extern void sl_LogPlayerName( game_import_t  *gi,
                              char           *pPlayerName,
                              char           *pTeamName,
                              float           timeInSeconds );

extern void sl_LogScore( game_import_t  *gi,
                         char           *pKillerName,
                         char           *pTargetName,
                         char           *pScoreType,
                         char           *pWeaponName,
                         int             iScore,
                         float           timeInSeconds,
                         int             killerPing );

extern void sl_LogPlayerLeft( game_import_t  *gi,
                              char           *pPlayerName,
                              float           timeInSeconds );

extern void sl_LogGameStart( game_import_t  *gi,
                             float           timeInSeconds );

extern void sl_LogGameEnd( game_import_t  *gi,
                           float           timeInSeconds );

extern void sl_LogPlayerConnect( game_import_t  *gi,
                                 char           *pPlayerName,
                                 char           *pTeamName,
                                 float           timeInSeconds);

extern void sl_LogPlayerTeamChange( game_import_t  *gi,
                                    char           *pPlayerName,
                                    char           *pTeamName,
                                    float           timeInSeconds);

extern void sl_LogPlayerRename( game_import_t  *gi,
                                char           *pOldPlayerName,
                                char           *pNewPlayerName,
                                float           timeInSeconds);





#endif

/* end of file */
