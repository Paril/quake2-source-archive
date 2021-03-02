// gl_refl.c
// by Matt Ownby - updates by jitspoe

// adds reflective water to the Quake2 engine

#include "gl_local.h"
#include "gl_refl.h"


// width and height of the texture we are gonna use to capture our reflection
unsigned int REFL_TEXW;
unsigned int REFL_TEXH;		

unsigned int g_reflTexW;		// dynamic size of reflective texture
unsigned int g_reflTexH;

int		g_num_refl		= 0;	// how many reflections we need to generate
int		g_active_refl	= 0;	// which reflection is being rendered at the moment

float	*g_refl_X;
float	*g_refl_Y;
float	*g_refl_Z;				// the Z (vertical) value of each reflection
float	*g_waterDistance;		// the rough distance from player to water .. we want to render the closest water surface.
image_t	**g_refl_images;
int		maxReflections;			// maximum number of reflections
unsigned int g_water_fragment_program_id; // jitwater
unsigned int g_water_vertex_program_id; // jitwater
image_t *distort_tex = NULL; // jitwater
image_t *water_normal_tex = NULL; // jitwater

// whether we are actively rendering a reflection of the world
// (instead of the world itself)
qboolean g_drawing_refl = false;
qboolean g_refl_enabled = true;	// whether reflections should be drawn at all
//#define USE_FBO
#ifdef USE_FBO
GLuint g_refl_framebuffer = 0; // jitwater
GLuint g_refl_renderbuffer = 0; // jitwater
#endif

float	g_last_known_fov = 90.0f;	// jit - default to 90.

void MYgluPerspective (GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar); // jit
void GL_MipMap (byte *in, int width, int height); // jit
void R_Clear (void); // jitwater


/*
================
R_init_refl

sets everything up 
================
*/

image_t *Draw_FindPic(const char *name);

void R_init_refl (int maxNoReflections)
{
	//===========================
	int				power;
	int				maxSize;
	unsigned char	*buf = NULL;
	int				i = 0;
	int len; // jitwater
	char *fragment_program_text; // jitwater
#ifdef _DEBUG
	int err;
#endif
	//===========================

	if (maxNoReflections < 1) // jit
		maxNoReflections = 1;

	R_setupArrays(maxNoReflections);	// setup number of reflections
	assert((err = qgl.GetError()) == GL_NO_ERROR);

	//okay we want to set REFL_TEXH etc to be less than the resolution 
	//otherwise white boarders are left .. we dont want that.
	//if waves function is turned off we can set reflection size to resolution.
	//however if it is turned on in game white marks round the sides will be left again
	//so maybe its best to leave this alone.

	for (power = 2; power < vid.height; power*=2)
	{	
		REFL_TEXW = power;	
		REFL_TEXH = power;  
	}

	qgl.GetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);		//get max supported texture size

	if (REFL_TEXW > maxSize)
	{
		for(power = 2; power < maxSize; power*=2)
		{	
			REFL_TEXW = power;	
			REFL_TEXH = power;  
		}
	}

	g_reflTexW = REFL_TEXW;
	g_reflTexH = REFL_TEXH;
	
	// if screen dimensions are smaller than texture size, we have to use screen dimensions instead (doh!)
	g_reflTexW = (vid.width < REFL_TEXW) ? vid.width : REFL_TEXW;	//keeping these in for now ..
	g_reflTexH = (vid.height < REFL_TEXH) ? vid.height : REFL_TEXH;

	for (i = 0; i < maxReflections; i++)
	{
		g_refl_images[i] = GL_CreateBlankImage("_reflection", g_reflTexW, g_reflTexH, it_reflection);

		if (gl_debug->value)
			ri.Con_Printf(PRINT_ALL, "Reflection texture %d texnum = %d.\n", i, g_refl_images[i]->texnum);
	}

	if (gl_debug->value)
	{
		ri.Con_Printf(PRINT_ALL, "Initialising reflective textures.\n");
		ri.Con_Printf(PRINT_ALL, "...reflective texture size set at %d.\n",g_reflTexH);
		ri.Con_Printf(PRINT_ALL, "...maximum reflective textures %d.\n", maxReflections);
	}

	// === jitwater - fragment program initializiation
	if (gl_state.fragment_program)
	{
		int err = 0;

		// Fragment program
		qgl.GenProgramsARB(1, &g_water_fragment_program_id);
		qgl.BindProgramARB(GL_FRAGMENT_PROGRAM_ARB, g_water_fragment_program_id);
		//qgl.ProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, 1.0f, 0.1f, 0.6f, 0.5f); // jitest
		len = ri.FS_LoadFileZ("scripts/water1.arbf", (void *)&fragment_program_text);

		if (len > 0)
		{
			qgl.ProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, len, fragment_program_text);
			ri.FS_FreeFile(fragment_program_text);
		}
		else
		{
			ri.Con_Printf(PRINT_ALL, "Unable to find scripts/water1.arbf.\n");
		}
		
		// Make sure the program loaded correctly
		err = qgl.GetError();
		if (err != GL_NO_ERROR)
		{
			ri.Con_Printf(PRINT_ALL, "OpenGL error with ARB fragment program: 0x%x\n", err);
			assert(err == GL_NO_ERROR);
		}

		// Vertex program
		qgl.GenProgramsARB(1, &g_water_vertex_program_id);
		qgl.BindProgramARB(GL_VERTEX_PROGRAM_ARB, g_water_vertex_program_id);
		len = ri.FS_LoadFileZ("scripts/water1.arbv", (void *)&fragment_program_text);

		if (len > 0)
		{
			qgl.ProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, len, fragment_program_text);
			ri.FS_FreeFile(fragment_program_text);
		}
		else
		{
			ri.Con_Printf(PRINT_ALL, "Unable to find scripts/water1.arbv.\n");
		}

		// Make sure the program loaded correctly
		err = qgl.GetError();
		if (err != GL_NO_ERROR)
		{
			ri.Con_Printf(PRINT_ALL, "OpenGL error with ARB fragment program: 0x%x\n", err);
			assert(err == GL_NO_ERROR);
		}

		// Textures
		distort_tex = Draw_FindPic("/textures/sfx/water/distort1.tga");
		water_normal_tex = Draw_FindPic("/textures/sfx/water/normal1.tga");
	}
