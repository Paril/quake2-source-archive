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
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "../client/client.h"
#include "winquake.h"

extern	unsigned	sys_msg_time;

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5

enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn, AxisUp
};

DWORD	dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

DWORD	dwAxisMap[JOY_MAX_AXES];
DWORD	dwControlMap[JOY_MAX_AXES];
PDWORD	pdwRawValue[JOY_MAX_AXES];

cvar_t	*in_mouse;
cvar_t	*in_joystick;


// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
cvar_t	*joy_name;
cvar_t	*joy_advanced;
cvar_t	*joy_advaxisx;
cvar_t	*joy_advaxisy;
cvar_t	*joy_advaxisz;
cvar_t	*joy_advaxisr;
cvar_t	*joy_advaxisu;
cvar_t	*joy_advaxisv;
cvar_t	*joy_forwardthreshold;
cvar_t	*joy_sidethreshold;
cvar_t	*joy_pitchthreshold;
cvar_t	*joy_yawthreshold;
cvar_t	*joy_forwardsensitivity;
cvar_t	*joy_sidesensitivity;
cvar_t	*joy_pitchsensitivity;
cvar_t	*joy_yawsensitivity;
cvar_t	*joy_upthreshold;
cvar_t	*joy_upsensitivity;

cvar_t *m_noaccel; // jitmouse
cvar_t *m_rawinput; // jitmouse

qboolean	joy_avail, joy_advancedinit, joy_haspov;
DWORD		joy_oldbuttonstate, joy_oldpovstate;

int			joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;

static JOYINFOEX	ji;

qboolean	in_appactive;
qboolean	g_windowed = false;

int g_raw_mouse_x = 0; // jitmouse
int g_raw_mouse_y = 0; // jitmouse

// forward-referenced functions
void IN_StartupJoystick (void);
void Joy_AdvancedUpdate_f (void);
void IN_JoyMove (usercmd_t *cmd);

/*
============================================================

  MOUSE CONTROL

============================================================
*/

// mouse variables
cvar_t	*m_filter;

qboolean	mlooking;

void IN_MLookDown (void)
{
	mlooking = true;
}

void IN_MLookUp (void)
{
	mlooking = false;
	if (!freelook->value && lookspring->value)
		IN_CenterView ();
}

int			mouse_buttons;
int			mouse_oldbuttonstate;
POINT		current_pos;
float		mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum; // jitmouse - use float, so we don't lose as much precision

qboolean	mouseactive;	// false when not focus app

qboolean	restore_spi;
qboolean	mouseinitialized;
int			originalmouseparms[3];
int			newmouseparms_noaccel[3] = { 0, 0, 0 }; // jitmouse
qboolean	mouseparmsvalid;

int			window_center_x, window_center_y;
RECT		window_rect;


void IN_ShowCursor () // jitmouse
{
	// So windows has this crazy thing where it increments each time you call this.  We want to keep it at 0 for normal showing.
	int showval;

	while ((showval = ShowCursor(TRUE)) < 0)
		;

	if (showval > 0)
		ShowCursor(FALSE);
}


void IN_HideCursor () // jitmouse
{
	// All this crazy stuff is to keep the cursor counter from decrementing too much when we hide it (since it's called every frame in some cases).
	// Not sure if it's actually necessary, but might as well do it to be safe
	int showval;

	while ((showval = ShowCursor(FALSE)) >= 0)
		;

	if (showval < -1)
		ShowCursor(TRUE);
}


