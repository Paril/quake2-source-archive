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

// draw.c

#include "gl_local.h"

image_t		*draw_chars = NULL;
byte		*char_colors = NULL; // jittext

extern	qboolean	scrap_dirty;
extern cvar_t *cl_hudscale; //jithudscale
extern cvar_t *cl_crosshairscale; // viciouz - crosshair scale
void Scrap_Upload (void);
extern cvar_t	*gl_textshadow; // jittext

// vertex arrays
float	tex_array[MAX_ARRAY][2];
float	vert_array[MAX_ARRAY][3];
float	col_array[MAX_ARRAY][4];


/*
===============
Draw_InitLocal
===============
*/

void Draw_InitLocal (void)
{
	int width, height;
	void LoadTGA (char *name, byte **pic, int *width, int *height);

	//draw_chars = GL_FindImage("pics/conchars.pcx", it_pic);
	if (ri.Cvar_Get("gl_overbright", "1", CVAR_ARCHIVE)->value && gl_state.texture_combine)
		draw_chars = GL_FindImage("pics/conchars1ovb.tga", it_pic); // dark conchars (brightness doubled)
	else
		draw_chars = GL_FindImage("pics/conchars1.tga", it_pic); // jitconsole

	GL_Bind(draw_chars->texnum);
	//qgl.TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//qgl.TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	LoadTGA("pics/char_colors.tga", &char_colors, &width, &height); // jittext

	if (!char_colors || (width*height != 256))
		Sys_Error("Invalid or missing char_colors.tga.");
}


/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char (float x, float y, int num) // jitodo -- try to remove all calls to this, use draw_string
{
	int				row, col;
	float			frow, fcol, size;
	float textscale;

	textscale = cl_hudscale->value; // jithudscale
	num &= 255;
	
	if ((num & 127) == 32)
		return;		// space

	if (y <= -CHARHEIGHT)
		return;			// totally off screen

	row = num >> 4;
	col = num & 15;
	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625f;
	GLSTATE_DISABLE_ALPHATEST // jitconsole
	GLSTATE_ENABLE_BLEND // jitconsole
	GL_Bind(draw_chars->texnum);

#ifdef BEEFQUAKERENDER // jit3dfx
	VA_SetElem2(tex_array[0],fcol, frow);
	VA_SetElem2(vert_array[0],x, y);
	VA_SetElem2(tex_array[1],fcol + size, frow);
	VA_SetElem2(vert_array[1],x+8*textscale, y);
	VA_SetElem2(tex_array[2],fcol + size, frow + size);
	VA_SetElem2(vert_array[2],x+8*textscale, y+8*textscale);
	VA_SetElem2(tex_array[3],fcol, frow + size);
	VA_SetElem2(vert_array[3],x, y+8*textscale);
	qgl.DrawArrays (GL_QUADS, 0, 4);
#else
	qgl.Begin(GL_QUADS);
	qgl.TexCoord2f(fcol, frow);
	qgl.Vertex2f(x, y);
	qgl.TexCoord2f(fcol + size, frow);
	qgl.Vertex2f(x + CHARWIDTH * textscale, y); // jithudscale...
	qgl.TexCoord2f(fcol + size, frow + size);
	qgl.Vertex2f(x + CHARWIDTH * textscale, y+8*textscale);
	qgl.TexCoord2f(fcol, frow + size);
	qgl.Vertex2f(x, y + 8 * textscale);
	qgl.End();
#endif
}

//#define CHAR_UNDERLINE_NUM 158
#define CHAR_UNDERLINE_NUM 2
#define SHADOW_OFFSET 0.8f

