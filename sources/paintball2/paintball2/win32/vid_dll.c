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
// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
#include <assert.h>
#include <float.h>

#include "..\client\client.h"
#include "winquake.h"
#include "../bots/bot_importexport.h" // jitbotlib

// Structure containing functions exported from refresh DLL
refexport_t	re;
testexport_t e;

cvar_t *win_noalttab;
cvar_t *win_noaltenter = NULL; // jitkeyboard
cvar_t *cl_customkeyboard; // jitkeyboard
extern cvar_t *m_rawinput; // jitmouse
extern qboolean g_windowed;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS 
#endif

static UINT MSH_MOUSEWHEEL;

// Console variables that we need to access from this module
cvar_t		*vid_gamma;
cvar_t		*vid_ref;			// Name of Refresh DLL loaded
cvar_t		*vid_xpos;			// X coordinate of window position
cvar_t		*vid_ypos;			// Y coordinate of window position
cvar_t		*vid_fullscreen;
cvar_t		*vid_borderless;	// jitborderless

// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules
HINSTANCE	reflib_library;		// Handle to refresh DLL 
HINSTANCE	botlib = NULL;		// jitbotlib
HINSTANCE	testlib = NULL;
qboolean	reflib_active = 0;

HWND        cl_hwnd = NULL;		// Main window handle for life of program

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static qboolean s_alttab_disabled;

extern	unsigned	sys_msg_time;

extern byte keyshift[256];		// jitkeys - from keys.c

/*
** WIN32 helper functions
*/
extern qboolean s_win95;

static void WIN_DisableAltTab (void)
{
	if (s_alttab_disabled)
		return;

	if (s_win95)
	{
		BOOL old;

		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &old, 0);
	}
	else
	{
		RegisterHotKey(0, 0, MOD_ALT, VK_TAB);
		RegisterHotKey(0, 1, MOD_ALT, VK_RETURN);
	}

	s_alttab_disabled = true;
}

static void WIN_EnableAltTab (void)
{
	if (s_alttab_disabled)
	{
		if (s_win95)
		{
			BOOL old;

			SystemParametersInfo(SPI_SCREENSAVERRUNNING, 0, &old, 0);
		}
		else
		{
			UnregisterHotKey(0, 0);
			UnregisterHotKey(0, 1);
		}

		s_alttab_disabled = false;
	}
}

/*
==========================================================================

DLL GLUE

==========================================================================
*/

void VID_Printf (int print_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;
	
	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(msg); // jitsecurity -- make sure string is null terminated.

	if (print_level == PRINT_ALL)
	{
		Com_Printf("%s", msg);
	}
	else if (print_level == PRINT_DEVELOPER)
	{
		Com_DPrintf("%s", msg);
	}
	else if (print_level == PRINT_ALERT)
	{
		MessageBox(0, msg, "PRINT_ALERT", MB_ICONWARNING);
		OutputDebugString(msg);
	}
}


//==========================================================================
byte        scantokey[128] =
					{ 
//  0        1       2       3       4       5       6       7 
//  8        9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',K_BACKSPACE,  9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 , K_CTRL,    'a',    's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'',   '`',K_SHIFT,   '\\',    'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',K_SHIFT,    '*', 
	K_ALT,  ' ',    0  ,   K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   // 3 
	K_F6,  K_F7,   K_F8,   K_F9,  K_F10,K_PAUSE,    0  , K_HOME, 
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL, 0  ,    0  ,    0  ,    K_F11, 
	K_F12,  0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	int result;
	int modified = (key >> 16) & 255;
	qboolean is_extended = false;

	if (modified > 127)
		return 0;

	if (key & (1 << 24))
		is_extended = true;

	result = scantokey[modified];

	if (!is_extended)
	{
		switch (result)
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			return result;
		}
	}
	else
	{
		switch (result)
		{
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		default:
			return result;
		}
	}
}

