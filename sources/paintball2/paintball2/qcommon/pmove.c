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

#include "qcommon.h"

#define	STEPSIZE	18.0f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

typedef struct pml_s
{
	vec3_t		origin;			// full float precision
	vec3_t		velocity;		// full float precision

	vec3_t		forward, right, up;
	float		frametime;


	csurface_t	*groundsurface;
	cplane_t	groundplane;
	int			groundcontents;

	short		previous_origin[3]; // jitmove - this should have always been a short... probably didn't hurt anything, though, since floats have more precision.
	qboolean	ladder;
	qboolean	rampslide; // jitmove - special case ramp sliding so players can have control but air accel at the same time.
	qboolean	crouchslide;
} pml_t;

pmove_t		*pm;
pml_t		pml;
float		g_viewheight = 0.0f; // jit
float		g_stepheight = 0.0f; // jitmove - for step smoothing

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_maxspeed = 300.0f;
float	pm_duckspeed = 100.0f;
float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 0.0f;
float	pm_wateraccelerate = 10.0f;
float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_waterspeed = 400.0f;
float	pm_crouchslidefriction = 0.0f; // jitmove (initialized in sv_init.c)
float	pm_skyglide_maxvel = 0.0f; // jitmove - set by sv_skyglide_maxvel
float	pm_crouchslidestopspeed = 110.0f; // jitmove
qboolean pm_oldmovephysics = false; // jitmove

/*

  walking up a step should kill some velocity

*/


/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

void PM_ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
}


/*
==================
PM_StepSlideMove

Each intersection will try to step over the obstruction instead of
sliding along it.

Returns a new origin, velocity, and contact entity
Does not modify any world state?
==================
*/
#define	MIN_STEP_NORMAL	0.7		// can't step up onto very steep slopes
#define	MAX_CLIP_PLANES	5

void PM_StepSlideMove_ (void)
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	int			i, j;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	qboolean	hitsky = false;
	float		initial_z_vel = pml.velocity[2];
	
	numbumps = 4;
	VectorCopy(pml.velocity, primal_velocity);
	numplanes = 0;
	time_left = pml.frametime;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		for (i = 0; i < 3; i++)
			end[i] = pml.origin[i] + time_left * pml.velocity[i];

		trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			pml.velocity[2] = 0.0f;	// don't build up falling damage
			return;
		}

		if (trace.fraction > 0.0f)
		{	// actually covered some distance
			VectorCopy(trace.endpos, pml.origin);
			numplanes = 0;
		}

		if (trace.fraction == 1.0f)
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
			VectorCopy(vec3_origin, pml.velocity);
			break;
		}

		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		// modify original_velocity so it parallels all of the clip planes
		for (i = 0; i < numplanes; i++)
		{	
			PM_ClipVelocity(pml.velocity, planes[i], pml.velocity, 1.01f);

			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if (DotProduct(pml.velocity, planes[j]) < 0.0f)
						break; // not ok
				}
			}

			if (trace.surface->flags & SURF_SKY && trace.plane.normal[2] < -0.95f && (pm->s.pm_flags & PMF_JUMP_HELD)) // jitmove - don't kill upward velocity if you hit your head on sky while holding jump
				hitsky = true;

			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{
			// go along this plane
		}
		else
		{
			// go along the crease
			if (numplanes != 2)
			{
				VectorCopy(vec3_origin, pml.velocity);
				break;
			}

			CrossProduct(planes[0], planes[1], dir);
			d = DotProduct(dir, pml.velocity);
			VectorScale(dir, d, pml.velocity);
		}

#if 0 // jitmove / jitjump - removed - I don't think this even does anything useful
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		if (DotProduct(pml.velocity, primal_velocity) <= 0)
		{
			VectorCopy(vec3_origin, pml.velocity);
			break;
		}
#endif
	}

	if (hitsky && pm_skyglide_maxvel > 0.0f)
	{
		pml.velocity[2] = initial_z_vel; // jitmove - don't kill upward velocity if you hit your head on sky

		if (pml.velocity[2] > pm_skyglide_maxvel) // ice jumps can be a bit extreme, so keep sky gliding to a minimum
			pml.velocity[2] = pm_skyglide_maxvel;
	}

	// For jumping out of water, teleports, server corrected positions, etc.
	if (pm->s.pm_time)
	{
		VectorCopy(primal_velocity, pml.velocity);
	}
}