/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (qboolean clipcursor)
{
	int		width, height;
	static qboolean was_menu_active = false;

	// NiceAss: reset mouse settings if m_noaccel (formerly m_xp) changes
	if (m_noaccel->modified)
	{
		mouseactive = false;
		m_noaccel->modified = false;
	}

	if (!mouseinitialized)
		return;

	if (!in_mouse->value)
	{
		mouseactive = false;
		return;
	}

	// Check for a menu state change
	if (M_MenuActive() != was_menu_active)
	{
		mouseactive = false;
		was_menu_active = M_MenuActive();
	}

	if (mouseactive)
		return;

	mouseactive = true;

	if (mouseparmsvalid)
	{
		if (m_noaccel->value) // jitmouse - ignore windows version checks so this works for Windows 7 and hopefully future versions.
		{
			restore_spi = SystemParametersInfo(SPI_SETMOUSE, 0, newmouseparms_noaccel, 0);
		}
		else
		{
			restore_spi = SystemParametersInfo(SPI_SETMOUSE, 0, originalmouseparms, 0);
		}
	}

	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(cl_hwnd, &window_rect);
	window_center_x = (window_rect.right + window_rect.left) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;

	// jitmenu - in windowed mode, don't clip the cursor for the menu.
	if (clipcursor)
	{
		SetCursorPos(window_center_x, window_center_y);
		SetCapture(cl_hwnd);
		ClipCursor(&window_rect);
	}
	else
	{
		ReleaseCapture();
		ClipCursor(NULL);
	}

	IN_HideCursor();

	// jitmouse -- raw input support:
	{
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
		Rid[0].dwFlags = RIDEV_INPUTSINK;   
		Rid[0].hwndTarget = cl_hwnd;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
	}
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;

	if (!mouseactive)
		return;

	if (restore_spi)
		SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);

	mouseactive = false;
	ClipCursor(NULL);
	ReleaseCapture();
	IN_ShowCursor();
}



/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	cvar_t		*cv;

	cv = Cvar_Get("in_initmouse", "1", CVAR_NOSET);

	if (!cv->value) 
		return; 

	mouseinitialized = true;
	mouseparmsvalid = SystemParametersInfo(SPI_GETMOUSE, 0, originalmouseparms, 0);
	mouse_buttons = 9; // jitmouse (was 3)
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;

	if (!mouseinitialized)
		return;

	// perform button actions
	for (i = 0; i < mouse_buttons; i++)
	{
		if ((mstate & (1<<i)) &&
			!(mouse_oldbuttonstate & (1<<i)))
		{
			Key_Event(K_MOUSE1 + i, true, sys_msg_time);
		}

		if (!(mstate & (1<<i)) &&
			(mouse_oldbuttonstate & (1<<i)))
		{
			Key_Event(K_MOUSE1 + i, false, sys_msg_time);
		}
	}	
		
	mouse_oldbuttonstate = mstate;
}



/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;
	float	fovscale = 1.0f; // jitmouse

	if (!mouseactive)
		return;

	// find mouse movement
	if (!GetCursorPos(&current_pos))
		return;

	// jitmenu - if we're in windowed mode in the menu, use the actual Windows mouse coordinates for the cursor position.
	if (g_windowed && M_MenuActive())
	{
		ScreenToClient(cl_hwnd, &current_pos);
		M_MouseSet(current_pos.x, current_pos.y);

		// If we're outside of the actual game area of the window, show the cursor again, so people can see it to resize/close/etc. the window
		if (current_pos.x < 0 || current_pos.y < 0 || current_pos.x > viddef.width || current_pos.y > viddef.height)
		{
			IN_ShowCursor();
		}
		else
		{
			IN_HideCursor();
		}

		return;
	}

	if (m_rawinput->value) // jitmouse
	{
		mx = g_raw_mouse_x;
		my = g_raw_mouse_y;
		g_raw_mouse_x = 0;
		g_raw_mouse_y = 0;
	}
	else
	{
		mx = current_pos.x - window_center_x;
		my = current_pos.y - window_center_y;
	}

	// force the mouse to the center, so there's room to move
	if (mx || my)
		SetCursorPos(window_center_x, window_center_y);

	if (m_filter->value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5f;
		mouse_y = (my + old_mouse_y) * 0.5f;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	// If the menu is visible, move the menu cursor
	if (M_MenuActive())
	{
		M_MouseMove(mx, my);
		return;
	}

	if (m_fovscale->value && cl.refdef.fov_x > 0)
		fovscale = cl.refdef.fov_x / 90.0f; // jitmouse - scale turn speed based of FOV

	mouse_x *= sensitivity->value * fovscale;
	mouse_y *= sensitivity->value * fovscale;

// add mouse X/Y movement to cmd
	if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	if ((mlooking || freelook->value) && !(in_strafe.state & 1))
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
	else
		cmd->forwardmove -= m_forward->value * mouse_y;
}


