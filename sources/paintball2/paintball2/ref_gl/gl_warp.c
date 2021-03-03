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
// gl_warp.c -- sky and water polygons

#include "gl_local.h"
#include "gl_refl.h" // jitwater / MPO

extern	model_t	*loadmodel;
extern	image_t	**g_refl_images;

char	skyname[MAX_QPATH];
float	skyrotate;
vec3_t	skyaxis;
image_t	*sky_images[6];

extern vec3_t fogcolor; // jitfog...
extern float fogdensity;
extern float fogdistance;
extern qboolean fogenabled;

msurface_t	*warpface;

#define	SUBDIVIDE_SIZE	64 // todo - make this a cvar.
//#define	SUBDIVIDE_SIZE	1024

void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
	int		i, j;
	float	*v;

	mins[0] = mins[1] = mins[2] = 9999;
	maxs[0] = maxs[1] = maxs[2] = -9999;
	v = verts;
	for (i=0; i<numverts; i++)
		for (j=0; j<3; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
}

void SubdividePolygon (int numverts, float *verts)
{
	int		i, j, k;
	vec3_t	mins, maxs;
	float	m;
	float	*v;
	vec3_t	front[64], back[64];
	int		f, b;
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;
	vec3_t	total;
	float	total_s, total_t;

	if (numverts > 60)
		ri.Sys_Error(ERR_DROP, "numverts = %i", numverts);

	BoundPoly(numverts, verts, mins, maxs);

	for (i = 0; i < 3; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5f;
		m = SUBDIVIDE_SIZE * floor(m / SUBDIVIDE_SIZE + 0.5f);

		if (maxs[i] - m < 8)
			continue;

		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;

		for (j = 0; j < numverts; j++, v += 3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v -= i;
		VectorCopy (verts, v);
		f = b = 0;
		v = verts;

		for (j = 0; j < numverts; j++, v += 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}

			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}

			if (dist[j] == 0 || dist[j+1] == 0)
				continue;

			if ((dist[j] > 0) != (dist[j+1] > 0))
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);

				for (k = 0; k < 3; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);

				f++;
				b++;
			}
		}

		SubdividePolygon(f, front[0]);
		SubdividePolygon(b, back[0]);
		return;
	}

	// add a point in the center to help keep warp valid
	poly = Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * VERTEXSIZE*sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts+2;
	VectorClear (total);
	total_s = 0;
	total_t = 0;
	for (i=0; i<numverts; i++, verts+= 3)
	{
		VectorCopy (verts, poly->verts[i+1]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);

		total_s += s;
		total_t += t;
		VectorAdd (total, verts, total);

		poly->verts[i+1][3] = s;
		poly->verts[i+1][4] = t;
	}

	VectorScale (total, (1.0/numverts), poly->verts[0]);
	poly->verts[0][3] = total_s/numverts;
	poly->verts[0][4] = total_t/numverts;

	// copy first vertex to last
	memcpy (poly->verts[i+1], poly->verts[1], sizeof(poly->verts[0]));
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (msurface_t *fa)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = fa;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0; i<fa->numedges; i++)
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdividePolygon (numverts, verts[0]);
}

//=========================================================



// speed up sin calculations - Ed
float	r_turbsin[] =
{
	#include "warpsin.h"
};
#define TURBSCALE (256.0f / (2.0f * (float)M_PI))
#define TURBOSCALE (256.0f / ((float)M_PI / 4.0f)) // jitwater / dukey


/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/

void EmitWaterPolys_original (msurface_t *fa) // jitwater - old code
{
	glpoly_t	*p, *bp;
	float		*v;
	int			i;
	float		s, t;
	float		scroll;
	float		rdt = r_newrefdef.time;

	if (fa->texinfo->flags & SURF_FLOWING)
		scroll = -64 * ((r_newrefdef.time*0.5f) - (int)(r_newrefdef.time*0.5f));
	else
		scroll = 0;

	for (bp=fa->polys; bp; bp=bp->next)
	{
		p = bp;

		qgl.Begin(GL_TRIANGLE_FAN);

		for (i=0, v=p->verts[0]; i<p->numverts; i++, v+=VERTEXSIZE)
		{
#if !id386
			s = v[3] + r_turbsin[(int)((v[4]*0.125f+r_newrefdef.time) * TURBSCALE) & 255];
			t = v[4] + r_turbsin[(int)((v[3]*0.125f+rdt) * TURBSCALE) & 255];
#else
			s = v[3] + r_turbsin[Q_ftol(((v[4]*0.125f+rdt) * TURBSCALE)) & 255];
			t = v[4] + r_turbsin[Q_ftol(((v[3]*0.125f+rdt) * TURBSCALE)) & 255];
#endif
			s += scroll;
			s *= 0.015625;	// divide by empatpuluh enam
			t *= 0.015625;	// ditto
							// dont we love multilingual comments? :)

			qgl.TexCoord2f(s,t);
	 		qgl.Vertex3fv(v);
		}

		qgl.End();
	}
}

