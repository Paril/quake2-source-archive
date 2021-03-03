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

#include "qrad.h"
#include "parsecfg.h"

/*

NOTES
-----

every surface must be divided into at least two patches each axis

*/

patch_t		*face_patches[MAX_MAP_FACES];
entity_t	*face_entity[MAX_MAP_FACES];
patch_t		patches[MAX_PATCHES];
unsigned	num_patches;

vec3_t		radiosity[MAX_PATCHES];		// light leaving a patch
vec3_t		illumination[MAX_PATCHES];	// light arriving at a patch

vec3_t		face_offset[MAX_MAP_FACES];		// for rotating bmodels
dplane_t	backplanes[MAX_MAP_PLANES];

char		inbase[32], outbase[32];

int			fakeplanes;					// created planes for origin offset 

int		numbounce = 8;
qboolean	extrasamples;

int noblock = false;
int memory = false;

float patch_cutoff = 0.0f;

float	subdiv = 64;
qboolean	dumppatches;

void BuildLightmaps (void);
int TestLine (vec3_t start, vec3_t stop);

int		junk;

float	ambient = 0;
float	maxlight = 196;

float	lightscale = 1.0;
float	g_texscale = 1.0f; // jit

qboolean sun = false;
qboolean sun_alt_color = true; // jit
vec3_t sun_pos = {0.0f, 0.0f, 1.0f};
int sun_main = 250.0f;
int sun_ambient = 60.0f; // jit
vec3_t sun_color = {1, 1, 1};
vec3_t sun_angle = {146, -22, 0}; // jit

qboolean	glview;
qboolean    nocolor = false;
qboolean	nopvs;
qboolean	save_trace = false;

char		source[1024];

float	direct_scale =	0.4;
float	entity_scale =	1.0;
//float	sky_scale = 0.00001f;
float	sky_scale = 1.0f;

/*
===================================================================

MISC

===================================================================
*/


/*
=============
MakeBackplanes
=============
*/
void MakeBackplanes (void)
{
	int		i;

	for (i=0 ; i<numplanes ; i++)
	{
		backplanes[i].dist = -dplanes[i].dist;
		VectorSubtract (vec3_origin, dplanes[i].normal, backplanes[i].normal);
	}
}

int		leafparents[MAX_MAP_LEAFS];
int		nodeparents[MAX_MAP_NODES];

/*
=============
MakeParents
=============
*/
void MakeParents (int nodenum, int parent)
{
	int		i, j;
	dnode_t	*node;

	nodeparents[nodenum] = parent;
	node = &dnodes[nodenum];

	for (i=0 ; i<2 ; i++)
	{
		j = node->children[i];
		if (j < 0)
			leafparents[-j - 1] = nodenum;
		else
			MakeParents (j, nodenum);
	}
}


/*
===================================================================

TRANSFER SCALES

===================================================================
*/

int	PointInLeafnum (vec3_t point)
{
	int		nodenum;
	vec_t	dist;
	dnode_t	*node;
	dplane_t	*plane;

	nodenum = 0;
	while (nodenum >= 0)
	{
		node = &dnodes[nodenum];
		plane = &dplanes[node->planenum];
		dist = DotProduct (point, plane->normal) - plane->dist;
		if (dist > 0)
			nodenum = node->children[0];
		else
			nodenum = node->children[1];
	}

	return -nodenum - 1;
}


dleaf_t		*PointInLeaf (vec3_t point)
{
	int		num;

	num = PointInLeafnum (point);
	return &dleafs[num];
}


qboolean PvsForOrigin (vec3_t org, byte *pvs)
{
	dleaf_t	*leaf;

	if (!visdatasize)
	{
		memset (pvs, 255, (numleafs+7)/8 );
		return true;
	}

	leaf = PointInLeaf (org);
	if (leaf->cluster == -1)
		return false;		// in solid leaf

	DecompressVis (dvisdata + dvis->bitofs[leaf->cluster][DVIS_PVS], pvs);
	return true;
}