/*
=========================================================================

VIEW CENTERING

=========================================================================
*/

cvar_t	*v_centermove;
cvar_t	*v_centerspeed;


/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	// mouse variables
	m_filter				= Cvar_Get ("m_filter",					"0",		CVAR_ARCHIVE);
    in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE);
	m_noaccel				= Cvar_Get ("m_noaccel",				"1",		CVAR_ARCHIVE);
	m_rawinput				= Cvar_Get ("m_rawinput",				"1",		0); // jitmouse

	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);
	joy_name				= Cvar_Get ("joy_name",					"joystick",	0);
	joy_advanced			= Cvar_Get ("joy_advanced",				"0",		0);
	joy_advaxisx			= Cvar_Get ("joy_advaxisx",				"0",		0);
	joy_advaxisy			= Cvar_Get ("joy_advaxisy",				"0",		0);
	joy_advaxisz			= Cvar_Get ("joy_advaxisz",				"0",		0);
	joy_advaxisr			= Cvar_Get ("joy_advaxisr",				"0",		0);
	joy_advaxisu			= Cvar_Get ("joy_advaxisu",				"0",		0);
	joy_advaxisv			= Cvar_Get ("joy_advaxisv",				"0",		0);
	joy_forwardthreshold	= Cvar_Get ("joy_forwardthreshold",		"0.15",		0);
	joy_sidethreshold		= Cvar_Get ("joy_sidethreshold",		"0.15",		0);
	joy_upthreshold  		= Cvar_Get ("joy_upthreshold",			"0.15",		0);
	joy_pitchthreshold		= Cvar_Get ("joy_pitchthreshold",		"0.15",		0);
	joy_yawthreshold		= Cvar_Get ("joy_yawthreshold",			"0.15",		0);
	joy_forwardsensitivity	= Cvar_Get ("joy_forwardsensitivity",	"-1",		0);
	joy_sidesensitivity		= Cvar_Get ("joy_sidesensitivity",		"-1",		0);
	joy_upsensitivity		= Cvar_Get ("joy_upsensitivity",		"-1",		0);
	joy_pitchsensitivity	= Cvar_Get ("joy_pitchsensitivity",		"1",		0);
	joy_yawsensitivity		= Cvar_Get ("joy_yawsensitivity",		"-1",		0);

	// centering
	v_centermove			= Cvar_Get ("v_centermove",				"0.15",		0);
	v_centerspeed			= Cvar_Get ("v_centerspeed",			"500",		0);

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

	Cmd_AddCommand ("joy_advancedupdate", Joy_AdvancedUpdate_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	IN_DeactivateMouse();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active)
{
	in_appactive = active;

	if (!(cls.key_dest == key_console && 
		Cvar_VariableValue("vid_fullscreen") == 0)) // jitmouse -- stop mouse from recentering when quake2 not active!
	{
		mouseactive = !active;		// force a new window check or turn off
	}
}


void CheckActive (HWND hWnd);

/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void)
{
	CheckActive(cl_hwnd);

	if (!mouseinitialized)
		return;

	if (!in_mouse || !in_appactive)
	{
		IN_DeactivateMouse();
		return;
	}

	g_windowed = Cvar_VariableValue("vid_fullscreen") == 0.0f;

	if (!cl.refresh_prepped
		|| cls.key_dest == key_console
		)//|| cls.key_dest == key_menu) // jitmenu
	{
		// temporarily deactivate if in fullscreen
		if (g_windowed && !M_MenuActive()) // jitmenu / jitmouse
		{
			IN_DeactivateMouse();
			return;
		}
	}

	IN_ActivateMouse(!(M_MenuActive() && g_windowed)); // jitmenu - don't clip the cursor when we're on the menu in windowed mode.
}


