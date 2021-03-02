/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sys_win.h


///// just testing:

#ifdef WIN32
#include <windows.h>
#include <DelayImp.h>

FARPROC WINAPI DelayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	exit(0);
	/*
    switch (dliNotify)
    {
    case dliStartProcessing:
 
        // If you want to return control to the delay-load helper, return 0. 
        // Otherwise, return a pointer to a FARPROC helper function that will 
        // be used instead, thereby bypassing the rest of the helper.
        break;
 
    case dliNotePreLoadLibrary:
 
        // If you want to return control to the delay-load helper, return 0.
        // Otherwise, return your own HMODULE to be used by the helper 
        // instead of having it call LoadLibrary itself.
        {
            // You can build the DLL path by yourself, and call LoadLibrary 
            // to load the DLL from the path. For simplicity, the sample uses 
            // the dll name to load the DLL, which is the default behavior of 
            // the helper function.
            HMODULE hLib = LoadLibraryA(pdli->szDll);
            return reinterpret_cast<FARPROC>(hLib);
        }
 
    case dliNotePreGetProcAddress:
 
        // If you want to return control to the delay-load helper, return 0. 
        // If you choose you may supply your own FARPROC function address and 
        // bypass the helper's call to GetProcAddress.
        break;
 
    case dliFailLoadLib : 
 
        // LoadLibrary failed.
        // If you don't want to handle this failure yourself, return 0. In 
        // this case the helper will raise an exception (ERROR_MOD_NOT_FOUND) 
        // and exit. If you want to handle the failure by loading an 
        // alternate DLL (for example), then return the HMODULE for the 
        // alternate DLL. The helper will continue execution with this 
        // alternate DLL and attempt to find the requested entrypoint via 
        // GetProcAddress.
        {
            printf("Failed to load the DLL %s w/err 0x%08lx.\n", pdli->szDll, 
                pdli->dwLastError);
            printf("Please input the path of the DLL file:\n");
 
            wchar_t szDll[MAX_PATH];
            fgetws(szDll, ARRAYSIZE(szDll), stdin);
            wchar_t *p = wcschr(szDll, L'\n');
            if (p != NULL)
            {
                *p = L'\0';  // Remove the trailing L'\n'
            }
 
            // Try to load the DLL again.
            HMODULE hLib = LoadLibrary(szDll);
            if (hLib == NULL)
            {
                wprintf(L"Still failed to load the DLL %s w/err 0x%08lx.\n", 
                    szDll, GetLastError());
            }
            return reinterpret_cast<FARPROC>(hLib);
        }
 
        break;
 
    case dliFailGetProc :
 
        // GetProcAddress failed.
        // If you don't want to handle this failure yourself, return 0. In 
        // this case the helper will raise an exception (ERROR_PROC_NOT_FOUND) 
        // and exit. If you choose you may handle the failure by returning an 
        // alternate FARPROC function address.
        printf("Failed to get the function %s.\n", pdli->dlp.szProcName);
 
        break;
 
    case dliNoteEndProcessing : 
 
        // This notification is called after all processing is done. There is 
        // no opportunity for modifying the helper's behavior at this point 
        // except by longjmp()/throw()/RaiseException. No return value is 
        // processed.
 
        break;
    }*/
 
    return NULL;
}
 
 
// At the global level, set the delay-load hooks.
PfnDliHook __pfnDliNotifyHook2 = DelayLoadHook;
PfnDliHook __pfnDliFailureHook2 = DelayLoadHook;
 
#endif


#include "../qcommon/qcommon.h"
#include "winquake.h"
#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include "../win32/conproc.h"

#define MINIMUM_WIN_MEMORY	0x0a00000
#define MAXIMUM_WIN_MEMORY	0x1000000

//#define DEMO

qboolean s_win95;

int			starttime;
int			ActiveApp;
qboolean	Minimized;

static HANDLE		hinput, houtput;

unsigned	sys_msg_time;
unsigned	sys_frame_time;


static HANDLE		qwclsemaphore;

#define	MAX_NUM_ARGVS	128
int			argc;
char		*argv[MAX_NUM_ARGVS];


/*
===============================================================================

SYSTEM IO

===============================================================================
*/



void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	CL_Shutdown();
	Qcommon_Shutdown();

	va_start(argptr, error);
	_vsnprintf(text, sizeof(text), error, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(text); // jitsecurity -- make sure string is null terminated.

	// <!-- jit: don't kill servers with the msg box
	if (dedicated->value)
		printf("Error: %s\n",text);
	else
		MessageBox(NULL, text, "Error", 0 /* MB_OK */ );
	// jit -->

	if (qwclsemaphore)
		CloseHandle(qwclsemaphore);

// shut down QHOST hooks if necessary
	DeinitConProc();

	exit(1);
}

void Sys_Quit (void)
{
	timeEndPeriod( 1 );

	CL_Shutdown();
	Qcommon_Shutdown ();
	CloseHandle (qwclsemaphore);
	if (dedicated && dedicated->value)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (0);
}


void WinError (void)
{
	LPVOID lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	// Display the string.
	MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}


/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	OSVERSIONINFO	vinfo;

	timeBeginPeriod(1);

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx(&vinfo))
		Sys_Error ("Couldn't get OS info.");

	if (vinfo.dwMajorVersion < 4)
		Sys_Error ("Paintball2 requires windows version 4 or greater.");

	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error ("Paintball2 doesn't run on Win32s.");
	else if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		s_win95 = true;

	if (dedicated->value)
	{
		if (!AllocConsole ())
			Sys_Error ("Couldn't create dedicated server console.");

		hinput = GetStdHandle (STD_INPUT_HANDLE);
		houtput = GetStdHandle (STD_OUTPUT_HANDLE);
	
		// let QHOST hook in
		InitConProc (argc, argv);
	}
}