typedef struct tnode_s
{
	int		type;
	vec3_t	normal;
	float	dist;
	int		children[2];
	int		pad;
} tnode_t;

extern tnode_t		*tnodes;

int	total_transfer;

static long total_mem;

static int first_transfer = 1;

#define MAX_TRACE_BUF ((MAX_PATCHES + 7) / 8)

#define TRACE_BYTE(x) (((x)+7) >> 3)
#define TRACE_BIT(x) ((x) & 0x1F)

static byte trace_buf[MAX_TRACE_BUF + 1];
static byte trace_tmp[MAX_TRACE_BUF + 1];
static int trace_buf_size;

int CompressBytes (int size, byte *source, byte *dest)
{
	int		j;
	int		rep;
	byte	*dest_p;

	dest_p = dest + 1;

	for (j=0 ; j<size ; j++)
	{
		*dest_p++ = source[j];

        if((dest_p - dest - 1) >= size)
            {
            memcpy(dest+1, source, size);
            dest[0] = 0;

            return size + 1;
            }

		if (source[j])
			continue;

		rep = 1;
		for ( j++; j<size ; j++)
			if (source[j] || rep == 255)
				break;
			else
				rep++;
		*dest_p++ = rep;

        if((dest_p - dest - 1) >= size)
            {
            memcpy(dest+1, source, size);
            dest[0] = 0;

            return size + 1;
            }

		j--;

	}
	
    dest[0] = 1;
	return dest_p - dest;
}


void DecompressBytes (int size, byte *in, byte *decompressed)
{
	int		c;
	byte	*out;

    if(in[0] == 0) // not compressed
        {
        memcpy(decompressed, in + 1, size);

        return;
        }

	out = decompressed;
    in++;

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		if (!c)
			Error ("DecompressBytes: 0 repeat");
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < size);
}

static int trace_bytes = 0;

