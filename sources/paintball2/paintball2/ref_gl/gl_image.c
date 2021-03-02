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

#include "gl_local.h"
#include "gl_cin.h"
#ifdef WIN32
#include "jpeg/jpeglib.h"
#include "png/png.h"
#else
#include <jpeglib.h>
#include <png.h>
#endif
#include <ctype.h>

image_t			gltextures[MAX_GLTEXTURES];
hash_table_t	gltextures_hash; // jithash
int				numgltextures;
int				base_textureid;		// gltextures[i] = base_textureid+i

static int		g_scrap_texnum = 0; // jitgentex

unsigned		d_8to24table[256];

int				gl_solid_format = 3;
int				gl_alpha_format = 4;

int				gl_tex_solid_format = 3;
int				gl_tex_alpha_format = 4;

int				gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;	// default to trilinear filtering - MrG
int				gl_filter_max = GL_LINEAR;


qboolean GL_Upload8 (byte *data, int width, int height, imagetype_t imagetype, qboolean sharp); // jitsky
qboolean GL_Upload32 (unsigned *data, int width, int height, imagetype_t imagetype, qboolean sharp); // jitsky


void GL_EnableMultitexture (qboolean enable)
{
	if (!qgl.SelectTextureSGIS && !qgl.ActiveTextureARB)
		return;

	if (enable)
	{
		GL_SelectTexture(QGL_TEXTURE1);
		qgl.Enable(GL_TEXTURE_2D);
		GL_TexEnv(GL_REPLACE);
	}
	else
	{
		GL_SelectTexture(QGL_TEXTURE1);
		qgl.Disable(GL_TEXTURE_2D);
		GL_TexEnv(GL_REPLACE);
	}

	GL_SelectTexture(QGL_TEXTURE0);
	GL_TexEnv(GL_REPLACE);

#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif
}

void GL_SelectTexture (GLenum texture)
{
	int tmu;

	if (!qgl.SelectTextureSGIS && !qgl.ActiveTextureARB)
		return;

	/*if (texture == QGL_TEXTURE0)
		tmu = 0;
	else
		tmu = 1;*/
	tmu = texture - QGL_TEXTURE0; // jit

	if (tmu == gl_state.currenttmu)
		return;

	gl_state.currenttmu = tmu;

	if (qgl.SelectTextureSGIS)
	{
		qgl.SelectTextureSGIS(texture);
	}
	else if (qgl.ActiveTextureARB)
	{
		qgl.ActiveTextureARB(texture);
		qgl.ClientActiveTextureARB(texture);
	}
}

// <!-- jitbright
extern cvar_t	*gl_overbright;

void GL_TexEnv(GLenum mode) 
{
	static int lastmodes[32] = { -1, -1 };

	if (mode != lastmodes[gl_state.currenttmu])
	{
		if (GL_COMBINE_EXT == mode) // a bit of a hack...
		{
			if (gl_state.texture_combine && gl_overbright->value)
			{
				qgl.TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
#ifdef QUAKE2
				qgl.TexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 4);
#else
				qgl.TexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
#endif
			}
			else // failed to combine, default to modulate.
			{
				mode = GL_MODULATE;
			}
		}

		//qgl.TexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		qgl.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode); // jit - should be i, right?
		lastmodes[gl_state.currenttmu] = mode;		
	}
}
// jit -->

void GL_Bind (int texnum)
{
#if 0 // jit - don't need that
	extern	image_t	*draw_chars;

	if (gl_nobind->value && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
#endif

	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;

	gl_state.currenttextures[gl_state.currenttmu] = texnum;
	qgl.BindTexture(GL_TEXTURE_2D, texnum);
}

void GL_MBind (GLenum target, int texnum)
{
	GL_SelectTexture(target);

	if (target == QGL_TEXTURE0)
	{
		if (gl_state.currenttextures[0] == texnum)
			return;
	}
	else
	{
		if (gl_state.currenttextures[1] == texnum)
			return;
	}

	GL_Bind(texnum);
}

typedef struct
{
	char *name;
	int	minimize, maximize;
} glmode_t;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

#define NUM_GL_MODES (sizeof(modes) / sizeof (glmode_t))

typedef struct
{
	char *name;
	int mode;
} gltmode_t;

gltmode_t gl_alpha_modes[] = {
	{"default", 4},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8", GL_RGBA8},
	{"GL_RGB5_A1", GL_RGB5_A1},
	{"GL_RGBA4", GL_RGBA4},
	{"GL_RGBA2", GL_RGBA2},
};

#define NUM_GL_ALPHA_MODES (sizeof(gl_alpha_modes) / sizeof (gltmode_t))

gltmode_t gl_solid_modes[] = {
	{"default", 3},
	{"GL_RGB", GL_RGB},
	{"GL_RGB8", GL_RGB8},
	{"GL_RGB5", GL_RGB5},
	{"GL_RGB4", GL_RGB4},
	{"GL_R3_G3_B2", GL_R3_G3_B2},
#ifdef GL_RGB2_EXT
	{"GL_RGB2", GL_RGB2_EXT},
#endif
};

#define NUM_GL_SOLID_MODES (sizeof(gl_solid_modes) / sizeof (gltmode_t))

/*
===============
GL_TextureMode
===============
*/
void GL_TextureMode(const char *string)
{
	int		i;
	image_t	*glt;

	for (i = 0; i < NUM_GL_MODES; i++)
		if (Q_strcaseeq(modes[i].name, string))
			break;

	if (i == NUM_GL_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "Bad filter name.\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky && glt->type != it_sharppic && glt->type != it_reflection) // jitrscript, jitwater
		{
			GL_Bind(glt->texnum);
			qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}
}

/*
===============
GL_TextureAlphaMode
===============
*/
void GL_TextureAlphaMode (const char *string)
{
	int		i;

	for (i = 0; i < NUM_GL_ALPHA_MODES; i++)
		if (Q_strcaseeq(gl_alpha_modes[i].name, string))
			break;

	if (i == NUM_GL_ALPHA_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "Bad alpha texture mode name.\n");
		return;
	}

	gl_tex_alpha_format = gl_alpha_modes[i].mode;
}

/*
===============
GL_TextureSolidMode
===============
*/
void GL_TextureSolidMode (const char *string)
{
	int		i;

	for (i = 0; i < NUM_GL_SOLID_MODES; ++i)
	{
		if (Q_strcaseeq(gl_solid_modes[i].name, string))
			break;
	}

	if (i == NUM_GL_SOLID_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "Bad solid texture mode name.\n");
		return;
	}

	gl_tex_solid_format = gl_solid_modes[i].mode;
}