static char	console_text[256];
static int	console_textlen;

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	int		dummy;
	int		/*ch,*/ numread, numevents;
	unsigned char ch; // jittext

	if (!dedicated || !dedicated->value)
		return NULL;


	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events.");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input.");

		if (numread != 1)
			Sys_Error ("Couldn't read console input.");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	

						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}

						break;

				}
			}
		}
	}

	return NULL;
}


/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
	int		dummy;
	char	text[256];

	if (!dedicated || !dedicated->value)
		return;

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}


/*
================
Sys_SendKeyEvents

Send Key_Event calls
================
*/
void Sys_SendKeyEvents (void)
{
    MSG        msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
		sys_msg_time = msg.time;
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	// grab frame time 
	sys_frame_time = timeGetTime();	// FIXME: should this be at start?
}



/*
================
Sys_GetClipboardData

================
*/
char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) != 0 )
	{
		HANDLE hClipboardData;

		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 )
		{
			if ( ( cliptext = GlobalLock( hClipboardData ) ) != 0 ) 
			{
				data = malloc( GlobalSize( hClipboardData ) + 1 );
				strcpy( data, cliptext );
				GlobalUnlock( hClipboardData );
			}
		}
		CloseClipboard();
	}
	return data;
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
=================
Sys_AppActivate
=================
*/
void Sys_AppActivate (void)
{
	ShowWindow(cl_hwnd, SW_RESTORE);
	SetForegroundWindow(cl_hwnd);
}

/*
========================================================================

GAME DLL

========================================================================
*/

static HINSTANCE	game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame (void)
{
	if (!FreeLibrary(game_library))
		Com_Error(ERR_FATAL, "FreeLibrary failed for game library.");

	game_library = NULL;
}

void (*geClientPacket)(void *ent, void *sizebuf); // jitclpacket

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	*path;
	char	cwd[MAX_OSPATH];
#if defined _M_IX86
	const char *gamename = "gamex86.dll";

#ifdef NDEBUG
	const char *debugdir = "release";
#else
	const char *debugdir = "debug";
#endif

#elif defined _M_ALPHA
	const char *gamename = "gameaxp.dll";

#ifdef NDEBUG
	const char *debugdir = "releaseaxp";
#else
	const char *debugdir = "debugaxp";
#endif

#endif

	if (game_library)
		Com_Error(ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame.");

	// check the current debug directory first for development purposes
	_getcwd(cwd, sizeof(cwd));
	Com_sprintf(name, sizeof(name), "%s/%s/%s", cwd, debugdir, gamename);
	game_library = LoadLibrary(name);

	if (game_library)
	{
		Com_DPrintf("LoadLibrary (%s).\n", name);
	}
	else
	{
#ifdef DEBUG
		// check the current directory for other development purposes
		Com_sprintf(name, sizeof(name), "%s/%s", cwd, gamename);
		game_library = LoadLibrary(name);

		if (game_library)
		{
			Com_DPrintf("LoadLibrary (%s).\n", name);
		}
		else
#endif
		{
			// now run through the search paths
			path = NULL;
			while (1)
			{
				path = FS_NextPath(path);
				
				if (!path)
					return NULL;		// couldn't find one anywhere

				Com_sprintf(name, sizeof(name), "%s/%s", path, gamename);
				game_library = LoadLibrary(name);

				if (game_library)
				{
					Com_DPrintf("LoadLibrary (%s).\n",name);
					break;
				}
			}
		}
	}

	GetGameAPI = (void *)GetProcAddress(game_library, "GetGameAPI");
	geClientPacket = (void *)GetProcAddress(game_library, "ClientPacket"); // jitclpacket

	if (!GetGameAPI)
	{
		Sys_UnloadGame();
		return NULL;
	}

	return GetGameAPI(parms);
}

//=======================================================================


/*
==================
ParseCommandLine

==================
*/
void ParseCommandLine (LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}
}

/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG			msg;
	int			time, oldtime, newtime;
	// === jiturl
	char		*cmdline, *s;
	char		working_dir[_MAX_PATH+_MAX_FNAME];
	qboolean	quote = false;
	int			i = 0;

	cmdline = GetCommandLine();
	
	if (cmdline[0] == '\"')
	{
		quote = true;
		cmdline++;
	}

	while (*cmdline != (quote ? '\"' : ' '))
	{
		working_dir[i++] = *cmdline;
		cmdline++;
	}
	
	working_dir[i] = '\0';

	for (s = working_dir + strlen(working_dir); s > working_dir && *s != '\\' && *s != '/'; s--);
	
	*s = '\0';
	_chdir(working_dir);
	// jiturl ===

    /* previous instances do not exist in Win32 */
    if (hPrevInstance)
        return 0;

	global_hInstance = hInstance;

	ParseCommandLine(lpCmdLine);

	Qcommon_Init(argc, argv);
	oldtime = Sys_Milliseconds();

    /* main window message loop */
	while (1)
	{
		// if at a full screen console, don't update unless needed
		if (Minimized || (dedicated && dedicated->value) )
		{
			Sleep(1);
		}

		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				Com_Quit();

			sys_msg_time = msg.time;
			TranslateMessage(&msg);
   			DispatchMessage(&msg);
		}

		do
		{
			NET_Sleep(1); // jit - don't bog down CPU on the server.
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		} while (time < 1);

		_controlfp(_PC_24, _MCW_PC);
		Qcommon_Frame(time);
		oldtime = newtime;
	}

	// never gets here
    return TRUE;
}
