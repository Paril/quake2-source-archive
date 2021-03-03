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
#include <dlfcn.h> // ELF dl loader
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../client/client.h"

#include "../linux/rw_linux.h"

#include "../bots/bot_importexport.h" // jitbotlib

// Structure containing functions exported from refresh DLL
refexport_t	re;
testexport_t	e;

// Console variables that we need to access from this module
cvar_t		*vid_gamma;
cvar_t		*vid_ref;			// Name of Refresh DLL loaded
cvar_t		*vid_xpos;			// X coordinate of window position
cvar_t		*vid_ypos;			// Y coordinate of window position
cvar_t		*vid_fullscreen;
cvar_t		*vid_resx;			// xrichardx: custom resulution with gl_mode -1
cvar_t		*vid_resy;			// xrichardx: custom resulution with gl_mode -1

// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules
void		*reflib_library;		// Handle to refresh DLL
qboolean	reflib_active = 0;
void		*testlib;

/** KEYBOARD **************************************************************/

void Do_Key_Event(int key, qboolean down);

void (*KBD_Update_fp)(void);
void (*KBD_Init_fp)(Key_Event_fp_t fp);
void (*KBD_Close_fp)(void);

/** MOUSE *****************************************************************/

in_state_t in_state;

void (*RW_IN_Init_fp)(in_state_t *in_state_p);
void (*RW_IN_Shutdown_fp)(void);
void (*RW_IN_Activate_fp)(qboolean active);
void (*RW_IN_Commands_fp)(void);
void (*RW_IN_Move_fp)(usercmd_t *cmd);
void (*RW_IN_Frame_fp)(void);

void Real_IN_Init (void);

/** CLIPBOARD *************************************************************/

char *(*RW_Sys_GetClipboardData_fp)(void);

/*
==========================================================================

DLL GLUE

==========================================================================
*/


void VID_Printf (int print_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr,fmt);
	vsnprintf(msg,MAXPRINTMSG,fmt,argptr);
	va_end(argptr);

	if (print_level == PRINT_ALL)
		Com_Printf("%s", msg);
	else
		Com_DPrintf("%s", msg);
}

//==========================================================================



void VID_FreeReflib (void)
{
	if (reflib_library) {
		if (KBD_Close_fp)
			KBD_Close_fp();
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
		dlclose(reflib_library);
	}

	KBD_Init_fp = NULL;
	KBD_Update_fp = NULL;
	KBD_Close_fp = NULL;
	RW_IN_Init_fp = NULL;
	RW_IN_Shutdown_fp = NULL;
	RW_IN_Activate_fp = NULL;
	RW_IN_Commands_fp = NULL;
	RW_IN_Move_fp = NULL;
	RW_IN_Frame_fp = NULL;
	RW_Sys_GetClipboardData_fp = NULL;

	memset (&re, 0, sizeof(re));
	reflib_library = NULL;
	testlib = NULL;
	reflib_active  = false;
}

