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
/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/
#include <assert.h>
#include <windows.h>
#include "../ref_gl/gl_local.h"
#include "../ref_gl/gl_cin.h" //Heffo - Cin Textures
#include "glw_win.h"
#include "winquake.h"
#include "resource.h"

static qboolean GLimp_SwitchFullscreen (int width, int height);
qboolean GLimp_InitGL (void);

glwstate_t glw_state;

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

qboolean have_stencil = false; // Stencil shadows - MrG

static qboolean VerifyDriver (void)
{
	char buffer[1024];

	strcpy(buffer, qgl.GetString(GL_RENDERER));
	strlwr(buffer);

	if (Q_streq(buffer, "gdi generic"))
		if (!glw_state.mcd_accelerated)
			return false;

	return true;
}



/*
** VID_CreateWindow
*/
//#define	WINDOW_CLASS_NAME	"Paintball 2" // jit
#ifdef QUAKE2
#define	WINDOW_CLASS_NAME	(ri.Cvar_Get("vid_windowclassname", "Quake 2", 0)->string) // jit
#else
#define	WINDOW_CLASS_NAME	(ri.Cvar_Get("vid_windowclassname", "Paintball 2", 0)->string) // jit
#endif

qboolean VID_CreateWindow (int width, int height, qboolean fullscreen)
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	cvar_t			*vid_borderless = ri.Cvar_Get("vid_borderless", "0", 0); // jitborderless
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)glw_state.wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;
    wc.hIcon         = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (void*)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass(&wc))
		ri.Sys_Error(ERR_FATAL, "Couldn't register window class.");

	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else
	{
		exstyle = 0;

		if (vid_borderless->value) // jitborderless
			stylebits = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_SYSMENU | WS_VISIBLE;
		else
			stylebits = WS_OVERLAPPEDWINDOW;
	}

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect(&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		POINT pt1, pt2;

		vid_xpos = ri.Cvar_Get("vid_xpos", "0", 0);
		vid_ypos = ri.Cvar_Get("vid_ypos", "0", 0);
		x = vid_xpos->value;
		y = vid_ypos->value;

		// === jit - make sure the window is visible on a monitor so it doesn't get stuck off-screen somewhere.
		pt1.x = x;
		pt1.y = y;
		pt2.x = x + 100;
		pt2.y = x + 100;
		
		if (!(MonitorFromPoint(pt1, MONITOR_DEFAULTTONULL) && MonitorFromPoint(pt2, MONITOR_DEFAULTTONULL)))
		{
			x = 3;
			y = 22;
		}
		// jit ===
	}

	glw_state.hWnd = CreateWindowEx (
		 exstyle, 
		 WINDOW_CLASS_NAME,
		 WINDOW_CLASS_NAME,
		 stylebits,
		 x, y, w, h,
		 NULL,
		 NULL,
		 glw_state.hInstance,
		 NULL);

	if (!glw_state.hWnd)
		ri.Sys_Error(ERR_FATAL, "Couldn't create window.");

	ShowWindow(glw_state.hWnd, SW_SHOW);
	UpdateWindow(glw_state.hWnd);

	// init all the gl stuff for the window
	if (!GLimp_InitGL())
	{
		ri.Con_Printf(PRINT_ALL, "VID_CreateWindow() - GLimp_InitGL failed.\n");
		return false;
	}

	SetForegroundWindow(glw_state.hWnd);
	SetFocus(glw_state.hWnd);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow(width, height);

	return true;
}


/*
** GLimp_SetMode
*/
void GL_UpdateSwapInterval (void);

