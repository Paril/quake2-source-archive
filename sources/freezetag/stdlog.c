/******************************************************************************
**
**	Standard Logging functions
**
**
**	$Id: stdlog.c 1.11 1999/04/25 17:34:09 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/

#include "g_local.h"

#include "stdlog.h"

#include "sl_packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

/* OS specific includes, defines and typdefs */
#ifdef WIN32
#   include <winsock.h>
typedef int socklen_t;
/* typedef struct sockaddr_in SOCKADDR_IN; */
#else /* WIN32 */
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
#   define INVALID_SOCKET -1
typedef int BOOL;
#define TRUE 1
#endif /* WIN32 */


/******************************************************************************
**
**	Extern Globals
**
******************************************************************************/
extern  game_locals_t   game;


/******************************************************************************
**
**	LOCAL DEFINES
**
******************************************************************************/

/*#define PRINT_ERRORS*/
/*#define PRINT_VERBOSE*/

#define BIND_SOC
#define SET_SOC_DEST

#define TCP_ALLOW_BUFFER

/* #define INIT_WSA */

/* #define _DEBUG_UDP */

#ifdef WIN32
#pragma warning( disable : 4002 )
#endif

#if defined(_DEBUG) && defined(PRINT_VERBOSE)
#define dbg_printf pGI->dprintf
#else
#ifdef WIN32
#define dbg_printf()
#else
#define dbg_printf(x...)
#endif
#endif

#if defined(_DEBUG) && defined(PRINT_ERRORS)
#define err_printf pGI->error
#else
#ifdef WIN32
#define err_printf()
#else
#define err_printf(x...)
#endif
#endif

#define inf_printf pGI->dprintf

#define K(x) (x *1024)


#define MAX_DATE_STRLEN     (MAX_QPATH)   /* enough for dd mmm yyyy TMZ */
#define MAX_TIME_STRLEN     (MAX_QPATH)     /* hh:mm:ss  */

#define MAX_FILENAME_LEN    (1025)           /* Max file path/name len */

/******************************************************************************
**
**	Logging Output Method Stuff
**
******************************************************************************/

#define SL_LOGOUT_NAME_STR          "sl_log_method"
#define SL_FILE_NAME_STR            "sl_filename"
#define SL_DEFFILE_NAME_STR         "StdLog.log" /* "<date>" == dd/mm/yy */

#define SL_LOGOUT_FILE_MASK         (1<<0)
#define SL_LOGOUT_UDP_MASK          (1<<1)
#define SL_LOGOUT_TCP_MASK          (1<<2)
#define SL_LOGOUT_FILE(x)           ((unsigned int)x & SL_LOGOUT_FILE_MASK)
#define SL_LOGOUT_UDP(x)            ((unsigned int)x & SL_LOGOUT_UDP_MASK)
#define SL_LOGOUT_TCP(x)            ((unsigned int)x & SL_LOGOUT_TCP_MASK)

/******************************************************************************
**
**	Logging Style Method Stuff
**
******************************************************************************/

#define SL_LOGSTYLE_NAME_STR        "sl_log_style"
#define SL_LOGSTYLE_DEF_STR         "0"
#define SL_LOGSTYLE_DEF             SL_LOG_STYLE_DEFAULT

#define SL_URL_NAME_STR             "sl_url"
#define SL_URL_DEF_STR              "http://www.planetquake.com/gslogmod/"

#define SL_LOGFLUSH_NAME_STR        "sl_log_flush"
#define SL_LOGFLUSH_DEF_STR         "0"
#define SL_LOGFLUSH_NEVER           0
#define SL_LOGFLUSH_MAPCHANGE       1
#define SL_LOGFLUSH_ALWAYS          2


#define SL_UDP_DEST_STR             "sl_udp_dest" /* IP:Port */
#define SL_UDP_BUFFSIZE_STR         "sl_udp_buffer_size"
#define SL_UDP_DEFBUFFSIZE_STR      "1024"
#define SL_UDP_MAXBUFFSIZE          2048
#define SL_UDP_PACKETNUM_STR        "sl_udp_packet_number"

#define SL_TCP_DEST_STR             "sl_tcp_dest" /* IP:Port */

#define SL_SERVER_INFO_STR          "sl_server_info"
#define SL_DEF_SERVER_INFO          "$HOST_NAME$ $HOST_IP$ cheats fraglimit game gamedate gamedir gamename hostname maxclients timelimit version port ip"

#define SL_CLIENT_INFO_STR          "sl_client_info"
#define SL_DEF_CLIENT_INFO          "motto ip skin"


/******************************************************************************
**
**	LOCAL VARIABLES
**
******************************************************************************/
static cvar_t   *log_method_cvar  = NULL;
static cvar_t   *log_style_cvar   = NULL;
static cvar_t   *log_flush_cvar   = NULL;

static FILE     *StdLogFile = NULL;

static unsigned int log_style = 0;

static const char *_unused_id_stdlog_c = "$Id: stdlog.c 1.11 1999/04/25 17:34:09 mdavies Exp $";
static const char *_unused_id_stdlog_h = __STDLOG_ID__;


static char *pServerInfo = NULL;

/* Share scratch buffer for date functions */
static char _sl_date[MAX_DATE_STRLEN+1] = {0};


#ifdef _DEBUG_UDP
static FILE *_debug_udp_file = NULL;
#endif

/******************************************************************************
**
**	LOCAL FUNCTION PROTOTYPES
**
******************************************************************************/
static void _sl_LogVers_1_2( void );
static void _sl_LogVers_1_2a( void );

static void _sl_LogPatch( char *pPatchName );

static void _sl_LogDate_1_2( void );
static void _sl_LogDate_1_2a( void );

static void _sl_LogTime( void );
static void _sl_LogDeathFlags( unsigned long dmFlags, const char *pString );
static void _sl_LogMapName( char *pMapName );

static void _sl_LogPlayerName( char *pPlayerName,
                               char *pTeamName,
                               float timeInSeconds );

static void _sl_LogScore( char *pKillerName,
                          char *pTargetName,
                          char *pScoreType,
                          char *pWeaponName,
                          int   iScore,
                          float timeInSeconds,
                          int   killerPing );

static void _sl_LogPlayerLeft( char *pPlayerName,
                               float timeInSeconds );

static void _sl_LogGameStart( float timeInSeconds );
static void _sl_LogGameEnd( float timeInSeconds );

static void _sl_LogPlayerConnect( char *pPlayerName,
                                  char *pTeamName,
                                  float timeInSeconds );

static void _sl_LogPlayerTeamChange( char *pPlayerName,
                                     char *pTeamName,
                                     float timeInSeconds );

static void _sl_LogPlayerRename( char *pOldPlayerName,
                                 char *pNewPlayerName,
                                 float timeInSeconds );





/******************************************************************************
**
**	Log Method Functions
**
******************************************************************************/
static __inline int _sl_open( game_import_t  *gi );
static __inline void _sl_printf( const char * fmt, ... );
static __inline void _sl_flush( void );
static __inline void _sl_close( void );

#ifdef WIN32
static void net_disp_err(void);
#endif

static unsigned long atoaddr(char *address);



static char *_sl_add_cvars( char *pCurrent, char *pVarNameList, game_import_t  *gi );
static char *_sl_add_string( char *pCurrent, const char *pString );
static char *_sl_strip_cntrl( const char *pOld );

static void _sl_create_clientinfos( PACKET_STRUCT *pPacketStruct );
static void _sl_free_clientinfos( PACKET_STRUCT *pPacketStruct );


/******************************************************************************
**
**	LOCAL TYPEDEFS
**
******************************************************************************/

typedef struct
{
    void (*pLogVers)( void );
    void (*pLogPatch)( char *pPatchName );
    void (*pLogDate)( void );
    void (*pLogTime)( void );
    void (*pLogDeathFlags)( unsigned long dmFlags, const char *pString );
    void (*pLogMapName)( char *pMapName );
    void (*pLogPlayerName)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogScore)( char *pKillerName, char *pTargetName, char *pScoreType, char *pWeaponName, int   iScore, float timeInSeconds, int killerPing  );
    void (*pLogPlayerLeft)( char *pPlayerName, float timeInSeconds );
    void (*pLogGameStart)( float timeInSeconds );
    void (*pLogGameEnd)( float timeInSeconds );
    void (*pLogPlayerConnect)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogPlayerTeamChange)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogPlayerRename)( char *pOldPlayerName, char *pNewPlayerName, float timeInSeconds );
} LOG_FUNCS;

