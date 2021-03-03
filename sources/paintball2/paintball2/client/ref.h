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
#ifndef __REF_H
#define __REF_H

#include "../qcommon/qcommon.h"

#define	MAX_DLIGHTS		32
#define	MAX_STAINS		32
#define	MAX_ENTITIES	512 // jit (was 128)
#define	MAX_PARTICLES	4096
#define	MAX_LIGHTSTYLES	256

#define MAX_MESHSKINS	32 // jitskm

#define POWERSUIT_SCALE		4.0F

#define SHELL_RED_COLOR		0xF2
#define SHELL_GREEN_COLOR	0xD0
#define SHELL_BLUE_COLOR	0xF3

#define SHELL_RG_COLOR		0xDC
//#define SHELL_RB_COLOR		0x86
#define SHELL_RB_COLOR		0x68
#define SHELL_BG_COLOR		0x78

//ROGUE
#define SHELL_DOUBLE_COLOR	0xDF // 223
#define	SHELL_HALF_DAM_COLOR	0x90
#define SHELL_CYAN_COLOR	0x72
//ROGUE

#define SHELL_WHITE_COLOR	0xD7

typedef struct entity_s
{
	struct model_s	*model;			// opaque type outside refresh
	float			angles[3];

	/*
	** most recent data
	*/
	float			origin[3];		// also used as RF_BEAM's "from"
	int				frame;			// also used as RF_BEAM's diameter
	vec3_t			axis[3];		// jit - from qfusion

	/*
	** previous data for lerping
	*/
	float			oldorigin[3];	// also used as RF_BEAM's "to"
	int				oldframe;

	/*
	** misc
	*/
	float			backlerp;				// 0.0 = current, 1.0 = old
	int				skinnum;				// also used as RF_BEAM's palette index

	int				lightstyle;				// for flashing entities
	float			alpha;					// ignore if RF_TRANSLUCENT isn't set

	//struct	image_s	*skin;			// NULL for inline skin
	struct image_s	*skins[MAX_MESHSKINS]; // jitskm
	struct model_s	*weapon_model; // jitskm
	int				flags;

	// jit: for smoke:
	float			startscale;
	float			scale;
	float			scalevel;
	float			alphavel;
	vec3_t			vel;
	vec3_t			accel;
	vec3_t			startorigin;
} entity_t;

#define ENTITY_FLAGS  68

typedef struct
{
	vec3_t	origin;
	vec3_t	color;
	float	intensity;
} dlight_t;

typedef struct
{
	vec3_t	origin;
	int		color;
	float	alpha;
} particle_t;

typedef struct
{
	float		rgb[3];			// 0.0 - 2.0
	float		white;			// highest of rgb
} lightstyle_t;

typedef struct {
	vec3_t			origin;
	float			size;
	float			color[3];
} stain_t;

typedef struct
{
	int			x, y, width, height;// in virtual screen coordinates
	float		fov_x, fov_y;
	float		vieworg[3];
	float		viewangles[3];
	float		blend[4];			// rgba 0-1 full screen blend
	float		time;				// time is uesed to auto animate
	int			rdflags;			// RDF_UNDERWATER, etc

	byte		*areabits;			// if not NULL, only areas with set bits will be drawn

	lightstyle_t	*lightstyles;	// [MAX_LIGHTSTYLES]

	int			num_entities;
	entity_t	*entities;

	int			num_dlights;
	dlight_t	*dlights;

	int			num_particles;
	particle_t	*particles;

	int			num_newstains;
	stain_t		*newstains;
} refdef_t;


/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky,
	it_sharppic, // jitrscript
	it_reflection // jitwater
} imagetype_t;

typedef struct image_s
{
	char	name[MAX_QPATH];			// game path, including extension
	//char	bare_name[MAX_QPATH];		// filename only, as called when searching
	imagetype_t	type;
	int		width, height;				// source image
	int		upload_width, upload_height;	// after power of two and picmip
	int		registration_sequence;		// 0 = free
	struct msurface_s	*texturechain;	// for sort-by-texture world drawing
	int		texnum;						// gl texture binding
	float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
	qboolean	scrap;
	qboolean	has_alpha;

	qboolean paletted;

	qboolean is_cin;					// Heffo - To identify a cin texture's image_t

	struct rscript_s	*rscript;		// jitrscript
	qboolean is_crosshair;				// viciouz - crosshair scale
} image_t;

#define BORDERED_PIC_COORD_COUNT 9

typedef struct bordered_pic_data_s
{
	float	screencoords[BORDERED_PIC_COORD_COUNT][4];
	float	texcoords[BORDERED_PIC_COORD_COUNT][4];
	image_t	*image;
} bordered_pic_data_t;


#define	API_VERSION		4