/*
==================
PM_StepSlideMove

==================
*/
void PM_StepSlideMove (void)
{
	vec3_t		start_o, start_v;
	vec3_t		down_o, down_v;
	trace_t		trace;
	float		down_dist, up_dist;
	vec3_t		up, down;
	float		stepfraction = 1.0f; // jitmove
	float		downmin;
	float		stepsize = STEPSIZE;
	float		up_end_Z;

	if (pml.crouchslide) // jitmove - don't crouch slide up stairs
		stepsize = 5.0f;

	VectorCopy(pml.origin, start_o);
	VectorCopy(pml.velocity, start_v);

	PM_StepSlideMove_();

	VectorCopy(pml.origin, down_o);
	VectorCopy(pml.velocity, down_v);

	VectorCopy(start_o, up);
	up[2] += stepsize;

	// jitmove - trace should start at the start, not at "up" (fix for not stepping in places with low ceilings)
	trace = pm->trace(start_o, pm->mins, pm->maxs, up);
	stepfraction = trace.fraction;

	if (trace.allsolid)
	{
		Com_DPrintf("Trace up all solid.\n");
		return;		// can't step up (jit - probably obsolete with fix above and below this - I think this only hits when stuck in geometry)
	}

	// try sliding above
	VectorCopy(trace.endpos, pml.origin); // jitmove - don't potentially start in something solid if we have steps with a low ceiling
	VectorCopy(start_v, pml.velocity);

	PM_StepSlideMove_();

	// push down the final amount
	VectorCopy(pml.origin, down);
	downmin = down[2] - stepsize * stepfraction; // jitmove - don't step down further than we stepped up

	if (start_o[2] - 1.0f < downmin && stepfraction == 1.0f && !pm_oldmovephysics && !(pm->s.pm_flags & PMF_TIME_WATERJUMP))
		down[2] = start_o[2] - 1.0f; // jitmove / jitjump - cast down a little further, so we don't bounce off of steps while jumping
	else
		down[2] = downmin;

	up_end_Z = pml.origin[2];
	trace = pm->trace(pml.origin, pm->mins, pm->maxs, down);

	if (trace.allsolid && !pm_oldmovephysics)
	{
		// jitmove - bring everything in 1 unit and re-cast to fix weird bouncing issue along some angled walls along ramps
		vec3_t mins, maxs;

		VectorCopy(pm->mins, mins);
		VectorCopy(pm->maxs, maxs);
		++mins[0];
		++mins[1];
		--maxs[0];
		--maxs[1];
		--maxs[2];

		trace = pm->trace(pml.origin, mins, maxs, down);
	}

	if (!trace.allsolid)
	{
		VectorCopy(trace.endpos, pml.origin);

		if (pml.origin[2] < downmin) // jitmove - since we cast down further, make sure we don't falsely move down too far
			pml.origin[2] = downmin;

		// jitmove - Sometimes the "down" cast ends up higher since we may have hit a sloped ceiling
		if (!pm_oldmovephysics && pml.origin[2] < down_o[2])
		{
			// up_end_Z check might not be necessary, but I don't want to put the player in solid geometry
			if (up_end_Z > down_o[2])
				pml.origin[2] = down_o[2];
			else
				pml.origin[2] = up_end_Z; // (Happens rarely, and they're almost equal, but better safe than stuck)
		}
	}
	else
	{
		Com_DPrintf("Trace down all solid.\n");
	}

	VectorCopy(pml.origin, up);

	// decide which one went farther
    down_dist = (down_o[0] - start_o[0]) * (down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
    up_dist = (up[0] - start_o[0]) * (up[0] - start_o[0]) + (up[1] - start_o[1]) * (up[1] - start_o[1]);

	// In almost all cases, up_dist is ~>= down_dist (much greater on up slopes and steps)
	// down_dist only appears to be greater when falling down onto downward slopes.
	if (down_dist > up_dist || trace.plane.normal[2] < MIN_STEP_NORMAL)
	{
		VectorCopy(down_o, pml.origin);
		VectorCopy(down_v, pml.velocity);
	}
	else
	{
		float stepdiff = up[2] - down_o[2];

		// jitmove - save this for step smoothing later
		if (stepdiff > 0.01f)
			g_stepheight = stepdiff;

		//!! Special case
		// if we were walking along a plane, then we need to copy the Z over
		pml.velocity[2] = down_v[2];
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction (void)
{
	float	*vel;
	float	speed, newspeed, control;
	float	friction;
	float	drop;
	vec3_t	currentdir;

	pml.crouchslide = false;
	vel = pml.velocity;
	VectorCopy(vel, currentdir);
	speed = VectorNormalizeRetLen(currentdir);

	if (speed < 1.0f)
	{
		vel[0] = 0.0f;
		vel[1] = 0.0f;
		return;
	}

	drop = 0.0f;

	// apply ground friction
	if ((pm->groundentity && pml.groundsurface && !pml.rampslide && !(pml.groundsurface->flags & SURF_SLICK)) || (pml.ladder)) // jitmove
	{
		friction = pm_friction;

		// === jitmove - crouchsliding
		if (pm->cmd.upmove < 0 && pm_crouchslidefriction > 0.0f)
		{
			float speed2d = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

			if (speed2d > pm_crouchslidestopspeed)
			{
				vec3_t wishvel;
				int i;

				for (i = 0; i < 3; ++i)
					wishvel[i] = pml.forward[i] * pm->cmd.forwardmove + pml.right[i] * pm->cmd.sidemove;

				if (DotProduct(currentdir, wishvel) > 0.001f) // user wants to go in the direction of the slide
				{
					friction = pm_crouchslidefriction;
					pml.crouchslide = true;
				}
			}
		}
		// jitmove ===

		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;
	}

	// apply water friction
	if (pm->waterlevel && !pml.ladder)
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;

	// scale the velocity
	newspeed = speed - drop;
	
	if (newspeed < 0)
		newspeed = 0;

	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
void PM_Accelerate (vec3_t wishdir, float wishspeed, float accel)
{
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0.0f)
		return;

	accelspeed = accel * pml.frametime * wishspeed;

	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	for (i = 0; i < 3; i++)
		pml.velocity[i] += accelspeed * wishdir[i];	
}


// Funky Quake1 style air acceleration:
void PM_AirAccelerate (vec3_t wishdir, float wishspeed, float accel)
{
	int			i;
	float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if (wishspd > 30.0f)
		wishspd = 30.0f;

	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspd - currentspeed;

	if (addspeed <= 0.0f)
		return;

	accelspeed = accel * wishspeed * pml.frametime;

	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	for (i = 0; i < 3; i++)
		pml.velocity[i] += accelspeed * wishdir[i];	
}

/*
=============
PM_AddCurrents
=============
*/
void PM_AddCurrents (vec3_t	wishvel)
{
	vec3_t	v;
	float	s;

	//
	// account for ladders
	//

	if (pml.ladder && fabs(pml.velocity[2]) <= 200.0f)
	{
		if ((pm->viewangles[PITCH] <= -15.0f) && (pm->cmd.forwardmove > 0))
			wishvel[2] = 200.0f;
		else if ((pm->viewangles[PITCH] >= 15.0f) && (pm->cmd.forwardmove > 0))
			wishvel[2] = -200.0f;
		else if (pm->cmd.upmove > 0)
			wishvel[2] = 200.0f;
		else if (pm->cmd.upmove < 0)
			wishvel[2] = -200.0f;
		else
			wishvel[2] = 0.0f;

		// limit horizontal speed when on a ladder
		if (pm_oldmovephysics || !pm->groundentity) // jitmove - if they're touching the ground, let them walk normally
		{
			if (wishvel[0] < -25.0f)
				wishvel[0] = -25.0f;
			else if (wishvel[0] > 25.0f)
				wishvel[0] = 25.0f;

			if (wishvel[1] < -25.0f)
				wishvel[1] = -25.0f;
			else if (wishvel[1] > 25.0f)
				wishvel[1] = 25.0f;
		}
	}

	//
	// add water currents
	//

	if (pm->watertype & MASK_CURRENT)
	{
		VectorClear (v);

		if (pm->watertype & CONTENTS_CURRENT_0)
			v[0] += 1.0f;
		if (pm->watertype & CONTENTS_CURRENT_90)
			v[1] += 1.0f;
		if (pm->watertype & CONTENTS_CURRENT_180)
			v[0] -= 1.0f;
		if (pm->watertype & CONTENTS_CURRENT_270)
			v[1] -= 1.0f;
		if (pm->watertype & CONTENTS_CURRENT_UP)
			v[2] += 1.0f;
		if (pm->watertype & CONTENTS_CURRENT_DOWN)
			v[2] -= 1.0f;

		s = pm_waterspeed;

		if ((pm->waterlevel == 1) && (pm->groundentity))
			s *= 0.5f;

		VectorMA(wishvel, s, v, wishvel);
	}

	//
	// add conveyor belt velocities
	//

	if (pm->groundentity)
	{
		VectorClear(v);

		if (pml.groundcontents & CONTENTS_CURRENT_0)
			v[0] += 1.0f;
		if (pml.groundcontents & CONTENTS_CURRENT_90)
			v[1] += 1.0f;
		if (pml.groundcontents & CONTENTS_CURRENT_180)
			v[0] -= 1.0f;
		if (pml.groundcontents & CONTENTS_CURRENT_270)
			v[1] -= 1.0f;
		if (pml.groundcontents & CONTENTS_CURRENT_UP)
			v[2] += 1.0f;
		if (pml.groundcontents & CONTENTS_CURRENT_DOWN)
			v[2] -= 1.0f;

		VectorMA(wishvel, 100.0f /* pm->groundentity->speed */, v, wishvel);
	}
}


/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;

//
// user intentions
//
	for (i = 0; i < 3; ++i)
		wishvel[i] = pml.forward[i]*pm->cmd.forwardmove + pml.right[i]*pm->cmd.sidemove;

	if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
		wishvel[2] -= 60.0f;		// drift towards bottom
	else
		wishvel[2] += pm->cmd.upmove;

	PM_AddCurrents(wishvel);
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalizeRetLen(wishdir);

	if (wishspeed > pm_maxspeed)
	{
		VectorScale (wishvel, pm_maxspeed/wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}

	wishspeed *= 0.5f;
	PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);
	PM_StepSlideMove();
}


/*
===================
PM_AirMove
Despite its name, this function gets called all the time, even if you're on the ground.
===================
*/
void PM_AirMove (void)
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		maxspeed;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
//!!!!! pitch should be 1/3 so this isn't needed??!
#if 0
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);
#endif

	for (i = 0; i < 2; i++)
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;

	wishvel[2] = 0.0f;
	PM_AddCurrents(wishvel);
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalizeRetLen(wishdir);

	// clamp to server defined max speed
	maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if (wishspeed > maxspeed)
	{
		VectorScale(wishvel, maxspeed / wishspeed, wishvel);
		wishspeed = maxspeed;
	}
	
	if (pml.ladder)
	{
		PM_Accelerate(wishdir, wishspeed, pm_accelerate);

		if (!wishvel[2])
		{
			if (pml.velocity[2] > 0.0f)
			{
				pml.velocity[2] -= pm->s.gravity * pml.frametime;

				if (pml.velocity[2] < 0.0f)
					pml.velocity[2] = 0.0f;
			}
			else
			{
				pml.velocity[2] += pm->s.gravity * pml.frametime;

				if (pml.velocity[2] > 0.0f)
					pml.velocity[2]  = 0.0f;
			}
		}

		PM_StepSlideMove();
	}
	else if (pm->groundentity)
	{
		if (pm_oldmovephysics || (pm->s.pm_flags & PMF_JUMP_HELD)) // jitmove - use old bouncing down slopes when the jump button is held since some players want it.
		{
			// walking on ground
			pml.velocity[2] = 0.0f; //!!! this is before the accel
			PM_Accelerate(wishdir, wishspeed, pm_accelerate);

			// PGM	-- fix for negative trigger_gravity fields
			//		pml.velocity[2] = 0;
			if (pm->s.gravity > 0)
				pml.velocity[2] = 0.0f;
			else
				pml.velocity[2] -= pm->s.gravity * pml.frametime;
			// PGM
		}
		else // === jitslope / jitmove - new handling of slopes makes things smoother and gives players more control
		{
			float accelerate = pm_accelerate;

			if (pml.velocity[2] > 0.0f && pml.velocity[2] < 150.0f) // jitmove - don't fling the player up in the air over little bumps and such
				pml.velocity[2] = 0.0f;

			if (pml.rampslide)
			{
				vec3_t currentdir2D;
				float dot;

				VectorCopy(pml.velocity, currentdir2D);
				currentdir2D[2] = 0.0f;
				VectorNormalize(currentdir2D);
				dot = DotProduct(currentdir2D, wishdir);

				if (dot > 0.01f)
				{
					accelerate = 1.0f; // Air acceleration value (used with old Q2 ramp sliding as it categorized you as being in the air)
				}
				else
				{
					pml.rampslide = false; // desired direction is away from current velocity, so stop ramp sliding and return to normal control
					PM_Friction(); // apply friction because it isn't normally applied when rampslide is active
				}
			}

			// apply gravity even when on a ground entity so we walk down slopes smoothly instead of skipping.
			pml.velocity[2] -= pm->s.gravity * pml.frametime;
			PM_Accelerate(wishdir, wishspeed, accelerate);
		} // jitslope / jitmove ===

		if (!pml.velocity[0] && !pml.velocity[1])
			return;

		PM_StepSlideMove();
	}
	else
	{	// not on ground, so little effect on velocity
		if (pm_airaccelerate)
			PM_AirAccelerate(wishdir, wishspeed, pm_accelerate);
		else
			PM_Accelerate(wishdir, wishspeed, 1.0f);

		// add gravity
		pml.velocity[2] -= pm->s.gravity * pml.frametime;
		PM_StepSlideMove();
	}
}