rserr_t GLimp_SetMode (int *pwidth, int *pheight, int width, int height, qboolean fullscreen)
{
	const char *win_fs[] = { "W", "FS" };

	ri.Con_Printf(PRINT_ALL, "Initializing OpenGL display.\n");
	ri.Con_Printf(PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen]);

	// destroy the existing window
	if (glw_state.hWnd)
	{
		GLimp_Shutdown();
	}

	// do a CDS if needed
	if (fullscreen)
	{
		DEVMODE dm;
		cvar_t *r_displayrefresh; // jitrefresh
		
		r_displayrefresh = ri.Cvar_Get("r_displayrefresh", "0", CVAR_ARCHIVE); // jitrefresh
		ri.Con_Printf(PRINT_ALL, "...attempting fullscreen.\n");
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;

		//ep::windows xp refresh rate fix, jitrefresh
		if (r_displayrefresh->value > 0)
		{
			dm.dmDisplayFrequency = r_displayrefresh->value;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		}
		else
		{
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		}
		//ep::windows xp refresh rate fix 

		if (gl_bitdepth->value != 0)
		{
			dm.dmBitsPerPel = gl_bitdepth->value;
			dm.dmFields |= DM_BITSPERPEL;
			ri.Con_Printf(PRINT_ALL, "...using gl_bitdepth of %d.\n", (int)gl_bitdepth->value);
		}
		else
		{
			HDC hdc = GetDC(NULL);
			int bitspixel = GetDeviceCaps(hdc, BITSPIXEL);

			ri.Con_Printf(PRINT_ALL, "...using desktop display depth of %d.\n", bitspixel);
			ReleaseDC(0, hdc);
		}

		ri.Con_Printf(PRINT_ALL, "...calling CDS: ");

		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			*pwidth = width;
			*pheight = height;

			gl_state.fullscreen = true;

			ri.Con_Printf(PRINT_ALL, "ok\n");

			if (!VID_CreateWindow (width, height, true))
				return rserr_invalid_resolution;

			return rserr_ok;
		}
		else
		{
			*pwidth = width;
			*pheight = height;

			ri.Con_Printf(PRINT_ALL, "failed\n");

			ri.Con_Printf(PRINT_ALL, "...calling CDS assuming dual monitors:");

			dm.dmPelsWidth = width * 2;
			dm.dmPelsHeight = height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			if ( gl_bitdepth->value != 0 )
			{
				dm.dmBitsPerPel = gl_bitdepth->value;
				dm.dmFields |= DM_BITSPERPEL;
			}

			/*
			** our first CDS failed, so maybe we're running on some weird dual monitor
			** system 
			*/
			if ( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
			{
				ri.Con_Printf( PRINT_ALL, " failed\n" );

				ri.Con_Printf( PRINT_ALL, "...setting windowed mode.\n" );

				ChangeDisplaySettings( 0, 0 );

				*pwidth = width;
				*pheight = height;
				gl_state.fullscreen = false;

				if (!VID_CreateWindow(width, height, false))
					return rserr_invalid_resolution;

				return rserr_invalid_fullscreen;
			}
			else
			{
				ri.Con_Printf(PRINT_ALL, " ok\n");

				if (!VID_CreateWindow(width, height, true))
					return rserr_invalid_resolution;

				gl_state.fullscreen = true;

				return rserr_ok;
			}
		}
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "...setting windowed mode.\n");
		ChangeDisplaySettings(0, 0);
		*pwidth = width;
		*pheight = height;
		gl_state.fullscreen = false;

		if (!VID_CreateWindow(width, height, false))
			return rserr_invalid_resolution;
	}

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
WORD original_ramp[3][256];
//void *original_ramp; // jitgamma
WORD gamma_ramp[3][256];

void GLimp_Shutdown( void )
{
//jitgamma	if (vid_gamma_hw->value && gl_state.gammaramp)
//		SetDeviceGammaRamp(glw_state.hDC, original_ramp);

	if ( qgl.wMakeCurrent && !qgl.wMakeCurrent( NULL, NULL ) )
		ri.Con_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - wglMakeCurrent failed.\n");

	if ( glw_state.hGLRC )
	{
		if (  qgl.wDeleteContext && !qgl.wDeleteContext( glw_state.hGLRC ) )
			ri.Con_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - wglDeleteContext failed.\n");
		glw_state.hGLRC = NULL;
	}

	if (glw_state.hDC)
	{
		if ( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) )
			ri.Con_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - ReleaseDC failed.\n" );
		glw_state.hDC   = NULL;
	}

	if (glw_state.hWnd)
	{
		ShowWindow(glw_state.hWnd, SW_HIDE); // jit -- destroy leftovers in taskbar (credit: Tomaz)
		DestroyWindow(glw_state.hWnd);
		glw_state.hWnd = NULL;
	}

	if (glw_state.log_fp)
	{
		fclose(glw_state.log_fp);
		glw_state.log_fp = 0;
	}

	UnregisterClass(WINDOW_CLASS_NAME, glw_state.hInstance);

	if (gl_state.fullscreen)
	{
		ChangeDisplaySettings(0, 0);
		gl_state.fullscreen = false;
	}
}