//
// these are the functions exported by the refresh module
//
typedef struct
{
	// if api_version is different, the dll cannot be used
	int		api_version;

	// called when the library is loaded
	qboolean	(*Init) ( void *hinstance, void *wndproc );

	// called before the library is unloaded
	void	(*Shutdown) (void);

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// EndRegistration will free any remaining data that wasn't registered.
	// Any model_s or skin_s pointers from before the BeginRegistration
	// are no longer valid after EndRegistration.
	//
	// Skins and images need to be differentiated, because skins
	// are flood filled to eliminate mip map edge errors, and pics have
	// an implicit "pics/" prepended to the name. (a pic name that starts with a
	// slash will not use the "pics/" prefix or the ".pcx" postfix)
	void	(*BeginRegistration) (const char *map);
	struct model_s *(*RegisterModel) (const char *name);
	//struct image_s *(*RegisterSkin) (char *name);
	void	(*RegisterSkin) (const char *name, struct model_s *model, struct image_s **skins); // jitskm
	struct image_s *(*RegisterPic) (const char *name);
	void	(*SetSky) (const char *name, float rotate, vec3_t axis);
	void	(*EndRegistration) (void);

	void	(*RenderFrame) (refdef_t *fd);

	void	(*DrawGetPicSize) (int *w, int *h, const char *name);	// will return 0 0 if not found
	void	(*DrawPic) (float x, float y, const char *name);
	void	(*DrawPic2) (float x, float y, image_t *gl);
	void	(*DrawStretchPic) (float x, float y, float w, float h, const char *name);
	void	(*DrawStretchPic2) (float x, float y, float w, float h, image_t *gl);
	void	(*DrawChar) (float x, float y, int c);
	void	(*DrawTileClear) (int x, int y, int w, int h, const char *name);
	void	(*DrawTileClear2) (int x, int y, int w, int h, image_t *image);
	void	(*DrawFill) (int x, int y, int w, int h, int c);
	void	(*DrawFadeScreen) (void);

	void	(*DrawString) (float x, float y, const char *str); // jit, shush little warning
	void	(*DrawStringAlpha) (float x, float y, const char *str, float alpha); // jit -- transparent strings (for fading out)
	int		(*DrawGetStates) (void);

	image_t	*(*DrawFindPic) (const char *name);

	// Draw images for cinematic rendering (which can have a different palette). Note that calls
	void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, byte *data);

	void	(*DrawResizeWindow) (int width, int height);

	/*
	** video mode and refresh state management entry points
	*/
	void	(*CinematicSetPalette)(const unsigned char *palette);	// NULL = game palette
	void	(*BeginFrame)(float camera_separation );
	void	(*EndFrame) (void);

	void	(*AppActivate)(qboolean activate);

	int		(*DrawGetIntVarByID)(int id);

	// Like DrawPic2, but lets you specify the texture coordinates
	void	(*DrawSubPic) (float x, float y, float w, float h, float tx1, float ty1, float tx2, float ty2, image_t *image);
	void	(*DrawBorderedPic) (bordered_pic_data_t *data, float x, float y, float w, float h, float scale, float alpha);

	// jitdebugdraw / jitbotlib - for debugging
	int		(*DrawDebugLine) (const vec_t *start, const vec_t *end, float r, float g, float b, float time, int id); // use -1 for the ID to create a new line.
	int		(*DrawDebugSphere) (const vec_t *pos, float radius, float r, float g, float b, float time, int id);

} refexport_t;

//
// these are the functions imported by the refresh module
//
typedef struct
{
	void	(*Sys_Error) (int err_level, char *str, ...);

	void	(*Cmd_AddCommand) (char *name, void(*cmd)(void));
	void	(*Cmd_RemoveCommand) (char *name);
	int		(*Cmd_Argc) (void);
	char	*(*Cmd_Argv) (int i);
	void	(*Cmd_ExecuteText) (int exec_when, char *text);

	void	(*Con_Printf) (int print_level, char *str, ...);

	// files will be memory mapped read only
	// the returned buffer may be part of a larger pak file,
	// or a discrete file from anywhere in the quake search path
	// a -1 return means the file does not exist
	// NULL can be passed for buf to just determine existance
	int		(*FS_LoadFile) (const char *name, void **buf);
	int		(*FS_LoadFileZ) (const char *name, void **buf); // jit
	void	(*FS_FreeFile) (void *buf);
	char	**(*FS_ListFiles) (const char *findname, int *numfiles, unsigned musthave, unsigned canthave, qboolean sort); // jitrscript
	void	(*FS_FreeFileList) (char **list, int n); // jit
	char	*(*FS_NextPath) (char*); // jitrscript

	// gamedir will be the current directory that generated
	// files should be stored to, ie: "f:\quake\id1"
	char	*(*FS_Gamedir) (void);

	cvar_t	*(*Cvar_Get) (const char *name, const char *value, int flags);
	cvar_t	*(*Cvar_Set) (const char *name, const char *value);
	void	 (*Cvar_SetValue) (const char *name, float value);

	testexport_t *e;
//jitmenu	void		(*Vid_MenuInit)( void );
	void		(*Vid_NewWindow) (int width, int height);
	void		*(*Z_Malloc) (size_t size); // jitmalloc
	void		(*Z_Free) (void *ptr); // jitmalloc
	qboolean	(*M_MenuActive) (void); // jitmenu, jitlinux
	void		(*M_MouseMove) (int, int); // jitmenu, jitlinux
	int			(*GetIntVarByID) (int); // jit
} refimport_t;

typedef struct
{
	void	(*Com_Printf) (char *str, ...);
	void	(*Cbuf_ExecuteText) (int exec_when, char *text);
	cvar_t	*(*Cvar_Get) (const char *name, const char *value, int flags);
	cvar_t	*(*Cvar_Set) (const char *name, const char *value);
	int		(*FS_LoadFileZ) (const char *path, void **buffer);
	void	(*FS_FreeFile) (void *buffer);
	int		(*GetIntVarByID) (int id);
} testimport_t;

// this is the only function actually exported at the linker level
typedef	refexport_t	(*GetRefAPI_t) (refimport_t);

#endif // __REF_H
