/* Gloom pmove
  Use this in clients to allow correct client side prediction.

  see gloompmove.h for usage details.  
*/

#warning "FIXME: Grapple release speed"
#warning "FIXME: Grapple retrigger delay"
#warning "FIXME: Increase grapple normal scale"
#warning "TODO: 2x2x2 grapple box again"

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

#include <math.h>
#include "gloompmove.h"

#ifndef CLASS_OBSERVER
enum {
  CLASS_GRUNT,
  CLASS_HATCHLING,
  CLASS_HEAVY,
  CLASS_COMMANDO,
  CLASS_DRONE,
  CLASS_MECH,
  CLASS_SHOCK,
  CLASS_STALKER,
  CLASS_BREEDER,
  CLASS_ENGINEER,
  CLASS_GUARDIAN,
  CLASS_KAMIKAZE,
  CLASS_EXTERM,
  CLASS_STINGER,
  CLASS_WRAITH,
  CLASS_BIO,
  CLASS_OBSERVER
};
#endif

static struct class_data_s {
  const char* skinkey;
  int16_t  jumpheight[3];  //negative values means continuous jump without release
  uint8_t  jumploop;
  int8_t   viewheight;
  int8_t   mins[3];
  int8_t   maxs[3];
  uint8_t  crouch;
  float    bounce;
  uint16_t speed_min;  //movement speed with minimal difference speed (for compatibility)
  uint16_t speed_max;  //movement speed with maximal difference speed (not recommended if clients without prediction are in game)
} classdata[] = {
  { //CLASS_GRUNT
    /*skinkey*/   "soldier",
    /*jumpheight*/ {270, 270, 270},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  300,
    /*speed_max*/  300,
  },
  { //CLASS_HATCHLING
    /*skinkey*/   "hatch/skin",
    /*jumpheight*/ {400, 512, -180},
    /*jumploop*/   0,
    /*vheight*/    -5,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, -8},
    /*crouch*/     0,
    /*bounce*/     0.5f,
    /*speed_min*/  340,
    /*speed_max*/  380,
  },
  { //CLASS_HEAVY
    /*skinkey*/   "hsold",
    /*jumpheight*/ {208, 208, 208},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  288,
    /*speed_max*/  280,
  },
  { //CLASS_COMMANDO
    /*skinkey*/   "commando",
    /*jumpheight*/ {320, 380, 400},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  320,
    /*speed_max*/  330,
  },
  { //CLASS_DRONE
    /*skinkey*/   "drone",
    /*jumpheight*/ {360, 540, 0},
    /*jumploop*/   0,
    /*vheight*/    16,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     0,
    /*bounce*/     0.5f,
    /*speed_min*/  320,
    /*speed_max*/  330,
  },
  { //CLASS_MECH
    /*skinkey*/   "mech",
    /*jumpheight*/ {72, 68, 0},
    /*jumploop*/   1,
    /*vheight*/    30,
    /*mins*/       {-24, -24, -24},
    /*maxs*/       {24, 24, 56},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  260,
    /*speed_max*/  280,
  },
  { //CLASS_SHOCK
    /*skinkey*/   "shotgun",
    /*jumpheight*/ {270, 270, 270},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  300,
    /*speed_max*/  300,
  },
  { //CLASS_STALKER
    /*skinkey*/   "stalker",
    /*jumpheight*/ {164, 144, 0},
    /*jumploop*/   1,
    /*vheight*/    30,
    /*mins*/       {-32, -32, -24},
    /*maxs*/       {32, 32, 40},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  300,
    /*speed_max*/  300,
  },
  { //CLASS_BREEDER
    /*skinkey*/   "breeder",
    /*jumpheight*/ {200, 200, 200},
    /*jumploop*/   0,
    /*vheight*/    2,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  308,
    /*speed_max*/  308,
  },
  { //CLASS_ENGINEER
    /*skinkey*/   "engineer",
    /*jumpheight*/ {256, 256, 256},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 40},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  308,
    /*speed_max*/  308,
  },
  { //CLASS_GUARDIAN
    /*skinkey*/   "guardian",
    /*jumpheight*/ {320, 400, 0},
    /*jumploop*/   1,
    /*vheight*/    11,
    /*mins*/       {-32, -32, -24},
    /*maxs*/       {32, 32, 24},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  308,
    /*speed_max*/  310,
  },
  { //CLASS_KAMIKAZE
    /*skinkey*/   "kami",
    /*jumpheight*/ {400, 512, -180},
    /*jumploop*/   0,
    /*vheight*/    -5,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, -8},
    /*crouch*/     0,
    /*bounce*/     0.5f,
    /*speed_min*/  340,
    /*speed_max*/  374,
  },
  { //CLASS_EXTERM
    /*skinkey*/   "exterm",
    /*jumpheight*/ {230, 230, 230},
    /*jumploop*/   0,
    /*vheight*/    26,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  290,
    /*speed_max*/  290,
  },
  { //CLASS_STINGER
    /*skinkey*/   "stinger",
    /*jumpheight*/ {386, 464, 496},
    /*jumploop*/   0,
    /*vheight*/    8,
    /*mins*/       {-24, -24, -24},
    /*maxs*/       {24, 24, 24},
    /*crouch*/     0,
    /*bounce*/     0.35f,
    /*speed_min*/  320,
    /*speed_max*/  330,
  },
  { //CLASS_WRAITH
    /*skinkey*/   "wraith",
    /*jumpheight*/ {380, 512, -204},
    /*jumploop*/   0,
    /*vheight*/    8,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 16},
    /*crouch*/     0,
    /*bounce*/     0.4f,
    /*speed_min*/  308,
    /*speed_max*/  310,
  },
  { //CLASS_BIO
    /*skinkey*/   "female",
    /*jumpheight*/ {270, 270, 270},
    /*jumploop*/   0,
    /*vheight*/    22,
    /*mins*/       {-16, -16, -24},
    /*maxs*/       {16, 16, 32},
    /*crouch*/     28,
    /*bounce*/     0,
    /*speed_min*/  300,
    /*speed_max*/  300,
  },
  { //CLASS_OBSERVER
    /*skinkey*/   NULL,
    /*jumpheight*/ {1, 1, 1},
    /*jumploop*/   0,
    /*vheight*/    0,
    /*mins*/       {-8, -8, -8},
    /*maxs*/       {8, 8, 8},
    /*crouch*/     0,
    /*bounce*/     0,
    /*speed_min*/  600,
    /*speed_max*/  650,
  }
};

enum {
  grapple_none,
  grapple_throw,
  grapple_pull,
  grapple_hold
};

#if 0
static inline void VectorClear(float*f) {f[0] = f[1] = f[2] = 0}
static inline void VectorCopy(float *src, float*dst) {dst[0] = src[0];dst[1] = src[1];dst[2] = src[2];}
static inline float VectorLength(float*f) {return sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);}
static inline void VectorMA(float *a, float s, float*b, float *d) {d[0] = a[0]+s*b[0]; d[1] = a[1]+s*b[1]; d[2] = a[2]+s*b[2];}
static inline void VectorScale(float *src, float s, float *dst) {d[0] = src[0]*s;d[1] = src[1]*s;d[2] = src[2]*s;}
static inline float VectorNormalize(float*f){float l = VectorLength(f); if (l > 0) VectorScale(f, 1.0f/l, f); return l;}
#endif