void MakeTransfers (int i)
{
	int			j;
	vec3_t		delta;
	vec_t		dist, inv_dist, scale;
	float		trans;
	int			itrans;
	patch_t		*patch, *patch2;
	float		total, inv_total;
	dplane_t	plane;
	vec3_t		origin;
	float		transfers[MAX_PATCHES], *all_transfers;
	int			s;
	int			itotal;
	byte		pvs[(MAX_MAP_LEAFS+7)/8];
	int			cluster;

    int calc_trace, test_trace;

    patch = patches + i;
	total = 0;

	VectorCopy (patch->origin, origin);
	plane = *patch->plane;

	if (!PvsForOrigin (patch->origin, pvs))
		return;

	// find out which patch2s will collect light
	// from patch

	all_transfers = transfers;
	patch->numtransfers = 0;

    calc_trace = (save_trace && memory && first_transfer);
    test_trace = (save_trace && memory && !first_transfer);

	if (calc_trace)
	{
		memset(trace_buf, 0, trace_buf_size);
	}
	else if (test_trace)
	{
		DecompressBytes(trace_buf_size, patch->trace_hit, trace_buf);
	}

	for (j = 0, patch2 = patches; j < num_patches; ++j, ++patch2)
	{
    	transfers[j] = 0;

		if (j == i)
			continue;

		// check pvs bit
		if (!nopvs)
		{
			cluster = patch2->cluster;

			if (cluster == -1)
				continue;

			if (!(pvs[cluster >> 3] & (1 << (cluster & 7))))
				continue;		// not in pvs
		}

        if(test_trace && !(trace_buf[TRACE_BYTE(j)] & TRACE_BIT(j)))
            continue;

		// calculate vector
		VectorSubtract(patch2->origin, origin, delta);
		//dist = VectorNormalize (delta, delta);

        // Not calling normalize function to save function call overhead
        dist = sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);

		if (dist == 0)
		{
			continue;
		}
		else
		{
			inv_dist = 1.0f / dist;
			delta[0] *= inv_dist;
			delta[1] *= inv_dist;
			delta[2] *= inv_dist;
		}

		// reletive angles
		scale = DotProduct(delta, plane.normal);
		scale *= -DotProduct(delta, patch2->plane->normal);
		if (scale <= 0)
			continue;

		// check exact transfer
		trans = scale * patch2->area * inv_dist * inv_dist;
		// jitodo - remove the inv_dist squared for sky.

//		if (trans < 0)
//			trans = 0;		// rounding errors.

		if (trans > patch_cutoff)
		{
            if(!test_trace && !noblock && TestLine_r (0, patch->origin, patch2->origin))
                {
                transfers[j] = 0;
    		    continue;
                }

    		transfers[j] = trans;

			total += trans;
			patch->numtransfers++;

    	}
	}

    // copy the transfers out and normalize
	// total should be somewhere near PI if everything went right
	// because partial occlusion isn't accounted for, and nearby
	// patches have underestimated form factors, it will usually
	// be higher than PI
	if (patch->numtransfers)
	{
		transfer_t	*t;

        if (patch->numtransfers < 0 || patch->numtransfers > MAX_PATCHES)
			Error ("Weird numtransfers");
		s = patch->numtransfers * sizeof(transfer_t);
		patch->transfers = malloc (s);

        total_mem += s;

		if (!patch->transfers)
			Error ("Memory allocation failure");

		//
		// normalize all transfers so all of the light
		// is transfered to the surroundings
		//
		t = patch->transfers;
		itotal = 0;

        inv_total = 65536.0f / total;

		for (j=0 ; j < num_patches; j++)
		{
			if (transfers[j] <= 0)
				continue;

			//itrans = transfers[j]*0x10000 / total;
			itrans = transfers[j]*inv_total;
			itotal += itrans;
			t->transfer = itrans;
			t->patch = j;
			t++;

            if(calc_trace)
                {
                trace_buf[TRACE_BYTE(j)] |= TRACE_BIT(j);
                }

		}

	}

    if(calc_trace)
        {
        j = CompressBytes(trace_buf_size, trace_buf, trace_tmp);
        patch->trace_hit = malloc(j);
        memcpy(patch->trace_hit, trace_tmp, j);

        trace_bytes += j;
        }

	// don't bother locking around this.  not that important.
	total_transfer += patch->numtransfers;
}




/*
=============
FreeTransfers
=============
*/
void FreeTransfers (void)
{
	int		i;

	for (i=0 ; i<num_patches ; i++)
	{
        if(!memory)
            {
		    free (patches[i].transfers);
		    patches[i].transfers = NULL;
            }
        else if(patches[i].trace_hit != NULL)
            {
		    free (patches[i].trace_hit);
		    patches[i].trace_hit = NULL;
            }
	}
}


//===================================================================

/*
=============
WriteWorld
=============
*/
void WriteWorld (char *name)
{
	int		i, j;
	FILE		*out;
	patch_t		*patch;
	winding_t	*w;

	out = fopen (name, "w");
	if (!out)
		Error ("Couldn't open %s", name);

	for (j=0, patch=patches ; j<num_patches ; j++, patch++)
	{
		w = patch->winding;
		fprintf (out, "%i\n", w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			fprintf (out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
				w->p[i][0],
				w->p[i][1],
				w->p[i][2],
				patch->totallight[0],
				patch->totallight[1],
				patch->totallight[2]);
		}
		fprintf (out, "\n");
	}

	fclose (out);
}

/*
=============
WriteGlView
=============
*/
void WriteGlView (void)
{
	char	name[1024];
	FILE	*f;
	int		i, j;
	patch_t	*p;
	winding_t	*w;

	strcpy (name, source);
	StripExtension (name);
	strcat (name, ".glr");

	f = fopen (name, "w");
	if (!f)
		Error ("Couldn't open %s", f);

	for (j=0 ; j<num_patches ; j++)
	{
		p = &patches[j];
		w = p->winding;
		fprintf (f, "%i\n", w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			fprintf (f, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
				w->p[i][0],
				w->p[i][1],
				w->p[i][2],
				p->totallight[0]/128,
				p->totallight[1]/128,
				p->totallight[2]/128);
		}
		fprintf (f, "\n");
	}

	fclose (f);
}


