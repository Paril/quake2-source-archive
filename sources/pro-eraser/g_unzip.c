/*
   This is a very simplistic example of how to load and make a call into the
   dll. This has been compiled and tested for a 32-bit console version, but
   not under 16-bit windows. However, the #ifdef's have been left in for the
   16-bit code, simply as an example.

 */

//#define WIN32

#include "g_local.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "g_unzip.h"
#include <winver.h>

#define UNZ_DLL_NAME "UNZIP32.DLL\0"

#define DLL_WARNING "Cannot find %s."\
            " The DLL must be in the application directory, the path, "\
            "the Windows directory or the Windows System directory."
#define DLL_VERSION_WARNING "%s has the wrong version number."\
            " Insure that you have the correct dll's installed, and that "\
            "an older dll is not in your path or Windows System directory."

int hFile;              /* file handle */

LPUSERFUNCTIONS lpUserFunctions;
HANDLE hUF = (HANDLE)NULL;
LPDCL lpDCL = NULL;
HANDLE hDCL = (HANDLE)NULL;
HINSTANCE hUnzipDll;
HANDLE hZCL = (HANDLE)NULL;
#ifdef WIN32
DWORD dwPlatformId = 0xFFFFFFFF;
#endif


/* Forward References */
int WINAPI DisplayBuf(char far *, unsigned long);
int WINAPI GetReplaceDlgRetVal(char *);
int WINAPI password(char *, char *, int, char *);
void WINAPI ReceiveDllMessage(unsigned long,unsigned long,
    ush, ush, ush, ush, ush, ush, char, char *, char *, unsigned long, char);
_DLL_UNZIP windll_unzip;
_USER_FUNCTIONS UzInit;
void FreeUpMemory(void);
#ifdef WIN32
BOOL IsNT(VOID);
#endif

// Extracts all files in zipfile to extract_dir.
// NOTE: make sure the current dir is the directory in which the .dll files are
//		in before calling this!
int G_UnzipFile(char *zipfile, char *extract_dir)
{
//int exfc, infc;
//char **exfv, **infv;
//char *x_opt;
//DWORD dwVerInfoSize;
//DWORD dwVerHnd;
char szFullPath[PATH_MAX];
int retcode;
char *ptr;
//HANDLE  hMem;         /* handle to mem alloc'ed */

if (!zipfile)   /* We must have an archive to unzip */
   return 0;

hDCL = GlobalAlloc( GPTR, (DWORD)sizeof(DCL));
if (!hDCL)
   {
   return 0;
   }
lpDCL = (LPDCL)GlobalLock(hDCL);
if (!lpDCL)
   {
   GlobalFree(hDCL);
   return 0;
   }

hUF = GlobalAlloc( GPTR, (DWORD)sizeof(USERFUNCTIONS));
if (!hUF)
   {
   GlobalUnlock(hDCL);
   GlobalFree(hDCL);
   return 0;
   }
lpUserFunctions = (LPUSERFUNCTIONS)GlobalLock(hUF);

if (!lpUserFunctions)
   {
   GlobalUnlock(hDCL);
   GlobalFree(hDCL);
   GlobalFree(hUF);
   return 0;
   }

lpUserFunctions->password = password;
lpUserFunctions->print = DisplayBuf;
lpUserFunctions->sound = NULL;
lpUserFunctions->replace = GetReplaceDlgRetVal;
lpUserFunctions->SendApplicationMessage = ReceiveDllMessage;

/* First we go look for the unzip dll */
#ifdef WIN32
if (SearchPath(
    NULL,               /* address of search path               */
    UNZ_DLL_NAME,       /* address of filename                  */
    NULL,               /* address of extension                 */
    PATH_MAX,           /* size, in characters, of buffer       */
    szFullPath,         /* address of buffer for found filename */
    &ptr                /* address of pointer to file component */
   ) == 0)
#else
hfile = OpenFile(UNZ_DLL_NAME,  &ofs, OF_SEARCH);
if (hfile == HFILE_ERROR)
#endif
   {	// cannot find unzip32.dll file
//   char str[256];
//   wsprintf (str, DLL_WARNING, UNZ_DLL_NAME);
//   gi.dprintf("%s\n", str);
   FreeUpMemory();
   return 0;
   }
#ifndef WIN32
else
   lstrcpy(szFullPath, ofs.szPathName);
_lclose(hfile);
#endif

/* Okay, now we know that the dll exists, and has the proper version
 * information in it. We can go ahead and load it.
 */
hUnzipDll = LoadLibrary(UNZ_DLL_NAME);
#ifndef WIN32
if (hUnzipDll > HINSTANCE_ERROR)
#else
if (hUnzipDll != NULL)
#endif
   {
   (_DLL_UNZIP)windll_unzip = (_DLL_UNZIP)GetProcAddress(hUnzipDll, "windll_unzip");
   }
else
   {	// unable to load the dll
//   char str[256];
//   wsprintf (str, "Could not load %s", UNZ_DLL_NAME);
//   printf("%s\n", str);
   FreeUpMemory();
   return 0;
   }

/*
   Here is where the actual extraction process begins. First we set up the
   flags to be passed into the dll.
 */
lpDCL->ncflag = 0; /* Write to stdout if true */
lpDCL->fQuiet = 2; /* We want all messages.
                      1 = fewer messages,
                      2 = no messages */
lpDCL->ntflag = 0; /* test zip file if true */
lpDCL->nvflag = 0; /* give a verbose listing if true */
lpDCL->nUflag = 0; /* Do not extract only newer */
lpDCL->nzflag = 0; /* display a zip file comment if true */
lpDCL->ndflag = 0; /* Recreate directories if true */
lpDCL->noflag = 1; /* Over-write all files if true */
lpDCL->naflag = 0; /* Do not convert CR to CRLF */
lpDCL->lpszZipFN = (LPSTR) zipfile; /* The archive name */
lpDCL->lpszExtractDir = (LPSTR) extract_dir; 
							  /* The directory to extract to. This is set
                                 to NULL if you are extracting to the
                                 current directory.
                               */

// UNZIP the file!

retcode = (*windll_unzip)(0, NULL, 0, NULL, lpDCL, lpUserFunctions);

if (retcode != 0)
   gi.dprintf("Error unzipping \"%s\"\n", zipfile);

FreeUpMemory();
FreeLibrary(hUnzipDll);
return (retcode == 0);
}

