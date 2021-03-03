/* g_chase.c */
#include "g_local.h"


void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	vec3_t oldgoal;
	vec3_t angles;

	// is our chase target gone?
	if (!ent->client->chase_target->inuse) {
		ent->client->chase_target = NULL;
		//K2: Just bring up the menu instead   HACK 3/99 acrid botcam
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;//3
		if(ctf->value)//3
			CTFOpenJoinMenu(ent);//3
		return;
	}

	targ = ent->client->chase_target;

	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(ent->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -30, forward, o);

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
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
	VectorCopy(targ->client->v_angle, ent->client->v_angle);

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);

	if ((!ent->client->showscores && !ent->client->menu &&
		!ent->client->showinventory && !ent->client->showhelp &&
		!(level.framenum & 31)) || ent->client->update_chase) {
		char s[1024];

		ent->client->update_chase = false;
		sprintf(s, "xv 0 yb -58 string2 \"Chasing %s\"",
			targ->client->pers.netname);
		gi.WriteByte (svc_layout);
		gi.WriteString (s);
		gi.unicast(ent, false);
	}

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
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;

		//No chasing bots 3/99 K2?
		if (e->bot_client)
			continue;

		if (e->solid != SOLID_NOT)
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
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse)
			continue;

		//No chasing bots //3/99 k2?
		if (e->bot_client)
			continue;

		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}
