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
// gl_mesh.c: triangle model functions

#include "gl_local.h"

/*
=============================================================

  ALIAS MODELS

=============================================================
*/

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec4_t	s_lerped[MAX_VERTS];
//static	vec3_t	lerped[MAX_VERTS];

vec3_t	shadevector;
float	shadelight[3];

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
extern float	r_avertexnormal_dots[SHADEDOT_QUANT][256];
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anormtab.h"
;

float	*shadedots = r_avertexnormal_dots[0];

void GL_LerpVerts(int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3])
{
	int i;

	//PMM -- added RF_SHELL_DOUBLE, RF_SHELL_HALF_DAM
	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM))
	{
		for (i=0; i < nverts; i++, v++, ov++, lerp+=4)
		{
			float *normal = r_avertexnormals[verts[i].lightnormalindex];

			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0] + normal[0] * POWERSUIT_SCALE;
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1] + normal[1] * POWERSUIT_SCALE;
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2] + normal[2] * POWERSUIT_SCALE; 
		}
	}
	else
	{
		for (i=0; i < nverts; i++, v++, ov++, lerp+=4)
		{
			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0];
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1];
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2];
		}
	}

}

/*
=============
GL_DrawAliasFrameLerp

interpolates between two frames and origins
FIXME: batch lerp all vertexes
=============
*/

void GL_DrawAliasFrameLerpShell (dmdl_t *paliashdr, float backlerp)
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order;
	int		count;
	float	frontlerp;
	float	alpha;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	int		index_xyz;
	float	*lerp;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;

	// PMM - added double shell
	qgl.Disable(GL_TEXTURE_2D);

	frontlerp = 1.0f - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; ++i)
	{
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];
	}

	for (i = 0; i < 3; ++i)
	{
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = backlerp * oldframe->scale[i];
	}

	lerp = s_lerped[0];

	GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;

		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			qgl.Begin(GL_TRIANGLE_FAN);
		}
		else
		{
			qgl.Begin(GL_TRIANGLE_STRIP);
		}

		do
		{
			index_xyz = order[2];
			order += 3;

			qgl.Color4f(shadelight[0], shadelight[1], shadelight[2], alpha);
			qgl.Vertex3fv(s_lerped[index_xyz]);

		} while (--count);

		qgl.End();
	}

	qgl.Enable(GL_TEXTURE_2D);
}


void GL_DrawAliasFrameLerp (dmdl_t *paliashdr, float backlerp)
{
	float 	l;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order, *tmp_order;
	int		count;
	float	frontlerp;
	float	alpha;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i, tmp_count;
	int		index_xyz;
	float	*lerp;
	int		va = 0; float mode;

	rscript_t *rs;
	float	txm,tym;
	rs_stage_t *stage;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

//	glTranslatef (frame->translate[0], frame->translate[1], frame->translate[2]);
//	glScalef (frame->scale[0], frame->scale[1], frame->scale[2]);

	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0f;

	frontlerp = 1.0f - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldframe->translate, move);

	for (i = 0; i < 3; ++i)
	{
		move[i] = backlerp * move[i] + frontlerp * frame->translate[i];
	}

	for (i = 0; i < 3; ++i)
	{
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = backlerp * oldframe->scale[i];
	}

	lerp = s_lerped[0];
	GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv);
	//qgl.EnableClientState(GL_COLOR_ARRAY);
	assert(currententity->skinnum >= 0 && currententity->skinnum < MAX_MD2SKINS);
	rs = (rscript_t*)currententity->model->script[currententity->skinnum];

	if (!rs)
	{
#ifdef BEEFQUAKERENDER // jit3dfx
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			va = 0;

			if (!count)
				break;		// done

			if (count < 0)
			{
				count = -count;
				mode = GL_TRIANGLE_FAN;
			}
			else
			{
				mode = GL_TRIANGLE_STRIP;
			}

			do {
				// texture coordinates come from the draw list
				index_xyz = order[2];
				l = shadedots[verts[index_xyz].lightnormalindex];

				VA_SetElem2(tex_array[va],((float *)order)[0], ((float *)order)[1]);
				VA_SetElem3(vert_array[va],s_lerped[index_xyz][0],s_lerped[index_xyz][1],s_lerped[index_xyz][2]);
				VA_SetElem4(col_array[va],l* shadelight[0], l*shadelight[1], l*shadelight[2], alpha);
				va++;
				order += 3;
			} while (--count);

			qgl.DrawArrays(mode, 0, va);
		}