#ifdef USE_FBO
	// FBO initialization
	qgl.GenFramebuffersEXT(1, &g_refl_framebuffer);
	assert((err = qgl.GetError()) == GL_NO_ERROR);
	qgl.BindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_refl_framebuffer);
	assert((err = qgl.GetError()) == GL_NO_ERROR);

	qgl.GenRenderbuffersEXT(1, &g_refl_renderbuffer);
	qgl.BindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_refl_renderbuffer);
	qgl.RenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, g_reflTexW, g_reflTexH);
	qgl.FramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, g_refl_renderbuffer);
	assert((err = qgl.GetError()) == GL_NO_ERROR);

	if (gl_debug->value)
	{
		int wtf = g_refl_images[0]->texnum;
		ri.Con_Printf(PRINT_ALL, "Reflective texture bound = %d\n", wtf);
	}

	GL_Bind(g_refl_images[0]->texnum);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_refl_images[0]->texnum, 0);
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);

	// Unbind framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif // USE_FBO
	// jitwater ===
}

void R_shutdown_refl (void) // jitodo - call this.
{
	if (gl_state.fragment_program)
	{
		qgl.DeleteProgramsARB(1, &g_water_fragment_program_id);
		qgl.DeleteProgramsARB(1, &g_water_vertex_program_id);
	}
}

/*
================
R_setupArrays

creates the actual arrays
to hold the reflections in
================
*/
void R_setupArrays (int maxNoReflections)
{
	R_clear_refl();

	if (g_refl_X)
		free(g_refl_X);

	if (g_refl_Y)
		free(g_refl_Y);

	if (g_refl_Z)
		free(g_refl_Z);

	if (g_refl_images)
		free(g_refl_images); // TODO: Make sure there isn't a memory leak here.  Do we need to free the images, too?

	if (g_waterDistance)
		free(g_waterDistance);

	g_refl_X		= (float *)	malloc ( sizeof(float) * maxNoReflections );
	g_refl_Y		= (float *)	malloc ( sizeof(float) * maxNoReflections );
	g_refl_Z		= (float *)	malloc ( sizeof(float) * maxNoReflections );
	g_waterDistance	= (float *)	malloc ( sizeof(float) * maxNoReflections );
	g_refl_images	= (image_t **)malloc(sizeof(image_t *) * maxNoReflections);
	
	memset(g_refl_X			, 0, sizeof(float));
	memset(g_refl_Y			, 0, sizeof(float));
	memset(g_refl_Z			, 0, sizeof(float));
	memset(g_waterDistance	, 0, sizeof(float));

	maxReflections = maxNoReflections;
}


