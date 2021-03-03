/*
** GL_SDL.C
**
** This file contains ALL Linux specific stuff having to do with the
** software refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_InitGraphics
** GLimp_SetPalette
** GLimp_Shutdown
** GLimp_SwitchFullscreen
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "../ref_gl/gl_local.h"
#include "../linux/glw_linux.h"

#include "../client/keys.h"
#include "rw_linux.h"

#ifdef Joystick
#include "joystick.h"
#endif

/*****************************************************************************/

static qboolean                 X11_active = false;

qboolean have_stencil = false;
extern cvar_t	*use_stencil;

static SDL_Surface *surface;

struct
{
	int key;
	int down;
} keyq[64];

int keyq_head=0;
int keyq_tail=0;
int config_notify=0;
int config_notify_width;
int config_notify_height;

glwstate_t glw_state;
      
// Console variables that we need to access from this module

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

#define MOUSE_MAX 3000
#define MOUSE_MIN 40
qboolean mouse_active;
int mx, my, mouse_buttonstate;

// this is inside the renderer shared lib, so these are called from vid_so
static float old_windowed_mouse;

static cvar_t	*_windowed_mouse;
extern cvar_t   *print_keymap;

/************************
 * Joystick
 ************************/
#ifdef Joystick
static SDL_Joystick *joy;
static int joy_oldbuttonstate;
static int joy_numbuttons;
#endif

void RW_IN_PlatformInit() {
	_windowed_mouse = ri.Cvar_Get ("_windowed_mouse", "1", CVAR_ARCHIVE);
}

#ifdef Joystick
qboolean CloseJoystick(void) 
{
	if (joy) {
		SDL_JoystickClose(joy);
		joy = NULL;
	}
	
	return true;
}

void PlatformJoyCommands(int *axis_vals, int *axis_map) 
{
	int i;
	int key_index;
	in_state_t *in_state = getState();
	
	if (joy) {
		for (i=0 ; i < joy_numbuttons ; i++) {
			if ( SDL_JoystickGetButton(joy, i) && joy_oldbuttonstate!=i ) {
				key_index = (i < 4) ? K_JOY1 : K_AUX1;
				in_state->Key_Event_fp (key_index + i, true);
				joy_oldbuttonstate = i;
      			}
      
      			if ( !SDL_JoystickGetButton(joy, i) && joy_oldbuttonstate!=i ) {
				key_index = (i < 4) ? K_JOY1 : K_AUX1;
				in_state->Key_Event_fp (key_index + i, false);
				joy_oldbuttonstate = i;
      			}
    		}
    		for (i=0; i < 6; i++) {
      			axis_vals[axis_map[i]] = (int)SDL_JoystickGetAxis(joy, i);
    		}
  	}
}
#endif

void RW_IN_Activate(qboolean active)
{
	mx = my = 0;
}

/*****************************************************************************/

