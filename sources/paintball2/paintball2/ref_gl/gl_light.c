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
// r_light.c

#include "gl_local.h"

#ifdef DEBUG
#define DEBUG_FLOATS
#endif

#ifdef DEBUG_FLOATS
#include <float.h> // jitdebug - trying to catch bad floating point numbers in point lights
#endif

int	r_dlightframecount;

extern cvar_t *gl_lightmap_saturation; // jitlight

#define	DLIGHT_CUTOFF	0

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

void R_RenderDlight (dlight_t *light)
{
	int		i, j;
	float	a;
	vec3_t	v;
	float	rad;

	rad = light->intensity * 0.35;

	VectorSubtract (light->origin, r_origin, v);
#if 0
	// FIXME?
	if (VectorLength (v) < rad)
	{	// view is inside the dlight
		V_AddBlend (light->color[0], light->color[1], light->color[2], light->intensity * 0.0003, v_blend);
		return;
	}
#endif

	qgl.Begin(GL_TRIANGLE_FAN);
	qgl.Color3f (light->color[0]*0.2, light->color[1]*0.2, light->color[2]*0.2);
	for (i=0 ; i<3 ; i++)
		v[i] = light->origin[i] - vpn[i]*rad;
	qgl.Vertex3fv (v);
	qgl.Color3f (0,0,0);
	for (i=16 ; i>=0 ; i--)
	{
		a = i*0.39269875;
		for (j=0 ; j<3 ; j++)
			v[j] = light->origin[j] + vright[j]*cos(a)*rad
				+ vup[j]*sin(a)*rad;
		qgl.Vertex3fv (v);
	}
	qgl.End ();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!gl_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	qgl.DepthMask (0);
	qgl.Disable (GL_TEXTURE_2D);
	qgl.ShadeModel (GL_SMOOTH);
	GLSTATE_ENABLE_BLEND
	qgl.BlendFunc (GL_ONE, GL_ONE);

	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
		R_RenderDlight (l);

	qgl.Color3f (1,1,1);
	GLSTATE_DISABLE_BLEND
	qgl.Enable (GL_TEXTURE_2D);
	qgl.BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qgl.DepthMask (1);
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	cplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;
	
	if (node->contents != -1)
		return;

	splitplane = node->plane;
	dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;
	
	if (dist > light->intensity-DLIGHT_CUTOFF)
	{
		R_MarkLights (light, bit, node->children[0]);
		return;
	}
	if (dist < -light->intensity+DLIGHT_CUTOFF)
	{
		R_MarkLights (light, bit, node->children[1]);
		return;
	}
		
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (gl_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
		R_MarkLights ( l, 1<<i, r_worldmodel->nodes );
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/


vec3_t			pointcolor;
vec3_t			lightspot;


// Taken from LordHavoc's DarkPlaces, slightly modified by jitspoe to be compatible with Quake2
static int LightPoint_RecursiveBSPNode (model_t *model, vec3_t ambientcolor, const mnode_t *node, float x, float y, float startz, float endz)
{
	int side;
	float front, back;
	float mid, distz = endz - startz;

#ifdef DEBUG_FLOATS // jitdebug trying to catch bad floating point values..
	_clearfp();
//	_controlfp(0, _MCW_EM);
	_controlfp(_controlfp(0, 0) & ~(_EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW), _MCW_EM);
#endif

loc0:
	//if (!node->plane)
	if (node->contents != -1)
		return false;		// didn't hit anything

	switch (node->plane->type)
	{
	case PLANE_X:
		node = node->children[x < node->plane->dist];
		goto loc0;
	case PLANE_Y:
		node = node->children[y < node->plane->dist];
		goto loc0;
	case PLANE_Z:
		side = startz < node->plane->dist;

		if ((endz < node->plane->dist) == side)
		{
			node = node->children[side];
			goto loc0;
		}

		// found an intersection
		mid = node->plane->dist;
		break;
	default:
		back = front = x * node->plane->normal[0] + y * node->plane->normal[1];
		front += startz * node->plane->normal[2];
		back += endz * node->plane->normal[2];
		side = front < node->plane->dist;

		if ((back < node->plane->dist) == side)
		{
			node = node->children[side];
			goto loc0;
		}

		// found an intersection
		mid = startz + distz * (front - node->plane->dist) / (front - back);
		break;
	}

	// go down front side
	if (node->children[side]->plane && LightPoint_RecursiveBSPNode(model, ambientcolor, node->children[side], x, y, startz, mid))
	{
		return true;	// hit something
	}
	else
	{
		// check for impact on this node
		if (node->numsurfaces)
		{
			unsigned int i;
			int lmwidth, lmheight;
			float ds, dt;
			msurface_t *surface;
			unsigned char *lightmap;
			int maps, line3, size3;
			float dsfrac;
			float dtfrac;
			float w00, w01, w10, w11;

			surface = model->surfaces + node->firstsurface;

			for (i = 0; i < node->numsurfaces; ++i, ++surface)
			{
				if (surface->flags & (SURF_DRAWTURB|SURF_DRAWSKY))
					continue;	// no lightmaps

				// location we want to sample in the lightmap
				ds = ((x * surface->texinfo->vecs[0][0] + y * surface->texinfo->vecs[0][1] + mid * surface->texinfo->vecs[0][2] + surface->texinfo->vecs[0][3]) - surface->texturemins[0]) * 0.0625f;
				dt = ((x * surface->texinfo->vecs[1][0] + y * surface->texinfo->vecs[1][1] + mid * surface->texinfo->vecs[1][2] + surface->texinfo->vecs[1][3]) - surface->texturemins[1]) * 0.0625f;

				if (ds >= 0.0f && dt >= 0.0f) // jit - fix for negative light values
				{
					int dsi = (int)ds;
					int dti = (int)dt;

					lmwidth = ((surface->extents[0] >> 4) + 1);
					lmheight = ((surface->extents[1] >> 4) + 1);

					// is it in bounds?
					if (dsi < lmwidth && dti < lmheight) // jit - fix for black models right on brush splits.
					{
						// calculate bilinear interpolation factors
						// and also multiply by fixedpoint conversion factors
						dsfrac = ds - dsi;
						dtfrac = dt - dti;

						w00 = (1 - dsfrac) * (1 - dtfrac) * (1.0f / 255.0f);
						w01 = (    dsfrac) * (1 - dtfrac) * (1.0f / 255.0f);
						w10 = (1 - dsfrac) * (    dtfrac) * (1.0f / 255.0f);
						w11 = (    dsfrac) * (    dtfrac) * (1.0f / 255.0f);

						// values for pointer math
						line3 = lmwidth * 3;
						size3 = lmwidth * lmheight * 3;

						// look up the pixel
						//lightmap = surface->samples + dti * line3 + dsi * 3;
						lightmap = surface->stain_samples + dti * line3 + dsi * 3; // Note: comment this line out and use the one above if you do not have stainmaps

						// bilinear filter each lightmap style, and sum them
						for (maps = 0; maps < MAXLIGHTMAPS && surface->styles[maps] != 255; maps++)
						{
							VectorMA(ambientcolor, w00, lightmap            , ambientcolor);
							VectorMA(ambientcolor, w01, lightmap + 3        , ambientcolor);
							VectorMA(ambientcolor, w10, lightmap + line3    , ambientcolor);
							VectorMA(ambientcolor, w11, lightmap + line3 + 3, ambientcolor);
							lightmap += size3;
						}

#ifdef DEBUG // jitdebug - check for screwed up floating point values.
						assert(ambientcolor[0] == ambientcolor[0]);
						assert(ambientcolor[1] == ambientcolor[1]);
						assert(ambientcolor[2] == ambientcolor[2]);

						assert(ambientcolor[0] >= 0.0f);
						assert(ambientcolor[1] >= 0.0f);
						assert(ambientcolor[2] >= 0.0f);
#endif
						return true; // success
					}
				}
			}
		}

		// go down back side
		node = node->children[side ^ 1];
		startz = mid;
		distz = endz - startz;
		goto loc0;
	}
}


// Old function -- should be deleted after we've verified there are no bugs with the new one.
static int RecursiveLightPoint (mnode_t *node, vec3_t start, vec3_t end)
{
	float		front, back, frac;
	int			side;
	cplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	//float		sf, tf;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	int			maps;
	int			r;

	if (node->contents != -1)
		return -1;		// didn't hit anything
	
	// calculate mid point

	// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct(start, plane->normal) - plane->dist;
	back = DotProduct(end, plane->normal) - plane->dist;
	side = front < 0.0f;

	if ((back < 0.0f) == side)
		return RecursiveLightPoint(node->children[side], start, end);

	frac = front / (front - back);
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;

	// go down front side	
	r = RecursiveLightPoint(node->children[side], start, mid);

	if (r >= 0)
		return r;		// hit something

	assert((back < 0.0f) != side); // jit - should never happen, so I removed the check.  Added an assert to be sure.

	// check for impact on this node
	VectorCopy(mid, lightspot);
	surf = r_worldmodel->surfaces + node->firstsurface;
	VectorCopy(vec3_origin, pointcolor);

	for (i = 0; i < node->numsurfaces; ++i, ++surf)
	{
		if (surf->flags & (SURF_DRAWTURB|SURF_DRAWSKY)) 
			continue;	// no lightmaps

		tex = surf->texinfo;
		s = DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3];

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;
		lightmap = surf->stain_samples;

		if (lightmap)
		{
			lightmap += 3 * (dt * ((surf->extents[0] >> 4) + 1) + ds);

			for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
			{
				pointcolor[0] += (float)lightmap_gammatable[max(lightmap[0] - 1, 0)] / 255.0f; // jitgamma:
				pointcolor[1] += (float)lightmap_gammatable[max(lightmap[1] - 1, 0)] / 255.0f;
				pointcolor[2] += (float)lightmap_gammatable[max(lightmap[2] - 1, 0)] / 255.0f;
				lightmap += 3 * ((surf->extents[0] >> 4) + 1) * ((surf->extents[1] >> 4) + 1);
			}
		}

		return 1;
	}

	// go down back side
	return RecursiveLightPoint(node->children[!side], mid, end);
}

/*
===============
R_LightPoint
===============
*/
void R_LightPoint (vec3_t p, vec3_t color) // jitodo -- light points on average of 4 or so points, rather than just straight down.
{
	float		r;
	int			lnum;
	dlight_t	*dl;
	float		light;
	vec3_t		dist;
	float		add;

	if (!r_worldmodel || !r_worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 1.0f;
		return;
	}
	
	if (r_oldlightpoint->value)
	{	
		vec3_t end;

		end[0] = p[0];
		end[1] = p[1];
		end[2] = p[2] - 2048.0f;
		r = RecursiveLightPoint(r_worldmodel->nodes, p, end);
	}
	else
	{
		int i;

		VectorClear(pointcolor);
		r = LightPoint_RecursiveBSPNode(r_worldmodel, pointcolor, r_worldmodel->nodes, p[0], p[1], p[2], p[2] - 2048.0f);

		for (i = 0; i < 3; ++i)
		{
			pointcolor[i] = pow(pointcolor[i], gl_lightmapgamma->value);
		}
	}

	if (r <= 0)
	{
		VectorCopy(vec3_origin, color);
	}
	else
	{
		// ===[
		// jitlight - adjust lightmap saturation for point entities
		register float r, g, b, a, v;

		r = pointcolor[0];
		g = pointcolor[1];
		b = pointcolor[2];
		a = r * 0.33f + g * 0.34f + b * 0.33f; // greyscale value
		v = gl_lightmap_saturation->value;
		color[0] = r * v + a * (1.0f - v);
		color[1] = g * v + a * (1.0f - v);
		color[2] = b * v + a * (1.0f - v);
		// ]===
	}

	// add dynamic lights
	light = 0;
	dl = r_newrefdef.dlights;

	for (lnum = 0; lnum < r_newrefdef.num_dlights; ++lnum, ++dl)
	{
		VectorSubtract(currententity->origin, dl->origin, dist);
		add = dl->intensity - VectorLength(dist);
		add *= 0.00390625f;

		if (add > 0)
			VectorMA(color, add, dl->color, color);
	}

#ifdef DEBUG // jitdebug - check for screwed up floating point values.
	assert(color[0] == color[0]);
	assert(color[1] == color[1]);
	assert(color[2] == color[2]);
#endif

	//VectorScale (color, gl_modulate->value, color);
	//VectorScale (color, gl_modulate->value/1.5f, color); // jit, ents too bright
}


//===================================================================

void R_StainNode (stain_t *st, mnode_t *node)
{ 
	msurface_t *surf;
	float		dist;
	int			c;
	
	if (node->contents != -1)
		return;

	dist = DotProduct(st->origin, node->plane->normal) - node->plane->dist;

	if (dist > st->size)
	{
		R_StainNode(st, node->children[0]);
		return;
	}

	if (dist < -st->size)
	{
		R_StainNode(st, node->children[1]);
		return;
	}

	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c ; c--, surf++)
	{
		int			i;
		mtexinfo_t	*tex;
		int			sd, td;
		float		fdist, frad, fminlight;
		vec3_t		impact, local;
		int			s, t;
		int			smax, tmax;
		byte		*pfBL;
		float		fsacc, ftacc;
		long		col;

		smax = (surf->extents[0] >> 4) + 1;
		tmax = (surf->extents[1] >> 4) + 1;
		tex = surf->texinfo;

		if ((tex->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP)))
			continue;

		frad = st->size;
		fdist = DotProduct(st->origin, surf->plane->normal) - surf->plane->dist;

		if (surf->flags & SURF_PLANEBACK)
			fdist *= -1.0f;

		frad -= fabs(fdist);
		fminlight = DLIGHT_CUTOFF;	// FIXME: make configurable?

		if (frad < fminlight)
			continue;

		fminlight = frad - fminlight;

		for (i = 0; i < 3; i++)
			impact[i] = st->origin[i] - surf->plane->normal[i] * fdist;

		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		if (!surf->samples)
			return;	// fix crash when no lightmap exists

		pfBL = surf->samples;
		surf->cached_light[0] = 0.0f;

		for (t = 0, ftacc = 0; t < tmax; t++, ftacc += 16)
		{
			td = local[1] - ftacc;

			if (td < 0)
				td = -td;

			for (s = 0, fsacc = 0; s < smax; s++, fsacc += 16, pfBL += 3)
			{
				sd = Q_ftol(local[0] - fsacc);

				if (sd < 0)
					sd = -sd;

				if (sd > td)
					fdist = sd + (td >> 1);
				else
					fdist = td + (sd >> 1);

				if (fdist < fminlight)
				{
					int test;

					for(i = 0; i < 3; i++)
					{
						test = pfBL[i] + ((frad - fdist) * st->color[i]);

						if (test < 255 && test > 0)
						{
							col = pfBL[i] * st->color[i];

							if (col > 255)
								col = 255;

							if (col < 0)
								col=0;

							pfBL[i] = (byte)col;
						}
					}
				}
			}
		}
	}

	R_StainNode(st, node->children[0]);
	R_StainNode(st, node->children[1]);
}