//float redtext[] = { 1.0f, .8f, .5f };
//float whitetext[] = { 1.0f, 1.0f, 1.0f };
void Draw_StringAlpha (float x, float y, const char *str, float alpha) // jit
{
	float			px, py;
	int				row, col, num;
	float			frow, fcol, size;
	const char		*s = str; // jit, shush little warning
	float			textscale; // jithudscale
	int				coloredtext = 0;
	qboolean		nextiscolor = false;
	qboolean		shadowpass = false;
	qboolean		passagain = false;
	qboolean		italicized = false;
	qboolean		underlined = false;

	if (!*s)
		return;

	textscale = cl_hudscale->value;

	if (gl_state.currenttextures[gl_state.currenttmu] != draw_chars->texnum)
		GL_Bind(draw_chars->texnum);

	GLSTATE_DISABLE_ALPHATEST // jitconsole
	GLSTATE_ENABLE_BLEND // jitconsole
	GL_TexEnv(GL_COMBINE_EXT); // jittext (brighter text now)
	size = 0.0625f;

	if (gl_textshadow->value)
	{
		shadowpass = true;
		px = x + textscale * SHADOW_OFFSET;
		py = y + textscale * SHADOW_OFFSET;
	}
	else
	{
		px = x;
		py = y;
	}

	qgl.Begin(GL_QUADS);

	do
	{
		if (shadowpass)
			qgl.Color4f(0.0f, 0.0f, 0.0f, alpha * gl_textshadow->value);
		else
			qgl.Color4f(1.0f, 1.0f, 1.0f, alpha);

		while (*s)
		{
			num = *s;
			num &= 255;

			// ===[
			// jittext
			if (!nextiscolor)
			{
				switch (num)
				{
				case UCHAR_COLOR:

					if (!(*(s+1))) // end of string
					{
						nextiscolor = false;
					}
					else
					{
						nextiscolor = true;
						coloredtext = 1;
						s++;
						continue;
					}

					break;
				case UCHAR_UNDERLINE:
					s++;
					underlined = !underlined;

					//if (*s != '\0') // only draw if at end of string
						continue;
					//else // so the string null-terminates!
					//	s--;

					break;
				case UCHAR_ITALICS:
					s++;
					italicized = !italicized;

					//if (*s != '\0') // only draw if at end of string
						continue;
					//else
					//	s--;

					break;
				case UCHAR_ENDFORMAT:
					s++;
					italicized = false;
					underlined = false;

					if (!shadowpass)
						qgl.Color4f(1.0f, 1.0f, 1.0f, alpha);

					//if (*s != '\0') // draw character if at end of string.
						continue;
					//else
					//	s--;

					break;
				default:
					break;
				}
			}
			else // (nextiscolor)
			{
				// look up color in char_colors.tga:
				if (!shadowpass)
				{
					register int num4 = num * 4;

					qgl.Color4f(*(char_colors + num4) / 255.0f, 
						*(char_colors + num4 + 1) / 255.0f,
						*(char_colors + num4 + 2) / 255.0f, alpha);
				}

				nextiscolor = false;
				s++;
				continue;
			}
			// ]==

			if (py <= -CHARWIDTH * textscale)
			{	// totally off screen
				s++;
				px += CHARWIDTH * textscale; //jithudscale
				continue;
			}

			if (underlined) // jitconsole
			{
				row = CHAR_UNDERLINE_NUM >> 4;
				col = CHAR_UNDERLINE_NUM & 15;
				frow = row * 0.0625f;
				fcol = col * 0.0625f;
				qgl.TexCoord2f(fcol, frow);
				qgl.Vertex2f(px, py + 4.0f * textscale);
				qgl.TexCoord2f(fcol + size, frow);
				qgl.Vertex2f(px + CHARWIDTH * textscale, py + 4.0f * textscale); // jithudscale...
				qgl.TexCoord2f(fcol + size, frow + size);
				qgl.Vertex2f(px + CHARWIDTH * textscale, py + 12.0f * textscale);
				qgl.TexCoord2f(fcol, frow + size);
				qgl.Vertex2f(px, py + 12.0f * textscale);
			}

			if ((num & 127) == 32)		// space
			{
				++s;
				px += CHARWIDTH * textscale; //jithudscale
				continue;
			}
			else if (num == '\n') // newline -- jittext
			{
				++s;
				px = x;

				if (shadowpass)
					px += SHADOW_OFFSET * textscale;

				py += CHARHEIGHT * textscale;
				continue;
			}

			row = num >> 4;
			col = num & 15;
			frow = row * 0.0625f;
			fcol = col * 0.0625f;

			if (italicized)
			{
				qgl.TexCoord2f(fcol, frow);
				qgl.Vertex2f(px + 2.0f * textscale, py);
				qgl.TexCoord2f(fcol + size, frow);
				qgl.Vertex2f(px + (2 + CHARWIDTH) * textscale, py); // jithudscale...
				qgl.TexCoord2f(fcol + size, frow + size);
				qgl.Vertex2f(px + (CHARWIDTH - 2) * textscale, py + 8.0f * textscale);
				qgl.TexCoord2f(fcol, frow + size);
				qgl.Vertex2f(px - 2.0f * textscale, py + 8.0f * textscale);
			}
			else
			{
				qgl.TexCoord2f(fcol, frow);
				qgl.Vertex2f(px, py);
				qgl.TexCoord2f(fcol + size, frow);
				qgl.Vertex2f(px + CHARWIDTH * textscale, py); // jithudscale...
				qgl.TexCoord2f(fcol + size, frow + size);
				qgl.Vertex2f(px + CHARWIDTH * textscale, py + 8.0f * textscale);
				qgl.TexCoord2f(fcol, frow + size);
				qgl.Vertex2f(px, py + 8.0f * textscale);
			}

			s++;
			px += CHARWIDTH * textscale; //jithudscale
		}

		if (shadowpass)
		{
			nextiscolor = false;
			italicized = false;
			underlined = false;
			shadowpass = false;
			passagain = true;
			s = str;
			px = x;
			py = y;
		}
		else
		{
			passagain = false;
		}
	} while(passagain);

	qgl.End();

	if (coloredtext || alpha < 1.0f)
	{
		qgl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	GL_TexEnv(GL_MODULATE); // jittext
}


void Draw_String (float x, float y, const char *str)
{
	Draw_StringAlpha(x, y, str, 1.0f); // jit
}


/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (const char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf(fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage(fullname, it_pic);
	}
	else
	{
		gl = GL_FindImage(name + 1, it_pic);
	}

	if (strstr(fullname, "pics/ch") && strchr("0123456789", fullname[7]) && (strlen(fullname) <= 15)) // still a bit ambiguous
		gl->is_crosshair = true;

	if (!gl) // jit -- remove "can't find pic" spam
		return r_notexture;
	else
		return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, const char *pic)
{
	image_t *gl;

	if (!(gl = Draw_FindPic(pic)))
	{
		*w = *h = -1;
		return;
	}

	*w = gl->width;
	*h = gl->height;
}


/*
=============
Draw_StretchPic
=============
*/
void RS_SetTexcoords2D (rs_stage_t *stage, float *os, float *ot); // jit
void Draw_StretchPic2 (float x, float y, float w, float h, image_t *gl)
{
	rscript_t *rs;
	float	txm,tym, alpha,s,t;
	rs_stage_t *stage;

	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "NULL pic in Draw_StretchPic.\n");
		return;
	}

	if (scrap_dirty)
		Scrap_Upload();

	if (((gl_config.renderer == GL_RENDERER_MCD) ||
		(gl_config.renderer & GL_RENDERER_RENDITION)) && !gl->has_alpha)
	{
		GLSTATE_DISABLE_ALPHATEST
	}

	/*rs = RS_FindScript(gl->name); // jitrscript
	if (!rs)
		rs = RS_FindScript(gl->bare_name);*/
	rs = gl->rscript; // jitrscript


	if (!rs) 
	{
		//GLSTATE_ENABLE_ALPHATEST //jitodo / jitmenu - reenable this after rscripts for menu stuff are made.
		GLSTATE_DISABLE_ALPHATEST // jitodo (see above)
		GLSTATE_ENABLE_BLEND // jitodo (see above)
		qgl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_Bind(gl->texnum);

		qgl.Begin(GL_QUADS);
		qgl.TexCoord2f(gl->sl, gl->tl);
		qgl.Vertex2f(x, y);
		qgl.TexCoord2f(gl->sh, gl->tl);
		qgl.Vertex2f(x + w, y);
		qgl.TexCoord2f(gl->sh, gl->th);
		qgl.Vertex2f(x + w, y + h);
		qgl.TexCoord2f(gl->sl, gl->th);
		qgl.Vertex2f(x, y + h);
		qgl.End();
	}
	else
	{
		image_t *stage_pic; // jitrscript
		image_t *RS_Animate_image (rs_stage_t *stage); // jitrscript

		if (!rs->ready) // jit
			RS_ReadyScript(rs);

		stage = rs->stage;

		while (stage)
		{
			if (stage->anim_count)
			//	GL_Bind(RS_Animate(stage));
				stage_pic = RS_Animate_image(stage); // jitrscript
			else
			//	GL_Bind(stage->texture->texnum);
				stage_pic = stage->texture; // jitrscript

			GL_Bind(stage_pic->texnum);

			if (stage->scroll.speedX)
			{
				switch(stage->scroll.typeX)
				{
					case 0:	// static
						txm=rs_realtime*stage->scroll.speedX;
						break;
					case 1:	// sine
						txm=sin(rs_realtime*stage->scroll.speedX);
						break;
					case 2:	// cosine
						txm=cos(rs_realtime*stage->scroll.speedX);
						break;
				}
			}
			else
			{
				txm=0;
			}

			if (stage->scroll.speedY)
			{
				switch(stage->scroll.typeY)
				{
					case 0:	// static
						tym=rs_realtime*stage->scroll.speedY;
						break;
					case 1:	// sine
						tym=sin(rs_realtime*stage->scroll.speedY);
						break;
					case 2:	// cosine
						tym=cos(rs_realtime*stage->scroll.speedY);
						break;
				}
			}
			else
			{
				tym=0;
			}

			if (stage->blendfunc.blend)
			{
				qgl.BlendFunc(stage->blendfunc.source,stage->blendfunc.dest);
				GLSTATE_ENABLE_BLEND
			}
			else
			{
				GLSTATE_DISABLE_BLEND
			}

			if (stage->alphashift.min || stage->alphashift.speed)
			{
				if (!stage->alphashift.speed && stage->alphashift.min > 0)
				{
					alpha=stage->alphashift.min;
				}
				else if (stage->alphashift.speed)
				{
					alpha = sin(rs_realtime * stage->alphashift.speed);
					// jit alpha=(alpha+1)*0.5f;
					alpha = 0.5f * (alpha + 1) * 
						(stage->alphashift.max - stage->alphashift.min) +
						stage->alphashift.min; // jitrscript
					// jit if (alpha > stage->alphashift.max) alpha=stage->alphashift.max;
					// jit if (alpha < stage->alphashift.min) alpha=stage->alphashift.min;
				}
			}
			else
			{
				alpha=1.0f;
			}

			qgl.Color4f(1,1,1,alpha);
			GL_TexEnv(GL_MODULATE); // jitrscript

//			if (stage->envmap)
			if (stage->tcGen == TC_GEN_ENVIRONMENT)
			{
				qgl.TexGenf(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
				qgl.TexGenf(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
				GLSTATE_ENABLE_TEXGEN
			}

			if (stage->alphamask)
			{
				GLSTATE_ENABLE_ALPHATEST
			}
			else
			{
				GLSTATE_DISABLE_ALPHATEST
			}

			qgl.Begin(GL_QUADS);

			s = stage_pic->sl;//0.0f;//gl->sl; //
			t = stage_pic->tl;//0.0f;//gl->tl; //
			RS_SetTexcoords2D(stage, &s, &t);
			qgl.TexCoord2f(s+txm, t+tym);
			qgl.Vertex2f(x, y);

			s = stage_pic->sh;//1.0f;//gl->sh; //
			t = stage_pic->tl;//0.0f;//gl->tl;//
			RS_SetTexcoords2D(stage, &s, &t);
			qgl.TexCoord2f(s+txm, t+tym);
			qgl.Vertex2f(x+w, y);

			s = stage_pic->sh;//1.0f;//gl->sh;//
			t = stage_pic->th;//1.0f;//gl->th;//
			RS_SetTexcoords2D(stage, &s, &t);
			qgl.TexCoord2f(s+txm, t+tym);
			qgl.Vertex2f(x+w, y+h);

			s = stage_pic->sl;//0.0f;//gl->sl;//
			t = stage_pic->th;//1.0f;//gl->th;//
			RS_SetTexcoords2D(stage, &s, &t);
			qgl.TexCoord2f(s+txm, t+tym);
			qgl.Vertex2f(x, y+h);

			qgl.End();

			//jit qgl.Color4f(1,1,1,1);
			//jit qgl.BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			GLSTATE_DISABLE_TEXGEN

			stage=stage->next;
		}
		qgl.Color4f(1,1,1,1);
		qgl.BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLSTATE_ENABLE_ALPHATEST
		GLSTATE_DISABLE_BLEND
	}

	/*jit if ( ( ( gl_config.renderer == GL_RENDERER_MCD ) || ( gl_config.renderer & GL_RENDERER_RENDITION ) ) && !gl->has_alpha) {
		GLSTATE_ENABLE_ALPHATEST
	}*/
}


void Draw_StretchPic (float x, float y, float w, float h, const char *pic)
{
	image_t *gl;

	gl = Draw_FindPic(pic);

	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	Draw_StretchPic2(x, y, w, h, gl);
}


/*
=============
Draw_Pic
=============
*/

void Draw_Pic2 (float x, float y, image_t *gl)
{
// jit - no sense in doing all the exact same stuff twice -- just call stretchpic.
	float picscale;

	picscale = cl_hudscale->value; // jithudscale

	if (gl->is_crosshair) // find a better way to do this
		picscale = cl_crosshairscale->value; // viciouz - crosshair scale

	Draw_StretchPic2(x, y, gl->width * picscale, gl->height * picscale, gl);
}

void Draw_Pic (float x, float y, const char *pic)
{
	image_t *gl;

	gl = Draw_FindPic(pic);

	if (!gl)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	Draw_Pic2(x, y, gl);
}

void Draw_SubPic (float x, float y, float w, float h, float tx1, float ty1, float tx2, float ty2, image_t *image) // jit
{
	// todo.
}

void Draw_BorderedPic (bordered_pic_data_t *data, float x, float y, float w, float h, float scale, float alpha)
{
	// TODO: Support rscripts?
	image_t *gl = data->image;
	int i;
	float xdiff = 0.0f, ydiff = 0.0f;
	float ytotal, xtotal;

	if (!gl)
	{
		gl = r_notexture;

		if (!gl)
		{
			ri.Con_Printf(PRINT_ALL, "NULL pic in Draw_BorderedPic.\n");
			return;
		}
	}

	ytotal =
		(data->screencoords[2][3] - data->screencoords[2][1] +
		data->screencoords[3][3] - data->screencoords[3][1] +
		data->screencoords[4][3] - data->screencoords[4][1]) * scale;

	if (ytotal < h)
		ydiff = h - ytotal;

	xtotal =
		(data->screencoords[0][2] - data->screencoords[0][0] +
		data->screencoords[1][2] - data->screencoords[1][0] +
		data->screencoords[2][2] - data->screencoords[2][0]) * scale;

	if (xtotal < w)
		xdiff = w - xtotal;

	GLSTATE_DISABLE_ALPHATEST;
	GLSTATE_ENABLE_BLEND;
	qgl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (alpha < 1.0f)
		qgl.Color4f(1.0f, 1.0f, 1.0f, alpha);

	GL_Bind(gl->texnum);

	qgl.Begin(GL_QUADS);

	// bordered pics are drawn in a spiral from upper left and around clockwise.
	for (i = 0; i < BORDERED_PIC_COORD_COUNT; ++i)
	{
		float xstretch = 0.0f, ystretch = 0.0f;
		float xoff = 0.0f, yoff = 0.0f;

		if (i > 1 && i < 5) // right 3 parts
			xoff = xdiff;

		if (i > 3 && i < 7) // bottom 3 parts
			yoff = ydiff;

		// odd indexes (areas between the corners) get stretched to fit.
		if (i & 1)
		{
			if (i & 2) // every other one gets stretched vertically.
				ystretch = ydiff;
			else
				xstretch = xdiff;
		}

		if (i == 8) // last one gets stretched both ways
		{
			xstretch = xdiff;
			ystretch = ydiff;
		}

		qgl.TexCoord2f(data->texcoords[i][0], data->texcoords[i][1]);
		qgl.Vertex2f(x + xoff + data->screencoords[i][0] * scale, y + yoff + data->screencoords[i][1] * scale);
		qgl.TexCoord2f(data->texcoords[i][2], data->texcoords[i][1]);
		qgl.Vertex2f(x + xoff + data->screencoords[i][2] * scale + xstretch, y + yoff + data->screencoords[i][1] * scale);
		qgl.TexCoord2f(data->texcoords[i][2], data->texcoords[i][3]);
		qgl.Vertex2f(x + xoff + data->screencoords[i][2] * scale + xstretch, y + yoff + data->screencoords[i][3] * scale + ystretch);
		qgl.TexCoord2f(data->texcoords[i][0], data->texcoords[i][3]);
		qgl.Vertex2f(x + xoff + data->screencoords[i][0] * scale, y + yoff + data->screencoords[i][3] * scale + ystretch);
	}

	qgl.End();

	if (alpha < 1.0f)
		qgl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear2 (int x, int y, int w, int h, image_t *image)
{
	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "NULL pic in Draw_TileClear.\n");
		return;
	}

	if (((gl_config.renderer == GL_RENDERER_MCD) ||
		(gl_config.renderer & GL_RENDERER_RENDITION)) && !image->has_alpha)
	{
		GLSTATE_DISABLE_ALPHATEST
	}

	GL_Bind(image->texnum);

	qgl.Begin(GL_QUADS);
	qgl.TexCoord2f(x/256.0f, y/128.0f); // jit -- new pic dimensions
	qgl.Vertex2f(x, y);
	qgl.TexCoord2f((x+w)/256.0f, y/128.0f);
	qgl.Vertex2f(x+w, y);
	qgl.TexCoord2f((x+w)/256.0f, (y+h)/128.0f);
	qgl.Vertex2f(x+w, y+h);
	qgl.TexCoord2f(x/256.0f, (y+h)/128.0f);
	qgl.Vertex2f(x, y+h);
	qgl.End();

	if (((gl_config.renderer == GL_RENDERER_MCD) ||
		(gl_config.renderer & GL_RENDERER_RENDITION)) && !image->has_alpha)
	{
		GLSTATE_ENABLE_ALPHATEST
	}
}

