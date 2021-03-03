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
// r_bloom.c: 2D lighting post process effect

#include "r_local.h"

#ifdef LIGHT_BLOOMS

/* 
============================================================================== 
 
						LIGHT BLOOMS
 
============================================================================== 
*/ 

static float Diamond8x[8][8] = { 
		0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
		0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
		0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
		0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
		0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
		0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f };

static float Diamond6x[6][6] = { 
		0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 
		0.0f, 0.3f, 0.5f, 0.5f, 0.3f, 0.0f,  
		0.1f, 0.5f, 0.9f, 0.9f, 0.5f, 0.1f, 
		0.1f, 0.5f, 0.9f, 0.9f, 0.5f, 0.1f, 
		0.0f, 0.3f, 0.5f, 0.5f, 0.3f, 0.0f, 
		0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f };

static float Diamond4x[4][4] = {  
		0.3f, 0.4f, 0.4f, 0.3f,  
		0.4f, 0.9f, 0.9f, 0.4f, 
		0.4f, 0.9f, 0.9f, 0.4f, 
		0.3f, 0.4f, 0.4f, 0.3f };


static int		BLOOM_SIZE;

cvar_t		*r_bloom_alpha;
cvar_t		*r_bloom_diamond_size;
cvar_t		*r_bloom_intensity;
cvar_t		*r_bloom_threshold;
cvar_t		*r_bloom_darken;
cvar_t		*r_bloom_sample_size;
cvar_t		*r_bloom_fast_sample;

image_t		*r_bloomscreentexture;
image_t		*r_bloomeffecttexture;
image_t		*r_bloombackuptexture;
image_t		*r_bloomdownsamplingtexture;

static int		r_screendownsamplingtexture_size;
static int		screen_texture_width, screen_texture_height;
static int		r_screenbackuptexture_size;

//current refdef size:
static int	curView_x;
static int	curView_y;
static int	curView_width;
static int	curView_height;

//texture coordinates of screen data inside screentexture
static float screenText_tcw;
static float screenText_tch;

static int	sample_width;
static int	sample_height;

//texture coordinates of adjusted textures
static float sampleText_tcw;
static float sampleText_tch;

//this macro is in sample size workspace coordinates
#define R_Bloom_SamplePass( xpos, ypos )							\
	qglBegin(GL_QUADS);												\
	qglTexCoord2f(	0,						sampleText_tch);		\
	qglVertex2f(	xpos,					ypos);					\
	qglTexCoord2f(	0,						0);						\
	qglVertex2f(	xpos,					ypos+sample_height);	\
	qglTexCoord2f(	sampleText_tcw,			0);						\
	qglVertex2f(	xpos+sample_width,		ypos+sample_height);	\
	qglTexCoord2f(	sampleText_tcw,			sampleText_tch);		\
	qglVertex2f(	xpos+sample_width,		ypos);					\
	qglEnd();

#define R_Bloom_Quad( x, y, width, height, textwidth, textheight )	\
	qglBegin(GL_QUADS);												\
	qglTexCoord2f(	0,			textheight);						\
	qglVertex2f(	x,			y);									\
	qglTexCoord2f(	0,			0);									\
	qglVertex2f(	x,			y+height);							\
	qglTexCoord2f(	textwidth,	0);									\
	qglVertex2f(	x+width,	y+height);							\
	qglTexCoord2f(	textwidth,	textheight);						\
	qglVertex2f(	x+width,	y);									\
	qglEnd();



/*
=================
R_Bloom_InitBackUpTexture
=================
*/
void R_Bloom_InitBackUpTexture( int width, int height )
{
	byte	*data;
	
	data = malloc( width * height * 4 );
	memset( data, 0, width * height * 4 );

	r_screenbackuptexture_size = width;

	r_bloombackuptexture = R_LoadPic( "***r_bloombackuptexture***", (byte *)data, width, height, it_pic, 3 );
	
	free ( data );
}

