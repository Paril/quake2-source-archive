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
// g_phys.c

#include "g_local.h"

/*
============
SV_TestEntityPosition

============
*/
static bool SV_TestEntityPosition (const edict_t &ent)
{
	const brushcontents_t mask = ent.clipmask ? ent.clipmask : MASK_SOLID;
	const trace_t trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, ent.s.origin, ent, mask);
	return trace.startsolid;
}

/*
================
SV_CheckVelocity
================
*/
static void SV_CheckVelocity (edict_t &ent)
{
	const vec_t length = ent.velocity.Length();

	if (length <= sv_maxvelocity->value)
		return;

	ent.velocity = ent.velocity.Normalized() * sv_maxvelocity->value;
}

/*
=============
SV_RunThink

Runs thinking code for this frame if necessary
=============
*/
static bool SV_RunThink (edict_t &ent)
{
	vec_t thinktime = ent.nextthink;
	
	if (!thinktime || thinktime > level.time)
		return true;
	
	ent.nextthink = 0;
	
	if (ent.think)
		ent.think (ent);

	return false;
}

/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
static void SV_Impact (edict_t &e1, const trace_t &trace)
{
	edict_ref e2 = trace.ent;

	if (e1.touch && e1.solid != SOLID_NOT)
		e1.touch (e1, e2, &trace.plane, trace.surface);
	
	if (e2->touch && e2->solid != SOLID_NOT)
		e2->touch (e2, e1, nullptr, nullptr);
}

/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
constexpr vec_t STOP_EPSILON = 0.1;

void ClipVelocity (const vec3_t &in, const vec3_t &normal, vec3_t &out, const vec_t &overbounce)
{
	const vec_t backoff = in.Dot(normal) * overbounce;

	for (size_t i = 0; i < 3; i++)
	{
		const vec_t change = normal[i] * backoff;
		out[i] = in[i] - change;
		
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
}

/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
============
*/
constexpr size_t MAX_CLIP_PLANES	= 5;
constexpr size_t NUM_BUMPS			= 4;

static void SV_FlyMove (edict_t &ent, const vec_t &time, const brushcontents_t &mask)
{
	const vec3_t primal_velocity = ent.velocity;
	vec3_t original_velocity = ent.velocity;
	size_t numplanes = 0;
	vec3_t planes[MAX_CLIP_PLANES];
	vec_t time_left = time;

	ent.groundentity = nullptr;

	for (size_t bumpcount = 0; bumpcount < NUM_BUMPS; bumpcount++)
	{
		const vec3_t end = ent.s.origin + (ent.velocity * time_left);
		trace_t trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, end, ent, mask);

		if (trace.allsolid)
		{
			// entity is trapped in another solid
			ent.velocity = vec3_origin;
			return;
		}

		if (trace.fraction > 0)
		{
			// actually covered some distance
			ent.s.origin = trace.endpos;
			original_velocity = ent.velocity;
			numplanes = 0;
		}

		if (trace.fraction == 1)
			break;		// moved the entire distance

		edict_t &hit = trace.ent;

		if (trace.plane.normal[2] > 0.7f && hit.solid == SOLID_BSP)
		{
			ent.groundentity = hit;
			ent.groundentity_linkcount = hit.linkcount;
		}

		//
		// run the impact function
		//
		SV_Impact (ent, trace);
		
		if (!ent.inuse)
			break;		// removed by the impact function

		time_left -= time_left * trace.fraction;

		// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{
			// this shouldn't really happen
			ent.velocity = vec3_origin;
			return;
		}

		planes[numplanes++] = trace.plane.normal;

		//
		// modify original_velocity so it parallels all of the clip planes
		//
		vec3_t new_velocity = vec3_origin;
		size_t i = 0;

		for (; i < numplanes; i++)
		{
			size_t j = 0;
			
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);

			for (; j < numplanes; j++)
				if (j != i && planes[i] != planes[j] && new_velocity.Dot(planes[j]) < 0)
					break;	// not ok

			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
			ent.velocity = new_velocity; // go along this plane
		else
		{	// go along the crease
			if (numplanes != 2)
			{
				ent.velocity = vec3_origin;
				return;
			}

			const vec3_t dir = planes[0].Cross(planes[1]);
			const vec_t d = dir.Dot(ent.velocity);
			ent.velocity = dir * d;
		}

		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (ent.velocity.Dot(primal_velocity) <= 0)
			ent.velocity = vec3_origin;
	}
}