/*
================
R_clear_refl

clears the relfection array
================
*/
void R_clear_refl (void)
{
	g_num_refl = 0;
}


/*
================
R_add_refl

creates an array of reflections
================
*/
void R_add_refl (float x, float y, float z)
{
	float distance;
	int i = 0;
	vec3_t v, v2;

	if (!maxReflections)
		return;		//safety check.

	if (r_reflectivewater_max->value != maxReflections)
		R_init_refl(r_reflectivewater_max->value);

	// make sure this isn't a duplicate entry
	// (I expect a lot of duplicates, which is why I put this check first)
	for (; i < g_num_refl; i++)
	{
		// if this is a duplicate entry then we don't want to add anything
		if (fabs(g_refl_Z[i] - z) < 8.0f)
			return;
	}

	VectorSet(v, x, y, z);
	VectorSubtract(v, r_newrefdef.vieworg, v2);
	distance = VectorLength(v2);

	// make sure we have room to add
	if (g_num_refl < maxReflections)
	{
		g_refl_X[g_num_refl]			= x;
		g_refl_Y[g_num_refl]			= y;
		g_refl_Z[g_num_refl]			= z;
		g_waterDistance[g_num_refl]		= distance;
		g_num_refl++;
	}
	else
	{
		// we want to use the closest surface
		// not just any random surface
		// good for when 1 reflection enabled.
		for (i = 0; i < g_num_refl; i++)
		{
			if (distance < g_waterDistance[i])
			{
				g_refl_X[i]			= x;
				g_refl_Y[i]			= y;
				g_refl_Z[i]			= z;
				g_waterDistance[i]	= distance;
				return;	//lets go
			}
		}
	}
}


/*
================
R_DrawDebugReflTexture

draws debug texture in game
so you can see whats going on
================
*/
void R_DrawDebugReflTexture (void)
{
	qgl.BindTexture(GL_TEXTURE_2D, g_refl_images[0]->texnum);	// do the first texture
	qgl.Begin(GL_QUADS);
	qgl.TexCoord2f(1, 1); qgl.Vertex3f(0, 0, 0);
	qgl.TexCoord2f(0, 1); qgl.Vertex3f(200, 0, 0);
	qgl.TexCoord2f(0, 0); qgl.Vertex3f(200, 200, 0);
	qgl.TexCoord2f(1, 0); qgl.Vertex3f(0, 200, 0);
	qgl.End();
}

/*
================
R_UpdateReflTex

this method renders the reflection
into the right texture (slow)
we have to draw everything a 2nd time
================
*/
void R_UpdateReflTex (refdef_t *fd)
{
	if (!g_num_refl)
		return;	// nothing to do here

	g_drawing_refl = true;	// begin drawing reflection
	g_last_known_fov = fd->fov_y;
	
	// go through each reflection and render it
	for (g_active_refl = 0; g_active_refl < g_num_refl; g_active_refl++)
	{
		//qgl.ClearColor(0, 0, 0, 1);								//clear screen
		//qgl.Clear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT); jitwater
#ifdef USE_FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_refl_framebuffer); // todo - array of framebuffers
#endif
		qgl.PushAttrib(GL_VIEWPORT_BIT);
		qgl.Viewport(0, 0, g_reflTexW, g_reflTexH);
		//qgl.BindTexture(GL_TEXTURE_2D, g_refl_images[g_active_refl]->texnum); // not necessary, but can't get the stupid texture rendered to
		R_RenderView(fd);	// draw the scene here!
#ifdef USE_FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#else
		qgl.BindTexture(GL_TEXTURE_2D, g_refl_images[g_active_refl]->texnum);
		qgl.CopyTexSubImage2D(GL_TEXTURE_2D, 0,
			0,//(REFL_TEXW - g_reflTexW) >> 1,
			0,//(REFL_TEXH - g_reflTexH) >> 1,
			0, 0, g_reflTexW, g_reflTexH);
		//qgl.Viewport(0, 0, vid.width, vid.height);
#endif // !USE_FBO
		qgl.PopAttrib();

		R_Clear(); // jitwater
	}

	g_drawing_refl = false;	// done drawing refl
	// jitwater qgl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//clear stuff now cause we want to render scene
}															