int XLateKey(unsigned int keysym)
{
	int key;
  
	key = 0;
	switch(keysym) {
		case SDLK_KP9: key = K_KP_PGUP; break;
		case SDLK_PAGEUP: key = K_PGUP; break;
		case SDLK_KP3: key = K_KP_PGDN; break;
		case SDLK_PAGEDOWN: key = K_PGDN; break;
		case SDLK_KP7: key = K_KP_HOME; break;
		case SDLK_HOME: key = K_HOME; break;
		case SDLK_KP1: key = K_KP_END; break;
		case SDLK_END: key = K_END; break;
		case SDLK_KP4: key = K_KP_LEFTARROW; break;
		case SDLK_LEFT:key = K_LEFTARROW; break;
		case SDLK_KP6:key = K_KP_RIGHTARROW; break;
		case SDLK_RIGHT:key = K_RIGHTARROW; break;
		case SDLK_KP2:key = K_KP_DOWNARROW; break;
		case SDLK_DOWN:key = K_DOWNARROW; break;
		case SDLK_KP8:key = K_KP_UPARROW; break;
		case SDLK_UP:key = K_UPARROW; break;
		case SDLK_ESCAPE:key = K_ESCAPE; break;
		case SDLK_KP_ENTER:key = K_KP_ENTER; break;
		case SDLK_RETURN:key = K_ENTER; break;
		case SDLK_TAB:key = K_TAB; break;
		case SDLK_F1:key = K_F1; break;
		case SDLK_F2:key = K_F2; break;
		case SDLK_F3:key = K_F3; break;
		case SDLK_F4:key = K_F4; break;
		case SDLK_F5:key = K_F5; break;
		case SDLK_F6:key = K_F6; break;
		case SDLK_F7:key = K_F7; break;
		case SDLK_F8:key = K_F8; break;
		case SDLK_F9:key = K_F9; break;
		case SDLK_F10:key = K_F10; break;
		case SDLK_F11:key = K_F11; break;
		case SDLK_F12:key = K_F12; break;
		case SDLK_BACKSPACE:key = K_BACKSPACE; break;
		case SDLK_KP_PERIOD:key = K_KP_DEL; break;
		case SDLK_DELETE:key = K_DEL; break;
		case SDLK_PAUSE:key = K_PAUSE; break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:key = K_SHIFT; break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:key = K_CTRL; break;
		case SDLK_LMETA:
		case SDLK_RMETA:
		case SDLK_LALT:
		case SDLK_RALT:key = K_ALT; break;
		case SDLK_KP5:key = K_KP_5; break;
		case SDLK_INSERT:key = K_INS; break;
		case SDLK_KP0:key = K_KP_INS; break;
		case SDLK_KP_MULTIPLY:key = '*'; break;
		case SDLK_KP_PLUS:key = K_KP_PLUS; break;
		case SDLK_KP_MINUS:key = K_KP_MINUS; break;
		case SDLK_KP_DIVIDE:key = K_KP_SLASH; break;
		/* suggestions on how to handle this better would be appreciated */
		case SDLK_WORLD_7:key = '`'; break;
		
		case 92: key = '~'; break;/* Italy */ 
		case 94: key = '~'; break;  /* Deutschland */
		case 178: key = '~'; break; /* France */
		case 186: key = '~'; break; /* Spain */ 
		
		default: /* assuming that the other sdl keys are mapped to ascii */
		
		if (keysym < 256)
			key = keysym;
		break;
	}
	
	if (print_keymap->value) {
		printf( "Key '%c' (%d) -> '%c' (%d)\n", keysym, keysym, key, key );
	}
	
	return key;
}

static unsigned char KeyStates[SDLK_LAST];

void getMouse(int *x, int *y, int *state) 
{
	*x = mx;
	*y = my;
	
	*state = mouse_buttonstate;
}

void doneMouse()
{
	mx = my = 0;
}

