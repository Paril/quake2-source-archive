/*
 *  Standard Logging functions
 *
 *  Public header file.
 *
 * $Id: stdlog.c 1.5 1998/04/06 09:23:17 mdavies Exp $
 *
 */

#include <stdlib.h>
#include <time.h>

#include "g_local.h"

#include "stdlog.h"


/*
 *  LOCAL DEFINES
 *
 */

#define MAX_DATE_STRLEN     (2+1+3+1+4+10)   /* dd mmm yyyy (10 for good luck) */
#define MAX_TIME_STRLEN     (2+1+2+1+2+10)   /* hh:mm:ss (10 for good luck) */

/*
 *  LOCAL VARIABLES
 *
 */
static cvar_t   *logfile    = NULL;
static cvar_t   *logstyle   = NULL;
static FILE     *StdLogFile = NULL;

static unsigned int uiLogstyle = 0;

static const char *_unused_id_stdlog_c = "$Id: stdlog.c 1.5 1998/04/06 09:23:17 mdavies Exp $";
static const char *_unused_id_stdlog_h = __STDLOG_ID__;

/*
 *  LOCAL FUNCTION PROTOTYPES
 *
 */
static void _sl_LogVers( void );
static void _sl_LogPatch( char *pPatchName );
static void _sl_LogDate( void );
static void _sl_LogTime( void );
static void _sl_LogDeathFlags( unsigned long dmFlags );
static void _sl_LogMapName( char *pMapName );

static void _sl_LogPlayerName( char *pPlayerName,
                               char *pTeamName,
                               float timeInSeconds );

static void _sl_LogScore( char *pKillerName,
                          char *pTargetName,
                          char *pScoreType,
                          char *pWeaponName,
                          int   iScore,
                          float timeInSeconds );

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

static int _sl_MaybeOpenFile( game_import_t  *gi );
static void  _sl_MaybeCloseFile( void );

/*
 *  LOCAL TYPEDEFS
 *
 */

#if 1
typedef struct
{
    void (*pLogVers)( void );
    void (*pLogPatch)( char *pPatchName );
    void (*pLogDate)( void );
    void (*pLogTime)( void );
    void (*pLogDeathFlags)( unsigned long dmFlags );
    void (*pLogMapName)( char *pMapName );
    void (*pLogPlayerName)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogScore)( char *pKillerName, char *pTargetName, char *pScoreType, char *pWeaponName, int   iScore, float timeInSeconds );
    void (*pLogPlayerLeft)( char *pPlayerName, float timeInSeconds );
    void (*pLogGameStart)( float timeInSeconds );
    void (*pLogGameEnd)( float timeInSeconds );
    void (*pLogPlayerConnect)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogPlayerTeamChange)( char *pPlayerName, char *pTeamName, float timeInSeconds );
    void (*pLogPlayerRename)( char *pOldPlayerName, char *pNewPlayerName, float timeInSeconds );
} LOG_FUNCS;

static LOG_FUNCS _sl_LogStyles[] =
{
    {
        _sl_LogVers,
        _sl_LogPatch,
        _sl_LogDate,
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
#endif

/*
 *  PRIVATE FUNCTIONS
 *
 *
 *
 */

static void _sl_LogVers( void )
{
    /* StdLog + VersNr */
    fprintf( StdLogFile, "\t\tStdLog\t1.2\n" );
}

static void _sl_LogPatch( char *pPatchName )
{
    if( NULL != pPatchName )
    {
        fprintf( StdLogFile, "\t\tPatchName\t%s\n", pPatchName );
    }
    else
    {
        fprintf( StdLogFile, "\t\tPatchName\t\n" );
    }
}

static void _sl_LogDate( void )
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
            char        date[MAX_DATE_STRLEN+1] = {0};

            strftime( &date[0],
                      (sizeof(date)/sizeof(date[0]))-1,
                      "%d %b %Y",
                      ptm );

            fprintf( StdLogFile, "\t\tLogDate\t%s\n", &date[0] );
        }
    }
}

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
            char        time[MAX_TIME_STRLEN+1] = {0};

            strftime( &time[0],
                      (sizeof(time)/sizeof(time[0]))-1,
                      "%H:%M:%S",
                      ptm );

            fprintf( StdLogFile, "\t\tLogTime\t%s\n", &time[0] );
        }
    }
}