float CalcWave (float x, float y) // jitwater / MPO
{
	return (r_turbsin[(int)((x*3+r_newrefdef.time) * TURBOSCALE) & 255] / 2.0f) +
		(r_turbsin[(int)((y*5+r_newrefdef.time) * TURBOSCALE) & 255] / 2.0f);
}

// === jitwater
extern image_t *distort_tex;
extern image_t *water_normal_tex;
void R_GetReflTexScale (float *w, float *h);
// MPO : this is my version...
void EmitWaterPolys (msurface_t *fa)
{
	//==============================
	glpoly_t	*p;
	glpoly_t	*bp;
	float		*v;
	int			i;
	float		s;
	float		t;
	float		os;
	float		ot;
	float		scroll;
	float		rdt = r_newrefdef.time;
	float		zValue = 0.0;			// height of water
	qboolean	waterNotFlat = false;
	qboolean	flowing;
	//==============================

	if (g_drawing_refl)
		return;	// we don't want any water drawn while we are doing our reflection

	if (fa->texinfo->flags & SURF_FLOWING)
	{
		scroll = -64.0f * ((r_newrefdef.time * 0.5f) - (int)(r_newrefdef.time * 0.5f));
		flowing = true;
	}
	else
	{
		scroll = 0.0f;
		flowing = false;
	}

	// skip the water texture on transparent surfaces
	if (r_reflectivewater->value && (fa->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)))
	{
		for (bp = fa->polys; bp; bp = bp->next)
		{
			p = bp;

			for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
			{
				// if it hasn't been initalized before
				if (zValue == 0.0f)
					zValue = v[2];

				// Make sure polygons are on the same plane
				// Fix for not perfectly flat water on base1 - strange ..
				else if (fabs(zValue - v[2]) > 8.0f)
					waterNotFlat = true;
			}
		}
	}

	if (waterNotFlat || !r_reflectivewater->value || !(fa->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)))
	{
		for (bp = fa->polys; bp; bp = bp->next)
		{
			p = bp;
			qgl.Begin(GL_TRIANGLE_FAN);
			c_brush_polys += p->numverts / 3; // jitrspeeds

			for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
			{
				os = v[3];
				ot = v[4];

				#if !id386
				s = os + r_turbsin[(int)((ot * 0.125f + r_newrefdef.time) * TURBSCALE) & 255];
				#else
				s = os + r_turbsin[Q_ftol(((ot * 0.125f + rdt) * TURBSCALE)) & 255];
				#endif

				s += scroll;
				s *= 0.015625f; // 1/64

				#if !id386
				t = ot + r_turbsin[(int)((os * 0.125f + rdt) * TURBSCALE) & 255];
				#else
				t = ot + r_turbsin[Q_ftol(((os * 0.125f + rdt) * TURBSCALE)) & 255];
				#endif
				t *= 0.015625f; // 1/64

				// if it hasn't been initalized before
				if (zValue == 0.0f)
					zValue = v[2];

				// Make sure polygons are on the same plane
				// Fix for not perfectly flat water on base1 - strange ..
				else if (fabs(zValue - v[2]) > 0.1f)
					waterNotFlat = true;

				qgl.TexCoord2f(s, t);
				qgl.Vertex3f(v[0], v[1], v[2]);
			}

			qgl.End();
		}
	}

	if (waterNotFlat)
		return;

	if (r_reflectivewater->value)
	{
#if 0
		//====================
		vec3_t	distanceVector;
		float	distance;
		//====================
#endif

		v = p->verts[0];
#if 0
		VectorSubtract(v, r_newrefdef.vieworg, distanceVector);
		distance = VectorLength(distanceVector);
		//R_add_refl(zValue, distance);
#endif
		R_add_refl(v[0], v[1], zValue);
		g_refl_enabled = true;
	}

	// find out which reflection we have that corresponds to the surface that we're drawing
	for (g_active_refl = 0; g_active_refl < g_num_refl; g_active_refl++)
	{
		// if we find which reflection to bind
		if (fabs(g_refl_Z[g_active_refl] - zValue) < 8.0f)
		{
			// === jitwater
			if (gl_state.fragment_program)
			{
				qgl.Enable(GL_VERTEX_PROGRAM_ARB);
				qgl.BindProgramARB(GL_VERTEX_PROGRAM_ARB, g_water_vertex_program_id);
				qgl.Enable(GL_FRAGMENT_PROGRAM_ARB);
				qgl.BindProgramARB(GL_FRAGMENT_PROGRAM_ARB, g_water_fragment_program_id);

				GL_MBind(QGL_TEXTURE1, distort_tex->texnum);      // Distortion texture
				GL_MBind(QGL_TEXTURE2, water_normal_tex->texnum); // Normal texture
			}

			GL_MBind(QGL_TEXTURE0, g_refl_images[g_active_refl]->texnum); // Reflection texture
			// jitwater ===

			break;
		}
	}

	// if we found a reflective surface correctly, then go ahead and draw it
	if (g_active_refl != g_num_refl)
	{
		qgl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);

		if (!gl_state.blend)
			qgl.Enable(GL_BLEND);

		GL_TexEnv(GL_MODULATE);
		qgl.ShadeModel(GL_SMOOTH);

		if (gl_state.fragment_program)
		{
			float w, h;
			R_GetReflTexScale(&w, &h); // Probably unnecessary
			qgl.ProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0,
				w, h, rs_realtime * (flowing ? -0.3f : 0.2f), rs_realtime * -0.2f);
			qgl.ProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1,
				r_newrefdef.vieworg[0], r_newrefdef.vieworg[1], r_newrefdef.vieworg[2], 1.0f);
		}
		else
		{
			// Put UV coords in screen space for rendering the reflection texture.  Only need to do this when fragment programs are disabled.  It's handled in the vertex shader otherwise.
			R_LoadReflMatrix();
		}

  		// draw reflected water layer on top of regular
  		for (bp = fa->polys; bp; bp = bp->next)
  		{
			p = bp;
			qgl.Begin(GL_TRIANGLE_FAN);
			c_brush_polys += p->numverts / 3; // jitrspeeds

			for (i = 0, v = p->verts[0]; i < p->numverts; ++i, v += VERTEXSIZE)
			{
				if (gl_state.fragment_program)
				{
					qgl.MultiTexCoord3fvARB(QGL_TEXTURE0, v); // Used for world space
					qgl.MultiTexCoord3fvARB(QGL_TEXTURE1, v + 3); // Actual texture UV's.
				}
				else
				{
					vec3_t	vAngle;

					qgl.TexCoord3f(v[0], v[1] + CalcWave(v[0], v[1]), v[2]);

					if (r_newrefdef.rdflags & RDF_UNDERWATER)
					{
						VectorSubtract(v, r_newrefdef.vieworg, vAngle);
						VectorNormalize(vAngle);

						if (vAngle[2] > 0.55f)
							vAngle[2] = 0.55f;

						qgl.Color4f(1.0f, 1.0f, 1.0f, 0.9f - (vAngle[2] * 1.0f));
					}
					else
					{
						VectorSubtract(r_newrefdef.vieworg, v, vAngle);
						VectorNormalize(vAngle);

						if (vAngle[2] > 0.55f)
							vAngle[2] = 0.55f;

						qgl.Color4f(1.0f, 1.0f, 1.0f, 0.9f - (vAngle[2] * 1.0f));
					}
				}

				qgl.Vertex3f(v[0], v[1], v[2]);
			}

			qgl.End();
  		}

		R_ClearReflMatrix();

		if (!gl_state.blend)
			qgl.Disable(GL_BLEND);

		if (gl_state.fragment_program) // jitwater
		{
			qgl.Disable(GL_FRAGMENT_PROGRAM_ARB);
			qgl.Disable(GL_VERTEX_PROGRAM_ARB);
		}
    }
}
// jitwater ===