// sets modelview to reflection instead of normal view
void R_DoReflTransform()
{
	//qgl.Rotatef (180, 1, 0, 0);	// flip upside down (X-axis is forward)
    qgl.Rotatef(r_newrefdef.viewangles[2],  1, 0, 0);
    qgl.Rotatef(r_newrefdef.viewangles[0],  0, 1, 0);	// up/down rotation (reversed)
    qgl.Rotatef(-r_newrefdef.viewangles[1], 0, 0, 1);	// left/right rotation
    qgl.Translatef(-r_newrefdef.vieworg[0],
    	-r_newrefdef.vieworg[1],
    	-((2*g_refl_Z[g_active_refl]) - r_newrefdef.vieworg[2]));
}

///////////

void print_matrix(int which_matrix, const char *desc)
{
	GLfloat m[16];	// receives our matrix
	qgl.GetFloatv(which_matrix, m);	// snag the matrix
	
	printf("[%s]\n", desc);
	printf("%0.3f %0.3f %0.3f %0.3f\n", m[0], m[4], m[8],  m[12]);
	printf("%0.3f %0.3f %0.3f %0.3f\n", m[1], m[5], m[9],  m[13]);
	printf("%0.3f %0.3f %0.3f %0.3f\n", m[2], m[6], m[10], m[14]);
	printf("%0.3f %0.3f %0.3f %0.3f\n", m[3], m[7], m[11], m[15]);
}


// alters texture matrix to handle our reflection
void R_LoadReflMatrix (void)
{
	float aspect = (float)r_newrefdef.width/r_newrefdef.height;

	qgl.MatrixMode(GL_TEXTURE);
	qgl.LoadIdentity();

	qgl.Translatef(0.5f, 0.5f, 0.0f); // Center texture

	qgl.Scalef(0.5f * (float)g_reflTexW / REFL_TEXW,
			  0.5f * (float)g_reflTexH / REFL_TEXH,
			  1.0f);								/* Scale and bias */

	MYgluPerspective(g_last_known_fov, aspect, 4, 4096);

	qgl.Rotatef(-90.0f, 1.0f, 0.0f, 0.0f);	    // put Z going up
	qgl.Rotatef(90.0f,  0.0f, 0.0f, 1.0f);	    // put Z going up

	// do transform
	R_DoReflTransform();
	qgl.Translatef(0.0f, 0.0f, 0.0f);
	qgl.MatrixMode(GL_MODELVIEW);
}

void R_GetReflTexScale (float *w, float *h)
{
	*w = 0.5f * (float)g_reflTexW / (float)REFL_TEXW;
	*h = 0.5f * (float)g_reflTexH / (float)REFL_TEXH;
}

/*
 * Load identity into texture matrix
 */
void R_ClearReflMatrix()
{
	qgl.MatrixMode(GL_TEXTURE);
	qgl.LoadIdentity();
	qgl.MatrixMode(GL_MODELVIEW);
}

// the frustum function from the Mesa3D Library
// Apparently the regular glFrustum function can be broken in certain instances?
void mesa_frustum(GLdouble left, GLdouble right,
        GLdouble bottom, GLdouble top, 
        GLdouble nearval, GLdouble farval)
{
   GLdouble x, y, a, b, c, d;
   GLdouble m[16];

   x = (2.0 * nearval) / (right - left);
   y = (2.0 * nearval) / (top - bottom);
   a = (right + left) / (right - left);
   b = (top + bottom) / (top - bottom);
   c = -(farval + nearval) / ( farval - nearval);
   d = -(2.0 * farval * nearval) / (farval - nearval);

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

   qgl.MultMatrixd(m);
}