static LOG_FUNCS _sl_log_styles[] =
{
    /* 0    =   SL1.2 */
    {
        _sl_LogVers_1_2,
        _sl_LogPatch,
        _sl_LogDate_1_2,
        _sl_LogTime,
        _sl_LogDeathFlags,
        _sl_LogMapName,
        _sl_LogPlayerName,
        _sl_LogScore,
        _sl_LogPlayerLeft,
        _sl_LogGameStart,
        _sl_LogGameEnd,
        _sl_LogPlayerConnect,
        _sl_LogPlayerTeamChange,
        _sl_LogPlayerRename,
    },

    /* 1    =   SL1.2a */
    {
            _sl_LogVers_1_2a,
            _sl_LogPatch,
            _sl_LogDate_1_2a,
            _sl_LogTime,
            _sl_LogDeathFlags,
            _sl_LogMapName,
            _sl_LogPlayerName,
            _sl_LogScore,
            _sl_LogPlayerLeft,
            _sl_LogGameStart,
            _sl_LogGameEnd,
            _sl_LogPlayerConnect,
            _sl_LogPlayerTeamChange,
            _sl_LogPlayerRename,
    }
    
};

static game_import_t  *pGI = NULL;



/******************************************************************************
**
**	PRIVATE FUNCTIONS - Standard Logging 1.2
**
******************************************************************************/


/******************************************************************************
**
**	_sl_LogVers (SL1.2)
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogVers_1_2( void )
{
    /* StdLog + VersNr */
    _sl_printf( "\t\tStdLog\t1.2\n" );
}

/******************************************************************************
**
**	_sl_LogVers (SL1.2a)
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogVers_1_2a( void )
{
    /* StdLog + VersNr */
    _sl_printf( "\t\tStdLog\t1.2a\n" );
}

/******************************************************************************
**
**	_sl_LogPatch
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPatch( char *pPatchName )
{
    char *pStripPatch = pPatchName?_sl_strip_cntrl(pPatchName):NULL;

    _sl_printf( "\t\tPatchName\t%s\n",
                pStripPatch?pStripPatch:(pPatchName?pPatchName:"") );

    if( pStripPatch )        free( pStripPatch );
}

/******************************************************************************
**
**	_sl_LogDate_1_2
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogDate_1_2( void )
{
    time_t t;

    /* Get the time */
    t = time(NULL);
    if( -1 != t )
    {
        struct tm  *ptm;

        ptm = localtime( &t );
        if( NULL != ptm )
        {
            strftime( &_sl_date[0],
                      (sizeof(_sl_date)/sizeof(_sl_date[0]))-1,
                      "%d %b %Y",
                      ptm );

            _sl_printf( "\t\tLogDate\t%s\n", &_sl_date[0] );
        }
    }
}

/******************************************************************************
**
**	_sl_LogDate_1_2a
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogDate_1_2a( void )
{
    time_t t;

    /* Get the time */
    t = time(NULL);
    if( -1 != t )
    {
        struct tm  *ptm;

        ptm = localtime( &t );
        if( NULL != ptm )
        {
#ifdef WIN32
            int fUseDefMethod = 1;
            TIME_ZONE_INFORMATION tzInfo;
            DWORD                 res;
            char                  tzName[35];

            res = GetTimeZoneInformation( &tzInfo );
            switch( res )
            {
                case TIME_ZONE_ID_STANDARD:
                {
                    if( WideCharToMultiByte( CP_ACP,
                                             0L,
                                             tzInfo.StandardName,
                                             wcslen(tzInfo.StandardName),
                                             &tzName[0],
                                             sizeof(tzName),
                                             NULL,NULL ) )
                    {
                        strftime( &_sl_date[0],
                                  (sizeof(_sl_date)/sizeof(_sl_date[0]))-1,
                                  "%d %b %Y\t",
                                  ptm );


                        sprintf( &_sl_date[strlen(_sl_date)],
                                 "%s\t%ld",
                                 tzName,
                                 tzInfo.Bias );


                        fUseDefMethod = 0;
                    }
                    break;
                }

                case TIME_ZONE_ID_DAYLIGHT:
                {
                    if( WideCharToMultiByte( CP_ACP,
                                             0L,
                                             tzInfo.DaylightName,
                                             wcslen(tzInfo.DaylightName),
                                             &tzName[0],
                                             sizeof(tzName),
                                             NULL,NULL ) )
                    {
                        strftime( &_sl_date[0],
                                  (sizeof(_sl_date)/sizeof(_sl_date[0]))-1,
                                  "%d %b %Y\t",
                                  ptm );


                        sprintf( &_sl_date[strlen(_sl_date)],
                                 "%s\t%ld",
                                 tzName,
                                 tzInfo.Bias );


                        fUseDefMethod = 0;
                    }
                    break;
                }

                case TIME_ZONE_ID_UNKNOWN:
                default:
                {
                    break;
                }
            }

            if( fUseDefMethod )
                strftime( &_sl_date[0],
                          (sizeof(_sl_date)/sizeof(_sl_date[0]))-1,
                          "%d %b %Y\t%Z",
                          ptm );
#else
            strftime( &_sl_date[0],
                      (sizeof(_sl_date)/sizeof(_sl_date[0]))-1,
                      "%d %b %Y\t%Z",
                      ptm );

#if 0
            if( daylight )
                sprintf( &_sl_date[strlen(_sl_date)],
                         "\t%ld",
                         altzone );
            else
#endif
                sprintf( &_sl_date[strlen(_sl_date)],
                         "\t%ld",
                         timezone );
#endif


            _sl_printf( "\t\tLogDate\t%s\n", &_sl_date[0] );
        }
    }
}

/******************************************************************************
**
**	_sl_LogTime
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogTime( void )
{
    time_t t;

    /* Get the time */
    t = time(NULL);
    if( -1 != t )
    {
        struct tm  *ptm;

        ptm = localtime( &t );
        if( NULL != ptm )
        {
            static char time[MAX_TIME_STRLEN+1] = {0};

            strftime( &time[0],
                      (sizeof(time)/sizeof(time[0]))-1,
                      "%H:%M:%S",
                      ptm );

            _sl_printf( "\t\tLogTime\t%s\n", &time[0] );
        }
    }
}


