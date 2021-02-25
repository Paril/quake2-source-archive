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
// m_move.c -- monster movement

#include "g_local.h"

constexpr vec_t STEPSIZE = 18;

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
bool M_CheckBottom (edict_t &ent)
{
	return gi.trace(ent.s.origin, ent.mins, ent.maxs, { ent.s.origin[0], ent.s.origin[1], ent.s.origin[2] - STEPSIZE }, ent, MASK_SOLID).fraction < 1.0f;
}

#include <unordered_set>

/*
==================
PM_StepSlideMove

Each intersection will try to step over the obstruction instead of
sliding along it.

Returns a new origin, velocity, and contact entity
Does not modify any world state?
==================
*/
constexpr vec_t MIN_STEP_NORMAL		= 0.7f;		// can't step up onto very steep slopes
constexpr size_t MAX_CLIP_PLANES	= 5;
constexpr size_t MAX_BUMPS = 4;

static void M_StepSlideMove_ (edict_t &ent, vec3_t &velocity, std::unordered_set<edict_ref> &touchents)
{
	size_t numplanes = 0;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity = velocity;
	vec_t time_left = 1.0;

	for (size_t bumpcount = 0; bumpcount < MAX_BUMPS; bumpcount++)
	{
		const vec3_t end = ent.s.origin + (velocity * time_left);
		const trace_t trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, end, ent, MASK_MONSTERSOLID);

		if (trace.allsolid)
		{
			// entity is trapped in another solid
			velocity[2] = 0;	// don't build up falling damage
			return;
		}

		if (trace.fraction > 0)
		{
			// actually covered some distance
			ent.s.origin = trace.endpos;
			numplanes = 0;
		}

		if (trace.fraction == 1)
			break;		// moved the entire distance

		// save entity for contact
		if (trace.ent)
			touchents.emplace(trace.ent);
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if (numplanes >= MAX_CLIP_PLANES)
		{
			// this shouldn't really happen
			velocity.Clear();
			break;
		}

		planes[numplanes] = trace.plane.normal;
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		size_t i = 0;

		for (; i < numplanes; i++)
		{
			ClipVelocity (velocity, planes[i], velocity, 1.01f);

			size_t j = 0;
			for (; j < numplanes; j++)
				if (j != i && velocity.Dot(planes[j]) < 0)
					break;	// not ok

			if (j == numplanes)
				break;
		}
		
		if (i == numplanes)
		{
			// go along the crease
			if (numplanes != 2)
			{
				velocity.Clear();
				break;
			}

			const vec3_t dir = planes[0].Cross(planes[1]);
			const vec_t d = dir.Dot(velocity);
			velocity = dir * d;
		}

		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		if (velocity.Dot(primal_velocity) <= 0)
		{
			velocity.Clear();
			break;
		}
	}

	velocity = primal_velocity;
}

/*
==================
PM_StepSlideMove

==================
*/
static bool M_StepSlideMove (edict_t &ent, vec3_t &velocity, std::unordered_set<edict_ref> &touch_ents)
{
	const vec3_t start_o = ent.s.origin;
	const vec3_t start_v = velocity;

	M_StepSlideMove_ (ent, velocity, touch_ents);

	const vec3_t down_o = ent.s.origin;
	const vec3_t down_v = velocity;

	vec3_t up = start_o;
	up[2] += STEPSIZE;

	trace_t trace = gi.trace (up, ent.mins, ent.maxs, up, ent, MASK_MONSTERSOLID);

	if (!trace.allsolid)
	{
		// try sliding above
		ent.s.origin = up;
		velocity = start_v;

		M_StepSlideMove_ (ent, velocity, touch_ents);

		// push down the final amount
		vec3_t down = ent.s.origin;
		down[2] -= STEPSIZE * 2;
		trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, down, ent, MASK_MONSTERSOLID);

		if (!trace.allsolid)
			ent.s.origin = trace.endpos;

		up = ent.s.origin;

		// decide which one went farther
		const vec_t down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
		const vec_t up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);

		if (down_dist > up_dist || trace.plane.normal[2] < MIN_STEP_NORMAL)
		{
			ent.s.origin = down_o;
			velocity = down_v;
			return true;
		}
	}

	//!! Special case
	// if we were walking along a plane, then we need to copy the Z over
	velocity[2] = down_v[2];
	return true;
}

static bool M_CanStepDownToFollow(edict_t &ent, edict_t &other)
{
	return (ent.absmin[2] - other.absmin[2]) > STEPSIZE;
}

/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
static bool SV_movestep (edict_t &ent, const vec3_t &move, const bool &relink)
{
	const vec3_t start_o = ent.s.origin;
	std::unordered_set<edict_ref> touch_ents;
	vec3_t velocity = move;

	bool moved = M_StepSlideMove(ent, velocity, touch_ents);

	// touch other objects
	for (auto other : touch_ents)
		if (other->touch)
			other->touch (other, ent, nullptr, nullptr);

	if (!moved)
		return false;

	if (!M_CheckBottom (ent) && (!ent.control || !ent.control->client->cmd.upmove) && (!ent.monsterinfo.follow_ent || !M_CanStepDownToFollow(ent, ent.monsterinfo.follow_ent)))
	{
		ent.s.origin = start_o;
		return false;
	}

	M_CatagorizePosition(ent);

	// don't go in to water
	if (ent.waterlevel == WATER_UNDER)
	{
		ent.s.origin = start_o;
		M_CatagorizePosition(ent);
		return false;
	}

	if (relink)
	{
		ent.Link();
		G_TouchTriggers (ent);
	}

	return true;
}
//============================================================================

