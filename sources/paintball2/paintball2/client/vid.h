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
// vid.h -- video driver defs

#ifndef _VID_H_
#define _VID_H_

#define	MAXPRINTMSG	4096

typedef struct vrect_s
{
	int				x,y,width,height;
} vrect_t;

typedef struct
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

typedef struct vidmode_s
{
	const char *description;
	int         width, height;
	int         mode;
} vidmode_t;

extern	viddef_t	viddef;				// global video state

// Video module initialisation etc
void VID_Init (void);
void VID_Shutdown (void);
void VID_CheckChanges (void);
void VID_Restart_f (void);
void VID_MenuDraw (void);
const char *VID_MenuKey (int);
void VID_NewWindow (int width, int height);
void VID_Error (int err_level, char *fmt, ...);

extern	cvar_t		*vid_gamma;
extern	cvar_t		*vid_ref;			// Name of Refresh DLL loaded
extern	cvar_t		*vid_xpos;			// X coordinate of window position
extern	cvar_t		*vid_ypos;			// Y coordinate of window position
extern	cvar_t		*vid_fullscreen;
extern	cvar_t		*vid_borderless;	// jitborderless

#endif