//==============================================================

/*
=============
CollectLight
=============
*/
float CollectLight (void)
{
	int		i, j;
	patch_t	*patch;
	vec_t	total;

	total = 0;

	for (i=0, patch=patches ; i<num_patches ; i++, patch++)
	{
		// skys never collect light, it is just dropped
		if (patch->sky)
		{
			VectorClear (radiosity[i]);
			VectorClear (illumination[i]);
			continue;
		}

		for (j=0 ; j<3 ; j++)
		{
			patch->totallight[j] += illumination[i][j] / patch->area;
			radiosity[i][j] = illumination[i][j] * patch->reflectivity[j];
		}

		total += radiosity[i][0] + radiosity[i][1] + radiosity[i][2];
		VectorClear (illumination[i]);
	}

	return total;
}


/*
=============
ShootLight

Send light out to other patches
  Run multi-threaded
=============
*/
int c_progress;
int p_progress;

void ShootLight (int patchnum)
{
	int			k, l;
	transfer_t	*trans;
	int			num;
	patch_t		*patch;
	vec3_t		send;

	// this is the amount of light we are distributing
	// prescale it so that multiplying by the 16 bit
	// transfer values gives a proper output value
	for (k=0 ; k<3 ; k++)
		send[k] = radiosity[patchnum][k] / 0x10000;
	patch = &patches[patchnum];

    if(memory)
        {
        c_progress = 10 * patchnum / num_patches;

        if(c_progress != p_progress)
            {
			printf ("%i...", c_progress);
            p_progress = c_progress;
            }

        MakeTransfers(patchnum);
        }

	trans = patch->transfers;
	num = patch->numtransfers;

    for (k=0 ; k<num ; k++, trans++)
	{
		for (l=0 ; l<3 ; l++)
			illumination[trans->patch][l] += send[l]*trans->transfer;
	}

    if(memory)
        {
		free (patches[patchnum].transfers);
		patches[patchnum].transfers = NULL;
        }
}

/*
=============
BounceLight
=============
*/
void BounceLight (void)
{
	int		i, j, start, stop;
	float	added;
	char	name[64];
	patch_t	*p;

	for (i=0 ; i<num_patches ; i++)
	{
		p = &patches[i];
		for (j=0 ; j<3 ; j++)
		{
//			p->totallight[j] = p->samplelight[j];
			radiosity[i][j] = p->samplelight[j] * p->reflectivity[j] * p->area;
		}
	}

    if(memory)
        trace_buf_size = (num_patches + 7) / 8;

	for (i=0 ; i<numbounce ; i++)
	{
        if(memory)
            {
            p_progress = -1;
            start = I_FloatTime();
            printf("[%d remaining]  ", numbounce - i);
            total_mem = 0;
            }

		RunThreadsOnIndividual (num_patches, false, ShootLight);

        first_transfer = 0;
        
        if(memory)
            {
            stop = I_FloatTime();
    		printf (" (%i)\n", stop-start);
            }

		added = CollectLight ();

		qprintf ("bounce:%i added:%f\n", i, added);
		if ( dumppatches && (i==0 || i == numbounce-1) )
		{
			sprintf (name, "bounce%i.txt", i);
			WriteWorld (name);
		}
	}
}



//==============================================================

void CheckPatches (void)
{
    int i;
	patch_t	*patch;

	for (i=0 ; i<num_patches ; i++)
	{
		patch = &patches[i];
		if (patch->totallight[0] < 0 || patch->totallight[1] < 0 || patch->totallight[2] < 0)
			Error ("negative patch totallight\n");
	}
}

