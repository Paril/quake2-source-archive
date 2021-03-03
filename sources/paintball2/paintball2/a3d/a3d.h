/**********************************
Copyright (C) mSparks 2K1 (markj.parker@ntlworld.com - www.cam-direct.co.uk)
This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307, USA.
	Or go to http://www.gnu.org/copyleft/lgpl.html
***********************************/
// a3d.h -- A3D sound functions

/*#define Q2A3DAPI __declspec(dllimport)
#define Q2A3DAPIENTRY __cdecl*/
#ifndef A3D_H
#define A3D_H

#ifndef __unix__
#include <Windows.h>
#endif

typedef struct {

	void (*A3D_OggStreamStart)(char *filename);
	void (*A3D_OggStreamStop)(void);
	void (*A3D_PCMStream) (int samples, int rate, int width, int channels, void *data);
	void (*A3D_Update)(float origin[3], float forward[3]);
	int (*Init_A3D)(void);
	void (*A3D_Shutdown)(void);
	int (*A3D_cachefile)(char *filename,void *A3D_buffer);
	int (*A3D_StartSound)(float origin[3], float forward[3],float listener_origin[3], char *filename,  float attenuation, float volume, int id,int Env);
	int (*A3D_UpdateSound)(int id,char *name,float origin[3],float forward[3],float listener_origin[3],float attenuation);
	void (*A3D_StopSound)(int id);
	void (*A3D_SetEnv)(char *envname);
#ifdef __unix__
	void *reflib;
#else
	HINSTANCE reflib;
#endif

} s_a3d_t;

extern s_a3d_t a3d;

#endif



