/*
Copyright (C) 1997-2001 Id Software, Inc., 2008 Nathan "jitspoe" Wulf

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

// Cross-platform compatible code to avoid redundant functions for each OS.


#include <assert.h>
#include <float.h>

#include "client.h"

/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void VID_Restart_f (void)
{
	vid_ref->modified = true;
}

/*
** VID_NewWindow
*/
void VID_NewWindow (int width, int height)
{
	viddef.width = width;
	viddef.height = height;
	cl.force_refdef = true;		// can't use a paused refdef - Linux code didn't have this, should we remove it for linux?
}


void VID_Error (int err_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;
	
	va_start(argptr,fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr); // jitsecurity -- prevent buffer overruns
	va_end(argptr);
	NULLTERMINATE(msg); // jitsecurity -- make sure string is null terminated.
	Com_Error(err_level,"%s", msg);
}