/*
============
SV_AddGravity

============
*/
inline void SV_AddGravity (edict_t &ent)
{
	ent.velocity[2] -= ent.gravity * sv_gravity->value * FRAME_S;
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
static trace_t SV_PushEntity (edict_t &ent, const vec3_t &push)
{
	const vec3_t start = ent.s.origin;
	const vec3_t end = start + push;

retry:
	const brushcontents_t mask = ent.clipmask ? ent.clipmask : MASK_SOLID;
	const trace_t trace = gi.trace (start, ent.mins, ent.maxs, end, ent, mask);
	
	ent.s.origin = trace.endpos;
	ent.Link();

	if (trace.fraction != 1.0f)
	{
		SV_Impact(ent, trace);

		// if the pushed entity went away and the pusher is still there
		if (!trace.ent->inuse && ent.inuse)
		{
			// move the pusher back and try again
			ent.s.origin = start;
			ent.Link();
			goto retry;
		}
	}

	if (ent.inuse)
		G_TouchTriggers (ent);

	return trace;
}					

struct pushed_t
{
	edict_ref	ent;
	vec3_t		origin;
	vec3_t		angles;
	vec_t		deltayaw;
};

static pushed_t pushed[MAX_EDICTS], *pushed_p;

/*
============
SV_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/
static edict_ref SV_Push (edict_t &pusher, const vec3_t &move, const vec3_t &amove)
{
	// find the bounding box
	const vec3_t mins = pusher.absmin + move;
	const vec3_t maxs = pusher.absmax + move;

// we need this for pushing things later
	const auto &[ forward, right, up ] = (-amove).AngleVectors();

// save the pusher's original position
	pushed_p->ent = pusher;
	pushed_p->origin = pusher.s.origin;
	pushed_p->angles = pusher.s.angles;
	if (pusher.client)
		pushed_p->deltayaw = pusher.client->ps.pmove.delta_angles[YAW];
	pushed_p++;

// move the pusher to it's final position
	pusher.s.origin += move;
	pusher.s.angles += amove;
	pusher.Link();

// see if any solid entities are inside the final position
	for (auto &check : game.entities.range(1))
	{
		if (!check.inuse)
			continue;

		if (check.movetype == MOVETYPE_PUSH
		|| check.movetype == MOVETYPE_STOP
		|| check.movetype == MOVETYPE_NONE
		|| check.movetype == MOVETYPE_NOCLIP)
			continue;

		if (!check.area.prev)
			continue;		// not linked in anywhere

	// if the entity is standing on the pusher, it will definitely be moved
		if (check.groundentity != pusher)
		{
			// see if the ent needs to be tested
			if (check.absmin[0] >= maxs[0]
			|| check.absmin[1] >= maxs[1]
			|| check.absmin[2] >= maxs[2]
			|| check.absmax[0] <= mins[0]
			|| check.absmax[1] <= mins[1]
			|| check.absmax[2] <= mins[2])
				continue;

			// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		if (pusher.movetype == MOVETYPE_PUSH || check.groundentity == pusher)
		{
			// move this entity
			pushed_p->ent = check;
			pushed_p->origin = check.s.origin;
			pushed_p->angles = check.s.angles;
			pushed_p++;

			// try moving the contacted entity 
			check.s.origin += move;
			if (check.client)
				check.client->ps.pmove.delta_angles[YAW] += amove[YAW];
			else if (check.svflags & SVF_MONSTER)
				check.s.angles[YAW] += amove[YAW];

			// figure movement due to the pusher's amove
			const vec3_t org = check.s.origin - pusher.s.origin;
			const vec3_t org2 = { org.Dot(forward), -org.Dot(right), org.Dot(up) };
			const vec3_t move2 = org2 - org;
			check.s.origin += move2;

			// may have pushed them off an edge
			if (check.groundentity != pusher)
				check.groundentity = nullptr;

			if (!SV_TestEntityPosition (check))
			{
				// pushed ok
				check.Link();
				// impact?
				continue;
			}

			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			check.s.origin -= move;
			if (!SV_TestEntityPosition (check))
			{
				pushed_p--;
				continue;
			}
		}

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for (pushed_t *p = pushed_p - 1; p >= pushed; p--)
		{
			p->ent->s.origin = p->origin;
			p->ent->s.angles = p->angles;

			if (p->ent->client)
				p->ent->client->ps.pmove.delta_angles[YAW] = p->deltayaw;

			p->ent->Link();
		}

		return check;
	}

	//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for (pushed_t *p = pushed_p - 1; p >= pushed; p--)
		G_TouchTriggers (p->ent);

	return nullptr;
}

static vec_t SV_ClampMove(const vec_t &v)
{
	vec_t temp = v * 8.0f;

	if (temp > 0.0f)
		temp += 0.5f;
	else
		temp -= 0.5f;

	return 0.125f * static_cast<int32_t>(temp);
}

/*
================
SV_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
static void SV_Physics_Pusher (edict_t &ent)
{
	// if not a team captain, so movement will be handled elsewhere
	if (ent.flags & FL_TEAMSLAVE)
		return;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;

	edict_ref part = ent, obstacle;

	for (part = ent; part; part = part->teamchain)
	{
		if (part->velocity[0] || part->velocity[1] || part->velocity[2] ||
			part->avelocity[0] || part->avelocity[1] || part->avelocity[2])
		{	// object is moving
			const vec3_t move = (part->velocity * FRAME_S).Apply(SV_ClampMove);
			const vec3_t amove = part->avelocity * FRAME_S;

			if ((obstacle = SV_Push (part, move, amove)))
				break;	// move was blocked
		}
	}

	if (pushed_p > &pushed[MAX_EDICTS])
		gi.error ("pushed_p > &pushed[MAX_EDICTS], memory corrupted");

	if (part)
	{
		// the move failed, bump all nextthink times and back out moves
		for (edict_ref mv = ent; mv; mv = mv->teamchain)
			if (mv->nextthink > 0)
				mv->nextthink += FRAME_MS;

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		if (part->blocked)
			part->blocked (part, obstacle);

		return;
	}

	// the move succeeded, so call all think functions
	for (part = ent; part; part = part->teamchain)
		SV_RunThink (part);
}

//==================================================================

/*
=============
SV_Physics_None

Non moving objects can only think
=============
*/
static void SV_Physics_None (edict_t &ent)
{
	// regular thinking
	SV_RunThink (ent);
}

/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
static void SV_Physics_Noclip (edict_t &ent)
{
	// regular thinking
	if (!SV_RunThink (ent))
		return;
	
	ent.s.angles += ent.avelocity * FRAME_S;
	ent.s.origin += ent.velocity * FRAME_S;

	ent.Link();
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
static void SV_Physics_Toss (edict_t &ent)
{
// regular thinking
	SV_RunThink (ent);

	// if not a team captain, so movement will be handled elsewhere
	if (ent.flags & FL_TEAMSLAVE)
		return;

	if (ent.velocity[2] > 0)
		ent.groundentity = nullptr;

// if onground, return without moving
	if (ent.groundentity)
		return;

	const vec3_t old_origin = ent.s.origin;

	SV_CheckVelocity (ent);

// add gravity
	if (ent.movetype != MOVETYPE_FLY && ent.movetype != MOVETYPE_FLYMISSILE)
		SV_AddGravity (ent);

// move angles
	ent.s.angles += ent.avelocity * FRAME_S;

// move origin
	const vec3_t move = ent.velocity * FRAME_S;
	const trace_t &trace = SV_PushEntity (ent, move);
	if (!ent.inuse)
		return;

	if (trace.fraction < 1)
	{
		const vec_t backoff = (ent.movetype == MOVETYPE_BOUNCE) ? 1.5 : 1.0;

		ClipVelocity (ent.velocity, trace.plane.normal, ent.velocity, backoff);

	// stop if on ground
		if (trace.plane.normal[2] > 0.7f)
		{		
			if (ent.velocity[2] < 60 || ent.movetype != MOVETYPE_BOUNCE )
			{
				ent.groundentity = trace.ent;
				ent.groundentity_linkcount = trace.ent->linkcount;
				ent.velocity = vec3_origin;
				ent.avelocity = vec3_origin;
			}
		}

//		if (ent->touch)
//			ent->touch (ent, trace.ent, &trace.plane, trace.surface);
	}
	
// check for water transition
	const bool wasinwater = (ent.watertype & MASK_WATER);
	ent.watertype = gi.pointcontents (ent.s.origin);

	const bool isinwater = ent.watertype & MASK_WATER;
	ent.waterlevel = isinwater ? WATER_FEET : WATER_NONE;

	if (!wasinwater && isinwater)
		game.world().PlayPositionedSound(gi.soundindex("misc/h2ohit1.wav"), old_origin);
	else if (wasinwater && !isinwater)
		game.world().PlayPositionedSound (gi.soundindex("misc/h2ohit1.wav"), ent.s.origin);

// move teamslaves
	for (edict_ref slave = ent.teamchain; slave; slave = slave->teamchain)
	{
		slave->s.origin = ent.s.origin;
		slave->Link();
	}
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

//FIXME: hacked in for E3 demo
constexpr vec_t sv_stopspeed		= 100;
constexpr vec_t sv_friction			= 6;

static void SV_AddRotationalFriction (edict_t &ent)
{
	ent.s.angles += ent.avelocity * FRAME_S;
	
	constexpr vec_t adjustment = FRAME_S * sv_stopspeed * sv_friction;
	
	for (auto &v : ent.avelocity)
	{
		if (v > 0)
			v = max(0.0f, v - adjustment);
		else
			v = min(0.0f, v + adjustment);
	}
}

static void SV_Physics_Step (edict_t &ent)
{
	// airborn monsters should always check for ground
	if (!ent.groundentity)
		M_CheckGround (ent);

	SV_CheckVelocity (ent);

	const bool wasonground = !!ent.groundentity;
		
	if (ent.avelocity[0] || ent.avelocity[1] || ent.avelocity[2])
		SV_AddRotationalFriction (ent);

	bool hitsound = false;

	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if (!wasonground)
	{
		if (ent.velocity[2] < sv_gravity->value * -0.1f)
			hitsound = true;
		if (ent.waterlevel != WATER_UNDER)
			SV_AddGravity (ent);
	}

	if (!wasonground && (ent.velocity[2] || ent.velocity[1] || ent.velocity[0]))
	{
		const brushcontents_t mask = (ent.svflags & SVF_MONSTER) ? MASK_MONSTERSOLID : MASK_SOLID;

		SV_FlyMove (ent, FRAME_S, mask);

		ent.Link();

		G_TouchTriggers (ent);

		if (!ent.inuse)
			return;

		if (ent.groundentity && !wasonground && hitsound)
			ent.PlaySound(gi.soundindex("world/land.wav"));
	}

	// regular thinking
	SV_RunThink (ent);
}

//============================================================================
/*
================
G_RunEntity

================
*/
void G_RunEntity (edict_t &ent)
{
	if (ent.prethink)
		ent.prethink (ent);

	switch (ent.movetype)
	{
	case MOVETYPE_WALK:
		break;
	case MOVETYPE_PUSH:
	case MOVETYPE_STOP:
		SV_Physics_Pusher (ent);
		break;
	case MOVETYPE_NONE:
		SV_Physics_None (ent);
		break;
	case MOVETYPE_NOCLIP:
		SV_Physics_Noclip (ent);
		break;
	case MOVETYPE_STEP:
		SV_Physics_Step (ent);
		break;
	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
	case MOVETYPE_FLY:
	case MOVETYPE_FLYMISSILE:
		SV_Physics_Toss (ent);
		break;
	}
}