//===================================================================

vec3_t	skyclip[6] = {
	{ 1.0f, 1.0f, 0.0f },
	{ 1.0f,-1.0f, 0.0f },
	{ 0.0f,-1.0f, 1.0f },
	{ 0.0f, 1.0f, 1.0f },
	{ 1.0f, 0.0f, 1.0f },
	{-1.0f, 0.0f, 1.0f }
};

int	c_sky;

// 1 = s, 2 = t, 3 = 2048
int	st_to_vec[6][3] =
{
	{ 3.0f,-1.0f, 2.0f },
	{-3.0f, 1.0f, 2.0f },

	{ 1.0f, 3.0f, 2.0f },
	{-1.0f,-3.0f, 2.0f },

	{-2.0f,-1.0f, 3.0f },		// 0 degrees yaw, look straight up
	{ 2.0f,-1.0f,-3.0f }		// look straight down

//	{-1,2,3},
//	{1,2,-3}
};

// s = [0]/[2], t = [1]/[2]
int	vec_to_st[6][3] =
{
	{-2.0f, 3.0f, 1.0f },
	{ 2.0f, 3.0f,-1.0f },

	{ 1.0f, 3.0f, 2.0f },
	{-1.0f, 3.0f,-2.0f },

	{-2.0f,-1.0f, 3.0f },
	{-2.0f, 1.0f,-3.0f }

//	{-1,2,3},
//	{1,2,-3}
};

