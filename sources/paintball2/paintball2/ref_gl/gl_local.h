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

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <GL/gl.h>
#include "glext.h"
#include <math.h>

#ifndef GL_COLOR_INDEX8_EXT
#define GL_COLOR_INDEX8_EXT GL_COLOR_INDEX
#endif

#ifdef _WIN32
#define GL_DRIVER_LIB "opengl32"
#else
#define GL_DRIVER_LIB "libGL.so"
#define GL_DRIVER_LIB2 "libGL.so.1.2"
#define GL_DRIVER_LIB3 "libGL.so.1"
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include "../client/ref.h"

#include "qgl.h"

#define	REF_VERSION	"PB2GL 0.40" // jitversion / jitbuild

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


#ifndef __VIDDEF_T
#define __VIDDEF_T
typedef struct
{
	int		width, height;			// coordinates from main game, jit -- got rid of unsigned warning
} viddef_t;
#endif

extern	viddef_t	vid;


#define		MAX_LIGHTMAPS	128 // jitgentex
#define		MAX_GLTEXTURES	1024


//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_resolution,

	rserr_unknown
} rserr_t;

#include "gl_script.h"

#include "gl_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);
void GL_SetDefaultState (void);
void GL_UpdateSwapInterval (void);

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;


extern	image_t		*r_notexture;
extern	image_t		*r_whitetexture; // jitfog
extern	image_t		*r_particletexture;
extern	image_t		*r_startexture; // jittemp
extern	image_t		*r_caustictexture; // jitcaustics
extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


extern	int			gl_filter_min, gl_filter_max;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern cvar_t	*gl_vertex_arrays;

extern cvar_t	*gl_ext_multitexture;
extern cvar_t	*gl_ext_pointparameters;
extern cvar_t	*gl_ext_compiled_vertex_array;
extern cvar_t	*gl_ext_texture_compression; // Heffo - ARB Texture Compression

extern cvar_t	*gl_screenshot_jpeg;			// Heffo - JPEG Screenshots
extern cvar_t	*gl_screenshot_jpeg_quality;	// Heffo - JPEG Screenshots
extern cvar_t	*vid_gamma_hw;

extern cvar_t	*gl_particle_min_size;
extern cvar_t	*gl_particle_max_size;
extern cvar_t	*gl_particle_size;
extern cvar_t	*gl_particle_att_a;
extern cvar_t	*gl_particle_att_b;
extern cvar_t	*gl_particle_att_c;

extern	cvar_t	*gl_nosubimage;
extern	cvar_t	*gl_bitdepth;
extern	cvar_t	*gl_log;
extern	cvar_t	*gl_lightmap;
extern	cvar_t	*gl_shadows;
extern	cvar_t	*gl_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_brightness; // jit
extern	cvar_t	*gl_autobrightness; // jit
extern	cvar_t	*gl_showbbox;  // jit / guy
extern	cvar_t	*gl_round_down;
extern	cvar_t	*gl_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_skyedge; // jitsky
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_colorpolycount; // jitdebug
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl_ztrick;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_lightmaptype;
//extern	cvar_t	*gl_modulate;
extern	cvar_t	*gl_lightmapgamma;
extern	cvar_t	*r_oldlightpoint; // jit
extern	cvar_t	*r_hardware_light; // jit
extern	cvar_t	*gl_drawbuffer;
extern	cvar_t	*gl_3dlabs_broken;
extern  cvar_t  *gl_driver;
extern	cvar_t	*gl_swapinterval;
extern	cvar_t	*gl_texturemode;
extern	cvar_t	*gl_texturealphamode;
extern	cvar_t	*gl_texturesolidmode;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *gl_lockpvs;

extern	cvar_t	*vid_resolution;
extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;
extern	cvar_t	*vid_lighten; // jitgamma
extern	cvar_t	*gl_debug; // jit
extern	cvar_t	*gl_arb_fragment_program; // jit
extern	cvar_t	*r_caustics; // jitcaustics
extern	cvar_t	*r_reflectivewater; // jitwater
extern	cvar_t	*r_reflectivewater_debug; // jitwater
extern	cvar_t	*r_reflectivewater_max; // jitwater
extern	cvar_t	*r_oldmodels; // jit


extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;
extern	int		gl_tex_solid_format;
extern	int		gl_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