void R_ApplyStains (void)
{
	int i;
	stain_t *st;

	for (i=0, st = r_newrefdef.newstains; i < r_newrefdef.num_newstains; i++, st++)
		R_StainNode(st, r_worldmodel->nodes);
}


//static float s_blocklights[34*34*3];
static float s_blocklights[128*128*3]; // jitlightmap

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights (msurface_t *surf)
{
	int			lnum;
	int			sd, td;
	float		fdist, frad, fminlight;
	vec3_t		impact, local;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	float		*pfBL;
	float		fsacc, ftacc;

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;
	tex = surf->texinfo;

	for (lnum = 0; lnum < r_newrefdef.num_dlights; lnum++)
	{
		if (!(surf->dlightbits & (1 << lnum)))
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[lnum];
		frad = dl->intensity;
		fdist = DotProduct (dl->origin, surf->plane->normal) - surf->plane->dist;
		frad -= fabs(fdist);
		// rad is now the highest intensity on the plane

		fminlight = DLIGHT_CUTOFF;	// FIXME: make configurable?

		if (frad < fminlight)
			continue;

		fminlight = frad - fminlight;

		for (i = 0; i < 3; ++i)
		{
			impact[i] = dl->origin[i] -
					surf->plane->normal[i]*fdist;
		}

		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];
		pfBL = s_blocklights;

		for (t = 0, ftacc = 0; t < tmax; ++t, ftacc += 16)
		{
			td = local[1] - ftacc;

			if (td < 0)
				td = -td;

			for (s = 0, fsacc = 0; s < smax; ++s, fsacc += 16, pfBL += 3)
			{
				sd = Q_ftol(local[0] - fsacc);

				if (sd < 0)
					sd = -sd;

				if (sd > td)
					fdist = sd + (td >> 1);
				else
					fdist = td + (sd >> 1);

				if (fdist < fminlight)
				{
					// jit - dynamic light fix (as if this will ever get used) credit: mSparks
					pfBL[0] += (fminlight - fdist) * dl->color[0];
					pfBL[1] += (fminlight - fdist) * dl->color[1];
					pfBL[2] += (fminlight - fdist) * dl->color[2];
				}
			}
		}
	}
}