/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/
qboolean GLimp_Init (void *hinstance, void *wndproc)
{
#define OSR2_BUILD_NUMBER 1111

	OSVERSIONINFO	vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	glw_state.allowdisplaydepthchange = false;

	if (GetVersionEx(&vinfo))
	{
		if (vinfo.dwMajorVersion > 4)
		{
			glw_state.allowdisplaydepthchange = true;
		}
		else if (vinfo.dwMajorVersion == 4)
		{
			if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				glw_state.allowdisplaydepthchange = true;
			}
			else if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				if (LOWORD(vinfo.dwBuildNumber) >= OSR2_BUILD_NUMBER)
				{
					glw_state.allowdisplaydepthchange = true;
				}
			}
		}
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - GetVersionEx failed.\n");
		return false;
	}

	glw_state.hInstance = (HINSTANCE)hinstance;
	glw_state.wndproc = wndproc;

	return true;
}

extern cvar_t *gl_debug; // jit

qboolean GLimp_InitGL (void)
{
    PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		32,								// 32-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		24,								// 24-bit z-buffer	
		8,								// 8bit stencil buffer
										// Stencil Shadows - MrG
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };
    int  pixelformat;
	cvar_t *stereo;
	
	stereo = ri.Cvar_Get("cl_stereo", "0", 0);

	/*
	** set PFD_STEREO if necessary
	*/
	if (stereo->value)
	{
		ri.Con_Printf(PRINT_ALL, "...attempting to use stereo.\n");
		pfd.dwFlags |= PFD_STEREO;
		gl_state.stereo_enabled = true;
	}
	else
	{
		gl_state.stereo_enabled = false;
	}

	/*
	** figure out if we're running on a minidriver or not
	*/
	if (strstr(gl_driver->string, "opengl32"))
		glw_state.minidriver = false;
	else
		glw_state.minidriver = true;

	/*
	** Get a DC for the specified window
	*/
	if (glw_state.hDC != NULL)
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - non-NULL DC exists.\n");

    if ((glw_state.hDC = GetDC(glw_state.hWnd)) == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - GetDC failed.\n");
		return false;
	}

	if (glw_state.minidriver)
	{
		if ((pixelformat = qgl.wChoosePixelFormat(glw_state.hDC, &pfd)) == 0)
		{
			ri.Con_Printf(PRINT_ALL, "GLimp_Init() - qgl.wChoosePixelFormat failed.\n");
			return false;
		}

		if (qgl.wSetPixelFormat(glw_state.hDC, pixelformat, &pfd) == FALSE)
		{
			ri.Con_Printf(PRINT_ALL, "GLimp_Init() - qgl.wSetPixelFormat failed.\n");
			return false;
		}

		qgl.wDescribePixelFormat(glw_state.hDC, pixelformat, sizeof(pfd), &pfd);
	}
	else
	{
		if ((pixelformat = ChoosePixelFormat(glw_state.hDC, &pfd)) == 0)
		{
			ri.Con_Printf (PRINT_ALL, "GLimp_Init() - ChoosePixelFormat failed.\n");
			return false;
		}

		if (SetPixelFormat(glw_state.hDC, pixelformat, &pfd) == FALSE)
		{
			ri.Con_Printf (PRINT_ALL, "GLimp_Init() - SetPixelFormat failed.\n");
			return false;
		}

		DescribePixelFormat(glw_state.hDC, pixelformat, sizeof(pfd), &pfd);

		// PFD_GENERIC_FORMAT only = software
		// PFD_GENERIC_ACCELERATED only = MCD accelerated
		// Neither PFD_GENERIC_ACCELERATED nor PFD_GENERIC_FORMAT = ICD accelerated
		// I imagine PFD_SUPPORT_OPENGL must be true, otherwise we're screwed.
		if ((!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT)) || !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
		{
			int mb;
			extern cvar_t *gl_allow_software;
			static int prompted = 0; // Only pop this up once, otherwise it will hit on the retry.

			if (!prompted)
			{
				prompted = 1;
				mb = MessageBox(NULL, "You do not have graphic drivers installed that support hardware accelerated OpenGL. Would you like to load a web page that will help you find appropriate drivers?", "Paintball 2 - No Drivers", MB_YESNO | MB_ICONEXCLAMATION);

				if (mb == IDYES)
				{
					ShellExecute(NULL, "open", "http://www.digitalpaint.org/drivers/", NULL, NULL, SW_SHOWNORMAL);
				}
			}

			if (gl_allow_software->value)
				glw_state.mcd_accelerated = true;
			else
				glw_state.mcd_accelerated = false;
		}
		else
		{
			glw_state.mcd_accelerated = true;
		}
	}

	/*
	** report if stereo is desired but unavailable
	*/
	if (!(pfd.dwFlags & PFD_STEREO) && (stereo->value != 0)) 
	{
		ri.Con_Printf(PRINT_ALL, "...failed to select stereo pixel format.\n");
		ri.Cvar_SetValue("cl_stereo", 0);
		gl_state.stereo_enabled = false;
	}

	/*
	** startup the OpenGL subsystem by creating a context and making
	** it current
	*/
	if ((glw_state.hGLRC = qgl.wCreateContext(glw_state.hDC)) == 0)
	{
		ri.Con_Printf (PRINT_ALL, "GLimp_Init() - qgl.wCreateContext failed.\n");
		goto fail;
	}

    if (!qgl.wMakeCurrent(glw_state.hDC, glw_state.hGLRC))
	{
		ri.Con_Printf (PRINT_ALL, "GLimp_Init() - qgl.wMakeCurrent failed.\n");
		goto fail;
	}

	if (!VerifyDriver())
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - no hardware acceleration detected.\n");
		goto fail;
	}

	/*
	** print out PFD specifics 
	*/
	ri.Con_Printf(PRINT_ALL, "GL PFD: Color(%dbits) Depth(%dbits) Stencil(%dbits)\n", 
		(int)pfd.cColorBits, (int)pfd.cDepthBits, (int)pfd.cStencilBits);

	{
		char buffer[1024];

		strcpy( buffer, qgl.GetString(GL_RENDERER));

		if (!((int)pfd.cStencilBits))  // jit
		{
			if (gl_debug->value)
				ri.Con_Printf(PRINT_ALL, "... No stencil buffer.\n");

			have_stencil = false;
		} 
		else 
		{
			if (pfd.cStencilBits) 
			{
				if (gl_debug->value)
					ri.Con_Printf(PRINT_ALL, "... Using stencil buffer.\n");

				have_stencil = true; // Stencil shadows - MrG
			}
		}
	}