#else // old q2 code (with shells removed):
		if (gl_vertex_arrays->value)
		{
#if 1
			float colorArray[MAX_VERTS*4];

			qgl.EnableClientState(GL_VERTEX_ARRAY);
			qgl.VertexPointer(3, GL_FLOAT, 16, s_lerped);	// padded for SIMD

			qgl.EnableClientState(GL_COLOR_ARRAY);
			qgl.ColorPointer(3, GL_FLOAT, 0, colorArray);

			//
			// pre light everything
			//
			for (i = 0; i < paliashdr->num_xyz; i++)
			{
				float l = shadedots[verts[i].lightnormalindex];

				colorArray[i*3+0] = l * shadelight[0];
				colorArray[i*3+1] = l * shadelight[1];
				colorArray[i*3+2] = l * shadelight[2];
			}

			if (qgl.LockArraysEXT != 0)
				qgl.LockArraysEXT(0, paliashdr->num_xyz);

			while (1)
			{
				// get the vertex count and primitive type
				count = *order++;

				if (!count)
					break;		// done

				if (count < 0)
				{
					count = -count;
					qgl.Begin(GL_TRIANGLE_FAN);
				}
				else
				{
					qgl.Begin(GL_TRIANGLE_STRIP);
				}

				do
				{
					// texture coordinates come from the draw list
					qgl.TexCoord2f(((float *)order)[0], ((float *)order)[1]);
					index_xyz = order[2];
					order += 3;
					qgl.ArrayElement(index_xyz);
				} while (--count);

				qgl.End();
			}

			if (qgl.UnlockArraysEXT != 0)
				qgl.UnlockArraysEXT();
			
			qgl.DisableClientState(GL_VERTEX_ARRAY); // jit
			qgl.DisableClientState(GL_COLOR_ARRAY); // jit
#else // (temp) testing beefquake code
			while (1)
			{
				// get the vertex count and primitive type
				count = *order++;
				va = 0;

				if (!count)
					break;		// done

				if (count < 0)
				{
					count = -count;
					mode = GL_TRIANGLE_FAN;
				}
				else
				{
					mode = GL_TRIANGLE_STRIP;
				}

				do {
					// texture coordinates come from the draw list
					index_xyz = order[2];
					l = shadedots[verts[index_xyz].lightnormalindex];

					VA_SetElem2(tex_array[va],((float *)order)[0], ((float *)order)[1]);
					VA_SetElem3(vert_array[va],s_lerped[index_xyz][0],s_lerped[index_xyz][1],s_lerped[index_xyz][2]);
					VA_SetElem4(col_array[va],l* shadelight[0], l*shadelight[1], l*shadelight[2], alpha);
					va++;
					order += 3;
				} while (--count);
				qgl.DrawArrays(mode,0,va);
			}
#endif
		}
		else // (!gl_vertex_arrays->value)
		{
			while (1)
			{
				// get the vertex count and primitive type
				count = *order++;

				if (!count)
					break;		// done

				if (count < 0)
				{
					count = -count;
					qgl.Begin(GL_TRIANGLE_FAN);
				}
				else
				{
					qgl.Begin(GL_TRIANGLE_STRIP);
				}

				do
				{
					// texture coordinates come from the draw list
					qgl.TexCoord2f(((float *)order)[0], ((float *)order)[1]);
					index_xyz = order[2];
					order += 3;
					// normals and vertexes come from the frame list
					l = shadedots[verts[index_xyz].lightnormalindex];
					qgl.Color4f(l * shadelight[0], l * shadelight[1], l * shadelight[2], alpha);
					qgl.Vertex3fv(s_lerped[index_xyz]);
				} while (--count);

				qgl.End();
			}
		}
