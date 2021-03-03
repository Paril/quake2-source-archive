/*
 Example header file

 Do not use this header file in the WiZ application, use WIZ.H
 instead.

*/
#ifndef _EXAMPLE_H
#define _EXAMPLE_H

#include <windows.h>
//#include <assert.h>    /* required for all Windows applications */
#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>
#include <dlgs.h>
#include <windowsx.h>


//=============================== VERSION INFO ==============================
#ifndef __unzver_h   /* prevent multiple inclusions */
#define __unzver_h

#define UNZ_DLL_VERSION "5.32\0"
#define COMPANY_NAME "Info-ZIP\0"

#endif /* __unzver_h */
//===========================================================================


//================================ STRUCTURES ===============================
#ifndef _STRUCTS_H
#define _STRUCTS_H

#ifndef Far
#  define Far far
#endif

/* Porting definitions between Win 3.1x and Win32 */
#ifdef WIN32
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
#  ifndef FAR
#    define FAR
#  endif
#endif

#ifndef PATH_MAX
#  define PATH_MAX 128            /* max total file or directory name path */
#endif

#ifndef DEFINED_ONCE
#define DEFINED_ONCE
#ifndef ush
typedef unsigned short  ush;
#endif
typedef int (WINAPI DLLPRNT) (LPSTR, unsigned long);
typedef int (WINAPI DLLPASSWORD) (char *, char *, int, char *);
#endif
typedef void (WINAPI DLLSND) (void);
typedef int (WINAPI DLLREPLACE)(LPSTR);
typedef void (WINAPI DLLMESSAGE)(unsigned long,unsigned long,
   ush, ush, ush, ush, ush, ush, char, LPSTR, LPSTR, unsigned long, char);

typedef struct {
DLLPRNT *print;
DLLSND *sound;
DLLREPLACE *replace;
DLLPASSWORD *password;
DLLMESSAGE *SendApplicationMessage;
WORD cchComment;
unsigned long TotalSizeComp;
unsigned long TotalSize;
int CompFactor;
unsigned int NumMembers;
} USERFUNCTIONS, far * LPUSERFUNCTIONS;

typedef struct {
int ExtractOnlyNewer;
int SpaceToUnderscore;
int PromptToOverwrite;
int fQuiet;
int ncflag;
int ntflag;
int nvflag;
int nUflag;
int nzflag;
int ndflag;
int noflag;
int naflag;
int nZIflag;
int C_flag;
int fPrivilege;
LPSTR lpszZipFN;
LPSTR lpszExtractDir;
} DCL, _far *LPDCL;

#endif /* _STRUCTS_H */
//=======================================================================

/* Defines */
#ifndef MSWIN
#define MSWIN
#endif

typedef int (WINAPI * _DLL_UNZIP)(int, char **, int, char **,
                                  LPDCL, LPUSERFUNCTIONS);
typedef int (WINAPI * _USER_FUNCTIONS)(LPUSERFUNCTIONS);

/* Global variables */

extern LPUSERFUNCTIONS lpUserFunctions;
extern LPDCL lpDCL;

extern HINSTANCE hUnzipDll;

extern int hFile;                 /* file handle             */

/* Global functions */

extern _DLL_UNZIP windll_unzip;
extern _USER_FUNCTIONS UzInit;
int WINAPI DisplayBuf(char far *, unsigned long int);

/* Procedure Calls */
void WINAPI ReceiveDllMessage(unsigned long,unsigned long,
   ush, ush, ush, ush, ush, ush, char, char *, char *, unsigned long, char);
#endif /* _EXAMPLE_H */