float	skymins[2][6], skymaxs[2][6];
float	sky_min, sky_max;

void DrawSkyPolygon (int nump, vec3_t vecs)
{
	int		i,j;
	vec3_t	v, av;
	float	s, t, dv;
	int		axis;
	float	*vp;

	c_sky++;
#if 0
glBegin (GL_POLYGON);
for (i=0; i<nump; i++, vecs+=3)
{
	VectorAdd(vecs, r_origin, v);
	qgl.Vertex3fv (v);
}
glEnd();
return;
#endif
	// decide which face it maps to
	VectorCopy (vec3_origin, v);
	for (i=0, vp=vecs; i<nump; i++, vp+=3)
	{
		VectorAdd (vp, v, v);
	}
	av[0] = fabs(v[0]);
	av[1] = fabs(v[1]);
	av[2] = fabs(v[2]);
	if (av[0] > av[1] && av[0] > av[2])
	{
		if (v[0] < 0)
			axis = 1;
		else
			axis = 0;
	}
	else if (av[1] > av[2] && av[1] > av[0])
	{
		if (v[1] < 0)
			axis = 3;
		else
			axis = 2;
	}
	else
	{
		if (v[2] < 0)
			axis = 5;
		else
			axis = 4;
	}

	// project new texture coords
	for (i=0; i<nump; i++, vecs+=3)
	{
		j = vec_to_st[axis][2];
		if (j > 0)
			dv = vecs[j - 1];
		else
			dv = -vecs[-j - 1];
		if (dv < 0.001)
			continue;	// don't divide by zero
		j = vec_to_st[axis][0];
		if (j < 0)
			s = -vecs[-j -1] / dv;
		else
			s = vecs[j-1] / dv;
		j = vec_to_st[axis][1];
		if (j < 0)
			t = -vecs[-j -1] / dv;
		else
			t = vecs[j-1] / dv;

		if (s < skymins[0][axis])
			skymins[0][axis] = s;
		if (t < skymins[1][axis])
			skymins[1][axis] = t;
		if (s > skymaxs[0][axis])
			skymaxs[0][axis] = s;
		if (t > skymaxs[1][axis])
			skymaxs[1][axis] = t;
	}
}

