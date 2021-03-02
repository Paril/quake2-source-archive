/*
 *  GibStats Logging functions
 *
 *
 * $Id: stdlog.h 1.4 1998/03/14 13:19:09 mdavies Exp mdavies $
 */

#ifndef __STDLOG_H__
#define __STDLOG_H__

#define _USE_LOGGING_
#ifdef _USE_LOGGING_

//#define __STDLOG_ID__ "$Id: stdlog.h 1.4 1998/03/14 13:19:09 mdavies Exp $"
#define __STDLOG_ID__ "$Id: stdlog.h 1.7 1998/04/06 09:24:23 mdavies Exp $"

/*
 *   NEW API
 *
 */

extern void sl_LogPlayerTeamChange( game_import_t  *gi,
                                    char           *pPlayerName,
                                    char           *pTeamName);

extern void sl_LogPlayerRename( game_import_t  *gi,
                                char           *pOldPlayerName,
                                char           *pNewPlayerName);

extern void sl_Logging( game_import_t  *gi,
                        char           *pPatchName );

extern void sl_GameStart( game_import_t    *gi,
                          level_locals_t    level );

extern void sl_GameEnd( game_import_t      *gi,
                        level_locals_t      level );

void sl_LogScore( game_import_t  *gi,
                         char           *pKillerName,
                         char           *pTargetName,
                         char           *pScoreType,
                         int			mod,
                         int             iScore);

extern void sl_LogPlayerConnect( game_import_t     *gi,
                                         level_locals_t     level,
                                         edict_t           *ent );

extern void sl_LogPlayerDisconnect( game_import_t      *gi,
                                    level_locals_t      level,
                                    edict_t            *ent );

extern void sl_LogPlayerName( game_import_t  *gi,
                              char           *pPlayerName,
                              char           *pTeamName );
#else
#define sl_LogPlayerTeamChange( gi, pPlayerName, pTeamName)
#define sl_LogPlayerRename( gi, OldPlayerName, pNewPlayerName)
#define sl_Logging( gi, pPatchName )
#define sl_GameStart(gi, level )
#define sl_GameEnd( gi,  level )
#define sl_LogScore( gi,  pKillerName, pTargetName, pScoreType, pWeaponName,  iScore)
#define sl_LogPlayerConnect( gi, level, ent )
#define sl_LogPlayerDisconnect( gi, level, ent )
#define sl_LogPlayerName( gi, pPlayerName, pTeamName)
#endif

#endif

/* end of file */