/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (ActiveApp) // jit/Echon - fix mouse recentering when clicking out of game.
	{
		IN_MouseMove(cmd);
		IN_JoyMove(cmd);
	}
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
	mx_accum = 0;
	my_accum = 0;
	mouse_oldbuttonstate = 0;
}


/*
=========================================================================

JOYSTICK

=========================================================================
*/

/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
	int			numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;
	cvar_t		*cv;

 	// assume no joystick
	joy_avail = false; 

	// abort startup if user requests no joystick
	cv = Cvar_Get ("in_initjoy", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 
 
	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
//		Com_Printf ("\nJoystick not found:\nDriver not present.\n\n");
		return;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id=0 ; joy_id<numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx (joy_id, &ji)) == JOYERR_NOERROR)
			break;
	} 

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Com_Printf ("\nJoystick not found:\nNo valid joysticks (%x).\n\n", mmr);
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Com_Printf ("\nJoystick not found:\nInvalid joystick capabilities (%x).\n\n", mmr); 
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_avail = true; 
	joy_advancedinit = false;

	Com_Printf ("\nJoystick detected.\n\n"); 
}


/*
===========
RawValuePointer
===========
*/
PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
	default: // jit -- kill warning
		return &ji.dwVpos;
	}
}


/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Joy_AdvancedUpdate_f (void)
{

	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD dwTemp;

	// initialize all the maps
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if ( joy_advanced->value == 0.0)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (!Q_streq(joy_name->string, "joystick"))
		{
			// notify user of advanced controller
			Com_Printf ("\n%s configured.\n\n", joy_name->string);
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD) joy_advaxisx->value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy->value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz->value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr->value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu->value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv->value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (dwAxisMap[i] != AxisNada)
		{
			joy_flags |= dwAxisFlags[i];
		}
	}
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail)
	{
		return;
	}

	
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i=0 ; i < joy_numbuttons ; i++)
	{
		if ( (buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true, 0);
		}

		if ( !(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false, 0);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if (ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i < 4 ; i++)
		{
			if ( (povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, true, 0);
			}

			if ( !(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, false, 0);
			}
		}
		joy_oldpovstate = povstate;
	}
}


/* 
=============== 
IN_ReadJoystick
=============== 
*/  
qboolean IN_ReadJoystick (void)
{

	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR)
	{
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Com_Printf ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
}


/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;
	float	fAxisValue;
	int		i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if ( joy_advancedinit != true )
	{
		Joy_AdvancedUpdate_f();
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
	if (!joy_avail || !in_joystick->value)
	{
		return; 
	}
 
	// collect the joystick data, if possible
	if (IN_ReadJoystick () != true)
	{
		return;
	}

	if ( (in_speed.state & 1) ^ (int)cl_run->value)
		speed = 2;
	else
		speed = 1;
	aspeed = speed * cls.frametime;

	// loop through the axes
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = (float) *pdwRawValue[i];
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0;

		switch (dwAxisMap[i])
		{
		case AxisForward:
			if ((joy_advanced->value == 0.0) && mlooking)
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold->value)
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is false)
					if (m_pitch->value < 0.0)
					{
						cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold->value)
				{
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity->value) * speed * cl_forwardspeed->value;
				}
			}
			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold->value)
			{
				cmd->sidemove += (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
			}
			break;

		case AxisUp:
			if (fabs(fAxisValue) > joy_upthreshold->value)
			{
				cmd->upmove += (fAxisValue * joy_upsensitivity->value) * speed * cl_upspeed->value;
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold->value)
				{
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold->value)
				{
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * aspeed * cl_yawspeed->value;
					}
					else
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * speed * 180.0;
					}

				}
			}
			break;

		case AxisLook:
//			if (mlooking) // jitjoystick - disabled -- dunno why this was there, but it breaks it
			{
				if (fabs(fAxisValue) > joy_pitchthreshold->value)
				{
					// pitch movement detected and pitch movement desired by user
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * speed * 180.0;
					}
				}
			}
			break;

		default:
			break;
		}
	}
}

