/*
 A very simplistic example of how to load the zip dll and make a call into it.
 Note that none of the command line options are implemented in this example.

 */

//#define WIN32
#define API

#include "g_local.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <direct.h>
#include "g_zip.h"

//======================= VERSION INFO ==========================
#ifndef __zipver_h   /* prevent multiple inclusions */
#define __zipver_h

#define ZIP_DLL_VERSION "2.2\0"
#define COMPANY_NAME "Info-ZIP\0"

#endif /* __zipver_h */
//===============================================================

//#include <commctrl.h>
#include <winver.h>

#define ZIP_DLL_NAME "ZIP32.DLL\0"

#define DLL_WARNING "Cannot find %s."\
            " The Dll must be in the application directory, the path, "\
            "the Windows directory or the Windows System directory."
#define DLL_VERSION_WARNING "%s has the wrong version number."\
            " Insure that you have the correct dll's installed, and that "\
            "an older dll is not in your path or Windows System directory."

int hFile;              /* file handle */

ZCL ZpZCL;
LPZIPUSERFUNCTIONS lpZipUserFunctions;
HANDLE hZUF = (HANDLE)NULL;
HINSTANCE hUnzipDll;
HANDLE hFileList;
ZPOPT ZpOpt;
extern DWORD dwPlatformId;
HINSTANCE hZipDll;


/* Forward References */
_DLL_ZIP ZpArchive;
_ZIP_USER_FUNCTIONS ZpInit;
ZIPSETOPTIONS ZpSetOptions;

void FreeUpMemoryZip(void);
int WINAPI DummyPassword(char *, char *, int, char *);
int far DummyPrint(FILE *, unsigned int, char *);
char far * WINAPI DummyComment(char far *);

#ifdef WIN32
extern BOOL IsNT(VOID);
#endif

/****************************************************************************

    FUNCTION: Main(int argc, char **argv)

****************************************************************************/
#ifdef __BORLANDC__
#  ifdef WIN32
#pragma argsused
#  endif
#endif

