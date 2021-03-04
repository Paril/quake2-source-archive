/*
 *  Standard Logging functions
 *
 *
 * $Id: stdlog.h 1.7 1998/04/06 09:24:23 mdavies Exp $
 */

#ifndef __STDLOG_H__
#define __STDLOG_H__

// #include "g_local.h"


#define __STDLOG_ID__ "$Id: stdlog.h 1.7 1998/04/06 09:24:23 mdavies Exp $"

/*
 *   StdLog API
 *
 */

extern int  sl_OpenLogFile( game_import_t  *gi );
extern void sl_CloseLogFile( void );
extern void sl_LogVers( game_import_t  *gi );

extern void sl_LogPatch( game_import_t  *gi,
                         char           *pPatchName );

extern void sl_LogDate( game_import_t  *gi );
extern void sl_LogTime( game_import_t  *gi );

extern void sl_LogDeathFlags( game_import_t  *gi,
                              unsigned long   dmFlags);

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
                         float           timeInSeconds );

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


/*
 *   OLD API
 *
 */

extern int  sl_Logging( game_import_t  *gi,
                        char           *pPatchName );

extern void sl_GameStart( game_import_t    *gi,
                          level_locals_t    level );

extern void sl_GameEnd( game_import_t      *gi,
                        level_locals_t      level );

extern void sl_WriteStdLogDeath( game_import_t     *gi,
                                 level_locals_t     level,
                                 edict_t           *self,
                                 edict_t           *inflictor,
                                 edict_t           *attacker );

extern void sl_WriteStdLogPlayerEntered( game_import_t     *gi,
                                         level_locals_t     level,
                                         edict_t           *ent );

extern void sl_LogPlayerDisconnect( game_import_t      *gi,
                                    level_locals_t      level,
                                    edict_t            *ent );

#else
#define sl_Logging( gi, pPatchName )
#define sl_GameStart( gi, level )
#define sl_GameEnd( gi, level )
#define sl_WriteStdLogDeath( gi, level, self, inflictor, attacker )
#define sl_WriteStdLogPlayerEntered( gi, level, ent )
#define sl_LogPlayerDisconnect( gi, level, ent )
#endif


/* end of file */