/*
===============
GL_ImageList_f
===============
*/
void	GL_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	ri.Con_Printf(PRINT_ALL, "------------------\n");
	texels = 0;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (image->texnum <= 0)
			continue;

		texels += image->upload_width*image->upload_height;

		switch (image->type)
		{
		case it_skin:
			ri.Con_Printf(PRINT_ALL, "M");
			break;
		case it_sprite:
			ri.Con_Printf(PRINT_ALL, "S");
			break;
		case it_wall:
			ri.Con_Printf(PRINT_ALL, "W");
			break;
		case it_pic:
		case it_sharppic: // jitrscript
			ri.Con_Printf(PRINT_ALL, "P");
			break;
		default:
			ri.Con_Printf(PRINT_ALL, " ");
			break;
		}

		ri.Con_Printf(PRINT_ALL,  " %3i %3i %s: %s\n",
			image->upload_width, image->upload_height, palstrings[image->paletted], image->name);
	}

	ri.Con_Printf(PRINT_ALL, "Total texel count (not counting mipmaps): %i\n", texels);
}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT];
qboolean	scrap_dirty;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0; texnum<MAX_SCRAPS; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0; i<BLOCK_WIDTH-w; i++)
		{
			best2 = 0;

			for (j=0; j<w; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0; i<w; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	if (!g_scrap_texnum)
		qgl.GenTextures(1, &g_scrap_texnum);

	scrap_uploads++;
	GL_Bind(g_scrap_texnum);
	GL_Upload8(scrap_texels[0], BLOCK_WIDTH, BLOCK_HEIGHT, it_pic, true); // (sharp) false);
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void LoadPCX (const char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;
	*palette = NULL;

	//
	// load the file
	//
	len = ri.FS_LoadFile(filename, (void **)&raw);
	if (!raw)
	{
		//ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s.\n", filename);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

#ifndef USEFILEMAP // jitfilemap
	// note: Filemapping prevents write access, but since it only works on windows
	// and windows only works on little-endian computers, this conversion is unneccessary.
    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);
#endif

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ri.Con_Printf(PRINT_ALL, "Bad pcx file %s.\n", filename);
		return;
	}

	out = malloc((pcx->ymax+1)*(pcx->xmax+1));
	*pic = out;
	pix = out;

	if (palette)
	{
		*palette = malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0; y<=pcx->ymax; y++, pix += pcx->xmax+1)
	{
		for (x=0; x<=pcx->xmax;)
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - (byte *)pcx > len)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed.", filename);
		free (*pic);
		*pic = NULL;
	}

	ri.FS_FreeFile(pcx);
}

/*
=========================================================

PNG LOADING - Code by R1CH, taken from R1GL

=========================================================
*/
#define MAX_TEXTURE_DIMENSIONS 2048
// correct this if wrong - viciouz

typedef struct {
    byte *Buffer;
    size_t Pos;
} TPngFileBuffer;

void PngReadFunc(png_struct *Png, png_bytep buf, png_size_t size)
{
    TPngFileBuffer *PngFileBuffer=(TPngFileBuffer*)png_get_io_ptr(Png);
    memcpy(buf,PngFileBuffer->Buffer+PngFileBuffer->Pos,size);
    PngFileBuffer->Pos+=size;
}

