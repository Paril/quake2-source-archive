/*
 Example header file
*/
#ifndef _EXAMPLE_H
#define _EXAMPLE_H

#include <windows.h>
//#include <assert.h>    /* required for all Windows applications */
#include <stdlib.h>
#include <stdio.h>
//#include <commdlg.h>
#include <dlgs.h>
#include <windowsx.h>

//=================================== STRUCTS ==============================
#ifndef _ZIP_STRUCTS_H
#define _ZIP_STRUCTS_H

#ifndef Far
#  define Far far
#endif

#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near

#ifndef PATH_MAX
# define PATH_MAX 128
#endif

#include "z_api.h"

#endif /* _ZIP_STRUCTS_H */
//==================================================================

/* Defines */
#ifndef MSWIN
#define MSWIN
#endif

typedef int (WINAPI * _DLL_ZIP)(ZCL);
typedef int (WINAPI * _ZIP_USER_FUNCTIONS)(LPZIPUSERFUNCTIONS);
typedef BOOL (WINAPI * ZIPSETOPTIONS)(ZPOPT);

/* Global variables */

extern LPZIPUSERFUNCTIONS lpZipUserFunctions;

extern HINSTANCE hZipDll;

extern int hFile;                 /* file handle             */

/* Global functions */

extern _DLL_ZIP ZpArchive;
extern _ZIP_USER_FUNCTIONS ZpInit;
int WINAPI DisplayBuf(char far *, unsigned long int);
extern ZIPSETOPTIONS ZpSetOptions;

#endif /* _EXAMPLE_H */