void R_TranslatePlayerSkin (int playernum);
void GL_Bind (int texnum);
void GL_MBind( GLenum target, int texnum );
void GL_TexEnv( GLenum value );
void GL_EnableMultitexture( qboolean enable );
void GL_SelectTexture( GLenum );

void R_LightPoint (vec3_t p, vec3_t color);
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


void V_AddBlend (float r, float g, float b, float a, float *v_blend);

qboolean 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t *e);
void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_DrawCaustics (void); // jitcaustics
void R_RenderBrushPoly (msurface_t *fa);
void R_InitNoTexture (void); // jit - renamed
void Draw_InitLocal (void);
void GL_SubdivideSurface (msurface_t *fa);
void GL_SubdivideLightmappedSurface (msurface_t *fa, float subdivide_size); //Heffo surface subdivision
//qboolean R_CullBox (vec3_t mins, vec3_t maxs);
qboolean R_CullBox (const vec3_t mins, const vec3_t maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);
void R_Init3dfxGamma (void); // jit3dfx

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
float CalcWave (float x, float y); // jitwater
void EmitWaterPolys (msurface_t *fa);
void R_AddSkySurface (msurface_t *fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

void R_DrawDebug (void); // jitdebugdraw

#if 0
short LittleShort (short l);
short BigShort (short l);
int	LittleLong (int l);
float LittleFloat (float f);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer
#endif

void COM_StripExtension (const char *in, char *out, int out_size); // jitsecurity

void	Draw_GetPicSize (int *w, int *h, const char *name);
void	Draw_Pic (float x, float y, const char *name);
void	Draw_StretchPic (float x, float y, float w, float h, const char *name);
void	Draw_Char (float x, float y, int c);
void	Draw_TileClear (int x, int y, int w, int h, const char *name);
void	Draw_Fill (int x, int y, int w, int h, int c);
void	Draw_FadeScreen (void);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame (float camera_separation);
void	R_SwapBuffers (int);
void	R_SetPalette (const unsigned char *palette);

int		Draw_GetPalette (void);

void	GL_ResampleTexture (const unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

//struct image_s *R_RegisterSkin(const char *name);
void	R_RegisterSkin (const char *name, struct model_s *model, struct image_s **skins);

void	LoadPCX (const char *filename, byte **pic, byte **palette, int *width, int *height);
image_t *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*GL_FindImage (const char *name, imagetype_t type);
image_t *GL_CreateBlankImage (const char *name, int width, int height, imagetype_t type);
void	GL_TextureMode (const char *string );
void	GL_ImageList_f (void);

void	GL_InitImages (void);
void	GL_ShutdownImages (void);

void	GL_FreeUnusedImages (void);

void	GL_TextureAlphaMode (const char *string);
void	GL_TextureSolidMode (const char *string);

/*
** GL extension emulation functions
*/
void GL_DrawParticles();

/*
** GL config stuff
*/
#define GL_RENDERER_VOODOO		0x00000001
#define GL_RENDERER_VOODOO2   	0x00000002
#define GL_RENDERER_VOODOO_RUSH	0x00000004
#define GL_RENDERER_BANSHEE		0x00000008
#define	GL_RENDERER_3DFX		0x0000000F

#define GL_RENDERER_PCX1		0x00000010
#define GL_RENDERER_PCX2		0x00000020
#define GL_RENDERER_PMX			0x00000040
#define	GL_RENDERER_POWERVR		0x00000070

#define GL_RENDERER_PERMEDIA2	0x00000100
#define GL_RENDERER_GLINT_MX	0x00000200
#define GL_RENDERER_GLINT_TX	0x00000400
#define GL_RENDERER_3DLABS_MISC	0x00000800
#define	GL_RENDERER_3DLABS		0x00000F00

#define GL_RENDERER_REALIZM		0x00001000
#define GL_RENDERER_REALIZM2	0x00002000
#define	GL_RENDERER_INTERGRAPH	0x00003000

#define GL_RENDERER_3DPRO		0x00004000
#define GL_RENDERER_REAL3D		0x00008000
#define GL_RENDERER_RIVA128		0x00010000
#define GL_RENDERER_DYPIC		0x00020000

#define GL_RENDERER_V1000		0x00040000
#define GL_RENDERER_V2100		0x00080000
#define GL_RENDERER_V2200		0x00100000
#define	GL_RENDERER_RENDITION	0x001C0000

#define GL_RENDERER_O2          0x00100000
#define GL_RENDERER_IMPACT      0x00200000
#define GL_RENDERER_RE			0x00400000
#define GL_RENDERER_IR			0x00800000
#define	GL_RENDERER_SGI			0x00F00000

#define GL_RENDERER_MCD			0x01000000
#define GL_RENDERER_ATI			0x02000000
#define GL_RENDERER_OTHER		0x80000000

typedef struct
{
	int         renderer;
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	float       version;
	const char *extensions_string;

	qboolean	allow_cds;
} glconfig_t;


#define GLSTATE_DISABLE_ALPHATEST	if (gl_state.alpha_test) { qgl.Disable(GL_ALPHA_TEST); gl_state.alpha_test=false; }
#define GLSTATE_ENABLE_ALPHATEST	if (!gl_state.alpha_test) { qgl.Enable(GL_ALPHA_TEST); gl_state.alpha_test=true; }

#define GLSTATE_DISABLE_BLEND		if (gl_state.blend) { qgl.Disable(GL_BLEND); gl_state.blend=false; }
#define GLSTATE_ENABLE_BLEND		if (!gl_state.blend) { qgl.Enable(GL_BLEND); gl_state.blend=true; }

#define GLSTATE_DISABLE_TEXGEN		if (gl_state.texgen) { qgl.Disable(GL_TEXTURE_GEN_S); qgl.Disable(GL_TEXTURE_GEN_T); qgl.Disable(GL_TEXTURE_GEN_R); gl_state.texgen=false; }
#define GLSTATE_ENABLE_TEXGEN		if (!gl_state.texgen) { qgl.Enable(GL_TEXTURE_GEN_S); qgl.Enable(GL_TEXTURE_GEN_T); qgl.Enable(GL_TEXTURE_GEN_R); gl_state.texgen=true; }

typedef struct
{
	qboolean fullscreen;

	int prev_width;
	int prev_height;

	unsigned char *d_16to8table;

	int lightmap_texnums[MAX_LIGHTMAPS]; // jitgentex

	int	currenttextures[32];
	int currenttmu;

	float camera_separation;
	qboolean stereo_enabled;

	// advanced state manager - MrG

	qboolean	alpha_test;
	qboolean	blend;
	qboolean	texgen;

	qboolean		reg_combiners;
	qboolean		texshaders;
	qboolean		sgis_mipmap;
	qboolean		gammaramp;

	long			tex_rectangle; // jitblur

	// End - MrG

	qboolean	texture_compression; // Heffo - ARB Texture Compression

	qboolean	texture_combine; // jitbright
	float		max_anisotropy; // jitanisotropy
	qboolean	fragment_program; // jitwater
	qboolean	fbo; // jitwater
	qboolean	sse_enabled; // jitsimd

	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];
} glstate_t;

extern glconfig_t  gl_config;
extern glstate_t   gl_state;

extern byte *char_colors; // jittext

// vertex arrays

#define MAX_ARRAY MAX_PARTICLES*4

#define VA_SetElem2(v,a,b)		((v)[0]=(a),(v)[1]=(b))
#define VA_SetElem3(v,a,b,c)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c))
#define VA_SetElem4(v,a,b,c,d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

extern float	tex_array[MAX_ARRAY][2];
extern float	vert_array[MAX_ARRAY][3];
extern float	col_array[MAX_ARRAY][4];

//#define SHADOW_VOLUMES

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_BeginFrame (float camera_separation);
void		GLimp_EndFrame (void);
int			GLimp_Init (void *hinstance, void *hWnd);
void		GLimp_Shutdown (void);
rserr_t		GLimp_SetMode (int *pwidth, int *pheight, int width, int height, qboolean fullscreen);
void		GLimp_AppActivate (qboolean active);
void		GLimp_EnableLogging (qboolean enable);
void		GLimp_LogNewFrame (void);


// #define BEEFQUAKERENDER // this must be off for 3dfx compatibilty (jit3dfx)

#if 0
#define malloc(a) ri.Z_Malloc(a) // jitmalloc
#define free(a) ri.Z_Free(a)
#else
#define Z_Malloc(a) malloc(a)
#define Z_Free(a) free(a)
#endif

#define RSCRIPT_STATIC 0 // jitrscript
#define RSCRIPT_SINE   1
#define RSCRIPT_COSINE 2
#define RSCRIPT_SINABS 3
#define RSCRIPT_COSABS 4
#define RSCRIPT_CONST  5 // jitrscript (for nonanimated rotations)

extern unsigned char lightmap_gammatable[256]; // jitgamma