/******************************************************************************
**
**	_sl_LogDeathFlags
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogDeathFlags( unsigned long dmFlags, const char *pString )
{
    char *pStripStr = pString?_sl_strip_cntrl(pString):NULL;

    _sl_printf( "\t\tLogDeathFlags\t%u%s%s\n",
                dmFlags,
                pString?"\t":"",
                pStripStr?pStripStr:(pString?pString:"") );

    if( pStripStr )        free( pStripStr );
}

/******************************************************************************
**
**	_sl_LogMapName
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogMapName( char *pMapName )
{
    char *pStripMap = pMapName?_sl_strip_cntrl(pMapName):NULL;

    _sl_printf( "\t\tMap\t%s\n",
                pStripMap?pStripMap:(pMapName?pMapName:"") );

    if( pStripMap )        free( pStripMap );
}

/******************************************************************************
**
**	_sl_LogPlayerName
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPlayerName( char *pPlayerName,
                               char *pTeamName,
                               float timeInSeconds )
{
    char *pStripName = _sl_strip_cntrl(pPlayerName);
    char *pStripTeam = pTeamName?_sl_strip_cntrl(pTeamName):NULL;
    
    _sl_printf( "\t\tPlayer\t%s\t%s\t%.1f\n",
                pStripName?pStripName:pPlayerName,
                pStripTeam?pStripTeam:(pTeamName?pTeamName:""),
                timeInSeconds );

    if( pStripName )        free( pStripName );
    if( pStripTeam )        free( pStripTeam );
}

/******************************************************************************
**
**	_sl_LogScore
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogScore( char *pKillerName,
                          char *pTargetName,
                          char *pScoreType,
                          char *pWeaponName,
                          int   iScore,
                          float timeInSeconds,
                          int   killerPing )
{
    char *pStripKill = pKillerName?_sl_strip_cntrl(pKillerName):NULL;
    char *pStripTarg = pTargetName?_sl_strip_cntrl(pTargetName):NULL;
    char *pStripType = pScoreType?_sl_strip_cntrl(pScoreType):NULL;
    char *pStripWeap = pWeaponName?_sl_strip_cntrl(pWeaponName):NULL;

    if( -1 == killerPing )
        _sl_printf( "%s\t%s\t%s\t%s\t%d\t%.1f\n",
                    pStripKill?pStripKill:(pKillerName?pKillerName:""),
                    pStripTarg?pStripTarg:(pTargetName?pTargetName:""),
                    pStripType?pStripType:(pScoreType?pScoreType:""),
                    pStripWeap?pStripWeap:(pWeaponName?pWeaponName:""),
                    iScore,
                    timeInSeconds );
    else
        _sl_printf( "%s\t%s\t%s\t%s\t%d\t%.1f\t%d\n",
                    pStripKill?pStripKill:(pKillerName?pKillerName:""),
                    pStripTarg?pStripTarg:(pTargetName?pTargetName:""),
                    pStripType?pStripType:(pScoreType?pScoreType:""),
                    pStripWeap?pStripWeap:(pWeaponName?pWeaponName:""),
                    iScore,
                    timeInSeconds,
                    killerPing );


    if( pStripWeap )        free( pStripWeap );
    if( pStripType )        free( pStripType );
    if( pStripTarg )        free( pStripTarg );
    if( pStripKill )        free( pStripKill );
}

/******************************************************************************
**
**	_sl_LogPlayerLeft
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPlayerLeft( char *pPlayerName,
                               float timeInSeconds )
{
    char *pStripName = pPlayerName?_sl_strip_cntrl(pPlayerName):NULL;

    _sl_printf( "\t\tPlayerLeft\t%s\t\t%.1f\n",
                pStripName?pStripName:(pPlayerName?pPlayerName:""),
                timeInSeconds );
    
    if( pStripName )        free( pStripName );
}

/******************************************************************************
**
**	_sl_LogGameStart
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogGameStart( float timeInSeconds )
{
    _sl_printf( "\t\tGameStart\t\t\t%.1f\n", timeInSeconds );
}

/******************************************************************************
**
**	_sl_LogGameEnd
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogGameEnd( float timeInSeconds )
{
    _sl_printf( "\t\tGameEnd\t\t\t%.1f\n", timeInSeconds );
}

/******************************************************************************
**
**	_sl_LogPlayerConnect
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPlayerConnect( char *pPlayerName,
                                  char *pTeamName,
                                  float timeInSeconds )
{
    char *pStripName = _sl_strip_cntrl(pPlayerName);
    char *pStripTeam = pTeamName?_sl_strip_cntrl(pTeamName):NULL;

    _sl_printf( "\t\tPlayerConnect\t%s\t%s\t%.1f\n",
                pStripName?pStripName:pPlayerName,
                pStripTeam?pStripTeam:(pTeamName?pTeamName:""),
                timeInSeconds );

    if( pStripName )        free( pStripName );
    if( pStripTeam )        free( pStripTeam );
}

/******************************************************************************
**
**	_sl_LogPlayerTeamChange
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPlayerTeamChange( char *pPlayerName,
                                     char *pTeamName,
                                     float timeInSeconds )
{
    char *pStripName = _sl_strip_cntrl(pPlayerName);
    char *pStripTeam = pTeamName?_sl_strip_cntrl(pTeamName):NULL;

    _sl_printf( "\t\tPlayerTeamChange\t%s\t%s\t%.1f\n",
                pStripName?pStripName:pPlayerName,
                pStripTeam?pStripTeam:(pTeamName?pTeamName:""),
                timeInSeconds );

    if( pStripName )        free( pStripName );
    if( pStripTeam )        free( pStripTeam );
}

/******************************************************************************
**
**	_sl_LogPlayerRename
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_LogPlayerRename( char *pOldPlayerName,
                                 char *pNewPlayerName,
                                 float timeInSeconds )
{
    char *pStripOld = _sl_strip_cntrl(pOldPlayerName);
    char *pStripNew = _sl_strip_cntrl(pNewPlayerName);

    _sl_printf( "\t\tPlayerRename\t%s\t%s\t%.1f\n",
                pStripOld?pStripOld:pOldPlayerName,
                pStripNew?pStripNew:pNewPlayerName,
                timeInSeconds );

    if( pStripOld )        free( pStripOld );
    if( pStripNew )        free( pStripNew );
}



/******************************************************************************
**
**	_sl_SetStyle
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_SetStyle( game_import_t  *gi )
{
    if( NULL == log_style_cvar )
        log_style_cvar = gi->cvar( SL_LOGSTYLE_NAME_STR, SL_LOGSTYLE_DEF_STR, 0 );

    log_style = (unsigned int)log_style_cvar->value;
    if( log_style >= (sizeof(_sl_log_styles)/sizeof(_sl_log_styles[0])) )
        log_style = SL_LOGSTYLE_DEF;
}

/*
 *  PUBLIC FUNCTIONS
 *
 *  NEW Server Logging API
 *
 */

/******************************************************************************
**
**	sl_OpenLogFile
**
**
*******************************************************************************
**
******************************************************************************/
int sl_OpenLogFile( game_import_t  *gi )
{
    /* Set URL String */
    gi->cvar_forceset(SL_URL_NAME_STR, SL_URL_DEF_STR);
    gi->cvar( SL_URL_NAME_STR, SL_URL_DEF_STR, CVAR_SERVERINFO );
    
            
    return _sl_open( gi );
}

/******************************************************************************
**
**	sl_CloseLogFile
**
**
*******************************************************************************
**
******************************************************************************/
void sl_CloseLogFile( void )
{
    _sl_close();
}

/******************************************************************************
**
**	sl_FlushLogFile
**
**
*******************************************************************************
**
******************************************************************************/
void sl_FlushLogFile( void )
{
    _sl_flush();
}

/******************************************************************************
**
**	sl_LogVers
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogVers( game_import_t  *gi )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogVers();
    }
}

/******************************************************************************
**
**	sl_LogPatch
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPatch( game_import_t  *gi,
                  char           *pPatchName )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPatch( pPatchName );
    }
}

/******************************************************************************
**
**	sl_LogDate
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogDate( game_import_t  *gi )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogDate();
    }
}

/******************************************************************************
**
**	sl_LogTime
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogTime( game_import_t  *gi )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogTime();
    }
}

/******************************************************************************
**
**	sl_LogDeathFlags
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogDeathFlags( game_import_t  *gi,
                       unsigned long   dmFlags )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogDeathFlags( dmFlags, NULL );
    }
}

/******************************************************************************
**
**	sl_LogDeathFlagsEx
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogDeathFlagsEx( game_import_t  *gi,
                         unsigned long   dmFlags,
                         const char     *pString )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogDeathFlags( dmFlags, pString );
    }
}

/******************************************************************************
**
**	sl_LogMapName
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogMapName( game_import_t  *gi,
                    char           *pMapName )
{
    /* Flush if SL_LOGFLUSH_MAPCHANGE is set */
    if( NULL == log_flush_cvar )
        log_flush_cvar = gi->cvar( SL_LOGFLUSH_NAME_STR, SL_LOGFLUSH_DEF_STR, 0 );
    if( (NULL != log_flush_cvar) &&
        (SL_LOGFLUSH_MAPCHANGE == log_flush_cvar->value) )
    {
        _sl_flush();
    }

    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogMapName( pMapName );
    }
}