#endif
	}
	else // skin has rscript:
	{
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;

			if (!count)
				break;		// done

			if (count < 0)
			{
				count = -count;
				mode = GL_TRIANGLE_FAN;
			}
			else
			{
				mode = GL_TRIANGLE_STRIP;
			}

			stage = rs->stage;
			tmp_count = count;
			tmp_order = order;

			while (stage)
			{
				count = tmp_count;
				order = tmp_order;
				va = 0;

				if (stage->anim_count)
					GL_Bind(RS_Animate(stage));
				else
					GL_Bind(stage->texture->texnum);

				if (stage->scroll.speedX)
				{
					switch (stage->scroll.typeX)
					{
							case 0:	// static
								txm = rs_realtime*stage->scroll.speedX;
								break;
							case 1:	// sine
								txm = sin(rs_realtime*stage->scroll.speedX);
								break;
							case 2:	// cosine
								txm = cos(rs_realtime*stage->scroll.speedX);
								break;
					}
				}
				else
				{
					txm = 0;
				}

				if (stage->scroll.speedY)
				{
					switch(stage->scroll.typeY)
					{
							case 0:	// static
								tym = rs_realtime*stage->scroll.speedY;
								break;
							case 1:	// sine
								tym = sin(rs_realtime*stage->scroll.speedY);
								break;
							case 2:	// cosine
								tym = cos(rs_realtime*stage->scroll.speedY);
								break;
					}
				}
				else
				{
					tym = 0;
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
						alpha = stage->alphashift.min;
					} 
					else if (stage->alphashift.speed) 
					{
						alpha = sin(rs_realtime * stage->alphashift.speed);

						if (alpha < 0) 
							alpha = -alpha;

						if (alpha > stage->alphashift.max) 
							alpha = stage->alphashift.max;

						if (alpha < stage->alphashift.min) 
							alpha = stage->alphashift.min;
					}
				}
				else
				{
					alpha = 1.0f;
				}

				//if (stage->envmap)
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
#ifdef BEEFQUAKERENDER // jit3dfx
				do
				{
					// texture coordinates come from the draw list
					index_xyz = order[2];

					l = shadedots[verts[index_xyz].lightnormalindex];

					VA_SetElem2(tex_array[va],((float *)order)[0]+txm,
						((float *)order)[1]+tym);
					VA_SetElem3(vert_array[va],s_lerped[index_xyz][0],
						s_lerped[index_xyz][1],s_lerped[index_xyz][2]);
					VA_SetElem4(col_array[va],l* shadelight[0],
						l*shadelight[1], l*shadelight[2], alpha);
					va++;
					order += 3;
				} while (--count);
				qgl.DrawArrays(mode,0,va);

#else
				qgl.Begin(mode);
				do
				{
					// texture coordinates come from the draw list
					qgl.TexCoord2f (((float *)order)[0]+txm,
						((float *)order)[1]+tym);
					index_xyz = order[2];
					order += 3;

					// normals and vertexes come from the frame list
					l = shadedots[verts[index_xyz].lightnormalindex];

					qgl.Color4f (l* shadelight[0], l*shadelight[1], 
						l*shadelight[2], alpha);
					qgl.Vertex3fv (s_lerped[index_xyz]);
				} while (--count);
				qgl.End();
#endif

				GLSTATE_DISABLE_ALPHATEST;
				GLSTATE_DISABLE_BLEND;
				qgl.Color4f(1,1,1,1);
				qgl.BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				GLSTATE_DISABLE_TEXGEN;

				stage=stage->next;
			}
		}
	}

	//qgl.DisableClientState(GL_COLOR_ARRAY);
	//qgl.EnableClientState(GL_TEXTURE_COORD_ARRAY);
}


/*
=============
GL_DrawAliasShadow
=============
*/
extern	vec3_t			lightspot;
extern qboolean have_stencil; 

#define CARMACK_REVERSE