void Draw_TileClear (int x, int y, int w, int h, const char *pic)
{
	image_t	*image;

	image = Draw_FindPic(pic);

	if (!image)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	Draw_TileClear2(x,y,w,h,image);
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/

void Draw_Fill (int x, int y, int w, int h, int c)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	if ((unsigned)c > 255)
		ri.Sys_Error(ERR_FATAL, "Draw_Fill: bad color");

	qgl.Disable(GL_TEXTURE_2D);
	color.c = d_8to24table[c];
	qgl.Color3f(color.v[0] / 255.0f, color.v[1] / 255.0f, color.v[2] / 255.0f);
	qgl.Begin(GL_QUADS);
	qgl.Vertex2f(x, y);
	qgl.Vertex2f(x + w, y);
	qgl.Vertex2f(x + w, y + h);
	qgl.Vertex2f(x, y + h);
	qgl.End();
	qgl.Color3f(1.0f, 1.0f, 1.0f);
	qgl.Enable(GL_TEXTURE_2D);
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	GLSTATE_DISABLE_ALPHATEST
	GLSTATE_ENABLE_BLEND
	qgl.Disable (GL_TEXTURE_2D);
	qgl.Color4f (0, 0, 0, 0.5);

	VA_SetElem2(vert_array[0],0,0);
	VA_SetElem2(vert_array[1],vid.width, 0);
	VA_SetElem2(vert_array[2],vid.width, vid.height);
	VA_SetElem2(vert_array[3],0, vid.height);
	qgl.DrawArrays (GL_QUADS, 0, 4);

	qgl.Color4f (1,1,1,1);
	qgl.Enable (GL_TEXTURE_2D);
	GLSTATE_DISABLE_BLEND
	GLSTATE_ENABLE_ALPHATEST
}


