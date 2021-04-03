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

void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	cmTrace_t trace;
	int i;
	vec3_t oldgoal;
	vec3_t angles;

	// is our chase target gone?
	if (!ent->client->chase_target->inUse
		|| ent->client->chase_target->client->resp.spectator) {
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old) {
			ent->client->chase_target = NULL;
			ent->client->ps.pMove.pmFlags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	targ = ent->client->chase_target;

	Vec3Copy(targ->s.origin, ownerv);
	Vec3Copy(ent->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	Vec3Copy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	Angles_Vectors (angles, forward, right, NULL);
	VectorNormalizef (forward, forward);
	Vec3MA (ownerv, -30, forward, o);

	if (o[2] < targ->s.origin[2] + 20)
		o[2] = targ->s.origin[2] + 20;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, vec3Origin, vec3Origin, o, targ, CONTENTS_MASK_SOLID);

	Vec3Copy(trace.endPos, goal);

	Vec3MA(goal, 2, forward, goal);

	// pad for floors and ceilings
	Vec3Copy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3Origin, vec3Origin, o, targ, CONTENTS_MASK_SOLID);
	if (trace.fraction < 1) {
		Vec3Copy(trace.endPos, goal);
		goal[2] -= 6;
	}

	Vec3Copy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3Origin, vec3Origin, o, targ, CONTENTS_MASK_SOLID);
	if (trace.fraction < 1) {
		Vec3Copy(trace.endPos, goal);
		goal[2] += 6;
	}

	if (targ->deadflag)
		ent->client->ps.pMove.pmType = PMT_DEAD;
	else
		ent->client->ps.pMove.pmType = PMT_FREEZE;

	Vec3Copy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pMove.deltaAngles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (targ->deadflag) {
		ent->client->ps.viewAngles[ROLL] = 40;
		ent->client->ps.viewAngles[PITCH] = -15;
		ent->client->ps.viewAngles[YAW] = targ->client->killer_yaw;
	} else {
		Vec3Copy(targ->client->v_angle, ent->client->ps.viewAngles);
		Vec3Copy(targ->client->v_angle, ent->client->v_angle);
	}

	ent->viewheight = 0;
	ent->client->ps.pMove.pmFlags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);
}

void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i++;
		if (i > maxclients->floatVal)
			i = 1;
		e = g_edicts + i;
		if (!e->inUse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->floatVal;
		e = g_edicts + i;
		if (!e->inUse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void GetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	for (i = 1; i <= maxclients->floatVal; i++) {
		other = g_edicts + i;
		if (other->inUse && !other->client->resp.spectator) {
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam(ent);
			return;
		}
	}
	gi.centerprintf(ent, "No other players to chase.");
}