/*
=================
R_Bloom_InitEffectTexture
=================
*/
void R_Bloom_InitEffectTexture( void )
{
	byte	*data;
	float	bloomsizecheck;
	
	if( (int)r_bloom_sample_size->value < 32 )
		Cvar_SetValue ("r_bloom_sample_size", 32);

	//make sure bloom size is a power of 2
	BLOOM_SIZE = (int)r_bloom_sample_size->value;
	bloomsizecheck = (float)BLOOM_SIZE;
	while(bloomsizecheck > 1.0f) bloomsizecheck /= 2.0f;
	if( bloomsizecheck != 1.0f )
	{
		BLOOM_SIZE = 32;
		while( BLOOM_SIZE < (int)r_bloom_sample_size->value )
			BLOOM_SIZE *= 2;
	}

	//make sure bloom size doesn't have stupid values
	if( BLOOM_SIZE > screen_texture_width ||
		BLOOM_SIZE > screen_texture_height )
		BLOOM_SIZE = min( screen_texture_width, screen_texture_height );

	if( BLOOM_SIZE != (int)r_bloom_sample_size->value )
		Cvar_SetValue ("r_bloom_sample_size", BLOOM_SIZE);

	data = malloc( BLOOM_SIZE * BLOOM_SIZE * 4 );
	memset( data, 0, BLOOM_SIZE * BLOOM_SIZE * 4 );

	r_bloomeffecttexture = R_LoadPic( "***r_bloomeffecttexture***", (byte *)data, BLOOM_SIZE, BLOOM_SIZE, it_pic, 3 );
	
	free ( data );
}

/*
=================
R_Bloom_InitTextures
=================
*/
void R_Bloom_InitTextures( void )
{
	byte	*data;
	int		size;

	//find closer power of 2 to screen size 
	for (screen_texture_width = 1; screen_texture_width < vid.width; screen_texture_width *= 2);
	for (screen_texture_height = 1; screen_texture_height < vid.height; screen_texture_height *= 2);

	//disable blooms if we can't handle a texture of that size
/*	if( screen_texture_width > gl_config.maxTextureSize ||
		screen_texture_height > glConfig.maxTextureSize ) {
		screen_texture_width = screen_texture_height = 0;
		Cvar_SetValue ("r_bloom", 0);
		Com_Printf( "WARNING: 'R_InitBloomScreenTexture' too high resolution for Light Bloom. Effect disabled\n" );
		return;
	}*/

	//init the screen texture
	size = screen_texture_width * screen_texture_height * 4;
	data = malloc( size );
	memset( data, 255, size );
	r_bloomscreentexture = R_LoadPic( "***r_bloomscreentexture***", (byte *)data, screen_texture_width, screen_texture_height, it_pic, 3 );
	free ( data );

	//validate bloom size and init the bloom effect texture
	R_Bloom_InitEffectTexture ();

	//if screensize is more than 2x the bloom effect texture, set up for stepped downsampling
	r_bloomdownsamplingtexture = NULL;
	r_screendownsamplingtexture_size = 0;
	if( vid.width > (BLOOM_SIZE * 2) && !r_bloom_fast_sample->value )
	{
		r_screendownsamplingtexture_size = (int)(BLOOM_SIZE * 2);
		data = malloc( r_screendownsamplingtexture_size * r_screendownsamplingtexture_size * 4 );
		memset( data, 0, r_screendownsamplingtexture_size * r_screendownsamplingtexture_size * 4 );
		r_bloomdownsamplingtexture = R_LoadPic( "***r_bloomdownsamplingtexture***", (byte *)data, r_screendownsamplingtexture_size, r_screendownsamplingtexture_size, it_pic, 3 );
		free ( data );
	}

	//Init the screen backup texture
	if( r_screendownsamplingtexture_size )
		R_Bloom_InitBackUpTexture( r_screendownsamplingtexture_size, r_screendownsamplingtexture_size );
	else
		R_Bloom_InitBackUpTexture( BLOOM_SIZE, BLOOM_SIZE );
	
}