void CastShadowEdge(vec3_t p1, vec3_t p2, vec3_t light, char pass)
{
		vec3_t	v[2];

#ifdef CARMACK_REVERSE
		if (pass==0) {
			qgl.FrontFace(GL_CW);
			qgl.StencilOp(GL_KEEP, GL_INCR, GL_KEEP);
		} else {
			qgl.FrontFace(GL_CCW);
			qgl.StencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		}
#else
		if (pass==0) {
			qgl.FrontFace(GL_CCW);
			qgl.StencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		} else {
			qgl.FrontFace(GL_CW);
			qgl.StencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		}
#endif
		qgl.Begin(GL_QUADS);
			v[0][0]=(p1[0]-light[0])*5;
			v[0][1]=(p1[1]-light[1])*5;
			v[0][2]=(p1[2]-light[2])*5;

			v[1][0]=(p2[0]-light[0])*5;
			v[1][1]=(p2[1]-light[1])*5;
			v[1][2]=(p2[2]-light[2])*5;

			qgl.Vertex3fv((float *)p1);
			qgl.Vertex3fv((float *)p2);
			qgl.Vertex4f(v[1][0],v[1][1],v[1][2],0);
			qgl.Vertex4f(v[0][0],v[0][1],v[0][2],0);
		qgl.End();
}


typedef struct {
	vec3_t		v1;
	vec3_t		v2;
	vec3_t		v3;

	vec3_t		normal;
	
	qboolean	visible;
	qboolean	edge;
} tri_t;

typedef struct {
	vec3_t		v1;
	vec3_t		v2;
} visedge_t;

void GetNormal(vec3_t v1, vec3_t v2, vec3_t v3, vec3_t normal)
{
	float	tempvec[3], tempvec2[3], tempvec3[3];
	
	// Subtract the relevent vectors
	tempvec[0] = v2[0] - v1[0];
	tempvec[1] = v2[1] - v1[1];
	tempvec[2] = v2[2] - v1[2];

	tempvec2[0] = v3[0] - v1[0];
	tempvec2[1] = v3[1] - v1[1];
	tempvec2[2] = v3[2] - v1[2];

	// Use cross-product to get the vector
	normal[0] = (tempvec[1] * tempvec2[2]) - (tempvec[2] * tempvec2[1]);
	normal[1] = (tempvec[2] * tempvec2[0]) - (tempvec[0] * tempvec2[2]);
	normal[2] = (tempvec[0] * tempvec2[1]) - (tempvec[1] * tempvec2[0]);
	
	// Normalize this vector (vec/sqrt(vec.vec)
	tempvec3[0] = (float)- (normal[0]/sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2])));
	tempvec3[1] = (float)- (normal[1]/sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2])));
	tempvec3[2] = (float)- (normal[2]/sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2])));

	normal[0] = tempvec3[0];
	normal[1] = tempvec3[1];
	normal[2] = tempvec3[2];
}

void GL_DrawAliasShadow (dmdl_t *paliashdr, int posenum)
{
	int		*order;
	vec3_t	point;
	float	height, lheight;
	int		count;
//	int		va = 0;
	//float mode;

//	vec3_t v[3];
//	char i,vert;
//	tri_t	triangles[1024];
//	visedge_t visedge[128];

#ifndef SHADOW_VOLUMES
	lheight = currententity->origin[2] - lightspot[2];

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);
	height = -lheight + 0.1f;	// lowered shadows to ground more - MrG

	if ((currententity->origin[2]+height) > currententity->origin[2])
		return;

	if (r_newrefdef.vieworg[2] < (currententity->origin[2] + height))
		return;

	// Stencil shadows - MrG
	if (have_stencil && gl_shadows->value == 2)
	{
		qgl.Enable(GL_STENCIL_TEST);
		qgl.StencilFunc(GL_GREATER, 2, 2);
		qgl.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}
	// End Stencil shadows - MrG

	//qgl.DisableClientState(GL_TEXTURE_COORD_ARRAY);
	//qgl.EnableClientState(GL_COLOR_ARRAY);
	qgl.Disable(GL_TEXTURE_2D);
	qgl.Color4f(0, 0, 0, 0.2f);


	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
	//	va=0;

		if (!count)
			break;		// done.

		if (count < 0)
		{
			count = -count;
			//mode = GL_TRIANGLE_FAN;
			qgl.Begin(GL_TRIANGLE_FAN); // jit
		}
		else
		{
			//mode = GL_TRIANGLE_STRIP;
			qgl.Begin(GL_TRIANGLE_STRIP);
		}

		do
		{
			point[0] = s_lerped[order[2]][0] - shadevector[0]*(s_lerped[order[2]][2]+lheight);
			point[1] = s_lerped[order[2]][1] - shadevector[1]*(s_lerped[order[2]][2]+lheight);
			point[2] = height;

			//VA_SetElem3(vert_array[va],point[0],point[1],point[2]);
			//VA_SetElem4(col_array[va],0, 0, 0, 0.2f);
			qgl.Vertex3fv(point); // jit
			order += 3;
			//va++;
		} while (--count);

		qgl.End (); // jit

		// jitest if (qgl.LockArraysEXT != 0) qgl.LockArraysEXT(0, paliashdr->num_xyz);
		//qgl.DrawArrays(mode,0,va);
		// jitest if (qgl.UnlockArraysEXT != 0) qgl.UnlockArraysEXT();
	}	

	//qgl.EnableClientState(GL_TEXTURE_COORD_ARRAY);
	//qgl.DisableClientState(GL_COLOR_ARRAY);

	if (have_stencil && gl_shadows->value == 2)
	{
		qgl.Disable(GL_STENCIL_TEST); // Stencil shadows - MrG
	}

