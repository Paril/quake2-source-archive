/*
 *  GibStats Logging functions
 *
 *
 * $Id: gslog.h,v 1.7 1999/05/22 22:32:12 major Exp $
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

#endif

/* end of file */