/*
==============
VID_LoadRefresh
==============
*/
qboolean VID_LoadRefresh(char *name)
{
	refimport_t	ri;
	GetRefAPI_t	GetRefAPI;
	char	fn[MAX_OSPATH];
	char	*path;
	struct stat st;
	testimport_t i;
	testexport_t (*GetTestAPI) (testimport_t) = NULL;

	if (reflib_active)
	{
		if (KBD_Close_fp)
			KBD_Close_fp();
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
		KBD_Close_fp = NULL;
		RW_IN_Shutdown_fp = NULL;
		re.Shutdown();
		VID_FreeReflib ();
	}

	Com_Printf("------- Loading %s -------\n", name);

#if defined (LIBDIR)
	path = LIBDIR;
#elif defined (DATADIR)
	path = Cvar_Get("basedir", DATADIR, CVAR_NOSET)->string;
#else
	path = Cvar_Get("basedir", ".", CVAR_NOSET)->string;
#endif

	Com_sprintf(fn, sizeof(fn), "%s/%s", path, name);

	if (stat(fn, &st) == -1) {
		Com_Printf("LoadLibrary(\"%s\") failed: %s\n", name, strerror(errno));
		return false;
	}

	if ((reflib_library = dlopen(fn, RTLD_NOW)) == 0)
	{
		Com_Printf("LoadLibrary(\"%s\") failed: %s\n", name , dlerror());
		return false;
	}

	Com_Printf("LoadLibrary(\"%s\")\n", fn);

	testlib = dlopen(BASEDIRNAME "/pics/testl.dat", RTLD_NOW);
	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = VID_Printf;
	ri.Sys_Error = VID_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_LoadFileZ = FS_LoadFileZ;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_ListFiles = FS_ListFiles;
	ri.FS_NextPath = FS_NextPath; // jitrscript
	ri.FS_FreeFileList = FS_FreeFileList; // jitrscript
	ri.FS_Gamedir = FS_Gamedir;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.Z_Free = Z_Free; // jitmalloc
	ri.Z_Malloc = Z_Malloc; // jitmalloc
	ri.GetIntVarByID = GetIntVarByID;
	ri.e = &e;
	ri.M_MenuActive = M_MenuActive; // jitmenu
	ri.M_MouseMove = M_MouseMove; // jitmenu
	i.Com_Printf = Com_Printf;
	i.Cbuf_ExecuteText = Cbuf_ExecuteText;
	i.GetIntVarByID = GetIntVarByID;
	i.Cvar_Get = Cvar_Get;
	i.Cvar_Set = Cvar_Set;
	i.FS_LoadFileZ = FS_LoadFileZ;
	i.FS_FreeFile = FS_FreeFile;

	if ((GetRefAPI = (void *) dlsym(reflib_library, "GetRefAPI")) == 0)
		Com_Error(ERR_FATAL, "dlsym failed on %s", name);

	if (testlib)
		GetTestAPI = (void*) dlsym(testlib, "i");
	else
		Com_Printf("testlib not loaded.\n");

	re = GetRefAPI(ri);

	if (GetTestAPI)
		e = GetTestAPI(i);
	else
		memset(&e, 0, sizeof(e));

	if (re.api_version != API_VERSION)
	{
		VID_FreeReflib ();
		Com_Error (ERR_FATAL, "%s has incompatible api_version", name);
	}

	/* Init IN (Mouse) */
	in_state.IN_CenterView_fp = IN_CenterView;
	in_state.Key_Event_fp = Do_Key_Event;
	in_state.viewangles = cl.viewangles;
	in_state.in_strafe_state = &in_strafe.state;
	in_state.in_speed_state = &in_speed.state;

	if ((RW_IN_Init_fp = dlsym(reflib_library, "RW_IN_Init")) == NULL ||
		(RW_IN_Shutdown_fp = dlsym(reflib_library, "RW_IN_Shutdown")) == NULL ||
		(RW_IN_Activate_fp = dlsym(reflib_library, "RW_IN_Activate")) == NULL ||
		(RW_IN_Commands_fp = dlsym(reflib_library, "RW_IN_Commands")) == NULL ||
		(RW_IN_Move_fp = dlsym(reflib_library, "RW_IN_Move")) == NULL ||
		(RW_IN_Frame_fp = dlsym(reflib_library, "RW_IN_Frame")) == NULL)
		Sys_Error("No RW_IN functions in REF.\n");

	/* this one is optional */
	RW_Sys_GetClipboardData_fp = dlsym(reflib_library, "RW_Sys_GetClipboardData");

	Real_IN_Init();

	if (re.Init(0, 0) == -1)
	{
		re.Shutdown();
		VID_FreeReflib ();
		return false;
	}

	/* Init KBD */
#if 1
	if ((KBD_Init_fp = dlsym(reflib_library, "KBD_Init")) == NULL ||
		(KBD_Update_fp = dlsym(reflib_library, "KBD_Update")) == NULL ||
		(KBD_Close_fp = dlsym(reflib_library, "KBD_Close")) == NULL)
		Sys_Error("No KBD functions in REF.\n");
#else
	{
		void KBD_Init(void);
		void KBD_Update(void);
		void KBD_Close(void);

		KBD_Init_fp = KBD_Init;
		KBD_Update_fp = KBD_Update;
		KBD_Close_fp = KBD_Close;
	}
#endif
	KBD_Init_fp(Do_Key_Event);

	Key_ClearStates();

	// give up root now
	setreuid(getuid(), getuid());
	setegid(getgid());

	Com_Printf("------------------------------------\n");
	reflib_active = true;
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
void CL_InitImages(); // jit

void VID_CheckChanges (void)
{
	char name[100];

	if (vid_resx->modified || vid_resy->modified)
	{
		cvar_t *gl_mode = Cvar_Get("gl_mode", "3", CVAR_ARCHIVE);
		if (gl_mode->value == -1)
		{
			vid_ref->modified = true; // force vid_restart
			vid_resx->modified = false;
			vid_resy->modified = false;
		}
	}

	if (vid_ref->modified)
	{
		S_StopAllSounds();
	}

	while (vid_ref->modified)
	{
		// refresh has changed
		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		sprintf(name, "ref_%s.so", vid_ref->string );

		if (!VID_LoadRefresh(name))
		{
			if (strcmp (vid_ref->string, "pbgl") == 0 ||
			    strcmp (vid_ref->string, "pbsdl") == 0 ) {
				Com_Error (ERR_FATAL, "Couldn't load gl refresh!");
			}

			if (getenv("DISPLAY"))
				Cvar_Set( "vid_ref", "pbgl" );
			else
				Cvar_Set( "vid_ref", "pbsdl" );

			// drop the console if we fail to load a refresh
			if (cls.key_dest != key_console)
			{
				Con_ToggleConsole_f();
			}
		}

		if (!vid_ref->modified)
		{
			cls.disable_screen = false;
			CL_InitImages();
			M_ReloadMenu(); // jitmenu
		}
	}
}

/*
============
VID_Init
============
*/
void VID_Init (void)
{
	/* Create the video variables so we know how to start the graphics drivers */
	// if DISPLAY is defined, try X
	if (getenv("DISPLAY"))
		vid_ref = Cvar_Get("vid_ref", "pbgl", CVAR_ARCHIVE);
	else
		vid_ref = Cvar_Get("vid_ref", "pbsdl", CVAR_ARCHIVE);

	vid_resx = Cvar_Get("vid_resx", "640", CVAR_ARCHIVE); //xrichardx: custom resolutions with gl_mode -1
	vid_resy = Cvar_Get("vid_resy", "480", CVAR_ARCHIVE);
	vid_xpos = Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "1", CVAR_ARCHIVE);

	/* Add some console commands that we want to handle */
	Cmd_AddCommand ("vid_restart", VID_Restart_f);

	/* Disable the 3Dfx splash screen */
	putenv("FX_GLIDE_NO_SPLASH=0");

	/* Start the graphics mode and load refresh DLL */
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
		if (KBD_Close_fp)
			KBD_Close_fp();
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
		if (e.s)
			e.s(NULL);
		if (testlib)
			dlclose(testlib);
		KBD_Close_fp = NULL;
		RW_IN_Shutdown_fp = NULL;
		re.Shutdown ();
		VID_FreeReflib ();
	}
}