//jitrscript - moved	RS_ScanPathForScripts(ri.FS_Gamedir());		// load all found scripts

	// Vertex arrays
	/*qgl.EnableClientState (GL_VERTEX_ARRAY);
	qgl.EnableClientState (GL_TEXTURE_COORD_ARRAY);

	qgl.TexCoordPointer (2, GL_FLOAT, sizeof(tex_array[0]), tex_array[0]);
	qgl.VertexPointer (3, GL_FLOAT, sizeof(vert_array[0]), vert_array[0]);
	qgl.ColorPointer (4, GL_FLOAT, sizeof(col_array[0]), col_array[0]);*/

	
	/*
		ZeroMemory(original_ramp,sizeof(original_ramp));
		gl_state.gammaramp = GetDeviceGammaRamp(glw_state.hDC,original_ramp);
	*/
	gl_state.gammaramp = GetDeviceGammaRamp(glw_state.hDC,original_ramp);

//jitgamma	if (!vid_gamma_hw->value)
//		gl_state.gammaramp = false;

	if (gl_state.gammaramp && vid_gamma_hw->value) // jitgamma
		vid_gamma->modified = true;

	return true;

fail:
	if ( glw_state.hGLRC )
	{
		qgl.wDeleteContext( glw_state.hGLRC );
		glw_state.hGLRC = NULL;
	}

	if ( glw_state.hDC )
	{
		ReleaseDC( glw_state.hWnd, glw_state.hDC );
		glw_state.hDC = NULL;
	}
	return false;
}

/*
** GLimp_BeginFrame
*/

void GLimp_BeginFrame( float camera_separation )
{
	if (gl_bitdepth->modified)
	{
		if (gl_bitdepth->value != 0 && !glw_state.allowdisplaydepthchange)
		{
			ri.Cvar_SetValue("gl_bitdepth", 0);
			ri.Con_Printf(PRINT_ALL, "gl_bitdepth requires Win95 OSR2.x or WinNT 4.x\n");
		}

		gl_bitdepth->modified = false;
	}

	if (camera_separation < 0 && gl_state.stereo_enabled)
	{
		qgl.DrawBuffer(GL_BACK_LEFT);
	}
	else if (camera_separation > 0 && gl_state.stereo_enabled)
	{
		qgl.DrawBuffer(GL_BACK_RIGHT);
	}
	else
	{
		qgl.DrawBuffer(GL_BACK);
	}

	//Heffo - CIN Texture Update
	CIN_ProcessCins();
}