void LoadPNG (const char *name, byte **pic, int *width, int *height)
{
	unsigned int	i, rowbytes;
	png_structp		png_ptr;
	png_infop		info_ptr;
	png_infop		end_info;
	png_bytep		row_pointers[MAX_TEXTURE_DIMENSIONS];
	double			file_gamma;

	TPngFileBuffer	PngFileBuffer = {NULL,0};

	*pic = NULL;

	ri.FS_LoadFile (name, (void *)&PngFileBuffer.Buffer);

    if (!PngFileBuffer.Buffer)
		return;

	if ((png_check_sig(PngFileBuffer.Buffer, 8)) == 0)
	{
		ri.FS_FreeFile (PngFileBuffer.Buffer); 
		ri.Con_Printf (PRINT_ALL, "Not a PNG file: %s\n", name);
		return;
    }

	PngFileBuffer.Pos=0;

    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL,  NULL, NULL);

    if (!png_ptr)
	{
		ri.FS_FreeFile (PngFileBuffer.Buffer);
		ri.Con_Printf (PRINT_ALL, "Bad PNG file: %s\n", name);
		return;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
	{
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		ri.FS_FreeFile (PngFileBuffer.Buffer);
		ri.Con_Printf (PRINT_ALL, "Bad PNG file: %s\n", name);
		return;
    }
    
	end_info = png_create_info_struct(png_ptr);
    if (!end_info)
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		ri.FS_FreeFile (PngFileBuffer.Buffer);
		ri.Con_Printf (PRINT_ALL, "Bad PNG file: %s\n", name);
		return;
    }

	png_set_read_fn (png_ptr,(png_voidp)&PngFileBuffer,(png_rw_ptr)PngReadFunc);

	png_read_info(png_ptr, info_ptr);

	if (info_ptr->height > MAX_TEXTURE_DIMENSIONS)
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		ri.FS_FreeFile (PngFileBuffer.Buffer);
		ri.Con_Printf (PRINT_ALL, "Oversized PNG file: %s\n", name);
		return;
	}

	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb (png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

	if ((info_ptr->color_type == PNG_COLOR_TYPE_GRAY) && info_ptr->bit_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY || info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	if (info_ptr->bit_depth == 16)
		png_set_strip_16(png_ptr);

	if (info_ptr->bit_depth < 8)
        png_set_packing(png_ptr);

	if (png_get_gAMA(png_ptr, info_ptr, &file_gamma))
		png_set_gamma (png_ptr, 2.0, file_gamma);

	png_read_update_info(png_ptr, info_ptr);

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	*pic = malloc (info_ptr->height * rowbytes);

	for (i = 0; i < info_ptr->height; i++)
		row_pointers[i] = *pic + i*rowbytes;

	png_read_image(png_ptr, row_pointers);

	*width = info_ptr->width;
	*height = info_ptr->height;

	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	ri.FS_FreeFile (PngFileBuffer.Buffer);
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void LoadTGA (char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile(name, (void **)&buffer);

	if (!buffer)
		return;

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort (*((short *)tmp));
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort (*((short *)tmp));
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort (*((short *)buf_p));
	buf_p+=2;
	targa_header.y_origin = LittleShort (*((short *)buf_p));
	buf_p+=2;
	targa_header.width = LittleShort (*((short *)buf_p));
	buf_p+=2;
	targa_header.height = LittleShort (*((short *)buf_p));
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	// jit - Knightmare- check for bad data
	if (!targa_header.width || !targa_header.height) {
		ri.Con_Printf (PRINT_ALL, "Bad tga file %s.\n", name);
		ri.FS_FreeFile (buffer);
		return;
	}

	if (targa_header.image_type != 2 
		&& targa_header.image_type != 10) {
		ri.Con_Printf (PRINT_ALL, "LoadTGA: %s has wrong image format; only type 2 and 10 targa RGB images supported.\n", name);
		ri.FS_FreeFile (buffer);
		return;
	}

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24)) {
		ri.Con_Printf (PRINT_ALL, "LoadTGA: %s has wrong image format; only 32 or 24 bit images supported (no colormaps).\n", name);
		ri.FS_FreeFile (buffer);
		return;
	} 

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2)
	{
		// Uncompressed, RGB images
		for(row=rows-1; row>=0; row--)
		{
			pixbuf = targa_rgba + row*columns*4;

			for(column=0; column<columns; column++)
			{
				unsigned char red,green,blue,alphabyte;

				switch (targa_header.pixel_size)
				{
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						break;
				}
			}
		}
	}
	else if (targa_header.image_type==10)
	{
		// Runlength encoded RGB images
		unsigned char red=0,green=0,blue=0,alphabyte=255,packetHeader,packetSize,j;

		for(row=rows-1; row>=0; row--)
		{
			pixbuf = targa_rgba + row*columns*4;

			for(column=0; column<columns;)
			{
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);

				if (packetHeader & 0x80)
				{
					// run-length packet
					switch (targa_header.pixel_size)
					{
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++)
					{
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;

						if (column==columns)
						{
							// run spans across rows
							column=0;

							if (row>0)
								row--;
							else
								goto breakOut;

							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else
				{
					// non run-length packet
					for(j=0;j<packetSize;j++)
					{
						switch (targa_header.pixel_size)
						{
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						}

						column++;

						if (column==columns)
						{
							// pixel packet run spans across rows
							column=0;

							if (row>0)
								row--;
							else
								goto breakOut;

							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}

			breakOut:;
		}
	}

	ri.FS_FreeFile (buffer);
}


/*
=================================================================

JPEG LOADING

By Robert 'Heffo' Heffernan

=================================================================
*/

void jpg_null(j_decompress_ptr cinfo)
{
}

boolean jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
    ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data.\n");
    return 1;
}

void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
        
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

    if (cinfo->src->bytes_in_buffer < 0) 
		ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data.\n");
}

void jpeg_mem_src(j_decompress_ptr cinfo, byte *mem, int len)
{
    cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
    cinfo->src->init_source = jpg_null;
    cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
    cinfo->src->skip_input_data = jpg_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart;
    cinfo->src->term_source = jpg_null;
    cinfo->src->bytes_in_buffer = len;
    cinfo->src->next_input_byte = mem;
}

/*
==============
LoadJPG
==============
*/
jmp_buf jpeg_error_jump;

void jpeg_error_exit (j_common_ptr cinfo)
{
	// Display jpeg error.
	char buffer[JMSG_LENGTH_MAX];

	(*cinfo->err->format_message)(cinfo, buffer);
	ri.Con_Printf(PRINT_ALL, "%s\n", buffer);
	longjmp(jpeg_error_jump, -1);
}

void LoadJPG (char *filename, byte **pic, int *width, int *height)
{
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr			jerr;
	byte							*rawdata, *rgbadata, *scanline, *p, *q;
	int								rawsize, i;

	// Load JPEG file into memory
	rawsize = ri.FS_LoadFile(filename, (void **)&rawdata);
	if (!rawdata)
		return;	

	// jits - If the library fails to load the image, it will jump back here.
	if (setjmp(jpeg_error_jump))
	{
		ri.Con_Printf(PRINT_ALL, "Bad jpg file: %s\n", filename);
		ri.FS_FreeFile(rawdata);
		return;
	}

	// Initialise libJpeg Object
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = jpeg_error_exit; // jit - don't exit on a bad jpeg image.
	jpeg_create_decompress(&cinfo);

	// Feed JPEG memory into the libJpeg Object
	jpeg_mem_src(&cinfo, rawdata, rawsize);

	// Process JPEG header
	jpeg_read_header(&cinfo, true);

	// Start Decompression
	jpeg_start_decompress(&cinfo);

	// Check Colour Components
	if (cinfo.output_components != 3)
	{
		ri.Con_Printf(PRINT_ALL, "Invalid JPEG colour components.\n");
		jpeg_destroy_decompress(&cinfo);
		ri.FS_FreeFile(rawdata);
		return;
	}

	// Allocate Memory for decompressed image
	rgbadata = malloc(cinfo.output_width * cinfo.output_height * 4);
	if (!rgbadata)
	{
		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG buffer.\n");
		jpeg_destroy_decompress(&cinfo);
		ri.FS_FreeFile(rawdata);
		return;
	}

	// Pass sizes to output
	*width = cinfo.output_width; *height = cinfo.output_height;

	// Allocate Scanline buffer
	scanline = malloc(cinfo.output_width * 3);
	if (!scanline)
	{
		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG scanline buffer.\n");
		free(rgbadata);
		jpeg_destroy_decompress(&cinfo);
		ri.FS_FreeFile(rawdata);
		return;
	}

	// Read Scanlines, and expand from RGB to RGBA
	q = rgbadata;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		p = scanline;
		jpeg_read_scanlines(&cinfo, &scanline, 1);

		for(i=0; i<cinfo.output_width; i++)
		{
			q[0] = p[0];
			q[1] = p[1];
			q[2] = p[2];
			q[3] = 255;

			p+=3; q+=4;
		}
	}

	// Free the scanline buffer
	free(scanline);

	// Finish Decompression
	jpeg_finish_decompress(&cinfo);

	// Destroy JPEG object
	jpeg_destroy_decompress(&cinfo);

	// Return the 'rgbadata'
	*pic = rgbadata;
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP(off, dx, dy) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin(byte *skin, int skinwidth, int skinheight)
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf("not filling skin from %d to %d\n", fillcolor, filledcolor);
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP(-1, -1, 0);
		if (x < skinwidth - 1)	FLOODFILL_STEP(1, 1, 0);
		if (y > 0)				FLOODFILL_STEP(-skinwidth, 0, -1);
		if (y < skinheight - 1)	FLOODFILL_STEP(skinwidth, 0, 1);
		skin[x + skinwidth * y] = fdc;
	}
}