/******************************************************************************
**
**	sl_LogPlayerName
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPlayerName( game_import_t  *gi,
                       char           *pPlayerName,
                       char           *pTeamName,
                       float           timeInSeconds )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPlayerName( pPlayerName, pTeamName, timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogScore
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogScore( game_import_t  *gi,
                  char           *pKillerName,
                  char           *pTargetName,
                  char           *pScoreType,
                  char           *pWeaponName,
                  int             iScore,
                  float           timeInSeconds,
                  int             killerPing )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogScore( pKillerName, pTargetName, pScoreType, pWeaponName, iScore, timeInSeconds, killerPing );
    }
}

/******************************************************************************
**
**	sl_LogPlayerLeft
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPlayerLeft( game_import_t  *gi,
                       char           *pPlayerName,
                       float           timeInSeconds )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPlayerLeft( pPlayerName, timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogGameStart
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogGameStart( game_import_t  *gi,
                      float           timeInSeconds )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogGameStart( timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogGameEnd
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogGameEnd( game_import_t  *gi,
                    float           timeInSeconds )
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogGameEnd( timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogPlayerConnect
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPlayerConnect( game_import_t  *gi,
                          char           *pPlayerName,
                          char           *pTeamName,
                          float           timeInSeconds)
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPlayerConnect( pPlayerName, pTeamName, timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogPlayerTeamChange
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPlayerTeamChange( game_import_t  *gi,
                             char           *pPlayerName,
                             char           *pTeamName,
                             float           timeInSeconds)
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPlayerTeamChange( pPlayerName, pTeamName, timeInSeconds );
    }
}

/******************************************************************************
**
**	sl_LogPlayerRename
**
**
*******************************************************************************
**
******************************************************************************/
void sl_LogPlayerRename( game_import_t  *gi,
                         char           *pOldPlayerName,
                         char           *pNewPlayerName,
                         float           timeInSeconds)
{
    if( _sl_open( gi ) )
    {
        _sl_log_styles[log_style].pLogPlayerRename( pOldPlayerName, pNewPlayerName, timeInSeconds );
    }
}




/******************************************************************************
**
**	File Base Logging Functions
**
******************************************************************************/

/******************************************************************************
**
**	_sl_file_open
**
**
*******************************************************************************
**
******************************************************************************/
static int _sl_file_open( game_import_t  *gi )
{
    if( NULL == StdLogFile )
    {
        cvar_t   *filename  = gi->cvar( SL_FILE_NAME_STR, SL_DEFFILE_NAME_STR, 0 );
        char     *pName     = SL_DEFFILE_NAME_STR;
        char      aTempStr[MAX_FILENAME_LEN+1] = {0};
        
        /*  Open File */
        if( filename )
        {
            pName = filename->string;

            if( strchr( pName, '%' ) )
            {
                time_t t;

                /* Get the time */
                t = time(NULL);
                if( -1 != t )
                {
                    struct tm  *ptm;

                    ptm = localtime( &t );
                    if( NULL != ptm )
                    {
                        strftime( &aTempStr[0],
                                  (sizeof(aTempStr)/sizeof(aTempStr[0]))-1,
                                  filename->string,
                                  ptm );
                    }

                    pName = &aTempStr[0];
                }
            }
        }
        
        StdLogFile = fopen( pName, "a+t" );

        if( NULL == StdLogFile )
        {
            gi->error( "Couldn't open %s", pName );
        }
    }

    return (NULL != StdLogFile);
}

/******************************************************************************
**
**	_sl_file_vprintf
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_file_vprintf( const char * fmt, va_list ap )
{
    if( NULL != StdLogFile )
    {
        vfprintf( StdLogFile, fmt, ap );
    }
}

static void _sl_file_flush( void )
{
    if( NULL != StdLogFile )
    {
        fflush( StdLogFile );
    }
}

/******************************************************************************
**
**	_sl_file_close
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_file_close( void )
{
    if( NULL != StdLogFile )
    {
        fclose( StdLogFile );
    }

    StdLogFile = NULL;
}


/******************************************************************************
**
**	UPD base Logging Functions
**
******************************************************************************/


static SOCKET       udp_socket   = INVALID_SOCKET;
SOCKADDR_IN         udp_ServAddr = {0};

static char        *pUDP_Buffer  = NULL;
static unsigned int udp_nBuff    = 0;

static cvar_t      *udp_buff_size_cvar = NULL;

static cvar_t      *udp_packet_num_cvar = NULL;
static DWORD        udp_packet_num = 1;

#ifdef WIN32
static WORD         udp_Version = 0;
#endif

static void _udp_buffered_send( const char *pString );
static void _sl_udp_send( const char *pString );

/******************************************************************************
**
**	_sl_udp_open
**
**
*******************************************************************************
**
******************************************************************************/
static __inline int _sl_udp_open( game_import_t  *gi )
{
    if(NULL == udp_buff_size_cvar)
        udp_buff_size_cvar = gi->cvar( SL_UDP_BUFFSIZE_STR, SL_UDP_DEFBUFFSIZE_STR, 0 );

    if( INVALID_SOCKET == udp_socket )
    {
        cvar_t   *pDest  = gi->cvar( SL_UDP_DEST_STR, "127.0.0.1:2222", 0 );
        cvar_t   *pInfo  = gi->cvar( SL_SERVER_INFO_STR, SL_DEF_SERVER_INFO, 0 );
        SOCKADDR_IN        myaddr      = {0};
        unsigned short     portnum     = 27910;
        char              *pServerAddr = pDest->string;
        char              *pServerMem  = NULL;
        int       fCont  = 1;

        /* Create Server Info String */
        if( NULL != pInfo )
        {
            char *pServInfo = _sl_add_cvars( NULL, pInfo->string, gi );
            if( NULL != pServInfo )
            {
                if( NULL != pServerInfo )
                {
                    free( pServerInfo );
                }

                pServerInfo = pServInfo;
            }
        }
        
        /* Work out Destination */

        /* copy address for manipulation */
        if( fCont )
        {
            pServerMem = malloc( strlen(pServerAddr)+1 );
            if( pServerMem )
            {
                strcpy( pServerMem, pServerAddr );
                pServerAddr = pServerMem;
            }
            else
            {
                err_printf("malloc() failed.\n");
                net_disp_err();
                fCont = 0;
            }            
        }

        /* Get port number */
        if( fCont )
        {
            char *pPort = strchr( pServerAddr, ':' );

            if( pPort )
            {
                portnum = (unsigned short)atoi((pPort+1));
                *pPort  = '\0';
            }
        }
        
#if defined(WIN32) && defined(INIT_WSA)
        if( fCont )
        {
            WSADATA wsaData;
            int     err;

            udp_Version = MAKEWORD( 2, 0 );

            err = WSAStartup( udp_Version, &wsaData );
            if ( err != 0 ) {
                /* Tell the user that we couldn't find a usable */
                /* WinSock DLL.                                  */
                err_printf("udp WSAStartup().\n");
                net_disp_err();
                fCont = 0;
            }

            /* Confirm that the WinSock DLL supports 2.0.*/
            /* Note that if the DLL supports versions greater    */
            /* than 2.0 in addition to 2.0, it will still return */
            /* 2.0 in wVersion since that is the version we      */
            /* requested.                                        */

            if( fCont )
            {
                dbg_printf("WinSock DLL Version %#.4lx.\n", wsaData.wVersion);
#ifdef CHECK_WINSOCK_VERSION
                if ( LOBYTE( wsaData.wVersion ) != 2 ||
                     HIBYTE( wsaData.wVersion ) != 0 )
                {
                    /* Tell the user that we couldn't find a usable */
                    /* WinSock DLL.                                  */
                    err_printf("udp Wrong Version %#.4lx.\n", wsaData.wVersion);
                    fCont = 0;
                }
#endif
            }
        }
#endif


        /* Create Socket */
        if( fCont )
        {
            udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (udp_socket == INVALID_SOCKET)
            {
                err_printf("udp socket() failed.\n");
                net_disp_err();
                fCont = 0;
            }
        }

        /* Bind Socket */
#ifdef BIND_SOC
        if( fCont )
        {
            myaddr.sin_family = AF_INET;
            myaddr.sin_addr.s_addr = INADDR_ANY;
            myaddr.sin_port   = 0;

            dbg_printf("bind(%d)\n", ntohs(myaddr.sin_port) );
            if( bind(udp_socket, (struct sockaddr *) &myaddr, sizeof(SOCKADDR_IN)) )
            {
                err_printf("udp bind() failed.\n");
                net_disp_err();
                return 0;
            }
        }
#endif


        /* Setup socket destination */
        if( fCont )
        {
            udp_ServAddr.sin_family      = AF_INET;
            udp_ServAddr.sin_addr.s_addr = atoaddr(pServerAddr);
            udp_ServAddr.sin_port        = htons(portnum);

#ifdef SET_SOC_DEST
            if( connect( udp_socket,
                         (struct sockaddr *)&udp_ServAddr,
                         sizeof(udp_ServAddr)) < 0 )
            {
                err_printf("udp connect() failed.\n");
                net_disp_err();
            }
#endif
        }

        /* Output Message */
        if( fCont )
        {
            SOCKADDR_IN  ServAddr = {0};
            socklen_t    namelen = sizeof(ServAddr);

            getsockname( udp_socket, (struct sockaddr *)&ServAddr, &namelen );

            inf_printf( "Using Port %d, Sending to %s port %d\n",
                        ntohs(ServAddr.sin_port),
                        pServerAddr,
                        ntohs(udp_ServAddr.sin_port) );
        }

        /* tidy up */
        if( pServerMem )
        {
            free(pServerMem);
            pServerMem = NULL;
        }

        inf_printf( "udp opened\n" );

#ifdef _DEBUG_UDP
        if( NULL == _debug_udp_file )
        {
            _debug_udp_file = fopen( "debug_udp.log", "a+b" );
        }
#endif
    }
    
    return (INVALID_SOCKET != udp_socket);
}

