/* Only the Windows DLL is currently supported */
#ifndef _ZIPAPI_H
#define _ZIPAPI_H

#include "z_zip.h"

#ifndef PATH_MAX
# define PATH_MAX 128
#endif

#if defined(WINDLL) || defined(API)
#include <windows.h>
/* Porting definations between Win 3.1x and Win32 */
#ifdef WIN32
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
#endif

/*---------------------------------------------------------------------------
    Prototypes for public Zip API (DLL) functions.
  ---------------------------------------------------------------------------*/

#define ZPVER_LEN    sizeof(ZpVer)
/* These defines are set to zero for now, until OS/2 comes out
   with a dll.
 */  
#define D2_MAJORVER 0
#define D2_MINORVER 0
#define D2_PATCHLEVEL 0

/* intended to be a private struct: */
typedef struct _zip_ver {
    uch major;              /* e.g., integer 5 */
    uch minor;              /* e.g., 2 */
    uch patchlevel;         /* e.g., 0 */
    uch not_used;
} _zip_version_type;

typedef struct _ZpVer {
    ulg structlen;          /* length of the struct being passed */
    ulg flag;               /* bit 0: is_beta   bit 1: uses_zlib */
    char betalevel[10];     /* e.g., "g BETA" or "" */
    char date[20];          /* e.g., "4 Sep 95" (beta) or "4 September 1995" */
    char zlib_version[10];  /* e.g., "0.95" or NULL */
    _zip_version_type zip;
    _zip_version_type os2dll;
    _zip_version_type windll;
} ZpVer;

#  ifndef EXPENTRY
#    define EXPENTRY WINAPI
#  endif

#ifndef DEFINED_ONCE
#define DEFINED_ONCE
typedef int (far *DLLPRNT) (FILE *, unsigned int, char *);
typedef int (WINAPI DLLPASSWORD) (char *, char *, int, char *);
#endif
typedef LPSTR (WINAPI DLLCOMMENT)(LPSTR);

/* Structures */

typedef struct {          /* zip options */
BOOL fSuffix;             /* include suffixes (not implemented) */
BOOL fEncrypt;            /* encrypt files */
BOOL fSystem;             /* include system and hidden files */
BOOL fVolume;             /* Include volume label */
BOOL fExtra;              /* Include extra attributes */
BOOL fNoDirEntries;       /* Do not add directory entries */
BOOL fExcludeDate;        /* Exclude files earlier than specified date */
BOOL fIncludeDate;        /* Include only files earlier than specified date */
BOOL fVerbose;            /* Mention oddities in zip file structure */
BOOL fQuiet;              /* Quiet operation */
BOOL fCRLF_LF;            /* Translate CR/LF to LF */
BOOL fLF_CRLF;            /* Translate LF to CR/LF */
BOOL fJunkDir;            /* Junk directory names */
BOOL fRecurse;            /* Recurse into subdirectories */
BOOL fGrow;               /* Allow appending to a zip file */
BOOL fForce;              /* Make entries using DOS names (k for Katz) */
BOOL fMove;               /* Delete files added or updated in zip file */
BOOL fDeleteEntries;      /* Delete files from zip file */
BOOL fUpdate;             /* Update zip file--overwrite only if newer */
BOOL fFreshen;            /* Freshen zip file--overwrite only */
BOOL fJunkSFX;            /* Junk SFX prefix */
BOOL fLatestTime;         /* Set zip file time to time of latest file in it */
BOOL fComment;            /* Put comment in zip file */
BOOL fOffsets;            /* Update archive offsets for SFX files */
BOOL fPrivilege;          /* Use privileges (WIN32 only) */
BOOL fEncryption;         /* TRUE if encryption supported, else FALSE.
                             this is a read only flag */
int  fRepair;             /* Repair archive. 1 => -F, 2 => -FF */
char fLevel;              /* Compression level (0 - 9) */
char Date[9];             /* Date to include after */
char szRootDir[PATH_MAX]; /* Directory to use as base for zipping */
} ZPOPT, _far * LPZPOPT;

typedef struct {
int  argc;              /* Count of files to zip */
LPSTR lpszZipFN;        /* name of archive to create/update */
char **FNV;             /* array of file names to zip up */
} ZCL, _far *LPZCL;

typedef struct {
DLLPRNT print;
DLLCOMMENT *comment;
DLLPASSWORD *password;
} ZIPUSERFUNCTIONS, far * LPZIPUSERFUNCTIONS;
/*
void  EXPENTRY ZpVersion(ZpVer far *);
int   EXPENTRY ZpInit(ZIPUSERFUNCTIONS far * lpZipUserFunc);
BOOL  EXPENTRY ZpSetOptions(ZPOPT);
ZPOPT EXPENTRY ZpGetOptions(void);
int   EXPENTRY ZpArchive(ZCL C);
*/
/* Functions not yet supported */
#if 0
int      EXPENTRY ZpMain            (int argc, char **argv);
int      EXPENTRY ZpAltMain         (int argc, char **argv, ZpInit *init);
#endif
#endif /* WINDLL? || API? */

#endif _ZIPAPI_H