#ifndef BIT
#define BIT(b) (1 << (b))
#endif


#define	STEPSIZE	18

#ifndef FLOAT_GT_ZERO
#if 0
#define FLOAT_GT_ZERO(f) (f > +0.0001f)
#define FLOAT_LT_ZERO(f) (f < -0.0001f)
#define FLOAT_GE_ZERO(f) (f > -0.0001f)
#define FLOAT_LE_ZERO(f) (f < +0.0001f)
#define FLOAT_EQ_ZERO(f) (fabs(f) < +0.0001f)
#else
#define	FLOAT_LT_ZERO(f) ((f) < 0)
#define FLOAT_LE_ZERO(f) ((f) <= 0)
#define FLOAT_GT_ZERO(f) ((f) > 0)
#define FLOAT_GE_ZERO(f) ((f) >= 0)
#define	FLOAT_EQ_ZERO(f) ((f) == 0)
#define	FLOAT_NE_ZERO(f) ((f) != 0)
#endif
#endif

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

typedef struct
{
	vec3_t		origin;			// full float precision
	vec3_t		velocity;		// full float precision

	vec3_t		forward, right, up;
	float		frametime;


	csurface_t	*groundsurface;
	cplane_t	groundplane;
	int		groundcontents;
	entity_state_t* groundtracedent;
	

//	vec3_t		previous_origin[3];
	qboolean	ladder;
	
	float maxspeed;
} pml_t;

static pmove_t*pm;
static pml_t	   pml;
static struct class_data_s*cd;
static gloom_pmove_t*gpm;
static gloom_pmove_state_t*gpms;

// movement parameters
#define		pm_stopspeed 100.0f
#define		pm_maxspeed pml.maxspeed   //was 300.0f
#define		pm_duckspeed 100.0f
#define		pm_accelerate 10.0f
#define		pm_airaccelerate (GloomPMoveIsAirAccelerateEnabled())
#define		pm_wateraccelerate 10.0f
#define		pm_friction 6.0f
#define		pm_waterfriction  1.0f
#define		pm_waterspeed 400.0f


vec3_t grapple_mins = {-0.25f, -0.25f, -0.25f};
vec3_t grapple_maxs = {0.25f, 0.25f, 0.25f};

/*

  walking up a step should kill some velocity

*/


/*
==================
GPM_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1f

static void GPM_ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
//		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
//			out[i] = 0;
	}
}




/*
==================
GPM_StepSlideMove

Each intersection will try to step over the obstruction instead of
sliding along it.

Returns a new origin, velocity, and contact entity
Does not modify any world state?
==================
*/
#define	MIN_STEP_NORMAL	0.7f		// can't step up onto very steep slopes

#define	MAX_CLIP_PLANES	5

static void GPM_StepSlideMove_ (void)
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	int			i, j;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	
	numbumps = 4;
	
	VectorCopy (pml.velocity, primal_velocity);
	numplanes = 0;
	
	time_left = pml.frametime;

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		end[0] = pml.origin[0] + time_left * pml.velocity[0];
		end[1] = pml.origin[1] + time_left * pml.velocity[1];
		end[2] = pml.origin[2] + time_left * pml.velocity[2];

		trace = pm->trace (pml.origin, pm->mins, pm->maxs, end);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			pml.velocity[2] = 0;	// don't build up falling damage
			return;
		}

		if (FLOAT_GT_ZERO(trace.fraction))
		{	// actually covered some distance
			VectorCopy (trace.endpos, pml.origin);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance
			 
		// save entity for contact
		if (pm->numtouch < MAXTOUCH && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorClear (pml.velocity);
//			PMPrint("MAX_CLIP_PLANES\n");
			break;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i=0 ; i<numplanes ; i++)
		{
			GPM_ClipVelocity (pml.velocity, planes[i], pml.velocity, 1.01f);
			for (j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct (pml.velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	// go along this plane
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				PMPrintf ("clip velocity, numplanes == %i\n",numplanes);
				VectorClear (pml.velocity);
				break;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, pml.velocity);
			VectorScale (dir, d, pml.velocity);
		}

		//
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (DotProduct (pml.velocity, primal_velocity) <= 0)
		{
			VectorClear (pml.velocity);
			break;
		}
	}

	if (pm->s.pm_time)
	{
		VectorCopy (primal_velocity, pml.velocity);
	}
}

/*
==================
GPM_StepSlideMove

==================
*/
static void GPM_StepSlideMove (void)
{
	vec3_t		start_o, start_v;
	vec3_t		down_o, down_v;
	trace_t		trace;
	float		down_dist, up_dist;
//	vec3_t		delta;
	vec3_t		up, down;



	VectorCopy (pml.origin, start_o);
	VectorCopy (pml.velocity, start_v);

	GPM_StepSlideMove_ ();

	VectorCopy (pml.origin, down_o);
	VectorCopy (pml.velocity, down_v);

	VectorCopy (start_o, up);
	up[2] += STEPSIZE;

	trace = pm->trace (up, pm->mins, pm->maxs, up);
	if (trace.allsolid)
		return;		// can't step up

	// try sliding above
	VectorCopy (up, pml.origin);
	VectorCopy (start_v, pml.velocity);

	GPM_StepSlideMove_ ();

	// push down the final amount
	VectorCopy (pml.origin, down);
	down[2] -= STEPSIZE;
	trace = pm->trace (pml.origin, pm->mins, pm->maxs, down);
	if (!trace.allsolid)
	{
		VectorCopy (trace.endpos, pml.origin);
	}

#if 0
	VectorSubtract (pml.origin, up, delta);
	up_dist = DotProduct (delta, start_v);

	VectorSubtract (down_o, start_o, delta);
	down_dist = DotProduct (delta, start_v);
#else
	VectorCopy(pml.origin, up);

	// decide which one went farther
    down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0])
        + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
    up_dist = (up[0] - start_o[0])*(up[0] - start_o[0])
        + (up[1] - start_o[1])*(up[1] - start_o[1]);
#endif

#warning "FIXME: jitter climbing"
/*
	PMPrintf("%.1f[%.1f]down, %.1f[%.1f]up,  %.5f\n",
	  down_dist, down_o[2] - start_o[2],
	  up_dist, up[2] - start_o[2],
	  trace.plane.normal[2]
	);
*/
	if (down_dist > up_dist || trace.plane.normal[2] < MIN_STEP_NORMAL)
	{
		VectorCopy (down_o, pml.origin);
		VectorCopy (down_v, pml.velocity);
		return;
	}

	//!! Special case
	// if we were walking along a plane, then we need to copy the Z over
	pml.velocity[2] = down_v[2];
}