// frame dump - MrG
// modified screenshot function
int cl_anim_count = 0;
void apply_gamma (byte *rgbdata, int w, int h); // jitgamma

void CL_AnimDump (void) 
{
	byte		*buffer;
	char		checkname[MAX_OSPATH];
	int			c, temp,o;
	unsigned int i;
	FILE		*f;

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/animdump", ri.FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 
	for (i = cl_anim_count; i <= 99999999; i++) 
	{ 
		Com_sprintf(checkname, sizeof(checkname), "%s/animdump/anim%5i.tga", ri.FS_Gamedir(), i);

		for (o = 0; o < strlen(checkname); o++)
			if (checkname[o] == ' ')
				checkname[o] = '0';

		f = fopen (checkname, "rb");

		if (!f)
			break;	// file doesn't exist

		fclose(f);
	} 

	if (i == 100000000) 
	{
		ri.Cvar_Set("cl_animdump", "0");
		ri.Con_Printf(PRINT_ALL, "CL_AnimDump: Max frames exported.\n"); 
		return;
 	}

	cl_anim_count = i;

	buffer = malloc(vid.width*vid.height*3 + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = vid.width&255;
	buffer[13] = vid.width>>8;
	buffer[14] = vid.height&255;
	buffer[15] = vid.height>>8;
	buffer[16] = 24;	// pixel size
	qgl.PixelStorei(GL_PACK_ALIGNMENT, 1);
	qgl.ReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer + 18); 
	apply_gamma(buffer + 18, vid.width, vid.height); // jitgamma -- apply video gammaramp to screenshot
	

	// swap rgb to bgr
	c = 18 + vid.width * vid.height * 3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	f = fopen (checkname, "wb");
	fwrite (buffer, 1, c, f);
	fclose (f);

	free (buffer);
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
extern cvar_t	*cl_animdump;


void GLimp_EndFrame (void)
{
#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif

	// frame dump - MrG
	if (cl_animdump->value)
		CL_AnimDump();

	if (Q_streq(gl_drawbuffer->string, "GL_BACK"))
	{
		if (!qgl.wSwapBuffers(glw_state.hDC))
		{
			static qboolean print = true;

			// Don't spam the console with this error.
			if (print)
			{
				ri.Con_Printf(PRINT_ALL, "GLimp_EndFrame() - SwapBuffers() failed!\n");
				print = false;
			}
		}
	}

	// rscript - MrG
	rs_realtime = (float)Sys_Milliseconds() * 0.001f;

#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif
}

void UpdateGammaRamp (void)
{
	int i, o;
	//int test;

	// jitgamma -- don't let them go to extreme values:
	if (vid_gamma->value < 0.7f)
		ri.Cvar_SetValue("vid_gamma", 0.7f);

	if (vid_gamma->value > 2.0f)
		ri.Cvar_SetValue("vid_gamma", 2.0f);

	if (vid_lighten->value > 0.5f)
		ri.Cvar_SetValue("vid_lighten", 0.5f);

	if (vid_lighten->value < 0.0f)
		ri.Cvar_SetValue("vid_lighten", 0.0f);

	if (gl_state.gammaramp)
	{
		float v, i_f;

		for (o = 0; o < 3; o++) // jitgamma
		{
			for(i = 0; i < 256; i++)
			{
				i_f = (float)i/255.0f;
				v = pow(i_f, vid_gamma->value);
				v += vid_lighten->value * (1.0f - v);

				if (v < 0.0f)
					v = 0.0f;

				else if (v > 1.0f)
					v = 1.0f;

				gamma_ramp[o][i] = (WORD)(v * 65535.0f + 0.5f);
			}
		}

		SetDeviceGammaRamp(glw_state.hDC, gamma_ramp);
	}
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate (qboolean active)
{
	if (active)
	{
		SetForegroundWindow(glw_state.hWnd);
		ShowWindow(glw_state.hWnd, SW_RESTORE);
	}
	else
	{
		if (vid_fullscreen->value)
			ShowWindow(glw_state.hWnd, SW_MINIMIZE);
	}
}