static void _sl_LogDeathFlags( unsigned long dmFlags )
{
    fprintf( StdLogFile, "\t\tLogDeathFlags\t%u\n", dmFlags );
}

static void _sl_LogMapName( char *pMapName )
{
    fprintf( StdLogFile, "\t\tMap\t%s\n", pMapName );
}

static void _sl_LogPlayerName( char *pPlayerName,
                               char *pTeamName,
                               float timeInSeconds )
{
    if( NULL != pTeamName )
        fprintf( StdLogFile, "\t\tPlayer\t%s\t%s\t%.1f\n", pPlayerName, pTeamName, timeInSeconds );
    else
        fprintf( StdLogFile, "\t\tPlayer\t%s\t\t%.1f\n", pPlayerName, timeInSeconds );
}

static void _sl_LogScore( char *pKillerName,
                          char *pTargetName,
                          char *pScoreType,
                          char *pWeaponName,
                          int   iScore,
                          float timeInSeconds )
{
    /* Killer Name */
    if( NULL != pKillerName )
        fprintf( StdLogFile, "%s", pKillerName );
    fprintf( StdLogFile, "\t" );

    /* Target Name */
    if( NULL != pTargetName )
        fprintf( StdLogFile, "%s", pTargetName );
    fprintf( StdLogFile, "\t" );
    
    /* Score Type */
    if( NULL != pScoreType )
        fprintf( StdLogFile, "%s", pScoreType );
    fprintf( StdLogFile, "\t" );

    /* Weapon Name */
    if( NULL != pWeaponName )
        fprintf( StdLogFile, "%s", pWeaponName );
    fprintf( StdLogFile, "\t" );

    /* Score & Time */
    fprintf( StdLogFile, "%d\t%.1f\n", iScore, timeInSeconds );
}

static void _sl_LogPlayerLeft( char *pPlayerName,
                               float timeInSeconds )
{
    fprintf( StdLogFile, "\t\tPlayerLeft\t%s\t\t%.1f\n", pPlayerName, timeInSeconds );
}

static void _sl_LogGameStart( float timeInSeconds )
{
    fprintf( StdLogFile, "\t\tGameStart\t\t\t%.1f\n", timeInSeconds );
}

static void _sl_LogGameEnd( float timeInSeconds )
{
    fprintf( StdLogFile, "\t\tGameEnd\t\t\t%.1f\n", timeInSeconds );
}

static void _sl_LogPlayerConnect( char *pPlayerName,
                                  char *pTeamName,
                                  float timeInSeconds )
{
    if( NULL != pTeamName )
        fprintf( StdLogFile, "\t\tPlayerConnect\t%s\t%s\t%.1f\n", pPlayerName, pTeamName, timeInSeconds );
    else
        fprintf( StdLogFile, "\t\tPlayerConnect\t%s\t\t%.1f\n", pPlayerName, timeInSeconds );
}

static void _sl_LogPlayerTeamChange( char *pPlayerName,
                                     char *pTeamName,
                                     float timeInSeconds )
{
    if( NULL != pTeamName )
        fprintf( StdLogFile, "\t\tPlayerTeamChange\t%s\t%s\t%.1f\n", pPlayerName, pTeamName, timeInSeconds );
    else
        fprintf( StdLogFile, "\t\tPlayerTeamChange\t%s\t\t%.1f\n", pPlayerName, timeInSeconds );
}

static void _sl_LogPlayerRename( char *pOldPlayerName,
                                 char *pNewPlayerName,
                                 float timeInSeconds )
{
    fprintf( StdLogFile, "\t\tPlayerRename\t%s\t%s\t%.1f\n", pOldPlayerName, pNewPlayerName, timeInSeconds );
}