/*
=============
PM_CategorizePosition
=============
*/
void PM_CategorizePosition (void)
{
	vec3_t		point;
	int			cont;
	trace_t		trace;
	int			sample1;
	int			sample2;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid	
	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] - 0.25f;

	pml.rampslide = false; // jitmove

	if (pml.velocity[2] > 180.0f && (pm_oldmovephysics || (pm->s.pm_flags & PMF_JUMP_HELD))) // jitmove / jitjump - don't slide uphill uncontrollably unless we're holding the jump button (or using old physics)
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundentity = NULL;
	}
	else
	{
		trace = pm->trace(pml.origin, pm->mins, pm->maxs, point);
		pml.groundplane = trace.plane;
		pml.groundsurface = trace.surface;
		pml.groundcontents = trace.contents;

		if (trace.ent && trace.plane.normal[2] < 0.7f && !trace.startsolid) // jitclipbug
		{
			trace_t		trace2;
			vec3_t		mins, maxs;

			// try a slightly smaller bounding box -- this is to fix getting stuck up
			// on angled walls and not being able to move (like you're stuck in the air)
			mins[0] = pm->mins[0] ? pm->mins[0] + 1.0f : 0.0f;
			mins[1] = pm->mins[1] ? pm->mins[1] + 1.0f : 0.0f;
			mins[2] = pm->mins[2];
			maxs[0] = pm->maxs[0] ? pm->maxs[0] - 1.0f : 0.0f;
			maxs[1] = pm->maxs[1] ? pm->maxs[1] - 1.0f : 0.0f;
			maxs[2] = pm->maxs[2];
			trace2 = pm->trace(pml.origin, mins, maxs, point);
			
			if (!(trace2.plane.normal[2] < 0.7f && !trace2.startsolid))
			{
				memcpy(&trace, &trace2, sizeof(trace));
				pml.groundplane = trace.plane;
				pml.groundsurface = trace.surface;
				pml.groundcontents = trace.contents;
				//pm->groundentity = trace.ent;
			}
		}
		
		if (!trace.ent || (trace.plane.normal[2] < 0.7f && !trace.startsolid))
		{
			pm->groundentity = NULL;
			pm->s.pm_flags &= ~PMF_ON_GROUND;
		}
		else
		{
			pm->groundentity = trace.ent;

			// hitting solid ground will end a waterjump
			if ((pm->s.pm_flags & PMF_TIME_WATERJUMP) && (pm_oldmovephysics || pml.velocity[2] < 180.0f)) // jitmove - don't allow crazy double jumps out of the water with the new physics.
			{
				pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | /*jitmove PMF_TIME_LAND |*/ PMF_TIME_TELEPORT);
				pm->s.pm_time = 0;
			}

			if (!(pm->s.pm_flags & PMF_ON_GROUND))
			{	// just hit the ground
				pm->s.pm_flags |= PMF_ON_GROUND;

#if 0 // jitjump - we don't want to stop jumping after touching the ground (code doesn't work 99% of the time, anyway)
				// don't do landing time if we were just going down a slope
				if (pml.velocity[2] < -200) // jit -  this crap doesn't work, velocity[2] is almost never < 0.
				{
					pm->s.pm_flags |= PMF_TIME_LAND;

					// don't allow another jump for a little while
					if (pml.velocity[2] < -400)
						pm->s.pm_time = 25;	
					else
						pm->s.pm_time = 18;
				}
#endif
			}
		}

		if (pm->numtouch < MAXTOUCH && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}

		if (!pm_oldmovephysics && pml.velocity[2] > 180.0f) // jitmove - special ramp handling
		{
			pml.rampslide = true;
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


#define JUMPVELOCITY 270.0f
#define MAXJUMPVELOCITY 450.0f // With the old physics, this is the maximum velocity you could reach jumping (180 + 270)


/*
=============
PM_CheckJump
=============
*/
void PM_CheckJump (void)
{
	float jumpvelocity = JUMPVELOCITY;

	// === jitjumphack -- don't kill people's strafe jumps
#if 0 // doubt even Q2 players want this... #ifdef QUAKE2
	if (pm->s.pm_flags & PMF_TIME_LAND)
	{	// hasn't been long enough since landing to jump again
		return;
	}
#endif
	// ===

	if (pm->cmd.upmove < 10)
	{	// not holding jump
		pm->s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}

	// must wait for jump to be released
	if (pm->s.pm_flags & PMF_JUMP_HELD)
		return;

	if (pm->s.pm_type == PM_DEAD)
		return;

	if (pm->waterlevel >= 2)
	{	// swimming, not jumping
		pm->groundentity = NULL;

		if (pml.velocity[2] <= -300.0f)
			return;

		if (pm->watertype == CONTENTS_WATER)
			pml.velocity[2] = 100.0f;
		else if (pm->watertype == CONTENTS_SLIME)
			pml.velocity[2] = 80.0f;
		else
			pml.velocity[2] = 50.0f;

		return;
	}

	if (pm->groundentity == NULL)
		return;		// in air, so no effect

	pm->s.pm_flags |= PMF_JUMP_HELD;
	pm->groundentity = NULL;

	if (!pm_oldmovephysics) // jitmove - cap velocity we jump at, since it was technically capped with old physics
	{
		if (pml.velocity[2] > MAXJUMPVELOCITY)
			return;

		if (pml.velocity[2] + jumpvelocity > MAXJUMPVELOCITY)
			jumpvelocity = MAXJUMPVELOCITY - pml.velocity[2];
	}

	pml.velocity[2] += jumpvelocity;

	if (pml.velocity[2] < jumpvelocity)
		pml.velocity[2] = jumpvelocity;
}


/*
=============
PM_CheckSpecialMovement
=============
*/
void PM_CheckSpecialMovement (void)
{
	vec3_t	spot;
	vec3_t	spot2;
	int		cont, cont2;
	vec3_t	flatforward;
	trace_t	trace;

	if (pm->s.pm_time)
		return;

	pml.ladder = false;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize(flatforward);
	VectorMA(pml.origin, 1, flatforward, spot);
	trace = pm->trace(pml.origin, pm->mins, pm->maxs, spot);

	if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
		pml.ladder = true;

	// check for water jump
	if (pm->waterlevel != 2)
		return;

	// === jitwaterjump
	/*flatforward[0] += pml.velocity[0];
	flatforward[1] += pml.velocity[1];
	VectorNormalize(flatforward);*/
//#ifdef _DEBUG
//	Com_DPrintf("Forward: %3g %3g\n", flatforward[0], flatforward[1]);
//#endif
	// jitwaterjump ===

	VectorMA(pml.origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents(spot);

	// === jitwaterjump
	VectorMA(pml.origin, 38, flatforward, spot2);
	spot2[2] -= 1;
	//spot[2] -= 5;
	cont2 = pm->pointcontents(spot2);
//#ifdef _DEBUG
//	Com_DPrintf("Conts: %d %d\n", cont, cont2);
//#endif

//	if (!(cont & CONTENTS_SOLID) && !(cont2 & CONTENTS_SOLID)) // jitwaterjump
//		return;
	if (cont & CONTENTS_SOLID)
	{
		spot[2] += 16;
		cont = pm->pointcontents(spot);
	}
	else if (cont2 & CONTENTS_SOLID)
	{
		spot2[2] += 21;
		cont = pm->pointcontents(spot2);
	}
	else
	{
		return;
	}
	// jitwaterjump ===

	if (cont)
		return;

	// jump out of water
	VectorScale(flatforward, 50, pml.velocity);
	pml.velocity[2] = 350;
	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}


/*
===============
PM_FlyMove
===============
*/
void PM_FlyMove (qboolean doclip)
{
	float	speed, drop, friction, control, newspeed;
	float	currentspeed, addspeed, accelspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	vec3_t		end;
	trace_t	trace;

	pm->viewheight = 22;

	// friction

	speed = VectorLength (pml.velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pml.velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pml.velocity, newspeed, pml.velocity);
	}

	// accelerate
	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 3; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;

	wishvel[2] += pm->cmd.upmove;
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalizeRetLen(wishdir);

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

	if (addspeed <= 0)
		return;

	accelspeed = pm_accelerate*pml.frametime*wishspeed;

	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	for (i = 0; i < 3; i++)
		pml.velocity[i] += accelspeed*wishdir[i];	

	if (doclip) {
		for (i=0 ; i<3 ; i++)
			end[i] = pml.origin[i] + pml.frametime * pml.velocity[i];

		trace = pm->trace (pml.origin, pm->mins, pm->maxs, end);

		VectorCopy (trace.endpos, pml.origin);
	} else {
		// move
		VectorMA (pml.origin, pml.frametime, pml.velocity, pml.origin);
	}
}


/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->viewheight
==============
*/
void PM_CheckDuck (void)
{
	trace_t	trace;

	pm->mins[0] = -16.0f;
	pm->mins[1] = -16.0f;

	pm->maxs[0] = 16.0f;
	pm->maxs[1] = 16.0f;

	if (pm->s.pm_type == PM_GIB)
	{
		pm->mins[2] = 0.0f;
		pm->maxs[2] = 16.0f;
		pm->viewheight = 8.0f;
		return;
	}

	pm->mins[2] = -24.0f;

	if (pm->s.pm_type == PM_DEAD)
	{
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else if (pm->cmd.upmove < 0 && (pm->s.pm_flags & PMF_ON_GROUND) && !pml.rampslide) // jitmove - don't allow crouching off ramps since we couldn't do that before
	{
		// duck
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else
	{
		// stand up if possible
		if ((pm->s.pm_flags & PMF_DUCKED) && (pm->cmd.upmove >= 0 || fabs(pml.velocity[2]) > 150.0f)) // jitmove - allow people to stay ducked on slopes where they briefly catch air.  It was like this for a while due to another bug.
		{
			// try to stand up
			pm->maxs[2] = 32.0f;
			trace = pm->trace(pml.origin, pm->mins, pm->maxs, pml.origin);

			if (!trace.allsolid)
				pm->s.pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->s.pm_flags & PMF_DUCKED)
	{
		pm->maxs[2] = 4.0f;
		pm->viewheight = -2.0f;
	}
	else
	{
		pm->maxs[2] = 32.0f;
		pm->viewheight = 22.0f;
	}
}


/*
==============
PM_DeadMove
==============
*/
void PM_DeadMove (void)
{
	float	forward;

	if (!pm->groundentity)
		return;

	// extra friction

	forward = VectorLength (pml.velocity);
	forward -= 20;
	if (forward <= 0)
	{
		VectorClear (pml.velocity);
	}
	else
	{
		VectorNormalize (pml.velocity);
		VectorScale (pml.velocity, forward, pml.velocity);
	}
}


qboolean PM_GoodPosition (void)
{
	trace_t	trace;
	vec3_t	origin, end;
	int		i;

	if (pm->s.pm_type == PM_SPECTATOR)
		return true;

	for (i = 0; i < 3; i++)
		origin[i] = end[i] = (float)pm->s.origin[i] * 0.125f;

	trace = pm->trace(origin, pm->mins, pm->maxs, end);

	return !trace.allsolid;
}

/*
================
PM_SnapPosition

On exit, the origin will have a value that is pre-quantized to the 0.125
precision of the network channel and in a valid position.
================
*/
void PM_SnapPosition (void)
{
	int		sign[3];
	int		i, j, bits;
	short	base[3];
	// try all single bits first
	static int jitterbits[8] = {0,4,1,2,3,5,6,7};
#ifdef USE_PMOVE_FLOAT
	// jitmove - testing, floating point precision
	VectorCopy(pml.origin, pm->s.forigin);
	VectorCopy(pml.velocity, pm->s.fvelocity);
#endif
	// snap velocity to eigths
	for (i = 0; i < 3; i++)
		pm->s.velocity[i] = (short)(pml.velocity[i] * 8.0f);

	for (i = 0; i < 3; i++)
	{
		if (pml.origin[i] >= 0 || i == 2)
			sign[i] = 1;
		else 
			sign[i] = -1;

#ifndef QUAKE2 // jitmove - round down on z axis so high framerate doesn't have an unfair advantage
		if (i == 2 && pml.origin[i] < 0) // z-axis
			pm->s.origin[i] = (short)((int)(pml.origin[i] * 8.0f + 32768.0f) - 32768);
		else
#endif
			pm->s.origin[i] = (short)(pml.origin[i] * 8.0f);

		if ((short)((float)pm->s.origin[i] * 0.125f) == pml.origin[i])
			sign[i] = 0;
	}

	VectorCopy(pm->s.origin, base);

	// try all combinations
	for (j = 0; j < 8; j++)
	{
		bits = jitterbits[j];
		VectorCopy(base, pm->s.origin);

		for (i = 0; i < 3; i++)
			if (bits & (1<<i))
				pm->s.origin[i] += sign[i];

		if (PM_GoodPosition())
			return;
	}

	// go back to the last position
	VectorCopy(pml.previous_origin, pm->s.origin);
//	Com_DPrintf ("using previous_origin\n");
}


/*
================
PM_InitialSnapPosition

================
*/
void PM_InitialSnapPosition (void)
{
	int        x, y, z;
	short      base[3];
	static int offset[3] = { 0, -1, 1 };

	VectorCopy(pm->s.origin, base);

	for (z = 0; z < 3; z++)
	{
		pm->s.origin[2] = base[2] + offset[z];

		for (y = 0; y < 3; y++)
		{
			pm->s.origin[1] = base[1] + offset[y];

			for (x = 0; x < 3; x++)
			{
				pm->s.origin[0] = base[0] + offset[x];

				if (PM_GoodPosition())
				{
#ifdef USE_PMOVE_FLOAT
					if (1) // jitmove - todo, cvar, testing
					{
						VectorCopy(pm->s.forigin, pml.origin);
					}
					else
#endif
					{
						pml.origin[0] = (float)pm->s.origin[0] * 0.125f;
						pml.origin[1] = (float)pm->s.origin[1] * 0.125f;
						pml.origin[2] = (float)pm->s.origin[2] * 0.125f;
					}

					VectorCopy(pm->s.origin, pml.previous_origin);
					return;
				}
			}
		}
	}

	Com_DPrintf("Bad InitialSnapPosition.\n");
}


/*
================
PM_ClampAngles

================
*/
void PM_ClampAngles (void)
{
	short	temp;
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
		for (i = 0; i < 3; i++)
		{
			temp = pm->cmd.angles[i] + pm->s.delta_angles[i];
			pm->viewangles[i] = SHORT2ANGLE(temp);
		}

		// don't let the player look up or down more than 90 degrees
		if (pm->viewangles[PITCH] > 89.0f && pm->viewangles[PITCH] < 180.0f)
			pm->viewangles[PITCH] = 89.0f;
		else if (pm->viewangles[PITCH] < 271.0f && pm->viewangles[PITCH] >= 180.0f)
			pm->viewangles[PITCH] = 271.0f;
	}

	AngleVectors(pm->viewangles, pml.forward, pml.right, pml.up);
}

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove)
{
	g_stepheight = 0.0f; // jitmove
	pm = pmove;

	// clear results
	pm->numtouch = 0;
	VectorClear(pm->viewangles);
	pm->viewheight = 0;
	pm->groundentity = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// clear all pmove local vars
	memset(&pml, 0, sizeof(pml));

	// convert origin and velocity to float values
#ifdef USE_PMOVE_FLOAT
	if (1) // testing
	{
		// Floating point, for more precision (but possible prediciton misses)
		VectorCopy(pm->s.forigin, pml.origin);
		VectorCopy(pm->s.fvelocity, pml.velocity);
	}
	else
#endif
	{
		pml.origin[0] = pm->s.origin[0] * 0.125f;
		pml.origin[1] = pm->s.origin[1] * 0.125f;
		pml.origin[2] = pm->s.origin[2] * 0.125f;

		pml.velocity[0] = pm->s.velocity[0] * 0.125f;
		pml.velocity[1] = pm->s.velocity[1] * 0.125f;
		pml.velocity[2] = pm->s.velocity[2] * 0.125f;
	}

	// save old org in case we get stuck
	VectorCopy(pm->s.origin, pml.previous_origin);

	pml.frametime = (float)pm->cmd.msec * 0.001f;

	PM_ClampAngles();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		PM_FlyMove(false);
		PM_SnapPosition();
		return;
	}

	if (pm->s.pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->s.pm_type == PM_FREEZE)
		return;		// no movement at all

	// set mins, maxs, and viewheight
	PM_CheckDuck();

	if (pm->snapinitial)
		PM_InitialSnapPosition();

	// set groundentity, watertype, and waterlevel
	PM_CategorizePosition();

	if (!pm_oldmovephysics) // jitmove - call this after categorize position so the crouch checks take into account the current surface.
		PM_CheckDuck();

	if (pm->s.pm_type == PM_DEAD)
		PM_DeadMove();

	PM_CheckSpecialMovement();

	// drop timing counter
	if (pm->s.pm_time)
	{
		int msec;

		msec = pm->cmd.msec >> 3;

		if (!msec)
			msec = 1;

		if (msec >= pm->s.pm_time) 
		{
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | /*jitmove PMF_TIME_LAND |*/ PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}
		else
		{
			pm->s.pm_time -= msec;
		}
	}

	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{	// teleport pause stays exactly in place
	}
	else if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{	// waterjump has no control, but falls
		pml.velocity[2] -= pm->s.gravity * pml.frametime;

		if (pml.velocity[2] < 0)
		{	// cancel as soon as we are falling down again
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | /*jitmove PMF_TIME_LAND |*/ PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}

		PM_StepSlideMove();
	}
	else
	{
		PM_CheckJump();
		PM_Friction();

		if (pm->waterlevel >= 2)
		{
			PM_WaterMove();
		}
		else
		{
			vec3_t	angles;

			VectorCopy(pm->viewangles, angles);
			
			if (angles[PITCH] > 180.0f)
				angles[PITCH] = angles[PITCH] - 360.0f;

			angles[PITCH] /= 3.0f;
			AngleVectors(angles, pml.forward, pml.right, pml.up);
			PM_AirMove();
		}
	}

	// set groundentity, watertype, and waterlevel for final spot
	PM_CategorizePosition();
	PM_SnapPosition();
	g_viewheight = pm->viewheight; // jit
}