// Zips up filename into zipfile
// NOTE: make sure the current dir is the directory in which the .dll files are
//		in before calling this!
int G_ZipFile(char *zipfile, char *filename)
{
LPSTR szFileList;
char **index, *sz;
int retcode/*, i*/, cc;
//DWORD dwVerInfoSize;
//DWORD dwVerHnd;
char szFullPath[PATH_MAX];
char *ptr;
//HANDLE  hMem;         /* handle to mem alloc'ed */

if (!filename || !zipfile)
   return 0;           /* Exits if not proper number of arguments */

hZUF = GlobalAlloc( GPTR, (DWORD)sizeof(ZIPUSERFUNCTIONS));
if (!hZUF)
   {
   return 0;
   }
lpZipUserFunctions = (LPZIPUSERFUNCTIONS)GlobalLock(hZUF);

if (!lpZipUserFunctions)
   {
   GlobalFree(hZUF);
   return 0;
   }

lpZipUserFunctions->print = &DummyPrint;
lpZipUserFunctions->password = DummyPassword;
lpZipUserFunctions->comment = DummyComment;

/* Let's go find the dll */
if (SearchPath(
    NULL,               /* address of search path               */
    ZIP_DLL_NAME,       /* address of filename                  */
    NULL,               /* address of extension                 */
    PATH_MAX,           /* size, in characters, of buffer       */
    szFullPath,         /* address of buffer for found filename */
    &ptr                /* address of pointer to file component */
   ) == 0)
   {
//   char str[256];
//   wsprintf (str, DLL_WARNING, ZIP_DLL_NAME);
//   printf("%s\n", str);
   FreeUpMemoryZip();
   return 0;
   }

/* Okay, now we know that the dll exists, and has the proper version
 * information in it. We can go ahead and load it.
 */
hZipDll = LoadLibrary(ZIP_DLL_NAME);
if (hZipDll != NULL)
   {
   (_DLL_ZIP)ZpArchive = (_DLL_ZIP)GetProcAddress(hZipDll, "ZpArchive");
   (ZIPSETOPTIONS)ZpSetOptions = (ZIPSETOPTIONS)GetProcAddress(hZipDll, "ZpSetOptions");
   if (!ZpArchive || !ZpSetOptions)
      {
//      char str[256];
//      wsprintf (str, "Could not get entry point to %s", ZIP_DLL_NAME);
//      MessageBox((HWND)NULL, str, "Info-ZIP Example", MB_ICONSTOP | MB_OK);
      FreeUpMemoryZip();
      return 0;
      }
   }
else
   {
//   char str[256];
//   wsprintf (str, "Could not load %s", ZIP_DLL_NAME);
//   printf("%s\n", str);
   FreeUpMemoryZip();
   return 0;
   }

(_ZIP_USER_FUNCTIONS)ZpInit = (_ZIP_USER_FUNCTIONS)GetProcAddress(hZipDll, "ZpInit");
if (!ZpInit)
   {
//   printf("Cannot get address of ZpInit in Zip dll. Terminating...");
   FreeLibrary(hZipDll);
   FreeUpMemoryZip();
   return 0;
   }
if (!(*ZpInit)(lpZipUserFunctions))
   {
//   printf("Application functions not set up properly. Terminating...");
   FreeLibrary(hZipDll);
   FreeUpMemoryZip();
   return 0;
   }

/* Here is where the action starts */
ZpOpt.fSuffix = FALSE;        /* include suffixes (not yet implemented) */
ZpOpt.fEncrypt = FALSE;       /* true if encryption wanted */
ZpOpt.fSystem = FALSE;        /* true to include system/hidden files */
ZpOpt.fVolume = FALSE;        /* true if storing volume label */
ZpOpt.fExtra = FALSE;         /* true if including extra attributes */
ZpOpt.fNoDirEntries = FALSE;  /* true if ignoring directory entries */
//ZpOpt.fDate = FALSE;          /* true if excluding files earlier than a
//                                  specified date */
ZpOpt.fVerbose = FALSE;       /* true if full messages wanted */
ZpOpt.fQuiet = TRUE;          /* true if minimum messages wanted */
ZpOpt.fCRLF_LF = FALSE;       /* true if translate CR/LF to LF */
ZpOpt.fLF_CRLF = FALSE;       /* true if translate LF to CR/LF */
ZpOpt.fJunkDir = TRUE;       /* true if junking directory names */
ZpOpt.fRecurse = FALSE;       /* true if recursing into subdirectories */
ZpOpt.fGrow = FALSE;          /* true if allow appending to zip file */
ZpOpt.fForce = FALSE;         /* true if making entries using DOS names */
ZpOpt.fMove = FALSE;          /* true if deleting files added or updated */
ZpOpt.fUpdate = FALSE;        /* true if updating zip file--overwrite only
                                  if newer */
ZpOpt.fFreshen = FALSE;       /* true if freshening zip file--overwrite only */
ZpOpt.fJunkSFX = FALSE;       /* true if junking sfx prefix*/
ZpOpt.fLatestTime = FALSE;    /* true if setting zip file time to time of
                                  latest file in archive */
ZpOpt.fComment = FALSE;       /* true if putting comment in zip file */
ZpOpt.fOffsets = FALSE;       /* true if updating archive offsets for sfx
                                  files */
ZpOpt.fDeleteEntries = FALSE; /* true if deleting files from archive */
ZpOpt.Date[0] = '\0';         /* Not using, set to 0 length */
getcwd(ZpOpt.szRootDir, MAX_PATH);    /* Set directory to current directory */

ZpZCL.argc = 1;               /* number of files to archive - adjust for the
                                  actual number of file names to be added */
ZpZCL.lpszZipFN = (LPSTR) zipfile;    /* archive to be created/updated */

/* Copy over the appropriate portions of argv, basically stripping out argv[0]
   (name of the executable) and argv[1] (name of the archive file)
 */

hFileList = GlobalAlloc( GPTR, 0x10000L);
if ( hFileList )
   {
   szFileList = (char far *)GlobalLock(hFileList);
   }
index = (char **)szFileList;
cc = (sizeof(char *) * ZpZCL.argc);
sz = szFileList + cc;
/*
for (i = 0; i < ZpZCL.argc; i++)
    {
    cc = lstrlen(argv[i+2]);
    lstrcpy(sz, argv[i+2]);
    index[i] = sz;
    sz += (cc + 1);
    }
*/
    cc = lstrlen(filename);
    lstrcpy(sz, filename);
    index[0] = sz;
    sz += (cc + 1);
ZpZCL.FNV = (char **)szFileList;  // list of files to archive


/* Set the options */
ZpSetOptions(ZpOpt);

/* Go zip 'em up */
retcode = ZpArchive(ZpZCL);
if (retcode != 0)
   gi.dprintf("Error during archiving.\nUnable to create \"%s\"\n", zipfile);

GlobalUnlock(hFileList);
GlobalFree(hFileList);
FreeUpMemoryZip();
FreeLibrary(hZipDll);
return (retcode == 0);
}

void FreeUpMemoryZip(void)
{
if (hZUF)
   {
   GlobalUnlock(hZUF);
   GlobalFree(hZUF);
   }
}

/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in Wiz. If you have an encrypted file,
   this will probably give you great pain. Note that none of the
   parameters are being used here, and this will give you warnings.
 */
int WINAPI DummyPassword(char *p, char *n, int m, char * name)
{
return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
int far DummyPrint(FILE *buf, unsigned int size, char *msg)
{
//printf("%s", buf);
return (unsigned int) size;
}


/* Dummy "comment" routine. See comment.c in the wiz directory for how
   this is actually implemented in Wiz. This will probably cause you
   great pain if you ever actually make a call into it.
 */
char far * WINAPI DummyComment(char far *szBuf)
{
szBuf[0] = '\0';
return szBuf;
}