/*
=================
R_InitBloomTextures
=================
*/
void R_InitBloomTextures( void )
{

	//r_bloom = Cvar_Get( "r_bloom", "0", CVAR_ARCHIVE );
	r_bloom_alpha = Cvar_Get( "r_bloom_alpha", "0.2", CVAR_ARCHIVE );
	r_bloom_diamond_size = Cvar_Get( "r_bloom_diamond_size", "8", CVAR_ARCHIVE );
	r_bloom_intensity = Cvar_Get( "r_bloom_intensity", "0.6", CVAR_ARCHIVE );
	r_bloom_threshold = Cvar_Get( "r_bloom_threshold", "0.08", CVAR_ARCHIVE );
	r_bloom_darken = Cvar_Get( "r_bloom_darken", "4", CVAR_ARCHIVE );
	r_bloom_sample_size = Cvar_Get( "r_bloom_sample_size", "128", CVAR_ARCHIVE );
	r_bloom_fast_sample = Cvar_Get( "r_bloom_fast_sample", "0", CVAR_ARCHIVE );

	BLOOM_SIZE = 0;
	if (!r_bloom->value)
		return;

	R_Bloom_InitTextures ();
}

 
/*
=================
R_Bloom_DrawEffect
=================
*/
void R_Bloom_DrawEffect( void )
{
	GL_Bind(r_bloomeffecttexture->texnum);  
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);
	qglColor4f(r_bloom_alpha->value, r_bloom_alpha->value, r_bloom_alpha->value, 1.0f);
	GL_TexEnv(GL_MODULATE);
	qglBegin(GL_QUADS);							
	qglTexCoord2f(	0,							sampleText_tch	);	
	qglVertex2f(	curView_x,					curView_y	);				
	qglTexCoord2f(	0,							0	);				
	qglVertex2f(	curView_x,					curView_y + curView_height	);	
	qglTexCoord2f(	sampleText_tcw,				0	);				
	qglVertex2f(	curView_x + curView_width,	curView_y + curView_height	);	
	qglTexCoord2f(	sampleText_tcw,				sampleText_tch	);	
	qglVertex2f(	curView_x + curView_width,	curView_y	);				
	qglEnd();
	
	GL_Disable(GL_BLEND);
}