#define	ON_EPSILON		0.1			// point on plane side epsilon
#define	MAX_CLIP_VERTS	64
void ClipSkyPolygon (int nump, vec3_t vecs, int stage)
{
	float	*norm;
	float	*v;
	qboolean	front, back;
	float	d, e;
	float	dists[MAX_CLIP_VERTS];
	int		sides[MAX_CLIP_VERTS];
	vec3_t	newv[2][MAX_CLIP_VERTS];
	int		newc[2];
	int		i, j;

	if (nump > MAX_CLIP_VERTS-2)
		ri.Sys_Error (ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");
	if (stage == 6)
	{	// fully clipped, so draw it
		DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for (i=0, v = vecs; i<nump; i++, v+=3)
	{
		d = DotProduct (v, norm);
		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		ClipSkyPolygon (nump, vecs, stage+1);
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs+(i*3)));
	newc[0] = newc[1] = 0;

	for (i=0, v = vecs; i<nump; i++, v+=3)
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			break;
		case SIDE_BACK:
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		case SIDE_ON:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i+1]);
		for (j=0; j<3; j++)
		{
			e = v[j] + d*(v[j+3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	ClipSkyPolygon (newc[0], newv[0][0], stage+1);
	ClipSkyPolygon (newc[1], newv[1][0], stage+1);
}

/*
=================
R_AddSkySurface
=================
*/
void R_AddSkySurface (msurface_t *fa)
{
	int			i;
	vec3_t		verts[MAX_CLIP_VERTS];
	glpoly_t	*p;

	// calculate vertex values for sky box
	for (p=fa->polys; p; p=p->next)
	{
		for (i=0; i<p->numverts; i++)
		{
			VectorSubtract (p->verts[i], r_origin, verts[i]);
		}
		ClipSkyPolygon (p->numverts, verts[0], 0);
	}
}


/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox (void)
{
	int		i;

	for (i=0; i<6; i++)
	{
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}


void MakeSkyVec (float s, float t, int axis)
{
	vec3_t		v, b;
	int			j, k;

	// jitodo: fix this so it works for REALLY big maps...
	b[0] = s * 8192.0f/*2300*/; // jit -- bigger skybox
	b[1] = t * 8192.0f/*2300*/; // jit -- bigger skybox
	b[2] = 8192.0f/*2300*/;   // jit -- bigger sky box

	for (j = 0; j < 3; ++j)
	{
		k = st_to_vec[axis][j];

		if (k < 0)
			v[j] = -b[-k - 1];
		else
			v[j] = b[k - 1];
	}

	// avoid bilerp seam
	s = (s + 1.0f) * 0.5f;
	t = (t + 1.0f) * 0.5f;

	if (s < sky_min)
		s = sky_min;
	else if (s > sky_max)
		s = sky_max;

	if (t < sky_min)
		t = sky_min;
	else if (t > sky_max)
		t = sky_max;

	t = 1.0 - t;
	qgl.TexCoord2f(s, t);
	qgl.Vertex3fv(v);
}


//#define ENABLE_HYG_STARS // jittemp - render star data for skyboxes
// http://astronexus.com/node/34
#define HYG_STARS_MAP_TO_SPHERE
#ifdef ENABLE_HYG_STARS

#define MAX_HYG_STARS 180000
//#define HYG_MIN_STARDIST 200.0f
#define HYG_MIN_STARDIST 2000.0f
#define HYG_STARDIST_SCALE 30.0f
#define HYG_MIN_MAG 10

//static vec3_t g_starpositions[MAX_HYG_STARS];
typedef struct {
	vec3_t origin;
	byte color[4];
	float scale;
} starparticle_t;

static starparticle_t g_starparticles[MAX_HYG_STARS];
//particle_t g_starparticles[MAX_HYG_STARS];
//particle_t g_starparticles_white[MAX_HYG_STARS];
static qboolean g_starsloaded = false;
static qboolean g_numstars = 0;

// VC's atof is super slow on long buffers because it does a strlen.
// Copy it to a small buffer temporarily to speed it up.  Meh.
float Meh_AtoF (const char *s)
{
	char meh[32];

	Q_strncpyzna(meh, s, sizeof(meh));
	return atof(meh);
}

void StarColorIndexToRGB (float colorindex, vec_t *color)
{
	// I have no idea how accurate this is.  I'm just guessing based on this:
	// http://domeofthesky.com/clicks/bv.html
	// -.29 = blue
	// 0 = white
	// .59 = yellow
	// .82 = orange
	// 1.41 = red
	// and http://curious.astro.cornell.edu/question.php?number=715
	// -.33 = blue
	// 0.15 = bluish white
	// 0.44 = yellow-white
	// 0.68 = yellow
	// 1.15 = orange
	// 1.64 = red
	int i;

	VectorSet(color, 1.0f, 1.0f, 1.0f);

	colorindex -= 0.4f; // shift so 0 is white

	if (colorindex < 0)
	{
		// remove red and green to make blue
		color[0] += colorindex * 1.5f;
		color[1] += colorindex * 1.5f;
	}
	else
	{
		// Just kind of guessing with arbitrary at this point to get something that looks decent.
		color[1] -= colorindex / 2; // remove green
		color[2] -= colorindex / 1.5; // remove blue
	}

	for (i = 0; i < 3; ++i)
	{
		if (color[i] < 0.0f)
			color[i] = 0.0f;

		if (color[i] > 1.0f)
			color[i] = 1.0f;
	}

	VectorNormalize(color);
}


void R_LoadStars (void) // jittemp
{
	char *buff;
	int file_len = ri.FS_LoadFileZ("hygxyz.csv", (void **)&buff);
	vec3_t x_axis = { 1.0f, 0.0f, 0.0f };
	vec3_t y_axis = { 0.0f, 1.0f, 0.0f };
	
	if (file_len > 0)
	{
		int commacount = 0;
		char *s = buff;
		int n = 0;
		int i;

		// Skip first 2 lines (header and sun)
		for (i = 0; i < 2; ++i)
		{
			while (n < file_len && *s != '\n')
			{
				++n;
				++s;
			}

			++n;
			++s;
		}

		while (n < file_len && g_numstars < MAX_HYG_STARS)
		{
			float absmag = 9999999.f;
			float mag = 99999.9f;
			float colorindex = 0.0f;
			qboolean has_proper_name = false;

			while (*s != '\n' && n < file_len)
			{
				if (*s == ',')
				{
					++commacount;

					if (commacount == 17) // X = up (vernal equinox)
						g_starparticles[g_numstars].origin[2] = Meh_AtoF(s + 1);
					else if (commacount == 18) // Y = along equator (RA)
						g_starparticles[g_numstars].origin[0] = Meh_AtoF(s + 1);
					else if (commacount == 19) // Z = toward north pole
						g_starparticles[g_numstars].origin[1] = Meh_AtoF(s + 1);
					else if (commacount == 14) // absmag
						absmag = Meh_AtoF(s + 1);
					else if (commacount == 13) // mag
						mag = Meh_AtoF(s + 1);
					else if (commacount == 16) // colorindex
						colorindex = Meh_AtoF(s + 1);
					else if (commacount == 6)
					{
						if (s[1] != ',')
							has_proper_name = true;
					}

				}

				++n;
				++s;
			}

			++n;
			++s;
			commacount = 0;

			if (absmag < 0)
				absmag = 0;

			if (absmag > 15)
				absmag = 15;

			if (mag < HYG_MIN_MAG)
			{
				float colormagscale;
#ifdef HYG_STARS_MAP_TO_SPHERE
				float magscale = (HYG_MIN_MAG - /*abs*/mag) / HYG_MIN_MAG;
#else
				float magscale = (HYG_MIN_MAG - absmag) / HYG_MIN_MAG;
#endif
				vec3_t color;
				
				StarColorIndexToRGB(colorindex, color);

				if (magscale > 1.0f)
					magscale = 1.0f;

				if (magscale < 0.0f)
					continue;

#ifdef HYG_STARS_MAP_TO_SPHERE
				colormagscale = 0.01f + powf(magscale, 2.5f) * 2.0f;

				if (colormagscale > 1.0f)
					colormagscale = 1.0f;
#else
				colormagscale = magscale * magscale;
#endif

#ifdef HYG_STARS_MAP_TO_SPHERE
				VectorNormalize(g_starparticles[g_numstars].origin);

				RotatePointAroundVector(g_starparticles[g_numstars].origin, y_axis, g_starparticles[g_numstars].origin, -100.0f);
				RotatePointAroundVector(g_starparticles[g_numstars].origin, x_axis, g_starparticles[g_numstars].origin, 35.0f); // latitude of 35 degrees

				VectorScale(g_starparticles[g_numstars].origin, HYG_MIN_STARDIST, g_starparticles[g_numstars].origin);
				g_starparticles[g_numstars].scale = 4.0f + magscale * magscale* magscale * magscale * magscale * 30.0f;

				if (g_starparticles[g_numstars].scale > 14.0f)
					g_starparticles[g_numstars].scale = 14.0f;

				g_starparticles[g_numstars].scale *= 0.8f;

#else
				VectorScale(g_starparticles[g_numstars].origin, HYG_STARDIST_SCALE, g_starparticles[g_numstars].origin);
				g_starparticles[g_numstars].scale = 1 + magscale * magscale * magscale * magscale * 20;
#endif
				g_starparticles[g_numstars].color[0] = 255 * color[0] * colormagscale;
				g_starparticles[g_numstars].color[1] = 255 * color[1] * colormagscale;// * (has_proper_name ? 1 : 0);
				g_starparticles[g_numstars].color[2] = 255 * color[2] * colormagscale;
				g_starparticles[g_numstars].color[3] = 255 * colormagscale;

				

				++g_numstars;
			}
		}

		ri.FS_FreeFile(buff);
		g_starsloaded = true;
	}
}

void R_DrawStarHelper (const vec_t *origin, const vec_t *up_i, const vec_t *right_i, const byte *color, float scale)
{
	vec3_t v, up, right;

	VectorScale(up_i, scale, up);
	VectorScale(right_i, scale, right);

	qgl.Color4ubv(color);

	// Upper left
	qgl.TexCoord2f(0.0f, 0.0f);
	VectorAdd(origin, up, v);
	VectorSubtract(v, right, v);
	qgl.Vertex3fv(v);
	// Upper right
	qgl.TexCoord2f(1.0f, 0.0f);
	VectorAdd(origin, up, v);
	VectorAdd(v, right, v);
	qgl.Vertex3fv(v);
	// Lower right
	qgl.TexCoord2f(1.0f, 1.0f);
	VectorSubtract(origin, up, v);
	VectorAdd(v, right, v);
	qgl.Vertex3fv(v);
	// Lower left
	qgl.TexCoord2f(0.0f, 1.0f);
	VectorSubtract(origin, up, v);
	VectorSubtract(v, right, v);
	qgl.Vertex3fv(v);
}

void R_DrawMoon (void)
{
	static image_t *r_moontexture = NULL;
	vec3_t moon_angles = { 37.62f, -97.73f, 0.0f };
	vec3_t moon_pos;
	vec3_t vec_from_moon;
	vec3_t up, right;
	byte white[4] = { 255, 255, 255, 255 };
	vec3_t			worldup = { 0.0f, 0.0f, 1.0f };

	if (!r_moontexture)
	{
		r_moontexture = GL_FindImage("textures/temp/moon1.tga", it_wall);
	}

	AngleVectors(moon_angles, moon_pos, NULL, NULL);
	VectorScale(moon_pos, HYG_MIN_STARDIST, moon_pos);

	VectorSubtract(r_origin, moon_pos, vec_from_moon);
	CrossProduct(worldup, vec_from_moon, right);
	CrossProduct(vec_from_moon, right, up);
	VectorNormalize(up);
	VectorNormalize(right);
	GL_Bind(r_moontexture->texnum);
	qgl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qgl.Begin(GL_QUADS);
	R_DrawStarHelper(moon_pos, up, right, white, 90.0f);
	qgl.End();
}

void R_DrawStars (void)
{
	if (!g_starsloaded)
	{
		R_LoadStars();
	}
	else
	{
		const starparticle_t *p;
		int				i;
		vec3_t			up, right, vec_from_star;
		vec3_t			worldup = { 0.0f, 0.0f, 1.0f };
		float			scale;
		byte			white_color[4];

		qgl.PushMatrix();
		qgl.Translatef(r_origin[0], r_origin[1], r_origin[2]);
		//qgl.Rotatef(40.0f, 1.0f, 0.0f, 0.0f);
		//qgl.Rotatef(-50.0f, 0.0f, 1.0f, 0.0f);
		GL_Bind(r_startexture->texnum);
		qgl.DepthMask(GL_FALSE);		// no z buffering
		qgl.BlendFunc(GL_ONE, GL_ONE);
		GLSTATE_ENABLE_BLEND;
		GL_TexEnv(GL_MODULATE);
		qgl.Begin(GL_QUADS);

		//VectorScale(vup, 1.5, up);
		//VectorScale(vright, 1.5, right);

		for (p = g_starparticles, i = 0; i < g_numstars; ++i, ++p)
		{
			VectorSubtract(r_origin, p->origin, vec_from_star);
			CrossProduct(worldup, vec_from_star, right);
			CrossProduct(vec_from_star, right, up);
			VectorNormalize(up);
			VectorNormalize(right);

			scale = p->scale;// * p->scale;
			R_DrawStarHelper(p->origin, up, right, p->color, scale);
			white_color[0] = p->color[3];
			white_color[1] = p->color[3];
			white_color[2] = p->color[3];
			white_color[3] = p->color[3];
			R_DrawStarHelper(p->origin, up, right, white_color, scale);
		}

		qgl.End();

#ifdef HYG_STARS_MAP_TO_SPHERE
		R_DrawMoon();
#endif

		qgl.PopMatrix();
		GLSTATE_DISABLE_BLEND;
		qgl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
		qgl.DepthMask(GL_TRUE);		// back to normal Z buffering
		GL_TexEnv(GL_REPLACE);
	}
}
#endif // ENABLE_HYG_STARS

/*
==============
R_DrawSkyBox
==============
*/
extern qboolean fogenabled;
int	skytexorder[6] = {0,2,1,3,4,5};
void R_DrawSkyBox (void)
{
#ifdef ENABLE_HYG_STARS
	R_DrawStars(); // jittemp
#else
	int		i;

	if (fogenabled) // jitfog
		qgl.Disable(GL_FOG);

	if (skyrotate)
	{
		// check for no sky at all
		for (i = 0; i < 6; ++i)
		{
			if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
				break;
		}

		if (i == 6)
			return;		// nothing visible
	}

	qgl.PushMatrix();
	qgl.Translatef(r_origin[0], r_origin[1], r_origin[2]);
	qgl.Rotatef(r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);

	if (fogenabled && sky_images[0] == r_whitetexture) // jitfog
	{
		qgl.Color3fv(fogcolor);
		GLSTATE_ENABLE_BLEND
		GL_TexEnv(GL_MODULATE);
	}

	for (i = 0; i < 6; ++i)
	{
		if (skyrotate)
		{
			// hack, forces full sky to draw when rotating
			skymins[0][i] = -1;
			skymins[1][i] = -1;
			skymaxs[0][i] = 1;
			skymaxs[1][i] = 1;
		}

		if (skymins[0][i] >= skymaxs[0][i] || skymins[1][i] >= skymaxs[1][i])
			continue;

		GL_Bind(sky_images[skytexorder[i]]->texnum);

		qgl.Begin(GL_QUADS);
		MakeSkyVec(skymins[0][i], skymins[1][i], i);
		MakeSkyVec(skymins[0][i], skymaxs[1][i], i);
		MakeSkyVec(skymaxs[0][i], skymaxs[1][i], i);
		MakeSkyVec(skymaxs[0][i], skymins[1][i], i);
		qgl.End();
	}

	qgl.PopMatrix();

	if (fogenabled) // jitfog
	{
		qgl.Color3f(1, 1, 1);
		GLSTATE_DISABLE_BLEND
		qgl.Enable(GL_FOG);
	}
#endif
}


/*
============
R_SetSky
============
*/
// 3dstudio environment map names
char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
qboolean skycanchange = true;

void R_SetSky (char *name, float rotate, vec3_t axis)
{
	int		i;
	char	pathname[MAX_QPATH];
	char	*s;

	if (!skycanchange) // so people can't shut fog off.
		return;

	Q_strncpyz(skyname, name, sizeof(skyname) - 1);
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	// ==
	// jitfog -- parse fog code from sky name
	if ((s = strstr(skyname, "fog "))) // jitodo
	{
		sscanf(s + 4, "%f %f %f %f",
			&fogcolor[0], &fogcolor[1], &fogcolor[2],
			&fogdistance);
		fogdensity = 0.0f;
		fogenabled = true;
		skycanchange = false;
	}
	else if ((s = strstr(skyname, "fogd "))) // jitfog
	{
		sscanf(s + 5, "%f %f %f %f",
			&fogcolor[0], &fogcolor[1], &fogcolor[2],
			&fogdensity);
		fogdistance = 0.0f;

		if (fogdensity)
			fogenabled = true;

		skycanchange = false;
	}

	// jitfog -- strip fog code from sky name:
	if ((s = strchr(skyname, ' ')))
		*s = 0;

	if (gl_skyedge->value > 0.01)
		ri.Cvar_Set("gl_skyedge", "0.01");

	for (i = 0; i < 6; i++)
	{
		Com_sprintf(pathname, sizeof(pathname), "env/%s%s.tga", skyname, suf[i]);
		sky_images[i] = GL_FindImage(pathname, it_sky);

		if (!sky_images[i])
		{
			if (fogenabled) // jitfog
				sky_images[i] = r_whitetexture;
			else
				sky_images[i] = r_notexture;
		}
	}

	sky_min = 0 + gl_skyedge->value;
	sky_max = 1 - gl_skyedge->value;

	if (s) // jitfog -- reenable fog code.
		*s = ' ';
}

/*
	Surface Subdivision
	Modified By Robert 'Heffo' Heffernan
	6/1/2002 12:13pm GMT +10
*/
#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

void SubdivideLightmappedPolygon (int numverts, float *verts, float subdivide_size)
{
	int		i, j, k;
	vec3_t	mins, maxs;
	float	m;
	float	*v;
	vec3_t	front[64], back[64];
	int		f, b;
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;
	vec3_t	total;
	float	total_s, total_t, total_u, total_v;

	if (numverts > 60)
		ri.Sys_Error (ERR_DROP, "numverts = %i", numverts);

	BoundPoly (numverts, verts, mins, maxs);

	for (i=0; i<3; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5f;
		m = subdivide_size * floor (m/subdivide_size + 0.5f);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0; j<numverts; j++, v+= 3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0; j<numverts; j++, v+= 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ((dist[j] > 0) != (dist[j+1] > 0))
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0; k<3; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdivideLightmappedPolygon (f, front[0], subdivide_size);
		SubdivideLightmappedPolygon (b, back[0], subdivide_size);
		return;
	}

	// add a point in the center to help keep warp valid
	poly = Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * VERTEXSIZE*sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts+2;
	VectorClear (total);
	total_s = 0; total_t = 0;
	total_u = 0; total_v = 0;

	for (i=0; i<numverts; i++, verts+= 3)
	{
		VectorCopy (verts, poly->verts[i+1]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]) + warpface->texinfo->vecs[0][3];
		s /= warpface->texinfo->image->width;

		t = DotProduct (verts, warpface->texinfo->vecs[1]) + warpface->texinfo->vecs[1][3];
		t /= warpface->texinfo->image->height;

		total_s += s;
		total_t += t;
		VectorAdd (total, verts, total);

		poly->verts[i+1][3] = s;
		poly->verts[i+1][4] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (verts, warpface->texinfo->vecs[0]) + warpface->texinfo->vecs[0][3];
		s -= warpface->texturemins[0];
		s += warpface->light_s*16;
		s += 8;
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (verts, warpface->texinfo->vecs[1]) + warpface->texinfo->vecs[1][3];
		t -= warpface->texturemins[1];
		t += warpface->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		total_u += s;
		total_v += t;

		poly->verts[i+1][5] = s;
		poly->verts[i+1][6] = t;
	}

	VectorScale (total, (1.0/numverts), poly->verts[0]);
	poly->verts[0][3] = total_s/numverts;
	poly->verts[0][4] = total_t/numverts;

	poly->verts[0][5] = total_u/numverts;
	poly->verts[0][6] = total_v/numverts;

	// copy first vertex to last
	memcpy (poly->verts[i+1], poly->verts[1], sizeof(poly->verts[0]));
}

/*
================
GL_SubdivideLightmappedSurface

Breaks a polygon up along axial arbitary ^2 unit
boundaries so that vertex warps
can be done without looking like shit.
================
*/
void GL_SubdivideLightmappedSurface (msurface_t *fa, float subdivide_size)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = fa;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0; i<fa->numedges; i++)
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdivideLightmappedPolygon (numverts, verts[0], subdivide_size);
}
