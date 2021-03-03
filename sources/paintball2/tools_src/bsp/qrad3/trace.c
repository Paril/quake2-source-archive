/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.
This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <assert.h>
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "threads.h"

#define	ON_EPSILON	0.1

typedef struct tnode_s
{
	int		type;
	vec3_t	normal;
	float	dist;
	int		children[2];
	int		dnode_children[2]; // jit - we need to get the actual leaf node for some tests.
	int     pad;
} tnode_t;

tnode_t		*tnodes, *tnode_p;

/*
==============
MakeTnode

Converts the disk node structure into the efficient tracing structure
==============
*/
void MakeTnode (int nodenum)
{
	tnode_t			*t;
	dplane_t		*plane;
	int				i;
	dnode_t 		*node;

	t = tnode_p++;

	node = dnodes + nodenum;
	plane = dplanes + node->planenum;

	t->type = plane->type;
	VectorCopy (plane->normal, t->normal);
	t->dist = plane->dist;

	for (i=0 ; i<2 ; i++)
	{
		t->dnode_children[i] = node->children[i]; // jit

		if (node->children[i] < 0)
		{
			t->children[i] = (dleafs[-node->children[i] - 1].contents & CONTENTS_SOLID) | (1<<31);
		}
		else
		{
			t->children[i] = tnode_p - tnodes;
			MakeTnode (node->children[i]);
		}
	}

}


/*
=============
MakeTnodes

Loads the node structure out of a .bsp file to be used for light occlusion
=============
*/
void MakeTnodes (dmodel_t *bm)
{
	// 32 byte align the structs
	tnodes = malloc( (numnodes+1) * sizeof(tnode_t));
	tnodes = (tnode_t *)(((int)tnodes + 31)&~31);
	tnode_p = tnodes;

	MakeTnode (0);
}


//==========================================================
#define	DIST_EPSILON	(0.03125f) // 1/32 epsilon to keep floating point happy