void GetEvent(SDL_Event *event)
{
	unsigned int key;
	
	switch(event->type) {
		case SDL_MOUSEBUTTONDOWN:
			if (event->button.button == 4) {
				keyq[keyq_head].key = K_MWHEELUP;
				keyq[keyq_head].down = true;
				keyq_head = (keyq_head + 1) & 63;
				keyq[keyq_head].key = K_MWHEELUP;
				keyq[keyq_head].down = false;
				keyq_head = (keyq_head + 1) & 63;
			} 
			else if (event->button.button == 5) {
				keyq[keyq_head].key = K_MWHEELDOWN;
				keyq[keyq_head].down = true;
				keyq_head = (keyq_head + 1) & 63;
				keyq[keyq_head].key = K_MWHEELDOWN;
				keyq[keyq_head].down = false;
				keyq_head = (keyq_head + 1) & 63;
			} 
			break;
		case SDL_MOUSEBUTTONUP:
			break;
#ifdef Joystick
		case SDL_JOYBUTTONDOWN:
			keyq[keyq_head].key = ((((SDL_JoyButtonEvent*)event)->button < 4) ? K_JOY1 : K_AUX1) +
			                        ((SDL_JoyButtonEvent*)event)->button;
			keyq[keyq_head].down = true;
			keyq_head = (keyq_head+1)&63;
			break;
		case SDL_JOYBUTTONUP:
			keyq[keyq_head].key = ((((SDL_JoyButtonEvent*)event)->button < 4) ? K_JOY1 : K_AUX1) +
		                                ((SDL_JoyButtonEvent*)event)->button;
			keyq[keyq_head].down = false;
			keyq_head = (keyq_head+1)&63;
			break;
#endif
		case SDL_KEYDOWN:
			if ( (KeyStates[SDLK_LALT] || KeyStates[SDLK_RALT]) &&
			     (event->key.keysym.sym == SDLK_RETURN) ) {
				cvar_t *fullscreen;
	    
				SDL_WM_ToggleFullScreen(surface);
	    
				if (surface->flags & SDL_FULLSCREEN) {
					ri.Cvar_SetValue( "vid_fullscreen", 1 );
				} 
				else {
					ri.Cvar_SetValue( "vid_fullscreen", 0 );
				}
	    
				fullscreen = ri.Cvar_Get( "vid_fullscreen", "0", 0 );
				fullscreen->modified = false; /* we just changed it with SDL. */
	    
				break; /* ignore this key */
			}
	  
			if ( (KeyStates[SDLK_LCTRL] || KeyStates[SDLK_RCTRL]) &&
	                     (event->key.keysym.sym == SDLK_g) ) {
				SDL_GrabMode gm = SDL_WM_GrabInput(SDL_GRAB_QUERY);
				ri.Cvar_SetValue("_windowed_mouse", 
				                 (gm == SDL_GRAB_ON) ? /*1*/ 0 : /*0*/ 1 );
				break; /* ignore this key */
			}
	  
			KeyStates[event->key.keysym.sym] = 1;
			key = XLateKey(event->key.keysym.sym);
			
			if (key) {
				keyq[keyq_head].key = key;
				keyq[keyq_head].down = true;
				keyq_head = (keyq_head + 1) & 63;
			}
			break;
		case SDL_KEYUP:
			if (KeyStates[event->key.keysym.sym]) {
				KeyStates[event->key.keysym.sym] = 0;
				key = XLateKey(event->key.keysym.sym);
				if (key) {
					keyq[keyq_head].key = key;
					keyq[keyq_head].down = false;
					keyq_head = (keyq_head + 1) & 63;
				}
			}
			break;
		case SDL_QUIT:
			ri.Cmd_ExecuteText(EXEC_NOW, "quit");
			break;
	}
}

#ifdef Joystick
qboolean OpenJoystick(cvar_t *joy_dev) 
{
	int num_joysticks, i;
	joy = NULL;

	if (!(SDL_INIT_JOYSTICK&SDL_WasInit(SDL_INIT_JOYSTICK))) {
		ri.Con_Printf(PRINT_ALL, 
		             "SDL Joystick not initialized, trying to init...\n");
		SDL_Init(SDL_INIT_JOYSTICK);
	}

	ri.Con_Printf(PRINT_ALL, "Trying to start-up joystick...\n");
		
	if ((num_joysticks=SDL_NumJoysticks())) {
		
		for(i=0;i<num_joysticks;i++) {
			
			ri.Con_Printf(PRINT_ALL, "Trying joystick [%s]\n", 
			
			SDL_JoystickName(i));
			
			if (!SDL_JoystickOpened(i)) {
				joy = SDL_JoystickOpen(i);
				if (joy) {
					ri.Con_Printf(PRINT_ALL, "Joytick activated.\n");
					joy_numbuttons = SDL_JoystickNumButtons(joy);
					break;
				}
      			}
		}
		if (!joy) {
			ri.Con_Printf(PRINT_ALL, "Failed to open any joysticks\n");
			return false;
    		}
	}
	else {
		ri.Con_Printf(PRINT_ALL, "No joysticks available\n");
		return false;
	}
	return true;
}
#endif


/*****************************************************************************/