//=======================================================


/*
================
GL_ResampleTexture
================
*/
void GL_ResampleTexture (const unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight)
{
	int			i, j;
	const unsigned *inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[4096], p2[4096]; // jittex
	byte		*pix1, *pix2, *pix3, *pix4;

	outwidth = min(outwidth, 4096); // jittex
	outheight = min(outheight, 4096); // jittex
	fracstep = inwidth * 0x10000 / outwidth;
	frac = fracstep >> 2;

	for (i = 0; i < outwidth; i++)
	{
		p1[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	frac = 3 * (fracstep >> 2);

	for (i = 0; i < outwidth; i++)
	{
		p2[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	for (i = 0; i < outheight; i++, out += outwidth)
	{
		inrow = in + inwidth * (int)((i + 0.25f) * inheight / outheight);
		inrow2 = in + inwidth * (int)((i + 0.75f) * inheight / outheight);
		frac = fracstep >> 1;

		for (j = 0; j < outwidth; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out + j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			((byte *)(out + j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			((byte *)(out + j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
			((byte *)(out + j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
		}
	}
}


/*
================
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<= 2;
	height >>= 1;
	out = in;

	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, out += 4, in += 8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4]) >> 2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5]) >> 2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6]) >> 2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7]) >> 2;

			// jitmipmap -- a little hack to make alpha things like fences not disappear in the distance
			if (in[7] > out[3])
				out[3] = (in[7] + out[3]) >> 1;

			if (in[width + 3] > out[3])
				out[3] = (in[width + 3] + out[3]) >> 1;
		}
	}
}

void GL_BuildPalettedTexture(unsigned char *paletted_texture, unsigned char *scaled, int scaled_width, int scaled_height)
{
	int i;

	for (i = 0; i < scaled_width * scaled_height; i++)
	{
		unsigned int r, g, b, c;

		r = (scaled[0] >> 3) & 31;
		g = (scaled[1] >> 2) & 63;
		b = (scaled[2] >> 3) & 31;
		c = r | (g << 5) | (b << 11);
		paletted_texture[i] = gl_state.d_16to8table[c];
		scaled += 4;
	}
}

int		upload_width, upload_height;
qboolean uploaded_paletted;

static int nearest_power_of_2 (int size)
{
	int i = 2;

	while (1)
	{
		i <<= 1;

		if (size == i)
			return i;

		if (size > i && size < (i <<1))
		{
			if (size >= ((i+(i<<1))/2))
				return i<<1;
			else
				return i;
		}
	};
}

extern cvar_t *gl_anisotropy; // jitanisotropy
extern cvar_t *gl_texture_saturation; // jitsaturation
void desaturate_texture(unsigned *udata, int width, int height) // jitsaturation
{
	int i,size;
	float r,g,b,v,s;
	unsigned char *data;

	data = (unsigned char*)udata;

	s = gl_texture_saturation->value;

	size = width*height*4;

	for (i=0; i<size; i+=4)
	{
		r = data[i];
		g = data[i+1];
		b = data[i+2];
		v = r * 0.30 + g * 0.59 + b * 0.11;

		data[i]   = (1-s)*v + s*r;
		data[i+1] = (1-s)*v + s*g;
		data[i+2] = (1-s)*v + s*b;
	}
}

// ===[
// jit3dfx -- taken from Mesa source code since some 3dfx drivers don't support glu.

/*
 * Compute ceiling of integer quotient of A divided by B:
 */
#define CEILING(A, B)  ((A) % (B) == 0 ? (A)/(B) : (A)/(B)+1)


/*
===============
GL_Upload32

Returns has_alpha
===============
*/
qboolean GL_Upload32 (unsigned *data, int width, int height, imagetype_t imagetype, qboolean sharp)
{
	int			samples;
	unsigned	*scaled;
	int			scaled_width, scaled_height;
	int			i, c;
	byte		*scan;
	int			comp;
	GLint		max_size;
	qboolean	mipmap = (imagetype != it_pic && imagetype != it_sharppic && imagetype != it_sky && imagetype != it_reflection); // jitrscript
	qboolean	repeat = (imagetype != it_sky && imagetype != it_reflection); // jitwater

#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif

	uploaded_paletted = false;

	for (scaled_width = 1; scaled_width < width; scaled_width <<= 1);
	for (scaled_height = 1; scaled_height < height; scaled_height <<= 1);

	if (gl_round_down->value && scaled_width > width && mipmap)
		scaled_width >>= 1;

	if (gl_round_down->value && scaled_height > height && mipmap)
		scaled_height >>= 1;

	// let people sample down the world textures for speed
	if (gl_picmip->value > 2.0f)
		ri.Cvar_Set("gl_picmip", "2");
	else if (gl_picmip->value < 0.0f)
		ri.Cvar_Set("gl_picmip", "0");

	if (mipmap)
	{
		scaled_width >>= (int)gl_picmip->value;
		scaled_height >>= (int)gl_picmip->value;
	}

	if (imagetype == it_sky) // jitsky
	{
		if (gl_skymip->value > 3.0f)
			ri.Cvar_Set("gl_skymip", "3");
		else if (gl_skymip->value < 0.0f)
			ri.Cvar_Set("gl_skymip", "0");

		scaled_width >>= (int)gl_skymip->value;
		scaled_height >>= (int)gl_skymip->value;
	}

	// find max size card can handle
	qgl.GetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

	if (scaled_width > max_size)
		scaled_width = max_size;

	if (scaled_height > max_size)
		scaled_height = max_size;

	if (scaled_width <= 0)
		scaled_width = 1;

	if (scaled_height <= 0)
		scaled_height = 1;

	upload_width = scaled_width;
	upload_height = scaled_height;

	// scan the texture for any non-255 alpha
	c = width*height;
	scan = ((byte*)data) + 3;
	samples = gl_solid_format;

	for (i = 0; i < c; i++, scan += 4)
	{
		if (*scan != 255)
		{
			samples = gl_alpha_format;
			break;
		}
	}

	//Heffo - ARB Texture Compression
	if (samples == gl_solid_format)
		comp = (gl_state.texture_compression && mipmap) ? GL_COMPRESSED_RGB_ARB : gl_tex_solid_format;
	else if (samples == gl_alpha_format)
		comp = (gl_state.texture_compression && mipmap) ? GL_COMPRESSED_RGBA_ARB : gl_tex_alpha_format;


	if (scaled_width == width && scaled_height == height)
	{
		scaled = data;
	}
	else
	{
		scaled = malloc(scaled_width * scaled_height * 4);
		GL_ResampleTexture(data, width, height, scaled, scaled_width, scaled_height);
	}

	if (gl_texture_saturation->value < 1) // jitsaturation
		desaturate_texture(scaled, scaled_width, scaled_height);

	if (mipmap)
	{
		if (gl_state.sgis_mipmap) 
		{
			qgl.TexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			qgl.TexImage2D(GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		} 
		else
		{
			int miplevel = 0;

			qgl.TexImage2D(GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);

			while (scaled_width > 1 || scaled_height > 1)
			{
				GL_MipMap((byte *)scaled, scaled_width, scaled_height);
				scaled_width >>= 1;
				scaled_height >>= 1;

				if (scaled_width < 1)
					scaled_width = 1;

				if (scaled_height < 1)
					scaled_height = 1;

				miplevel++;
				qgl.TexImage2D(GL_TEXTURE_2D, miplevel, comp, scaled_width, scaled_height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			}
		}

		qgl.TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);

		if (gl_anisotropy->value) // jitanisotropy
			qgl.TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_anisotropy->value);
		else
			qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

#ifdef DEBUG
		{
			int err;

			err = qgl.GetError();
			assert(err == GL_NO_ERROR);
		}
#endif
	}
	else
	{
		if (gl_state.sgis_mipmap)
			qgl.TexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);

		qgl.TexImage2D(GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sharp ? GL_NEAREST : gl_filter_max);
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sharp ? GL_NEAREST : gl_filter_max);

#ifdef DEBUG
		{
			int err;

			err = qgl.GetError();
			assert(err == GL_NO_ERROR);
		}
#endif
	}

	// Clamp to edge not supported until 1.2
	if (repeat || gl_config.version < 1.2f) // jitsky, jitwater
	{
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}


	if (upload_width != width || upload_height != height)
		free(scaled);

#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif

	return (samples == gl_alpha_format);
}


/*
===============
GL_Upload8

Returns has_alpha
===============
*/
#define MAX_8BIT_WIDTH 512
#define MAX_8BIT_HEIGHT 256
qboolean GL_Upload8 (byte *data, int width, int height, imagetype_t imagetype, qboolean sharp)
{
	unsigned	trans[MAX_8BIT_WIDTH * MAX_8BIT_HEIGHT];
	int			i, s;
	int			p;

	s = width * height;

	if (s > sizeof(trans) / 4)
	{
		assert(0);
		ri.Con_Printf(PRINT_HIGH, "GL_Upload8: too large (%d x %d)", width, height);
		
		if (width > MAX_8BIT_WIDTH)
			width = MAX_8BIT_WIDTH;

		if (height > MAX_8BIT_HEIGHT)
			height = MAX_8BIT_HEIGHT;

		s = width * height; // texture will be corrupted, but at least it won't stop the map from loading.
	}

	for (i = 0; i < s; i++)
	{
		p = data[i];
		trans[i] = d_8to24table[p];

		if (p == 255)
		{	// transparent, so scan around for another color
			// to avoid alpha fringes
			if (i > width && data[i-width] != 255)
				p = data[i-width];
			else if (i < s-width && data[i+width] != 255)
				p = data[i+width];
			else if (i > 0 && data[i-1] != 255)
				p = data[i-1];
			else if (i < s-1 && data[i+1] != 255)
				p = data[i+1];
			else
				p = 0;

			// copy rgb components
			((byte *)&trans[i])[0] = ((byte *)&d_8to24table[p])[0];
			((byte *)&trans[i])[1] = ((byte *)&d_8to24table[p])[1];
			((byte *)&trans[i])[2] = ((byte *)&d_8to24table[p])[2];
		}
	}

	return GL_Upload32(trans, width, height, imagetype, sharp);
}


image_t  *GL_LoadDummyPic (const char *name, int width, int height, imagetype_t type) // jitrscript
{
	image_t *image;
	int i;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->texnum)
			break;
	}
	
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error(ERR_DROP, "MAX_GLTEXTURES");

		numgltextures++;
	}

	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error (ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);

	strcpy(image->name, name);
	image->registration_sequence = registration_sequence;
	image->width = width;
	image->height = height;
	image->type = type;
	image->texnum = -1; // hopefully this won't break anything - it wasn't here before.

	return image;
}


image_t *GL_CreateBlankImage (const char *name, int width, int height, imagetype_t type)
{
	image_t		*image;
	qboolean	sharp = false;
	int		i;
	unsigned char	*buf;

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (!image->texnum)
			break;
	}

	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error (ERR_DROP, "MAX_GLTEXTURES");

		numgltextures++;
	}

	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);

	Q_strncpyz(image->name, name, sizeof(image->name));
	image->registration_sequence = registration_sequence;
	image->width = width;
	image->height = height;
	image->type = type;
	image->scrap = false;
	qgl.GenTextures(1, &image->texnum); // jitgentex
	GL_Bind(image->texnum);
	buf = (unsigned char *)malloc(width * height * 4);     // create empty buffer for texture
	assert(buf);
	memset(buf, 255, (width * height * 4)); // default to white
	//memset(buf, 90, (width * height * 4)); // default to dark grey
	image->has_alpha = GL_Upload32((unsigned *)buf, width, height,
				image->type, sharp);
	free(buf);
	image->upload_width = upload_width;		// after power of 2 and scales
	image->upload_height = upload_height;
	image->paletted = uploaded_paletted;
	image->sl = 0;
	image->sh = 1;
	image->tl = 0;
	image->th = 1;

	return image;
}


/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits)
{
	image_t		*image;
	int			i;
	qboolean	sharp = false; // jit, for images we don't want filtered.

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (!image->texnum)
			break;
	}

	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error (ERR_DROP, "MAX_GLTEXTURES");

		numgltextures++;
	}

	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);

	Q_strncpyz(image->name, name, sizeof(image->name));
	image->registration_sequence = registration_sequence;
	
	image->width = width;
	image->height = height;
	image->type = type;

	// ===
	// jit -- paintball2 texture fix
	// jitodo -- move these to rscripts, this is an ugly hack.
	if (Q_strcaseeq(name, "textures/pball/b_flag1") ||
		Q_strcaseeq(name, "textures/pball/y_flag1") || 
		Q_strcaseeq(name, "textures/pball/p_flag1") || 
		Q_strcaseeq(name, "textures/pball/r_flag1"))
	{
		image->width = 96;
		image->height = 96;
	}
	else if (Q_strcaseeq(name,"textures/pball/ksplat2"))
	{
		image->width = 288;
		image->height = 128;
	}
	else if (strstr(name, "pics/gamma") || strstr(name, "pics/menu_char_colors") ||
		(image->type == it_pic && bits == 8) || image->type == it_sharppic) // jitrscript
	{
		sharp = true; // no bilinear filtering
	}
	// jit
	// ===

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64 && sharp)// && !sharp)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock(image->width, image->height, &x, &y);

		if (texnum == -1)
			goto nonscrap;

		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;

		for (i = 0; i < image->height; i++)
			for (j = 0; j < image->width; j++, k++)
				scrap_texels[texnum][(y + i) * BLOCK_WIDTH + x + j] = pic[k];
		
		// === jitgentex
		if (!g_scrap_texnum)
			qgl.GenTextures(1, &g_scrap_texnum);

		image->texnum = g_scrap_texnum;
		// jitgentex ===
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x + 0.01f) / (float)BLOCK_WIDTH;
		image->sh = (x + image->width - 0.01f) / (float)BLOCK_WIDTH;
		image->tl = (y + 0.01f) / (float)BLOCK_WIDTH;
		image->th = (y + image->height - 0.01f) / (float)BLOCK_WIDTH;
	}
	else
	{
nonscrap:
		image->scrap = false;
		qgl.GenTextures(1, &image->texnum); // jitgentex
		GL_Bind(image->texnum);

		if (bits == 8)
			image->has_alpha = GL_Upload8(pic, width, height,
				image->type, sharp); // jitsky, image->type
		else
			image->has_alpha = GL_Upload32((unsigned *)pic, width, height,
				image->type, sharp); // jitsky, image->type

		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	return image;
}