#else

	v[0][0] = -10; v[0][1] =   0; v[0][2] = 50;
	v[1][0] =  15; v[1][1] =  20; v[1][2] = 50;
	v[2][0] =  15; v[2][1] = -20; v[2][2] = 50;

	qgl.Enable(GL_STENCIL_TEST);
	qgl.TexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	qgl.BlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	qgl.DepthMask(0);
	qgl.DepthFunc(GL_LEQUAL);
	qgl.Enable(GL_STENCIL_TEST);
	qgl.ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	qgl.StencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFL);

	point[0] = currententity->origin[0];
	point[1] = currententity->origin[1];
	point[2] = currententity->origin[2];

//	point[0]=0;
//	point[1]=0;
//	point[2]=80;

	for (i = 0; i < 2; i++)
	{
		for (vert = 0; vert < 2; vert++)
			CastShadowEdge(v[vert+1], v[vert], point, i);

		CastShadowEdge(v[0], v[2], point, i);
	}

	qgl.FrontFace(GL_CCW);
	qgl.ColorMask(1, 1, 1, 1);
	qgl.Color3f(1.0f, 1.0f, 1.0f);
	qgl.Enable(GL_TEXTURE_2D);
	qgl.DepthMask(1);
	qgl.Disable(GL_STENCIL_TEST);
#endif
}

/*
** R_CullAliasModel
*/
static qboolean R_CullAliasModel (vec3_t bbox[8], entity_t *e)
{
	int i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t *pframe, *poldframe;
//	vec3_t		angles;
	vec3_t		tmp; // jitbbox.

	paliashdr = (dmdl_t *)currentmodel->extradata;

	if ((e->frame >= paliashdr->num_frames) || (e->frame < 0))
	{
		ri.Con_Printf(PRINT_ALL, "R_CullAliasModel %s: No such frame %d.\n", 
			currentmodel->name, e->frame);
		e->frame = 0;
	}

	if ((e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0))
	{
		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel %s: No such oldframe %d.\n", 
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = (daliasframe_t *) ((byte *) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->frame * paliashdr->framesize);

	poldframe = (daliasframe_t *) ((byte *) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->oldframe * paliashdr->framesize);

	/*
	** compute axially aligned mins and maxs
	*/
	if (pframe == poldframe)
	{
		for (i = 0; i < 3; i++)
		{
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i]*255;
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i]*255;

			oldmins[i]  = poldframe->translate[i];
			oldmaxs[i]  = oldmins[i] + poldframe->scale[i]*255;

			if (thismins[i] < oldmins[i])
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if (thismaxs[i] > oldmaxs[i])
				maxs[i] = thismaxs[i];
			else
				maxs[i] = oldmaxs[i];
		}
	}