#if 0
/*
=================
R_Bloom_GeneratexCross - alternative bluring method
=================
*/
void R_Bloom_GeneratexCross( void )
{
	int			i;
	static int		BLOOM_BLUR_RADIUS = 8;
	//static float	BLOOM_BLUR_INTENSITY = 2.5f;
	float	BLOOM_BLUR_INTENSITY;
	static float intensity;
	static float range;

	//set up sample size workspace
	qglViewport( 0, 0, sample_width, sample_height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, sample_width, sample_height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();

	//copy small scene into r_bloomeffecttexture
	GL_Bind(0, r_bloomeffecttexture);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	//start modifying the small scene corner
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	GL_Enable(GL_BLEND);

	//darkening passes
	if( r_bloom_darken->value )
	{
		GL_BlendFunc(GL_DST_COLOR, GL_ZERO);
		GL_TexEnv(GL_MODULATE);
		
		for(i=0; i<r_bloom_darken->value ;i++) {
			R_Bloom_SamplePass( 0, 0 );
		}
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}

	//bluring passes
	if( BLOOM_BLUR_RADIUS ) {
		
		GL_BlendFunc(GL_ONE, GL_ONE);

		range = (float)BLOOM_BLUR_RADIUS;

		BLOOM_BLUR_INTENSITY = r_bloom_intensity->value;
		//diagonal-cross draw 4 passes to add initial smooth
		qglColor4f( 0.5f, 0.5f, 0.5f, 1.0);
		R_Bloom_SamplePass( 1, 1 );
		R_Bloom_SamplePass( -1, 1 );
		R_Bloom_SamplePass( -1, -1 );
		R_Bloom_SamplePass( 1, -1 );
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

		for(i=-(BLOOM_BLUR_RADIUS+1);i<BLOOM_BLUR_RADIUS;i++) {
			intensity = BLOOM_BLUR_INTENSITY/(range*2+1)*(1 - fabs(i*i)/(float)(range*range));
			if( intensity < 0.05f ) continue;
			qglColor4f( intensity, intensity, intensity, 1.0f);
			R_Bloom_SamplePass( i, 0 );
			//R_Bloom_SamplePass( -i, 0 );
		}

		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

		//for(i=0;i<BLOOM_BLUR_RADIUS;i++) {
		for(i=-(BLOOM_BLUR_RADIUS+1);i<BLOOM_BLUR_RADIUS;i++) {
			intensity = BLOOM_BLUR_INTENSITY/(range*2+1)*(1 - fabs(i*i)/(float)(range*range));
			if( intensity < 0.05f ) continue;
			qglColor4f( intensity, intensity, intensity, 1.0f);
			R_Bloom_SamplePass( 0, i );
			//R_Bloom_SamplePass( 0, -i );
		}

		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}
	
	//restore full screen workspace
	qglViewport( 0, 0, glState.width, glState.height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, glState.width, glState.height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();
}
#endif


/*
=================
R_Bloom_GeneratexDiamonds
=================
*/
void R_Bloom_GeneratexDiamonds( void )
{
	int			i, j;
	static float intensity;

	//set up sample size workspace
	qglViewport( 0, 0, sample_width, sample_height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, sample_width, sample_height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();

	//copy small scene into r_bloomeffecttexture
	GL_Bind(r_bloomeffecttexture->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	//start modifying the small scene corner
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	GL_Enable(GL_BLEND);

	//darkening passes
	if( r_bloom_darken->value )
	{
		GL_BlendFunc(GL_DST_COLOR, GL_ZERO);
		GL_TexEnv(GL_MODULATE);
		
		for(i=0; i<r_bloom_darken->value ;i++) {
			R_Bloom_SamplePass( 0, 0 );
		}
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}

	//bluring passes
	//GL_BlendFunc(GL_ONE, GL_ONE);
	GL_BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	
	if( r_bloom_diamond_size->value > 7 || r_bloom_diamond_size->value <= 3)
	{
		if( (int)r_bloom_diamond_size->value != 8 ) Cvar_SetValue( "r_bloom_diamond_size", 8 );

		for(i=0; i<r_bloom_diamond_size->value; i++) {
			for(j=0; j<r_bloom_diamond_size->value; j++) {
				intensity = r_bloom_intensity->value * 0.3 * Diamond8x[i][j];
				if( intensity < r_bloom_threshold->value ) continue;
				qglColor4f( intensity, intensity, intensity, 1.0);
				R_Bloom_SamplePass( i-4, j-4 );
			}
		}
	} else if( r_bloom_diamond_size->value > 5 ) {
		
		if( r_bloom_diamond_size->value != 6 ) Cvar_SetValue( "r_bloom_diamond_size", 6 );

		for(i=0; i<r_bloom_diamond_size->value; i++) {
			for(j=0; j<r_bloom_diamond_size->value; j++) {
				intensity = r_bloom_intensity->value * 0.5 * Diamond6x[i][j];
				if( intensity < r_bloom_threshold->value ) continue;
				qglColor4f( intensity, intensity, intensity, 1.0);
				R_Bloom_SamplePass( i-3, j-3 );
			}
		}
	} else if( r_bloom_diamond_size->value > 3 ) {

		if( (int)r_bloom_diamond_size->value != 4 ) Cvar_SetValue( "r_bloom_diamond_size", 4 );

		for(i=0; i<r_bloom_diamond_size->value; i++) {
			for(j=0; j<r_bloom_diamond_size->value; j++) {
				intensity = r_bloom_intensity->value * 0.8f * Diamond4x[i][j];
				if( intensity < r_bloom_threshold->value ) continue;
				qglColor4f( intensity, intensity, intensity, 1.0);
				R_Bloom_SamplePass( i-2, j-2 );
			}
		}
	}
	
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	//restore full screen workspace
	qglViewport( 0, 0, vid.width, vid.height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, vid.width, vid.height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();
}											

/*
=================
R_Bloom_DownsampleView
=================
*/
void R_Bloom_DownsampleView( void )
{
	GL_Disable( GL_BLEND );
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	//stepped downsample
	if( r_screendownsamplingtexture_size )
	{
		int		midsample_width = r_screendownsamplingtexture_size * sampleText_tcw;
		int		midsample_height = r_screendownsamplingtexture_size * sampleText_tch;
		
		//copy the screen and draw resized
		GL_Bind(r_bloomscreentexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, curView_x, vid.height - (curView_y + curView_height), curView_width, curView_height);
		R_Bloom_Quad( 0,  vid.height-midsample_height, midsample_width, midsample_height, screenText_tcw, screenText_tch  );
		
		//now copy into Downsampling (mid-sized) texture
		GL_Bind(r_bloomdownsamplingtexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, midsample_width, midsample_height);

		//now draw again in bloom size
		qglColor4f( 0.5f, 0.5f, 0.5f, 1.0f );
		R_Bloom_Quad( 0,  vid.height-sample_height, sample_width, sample_height, sampleText_tcw, sampleText_tch );
		
		//now blend the big screen texture into the bloom generation space (hoping it adds some blur)
		GL_Enable( GL_BLEND );
		GL_BlendFunc(GL_ONE, GL_ONE);
		qglColor4f( 0.5f, 0.5f, 0.5f, 1.0f );
		GL_Bind(r_bloomscreentexture->texnum);
		R_Bloom_Quad( 0,  vid.height-sample_height, sample_width, sample_height, screenText_tcw, screenText_tch );
		qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		GL_Disable( GL_BLEND );

	} else {	//downsample simple

		GL_Bind(r_bloomscreentexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, curView_x, vid.height - (curView_y + curView_height), curView_width, curView_height);
		R_Bloom_Quad( 0, vid.height-sample_height, sample_width, sample_height, screenText_tcw, screenText_tch );
	}
}

/*
=================
R_BloomBlend
=================
*/
void R_SetupGL (void);
void R_BloomBlend ( refdef_t *fd )
{
	if( !(fd->rdflags & RDF_BLOOM) || !r_bloom->value || r_showtris->value )
		return;

	if( !BLOOM_SIZE )
		R_Bloom_InitTextures();

	if( screen_texture_width < BLOOM_SIZE ||
		screen_texture_height < BLOOM_SIZE )
		return;
	
	//set up full screen workspace
	qglViewport ( 0, 0, vid.width, vid.height );
	GL_TexEnv (GL_REPLACE); // Knightmare added
	GL_Disable (GL_DEPTH_TEST);
	qglMatrixMode (GL_PROJECTION);
    qglLoadIdentity ();
	qglOrtho(0, vid.width, vid.height, 0, -10, 100);
	qglMatrixMode (GL_MODELVIEW);
    qglLoadIdentity ();
	GL_Disable (GL_CULL_FACE);

	GL_Disable (GL_BLEND);
	qglEnable (GL_TEXTURE_2D);

	qglColor4f (1, 1, 1, 1);

	//set up current sizes
	curView_x = fd->x;
	curView_y = fd->y;
	curView_width = fd->width;
	curView_height = fd->height;
	screenText_tcw = ((float)fd->width / (float)screen_texture_width);
	screenText_tch = ((float)fd->height / (float)screen_texture_height);
	if( fd->height > fd->width ) {
		sampleText_tcw = ((float)fd->width / (float)fd->height);
		sampleText_tch = 1.0f;
	} else {
		sampleText_tcw = 1.0f;
		sampleText_tch = ((float)fd->height / (float)fd->width);
	}
	sample_width = BLOOM_SIZE * sampleText_tcw;
	sample_height = BLOOM_SIZE * sampleText_tch;
	
	//copy the screen space we'll use to work into the backup texture
	GL_Bind(r_bloombackuptexture->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, r_screenbackuptexture_size * sampleText_tcw, r_screenbackuptexture_size * sampleText_tch);

	//create the bloom image
	R_Bloom_DownsampleView();
	R_Bloom_GeneratexDiamonds();
	//R_Bloom_GeneratexCross();

	//restore the screen-backup to the screen
	GL_Disable(GL_BLEND);
	GL_Bind(r_bloombackuptexture->texnum);
	qglColor4f( 1, 1, 1, 1 );
	R_Bloom_Quad( 0, 
		vid.height - (r_screenbackuptexture_size * sampleText_tch),
		r_screenbackuptexture_size * sampleText_tcw,
		r_screenbackuptexture_size * sampleText_tch,
		sampleText_tcw,
		sampleText_tch );

	R_Bloom_DrawEffect();

	// Knightmare added
	R_SetupGL ();
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglEnable (GL_TEXTURE_2D);
	qglColor4f(1,1,1,1);
}

#endif // LIGHT_BLOOMS
