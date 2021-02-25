/******************************************************************************
**
**	GibStats Logging functions
**
**
**	$Id: gslog.h 1.2 1999/04/07 21:17:37 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/

#ifndef __GSLOG_H__
#define __GSLOG_H__

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