/******************************************************************************
**
**	_sl_udp_close
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_udp_close( void )
{
    if( INVALID_SOCKET != udp_socket )
    {
        closesocket( udp_socket );
        udp_socket = INVALID_SOCKET;
        udp_buff_size_cvar = NULL;
        
#if defined(WIN32) && defined(INIT_WSA)
        if( 0 != udp_Version )
        {
            WSACleanup();
            udp_Version = 0;
        }
#endif
        inf_printf( "udp closed\n" );        
    }

#ifdef _DEBUG_UDP
    if( _debug_udp_file )
    {
        fclose( _debug_udp_file );
        _debug_udp_file = NULL;
    }
#endif
}


/******************************************************************************
**
**	_sl_udp_vprintf
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_udp_vprintf( const char * fmt, va_list ap )
{
    if( INVALID_SOCKET != udp_socket )
    {
        char       aSend[K(4)] = { 0 };

        vsprintf( aSend, fmt, ap );

        _udp_buffered_send( &aSend[0] );
    }
}

/******************************************************************************
**
**	_sl_udp_flush
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_udp_flush( void )
{
    if( INVALID_SOCKET != udp_socket )
    {
        /* Send buffer String */
        if( NULL != pUDP_Buffer )
        {
            _sl_udp_send( pUDP_Buffer );

            /* Free Buffered String */
            free( pUDP_Buffer );
            pUDP_Buffer = NULL;
            udp_nBuff   = 0;
        }
        
#ifdef _DEBUG_UDP
        if( _debug_udp_file )
            fflush( _debug_udp_file );
#endif
    }
}

/******************************************************************************
**
**	_udp_buffered_send
**
**
*******************************************************************************
**
******************************************************************************/
static void _udp_buffered_send( const char *pString )
{
    if( (NULL == udp_buff_size_cvar) || ((unsigned int)udp_buff_size_cvar->value <= 0) )
    {
        /* Buffering turned off so
        ** send the string now */
        _sl_udp_send( pString );
    } else if( ((unsigned int)udp_buff_size_cvar->value <= udp_nBuff) ||
               ( SL_UDP_MAXBUFFSIZE <= udp_nBuff) )
    {
        /* Send buffer String */
        _sl_udp_send( pUDP_Buffer );

        /* Free Buffered String */
        if( pUDP_Buffer )
            free( pUDP_Buffer );
        pUDP_Buffer = NULL;
        udp_nBuff   = 0;
        
        /* Add String to Buffer */
        pUDP_Buffer = _sl_add_string( pUDP_Buffer, pString );
        if( NULL == pUDP_Buffer )
        {
            /* String could not be added to
            ** buffer so send the string now */
            _sl_udp_send( pString );
        }
        else
        {
            /* Increase count of lines in buffer */
            udp_nBuff++;
        }
    }
    else
    {
        /* Add String to buffer */
        char *pNewLogString = _sl_add_string( pUDP_Buffer, pString );

        if( NULL != pNewLogString )
        {
            /* String added ok */
            pUDP_Buffer = pNewLogString;
        }
        else
        {
            /* Could not add string so
            ** Send buffer String now */
            _sl_udp_send( pUDP_Buffer );

            /* Free Buffered String */
            if( pUDP_Buffer )
                free( pUDP_Buffer );
            pUDP_Buffer = NULL;
            udp_nBuff   = 0;

            /* Add String to Buffer */
            pUDP_Buffer = _sl_add_string( pUDP_Buffer, pString );
            if( NULL == pUDP_Buffer )
            {
                /* String could not be added to
                 ** buffer so send the string now */
                _sl_udp_send( pString );
            }
            else
            {
                /* Increase count of lines in buffer */
                udp_nBuff++;
            }
            
        }
    }
}


/******************************************************************************
**
**	_sl_udp_send
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_udp_send( const char *pString )
{
    if( NULL != pString )
    {
        int                 nSent;
        PACKET_BYTES       *pPacketBytes = NULL;
        unsigned long       ulPacketBytesLength = 0;
        SOCKADDR_IN         ourAddr = {0};
        int                 namelen = sizeof(ourAddr);
        PACKET_STRUCT       packetStruct = {0};

        /* Update packet number */
        if( NULL == udp_packet_num_cvar )
            udp_packet_num_cvar = pGI->cvar( SL_UDP_PACKETNUM_STR, "1", 0 );
        
        if( NULL != udp_packet_num_cvar )
            udp_packet_num = (DWORD)udp_packet_num_cvar->value;

        packetStruct.nPacket = udp_packet_num++;

        if( NULL != udp_packet_num_cvar )
        {
            char aNumber[100];

            itoa( udp_packet_num, &aNumber[0], 10 );
            pGI->cvar_set(SL_UDP_PACKETNUM_STR, &aNumber[0]);
        }
        
        packetStruct.pServerInfo  = pServerInfo?pServerInfo:"";
        packetStruct.pLogString   = (char*)pString;

        _sl_create_clientinfos( &packetStruct );
        
        if( SL_ERROR_NONE == sl_ConstructPacketBytes( &pPacketBytes,
            &ulPacketBytesLength,
            &packetStruct,
            SL_FLAGS_NONE ) )
        {
            dbg_printf( "nPacket = %#lx %#lx (%#lx)\n",
                        packetStruct.nPacket,
                        (DWORD)*(pPacketBytes+4),
                        udp_packet_num );
                          
#ifdef SET_SOC_DEST
            nSent = send( udp_socket,
                          pPacketBytes,
                          ulPacketBytesLength,
                          0 );
#else
            nSent = sendto( udp_socket,
                            pPacketBytes,
                            ulPacketBytesLength,
                            0,
                            (struct sockaddr *)&udp_ServAddr,
                            sizeof(udp_ServAddr) );
#endif

#ifdef _DEBUG_UDP
            if( _debug_udp_file )
                fwrite( pPacketBytes, 1, ulPacketBytesLength, _debug_udp_file );
#endif
            
            if( nSent != ulPacketBytesLength )
            {
                err_printf("send failed.\n");
                net_disp_err();
            }

            sl_DestroyPacketBytes( pPacketBytes,
                                   ulPacketBytesLength );
        }
        else
        {
            err_printf("ConstuctPacketBytes() failed.\n");
        }

        _sl_free_clientinfos( &packetStruct );
    }
}


/******************************************************************************
**
**	TCP base Logging Functions
**
******************************************************************************/


static SOCKET       tcp_socket   = INVALID_SOCKET;
SOCKADDR_IN         tcp_ServAddr = {0};


#ifdef WIN32
static WORD         tcp_Version = 0;
#endif