int WINAPI GetReplaceDlgRetVal(char *filename)
{
/* This is where you will decide if you want to replace, rename etc existing
   files.
 */
return 1;
}

void FreeUpMemory(void)
{
if (hDCL)
   {
   GlobalUnlock(hDCL);
   GlobalFree(hDCL);
   }
if (hUF)
   {
   GlobalUnlock(hUF);
   GlobalFree(hUF);
   }
}

/* This simply determines if we are running on NT or Windows 95 */
#ifdef WIN32
BOOL IsNT(VOID)
{
if(dwPlatformId != 0xFFFFFFFF)
   return dwPlatformId;
else
/* note: GetVersionEx() doesn't exist on WinNT 3.1 */
   {
   if(GetVersion() < 0x80000000)
      {
      (BOOL)dwPlatformId = TRUE;
      }
   else
      {
      (BOOL)dwPlatformId = FALSE;
      }
    }
return dwPlatformId;
}
#endif

/* This is a very stripped down version of what is done in WiZ. Essentially
   what this function is for is to do a listing of an archive contents. It
   is actually never called in this example, but a dummy procedure had to
   be put in, so this was used.
 */
void WINAPI ReceiveDllMessage(unsigned long ucsize,unsigned long csiz,
        ush cfactor, ush mo, ush dy, ush yr, ush hh, ush mm,
    char c, char *filename, char *methbuf, unsigned long crc, char fCrypt)
{
//char psLBEntry[PATH_MAX];
char LongHdrStats[] =
          "%7lu  %7lu %4s  %02u-%02u-%02u  %02u:%02u  %c%s";
char CompFactorStr[] = "%c%d%%";
char CompFactor100[] = "100%%";
char szCompFactor[10];
char sgn;

if (csiz > ucsize)
   sgn = '-';
else
   sgn = ' ';
if (cfactor == 100)
   lstrcpy(szCompFactor, CompFactor100);
else
   sprintf(szCompFactor, CompFactorStr, sgn, cfactor);
//wsprintf(psLBEntry, LongHdrStats,
//      ucsize, csiz, szCompFactor, mo, dy, yr, hh, mm, c, filename);

//printf("%s\n", psLBEntry);
}

/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in WiZ. If you have an encrypted file,
   this will probably give you great pain.
 */
int WINAPI password(char *p, char *m, int n, char *name)
{
return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
int WINAPI DisplayBuf(char far *buf, unsigned long size)
{
//printf("%s", buf);
return (unsigned int) size;
}