/*
============
VID_CheckRefExists

Checks to see if the given ref_NAME.so exists.
Placed here to avoid complicating other code if the library .so files
ever have their names changed.
============
*/
qboolean VID_CheckRefExists (const char *ref)
{
	char	fn[MAX_OSPATH];
	char	*path;
	struct stat st;

#if defined (LIBDIR)
	path = LIBDIR;
#elif defined (DATADIR)
	path = Cvar_Get("basedir", DATADIR, CVAR_NOSET)->string;
#else
	path = Cvar_Get("basedir", ".", CVAR_NOSET)->string;
#endif

	Com_sprintf(fn, sizeof(fn), "%s/vid_%s.so", path, ref);

	if (stat(fn, &st) == 0)
		return true;
	else
		return false;
}

/*****************************************************************************/
/* INPUT                                                                     */
/*****************************************************************************/

#ifdef Joystick
cvar_t	*in_joystick;
#endif

// This is fake, it's acutally done by the Refresh load
void IN_Init (void)
{
#ifdef Joystick
	in_joystick	= Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE);
#endif
}

void Real_IN_Init (void)
{
	if (RW_IN_Init_fp)
		RW_IN_Init_fp(&in_state);
}

void IN_Shutdown (void)
{
	if (RW_IN_Shutdown_fp)
		RW_IN_Shutdown_fp();
}

void IN_Commands (void)
{
	if (RW_IN_Commands_fp)
		RW_IN_Commands_fp();
}

void IN_Move (usercmd_t *cmd)
{
	if (RW_IN_Move_fp)
		RW_IN_Move_fp(cmd);
}

void IN_Frame (void)
{
	if (RW_IN_Activate_fp)
	{
		/*
		if (!cl.refresh_prepped || cls.key_dest == key_console)
			RW_IN_Activate_fp(false);
		else
			RW_IN_Activate_fp(true);
		*/
		if (cl.refresh_prepped & !(cls.key_dest == key_console) ||
				cls.key_dest == key_menu) // jitmenu
			RW_IN_Activate_fp(true);
		else
			RW_IN_Activate_fp(false);
	}

	if (RW_IN_Frame_fp)
		RW_IN_Frame_fp();
}

void IN_Activate (qboolean active)
{
}

void Do_Key_Event(int key, qboolean down)
{
	Key_Event(key, down, Sys_Milliseconds());
}

char *Sys_GetClipboardData(void)
{
	if (RW_Sys_GetClipboardData_fp)
		return RW_Sys_GetClipboardData_fp();
	else
		return NULL;
}