/******************************************************************************
**
**	_sl_tcp_open
**
**
*******************************************************************************
**
******************************************************************************/
static __inline int _sl_tcp_open( game_import_t  *gi )
{
    if( INVALID_SOCKET == tcp_socket )
    {
        cvar_t   *pDest  = gi->cvar( SL_TCP_DEST_STR, "127.0.0.1:2222", 0 );
        SOCKADDR_IN        myaddr      = {0};
        unsigned short     portnum     = 27910;
        char              *pServerAddr = pDest->string;
        char              *pServerMem  = NULL;
        int       fCont  = 1;

        /* Work out Destination */

        /* copy address for manipulation */
        if( fCont )
        {
            pServerMem = malloc( strlen(pServerAddr)+1 );
            if( pServerMem )
            {
                strcpy( pServerMem, pServerAddr );
                pServerAddr = pServerMem;
            }
            else
            {
                err_printf("malloc() failed.\n");
                net_disp_err();
                fCont = 0;
            }            
        }

        /* Get port number */
        if( fCont )
        {
            char *pPort = strchr( pServerAddr, ':' );

            if( pPort )
            {
                portnum = (unsigned short)atoi((pPort+1));
                *pPort  = '\0';
            }
        }

#if defined(WIN32) && defined(INIT_WSA)
        if( fCont )
        {
            WSADATA wsaData;
            int     err;

            tcp_Version = MAKEWORD( 2, 0 );

            err = WSAStartup( tcp_Version, &wsaData );
            if ( err != 0 ) {
                /* Tell the user that we couldn't find a usable */
                /* WinSock DLL.                                  */
                err_printf("tcp WSAStartup().\n");
                net_disp_err();
                fCont = 0;
            }

            /* Confirm that the WinSock DLL supports 2.0.*/
            /* Note that if the DLL supports versions greater    */
            /* than 2.0 in addition to 2.0, it will still return */
            /* 2.0 in wVersion since that is the version we      */
            /* requested.                                        */

            if( fCont )
            {
                dbg_printf("WinSock DLL Version %#.4lx.\n", wsaData.wVersion);
#ifdef CHECK_WINSOCK_VERSION
                if ( LOBYTE( wsaData.wVersion ) != 2 ||
                     HIBYTE( wsaData.wVersion ) != 0 )
                {
                    /* Tell the user that we couldn't find a usable */
                    /* WinSock DLL.                                  */
                    err_printf("tcp Wrong Version %#.4lx.\n", wsaData.wVersion);
                    fCont = 0;
                }
#endif
            }
        }
#endif


        /* Create Socket */
        if( fCont )
        {
            tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (tcp_socket == INVALID_SOCKET)
            {
                err_printf("tcp socket() failed.\n");
                net_disp_err();
                fCont = 0;
            }
        }

#ifndef TCP_ALLOW_BUFFER
        if( fCont )
        {
            BOOL fFlag = TRUE;

            if( setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY,
                           (const char*) &fFlag, sizeof(fFlag)) )
            {

                err_printf("setsockopt(TCP_NODELAY) failed.\n");
                net_disp_err();
                fCont = 0;
            }
        }
#endif
        
        /* Bind Socket */
#ifdef BIND_SOC
        if( fCont )
        {
            myaddr.sin_family = AF_INET;
            myaddr.sin_addr.s_addr = INADDR_ANY;
            myaddr.sin_port   = 0;

            dbg_printf("bind(%d)\n", ntohs(myaddr.sin_port) );
            if( bind(tcp_socket, (struct sockaddr *) &myaddr, sizeof(SOCKADDR_IN)) )
            {
                err_printf("tcp bind() failed.\n");
                net_disp_err();
                return 0;
            }
        }
#endif


        /* Setup socket destination */
        if( fCont )
        {
            tcp_ServAddr.sin_family      = AF_INET;
            tcp_ServAddr.sin_addr.s_addr = atoaddr(pServerAddr);
            tcp_ServAddr.sin_port        = htons(portnum);

#ifdef SET_SOC_DEST
            if( connect( tcp_socket,
                         (struct sockaddr *)&tcp_ServAddr,
                         sizeof(tcp_ServAddr)) < 0 )
            {
                err_printf("tcp connect() failed.\n");
                net_disp_err();
            }
#endif
        }

        /* Output Message */
        if( fCont )
        {
            SOCKADDR_IN         ServAddr = {0};
            socklen_t           namelen = sizeof(ServAddr);

            getsockname( tcp_socket, (struct sockaddr *)&ServAddr, &namelen );

            inf_printf( "Using Port %d, Sending to %s port %d\n",
                        ntohs(ServAddr.sin_port),
                        pServerAddr,
                        ntohs(tcp_ServAddr.sin_port) );
        }

        /* tidy up */
        if( pServerMem )
        {
            free(pServerMem);
            pServerMem = NULL;
        }

        inf_printf( "tcp opened\n" );        
    }

    return (INVALID_SOCKET != tcp_socket);
}

/******************************************************************************
**
**	_sl_tcp_close
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_tcp_close( void )
{
    if( INVALID_SOCKET != tcp_socket )
    {
        closesocket( tcp_socket );
        tcp_socket = INVALID_SOCKET;

#if defined(WIN32) && defined(INIT_WSA)
        if( 0 != tcp_Version )
        {
            WSACleanup();
            tcp_Version = 0;
        }
#endif
        inf_printf( "tcp closed\n" );        
    }
}


/******************************************************************************
**
**	_sl_tcp_vprintf
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_tcp_vprintf( const char * fmt, va_list ap )
{
    if( INVALID_SOCKET != tcp_socket )
    {
        int           nSent;
        unsigned int  nSend;
        char          aSend[K(4)] = { 0 };

        vsprintf( aSend, fmt, ap );

        nSend = strlen(aSend);

        dbg_printf("send %d bytes - %s", nSend, &aSend[0] );

#ifdef SET_SOC_DEST
        nSent = send( tcp_socket,
                      &aSend[0],
                      nSend,
                      0 );
#else
        nSent = sendto( tcp_socket,
                        &aSend[0],
                        nSend,
                        0,
                        (struct sockaddr *)&tcp_ServAddr,
                        sizeof(tcp_ServAddr) );
#endif
        if( nSent != nSend )
        {
            err_printf("send failed.\n");
            net_disp_err();
        }
    }
}

/******************************************************************************
**
**	_sl_tcp_flush
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_tcp_flush( void )
{
    if( INVALID_SOCKET != tcp_socket )
    {
    }    
}


/******************************************************************************
**
**	Logging functions
**
**
*******************************************************************************
**
******************************************************************************/


/******************************************************************************
**
**	_sl_open
**
**
*******************************************************************************
**
******************************************************************************/
static __inline int _sl_open( game_import_t  *gi )
{
    int fOK = 1;

    /* Store this for later */
    pGI = gi;

    if( NULL == log_method_cvar )
        log_method_cvar = gi->cvar( SL_LOGOUT_NAME_STR, "0", 0 );

    _sl_SetStyle( gi );

    if( SL_LOGOUT_FILE(log_method_cvar->value) )
        fOK = _sl_file_open( gi );

    if( SL_LOGOUT_UDP(log_method_cvar->value) )
        fOK = _sl_udp_open( gi );

    if( SL_LOGOUT_TCP(log_method_cvar->value) )
        fOK = _sl_tcp_open( gi );
    
    return fOK;
}

/******************************************************************************
**
**	_sl_printf
**
**
*******************************************************************************
**
******************************************************************************/
static __inline void _sl_printf( const char * fmt, ... )
{
    va_list ap;
    va_start(ap,fmt);

    _sl_file_vprintf( fmt, ap );
    _sl_udp_vprintf( fmt, ap );
    _sl_tcp_vprintf( fmt, ap );

    va_end(ap);

    /* Flush if SL_LOGFLUSH_ALWAYS is set */
    if( NULL == log_flush_cvar )
        log_flush_cvar = pGI->cvar( SL_LOGFLUSH_NAME_STR, SL_LOGFLUSH_DEF_STR, 0 );
    if( (NULL != log_flush_cvar) &&
        (SL_LOGFLUSH_ALWAYS == log_flush_cvar->value) )
    {
        _sl_flush();
    }
}

/******************************************************************************
**
**	_sl_flush
**
**
*******************************************************************************
**
******************************************************************************/
static void __inline _sl_flush( void )
{
    _sl_file_flush();
    _sl_udp_flush();
    _sl_tcp_flush();
}

/******************************************************************************
**
**	_sl_close
**
**
*******************************************************************************
**
******************************************************************************/
static void __inline _sl_close( void )
{
    /* Flush any unsent data */
    _sl_flush();

    /* close */
    _sl_file_close();
    _sl_udp_close();
    _sl_tcp_close();

    /* reset */
    log_method_cvar  = NULL;
    log_style_cvar   = NULL;
    log_style        = SL_LOGSTYLE_DEF;
}







/******************************************************************************
**
**	Network Helper Functions
**
******************************************************************************/

#ifdef WIN32
struct socErrs
{
    int    err;
    char  *pString;
};