static int _sl_MaybeOpenFile( game_import_t  *gi )
{
    if( NULL == logfile )
        logfile = gi->cvar( "stdlogfile", "0", CVAR_SERVERINFO );

    if( (NULL != logfile) && (logfile->value != 0) )
    {
        if( NULL == StdLogFile )
        {
            cvar_t   *filename  = gi->cvar( "stdlogname", "ctc/StdLog.log", CVAR_SERVERINFO );

			/* Added by Chris Richards (cjr@tivoli.com) */
			/* cvars must always have to have unix path names */
#ifdef _WIN32
            char*   pName    = "ctc\\StdLog.log";
#else ifdef UNIX
            char*   pName    = "ctc/stdlog.log";
#endif
           
            // Open File
            if( filename )
                pName = filename->string;
            
            StdLogFile = fopen( pName, "a+t" );

            if( NULL == StdLogFile )
            {
                gi->error( "Couldn't open %s", pName );
            }
        }
    }

    return (NULL != StdLogFile);
}

static void _sl_MaybeCloseFile( void )
{
    if( NULL != logfile )
    {
        fclose( StdLogFile );
    }

    StdLogFile = NULL;
    logfile    = NULL;
    logstyle   = NULL;
    uiLogstyle  = 0;
}

static __inline void _sl_SetStyle( game_import_t  *gi )
{
    if( NULL == logstyle )
    {
        logstyle = gi->cvar( "stdlogstyle", "0", CVAR_SERVERINFO );
        if( logstyle )
        {
            uiLogstyle = (unsigned int)logstyle->value;
            if( uiLogstyle >= (sizeof(_sl_LogStyles)/sizeof(_sl_LogStyles[0])) )
                uiLogstyle = 0;
        }
    }
}

/*
 *  PUBLIC FUNCTIONS
 *
 *  NEW Server Logging API
 *
 */

int sl_OpenLogFile( game_import_t  *gi )
{
    return _sl_MaybeOpenFile( gi );
}

void sl_CloseLogFile( void )
{
    _sl_MaybeCloseFile();
}

void sl_LogVers( game_import_t  *gi )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogVers();
    }
}

void sl_LogPatch( game_import_t  *gi,
                  char           *pPatchName )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPatch( pPatchName );
    }
}

void sl_LogDate( game_import_t  *gi )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogDate();
    }
}

void sl_LogTime( game_import_t  *gi )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogTime();
    }
}

void sl_LogDeathFlags( game_import_t  *gi,
                       unsigned long   dmFlags)
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogDeathFlags( dmFlags );
    }
}

void sl_LogMapName( game_import_t  *gi,
                    char           *pMapName )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogMapName( pMapName );
    }
}

void sl_LogPlayerName( game_import_t  *gi,
                       char           *pPlayerName,
                       char           *pTeamName,
                       float           timeInSeconds )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPlayerName( pPlayerName, pTeamName, timeInSeconds );
    }
}

void sl_LogScore( game_import_t  *gi,
                  char           *pKillerName,
                  char           *pTargetName,
                  char           *pScoreType,
                  char           *pWeaponName,
                  int             iScore,
                  float           timeInSeconds )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogScore( pKillerName, pTargetName, pScoreType, pWeaponName, iScore, timeInSeconds );
    }
}

void sl_LogPlayerLeft( game_import_t  *gi,
                       char           *pPlayerName,
                       float           timeInSeconds )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPlayerLeft( pPlayerName, timeInSeconds );
    }
}

void sl_LogGameStart( game_import_t  *gi,
                      float           timeInSeconds )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogGameStart( timeInSeconds );
    }
}

void sl_LogGameEnd( game_import_t  *gi,
                    float           timeInSeconds )
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogGameEnd( timeInSeconds );
    }
}

void sl_LogPlayerConnect( game_import_t  *gi,
                          char           *pPlayerName,
                          char           *pTeamName,
                          float           timeInSeconds)
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPlayerConnect( pPlayerName, pTeamName, timeInSeconds );
    }
}

void sl_LogPlayerTeamChange( game_import_t  *gi,
                             char           *pPlayerName,
                             char           *pTeamName,
                             float           timeInSeconds)
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPlayerTeamChange( pPlayerName, pTeamName, timeInSeconds );
    }
}

void sl_LogPlayerRename( game_import_t  *gi,
                         char           *pOldPlayerName,
                         char           *pNewPlayerName,
                         float           timeInSeconds)
{
    if( _sl_MaybeOpenFile( gi ) )
    {
        _sl_SetStyle( gi );
        _sl_LogStyles[uiLogstyle].pLogPlayerRename( pOldPlayerName, pNewPlayerName, timeInSeconds );
    }
}



/* end of file */