#if 0 // jitbbox
	/*
	** compute a full bounding box
	*/
	for (i = 0; i < 8; i++)
	{
		vec3_t   tmp;

		if (i & 1)
			tmp[0] = mins[0];
		else
			tmp[0] = maxs[0];

		if (i & 2)
			tmp[1] = mins[1];
		else
			tmp[1] = maxs[1];

		if (i & 4)
			tmp[2] = mins[2];
		else
			tmp[2] = maxs[2];

		VectorCopy(tmp, bbox[i]);
	}

	/*
	** rotate the bounding box
	*/
	VectorCopy(e->angles, angles);
	angles[YAW] = -angles[YAW];
	//{ float bleh; bleh = angles[PITCH]; angles[PITCH]=angles[ROLL]; angles[ROLL]=bleh; }
/*angles[ROLL]*=2; jit--- this is all fubar FIXME! jitodo
angles[PITCH]/=2;*/
	AngleVectors(angles, vectors[0], vectors[1], vectors[2]);

	for (i = 0; i < 8; i++)
	{
		vec3_t tmp;

		VectorCopy(bbox[i], tmp);

		bbox[i][0] = DotProduct(vectors[0], tmp);
		bbox[i][1] = -DotProduct(vectors[1], tmp); // jitest
		bbox[i][2] = DotProduct(vectors[2], tmp);

		VectorAdd(e->origin, bbox[i], bbox[i]);
	}
#else // jitbbox -- PROPER bbox rotation.
	AngleVectors(e->angles, vectors[0], vectors[1], vectors[2]);
	VectorSubtract(vec3_origin, vectors[1], vectors[1]); // Angle vectors returns "right" instead of "left"

	for (i = 0; i < 8; i++)
	{
		tmp[0] = ((i & 1) ? mins[0] : maxs[0]);
		tmp[1] = ((i & 2) ? mins[1] : maxs[1]);
		tmp[2] = ((i & 4) ? mins[2] : maxs[2]);

		bbox[i][0] = vectors[0][0] * tmp[0] + vectors[1][0] * tmp[1] + vectors[2][0] * tmp[2] + e->origin[0];
		bbox[i][1] = vectors[0][1] * tmp[0] + vectors[1][1] * tmp[1] + vectors[2][1] * tmp[2] + e->origin[1];
		bbox[i][2] = vectors[0][2] * tmp[0] + vectors[1][2] * tmp[1] + vectors[2][2] * tmp[2] + e->origin[2];
	}
#endif
	{
		int p, f, aggregatemask = ~0;

		for (p = 0; p < 8; p++)
		{
			int mask = 0;

			for (f = 0; f < 4; f++)
			{
				float dp = DotProduct(frustum[f].normal, bbox[p]);

				if ((dp - frustum[f].dist) < 0)
				{
					mask |= (1 << f);
				}
			}

			aggregatemask &= mask;
		}

		if (aggregatemask)
		{
			return true;
		}

		return false;
	}
}