/*
=============
RadWorld
=============
*/
void RadWorld (void)
{
	if (numnodes == 0 || numfaces == 0)
		Error ("Empty map");
	MakeBackplanes ();
	MakeParents (0, -1);
	MakeTnodes (&dmodels[0]);

	// turn each face into a single patch
	MakePatches ();

	// subdivide patches to a maximum dimension
	SubdividePatches ();

	// create directlights out of patches and lights
	CreateDirectLights ();

	// build initial facelights
	RunThreadsOnIndividual (numfaces, true, BuildFacelights);

	if (numbounce > 0)
	{

		// build transfer lists
		if(!memory)
            {
    		RunThreadsOnIndividual (num_patches, true, MakeTransfers);

		    qprintf ("transfer lists: %5.1f megs\n"
		        , (float)total_transfer * sizeof(transfer_t) / (1024*1024));
            }

		// spread light around
		BounceLight ();

        FreeTransfers ();

		CheckPatches ();
	}

    if(memory)
        {
        printf("Non-memory conservation would require %4.1f\n",
            (float)(total_mem - trace_bytes) / 1048576.0f);
        printf("    megabytes more memory then currently used\n");
        }

	if (glview)
		WriteGlView ();

	// blend bounced light into direct light and save
	PairEdges ();
	LinkPlaneFaces ();

	lightdatasize = 0;
	RunThreadsOnIndividual (numfaces, true, FinalLightFace);


}