/*
==================
GPM_Friction

Handles both ground friction and water friction
==================
*/
static void GPM_Friction (void)
{
	float	*vel;
	float	speed, newspeed, control;
	float	friction;
	float	drop;
	
	vel = pml.velocity;
	
	speed = (float)sqrt(vel[0]*vel[0] +vel[1]*vel[1] + vel[2]*vel[2]);
	if (speed < 1)
	{
		vel[0] = 0;
		vel[1] = 0;
		return;
	}

	drop = 0;

// apply ground friction
	if ((pm->groundentity && pml.groundsurface && !(pml.groundsurface->flags & SURF_SLICK) ) || (pml.ladder) )
	{
		friction = pm_friction;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;
	}

// apply water friction
	if (pm->waterlevel && !pml.ladder)
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;

// scale the velocity
	newspeed = speed - drop;
	if (FLOAT_LT_ZERO(newspeed))
	{
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
GPM_Accelerate

Handles user intended acceleration
==============
*/
static void GPM_Accelerate (vec3_t wishdir, float wishspeed, float accel)
{
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (FLOAT_LE_ZERO (addspeed))
		return;

	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity[0] += accelspeed*wishdir[0];
	pml.velocity[1] += accelspeed*wishdir[1];
	pml.velocity[2] += accelspeed*wishdir[2];
}

static void GPM_AirAccelerate (vec3_t wishdir, float wishspeed, float accel)
{
	float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;
		
	if (wishspd > 30)
		wishspd = 30;
	currentspeed = DotProduct (pml.velocity, wishdir);
	addspeed = wishspd - currentspeed;

	if (FLOAT_LE_ZERO(addspeed))
		return;

	accelspeed = accel * wishspeed * pml.frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity[0] += accelspeed*wishdir[0];
	pml.velocity[1] += accelspeed*wishdir[1];
	pml.velocity[2] += accelspeed*wishdir[2];
}

/*
=============
GPM_AddCurrents
=============
*/
static void GPM_AddCurrents (vec3_t	wishvel)
{
	vec3_t	v;
	float	s;

	//
	// account for ladders
	//

	if (pml.ladder && fabs(pml.velocity[2]) <= 200)
	{
		if ((pm->viewangles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = 200;
		else if ((pm->viewangles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = -200;
		else if (pm->cmd.upmove > 0)
			wishvel[2] = 200;
		else if (pm->cmd.upmove < 0)
			wishvel[2] = -200;
		else
			wishvel[2] = 0;

		// limit horizontal speed when on a ladder
		if (wishvel[0] < -25)
			wishvel[0] = -25;
		else if (wishvel[0] > 25)
			wishvel[0] = 25;

		if (wishvel[1] < -25)
			wishvel[1] = -25;
		else if (wishvel[1] > 25)
			wishvel[1] = 25;
	}


	//
	// add water currents
	//

	if (pm->watertype & MASK_CURRENT)
	{
		VectorClear (v);

		if (pm->watertype & CONTENTS_CURRENT_0)
			v[0] += 1;
		if (pm->watertype & CONTENTS_CURRENT_90)
			v[1] += 1;
		if (pm->watertype & CONTENTS_CURRENT_180)
			v[0] -= 1;
		if (pm->watertype & CONTENTS_CURRENT_270)
			v[1] -= 1;
		if (pm->watertype & CONTENTS_CURRENT_UP)
			v[2] += 1;
		if (pm->watertype & CONTENTS_CURRENT_DOWN)
			v[2] -= 1;

		s = pm_waterspeed;
		if ((pm->waterlevel == 1) && (pm->groundentity))
			s /= 2;

		VectorMA (wishvel, s, v, wishvel);
	}

	//
	// add conveyor belt velocities
	//

	if (pm->groundentity)
	{
		VectorClear (v);

		if (pml.groundcontents & CONTENTS_CURRENT_0)
			v[0] += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_90)
			v[1] += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_180)
			v[0] -= 1;
		if (pml.groundcontents & CONTENTS_CURRENT_270)
			v[1] -= 1;
		if (pml.groundcontents & CONTENTS_CURRENT_UP)
			v[2] += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_DOWN)
			v[2] -= 1;

		VectorMA (wishvel, 100 /* pm->groundentity->speed */, v, wishvel);
	}
}


/*
===================
GPM_WaterMove

===================
*/
static void GPM_WaterMove (void)
{
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;

//
// user intentions
//
	wishvel[0] = pml.forward[0]*pm->cmd.forwardmove + pml.right[0]*pm->cmd.sidemove;
	wishvel[1] = pml.forward[1]*pm->cmd.forwardmove + pml.right[1]*pm->cmd.sidemove;
	wishvel[2] = pml.forward[2]*pm->cmd.forwardmove + pml.right[2]*pm->cmd.sidemove;

	if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += pm->cmd.upmove;

	GPM_AddCurrents (wishvel);

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > pm_maxspeed)
	{
		VectorScale (wishvel, pm_maxspeed/wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}
	wishspeed *= 0.5f;

	GPM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	GPM_StepSlideMove ();
}


static void GPM_WraithMove()
{
  float speed, z;
  vec3_t dir;
  int i;

  for (i = 0; i < 2; i++)
    dir[i] = (pml.forward[i]*pm->cmd.forwardmove + pml.right[i]*pm->cmd.sidemove)*2.7;
  dir[2] = 0;
  
  speed = VectorLength(dir);
  if (speed > 400) VectorScale(dir, 400/speed, dir);
  
  for (i = 0; i < 2; i++) {
    if (dir[i] < 0 && pml.velocity[i] < dir[i]) {
      dir[i] -= pml.velocity[i];
      if (dir[0] > 0) dir[i] = 0;
    }
    if (dir[i] > 0 && pml.velocity[i] > dir[i]) {
      dir[i] -= pml.velocity[i];
      if (dir[0] < 0) dir[i] = 0;
    }
  }

  dir[2] = (pml.forward[2]*pm->cmd.forwardmove + pml.right[2]*pm->cmd.sidemove + pm->cmd.upmove)*5;
  if (dir[2] < -500) dir[2] = -500;
  if (dir[2] > 550-pm->s.gravity*0.25f) dir[2] = 550-pm->s.gravity*0.25f;
  
  //save z speed for special friction
  z = pml.velocity[2];
  pml.velocity[2] = 0;
  
  //initial accel boost
  speed = VectorLength(pml.velocity);
  if (speed < 250)
    VectorScale(dir, 1+(250-speed)*0.005f, dir);

  //xy air friction
  speed = VectorNormalize(pml.velocity);
  if (speed > 0.125f) {
    float newspeed;
    float dl = VectorLength(dir);
    float decel = 1-DotProduct(pml.velocity, dir)/((dl>1)?dl:1);
    if (decel < 0) decel = 0; //happened in some rare cases (floating point precision errors?)
    newspeed = speed-(125+decel*25)*pml.frametime;
    if (newspeed < 0) newspeed = decel = 0;
    VectorScale(pml.velocity, newspeed, pml.velocity);
    VectorScale(dir, (1+decel*0.125f), dir);
  } else
    pml.velocity[0] = pml.velocity[1] = 0;

  //z air friction
  speed = fabs(z);
  if (speed > 0.125f) {
    float decel = fabs(z-dir[2])*0.125f-16;
    float newspeed = speed-(250+((decel<60)?(decel>0)?decel:0:60))*pml.frametime;
    if (newspeed < 0) newspeed = 0;
    pml.velocity[2] = z*(newspeed/speed);
  }
  
  VectorMA(pml.velocity, pml.frametime, dir, pml.velocity);
  
  //brake if going too fast on xy plane
  speed = pml.velocity[0]*pml.velocity[0] + pml.velocity[1]*pml.velocity[1];
  if (speed > 400*400) {
    float s = sqrt(speed);
    float newspeed = s-250*pml.frametime;
    if (newspeed < 0) newspeed = 0;
    s = newspeed/s;
    pml.velocity[0] *= s;
    pml.velocity[1] *= s;
  }
  
  //brake if going too fast against gravity (FIXME: Assumes gravity is downwards)
  if (pml.velocity[2] > 400) {
    pml.velocity[2] -= (pm->s.gravity + pml.velocity[2]-400)*pml.frametime;
    if (pml.velocity[2] < 0) pml.velocity[2] = 0;
  }

  GPM_StepSlideMove ();
}


static float clamp01(float n)
{
  if (n < 0) n = 0;
  else
  if (n > 1) n = 1;
  return n;
}

/*
===================
GPM_AirMove

===================
*/
static void GPM_AirMove (void)
{
	vec3_t  wishvel;
	float  fmove, smove;
	vec3_t wishdir;
	float  wishspeed;
	float  maxspeed;
	float  accel = pm_accelerate;
	float  gravity = pm->s.gravity;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	

	//for (i=0 ; i<2 ; i++)
	wishvel[0] = pml.forward[0]*fmove + pml.right[0]*smove;
	wishvel[1] = pml.forward[1]*fmove + pml.right[1]*smove;
	wishvel[2] = 0;
	
	if (pm->s.pm_type == PM_WRAITH) {
  	  float v = pml.forward[2]*pm->cmd.forwardmove + pml.right[2]*pm->cmd.sidemove + pm->cmd.upmove;
	  if (fabs(v) > 0.125f && VectorLength(wishvel) < 0.125f) { //unstick from floor
	    wishvel[0] = pml.forward[0];
	    wishvel[1] = pml.forward[1];
	  }
	  wishvel[2] = v;
  	} else
	if (pm->s.pm_type == PM_JETPACK && pm->cmd.upmove > 0) {
	  gravity = pml.velocity[2]-350;
	  if (gravity < -350) gravity = -350;
	  else
	  if (gravity > 0) gravity = 0;
	} else
	if (pml.groundplane.normal[2] >= 0 && pml.groundplane.normal[2] < 0.7f) {
	  float sx = clamp01(1-fabs(pml.groundplane.normal[0]*4));
	  float sy = clamp01(1-fabs(pml.groundplane.normal[1]*4));
	  wishvel[0] = wishvel[0]*sx + pml.velocity[0]*(1-sx);
	  wishvel[1] = wishvel[1]*sy + pml.velocity[1]*(1-sy);
//	  gi.dprintf("%.3f %.3f\n", sx, sy);
	}

	GPM_AddCurrents (wishvel);

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

//
// clamp to server defined max speed
//
        maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if (wishspeed > maxspeed)
	{
		VectorScale (wishvel, maxspeed/wishspeed, wishvel);
		wishspeed = maxspeed;
	}
	
	if ( pml.ladder )
	{
		GPM_Accelerate (wishdir, wishspeed, pm_accelerate);
		if (FLOAT_EQ_ZERO(wishvel[2]))
		{
			if (FLOAT_GT_ZERO(pml.velocity[2]))
			{
				pml.velocity[2] -= gravity * pml.frametime;
				if (FLOAT_LT_ZERO(pml.velocity[2]))
					pml.velocity[2]  = 0;
			}
			else
			{
				pml.velocity[2] += gravity * pml.frametime;
				if (FLOAT_GT_ZERO (pml.velocity[2]))
					pml.velocity[2]  = 0;
			}
		}
		GPM_StepSlideMove ();
	}
	else if ( pm->groundentity)
	{	// walking on ground
	
		  pml.velocity[2] = 0; //!!! this is before the accel

		GPM_Accelerate (wishdir, wishspeed, accel);

		if(gravity < 0)
			pml.velocity[2] -= gravity * pml.frametime;

		if (FLOAT_EQ_ZERO(pml.velocity[0]) && FLOAT_EQ_ZERO(pml.velocity[1]))
			return;
		GPM_StepSlideMove ();
	}
	else
	{	// not on ground, so little effect on velocity
		if (pm_airaccelerate)
			GPM_AirAccelerate (wishdir, wishspeed, accel);
		else
			GPM_Accelerate (wishdir, wishspeed, 1);
		// add gravity
		pml.velocity[2] -= gravity * pml.frametime;
		GPM_StepSlideMove ();
	}
}



/*
=============
GPM_CatagorizePosition
=============
*/
static void GPM_CatagorizePosition (void)
{
	vec3_t		point;
	int			cont;
	trace_t		trace;
	float		sample1;
	float		sample2;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid	
	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] - 0.25f;
	if (pml.velocity[2] > 180) //!!ZOID changed from 100 to 180 (ramp accel)
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundentity = NULL;
	}
	else
	{
		trace = pm->trace (pml.origin, pm->mins, pm->maxs, point);
		pml.groundplane = trace.plane;
		pml.groundsurface = trace.surface;
		pml.groundcontents = trace.contents;
		pml.groundtracedent = (entity_state_t*)trace.ent;

		if (!trace.ent || (trace.plane.normal[2] < 0.7f && !trace.startsolid) )
		{
			pm->groundentity = NULL;
			pm->s.pm_flags &= ~PMF_ON_GROUND;
		}
		else
		{
			pm->groundentity = trace.ent;

			// hitting solid ground will end a waterjump
			if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
			{
				pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
				pm->s.pm_time = 0;
			}

			if (! (pm->s.pm_flags & PMF_ON_GROUND) )
			{	// just hit the ground
				pm->s.pm_flags |= PMF_ON_GROUND;
				// don't do landing time if we were just going down a slope
				if (pml.velocity[2] < -200 && !1/* && !pm->strafehack*/)
				{
					//Com_Printf ("Zoink! ms=%d vel = %f\n", LOG_GENERAL, pm->cmd.msec, pml.velocity[2]);
					pm->s.pm_flags |= PMF_TIME_LAND;
					// don't allow another jump for a little while
					if (pml.velocity[2] < -400)
						pm->s.pm_time = 25;	
					else
						pm->s.pm_time = 18;
				}
			}
		}

#if 0
		if (trace.fraction < 1.0f && trace.ent && FLOAT_LT_ZERO(pml.velocity[2]))
			pml.velocity[2] = 0;
#endif

		if (pm->numtouch < MAXTOUCH && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
	}

//
// get waterlevel, accounting for ducking
//
	pm->waterlevel = 0;
	pm->watertype = 0;

	sample2 = pm->viewheight - pm->mins[2];
	sample1 = sample2 / 2;

	point[2] = pml.origin[2] + pm->mins[2] + 1;	
	cont = pm->pointcontents (point);

	if (cont & MASK_WATER)
	{
		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pml.origin[2] + pm->mins[2] + sample1;
		cont = pm->pointcontents (point);
		if (cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			point[2] = pml.origin[2] + pm->mins[2] + sample2;
			cont = pm->pointcontents (point);
			if (cont & MASK_WATER)
				pm->waterlevel = 3;
		}
	}

}


/*
=============
GPM_CheckJump
=============
*/
static void GPM_LateJump (void)
{
	float h, oldzv;
	trace_t tr;
	vec3_t jp;

	if (pm->s.pm_type != PM_NORMAL || gpms->grapple_state >= grapple_pull) return; //Cant jump while flying or grappling

	if (pm->s.pm_flags & PMF_TIME_LAND)
	{	// hasn't been long enough since landing to jump again
		return;
	}

	if (pm->waterlevel > 1) gpms->jump_index = 0;

	if (pm->cmd.upmove < 10)
	{	// not holding jump
		pm->s.pm_flags &= ~PMF_JUMP_HELD;

		if (pm->groundentity) gpms->jump_index = 0;

		return;
	}

	// must wait for jump to be released
	if (pm->s.pm_flags & PMF_JUMP_HELD)
		return;

	if (pm->s.pm_type == PM_DEAD || pm->s.pm_type == PM_GIB)
		return;

	if (pm->waterlevel >= 2)
	{	// swimming, not jumping
		pm->groundentity = NULL;

		if (pml.velocity[2] <= -300)
			return;

		if (pm->watertype == CONTENTS_WATER)
			pml.velocity[2] = 100;
		else if (pm->watertype == CONTENTS_SLIME)
			pml.velocity[2] = 80;
		else
			pml.velocity[2] = 50;
		return;
	}

	if (pm->groundentity == NULL) {// in air, so no effect
	  if (VectorLength(pml.velocity) < 0.01f) {
	    vec3_t end = {pml.origin[0], pml.origin[1], pml.origin[2]-2};
	    vec3_t start, xy;
	    trace_t tr2;

	    tr = pm->trace(pml.origin, pm->mins, pm->maxs, end);
	    if (tr.fraction > 0.75f) return;

            if (tr.plane.normal[2] >= 0.5f) {
	      xy[0] = tr.plane.normal[0];
	      xy[1] = tr.plane.normal[1];
	      xy[2] = 0;
	      VectorNormalize(xy);
	      VectorScale(xy, 2, xy);
	      VectorAdd(pml.origin, xy, start);
	      VectorAdd(end, xy, end);
	      tr2 = pm->trace(start, pm->mins, pm->maxs, end);
	    
	      if (!tr2.allsolid && tr2.fraction > tr.fraction) return; //if can start movign nextframe downwards, dont allow to jump
	    }

	    //stuck on two inclinated planes, allow to jump
	  } else
	    return;
	}

        h = cd->jumpheight[gpms->jump_index];
        if (h > 0)
          pm->s.pm_flags |= PMF_JUMP_HELD;
        else
          h = -h;

	oldzv = pml.velocity[2];
	pml.velocity[2] += h;
	if (pml.velocity[2] < h)
  	  pml.velocity[2] = h;

        if (++gpms->jump_index >= 3 || !cd->jumpheight[gpms->jump_index])
          gpms->jump_index = cd->jumploop;
       
	VectorCopy(pml.origin, jp); jp[2] += h*pml.frametime;
        tr = pm->trace(pml.origin, pm->mins, pm->maxs, jp);
        
        if (tr.fraction < 0.75f && tr.endpos[2]-pml.origin[2] < 8) tr.allsolid = true; //no room to jump

        if (!tr.allsolid) { //can jump
          //prestep some of the jump to keep with client prediction now that is done after movement, as client expect going up this frame, not next
          VectorCopy(tr.endpos, pml.origin);
          h -= tr.fraction*(h+0.5f*pm->s.gravity*tr.fraction)*pml.frametime;

          //is jumping
          gpm->sounds |= GPMS_JUMP;
	  pm->groundentity = NULL;
	  pm->s.pm_flags &= ~PMF_ON_GROUND;
        } else {
          //undo jump attempt and require a release for next one always
          pm->s.pm_flags |= PMF_JUMP_HELD;
	  pml.velocity[2] = oldzv;
        }
}


/*
=============
GPM_CheckSpecialMovement
=============
*/
static void GPM_CheckSpecialMovement (void)
{
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;
	trace_t	trace;

	if (pm->s.pm_time)
		return;

	pml.ladder = false;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pml.origin, 1, flatforward, spot);
	trace = pm->trace (pml.origin, pm->mins, pm->maxs, spot);
	if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
		pml.ladder = true;

	// check for water jump
	if (pm->waterlevel != 2)
		return;

	VectorMA (pml.origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot);
	if (!(cont & CONTENTS_SOLID))
		return;

	spot[2] += 16;
	cont = pm->pointcontents (spot);
	if (cont)
		return;
	// jump out of water
	VectorScale (flatforward, 50, pml.velocity);
	pml.velocity[2] = 350;

	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}


/*
===============
GPM_FlyMove
===============
*/
//void GPM_FlyMove (qboolean doclip)
static void GPM_FlyMove (void)
{
	float	speed, drop, friction, control, newspeed;
	float	currentspeed, addspeed, accelspeed;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	//vec3_t		end;
	//trace_t	trace;

	// friction

	speed = VectorLength (pml.velocity);
	if (speed < 1)
	{
		VectorClear (pml.velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5f;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (FLOAT_LT_ZERO(newspeed))
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pml.velocity, newspeed, pml.velocity);
	}

	// accelerate
	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	wishvel[0] = (pml.forward[0]*fmove + pml.right[0]*smove);
	wishvel[1] = (pml.forward[1]*fmove + pml.right[1]*smove);
	wishvel[2] = (pml.forward[2]*fmove + pml.right[2]*smove);

	wishvel[2] += pm->cmd.upmove*300/pm_maxspeed;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	//
	// clamp to server defined max speed
	//
	if (wishspeed > pm_maxspeed)
	{
		VectorScale (wishvel, pm_maxspeed/wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}


	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (FLOAT_LE_ZERO (addspeed))
		return;
	accelspeed = pm_accelerate*pml.frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity[0] += accelspeed*wishdir[0];
	pml.velocity[1] += accelspeed*wishdir[1];
	pml.velocity[2] += accelspeed*wishdir[2];

	VectorMA (pml.origin, pml.frametime, pml.velocity, pml.origin);
}


static qboolean GPM_NearFloor(float distance)
{
  vec3_t end = {pml.origin[0], pml.origin[1], pml.origin[2]-distance};
  trace_t tr = pm->trace(pml.origin, pm->mins, pm->maxs, end);
  return tr.fraction < 1;
}


/*
==============
GPM_CheckDuck

Sets mins, maxs, and pm->viewheight
==============
*/
static void GPM_CheckDuck (void)
{
	float maxstep = 18; //maximum height of a step can go down while staying crouched

	trace_t	trace;
	struct class_data_s* lcd = cd;
	
	if (gpms->flags & GPMF_GRUNTBOX_SMALL && (lcd->maxs[0] < 16 || lcd->maxs[2] < 32))
	  lcd = classdata; //grunt
	else
	if (gpms->flags & GPMF_GRUNTBOX_BIG && (lcd->maxs[0] > 16 || lcd->maxs[2] > 32))
	  lcd = classdata; //grunt

	pm->mins[0] = lcd->mins[0];
	pm->mins[1] = lcd->mins[1];
	pm->mins[2] = lcd->mins[2];

	pm->maxs[0] = lcd->maxs[0];
	pm->maxs[1] = lcd->maxs[0];
	pm->maxs[2] = lcd->mins[2]+1; //for GPM_TouchingFloor


	if (pm->s.pm_type == PM_GIB) {
		pm->mins[2] = 0;
		pm->maxs[2] = 16;
		pm->viewheight = 8;
		return;
	}

	pm->mins[2] = lcd->mins[2];
	if (pm->s.pm_type == PM_DEAD || pm->s.pm_type == PM_GIB) {
	  pm->s.pm_flags |= PMF_DUCKED;
	} else
	if (pm->cmd.upmove < 0 && VectorLength(pml.velocity) < pm_maxspeed+1 && GPM_NearFloor(maxstep))
  	  pm->s.pm_flags |= PMF_DUCKED;
	else
	if (pm->s.pm_flags & PMF_DUCKED) {	// stand up if possible
		pm->maxs[2] = lcd->maxs[2];// try to stand up
		trace = pm->trace (pml.origin, pm->mins, pm->maxs, pml.origin);
		if (!trace.allsolid)
			pm->s.pm_flags &= ~PMF_DUCKED;
	}

        pm->maxs[2] = lcd->maxs[2];
        pm->viewheight = cd->viewheight;
	if (pm->s.pm_flags & PMF_DUCKED && cd->crouch > 0){
	  pm->maxs[2] -= lcd->crouch;
	  pm->viewheight -= round(cd->crouch*0.85f);
	}

	if (pm->s.pm_type == PM_DEAD || pm->s.pm_type == PM_GIB)
	  pm->viewheight = (int)(lcd->maxs[2]-lcd->mins[2])/2;

        if (gpms->grapple_state && !(pm->s.pm_flags & PMF_ON_GROUND) && gpms->grapple_dir[2] > 0.5f && !GPM_NearFloor(4))
	  pm->viewheight -= 3; //Ugly hack to prevent hatchie from wallhacking on ceiling
}


/*
==============
GPM_DeadMove
==============
*/
static void GPM_DeadMove (void)
{
	float	forward;

	if (!pm->groundentity)
		return;

	// extra friction

	forward = VectorLength (pml.velocity);
	forward -= 20;
	if (FLOAT_LE_ZERO(forward))
	{
		VectorClear (pml.velocity);
	}
	else
	{
		VectorNormalize (pml.velocity);
		VectorScale (pml.velocity, forward, pml.velocity);
	}
}


static qboolean	GPM_GoodPosition (void)
{
	trace_t	trace;
	vec3_t	origin, end;

	if (pm->s.pm_type == PM_SPECTATOR)
		return true;

	origin[0] = end[0] = pm->s.origin[0]*0.125f;
	origin[1] = end[1] = pm->s.origin[1]*0.125f;
	origin[2] = end[2] = pm->s.origin[2]*0.125f;
	trace = pm->trace (origin, pm->mins, pm->maxs, end);

	return !trace.allsolid;
}


static inline float PClamp(float f)
{
  return round(f*128.0f)/128.0f;
}


/*
================
GPM_SnapPosition

On exit, the origin will have a value that is pre-quantized to the 0.125
precision of the network channel and in a valid position.
================
*/
static void GPM_SnapPosition(gloom_pmove_t*gpm)
{
	int		sign[3];
	int		i, j, bits;
	int16_t	base[3];
	// try all single bits first
	static int jitterbits[8] = {0,4,1,2,3,5,6,7};
	float oldz = gpm->ent_origin[2];


	//not really snapping position, but must be always too	
	gpm->layout_bits = (gpms->jump_index << 2); //jump data
	
        pm->s.pm_flags &= ~(PMF_NO_PREDICTION | PMF_TIME_LAND);
	if (gpms->grapple_state > grapple_throw) {
          pm->s.pm_flags |= PMF_NO_PREDICTION; //stop plain q2 clients from jittering
	  gpm->layout_bits |= 256; //force prediction on, mark client that grapple is pulling or holding
	  if (gpms->grapple_state == grapple_hold)
	    pm->s.pm_flags |= PMF_TIME_LAND;
	}

	if (pm->s.pm_type >= 8 && pm->s.pm_type < 15) {
	  gpm->layout_bits |= (pm->s.pm_type-7) << 4;
	  gpm->pmove->s.pm_type = PM_NORMAL; //movetypes 8 to 14 will be predicted in old clients as PM_NORMAL, and sent into layout bits
	}
	

        //set z to high accurancy value, which might be overwritten later
        gpm->ent_origin[2] = PClamp(pml.origin[2]); //z needs better accurancy to keep jump height unaffected by cl_maxfps

	//fp velocity	
	gpm->ent_velocity[0] = PClamp(pml.velocity[0]);
	gpm->ent_velocity[1] = PClamp(pml.velocity[1]);
	gpm->ent_velocity[2] = PClamp(pml.velocity[2]);


	// snap velocity to eigths
	pm->s.velocity[0] = (int)round(pml.velocity[0]*8);
	pm->s.velocity[1] = (int)round(pml.velocity[1]*8);
	pm->s.velocity[2] = (int)round(pml.velocity[2]*8);

	for (i=0 ; i<3 ; i++)
	{
		if (FLOAT_GE_ZERO (pml.origin[i]))
			sign[i] = 1;
		else 
			sign[i] = -1;
		pm->s.origin[i] = (int)(pml.origin[i]*8);
		if (pm->s.origin[i]*0.125f == pml.origin[i])
			sign[i] = 0;
	}
	//VectorCopy (pm->s.origin, base);
	base[0] = pm->s.origin[0];
	base[1] = pm->s.origin[1];
	base[2] = pm->s.origin[2];

	// try all combinations
	for (j=0 ; j<8 ; j++)
	{
		bits = jitterbits[j];
		VectorCopy (base, pm->s.origin);
		for (i=0 ; i<3 ; i++)
			if (bits & (1<<i) )
				pm->s.origin[i] += sign[i];

		if (GPM_GoodPosition ()) {
			//Low precision for xy plane, avoids client sticking to walls and other prediction problems
                 	gpm->ent_origin[0] = pm->s.origin[0]*0.125f;
                 	gpm->ent_origin[1] = pm->s.origin[1]*0.125f;

		        if (pm->groundentity || pm->s.pm_type == PM_SPECTATOR || pml.velocity == 0) //not jumping, exactly at top, not flying up/down
		          gpm->ent_origin[2] = pm->s.origin[2]*0.125f; //snap z too

			return;
                }
	}

	// go back to the last position
	//XXXXXXXXXXXXXXXXXXXXX this is short -> float !!!!!
	//VectorCopy (pml.previous_origin, pm->s.origin);
//	pm->s.origin[0] = pml.previous_origin[0];
//	pm->s.origin[1] = pml.previous_origin[1];
//	pm->s.origin[2] = pml.previous_origin[2];
//	gi.dprintf ("using previous_origin\n");

}



/*
================
GPM_InitialSnapPosition

================
*/
static void GPM_InitialSnapPosition(void)
{
	int        x, y, z;
	int16_t      base[3];
	static int offset[3] = { 0, -1, 1 };

	VectorCopy (pm->s.origin, base);

	for ( z = 0; z < 3; z++ ) {
		pm->s.origin[2] = base[2] + offset[ z ];
		for ( y = 0; y < 3; y++ ) {
			pm->s.origin[1] = base[1] + offset[ y ];
			for ( x = 0; x < 3; x++ ) {
				pm->s.origin[0] = base[0] + offset[ x ];
				if (GPM_GoodPosition ()) {
					pml.origin[0] = pm->s.origin[0]*0.125f;
					pml.origin[1] = pm->s.origin[1]*0.125f;
					pml.origin[2] = pm->s.origin[2]*0.125f;
//					VectorCopy (pm->s.origin, pml.previous_origin);
					return;
				}
			}
		}
	}

//	gi.dprintf ("Bad InitialSnapPosition\n");
}

/*
================
GPM_ClampAngles

================
*/
static void GPM_ClampAngles (void)
{
	int16_t	temp;
	int		i;

	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
		pm->viewangles[PITCH] = 0;
		pm->viewangles[ROLL] = 0;
	}
	else
	{
		// circularly clamp the angles with deltas
		for (i=0 ; i<3 ; i++)
		{
			temp = pm->cmd.angles[i] + pm->s.delta_angles[i];
			pm->viewangles[i] = SHORT2ANGLE(temp);
		}

		// don't let the player look up or down more than 90 degrees
		if (pm->viewangles[PITCH] > 89 && pm->viewangles[PITCH] < 180)
			pm->viewangles[PITCH] = 89;
		else if (pm->viewangles[PITCH] < 271 && pm->viewangles[PITCH] >= 180)
			pm->viewangles[PITCH] = 271;
	}
	AngleVectors (pm->viewangles, pml.forward, pml.right, pml.up);
}


static void GPM_Ungrapple()
{
  if (!gpms->grapple_state) return;

  gpms->grapple_state = grapple_none;
  VectorScale(pml.velocity, 0.75f, pml.velocity);
}

static void GPM_Grapple(float gframetime)
{
  trace_t tr;
  float throwspeed = 800;
  float pullspeed = 625;
  float jumpspeed = 500; //no "precharge" time yet, so keep slower speed
  float pframetime = pml.frametime;
  qboolean slowslidejump = false;

  //step grapple point
  if (1/*gpms->grapple_state == grapple_throw*/) {
    vec3_t end;
    VectorMA(gpms->grapple_point, throwspeed*gframetime, gpms->grapple_dir, end);

    tr = gpm->trace(gpms->grapple_point, grapple_mins, grapple_maxs, end, gpm->self, MASK_SOLID);
    VectorCopy(tr.endpos, gpms->grapple_point);
    if (tr.startsolid) {
      GPM_Ungrapple();
      return;
    }
    
    if (tr.fraction < 1.0f && tr.surface && (tr.surface->flags & SURF_SKY)) {
      GPM_Ungrapple();
      return;
    }

    if (tr.fraction > 0) {
      if (tr.fraction < 1.0f)
        gpms->grapple_state = grapple_throw;
    } else
    if (gpms->grapple_state < grapple_pull)
      gpms->grapple_state = grapple_pull;
  }
  
  //pull player
  if (GPMIfClient(gpms->grapple_state == grapple_hold)) {
    //Use server provided point instead of jittering
    VectorCopy(gpms->grapple_hold, pml.origin);
    VectorClear(pml.velocity);
  } else
  if (gpms->grapple_state >= grapple_pull) {
    vec3_t dir, end, pos, dest;
    float speed, d;
    
    VectorCopy(pml.origin, pos);
    VectorClear(pml.velocity);
    
    //offset destination point by hatchie size
    VectorCopy(gpms->grapple_point, dest);
    if (tr.plane.normal[2] > 0)
      dest[2] -= (pm->mins[0]-1)*tr.plane.normal[2];
    else
      dest[2] += (pm->maxs[0]+1)*tr.plane.normal[2];
    tr.plane.normal[2] = 0;
    VectorNormalize(tr.plane.normal);
    VectorMA(dest, pm->maxs[0]+1, tr.plane.normal, dest);

    VectorSubtract(dest, pos, dir);
    speed = VectorNormalize(dir);

    d = pullspeed*pframetime;
    if (d > speed) d = speed;
    VectorMA(pos, d, dir, end);
    tr = pm->trace(pos, pm->mins, pm->maxs, end);
    if (tr.allsolid) return;

    VectorSubtract(tr.endpos, pml.origin, pml.velocity);

    if (speed > 8 && tr.fraction < 1) {
      d = 1-fabs(DotProduct(dir, tr.plane.normal));
      if (d > 0)
        VectorMA(pml.velocity, d*2.5f, tr.plane.normal, pml.velocity);
    }

    speed = VectorLength(pml.velocity);
    if (speed > 4) {
      gpms->grapple_state = grapple_pull;
      VectorScale(pml.velocity, 1.0f/pml.frametime, pml.velocity);
    } else {
      gpms->grapple_state = grapple_hold;
      VectorCopy(tr.endpos, pml.origin);
      VectorClear(pml.velocity);
    }
  }


  if ((gpms->grapple_state == grapple_hold || slowslidejump) && pm->cmd.upmove > 10) {
    vec3_t forward;
    GPM_Ungrapple();
    AngleVectors(pm->viewangles, forward, NULL, NULL);
    VectorMA(pml.velocity, jumpspeed, forward, pml.velocity);
    gpm->sounds |= GPMS_JUMP;
  }
}




void GloomPMoveUnsnap(gloom_pmove_t*gpm)
{
  gpm->ent_origin[0] = gpm->pmove->s.origin[0]*0.125f;
  gpm->ent_origin[1] = gpm->pmove->s.origin[1]*0.125f;
  if (fabs(gpm->ent_origin[2]-gpm->pmove->s.origin[2]*0.125f) > 0.125f)
    gpm->ent_origin[2] = gpm->pmove->s.origin[2]*0.125f;
	
  gpm->ent_velocity[0] = gpm->pmove->s.velocity[0]*0.125f;
  gpm->ent_velocity[1] = gpm->pmove->s.velocity[1]*0.125f;
  if (fabs(gpm->ent_velocity[2]-gpm->pmove->s.velocity[2]*0.125f) > 0.125f)
    gpm->ent_velocity[2] = gpm->pmove->s.velocity[2]*0.125f;
}


uint8_t GloomPMoveInitState(gloom_pmove_t*gpm, int gloom_class, int flags)
{
  if (gloom_class < 0 || gloom_class > CLASS_OBSERVER) {
    gpm->state->gloom_class = 255;
    return 0;
  }

  gpm->state->gloom_class = gloom_class;
  gpm->state->jump_index = 0;
  gpm->state->flags = flags;

  GloomPMoveUnsnap(gpm);

  return 1;
}

uint8_t GloomPMove(gloom_pmove_t*gpmptr)
{
  vec3_t oldvelocity;

  gpm = gpmptr;

  if (gpm->state->gloom_class > CLASS_OBSERVER || !(gpm->pmove->s.pm_flags & PMF_GLOOM_PMOVE)) return 0;

	pm = gpm->pmove;
	gpms = gpm->state;
	cd = classdata + gpms->gloom_class;
	
        gpm->sounds = 0;
        
	// clear results
	pm->numtouch = 0;
	VectorClear (pm->viewangles);
	pm->viewheight = 0;
	pm->groundentity = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	switch (gpms->flags & (GPMF_MAX_SPEED_DIFF|GPMF_NO_SPEED_DIFF)) {
	  case GPMF_MAX_SPEED_DIFF:
	    pml.maxspeed = cd->speed_max;
	    break;
	  case GPMF_NO_SPEED_DIFF:
	    pml.maxspeed = 300; //grunt original hard coded
	    break;
	  case 0:
	    pml.maxspeed = cd->speed_min;
	    break;
	  case (GPMF_MAX_SPEED_DIFF|GPMF_NO_SPEED_DIFF):
	    pml.maxspeed = (cd->speed_min+cd->speed_max) >> 1;
	    break;
	}


	// convert origin and velocity to float values
	pml.origin[0] = gpm->ent_origin[0];
	pml.origin[1] = gpm->ent_origin[1];
	pml.origin[2] = gpm->ent_origin[2];

	pml.velocity[0] = gpm->ent_velocity[0];
	pml.velocity[1] = gpm->ent_velocity[1];
	pml.velocity[2] = gpm->ent_velocity[2];

	// save old org in case we get stuck
	//VectorCopy (pm->s.origin, pml.previous_origin);
//	pml.previous_origin[0] = pm->s.origin[0];
//	pml.previous_origin[1] = pm->s.origin[1];
//	pml.previous_origin[2] = pm->s.origin[2];

	pml.frametime = pm->cmd.msec * 0.001f;
	
	GPM_ClampAngles ();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
        	pm->viewheight = cd->viewheight;
		GPM_FlyMove ();
		GPM_SnapPosition(gpm);
		return 1;
	}

	if (pm->s.pm_type >= PM_DEAD && pm->s.pm_type <= PM_FREEZE)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}
	
	if (pm->s.pm_type == PM_FREEZE)
		return 1;		// no movement at all

	// set mins, maxs, and viewheight
	GPM_CheckDuck ();

	if (pm->snapinitial)
		GPM_InitialSnapPosition ();

	// set groundentity, watertype, and waterlevel
	GPM_CatagorizePosition ();

	if (pm->s.pm_type == PM_DEAD || pm->s.pm_type == PM_GIB)
		GPM_DeadMove ();

	GPM_CheckSpecialMovement ();

        if (gpms->grapple_state >= grapple_pull)
          pm->s.gravity = 0;

	// drop timing counter
	if (pm->s.pm_time)
	{
		int		msec;

		msec = pm->cmd.msec >> 3;
		if (!msec)
			msec = 1;
		if ( msec >= pm->s.pm_time) 
		{
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}
		else
			pm->s.pm_time -= msec;
	}

	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{	
		// teleport pause stays exactly in place
	}
	else if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{	
		// waterjump has no control, but falls
		pml.velocity[2] -= pm->s.gravity * pml.frametime;
		if (FLOAT_LT_ZERO(pml.velocity[2]))
		{	// cancel as soon as we are falling down again
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}

		GPM_StepSlideMove ();
	}
	else
	{
		GPM_Friction ();

		if (pm->waterlevel >= 2)
			GPM_WaterMove ();
		else {
			vec3_t	angles;

			VectorCopy (pm->viewangles, angles);
			if (angles[PITCH] > 180)
				angles[PITCH] = angles[PITCH] - 360;
			angles[PITCH] /= 3;

			AngleVectors (angles, pml.forward, pml.right, pml.up);

			if (pm->s.pm_type != PM_WRAITH || pm->groundentity)
			  GPM_AirMove ();
			else
			  GPM_WraithMove();
		}
	}

	//grapple
	if (pm->s.pm_type != PM_DEAD && pm->s.pm_type != PM_GIB) {
          if (!(pm->cmd.buttons & BUTTON_ATTACK)) {
            GPM_Ungrapple();
            gpms->grapple_prestep = 0;
          } else
	  if ((BIT(gpms->gloom_class) & (BIT(CLASS_HATCHLING) | BIT(CLASS_KAMIKAZE))) && (gpms->grapple_state == grapple_none) && (pm->cmd.buttons & BUTTON_ATTACK)) {
            gpms->grapple_state = grapple_throw;
            VectorCopy(pml.origin, gpms->grapple_point);
            AngleVectors(pm->viewangles, gpms->grapple_dir, NULL, NULL);
            gpms->grapple_prestep += 0.1f;
          }
          
          if (gpms->grapple_state) {
            float gt = gpms->grapple_prestep + pml.frametime;
            gpms->grapple_prestep = 0;
            
            GPM_Grapple(gt);
          }
	} else
	if (gpms->grapple_state) //dead aliens don't grapple
	  GPM_Ungrapple();


	// set groundentity, watertype, and waterlevel for final spot
	GPM_CatagorizePosition ();
	
	
        GPM_LateJump (); //Need to jump just before bouncing, and bounce needs old velocity, thus has to set speed at end of client frame for next one

	if (pm->groundentity && pml.groundplane.normal[2] > 0 && gpm->ent_velocity[2] < 0) {
	  float b = -gpm->ent_velocity[2] * cd->bounce;
	  if (pml.groundplane.normal[2]*b > 32) {
	    pml.velocity[0] += pml.groundplane.normal[0]*b;
	    pml.velocity[1] += pml.groundplane.normal[1]*b;
	    pml.velocity[2] += pml.groundplane.normal[2]*b;
	    pm->groundentity = NULL;
	    pm->s.pm_flags &= ~PMF_ON_GROUND;
	  }
	}