/*
=================
R_DrawAliasModel
=================
*/
void R_DrawAliasModel (entity_t *e)
{
	int			i;
	dmdl_t		*paliashdr;
	float		an;
	vec3_t		bbox[8];
	image_t		*skin;
	extern qboolean g_drawing_refl;
	qboolean counttris = true;

	if (!(e->flags & RF_WEAPONMODEL))
	{
		if (R_CullAliasModel(bbox, e))
			return;
	}
	else //if (e->flags & RF_WEAPONMODEL)
	{
		if (r_lefthand->value == 2 || g_drawing_refl) // jitwater - don't reflect the weapon model
			return;
	}

	paliashdr = (dmdl_t *)currentmodel->extradata;

	//
	// get lighting information
	//
	// PMM - rewrote, reordered to handle new shells & mixing
	// PMM - 3.20 code .. replaced with original way of doing it to keep mod authors happy
	//
	if (currententity->flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE))
	{
		VectorClear (shadelight);

		if (currententity->flags & RF_SHELL_HALF_DAM)
		{
				shadelight[0] = 0.56f;
				shadelight[1] = 0.59f;
				shadelight[2] = 0.45f;
		}

		if (currententity->flags & RF_SHELL_DOUBLE)
		{
			shadelight[0] = 0.9f;
			shadelight[1] = 0.7f;
		}

		if (currententity->flags & RF_SHELL_RED)
			shadelight[0] = 1.0f;

		if (currententity->flags & RF_SHELL_GREEN)
			shadelight[1] = 1.0f;

		if (currententity->flags & RF_SHELL_BLUE)
			shadelight[2] = 1.0f;
	}
	else if (currententity->flags & RF_FULLBRIGHT)
	{
		for (i = 0; i < 3; ++i)
			shadelight[i] = 1.0f;
	}
	else
	{
		R_LightPoint(currententity->origin, shadelight);

		// player lighting hack for communication back to server
		// big hack!
		if (currententity->flags & RF_WEAPONMODEL)
		{
			// pick the greatest component, which should be the same
			// as the mono value returned by software
			if (shadelight[0] > shadelight[1])
			{
				if (shadelight[0] > shadelight[2])
					r_lightlevel->value = 150*shadelight[0];
				else
					r_lightlevel->value = 150*shadelight[2];
			}
			else
			{
				if (shadelight[1] > shadelight[2])
					r_lightlevel->value = 150*shadelight[1];
				else
					r_lightlevel->value = 150*shadelight[2];
			}
		}
		
		if (gl_monolightmap->string[0] != '0')
		{
			float s = shadelight[0];

			if (s < shadelight[1])
				s = shadelight[1];
			if (s < shadelight[2])
				s = shadelight[2];

			shadelight[0] = s;
			shadelight[1] = s;
			shadelight[2] = s;
		}
	}

	if (currententity->flags & RF_MINLIGHT)
	{
		for (i=0; i<3; i++)
			if (shadelight[i] > 0.1)
				break;
		if (i == 3)
		{
			shadelight[0] = 0.1;
			shadelight[1] = 0.1;
			shadelight[2] = 0.1;
		}
	}

	if (currententity->flags & RF_GLOW)
	{	// bonus items will pulse with time
		float	scale;
		float	min;

		scale = 0.1 * sin(r_newrefdef.time*7);
		for (i=0; i<3; i++)
		{
			min = shadelight[i] * 0.8;
			shadelight[i] += scale;
			if (shadelight[i] < min)
				shadelight[i] = min;
		}
	}

// =================
// PGM	ir goggles color override
	if (r_newrefdef.rdflags & RDF_IRGOGGLES && currententity->flags & RF_IR_VISIBLE)
	{
		shadelight[0] = 1.0;
		shadelight[1] = 0.0;
		shadelight[2] = 0.0;
	}