/*
========
main

light modelfile
========
*/
int main (int argc, char **argv)
{
	int		n, full_help;
	double		start, end;
	char		name[1024];
    char		game_path[1024] = "";
    char *param, *param2;
	const char *mapname = NULL;

	printf ("----------- qrad3 ----------\n");
	printf ("original code by id Software\n");
    printf ("Modified by Geoffrey DeWan\n");
    printf ("Revision 1.05, jit r2 (2015-03-06a)\n");
    printf ("-----------------------------\n");

	verbose = false;
    full_help = false;

    LoadConfigurationFile("qrad3", 0);
    LoadConfiguration(argc-1, argv+1);

    while((param = WalkConfiguration()) != NULL)
	{
		mapname = param; // jit

		if (!strcmp(param,"-dump"))
			dumppatches = true;
		else if (!strcmp(param,"-gamedir"))
		{
            param2 = WalkConfiguration();
			strncpy(game_path, param2, 1024);
		}
		else if (!strcmp(param,"-moddir"))
		{
            param2 = WalkConfiguration();
			strncpy(moddir, param2, 1024);
		}
		else if (!strcmp(param,"-help"))
		{
			full_help = true;
		}
		else if (!strcmp(param,"-bounce"))
		{
            param2 = WalkConfiguration();
			numbounce = atoi (param2);
		}
		else if (!strcmp(param,"-v"))
		{
			verbose = true;
		}
		else if (!strcmp(param,"-extra"))
		{
			extrasamples = true;
			printf ("extrasamples = true\n");
		}
		else if (!strcmp(param,"-nocolor"))
		{
			nocolor = true;
			printf ("nocolor = true\n");
		}
		else if (!strcmp(param,"-threads"))
		{
            param2 = WalkConfiguration();
			numthreads = atoi (param2);
		}
		else if (!strcmp(param,"-chop"))
		{
            param2 = WalkConfiguration();
			subdiv = atoi (param2);
		}
		else if (!strcmp(param,"-radmin"))
		{
            param2 = WalkConfiguration();
			patch_cutoff = atof (param2);
			printf("radiosity minimum set to %f\n", patch_cutoff);
		}
		else if (!strcmp(param,"-scale"))
		{
            param2 = WalkConfiguration();
			lightscale = atoi (param2);
		}
		else if (!strcmp(param, "-texscale")) // jit - option to scale texture reflectivity
		{
			param2 = WalkConfiguration();
			g_texscale = atof(param2);
			printf("Texture reflectivity scale set to %g\n", g_texscale);
		}
		else if (!strcmp(param,"-direct"))
		{
            param2 = WalkConfiguration();
			direct_scale *= atof (param2);
			printf ("direct light scaling at %f\n", direct_scale);
		}
		else if (!strcmp(param,"-entity"))
		{
            param2 = WalkConfiguration();
			entity_scale *= atof(param2);
			printf ("entity light scaling at %f\n", entity_scale);
		}
		else if (!strcmp(param,"-glview"))
		{
			glview = true;
			printf ("glview = true\n");
		}
		else if (!strcmp(param,"-noblock"))
		{
			noblock = true;
			printf ("noblock = true\n");
		}
		else if (!strcmp(param,"-memory"))
		{
			memory = true;
			printf ("memory = true (threads forced to 1)\n");
		}
		else if (!strcmp(param,"-savetrace"))
		{
			memory = true;
			save_trace = true;
			printf ("savetrace = true (memory set to true, threads forced to 1)\n");
		}
		else if (!strcmp(param,"-nopvs"))
		{
			nopvs = true;
			printf ("nopvs = true\n");
		}
		else if (!strcmp(param,"-ambient"))
		{
            param2 = WalkConfiguration();
			ambient = atof (param2);
		}
		else if (!strcmp(param,"-maxlight"))
		{
            param2 = WalkConfiguration();
			maxlight = atof (param2) * 128.0f;
		}
		else if (!strcmp (param,"-tmpin"))
			strcpy (inbase, "/tmp");
		else if (!strcmp (param,"-tmpout"))
			strcpy (outbase, "/tmp");
		else if (param[0] == '+')
            LoadConfigurationFile(param+1, 1);
		/*else
			break;*/
	}

    if(memory)
        numthreads = 1;

	ThreadSetDefault ();

	if (maxlight > 255)
		maxlight = 255;
/*jit
    if(param != NULL)
        param2 = WalkConfiguration();

	printf("debug: param = %s, param2 = %s\n", param, param2);*/

	if (!mapname /*jit- param == NULL || param2 != NULL*/)
	{
		if(full_help)
		{
			printf ("usage: qrad3 [options] bspfile\n\n"
				"    -ambient #       -glview               -radmin #\n"
				"    -bounce #        -help                 -savetrace\n"
				"    -chop #          -maxlight #           -scale #\n"
				"    -direct #        -memory               -threads #\n"
				"    -dump            -moddir <path>        -tmpin\n"
				"    -entity #        -noblock              -tmpout\n"
				"    -extra           -nocolor              -v\n"
				"    -gamedir <path>  -nopvs                -texscale #\n"
				);

			exit(1);
		}
		else
		{
			Error ("usage: qrad3 [options] bspfile\n\n"
				"    qrad3 -help for full help\n");
		}
	}
/*jit-
    while(param2)  // make sure list is clean
        param2 = WalkConfiguration();*/

	start = I_FloatTime ();

	if(game_path[0] != 0)
	{
		n = strlen(game_path);

		if(n > 1 && n < 1023 && game_path[n-1] != '\\')
		{
			game_path[n] = '\\';
			game_path[n+1] = 0;
		}

		strcpy(gamedir, game_path);
	}
	else
		SetQdirFromPath(mapname/*jit- param*/);

    printf("gamedir set to %s\n", gamedir);

	if(moddir[0] != 0)
	{
		n = strlen(moddir);

		if(n > 1 && n < 1023 && moddir[n-1] != '\\')
		{
			moddir[n] = '\\';
			moddir[n+1] = 0;
		}

		printf("moddir set to %s\n", moddir);
	}

	strcpy(source, ExpandArg(mapname/*jit- param*/));
	StripExtension(source);
	DefaultExtension(source, ".bsp");

//	ReadLightFile ();

	sprintf(name, "%s%s", inbase, source);
	printf("reading %s\n", name);
	LoadBSPFile(name);
	ParseEntities();
	CalcTextureReflectivity();

	if (!visdatasize)
	{
		printf ("No vis information, direct lighting only.\n");
		numbounce = 0;
		ambient = 0.1;
	}

	RadWorld();

	sprintf (name, "%s%s", outbase, source);
	printf ("writing %s\n", name);
	WriteBSPFile (name);

	end = I_FloatTime ();
	printf ("%5.0f seconds elapsed\n", end-start);

	return 0;
}

