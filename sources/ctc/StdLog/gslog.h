/*
 *  GibStats Logging functions
 *
 *
 * $Id: gslog.h 1.1 1998/04/04 11:05:17 mdavies Exp $
 */

#ifndef __GSLOG_H__
#define __GSLOG_H__

/*
 *   OLD API
 *
 */

extern int  sl_Logging( game_import_t  *gi,
                        char           *pPatchName );

extern void sl_GameStart( game_import_t    *gi,
						  char             *pPatchName,
                          level_locals_t   level );

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

#endif

/* end of file */