// PGM	
// =================

	shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	
	an = currententity->angles[1] *0.0055555555555555555555555555555556 /* /180 */ *M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);

	//
	// draw all the triangles
	//
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		qgl.DepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
/*
	if ((currententity->flags & RF_WEAPONMODEL) && (r_lefthand->value == 1.0F))
	{
		extern void MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

		qgl.MatrixMode(GL_PROJECTION);
		qgl.PushMatrix();
		qgl.LoadIdentity();
		qgl.Scalef(-1, 1, 1);
	    MYgluPerspective(r_newrefdef.fov_y, (float)r_newrefdef.width / (float)r_newrefdef.height, 4, 4096);
		qgl.MatrixMode(GL_MODELVIEW);
		qgl.CullFace(GL_BACK);
	}
*/
	if ((currententity->flags & RF_WEAPONMODEL) && r_lefthand->value != 2.0f) // jithand
	{
		extern void MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

		qgl.MatrixMode(GL_PROJECTION);
		qgl.PushMatrix();
		qgl.LoadIdentity();

		if (r_lefthand->value == 1.0f)
		{
			qgl.Scalef(-1, 1, 1);
			qgl.CullFace(GL_BACK);
		}

		MYgluPerspective(min(r_newrefdef.fov_y, 75.0f), (float)r_newrefdef.width / (float)r_newrefdef.height, 4.0f, 4096.0f);
		qgl.MatrixMode(GL_MODELVIEW);
	}

    qgl.PushMatrix();
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.
	R_RotateForEntity(e);
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.

	// select skin
	if (currententity->skins[0])
	{
		skin = currententity->skins[0];	// custom player skin
		counttris = false;
	}
	else
	{
		if (((unsigned)currententity->skinnum) >= MAX_MD2SKINS) // jit - negative numbers = bad! (but how did this happen?!)
		{
			assert(0);
			skin = currentmodel->skins[0];
			currententity->skinnum = 0;
		}
		else
		{
			skin = currentmodel->skins[currententity->skinnum];

			if (!skin)
			{
				skin = currentmodel->skins[0];
				currententity->skinnum = 0;
			}
		}
	}

	if (!skin)
		skin = r_notexture;	// fallback...

	if (counttris) // jit
		c_alias_polys += paliashdr->num_tris;

	// draw it
	GL_Bind(skin->texnum);
	qgl.ShadeModel(GL_SMOOTH);
	GL_TexEnv(GL_COMBINE_EXT); // jitbright

	if (currententity->flags & RF_TRANSLUCENT)
	{
		GLSTATE_ENABLE_BLEND
	}

	if ((currententity->frame >= paliashdr->num_frames) 
		|| (currententity->frame < 0))
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: No such frame %d.\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ((currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0))
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: No such oldframe %d.\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if (!r_lerpmodels->value)
		currententity->backlerp = 0;

	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM))
	{
		GL_DrawAliasFrameLerpShell(paliashdr,currententity->backlerp);
	}
	else
	{
		GL_DrawAliasFrameLerp (paliashdr, currententity->backlerp);
	}

	GL_TexEnv(GL_REPLACE);
	qgl.ShadeModel(GL_FLAT);

	qgl.PopMatrix();

	// === jit
	if (gl_showbbox->value && !(currententity->flags & RF_WEAPONMODEL)) // r_drawbbox
	{
		qgl.Disable(GL_TEXTURE_2D);
		qgl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
		qgl.Begin(GL_LINES);
		qgl.Vertex3fv(bbox[0]);
		qgl.Vertex3fv(bbox[1]);
		qgl.Vertex3fv(bbox[0]);
		qgl.Vertex3fv(bbox[2]);
		qgl.Vertex3fv(bbox[2]);
		qgl.Vertex3fv(bbox[3]);
		qgl.Vertex3fv(bbox[3]);
		qgl.Vertex3fv(bbox[1]);
		qgl.Vertex3fv(bbox[0]);
		qgl.Vertex3fv(bbox[4]);
		qgl.Vertex3fv(bbox[4]);
		qgl.Vertex3fv(bbox[5]);
		qgl.Vertex3fv(bbox[4]);
		qgl.Vertex3fv(bbox[6]);
		qgl.Vertex3fv(bbox[5]);
		qgl.Vertex3fv(bbox[7]);
		qgl.Vertex3fv(bbox[6]);
		qgl.Vertex3fv(bbox[7]);
		qgl.Vertex3fv(bbox[7]);
		qgl.Vertex3fv(bbox[3]);
		qgl.Vertex3fv(bbox[6]);
		qgl.Vertex3fv(bbox[2]);
		qgl.Vertex3fv(bbox[5]);
		qgl.Vertex3fv(bbox[1]);
		qgl.End();
		qgl.Enable(GL_TEXTURE_2D);
	}
	// jit ===

	if ((currententity->flags & RF_WEAPONMODEL) && (r_lefthand->value != 2.0F)) // jithand
	{
		qgl.MatrixMode(GL_PROJECTION);
		qgl.PopMatrix();
		qgl.MatrixMode(GL_MODELVIEW);
		qgl.CullFace(GL_FRONT);
	}

	if (currententity->flags & RF_TRANSLUCENT)
	{
		GLSTATE_DISABLE_BLEND
	}

	if (currententity->flags & RF_DEPTHHACK)
		qgl.DepthRange (gldepthmin, gldepthmax);

	if (gl_shadows->value && !(currententity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)))
	{
		qgl.PushMatrix ();

		// Dont rotate shadows on ungodly axis' - MrG
		qgl.Translatef(e->origin[0],  e->origin[1],  e->origin[2]);
		qgl.Rotatef(e->angles[1],  0, 0, 1);
		// End

		qgl.Disable(GL_TEXTURE_2D);
		GLSTATE_ENABLE_BLEND
		GL_DrawAliasShadow(paliashdr, currententity->frame);
		qgl.Enable(GL_TEXTURE_2D);
		GLSTATE_DISABLE_BLEND
		qgl.PopMatrix ();
	}

	qgl.Color4f (1,1,1,1);
}