/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal (const char *name, const char *filename)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;

	ri.FS_LoadFile(filename, (void **)&mt);

	if (!mt)
	{
		//ri.Con_Printf (PRINT_ALL, "GL_FindImage: can't load %s\n", name);
		//return r_notexture;
		return NULL; // jithash / jittexture
	}

	width = LittleLong(mt->width);
	height = LittleLong(mt->height);
	ofs = LittleLong(mt->offsets[0]);
	image = GL_LoadPic(name, (byte *)mt + ofs, width, height, it_wall, 8);
	ri.FS_FreeFile((void *)mt);

	return image;
}

image_t *GL_LoadImage(const unsigned char *name, imagetype_t type) // jitimage / jithash
{
	static unsigned char tempname[MAX_QPATH];
	byte *pic=NULL, *palette=NULL;
	int width, height;
	image_t *image;
	
	// Try PNG:
	Com_sprintf(tempname, sizeof(tempname), "%s.png", name);
	LoadPNG(tempname, &pic, &width, &height);

	if (pic)
	{
		image = GL_LoadPic(name, pic, width, height, type, 32);
	}
	else
	{
	
		// Try TGA:
		Com_sprintf(tempname, sizeof(tempname), "%s.tga", name);
		LoadTGA(tempname, &pic, &width, &height);

		if (pic)
		{
			image = GL_LoadPic(name, pic, width, height, type, 32);
		}
		else	
		{	
			// Try JPG:
			Com_sprintf(tempname, sizeof(tempname), "%s.jpg", name);
			LoadJPG(tempname, &pic, &width, &height);
	
			if (pic)
			{
				image = GL_LoadPic(name, pic, width, height, type, 32);
			}
			else
			{
				// Try CIN: (or not)
				/*cinematics_t *newcin;
	
				sprintf(tempname, "%s.cin", name);
	
				newcin = CIN_OpenCin(name);
	
				if (newcin)
				{
					pic = malloc(256*256*4);
					memset(pic, 192, (256*256*4));
	
					image = GL_LoadPic(name, pic, 256, 256, type, 32);
	
					newcin->texnum = image->texnum;
					image->is_cin = true;
				}
				else*/
				{
					// Try PCX:
					Com_sprintf(tempname, sizeof(tempname), "%s.pcx", name);
					LoadPCX(tempname, &pic, &palette, &width, &height);
	
					if (pic)
					{
						image = GL_LoadPic(name, pic, width, height, type, 8);
					}
					else
					{
						// Try WAL:
						sprintf(tempname, "%s.wal", name);
						image = GL_LoadWal(name, tempname);					
					}
				}
			}
		}
	}

	if (pic)
		free(pic);

	if (palette)
		free(palette);

	return image;
}