//====================================================================


/*
=============
Draw_StretchRaw
=============
*/
extern unsigned	r_rawpalette[256];

void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	unsigned	image32[256*256];
	unsigned char image8[256*256];
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;

	GL_Bind(0);

	if (rows <= 256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows * 0.00390625; // /256.0;
		trows = 256;
	}

	t = rows * hscale * 0.00390625;// / 256;

	if (!qgl.ColorTableEXT)
	{
		unsigned *dest;

		for (i = 0; i < trows; i++)
		{
			row = (int)(i * hscale);

			if (row > rows)
				break;

			source = data + cols*row;
			dest = &image32[i*256];
			fracstep = cols * 0x10000 / 256;
			frac = fracstep >> 1;

			for (j = 0; j < 256; j++)
			{
				dest[j] = r_rawpalette[source[frac>>16]];
				frac += fracstep;
			}
		}

		qgl.TexImage2D(GL_TEXTURE_2D, 0, gl_tex_solid_format, 256,
			256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	}
	else
	{
		unsigned char *dest;

		for (i = 0; i < trows; i++)
		{
			row = (int)(i * hscale);

			if (row > rows)
				break;

			source = data + cols * row;
			dest = &image8[i * 256];
			fracstep = cols * 0x10000 / 256;
			frac = fracstep >> 1;

			for (j = 0; j < 256; j++)
			{
				dest[j] = source[frac >> 16];
				frac += fracstep;
			}
		}

		qgl.TexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, 256,
			256, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, image8);
	}

	qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qgl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ((gl_config.renderer == GL_RENDERER_MCD) ||
		(gl_config.renderer & GL_RENDERER_RENDITION))
	{
		GLSTATE_DISABLE_ALPHATEST
	}

	qgl.Begin(GL_QUADS);
	qgl.TexCoord2f(0, 0);
	qgl.Vertex2f(x, y);
	qgl.TexCoord2f(1, 0);
	qgl.Vertex2f(x+w, y);
	qgl.TexCoord2f(1, t);
	qgl.Vertex2f(x+w, y+h);
	qgl.TexCoord2f(0, t);
	qgl.Vertex2f(x, y+h);
	qgl.End();

	if ((gl_config.renderer == GL_RENDERER_MCD) ||
		(gl_config.renderer & GL_RENDERER_RENDITION))
	{
		GLSTATE_ENABLE_ALPHATEST
	}
}
