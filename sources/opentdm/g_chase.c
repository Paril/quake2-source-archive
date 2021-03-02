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

void DisableChaseCam (edict_t *ent)
{
	//remove a gun model if we were using one for in-eyes
	ent->client->ps.gunframe = ent->client->ps.gunindex = 0;
	VectorClear (ent->client->ps.gunoffset);
	VectorClear (ent->client->ps.kick_angles);

	ent->client->ps.viewangles[ROLL] = 0;
	ent->client->chase_target = NULL;
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	ent->client->clientNum = ent - g_edicts - 1;

	// wision: disable freefloat if one of the teams is speclocked and the match is in progress
	if ((teaminfo[TEAM_A].speclocked || teaminfo[TEAM_B].speclocked) && !ent->client->pers.admin)
	{
		gi.centerprintf(ent, "Can't freefloat while teams are locked against spectators.");
		GetChaseTarget(ent);
	}
}

void NextChaseMode (edict_t *ent)
{
	if (tdm_match_status == MM_TIMEOUT)
		return;

	ent->client->chase_mode = (ent->client->chase_mode + 1) % CHASE_MAX;

	if (ent->client->chase_mode == CHASE_EYES)
	{
		//set clientnum to hide chased person on supported server
		ent->client->clientNum = g_edicts - ent->client->chase_target - 1;
	}
	else if (ent->client->chase_mode == CHASE_THIRDPERSON)
	{
		//going 3rd person, remove gun and invisible player
		ent->client->clientNum = ent - g_edicts -1;
		ent->client->ps.gunindex = ent->client->ps.gunframe = 0;
	}
	else if (ent->client->chase_mode == CHASE_FREE)
	{
		DisableChaseCam (ent);
	}
}

void UpdateLockCam(edict_t *ent)
{
	/* position on the map */
	ent->s.origin[0] = 0.0;
	ent->s.origin[1] = 0.0;
	ent->s.origin[2] = 4000.0;

	/* view */
	ent->client->ps.viewangles[ROLL] = 0;
	ent->client->ps.viewangles[PITCH] = 90;
	ent->client->ps.viewangles[YAW] = 0;

	ent->client->ps.pmove.pm_type = PM_FREEZE;
}

void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	vec3_t oldgoal;
	vec3_t angles;

	targ = ent->client->chase_target;

	// is our chase target gone? or team is speclocked and we are not allowed to spec
	if (!targ->inuse || !targ->client->pers.team || (teaminfo[targ->client->pers.team].speclocked &&
				!ent->client->pers.specinvite[targ->client->pers.team] && !ent->client->pers.admin))
	{
		ChaseNext(ent);
		if (ent->client->chase_target == targ)
		{
			DisableChaseCam (ent);
			return;
		}
	}

	/* update it again since it might be changed */
	targ = ent->client->chase_target;

	VectorCopy(targ->client->v_angle, angles);

	if (ent->client->chase_mode == CHASE_EYES)
	{
		VectorCopy (targ->s.origin, goal);
		goal[2] += targ->viewheight;

		//for old servers we have to spec in front of the player so they don't clip into the view
		if (!(game.server_features & GMF_CLIENTNUM))
		{
			vec3_t	targorigin;

			VectorCopy (goal, targorigin);

			AngleVectors (angles, forward, right, NULL);
			VectorMA (goal, 30, forward, goal);

			// trace from targorigin to final chase origin goal
			trace = gi.trace (targorigin, vec3_origin, vec3_origin, goal, targ, MASK_SOLID);

			// test for hit so we don't go out of the map!
			if (trace.fraction < 1)
			{
				vec3_t	temp;

				// we hit something, need to do a bit of avoidance

				// take real end point
				VectorCopy (trace.endpos, goal);

				// real dir vector
				VectorSubtract (goal, targorigin, temp);

				// scale it back bit more
				VectorMA (targorigin, 0.9f, temp, goal);
			}
		}
	}
	else if (ent->client->chase_mode == CHASE_THIRDPERSON)
	{
		VectorCopy(targ->s.origin, ownerv);
		VectorCopy(ent->s.origin, oldgoal);

		ownerv[2] += targ->viewheight;

		if (angles[PITCH] > 56)
			angles[PITCH] = 56;

		AngleVectors (angles, forward, right, NULL);
		VectorNormalize (forward);
		VectorMA (ownerv, -50, forward, o);

		if (o[2] < targ->s.origin[2] + 20)
			o[2] = targ->s.origin[2] + 20;

		// jump animation lifts
		if (!targ->groundentity)
			o[2] += 16;

		trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

		VectorCopy(trace.endpos, goal);

		VectorMA(goal, 2, forward, goal);

		// pad for floors and ceilings
		VectorCopy(goal, o);
		o[2] += 6;
		trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
		if (trace.fraction < 1)
		{
			VectorCopy(trace.endpos, goal);
			goal[2] -= 6;
		}

		VectorCopy(goal, o);
		o[2] -= 6;
		trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
		if (trace.fraction < 1)
		{
			VectorCopy(trace.endpos, goal);
			goal[2] += 6;
		}
	}

	VectorCopy(goal, ent->s.origin);
	
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (targ->deadflag)
	{
		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
		ent->client->ps.pmove.pm_type = PM_DEAD;
	}
	else
	{
		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
		ent->client->ps.pmove.pm_type = PM_FREEZE;
	}

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity (ent);
}

void SetChase (edict_t *ent, edict_t *target)
{
	if (target != ent->client->chase_target && ent->client->chase_mode == CHASE_EYES)
		ent->client->clientNum = target - g_edicts - 1;

	ent->client->chase_target = target;
	ent->client->update_chase = true;
}

void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target || tdm_match_status == MM_TIMEOUT)
		return;

	i = ent->client->chase_target - g_edicts;
	do
	{
		i++;
		if (i > game.maxclients)
			i = 1;

		e = g_edicts + i;

		if (!e->inuse)
			continue;

		if (e->client->pers.team && (ent->client->pers.admin || 
					!teaminfo[e->client->pers.team].speclocked || ent->client->pers.specinvite[e->client->pers.team]))
			break;
	} while (e != ent->client->chase_target);

	SetChase (ent, e);
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target || tdm_match_status == MM_TIMEOUT)
		return;

	i = ent->client->chase_target - g_edicts;
	do
	{
		i--;
		if (i < 1)
			i = game.maxclients;
		e = g_edicts + i;

		if (!e->inuse)
			continue;

		if (e->client->pers.team && (ent->client->pers.admin || 
					!teaminfo[e->client->pers.team].speclocked || ent->client->pers.specinvite[e->client->pers.team]))
			break;
	} while (e != ent->client->chase_target);

	SetChase (ent, e);
}

void GetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	if (tdm_match_status == MM_TIMEOUT)
		return;

	for (i = 1; i <= game.maxclients; i++)
	{
		other = g_edicts + i;
		if (other->inuse && other->client->pers.team && (ent->client->pers.admin ||
				!teaminfo[other->client->pers.team].speclocked || ent->client->pers.specinvite[other->client->pers.team]))
		{
			ent->client->chase_mode = CHASE_EYES;
			SetChase (ent, other);
			UpdateChaseCam(ent);
			return;
		}
	}

	/* lock spectator's view */
	if (teaminfo[TEAM_A].speclocked && teaminfo[TEAM_B].speclocked && !ent->client->pers.admin)
	{
		ent->client->chase_mode = CHASE_LOCK;
		gi.centerprintf(ent, "Both teams are locked against spectators.");
		UpdateLockCam(ent);
		return;
	}

	gi.centerprintf(ent, "No other players to chase.");
}