image_t *GL_LoadRScriptImage (const char *name) // jitrscript
{
	rscript_t *rs;

	rs = RS_FindScript(name);

	if (rs)
	{
		int width, height;
		imagetype_t type;
		image_t *image;

		// The image file is missing, but there is an RScript under the same
		// name, so create a dummy image with an rscript pointer, since
		// it's really just the rscript that gets rendered, not the image
		if (rs->dontflush)
			type = it_pic;
		else
			type = it_wall;

		width = rs->width;
		height = rs->height;
		if (!width)
			width = 64; // just a lame default. Probably better to get width from 1st rscript layer.
		if (!height)
			height = 64;

		image = GL_LoadDummyPic(name, width, height, type);
		image->rscript = rs;
		rs->img_ptr = image;

		return image;
	}
	else
		return NULL;
}

/*
===============
GL_FindImage

Finds or loads the given image
===============
*/
//char override=0;


image_t	*GL_FindImage (const char *name, imagetype_t type)
{
	image_t	*image = NULL;
	int		i, len;
	const char *in;
	char *out;
	extern cvar_t *gl_highres_textures;
	extern cvar_t *gl_hash_textures; // jithash
	static unsigned char name_noext[MAX_QPATH];

	if (!name)
		return NULL;

	len = strlen(name);

	if (len < 5)
		return NULL;

	// look for it

	// strip extension:
	in = name;
	out = name_noext;

	while (*in && *in != '.')
		*out++ = tolower(*in++);

	*out = 0;

	if (gl_hash_textures->value) // jithash
	{
		image = hash_get(&gltextures_hash, name_noext);

		if (image)
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}
	else
	{
		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (Q_streq(name_noext, image->name))
			{
				image->registration_sequence = registration_sequence;
				return image;
			}
		}

		image = NULL; // image not found.
	}

	//
	// load the pic from disk
	//

	if (gl_highres_textures->value) // jithighres
	{
		static unsigned char name_hr4_noext[MAX_QPATH];
		int len;
		unsigned char *s;

		// insert hr4/ before the filename:
		len = strlen(name_noext);
		s = name_noext + len - 1; // start at the end of the string and work backwards.
		
		while (s > name_noext && *s != '/' && *s != '\\') // till we hit '/'
			s --;

		*s = 0; // temporarily terminate it at the '/'
		Com_sprintf(name_hr4_noext, sizeof(name_hr4_noext), "%s/hr4/%s", name_noext, s+1);
		*s = '/'; // put the '/' back.

		image = GL_LoadImage(name_hr4_noext, type);

		if (image)
		{
			image->width /= 4;
			image->height /= 4;
			strcpy(image->name, name_noext);
		}
	}

	if (!image) // highres textures disabled or no highres texture found
	{
		image = GL_LoadImage(name_noext, type);

		if (!image)
		{
			image = GL_LoadRScriptImage(name_noext); // jitrscript

			if (!image)
			{
				ri.Con_Printf(PRINT_ALL, "GL_FindImage: Can't load %s.\n", name_noext);
				image = r_notexture;
			}
		}
	}

	image->rscript = RS_FindScript(name_noext); // jitrscript

	if (image->rscript)
	{
		if (image->rscript->width)
			image->width = image->rscript->width;
		
		if (image->rscript->height)
			image->height = image->rscript->height;

		image->rscript->img_ptr = image; // jitrscript -- point back to this image
	}

	hash_add(&gltextures_hash, name_noext, image); // jithash

	return image;
}