int TraceToSkyBrushTest (vec3_t scale, const vec3_t p1, const vec3_t p2, dbrush_t *brush) // jit - modified CM_ClipBoxToBrush() from engine code.
{
	int			i;
	dplane_t	*plane, *clipplane;
	float		dist;
	float		enterfrac, leavefrac;
	//vec3_t		ofs;
	float		d1, d2;
	qboolean	getout, startout;
	float		f;
	dbrushside_t	*side, *leadside;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if (!brush->numsides)
	{
		assert(0);
		return 0; // shouldn't happen?
	}

	//c_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for (i = 0; i < brush->numsides; ++i)
	{
		//side = &map_brushsides[brush->firstbrushside + i];
		side = dbrushsides + (brush->firstside + i);
		//plane = side->plane;
		plane = dplanes + side->planenum;

		dist = plane->dist;

		d1 = DotProduct(p1, plane->normal) - dist;
		d2 = DotProduct(p2, plane->normal) - dist;

		if (d2 > 0)
			getout = true;	// endpoint is not in solid

		if (d1 > 0)
			startout = true;

		// if completely in front of face, no intersection
		if (d1 > 0 && d2 >= d1)
			return 0;

		if (d1 <= 0 && d2 <= 0)
			continue;

		// crosses face
		if (d1 > d2)
		{
			// enter
			f = (d1 - DIST_EPSILON) / (d1 - d2);

			if (f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{	// leave
			f = (d1 + DIST_EPSILON) / (d1 - d2);

			if (f < leavefrac)
				leavefrac = f;
		}
	}

	if (!startout)
	{
		/*
		// original point was inside brush
		trace->startsolid = true;

		if (!getout)
			trace->allsolid = true;
		*/
		scale[0] = scale[1] = scale[2] = 0.0f; // todo: we might start inside transparent brushes?  We'll worry about that later.
		return 1;
	}

	if (leadside && enterfrac < leavefrac)
	{
		/*
		if (enterfrac > -1 && enterfrac < trace->fraction)
		{
			if (enterfrac < 0)
				enterfrac = 0;

			trace->fraction = enterfrac;
			trace->plane = *clipplane;
			trace->surface = &(leadside->surface->c);
			trace->contents = brush->contents;
		}*/
		if (texinfo[leadside->texinfo].flags & SURF_SKY)
			return 0;
		else
			return 1;
	}

	return 0; // shouldn't hit?
}

// jit - mostly copied from TestLine_r.
static int TraceToSky_r (vec3_t scale, int node, int node_d, const vec3_t start_in, const vec3_t stop)
{
	//dnode_t	*dnode;
	tnode_t *tnode;
	float	front, back;
	vec3_t	mid, start;
	float	frac;
	int		side;
	int		r;

	start[0] = start_in[0];
	start[1] = start_in[1];
	start[2] = start_in[2];

re_test:

	// Nodes < 0 (highest bit set) are leaf nodes.
	//if (node & (1<<31))
	if (node_d < 0)
	{
		int leafnum = (-1 - node_d);//node & ~(1<<31);	// leaf node

		if (leafnum && (node & CONTENTS_SOLID)) // we might eventually check CONTENTS_WINDOW as well for transparent surface shadows.
		{
			int			k;
			int			brushnum;
			dleaf_t		*leaf;
			dbrush_t	*b;
			int			brushtestret;
			vec3_t		start_back;

			// Move the start point back slightly so it doesnt' start inside of the brush.
			VectorSubtract(start, stop, start_back);

			if (VectorLengthSq(start_back) > 0.0f)
				VectorNormalize(start_back, start_back);

			VectorMA(start_back, DIST_EPSILON, start, start_back);

			//leaf = &map_leafs[leafnum];
			leaf = dleafs + leafnum;

			// trace line against all brushes in the leaf
			for (k = 0; k < leaf->numleafbrushes; ++k)
			{
				//brushnum = map_leafbrushes[leaf->firstleafbrush + k];
				brushnum = dleafbrushes[leaf->firstleafbrush + k];
				//b = &map_brushes[brushnum];
				b = dbrushes + brushnum;

				//if (b->checkcount == checkcount)
				//	continue;	// already checked this brush in another leaf

				//b->checkcount = checkcount;

				//if (!(b->contents & trace_contents))
				//	continue;

				brushtestret = TraceToSkyBrushTest(scale, start_back, stop, b);

				if (brushtestret != 0)
					return brushtestret;
			}
		}

		return 0;
	}

	//dnode = dnodes + node_d;
	tnode = tnodes + node;

	switch (tnode->type)
	{
	case PLANE_X:
		front = start[0] - tnode->dist;
		back = stop[0] - tnode->dist;
		break;
	case PLANE_Y:
		front = start[1] - tnode->dist;
		back = stop[1] - tnode->dist;
		break;
	case PLANE_Z:
		front = start[2] - tnode->dist;
		back = stop[2] - tnode->dist;
		break;
	default:
		front = (start[0]*tnode->normal[0] + start[1]*tnode->normal[1] + start[2]*tnode->normal[2]) - tnode->dist;
		back = (stop[0]*tnode->normal[0] + stop[1]*tnode->normal[1] + stop[2]*tnode->normal[2]) - tnode->dist;
		break;
	}

	if (front >= -ON_EPSILON && back >= -ON_EPSILON)
	{
		node = tnode->children[0];
		node_d = tnode->dnode_children[0];

		goto re_test;
	}

	if (front < ON_EPSILON && back < ON_EPSILON)
	{
		node = tnode->children[1];
		node_d = tnode->dnode_children[1];

		goto re_test;
	}

	side = front < 0;

	frac = front / (front-back);

	mid[0] = start[0] + (stop[0] - start[0])*frac;
	mid[1] = start[1] + (stop[1] - start[1])*frac;
	mid[2] = start[2] + (stop[2] - start[2])*frac;

	r = TraceToSky_r(scale, tnode->children[side], tnode->dnode_children[side], start, mid);

	if (r)
		return r;

	node = tnode->children[!side];
	node_d = tnode->dnode_children[!side];

	start[0] = mid[0];
	start[1] = mid[1];
	start[2] = mid[2];

	goto re_test;
}

// returs 0 if there's line of sight to sky.
int TraceToSky (vec3_t scale, const vec3_t start, const vec3_t dir) // jit - function to see if sky is visible from a location
{
	vec3_t end;

	end[0] = start[0] - dir[0] * 9000.1f;
	end[1] = start[1] - dir[1] * 9000.1f;
	end[2] = start[2] - dir[2] * 9000.1f;

	return TraceToSky_r(scale, 0, 0, start, end);
}

//==========================================================

int TestLine_r (int node, vec3_t set_start, vec3_t stop)
{
	tnode_t	*tnode;
	float	front, back;
	vec3_t	mid, start;
	float	frac;
	int		side;
	int		r;

	start[0] = set_start[0];
	start[1] = set_start[1];
	start[2] = set_start[2];

re_test:

	if (node & (1<<31))
	{
		return node & ~(1<<31);	// leaf node
	}

	tnode = &tnodes[node];
	switch (tnode->type)
	{
	case PLANE_X:
		front = start[0] - tnode->dist;
		back = stop[0] - tnode->dist;
		break;
	case PLANE_Y:
		front = start[1] - tnode->dist;
		back = stop[1] - tnode->dist;
		break;
	case PLANE_Z:
		front = start[2] - tnode->dist;
		back = stop[2] - tnode->dist;
		break;
	default:
		front = (start[0]*tnode->normal[0] + start[1]*tnode->normal[1] + start[2]*tnode->normal[2]) - tnode->dist;
		back = (stop[0]*tnode->normal[0] + stop[1]*tnode->normal[1] + stop[2]*tnode->normal[2]) - tnode->dist;
		break;
	}

	if (front >= -ON_EPSILON && back >= -ON_EPSILON)
	{
		node = tnode->children[0];

		goto re_test;

		//		return TestLine_r (tnode->children[0], start, stop);
	}

	if (front < ON_EPSILON && back < ON_EPSILON)
	{
		node = tnode->children[1];

		goto re_test;

		//		return TestLine_r (tnode->children[1], start, stop);
	}

	side = front < 0;

	frac = front / (front-back);

	mid[0] = start[0] + (stop[0] - start[0])*frac;
	mid[1] = start[1] + (stop[1] - start[1])*frac;
	mid[2] = start[2] + (stop[2] - start[2])*frac;

	r = TestLine_r (tnode->children[side], start, mid);

	if (r)
		return r;

	node = tnode->children[!side];

	start[0] = mid[0];
	start[1] = mid[1];
	start[2] = mid[2];

	goto re_test;

	//    return TestLine_r (tnode->children[!side], mid, stop);
}

int TestLine (vec3_t start, vec3_t stop)
{
	return TestLine_r (0, start, stop);
}

/*
==============================================================================

LINE TRACING

The major lighting operation is a point to point visibility test, performed
by recursive subdivision of the line by the BSP tree.

==============================================================================
*/

typedef struct
{
	vec3_t	backpt;
	int		side;
	int		node;
} tracestack_t;


/*
==============
TestLine
==============
*/
qboolean _TestLine (vec3_t start, vec3_t stop)
{
	int				node;
	float			front, back;
	tracestack_t	*tstack_p;
	int				side;
	float 			frontx,fronty, frontz, backx, backy, backz;
	tracestack_t	tracestack[64];
	tnode_t			*tnode;

	frontx = start[0];
	fronty = start[1];
	frontz = start[2];
	backx = stop[0];
	backy = stop[1];
	backz = stop[2];

	tstack_p = tracestack;
	node = 0;

	while (1)
	{
		if (node == CONTENTS_SOLID)
		{
#if 0
			float	d1, d2, d3;

			d1 = backx - frontx;
			d2 = backy - fronty;
			d3 = backz - frontz;

			if (d1*d1 + d2*d2 + d3*d3 > 1)
#endif
				return false;	// DONE!
		}

		while (node < 0)
		{
			// pop up the stack for a back side
			tstack_p--;
			if (tstack_p < tracestack)
				return true;
			node = tstack_p->node;

			// set the hit point for this plane

			frontx = backx;
			fronty = backy;
			frontz = backz;

			// go down the back side

			backx = tstack_p->backpt[0];
			backy = tstack_p->backpt[1];
			backz = tstack_p->backpt[2];

			node = tnodes[tstack_p->node].children[!tstack_p->side];
		}

		tnode = &tnodes[node];

		switch (tnode->type)
		{
		case PLANE_X:
			front = frontx - tnode->dist;
			back = backx - tnode->dist;
			break;
		case PLANE_Y:
			front = fronty - tnode->dist;
			back = backy - tnode->dist;
			break;
		case PLANE_Z:
			front = frontz - tnode->dist;
			back = backz - tnode->dist;
			break;
		default:
			front = (frontx*tnode->normal[0] + fronty*tnode->normal[1] + frontz*tnode->normal[2]) - tnode->dist;
			back = (backx*tnode->normal[0] + backy*tnode->normal[1] + backz*tnode->normal[2]) - tnode->dist;
			break;
		}

		if (front > -ON_EPSILON && back > -ON_EPSILON)
			//		if (front > 0 && back > 0)
		{
			node = tnode->children[0];
			continue;
		}

		if (front < ON_EPSILON && back < ON_EPSILON)
			//		if (front <= 0 && back <= 0)
		{
			node = tnode->children[1];
			continue;
		}

		side = front < 0;

		front = front / (front-back);

		tstack_p->node = node;
		tstack_p->side = side;
		tstack_p->backpt[0] = backx;
		tstack_p->backpt[1] = backy;
		tstack_p->backpt[2] = backz;

		tstack_p++;

		backx = frontx + front*(backx-frontx);
		backy = fronty + front*(backy-fronty);
		backz = frontz + front*(backz-frontz);

		node = tnode->children[side];		
	}	
}