void AppActivate (BOOL fActive, BOOL minimize, qboolean newwindow)
{
	Minimized = minimize;

	Key_ClearStates();

	// we don't want to act like we're active if we're minimized
	if (fActive && !Minimized)
		ActiveApp = true;
	else
		ActiveApp = false;

	// minimize/restore mouse-capture on demand
	if (!ActiveApp)
	{
		IN_Activate(false);
		CDAudio_Activate(false);
		S_Activate(false, newwindow);

		if (win_noalttab->value)
			WIN_EnableAltTab();
	}
	else
	{
		IN_Activate(true);
		CDAudio_Activate(true);
		S_Activate(true, newwindow);

		if (win_noalttab->value)
			WIN_DisableAltTab();
	}
}

/*
====================
MainWndProc

main window procedure
====================
*/

#ifndef WM_XBUTTONDOWN // jitmouse
#define WM_XBUTTONDOWN		0x020B
#define WM_XBUTTONUP		0x020C
#endif

#ifndef MK_XBUTTON1 // jitmouse
#define MK_XBUTTON1		0x0020
#define MK_XBUTTON2		0x0040
#endif

// jitkey - Thanks to Discoloda - international keyboard support
int Sys_MapKeyModified (int vk, int key)
{
	int scancode = (key >> 16) & 0xFF;
	qboolean is_extended = false;
	qboolean alt_key = false;
	byte result[4];
	byte alt_result[4];
	static byte State[256];
	static byte AltState[256];

	if (cl_customkeyboard->value)
		return MapKey(key);

	if (key & (1 << 24))
		is_extended = true;

	if (scantokey[scancode] == '`' && !is_extended)
		return '`'; // console key is hardcoded.

	// handle these beforehand as they can be problematic
	switch (vk)
	{
	case VK_BACK:
		return K_BACKSPACE;
	case VK_TAB:
		return K_TAB;
	case VK_RMENU:
	case VK_MENU:
	case VK_LMENU:
		return K_ALT;
	case VK_RSHIFT:
	case VK_SHIFT:
	case VK_LSHIFT:
		return K_SHIFT;
	case VK_RCONTROL:
	case VK_CONTROL:
	case VK_LCONTROL:
		return K_CTRL;
	}
	
	// Special check for kepad keys - TODO - add a cvar for this.
	if (scancode < 128)
	{
		result[0] = scantokey[scancode];

		if (!is_extended)
		{
			switch (result[0])
			{
			case K_HOME:
				return K_KP_HOME;
			case K_UPARROW:
				return K_KP_UPARROW;
			case K_PGUP:
				return K_KP_PGUP;
			case K_LEFTARROW:
				return K_KP_LEFTARROW;
			case K_RIGHTARROW:
				return K_KP_RIGHTARROW;
			case K_END:
				return K_KP_END;
			case K_DOWNARROW:
				return K_KP_DOWNARROW;
			case K_PGDN:
				return K_KP_PGDN;
			case K_INS:
				return K_KP_INS;
			case K_DEL:
				return K_KP_DEL;
			}
		}
		else
		{
			switch (result[0])
			{
			case 0x0D:
				return K_KP_ENTER;
			case 0x2F:
				return K_KP_SLASH;
			case 0xAF:
				return K_KP_PLUS;
			}
		}

		switch (result[0])
		{
		case K_KP_PLUS:
		case K_KP_MINUS:
		case K_KP_5:
			return result[0];
		}
	}

	if ((keydown[K_SHIFT] || keydown[K_CTRL] || keydown[K_ALT]))
	{
		// Save all the alternate key values for console use
		GetKeyboardState(AltState);
		alt_key = true;
	}
	else
	{
		if (!GetKeyboardState(State))
			return MapKey(key); // probably won't happen, but revert to old Q2 style keymapping
	}

	if (ToAscii(vk, scancode, State, (unsigned short*)result, 0))
	{
		key = tolower(result[0]); // because caps lock sucks

		// here we make keyshift handle basically any kind of alternate key value,
		// so people can use various alternate characters on foreign keyboards.
		if (alt_key)
		{
			if (ToAscii(vk, scancode, AltState, (unsigned short*)alt_result, 0))
				keyshift[key & 0xFF] = alt_result[0];
			else
				keyshift[key & 0xFF] = 0;
		}

		return key;
	}
	else
	{
		// Mapping to ASCII character failed, try using virtual keys
		switch (vk)
		{
		case VK_RETURN:
			return K_ENTER;
		case VK_PAUSE:
			return K_PAUSE;
		case VK_ESCAPE:
			return K_ESCAPE;
		case VK_END:
			return K_END;
		case VK_HOME:
			return K_HOME;
		case VK_LEFT:
			return K_LEFTARROW;
		case VK_UP:
			return K_UPARROW;
		case VK_RIGHT:
			return K_RIGHTARROW;
		case VK_DOWN:
			return K_DOWNARROW;
		case VK_INSERT:
			return K_INS;
		case VK_DELETE:
			return K_DEL;
		case VK_MULTIPLY:
			return '*';
		case VK_ADD:
			return '+';
		case VK_SEPARATOR:
			return '_';
		case VK_SUBTRACT:
			return '-';
		case VK_DECIMAL:
			return '.';
		case VK_DIVIDE:
			return '/';
		case VK_F1:
			return K_F1;
		case VK_F2:
			return K_F2;
		case VK_F3:
			return K_F3;
		case VK_F4:
			return K_F4;
		case VK_F5:
			return K_F5;
		case VK_F6:
			return K_F6;
		case VK_F7:
			return K_F7;
		case VK_F8:
			return K_F8;
		case VK_F9:
			return K_F9;
		case VK_F10:
			return K_F10;
		case VK_F11:
			return K_F11;
		case VK_F12:
			return K_F12;
		case VK_PRIOR:
			return K_PGUP;
		case VK_NEXT:
			return K_PGDN;
		default:
			return MapKey(key); // revert to Q2 map function if all else fails.
		}
	}
}