void SDL_Version()
{
        const SDL_version *version;
        version = SDL_Linked_Version();
        
        ri.Con_Printf(PRINT_ALL, "Linked against SDL version %d.%d.%d\n" 
                                 "Using SDL library version %d.%d.%d\n",
                                 SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
                                 version->major, version->minor, version->patch );
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the implementation
** specific stuff in a software rendering subsystem.
*/
int GLimp_Init( void *hInstance, void *wndProc )
{
	
	if (SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_CDROM|SDL_INIT_VIDEO) == 0) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			Sys_Error("SDL Init failed: %s\n", SDL_GetError());
			return false;
    		}
	} 
	else if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
			Sys_Error("SDL Init failed: %s\n", SDL_GetError());
			return false;
		}
	}
	
	SDL_Version();
  
	return true;
}

void *GLimp_GetProcAddress(const char *func)
{
	return SDL_GL_GetProcAddress(func);
}

static void SetSDLIcon()
{
#include "q2icon.xbm"
	SDL_Surface *icon;
	SDL_Color color;
	Uint8 *ptr;
	int i, mask;

	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, q2icon_width, q2icon_height, 
	                            8, 0, 0, 0, 0);
	if (icon == NULL)
		return; /* oh well... */
		
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);

	color.r = 255;
	color.g = 255;
	color.b = 255;
	SDL_SetColors(icon, &color, 0, 1); /* just in case */
	color.r = 0;
	color.g = 16;
	color.b = 0;
	SDL_SetColors(icon, &color, 1, 1);

	ptr = (Uint8 *)icon->pixels;
	for (i = 0; i < sizeof(q2icon_bits); i++) {
		for (mask = 1; mask != 0x100; mask <<= 1) {
			*ptr = (q2icon_bits[i] & mask) ? 1 : 0;
			ptr++;
		}		
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

/*
 * * UpdateGammaRamp *
 *
 * We are using gamma relative to the desktop, so that we can share it * with
 * software renderer and don't require to change desktop gamma * to match
 * hardware gamma image brightness. It seems that Quake 3 is * using the
 * opposite approach, but it has no software renderer after * all.
 */
unsigned short gamma_ramp[3][256]; // jitgamma
void UpdateGammaRamp(void)
{
	float g = (1.3 - vid_gamma->value + 1);
	g = (g > 1 ? g : 1);
	
	SDL_SetGamma(g, g, g);
}

void SetSDLGamma(void)
{
	gl_state.gammaramp = true;
	vid_gamma->modified = true;
	ri.Con_Printf(PRINT_ALL, "Using hardware gamma\n");
}

/*
** GLimp_InitGraphics
**
** This initializes the software refresh's implementation specific
** graphics subsystem.  In the case of Windows it creates DIB or
** DDRAW surfaces.
**
** The necessary width and height parameters are grabbed from
** vid.width and vid.height.
*/
static qboolean GLimp_InitGraphics( qboolean fullscreen )
{
	int flags;
	
	/* Just toggle fullscreen if that's all that has been changed */
	if (surface && (surface->w == vid.width) && (surface->h == vid.height)) {
		int isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;
		if (fullscreen != isfullscreen)
			SDL_WM_ToggleFullScreen(surface);

		isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;
		if (fullscreen == isfullscreen)
			return true;
	}
	
	srandom(getpid());

	// free resources in use
	if (surface)
		SDL_FreeSurface(surface);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (vid.width, vid.height);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (1) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        }
        else {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        }
	
	flags = SDL_OPENGL;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	
	SetSDLIcon(); /* currently uses q2icon.xbm data */
	
	if ((surface = SDL_SetVideoMode(vid.width, vid.height, 0, flags)) == NULL) {
		Sys_Error("(SDLGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return false;
	}

	// stencilbuffer shadows
 	{
		int stencil_bits;
	  
		have_stencil = false;
	  
		if (!SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_bits)) {
			ri.Con_Printf(PRINT_ALL, "I got %d bits of stencil\n", 
			              stencil_bits);
		
			if (stencil_bits >= 1) {
				have_stencil = true;
	    		}
	  	}
	}

	SDL_WM_SetCaption(WINDOW_CLASS_NAME, WINDOW_CLASS_NAME);

	SDL_ShowCursor(0);

	X11_active = true;
		
	SetSDLGamma();
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	return true;
}