static const struct socErrs aScocketErrors[] =
{
    { WSAEACCES,          "WSAEACCES" },
    { WSAEADDRINUSE,      "WSAEADDRINUSE" },
    { WSAEADDRNOTAVAIL,   "WSAEADDRNOTAVAIL" },
    { WSAEAFNOSUPPORT,    "WSAEAFNOSUPPORT" },
    { WSAEALREADY,        "WSAEALREADY" },
    { WSAECONNREFUSED,    "WSAECONNREFUSED" },
    { WSAEFAULT,          "WSAEFAULT" },
    { WSAEINPROGRESS,     "WSAEINPROGRESS" },
    { WSAEINTR,           "WSAEINTR" },
    { WSAEINVAL,          "WSAEINVAL" },
    { WSAEISCONN,         "WSAEISCONN" },
    { WSAEMFILE,          "WSAEMFILE" },
    { WSAENETDOWN,        "WSAENETDOWN" },
    { WSAENETUNREACH,     "WSAENETUNREACH" },
    { WSAENOBUFS,         "WSAENOBUFS" },
    { WSAENOTSOCK,        "WSAENOTSOCK" },
    { WSAEPROTONOSUPPORT, "WSAEPROTONOSUPPORT" },
    { WSAEPROTOTYPE,      "WSAEPROTOTYPE" },
    { WSAESOCKTNOSUPPORT, "WSAESOCKTNOSUPPORT" },
    { WSAETIMEDOUT,       "WSAETIMEDOUT" },
    { WSAEWOULDBLOCK,     "WSAEWOULDBLOCK" },
    { WSANOTINITIALISED,  "WSANOTINITIALISED" },

    { WSAEINTR,             "WSAEINTR" },
    { WSAEBADF,             "WSAEBADF" },
    { WSAEACCES,            "WSAEACCES" },
    { WSAEFAULT,            "WSAEFAULT" },
    { WSAEINVAL,            "WSAEINVAL" },
    { WSAEMFILE,            "WSAEMFILE" },
    { WSAEWOULDBLOCK,       "WSAEWOULDBLOCK" },
    { WSAEINPROGRESS,       "WSAEINPROGRESS" },
    { WSAEALREADY,          "WSAEALREADY" },
    { WSAENOTSOCK,          "WSAENOTSOCK" },
    { WSAEDESTADDRREQ,      "WSAEDESTADDRREQ" },
    { WSAEMSGSIZE,          "WSAEMSGSIZE" },
    { WSAEPROTOTYPE,        "WSAEPROTOTYPE" },
    { WSAENOPROTOOPT,       "WSAENOPROTOOPT" },
    { WSAEPROTONOSUPPORT,   "WSAEPROTONOSUPPORT" },
    { WSAESOCKTNOSUPPORT,   "WSAESOCKTNOSUPPORT" },
    { WSAEOPNOTSUPP,        "WSAEOPNOTSUPP" },
    { WSAEPFNOSUPPORT,      "WSAEPFNOSUPPORT" },
    { WSAEAFNOSUPPORT,      "WSAEAFNOSUPPORT" },
    { WSAEADDRINUSE,        "WSAEADDRINUSE" },
    { WSAEADDRNOTAVAIL,     "WSAEADDRNOTAVAIL" },
    { WSAENETDOWN,          "WSAENETDOWN" },
    { WSAENETUNREACH,       "WSAENETUNREACH" },
    { WSAENETRESET,         "WSAENETRESET" },
    { WSAECONNABORTED,      "WSAECONNABORTED" },
    { WSAECONNRESET,        "WSAECONNRESET" },
    { WSAENOBUFS,           "WSAENOBUFS" },
    { WSAEISCONN,           "WSAEISCONN" },
    { WSAENOTCONN,          "WSAENOTCONN" },
    { WSAESHUTDOWN,         "WSAESHUTDOWN" },
    { WSAETOOMANYREFS,      "WSAETOOMANYREFS" },
    { WSAETIMEDOUT,         "WSAETIMEDOUT" },
    { WSAECONNREFUSED,      "WSAECONNREFUSED" },
    { WSAELOOP,             "WSAELOOP" },
    { WSAENAMETOOLONG,      "WSAENAMETOOLONG" },
    { WSAEHOSTDOWN,         "WSAEHOSTDOWN" },
    { WSAEHOSTUNREACH,      "WSAEHOSTUNREACH" },
    { WSAENOTEMPTY,         "WSAENOTEMPTY" },
    { WSAEPROCLIM,          "WSAEPROCLIM" },
    { WSAEUSERS,            "WSAEUSERS" },
    { WSAEDQUOT,            "WSAEDQUOT" },
    { WSAESTALE,            "WSAESTALE" },
    { WSAEREMOTE,           "WSAEREMOTE" },
    { WSAEDISCON,           "WSAEDISCON" },

    { SOCKET_ERROR,         "SOCKET_ERROR" },

    { 0, NULL }
};


/******************************************************************************
**
**	net_disp_err_num
**
**
*******************************************************************************
**
******************************************************************************/
static void net_disp_err_num(int err)
{
    const struct socErrs *a = aScocketErrors;

    while( a->pString && (a->err != err) )
        a++;

    if( a->pString )
        err_printf("%s.\n", a->pString);
    else
        err_printf("<unknown error> [%#lx].\n", err);
}


/******************************************************************************
**
**	net_disp_err
**
**
*******************************************************************************
**
******************************************************************************/
static void net_disp_err(void)
{
    int err;
    err = WSAGetLastError ();
    net_disp_err_num( err );
}

#endif



/******************************************************************************
**
**	atoaddr
**
**
*******************************************************************************
**
******************************************************************************/
static unsigned long atoaddr(char *address)
{
    struct hostent *host;
    struct in_addr saddr;

    /* First try it as aaa.bbb.ccc.ddd. */
    saddr.s_addr = inet_addr(address);
    if (saddr.s_addr != -1) {
        return saddr.s_addr;
    }
    host = gethostbyname(address);
    if (host != NULL) {
        return ((struct in_addr *) *host->h_addr_list)->s_addr;
    }
    return 0L;
}



/******************************************************************************
**
**	_sl_add_cvars
**
**
*******************************************************************************
**
******************************************************************************/
static char *_sl_add_cvars( char *pCurrent, char *pVarNameList, game_import_t  *gi )
{
    char    aCVarName[50];
    char   *pCVarNameS = pVarNameList;
    char   *pCVarNameE = " "; /* " " is just here to force entry into the while loop */

    while( ('\0' != *pCVarNameE) &&
           (pCVarNameE != pCVarNameS) )
    {
        char   *pValue = NULL;
        cvar_t *pCVar;

        /* skip spaces */
        while( ' ' == *pCVarNameS )
            pCVarNameS++;

        pCVarNameE = strchr( pCVarNameS, ' ' );
        if( NULL == pCVarNameE )
            pCVarNameE = pCVarNameS + strlen(pCVarNameS);

        if( pCVarNameE != pCVarNameS )
        {
            strncpy( &aCVarName[0], pCVarNameS, pCVarNameE - pCVarNameS );
            aCVarName[pCVarNameE - pCVarNameS] = '\0';

            if( ('$' == aCVarName[0]) &&
                ('$' == aCVarName[0]) )
            {
                /* Special Sequence */
                static char aTemp[MAX_QPATH];
                static char aHostName[MAX_QPATH] = "";
                static char aHostIP[MAX_QPATH] = "";

                if( !strcmp( "$HOST_NAME$", aCVarName ) )
                {
                    /* Host name requested */
                    if( ('\0' != aHostName[0]) ||
                        (0 == gethostname( aHostName, MAX_QPATH )) )
                        pValue = aHostName;
                }
                else if( !strcmp( "$HOST_IP$", aCVarName ) )
                {
                    /* host ip requested */
                    if( ('\0' != aHostName[0]) ||
                        (0 == gethostname( aHostName, MAX_QPATH )) )
                    {
                        if( '\0' != aHostIP[0] )
                        {
                            pValue = aHostIP;
                        }
                        else
                        {
                            struct hostent *pHost;

                            pHost = gethostbyname(aHostName);
                            if (NULL != pHost)
                            {
                                struct in_addr addr;

                                memcpy( &addr, *pHost->h_addr_list, sizeof(addr) );
                                pValue = inet_ntoa( addr );
                                if( pValue )
                                {
                                    strcpy( aHostIP, pValue );
                                    pValue = aHostIP;
                                }
                            }
                        }
                    }
                }
                else
                {
                    /* unknown special */
                }
            }
            else
            {
                /* normal client cvar name */
                pCVar = gi->cvar( &aCVarName[0], "", 0 );
                if( pCVar )
                    pValue = pCVar->string;
            }
            
            if( pValue )
            {
                pCurrent = _sl_add_string( pCurrent, aCVarName );
                pCurrent = _sl_add_string( pCurrent, " \"" );
                pCurrent = _sl_add_string( pCurrent, pValue );
                pCurrent = _sl_add_string( pCurrent, "\"\n" );
            }

            pCVarNameS = pCVarNameE+1;
        }
    }
    
    return pCurrent;
}