/*
===============
R_RegisterSkin
===============
*/
void R_RegisterSkin (const char *name, struct model_s *model, struct image_s **skins) // jitskm
{
	char skinname[MAX_QPATH];
	int len;
	char *skindata;
	mskmodel_t *skmodel;
	int i, max = MAX_MESHSKINS;

	if (!skins)
		return;

	if (!model || !model->skmodel)
	{
		skins[0] = GL_FindImage(name, it_skin);
		return;
	}

	skmodel = model->skmodel;
	
	if (skmodel->nummeshes < max)
		max = skmodel->nummeshes;

	COM_StripExtension(name, skinname, sizeof(skinname));
	strcat(skinname, ".skin");
	len = ri.FS_LoadFileZ(skinname, (void **)&skindata);

	if (len < 0)
	{
		skins[0] = GL_FindImage(name, it_skin);
	}
	else
	{
		char *pskindata = skindata;
		char *line = COM_Parse(&pskindata);
		char *s;

		while (pskindata)
		{
			s = strchr(line, ',');

			if (!s)
			{
				line = COM_Parse(&pskindata);
				continue;
			}

			*s = 0;
			s++;

			for (i = 0; i < max; ++i)
			{
				if (Q_strcasecmp(skmodel->meshes[i].shadername, line) == 0)
				{
					skins[i] = GL_FindImage(s, it_skin);
					break;
				}
			}

			line = COM_Parse(&pskindata);
		}

		ri.FS_FreeFile(skindata);
	}

	// make sure there are no null skins
	for (i = 0; i < max; ++i)
		if (!skins[i])
			skins[i] = r_notexture;
}