void GLimp_BeginFrame( float camera_separation )
{
}

/*
** GLimp_EndFrame
**
** This does an implementation specific copy from the backbuffer to the
** front buffer.  In the Win32 case it uses BitBlt or BltFast depending
** on whether we're using DIB sections/GDI or DDRAW.
*/

void GLimp_EndFrame (void)
{
	SDL_GL_SwapBuffers();
	rs_realtime = (float)Sys_Milliseconds() * 0.001f; // jitrscript
}

/*
** GLimp_SetMode
*/
int GLimp_SetMode( int *pwidth, int *pheight, int width, int height, qboolean fullscreen )
{
	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode );

	*pwidth = width;
	*pheight = height;

	ri.Con_Printf( PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if ( !GLimp_InitGraphics( fullscreen ) ) {
		// failed to set a valid mode in windowed mode
		return rserr_invalid_resolution;
	}

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** System specific graphics subsystem shutdown routine.  Destroys
** DIBs or DDRAW surfaces as appropriate.
*/
void GLimp_Shutdown( void )
{
	if (surface)
		SDL_FreeSurface(surface);
	surface = NULL;
	
	if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
		SDL_Quit();
	else
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		
	X11_active = false;
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}

//===============================================================================


/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/

Key_Event_fp_t Key_Event_fp;

void KBD_Init(Key_Event_fp_t fp)
{
	Key_Event_fp = fp;
}

void KBD_Update(void)
{
	SDL_Event event;
	static int KBD_Update_Flag;
  
	in_state_t *in_state = getState();

	if (KBD_Update_Flag == 1)
		return;
  
	KBD_Update_Flag = 1;
  
	// get events from x server
	if (X11_active) {
		int bstate;
		
		while (SDL_PollEvent(&event))
			GetEvent(&event);
      
		if (!mx && !my)
			SDL_GetRelativeMouseState(&mx, &my);
			mouse_buttonstate = 0;
			bstate = SDL_GetMouseState(NULL, NULL);
		
		if (SDL_BUTTON(1) & bstate)
			mouse_buttonstate |= (1 << 0);
		if (SDL_BUTTON(3) & bstate) /* quake2 has the right button be mouse2 */
			mouse_buttonstate |= (1 << 1);
		if (SDL_BUTTON(2) & bstate) /* quake2 has the middle button be mouse3 */
			mouse_buttonstate |= (1 << 2);
		if (SDL_BUTTON(6) & bstate)
			mouse_buttonstate |= (1 << 3);
		if (SDL_BUTTON(7) & bstate)
			mouse_buttonstate |= (1 << 4);
      
		if (old_windowed_mouse != _windowed_mouse->value) {
			old_windowed_mouse = _windowed_mouse->value;
	
			if (!_windowed_mouse->value) {
				/* ungrab the pointer */
				SDL_WM_GrabInput(SDL_GRAB_OFF);
			} 
			else {
				/* grab the pointer */
				SDL_WM_GrabInput(SDL_GRAB_ON);
			}
		}
		
		while (keyq_head != keyq_tail) {
			in_state->Key_Event_fp(keyq[keyq_tail].key, 
			                       keyq[keyq_tail].down);
			keyq_tail = (keyq_tail + 1) & 63;
		}
	}
	KBD_Update_Flag = 0;
}

void KBD_Close(void)
{
	keyq_head = 0;
	keyq_tail = 0;
	
	memset(keyq, 0, sizeof(keyq));
}

void Fake_glColorTableEXT( GLenum target, GLenum internalformat,
                             GLsizei width, GLenum format, GLenum type,
                             const GLvoid *table )
{
	byte temptable[256][4];
	byte *intbl;
	int i;

	for (intbl = (byte *)table, i = 0; i < 256; i++) {
		temptable[i][2] = *intbl++;
		temptable[i][1] = *intbl++;
		temptable[i][0] = *intbl++;
		temptable[i][3] = 255;
	}
	//qgl3DfxSetPaletteEXT((GLuint *)temptable);
}