/******************************************************************************
**
**	_sl_add_string
**
**
*******************************************************************************
**
******************************************************************************/
static char *_sl_add_string( char *pCurrent, const char *pString )
{
    size_t len1 = (NULL != pCurrent)?strlen( pCurrent ):0;
    size_t len2 = strlen( pString );
    char  *pNew;

    pNew = (char*)malloc( len1+len2+1 );
    if( pNew )
    {
        if( NULL != pCurrent )
        {
            strcpy( pNew, pCurrent );
            free( pCurrent );
        }
        
        strcpy( pNew+len1, pString );
        
        pCurrent = pNew;
    }
    
    return pCurrent;
}

/******************************************************************************
**
**	_sl_strip_cntrl
**
**
*******************************************************************************
**
******************************************************************************/
static char *_sl_strip_cntrl( const char *pOld )
{
    char *pNew = (char *)malloc( strlen(pOld)+1 );
    if( NULL != pNew )
    {
        char *pPoint = pNew;
        
        strcpy( pNew, pOld );
        while( '\0' != *pPoint )
        {
            /* strip control chars */
            if( iscntrl(*pPoint) )
                *pPoint = ' ';

            pPoint++;
        }
        
        return pNew;
    }

    return NULL;
}


/******************************************************************************
**
**	_sl_add_client_cvars
**
**
*******************************************************************************
**
******************************************************************************/
static char *_sl_add_client_cvars( char *pCurrent,
                                   const char *pVarNameList,
                                   char *pClientInfo )
{
    char         aCVarName[50];
    const char  *pCVarNameS = pVarNameList;
    const char  *pCVarNameE = " "; /* " " is just here to force entry into the while loop */

    while( ('\0' != *pCVarNameE) &&
           (pCVarNameE != pCVarNameS) )
    {
        const char   *pCVar;

        /* skip spaces */
        while( ' ' == *pCVarNameS )
            pCVarNameS++;

        pCVarNameE = strchr( pCVarNameS, ' ' );
        if( NULL == pCVarNameE )
            pCVarNameE = pCVarNameS + strlen(pCVarNameS);

        if( pCVarNameE != pCVarNameS )
        {
            strncpy( &aCVarName[0], pCVarNameS, pCVarNameE - pCVarNameS );
            aCVarName[pCVarNameE - pCVarNameS] = '\0';

            pCVar = Info_ValueForKey( pClientInfo, aCVarName );
            if( pCVar && (*pCVar != '\0') )
            {
                pCurrent = _sl_add_string( pCurrent, aCVarName );
                pCurrent = _sl_add_string( pCurrent, " \"" );
                pCurrent = _sl_add_string( pCurrent, pCVar );
                pCurrent = _sl_add_string( pCurrent, "\"\n" );
            }

            pCVarNameS = pCVarNameE+1;
        }
    }

    return pCurrent;
}

static char* _pClientInfoStr[MAX_CLIENTS] = {0};
static char* _pOldClientInfoStr[MAX_CLIENTS] = {0};
unsigned int _nOldClients = 0;

/******************************************************************************
**
**	_sl_create_clientinfos
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_update_ClientInfoStr( char         **_ppClientInfoStr,
                                      unsigned int   i,
                                      const edict_t *cl_ent,
                                      const char    *pInfoList )

{
        /* Update Global Client Info String */
    if( _pClientInfoStr[i] )
    {
        free(_ppClientInfoStr[i]);
        _ppClientInfoStr[i] = NULL;
    }

#if 0
    {
        char aTemp[20];

        sprintf( aTemp, "%d", i );
        
        _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], "i" );
        _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], " \"" );
        _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], aTemp );
        _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], "\"\n" );
    }
#endif
    
    _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], "netname" );
    _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], " \"" );
    _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], &game.clients[i].pers.netname[0] );
    _ppClientInfoStr[i] = _sl_add_string( _ppClientInfoStr[i], "\"\n" );
    
    _ppClientInfoStr[i] = _sl_add_client_cvars( _ppClientInfoStr[i],
                                               pInfoList,
                                               &game.clients[i].pers.userinfo[0] );
}


/******************************************************************************
**
**	_sl_create_clientinfos
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_update_ClientInfoStrs( char **_ppClientInfoStr )

{
    cvar_t   *pInfo  = pGI->cvar( SL_CLIENT_INFO_STR, SL_DEF_CLIENT_INFO, 0 );
    edict_t		*cl_ent;
    unsigned int i;

    for (i=0 ; i<game.maxclients ; i++)
    {
        cl_ent = g_edicts + 1 + i;
        if (!cl_ent->inuse)
        {
            if( _pClientInfoStr[i] )
            {
                free(_pClientInfoStr[i]);
                _pClientInfoStr[i] = NULL;
            }
        }
        else
            _sl_update_ClientInfoStr( _ppClientInfoStr, i, cl_ent, pInfo->string );
    }
}


/******************************************************************************
**
**	_sl_create_clientinfos
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_create_clientinfos( PACKET_STRUCT *pPacketStruct )
{
    char *pStrings[2*MAX_CLIENTS] = {0};
    unsigned int i, maxC;
    unsigned int nClients = 0;

    /* Update Global Client Info Strings */

    /* Create New one */
    _sl_update_ClientInfoStrs( _pClientInfoStr );

    
    /* Update Local List  - merge old and new */

    maxC = _nOldClients;
    if( _nOldClients < game.maxclients )
        maxC = game.maxclients;
    
    for (i=0 ; i<maxC ; i++)
    {
        int fAddOld = 0;
        int fAddNew = 0;

        if( i < _nOldClients )
            fAddOld = (NULL != _pOldClientInfoStr[i]);
        
        if( i < game.maxclients )
            fAddNew = (NULL != _pClientInfoStr[i]);

        if( fAddNew && fAddOld )
            if( !strcmp( _pOldClientInfoStr[i], _pClientInfoStr[i] ) )
                fAddOld = 0;

        if( fAddOld )
        {
            pStrings[nClients] = _pOldClientInfoStr[i];
            nClients++;
        }

        if( fAddNew )
        {
            pStrings[nClients] = _pClientInfoStr[i];
            nClients++;
        }
    }


    /* Update Packet Structure */
    pPacketStruct->nClients = nClients;
    
    pPacketStruct->ppClientInfo = (char**)malloc( sizeof(*(pPacketStruct->ppClientInfo)) * (nClients + 1) );
    if( pPacketStruct->ppClientInfo )
    {
        for (i=0 ; i<nClients ; i++)
            pPacketStruct->ppClientInfo[i] = pStrings[i];

        pPacketStruct->ppClientInfo[i] = NULL;
    }
    else
    {
        /* malloc error - free strings*/
        for (i=0 ; i<game.maxclients ; i++)
            if( pStrings[i] )
                free(pStrings[i]);
    }
}

/******************************************************************************
**
**	_sl_free_clientinfos
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_free_clientinfos( PACKET_STRUCT *pPacketStruct )
{
    unsigned int i;

    /* free old strings one */
    for( i=0; i<_nOldClients; i++ )
        if( _pOldClientInfoStr[i] )
        {
            free(_pOldClientInfoStr[i]);
            _pOldClientInfoStr[i] = NULL;
        }

    /* Copy new to old one and clear */
    memcpy( _pOldClientInfoStr, _pClientInfoStr, sizeof(_pClientInfoStr) );
    memset( _pClientInfoStr, 0, sizeof(_pClientInfoStr) );
    _nOldClients = game.maxclients;

    pPacketStruct->nClients = 0;
    if( pPacketStruct->ppClientInfo )
        free( pPacketStruct->ppClientInfo );
}


/* end of file */
