#include "g_local.h"

// GRIM
void NoLongerChase (edict_t *ent)
{
        ent->client->chase_target = NULL;
        ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        ent->client->update_chase = false;
        ent->client->chase_mode = 0;
        if (ent->client->ps.fov != 90)
                ZoomOff (ent);
}
// GRIM

void UpdateChaseCam (edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	//vec3_t oldgoal;
	vec3_t angles;

	// is our chase target gone?
        if ((!ent->client->chase_target->inuse) || ent->client->chase_target->client->resp.spectator
          || ent->client->chase_target->movetype == MOVETYPE_NOCLIP)
        {
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old)
                {
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	targ = ent->client->chase_target;

        // Dirty
        if (ent->client->chase_mode == CHASE_POV)
        {
                ent->client->ps.pmove.pm_type = PM_DEAD;
                VectorCopy(targ->s.origin, ownerv);
                VectorCopy(targ->client->v_angle, angles);

                AngleVectors (angles, forward, right, NULL);
                VectorNormalize(forward);
                VectorMA(ownerv, 16, forward, o);

                o[2] += targ->viewheight;

                VectorCopy(o, ent->s.origin);

                for (i=0 ; i<3 ; i++)
                        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

                if (targ->deadflag)
                {
                        ent->client->ps.viewangles[ROLL] = 40;
                        ent->client->ps.viewangles[PITCH] = -15;
                        ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
                }
                else 
                {
                        VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
                        VectorCopy(targ->client->v_angle, ent->client->v_angle);
                }
        }       
        else //if (ent->client->chase_mode == CHASE_ROTATE)
        {
                if (ent->client->resp.cmd_angles[PITCH] > 89)
                        ent->client->resp.cmd_angles[PITCH] = 89;
                if (ent->client->resp.cmd_angles[PITCH] < -89)
                        ent->client->resp.cmd_angles[PITCH] = -89;

                VectorCopy(targ->s.origin, ownerv);

                ownerv[2] += targ->viewheight;

                VectorCopy(ent->client->ps.viewangles, angles);
                AngleVectors (angles, forward, right, NULL);
                VectorNormalize(forward);
                i = ((16 + ent->speed) * -1);
                VectorMA (ownerv, i, forward, o);

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

                if (targ->deadflag)
                        ent->client->ps.pmove.pm_type = PM_DEAD;
                else
                        ent->client->ps.pmove.pm_type = PM_FREEZE;              
 
                VectorCopy(goal, ent->s.origin);

                for (i=0 ; i<3 ; i++)
                        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

                VectorCopy (ent->client->resp.cmd_angles, ent->client->ps.viewangles);
        }

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
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
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
                // GRIM
                if ((!e->client->resp.spectator) && e->movetype != MOVETYPE_NOCLIP)
			break;
                // GRIM

	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
        ent->menu_time = level.time; // ParanoiD
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
                // GRIM
                if ((!e->client->resp.spectator) && e->movetype != MOVETYPE_NOCLIP)
			break;
                // GRIM
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
        ent->menu_time = level.time; // ParanoiD
}

void GetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	for (i = 1; i <= maxclients->value; i++)
        {
		other = g_edicts + i;
                // GRIM
                if ((other->client->resp.spectator == false) && other->inuse && (!(other->movetype == MOVETYPE_NOCLIP)))
                { // GRIM
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam(ent);
			return;
		}
	}
        //gi.centerprintf(ent, "No other players to chase.");
}