/*
================
GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void GL_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	// never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_particletexture->registration_sequence = registration_sequence;
	r_whitetexture->registration_sequence = registration_sequence;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
			continue;		// used this sequence
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type == it_pic || image->type == it_sharppic)
			continue;		// don't free pics

		//Heffo - Free Cinematic
		if (image->is_cin)
			CIN_FreeCin(image->texnum);

		// free it
		hash_delete(&gltextures_hash, image->name);

		if (image->rscript) // jitrscript
			image->rscript->img_ptr = NULL;

		qgl.DeleteTextures(1, &image->texnum);
		memset(image, 0, sizeof(image_t)); // jit (not sure if this makes a difference)
	}
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette (void)
{
	int		i;
	int		r, g, b;
	unsigned	v;
	byte	*pic, *pal;
	int		width, height;

	// get the palette
	LoadPCX("pics/colormap.pcx", &pic, &pal, &width, &height);

	if (!pal)
	{
		ri.Sys_Error(ERR_FATAL, "Couldn't load pics/colormap.pcx\nPlease make sure the game is installed properly.\nView the documentation on digitalpaint.org for more details."); // jit
	}


	for (i = 0; i < 256; i++)
	{
		r = pal[i * 3 + 0];
		g = pal[i * 3 + 1];
		b = pal[i * 3 + 2];
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		d_8to24table[i] = LittleLong(v);
	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	free(pic);
	free(pal);

	return 0;
}


/*
===============
GL_InitImages
===============
*/
void GL_InitImages (void)
{
	hash_table_init(&gltextures_hash, 0x200, NULL);
	qgl.GenTextures(MAX_LIGHTMAPS, gl_state.lightmap_texnums); // jitgentex
	Draw_GetPalette();

	if (qgl.ColorTableEXT)
	{
		ri.FS_LoadFile("pics/16to8.dat", (void**)&gl_state.d_16to8table);

		if (!gl_state.d_16to8table)
			ri.Sys_Error(ERR_FATAL, "Couldn't load pics/16to8.dat"); // jit, just bugged me
	}
}


/*
===============
GL_ShutdownImages
===============
*/
void GL_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	// === jitgentex
	for (i = 0; i < MAX_LIGHTMAPS; ++i)
	{
		qgl.DeleteTextures(MAX_LIGHTMAPS, gl_state.lightmap_texnums);
	}

	qgl.DeleteTextures(1, &g_scrap_texnum);
	g_scrap_texnum = 0;
	// jitgentex ===

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot

		//Heffo - Free Cinematic
		if (image->is_cin)
			CIN_FreeCin(image->texnum);

		if (image->rscript) // jitrscript
			image->rscript->img_ptr = NULL;

		// free it
		qgl.DeleteTextures(1, &image->texnum);

		memset(image, 0, sizeof(*image));
	}

	hash_table_free(&gltextures_hash); // jithash
}
