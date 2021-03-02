/******************************************************************************
**
**	Standard Logging functions
**
**	Public header file.
**
**	$Id: stdlog.h 1.8 1998/08/29 13:52:24 mdavies Exp mdavies $
**
******************************************************************************/


#ifndef __STDLOG_H__
#define __STDLOG_H__

// #include "g_local.h"


#define __STDLOG_ID__ "$Id: stdlog.h 1.8 1998/08/29 13:52:24 mdavies Exp mdavies $"



/******************************************************************************
**
**	DEFINES
**
******************************************************************************/

#define SL_ERROR_NONE        0
#define SL_ERROR_NOMEM      -1
#define SL_ERROR_BADPACKET  -2

#define SL_FLAGS_NONE        0
#define SL_FLAGS_COMPRESS    (1<<0)

/******************************************************************************
**
**	Typedefs
**
******************************************************************************/

typedef unsigned long DWORD;
typedef unsigned char BYTE;
   
typedef struct
{
    DWORD   nPacket;
    DWORD   nClients;
    char   *pServerInfo;
    char   *pLogString;
    char  **ppClientInfo;               /* NULL terminated Array of strings pointers */
} PACKET_STRUCT;

typedef unsigned char   PACKET_BYTES;

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




extern int  sl_DeconstructPacketBytes( PACKET_STRUCT **ppPacketStruct,
                                       PACKET_BYTES   *pPacketBytes,
                                       unsigned long   ulPacketBytesLength,
                                       unsigned long   ulFlags );

extern void sl_DestroyPacketBytes( PACKET_STRUCT *pPacketStruct );




#endif

/* end of file */