// gi.dprintf("V=%.3f\n", VectorLength(pml.velocity));
	GPM_SnapPosition(gpm);
  return 1;
}

void GloomPMoveClientUnpack(gloom_pmove_t*gpm, int bits)
{
  int n;

  gpm->layout_bits = bits;

  gpms = gpm->state;

  gpms->jump_index = ((bits >> 2) & 3) % 3;
  n = (bits >> 4) & 7;
  if (n > 0) gpm->pmove->s.pm_type = 8+n;
  
  gpms->grapple_state = grapple_none;

  if (gpm->state->gloom_class > CLASS_OBSERVER || !(gpm->pmove->s.pm_flags & PMF_GLOOM_PMOVE)) return;

  if (BIT(gpms->gloom_class) & (BIT(CLASS_HATCHLING) | BIT(CLASS_KAMIKAZE))) {
    if (bits & 256) {
      gpms->grapple_state = grapple_pull;
      if (pm->s.pm_flags & PMF_TIME_LAND)
        gpms->grapple_state = grapple_hold;
    } else
    if (gpm->pmove->cmd.buttons & BUTTON_ATTACK)
      gpms->grapple_state = grapple_throw;
  }
}


int GuessGloomClass(const char*skinname)
{
  struct class_data_s*d = classdata;
  int n = 0;

  while (d->skinkey) {
    if (strstr(skinname, d->skinkey)) return n;
    n++;
    d++;
  }
  
  return strcmp(skinname, "male/obs")?-1:n;
}

void GloomPMoveSetGrapplePoint(gloom_pmove_state_t*s, float* vec3point, float* vec3dir, float prestep, float*holdpoint)
{
  float len = VectorLength(vec3dir);
  vec3_t dir;
  if (len < 0.99999f || len > 1.00001f)
    VectorScale(vec3dir, 1.0f/len, dir);
  else
    VectorCopy(vec3dir, dir);
  VectorCopy(vec3point, s->grapple_point);
  VectorCopy(holdpoint, s->grapple_hold);
  s->grapple_prestep = prestep;
}
