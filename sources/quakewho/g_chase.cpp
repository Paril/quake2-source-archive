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

#include "g_local.h"

constexpr vec3_t chase_offset { 0, 0, 6 };

void UpdateTargetCam(edict_t &ent)
{
	// is our chase target gone?
	if (!ent.control)
		gi.error("Controller went away!");

	const edict_t &targ = ent.control;
	const vec3_t ownerv = targ.s.origin + vec3_t{ 0, 0, targ.maxs[2] + targ.viewheight };
	const vec3_t angles { 0, ent.client->resp.cmd_angles[YAW] + SHORT2ANGLE(ent.client->ps.pmove.delta_angles[YAW]), 0 };

	vec3_t forward = angles.Forward();
	forward.Normalize();

	trace_t tr = gi.trace(targ.s.origin, targ.mins, targ.maxs, ownerv, targ, MASK_SOLID | CONTENTS_MONSTER);

	vec3_t goal = tr.endpos;

	tr = gi.trace(goal, targ.mins, targ.maxs, goal + (forward * -30.f), targ, MASK_SOLID | CONTENTS_MONSTER);

	goal = tr.endpos;

	ent.s.origin = goal;
	ent.viewheight = 0;
	ent.client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

	ent.Link();
}

void UpdateChaseCam(edict_t &ent)
{
	// is our chase target gone?
	if (!ent.client->chase_target || ent.client->chase_target->client->resp.spectator)
	{
		edict_ref &old = ent.client->chase_target;
		ChaseNext(ent);

		if (ent.client->chase_target == old)
		{
			ent.client->chase_target = nullptr;
			ent.client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	edict_t &targ = ent.client->chase_target;

	const vec3_t ownerv = targ.s.origin + vec3_t{ 0, 0, targ.viewheight };
	vec3_t angles = targ.client->v_angle;

	if (angles[PITCH] > 56)
		angles[PITCH] = 56;

	vec3_t forward, right;
	angles.AngleVectors(&forward, &right, nullptr);
	forward.Normalize();

	vec3_t o = ownerv + (forward * -30);

	if (o[2] < targ.s.origin[2] + 20)
		o[2] = targ.s.origin[2] + 20;

	// jump animation lifts
	if (!targ.groundentity)
		o[2] += 16;

	trace_t trace = gi.trace(ownerv, o, targ, MASK_SOLID);

	vec3_t goal = trace.endpos + (forward * 2);

	// pad for floors and ceilings
	o = goal + chase_offset;
	trace = gi.trace(goal, o, targ, MASK_SOLID);

	if (trace.fraction < 1)
		goal = trace.endpos - chase_offset;

	o = goal - chase_offset;
	trace = gi.trace(goal, o, targ, MASK_SOLID);

	if (trace.fraction < 1)
		goal = trace.endpos + chase_offset;

	if (targ.deadflag)
		ent.client->ps.pmove.pm_type = PM_DEAD;
	else
		ent.client->ps.pmove.pm_type = PM_FREEZE;

	ent.s.origin = goal;
	for (int32_t i = 0; i < 3; i++)
		ent.client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ.client->v_angle[i] - ent.client->resp.cmd_angles[i]);

	if (targ.deadflag)
		ent.client->ps.viewangles = { -15, targ.client->killer_yaw, 40 };
	else
		ent.client->v_angle = ent.client->ps.viewangles = targ.client->v_angle;

	ent.viewheight = 0;
	ent.client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

	ent.Link();
}

void ChaseNext(edict_t &ent)
{
	if (!ent.client->chase_target)
		return;

	size_t i = ent.client->chase_target->s.number;
	edict_ref e;
	
	do
	{
		i++;
		if (i > game.clients.size())
			i = 1;

		e = g_edicts[i];

		if (!e->inuse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent.client->chase_target);

	ent.client->chase_target = e;
	ent.client->update_chase = true;
}

void ChasePrev(edict_t &ent)
{
	if (!ent.client->chase_target)
		return;
	
	size_t i = ent.client->chase_target->s.number;
	edict_ref e;
	
	do
	{
		i--;
		if (i < 1)
			i = game.clients.size();

		e = g_edicts[i];

		if (!e->inuse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent.client->chase_target);

	ent.client->chase_target = e;
	ent.client->update_chase = true;
}

void GetChaseTarget(edict_t &ent)
{
	for (auto &other : game.players)
	{
		if (!other.inuse || other.client->resp.spectator)
			continue;

		ent.client->chase_target = other;
		ent.client->update_chase = true;
		UpdateChaseCam(ent);
		return;
	}

	ent.client->CenterPrint("No other players to chase.");
}