/*
** R_SetCacheState
*/
void R_SetCacheState (msurface_t *surf)
{
	int maps;

	for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; ++maps)
	{
		surf->cached_light[maps] = r_newrefdef.lightstyles[surf->styles[maps]].white;
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
	int			smax, tmax;
	int			r, g, b, a, max;
	int			i, j, size;
	byte		*lightmap;
	float		scale[4];
	int			nummaps;
	float		*bl;
	lightstyle_t *style;
	int			monolightmap;
	float		sat; // jitlight

	//if (surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP)) - jitlightmap
	//	ri.Sys_Error(ERR_DROP, "R_BuildLightMap called for non-lit surface");

	// === jitlightmap
	if (((surf->texinfo->flags & SURF_TRANS33) != 0) ^ ((surf->texinfo->flags & SURF_TRANS66) != 0))
		max = 128;
	else
		max = 255;
	// ===

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;
	size = smax*tmax;

	if (size > (sizeof(s_blocklights) >> 4) && surf->samples) // jitlightmap
		ri.Sys_Error(ERR_DROP, "Bad s_blocklights size");

	// === jitlight
	sat = gl_lightmap_saturation->value;

	if (sat < 0.3f)
		sat = 0.3f;

	if (sat > 1.0f)
		sat = 1.0f;
	// ===

// set to full bright if no light data
	if (!surf->samples)
	{
		int maps;

		for (i = 0; i < size*3; i++)
			s_blocklights[i] = 127; // jitlight

		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
			style = &r_newrefdef.lightstyles[surf->styles[maps]];

		goto store;
	}

	// count the # of maps
	for (nummaps = 0; nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255;
		 nummaps++)
		;

	lightmap = surf->samples;

	// add all the lightmaps
	if (nummaps == 1)
	{
		int maps = 0;

		bl = s_blocklights;

		for (i = 0; i < 3; i++)
			scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

		for (i = 0; i < size; i++, bl += 3)
		{
			r = lightmap[i * 3 + 0] * scale[0] - 1;
			g = lightmap[i * 3 + 1] * scale[1] - 1;
			b = lightmap[i * 3 + 2] * scale[2] - 1;
			r = r < 0 ? 0 : r;
			g = g < 0 ? 0 : g;
			b = b < 0 ? 0 : b;

			if (r + g + b > 0)
				r++, g++, b++;

			bl[0] = lightmap_gammatable[r > 255 ? 255 : r]; // jitgamma:
			bl[1] = lightmap_gammatable[g > 255 ? 255 : g]; // jitgamma:
			bl[2] = lightmap_gammatable[b > 255 ? 255 : b]; // jitgamma:

			if (bl[0] + bl[1] + bl[2] > max * 3)
			{
				int m = bl[0];
				register float f;

				if (bl[1] > m)
					m = bl[1];

				if (bl[2] > m)
					m = bl[2];

				f = (float)max / (float)m;
				VectorScale(bl, f, bl);
			}
		}

		lightmap += size*3;		// skip to next lightmap
	}
	else
	{
		int maps;

		memset(s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);

		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		{
			bl = s_blocklights;

			for (i = 0; i < 3; i++)
				scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			for (i = 0; i < size; i++, bl+=3)
			{
				bl[0] += (lightmap[i*3+0]) * scale[0];
				bl[1] += (lightmap[i*3+1]) * scale[1];
				bl[2] += (lightmap[i*3+2]) * scale[2];
			}
			
			lightmap += size*3;		// skip to next lightmap
		}

		bl = s_blocklights;

		for (i = 0; i < size; i++, bl += 3)
		{
			// === jitgamma
			r = bl[0] - 1;
			g = bl[1] - 1;
			b = bl[2] - 1;
			r = r < 0 ? 0 : r;
			g = g < 0 ? 0 : g;
			b = b < 0 ? 0 : b;

			if (r + g + b > 0)
				r++, g++, b++;

			bl[0] = lightmap_gammatable[r > 255 ? 255 : r];
			bl[1] = lightmap_gammatable[g > 255 ? 255 : g];
			bl[2] = lightmap_gammatable[b > 255 ? 255 : b];

			if (bl[0] + bl[1] + bl[2] > max * 3)
			{
				int m = bl[0];
				register float f;

				if (bl[1] > m)
					m = bl[1];

				if (bl[2] > m)
					m = bl[2];

				f = (float)max / (float)m;
				VectorScale(bl, f, bl);
			}
			// jitgamma ===
		}
	}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights (surf);

// put into texture format
store:
	stride -= (smax<<2);
	bl = s_blocklights;

	monolightmap = gl_monolightmap->string[0];

	if (monolightmap == '0')
	{
		for (i = 0; i < tmax; i++, dest += stride)
		{
			for (j = 0; j < smax; j++)
			{
				
				r = Q_ftol(bl[0]);
				g = Q_ftol(bl[1]);
				b = Q_ftol(bl[2]);

				// catch negative lights
				if (r < 0)
					r = 0;

				if (g < 0)
					g = 0;

				if (b < 0)
					b = 0;

				// jitlight -- reduce oversaturation:
				// greyscale value:
				a = (int)((float)r * 0.33f + (float)g * 0.34f + (float)b * 0.33f);
				
				r = r*sat + a*(1.0f-sat);
				g = g*sat + a*(1.0f-sat);
				b = b*sat + a*(1.0f-sat);

				/*
				** determine the brightest of the three color components
				*/
				if (r > g)
					max = r;
				else
					max = g;

				if (b > max)
					max = b;

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				// jitlight -- alpha value set above.
				//a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
				if (max > 255)
				{
					float t = 255.0F / max;

					r = r*t;
					g = g*t;
					b = b*t;
					a = a*t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
			}
		}
	}
	else
	{
		for (i = 0; i < tmax; i++, dest += stride)
		{
			for (j = 0; j < smax; j++)
			{
				r = Q_ftol(bl[0]);
				g = Q_ftol(bl[1]);
				b = Q_ftol(bl[2]);

				// catch negative lights
				if (r < 0)
					r = 0;

				if (g < 0)
					g = 0;

				if (b < 0)
					b = 0;

				/*
				** determine the brightest of the three color components
				*/
				if (r > g)
					max = r;
				else
					max = g;

				if (b > max)
					max = b;

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
				if (max > 255)
				{
					float t = 255.0F / max;

					r = r*t;
					g = g*t;
					b = b*t;
					a = a*t;
				}

				/*
				** So if we are doing alpha lightmaps we need to set the R, G, and B
				** components to 0 and we need to set alpha to 1-alpha.
				*/
				switch ( monolightmap )
				{
				case 'L':
				case 'I':
					r = a;
					g = b = 0;
					break;
				case 'C':
					// try faking colored lighting
					a = 255 - ((r+g+b)*0.3333333);
					r *= a*0.003921568627450980392156862745098; // /255.0;
					g *= a*0.003921568627450980392156862745098; // /255.0;
					b *= a*0.003921568627450980392156862745098; // /255.0;
					break;
				case 'A':
				default:
					r = g = b = 0;
					a = 255 - a;
					break;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)) ? 255 : a; // jitlightmap

				bl += 3;
				dest += 4;
			}
		}
	}
}