/*
===============
M_ChangeYaw

===============
*/
static void M_ChangeYaw (edict_t &ent)
{
	const vec_t current = anglemod(ent.s.angles[YAW]);
	const vec_t &ideal = ent.ideal_yaw;

	if (current == ideal)
		return;

	vec_t move = ideal - current;
	const vec_t &speed = ent.yaw_speed;

	if (ideal > current)
	{
		if (move >= 180)
			move -= 360;
	}
	else
	{
		if (move <= -180)
			move += 360;
	}

	if (move > 0)
		move = min(move, speed);
	else
		move = max(move, -speed);

	ent.s.angles[YAW] = anglemod(current + move);
}


/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
static bool SV_StepDirection (edict_t &ent, vec_t yaw, const vec_t &dist)
{
	ent.ideal_yaw = yaw;
	M_ChangeYaw (ent);

	if (!dist)
		return true;
	
	yaw = yaw*M_PI*2 / 360;
	const vec3_t move = { 
		cos(yaw)*dist,
		sin(yaw)*dist,
		0
	};

	const vec3_t oldorigin = ent.s.origin;

	if (SV_movestep (ent, move, false))
	{
		ent.Link();
		G_TouchTriggers (ent);

		const vec3_t v = oldorigin - ent.s.origin;

		if (ent.monsterinfo.follow_ent && v.Length() < dist * 0.4f)
		{
			if (M_GonnaHitSpecificThing(ent, game.world()))
			{
				ent.monsterinfo.follow_ent = nullptr;
				ent.monsterinfo.follow_check = level.time + frandom(4000, 24000);
			}
		}

		if (v.Length() < dist * 0.1f)
			return false;

		return true;
	}

	ent.Link();
	G_TouchTriggers (ent);
	return false;
}

/*
======================
M_GonnaHitSomething
======================
*/
static bool M_GonnaHitSomething(edict_t &ent)
{
	const vec3_t forward = ent.s.angles.Forward();
	const vec3_t end = ent.s.origin + (forward * 10);
	return gi.trace(ent.s.origin, ent.mins, ent.maxs, end, ent, MASK_PLAYERSOLID).fraction != 1.0f;
}

/*
======================
M_GonnaHitSomething
======================
*/
bool M_GonnaHitSpecificThing(edict_t &ent, edict_t &other)
{
	const vec3_t forward = ent.s.angles.Forward();
	const vec3_t end = ent.s.origin + (forward * 16);
	const trace_t tr = gi.trace(ent.s.origin, ent.mins, ent.maxs, end, ent, MASK_PLAYERSOLID);
	
	return tr.fraction != 1.0f && tr.ent == other;
}

/*
======================
M_MoveToGoal
======================
*/
void M_MoveToGoal (edict_t &ent, const vec_t &dist)
{
	if (ent.monsterinfo.follow_ent)
	{
		if (M_GonnaHitSpecificThing(ent, ent.monsterinfo.follow_ent))
		{
			ent.monsterinfo.stand(ent);
			return;
		}

		ent.ideal_yaw = (ent.monsterinfo.follow_ent->s.origin - ent.s.origin).Normalized().ToYaw();
	}
	else if (level.time > ent.monsterinfo.should_stand_check && ent.monsterinfo.stubborn_time < level.time)
	{
		ent.monsterinfo.should_stand_check = level.time + frandom(1000, 24000);

		if (prandom(50))
		{
			ent.monsterinfo.stand(ent);
			return;
		}
	}

	if ((!SV_StepDirection (ent, ent.ideal_yaw, dist) || M_GonnaHitSomething(ent)) && ent.monsterinfo.stubborn_time < level.time)
	{
		if (ent.inuse && FacingIdeal(ent) && prandom(65))
			ent.ideal_yaw = frandom(360);
	}
}

void M_MoveToController (edict_t &self, const vec_t &dist, const bool &turn)
{
	edict_t &ent = self.control;

	if (turn && !ent.client->cmd.forwardmove && !ent.client->cmd.sidemove)
	{
		self.monsterinfo.stand(self);
		return;
	}

	const vec3_t view_angles = {
		0, ent.client->resp.cmd_angles[YAW] + SHORT2ANGLE(ent.client->ps.pmove.delta_angles[YAW]), 0
	};

	if (turn && dist >= 0)
	{
		self.control_dist = dist;

		if (ent.client->cmd.upmove >= 0)
			self.ideal_yaw = view_angles[YAW];

		M_ChangeYaw (self);
	}

	if (dist <= 0)
		return;

	const vec3_t move_angles = {
		0, (ent.client->cmd.upmove >= 0) ? view_angles[1] : self.ideal_yaw, 0
	};

	vec3_t forward, right;

	move_angles.AngleVectors(&forward, &right, nullptr);

	vec3_t wanted = { static_cast<vec_t>(ent.client->cmd.forwardmove), static_cast<vec_t>(ent.client->cmd.sidemove), 0 };
	vec_t wanted_len = wanted.Normalize();

	if (!wanted_len)
		return;

	vec3_t move;

	for (int32_t i = 0; i < 3; i++)
		move[i] = forward[i] * wanted[0] * dist + right[i] * wanted[1] * dist;

	SV_movestep (self, move, false);

	self.Link();
	G_TouchTriggers (self);
}

/*
===============
M_walkmove
===============
*/
bool M_walkmove (edict_t &ent, vec_t yaw, const vec_t &dist)
{
	yaw = yaw*M_PI*2 / 360;
	
	const vec3_t move = {
		cos(yaw)*dist,
		sin(yaw)*dist,
		0
	};

	return SV_movestep(ent, move, true);
}
