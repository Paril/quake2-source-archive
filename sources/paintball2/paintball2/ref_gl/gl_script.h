#ifndef __GL_SCRIPT__
#define __GL_SCRIPT__

// Animation loop
typedef struct anim_stage_s {
	image_t					*texture;	// texture
	char					name[48];	// texture name
	struct anim_stage_s		*next;		// next anim stage
} anim_stage_t;

// Blending
typedef struct {
	int			source;		// source blend value
	int			dest;		// dest blend value
	qboolean	blend;		// are we going to blend?
} blendfunc_t;

// Alpha shifting
typedef struct {
	float		min, max;	// min/max alpha values
	float		speed;		// shifting speed
} alphashift_t;

// scaling
typedef struct
{
	char	typeX, typeY;	// scale types
	float	scaleX, scaleY;	// scaling factors
} rs_scale_t;

// offset
typedef struct
{
	float	offsetX, offsetY;	// offset factors
} rs_offset_t; // jitrscript

// scrolling
typedef struct
{
	char	typeX, typeY;	// scroll types
	float	speedX, speedY;	// speed of scroll
} rs_scroll_t;

typedef enum
{
	TC_GEN_BASE			= 0,
	TC_GEN_LIGHTMAP		= 1,
	TC_GEN_ENVIRONMENT	= 2,
	TC_GEN_VECTOR		= 3
} rs_tcGen_t;

// Script stage
typedef struct rs_stage_s {
	image_t					*texture;		// texture
	char					name[48];		// tex name
	
	anim_stage_t			*anim_stage;	// first animation stage
	float					anim_delay;		// Delay between anim frames
	float					last_anim_time; // gametime of last frame change
	char					anim_count;		// number of animation frames
	anim_stage_t			*last_anim;		// pointer to last anim

	blendfunc_t				blendfunc;		// image blending
	alphashift_t			alphashift;		// alpha shifting
	rs_scroll_t				scroll;			// tcmod
	rs_scale_t				scale;			// tcmod
	rs_scale_t				scaleadd;		// jitrscript
	rs_offset_t				offset;			// jitrscript
	rs_tcGen_t				tcGen;			// jitrscript
	vec4_t					tcGenVec[2];	// jitrscript
	qboolean				sharp;			// jitrscript

	float					rot_speed;		// rotate speed (0 for no rotate);

	//qboolean				envmap;			// fake envmapping
	qboolean				lightmap;		// lightmap this stage?
	qboolean				alphamask;		// alpha masking?

	struct rs_stage_s		*next;			// next stage
} rs_stage_t;

typedef struct rs_stagekey_s
{
	char *stage;
	void (*func)(rs_stage_t *shader, char **token);
} rs_stagekey_t;

// Base script
typedef struct rscript_s {
	char					name[64];	// name of script
	unsigned char			subdivide;	// Heffo - chop the surface up this much for vertex warping
	float					warpdist;	// Heffo - vertex warping distance;
	float					warpsmooth;	// Heffo - vertex warping smoothness;
	float					warpspeed;	// Heffo - vertex warping speed;
	qboolean				mirror;		// mirror
	int						width;		// jitrscript
	int						height;		// jitrscript
	qboolean				dontflush;	// dont flush from memory on map change
	qboolean				ready;		// readied by the engine?
	rs_stage_t				*stage;		// first rendering stage
	struct rscript_s		*next;		// next script in linked list
	image_t					*img_ptr;	// jitrscript -- pointer back to the image using the script
} rscript_t;

typedef struct rs_scriptkey_s
{
	char *script;
	void (*func)(rscript_t *rs, char **token);
} rs_scriptkey_t;

void RS_LoadScript(char *script);
void RS_FreeAllScripts(void);
void RS_FreeScript(rscript_t *rs);
void RS_FreeUnmarked(void);
rscript_t *RS_FindScript(const char *name);
void RS_ReadyScript(rscript_t *rs);
void RS_ScanPathForScripts(char *dir);
int RS_Animate(rs_stage_t *stage);
void RS_UpdateRegistration(void);
void RS_DrawSurface(struct msurface_s *surf, qboolean lightmap, rscript_t *rs); // jitrscript
//void RS_SetTexcoords(rs_stage_t *stage, float *os, float *ot, msurface_t *fa);

#define RS_DrawPoly(surf)	RS_DrawSurface((surf), true, NULL) // jitrscript
#define RS_DrawPolyNoLightMap(surf)	RS_DrawSurface((surf), false, NULL) // jitrscript

extern float rs_realtime;

#endif // __GL_SCRIPT__