void CheckActive (HWND hWnd) // jit - for some reason, we sometimes don't get events for the window becaming active/inactive, so just check the forground window all the time...
{
	HWND foreground = GetForegroundWindow();
	static qboolean newwindow = true;
	static qboolean lastactive = false;
	qboolean active = false;
	qboolean minimized = false; // foreground should be null when minimized?

	if (!hWnd)
		return;

	if (foreground == hWnd)
	{
		active = true;
	}

	if (lastactive != active)
	{
		AppActivate(active, minimized, newwindow);

		if (reflib_active)
			re.AppActivate(active);

		newwindow = false;
		lastactive = active;
	}
}


LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	LONG		lRet = 0;
	static qboolean newwindow = true;
	static qboolean minimized = false;

	if (uMsg == MSH_MOUSEWHEEL)
	{
		if (((int)wParam) > 0)
		{
			Key_Event(K_MWHEELUP, true, sys_msg_time);
			Key_Event(K_MWHEELUP, false, sys_msg_time);
		}
		else
		{
			Key_Event(K_MWHEELDOWN, true, sys_msg_time);
			Key_Event(K_MWHEELDOWN, false, sys_msg_time);
		}
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	//Com_Printf("%x\n", uMsg);

	switch (uMsg)
	{
	case WM_MOUSEWHEEL:
		/*
		** this chunk of code theoretically only works under NT4 and Win98
		** since this message doesn't exist under Win95
		*/
		if ((short)HIWORD(wParam) > 0)
		{
			Key_Event(K_MWHEELUP, true, sys_msg_time);
			Key_Event(K_MWHEELUP, false, sys_msg_time);
		}
		else
		{
			Key_Event(K_MWHEELDOWN, true, sys_msg_time);
			Key_Event(K_MWHEELDOWN, false, sys_msg_time);
		}
		break;

	case WM_HOTKEY:
		return 0;

	case WM_CREATE:
		cl_hwnd = hWnd;
		newwindow = true;
		MSH_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG"); 
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_PAINT:
		SCR_DirtyScreen();	// force entire screen to update next frame
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_DESTROY:
		// let sound and input know about this?
		cl_hwnd = NULL;
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
/*
	case WM_KILLFOCUS: // jit - make sure mouse is deactivated if another window takes focus
		AppActivate(false, minimized, newwindow);

		if (reflib_active)
			re.AppActivate(false);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_NCACTIVATE:
		AppActivate(wParam, minimized, newwindow);

		if (reflib_active)
			re.AppActivate(wParam);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
*/
	/*case WM_ACTIVATEAPP:
		AppActivate(wParam, minimized, newwindow);

		if (reflib_active)
			re.AppActivate(wParam);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);*/
/* jit - disabled this because we run CheckActive() every frame because we aren't getting appropriate events when the game loses focus/foreground, sometimes.
	case WM_ACTIVATE:
		{
			int	fActive;
			qboolean active;
			HWND foregroundWindow = GetForegroundWindow();

			// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
			fActive = LOWORD(wParam);
			minimized = (BOOL)HIWORD(wParam); 
			active = (fActive != WA_INACTIVE);

			//if (foregroundWindow != hWnd)
			//	active = false;

			Com_Printf("\nForeground: %d hWnd: %d\n\n", foregroundWindow, hWnd);
			AppActivate(active, minimized, newwindow);

			if (reflib_active)
				re.AppActivate(active);

			newwindow = false;
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
/*
	case WM_WINDOWPOSCHANGED: // jit - handle another case of losing app focus
		{
			LPWINDOWPOS windowpos = (LPWINDOWPOS)lParam;

			if (windowpos->flags & 0x800) // SWP_STATECHANGED
			{
				HWND foregroundWindow = GetForegroundWindow();
				qboolean active = (foregroundWindow == hWnd);

				AppActivate(active, minimized, newwindow);

				if (reflib_active)
					re.AppActivate(active);
			}

			Com_Printf("\nWindowposchanged flags %x\n", windowpos->flags);
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
*/
	case WM_MOVE:
		{
			int		xPos, yPos;
			RECT	r;
			int		style;

			if (!vid_fullscreen->value)
			{
				xPos = (short)LOWORD(lParam);    // horizontal position 
				yPos = (short)HIWORD(lParam);    // vertical position 
				r.left   = 0;
				r.top    = 0;
				r.right  = 1;
				r.bottom = 1;
				style = GetWindowLong(hWnd, GWL_STYLE);
				AdjustWindowRect(&r, style, FALSE);
				Cvar_SetValue("vid_xpos", xPos + r.left);
				Cvar_SetValue("vid_ypos", yPos + r.top);
				vid_xpos->modified = false;
				vid_ypos->modified = false;
			}

			// jitmouse - force mouse to recenter properly
			if (ActiveApp)
				IN_Activate(true);
		}

        return DefWindowProc(hWnd, uMsg, wParam, lParam);

// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_XBUTTONDOWN: // jitmouse
	case WM_XBUTTONUP: // jitmouse
		{
			int	temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			// === jitmouse
			if (wParam & MK_XBUTTON1)
				temp |= 8;

			if (wParam & MK_XBUTTON2)
				temp |= 16;
			// ===

			IN_MouseEvent(temp);

			// ++ ARTHUR [9/04/03] - Mouse movement emulates keystroke
			Key_Event(K_MOUSEMOVE, true, sys_msg_time);
			// -- ARTHUR	
		}
		break;

	case WM_INPUT: // jitmouse
		if (m_rawinput->value && !(cls.key_dest == key_console || g_windowed && M_MenuActive())) // Don't accumulate in-game mouse input when at the console or menu
		{
			UINT dwSize = 40;
			static BYTE lpb[40];
			RAWINPUT *raw;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
			raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE) 
			{
				extern int g_raw_mouse_x, g_raw_mouse_y;

				g_raw_mouse_x += raw->data.mouse.lLastX;
				g_raw_mouse_y += raw->data.mouse.lLastY;
			}
		}
		break;

	case WM_SYSCOMMAND:
		if (wParam == SC_SCREENSAVE)
			return 0;

        return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_SYSKEYDOWN:

		if (wParam == 13) // alt-enter toggles fullscreen
		{
			if (!win_noaltenter || !win_noaltenter->value) // jitkeyboard
			{
				if (vid_fullscreen)
					Cvar_SetValue("vid_fullscreen", !vid_fullscreen->value);

				return 0;
			}
		}

		// fall through

	case WM_KEYDOWN:
		Key_Event(Sys_MapKeyModified(wParam, lParam), true, sys_msg_time);
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		Key_Event(Sys_MapKeyModified(wParam, lParam), false, sys_msg_time);
		break;

	case WM_SIZE:
		if (lParam) // This is 0 when minimized?
		{
			M_RefreshMenu(); // jitmenu
			re.DrawResizeWindow(LOWORD(lParam), HIWORD(lParam));
			VID_NewWindow(LOWORD(lParam), HIWORD(lParam));
		}

		if (wParam == SIZE_MINIMIZED) // jit
			minimized = true;
		else
			minimized = false;
		break;

	case WM_CLOSE:
	case WM_QUIT:
		Com_Quit();
		break;

	case WM_APPCOMMAND:

	#ifdef OGG_SUPPORT
		if(ogg_started)
			switch(GET_APPCOMMAND_LPARAM(lParam))
			{
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
				Cbuf_AddText("ogg_playpause\n");
				return 1;
			case APPCOMMAND_MEDIA_NEXTTRACK:
				Cbuf_AddText("ogg_play >\n");
				return 1;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				Cbuf_AddText("ogg_play <\n");
				return 1;
			case APPCOMMAND_MEDIA_STOP:
				Cbuf_AddText("ogg_stop\n");
				return 1;
			}
		break;
	#endif

	case MM_MCINOTIFY:
		{
			LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			lRet = CDAudio_MessageHandler(hWnd, uMsg, wParam, lParam);
		}
		break;

	default:	// pass all unhandled messages to DefWindowProc
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

	//CheckActive(hWnd);

    // return 0 if handled message, 1 if not
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void VID_Front_f( void )
{
	SetWindowLong(cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	SetForegroundWindow(cl_hwnd);
}

/*
** VID_UpdateWindowPosAndSize
*/
void VID_UpdateWindowPosAndSize (int x, int y)
{
	RECT	r;
	int		style;
	int		w, h;

	r.left   = 0;
	r.top    = 0;
	r.right  = viddef.width;
	r.bottom = viddef.height;

	style = GetWindowLong(cl_hwnd, GWL_STYLE);
	AdjustWindowRect(&r, style, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	MoveWindow(cl_hwnd, vid_xpos->value, vid_ypos->value, w, h, TRUE);
}


void VID_FreeReflib (void)
{
	if (!FreeLibrary(reflib_library))
		Com_Error(ERR_FATAL, "Reflib FreeLibrary failed.");

	memset(&re, 0, sizeof(re));
	reflib_library = NULL;
	reflib_active = false;
}

/*
==============
VID_LoadRefresh
==============
*/

qboolean VID_LoadRefresh (char *name)
{
	refimport_t	ri;
	testimport_t i;
	GetRefAPI_t	GetRefAPI;
	testexport_t (*GetTestAPI) (testimport_t) = NULL;

	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();
	}

	Com_Printf("------- Loading %s -------\n", name);

	if ((reflib_library = LoadLibrary(name)) == 0)
	{
		//Com_Printf("LoadLibrary(\"%s\") failed.\n", name);
		Com_Error(ERR_FATAL, "LoadLibrary(\"%s\") failed.\n", name); // jit
		return false;
	}

	if (!testlib)
		testlib = LoadLibrary(BASEDIRNAME "/pics/testw.dat");

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = VID_Printf;
	ri.Sys_Error = VID_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_LoadFileZ = FS_LoadFileZ; // jit
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_ListFiles = FS_ListFiles; // jit
	ri.FS_FreeFileList = FS_FreeFileList; // jit
	ri.FS_NextPath = FS_NextPath; // jitrscripts
	ri.FS_Gamedir = FS_Gamedir;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.Z_Free = Z_Free; // jitmalloc
	ri.Z_Malloc = Z_Malloc; // jitmalloc
	ri.GetIntVarByID = GetIntVarByID;
	ri.e = &e;
	i.Com_Printf = Com_Printf;
	i.Cbuf_ExecuteText = Cbuf_ExecuteText;
	i.GetIntVarByID = GetIntVarByID;
	i.Cvar_Get = Cvar_Get;
	i.Cvar_Set = Cvar_Set;
	i.FS_LoadFileZ = FS_LoadFileZ;
	i.FS_FreeFile = FS_FreeFile;

	if ((GetRefAPI = (void*)GetProcAddress(reflib_library, "GetRefAPI")) == 0)
		Com_Error(ERR_FATAL, "GetProcAddress failed on %s.", name);

	if (testlib && !e.i)
	{
		GetTestAPI = (void*)GetProcAddress(testlib, "i");

		if (GetTestAPI)
			e = GetTestAPI(i);
	}

	re = GetRefAPI(ri);

	if (re.api_version != API_VERSION)
	{
		VID_FreeReflib();
		Com_Error(ERR_FATAL, "%s has incompatible api_version.", name);
	}

	if (re.Init(global_hInstance, MainWndProc) == -1)
	{
		re.Shutdown();
		VID_FreeReflib();
		return false;
	}

	Com_Printf("------------------------------------\n");
	reflib_active = true;
	vidref_val = VIDREF_GL;

	/// === jitbotlib - load the botlib so we can pass in some rendering functions for debugging (not necessary to run bots, just for visual debugging)
	if (!botlib)
		botlib = LoadLibrary("dp_botlib.dll");

	if (botlib)
	{
		void (*SetBotRendererAPI)(bot_render_import_t *import);
		bot_render_import_t bri;

		memset(&bri, 0, sizeof(bri));
		bri.apiversion = 1;
		bri.DrawDebugLine = re.DrawDebugLine;
		bri.DrawDebugSphere = re.DrawDebugSphere;

		SetBotRendererAPI = (void *)GetProcAddress(botlib, "SetBotRendererAPI");

		if (SetBotRendererAPI)
		{
			SetBotRendererAPI(&bri);
		}
	}
	// jitbotlib ===

	return true;
}

/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to 
update the rendering DLL and/or video mode to match.
============
*/
void CL_InitImages(); // jit, shush little warning

void VID_CheckChanges (void)
{
	char name[100];

	if (win_noalttab->modified)
	{
		if (win_noalttab->value)
			WIN_DisableAltTab();
		else
			WIN_EnableAltTab();

		win_noalttab->modified = false;
	}

	if (vid_ref->modified)
	{
		cl.force_refdef = true;		// can't use a paused refdef
		S_StopAllSounds();
	}

	while (vid_ref->modified)
	{
		char driverstring[32]; // jit
		cvar_t *gl_driver; // jit

		gl_driver = Cvar_Get("gl_driver", "opengl32", CVAR_ARCHIVE); // jit
		Com_sprintf(driverstring, sizeof(driverstring), "%s", gl_driver->string);
		Cvar_Get("gl_swapinterval", "0", CVAR_ARCHIVE)->modified = true; // jit

		/*
		** refresh has changed, jitodo, cheatcheck!
		*/
		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		// jit -- only allow opengl32 or 3dfxgl:
		if (!Q_streq(gl_driver->string, "opengl32") && !Q_streq(gl_driver->string, "3dfxgl"))
			Cvar_Set("gl_driver", "opengl32");

//		Com_sprintf(name, sizeof(name), "ref_%s.dll", vid_ref->string );
		Com_sprintf(name, sizeof(name), "ref_pbgl.dll"); // jit

		if (!VID_LoadRefresh(name))
		{
			// jit3dfx - check if driver string changed:
			if (!Q_streq(gl_driver->string, driverstring)) 
			{
				vid_ref->modified = true;
				Q_strncpyz(driverstring, gl_driver->string, sizeof(driverstring));
			}

			if (!vid_ref->modified) // jit
				Com_Error(ERR_FATAL, "Unable to load OpenGL refresh!"); // jit

			// drop the console if we fail to load a refresh
			if (cls.key_dest != key_console)
				Con_ToggleConsole_f();
		}

		if (!vid_ref->modified) // jit3dfx
		{
			cls.disable_screen = false;
			CL_InitImages();
			M_ReloadMenu(); // jitmenu (need to do a full reload beacuse pic indexes can change)
		}
	}

	/*
	** update our window position
	*/
	if (vid_xpos->modified || vid_ypos->modified)
	{
		if (!vid_fullscreen->value)
			VID_UpdateWindowPosAndSize(vid_xpos->value, vid_ypos->value);

		vid_xpos->modified = false;
		vid_ypos->modified = false;
	}

	if (vid_borderless->modified) // jitborderless
	{
		vid_ref->modified = true; // force vid_restart
		vid_borderless->modified = false;
	}
}

// ===
// jitgamma -- for saving and restoring system gamma configuration

WORD desktop_gammaramp[3][256]; // jitgamma
qboolean gammaramp_supported;

void VID_BackupGamma() // jitgamma
{
	HDC	hdc;

	hdc = GetDC (GetDesktopWindow());
	gammaramp_supported = GetDeviceGammaRamp (hdc, &desktop_gammaramp[0][0]);
	ReleaseDC (GetDesktopWindow(), hdc);
}

void VID_RestoreGamma() // jitgamma
{
	if (gammaramp_supported)
	{
		HDC	hdc;

		hdc = GetDC (GetDesktopWindow());
		SetDeviceGammaRamp (hdc, &desktop_gammaramp[0][0]);
		ReleaseDC (GetDesktopWindow(), hdc);
	}
}

// jitgamma
// ===

/*
============
VID_Init
============
*/
void VID_Init (void)
{
	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get("vid_ref", "pbgl", CVAR_ARCHIVE); // jit
	vid_xpos = Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_borderless = Cvar_Get("vid_borderless", "0", CVAR_ARCHIVE); // jitborderless
	vid_borderless->modified = false; // jitborderless - prevent immediate refresh
	vid_gamma = Cvar_Get("vid_gamma", "1", CVAR_ARCHIVE);
	Cvar_Get("vid_lighten", "0", CVAR_ARCHIVE); // jitgamma
	Cvar_Get("gl_screenshot_applygamma", "1", CVAR_ARCHIVE); // jitgamma
	win_noalttab = Cvar_Get("win_noalttab", "0", CVAR_ARCHIVE);
	win_noaltenter = Cvar_Get("win_noaltenter", "0", CVAR_ARCHIVE); // jitkeyboard
	cl_customkeyboard = Cvar_Get("cl_customkeyboard", "0", CVAR_ARCHIVE); // jitkeyboard

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_front", VID_Front_f);

	/* Disable the 3Dfx splash screen */
	putenv("FX_GLIDE_NO_SPLASH=0");

	/* Start the graphics mode and load refresh DLL */
	VID_BackupGamma(); //jitgamma
	VID_CheckChanges();
}


/*
============
VID_Shutdown
============
*/
void VID_Shutdown (void)
{
	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();

		if (e.s)
			e.s(NULL);

		if (testlib)
			FreeLibrary(testlib);

		VID_RestoreGamma(); // jitgamma
	}
}

