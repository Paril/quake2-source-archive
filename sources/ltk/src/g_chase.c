// This source file comes from the 3.20 source originally.
// 
// Added through-the-eyes cam mode, as well as the ability to spin around the player
// when in regular chase cam mode, among other Axshun-related mods.
// -Fireblade

#include "g_local.h"

int ChaseTargetGone(edict_t *ent)
{
        // is our chase target gone?
        if (!ent->client->chase_target->inuse
                || (ent->client->chase_target->solid == SOLID_NOT &&
                        ent->client->chase_target->deadflag != DEAD_DEAD))
        {
                edict_t *old = ent->client->chase_target;
                ChaseNext(ent);
                if (ent->client->chase_target == old)
                {
                        ent->client->chase_target = NULL;
                        ent->client->desired_fov = 90;
                        ent->client->ps.fov = 90;
                        ent->client->chase_mode = 0;
                        ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                        return 1;
                }
        }
        return 0;
}

void UpdateChaseCam(edict_t *ent)
{
        vec3_t o, ownerv, goal;
        edict_t *targ;
        vec3_t forward, right;
        trace_t trace;
        int i;
        vec3_t angles;

        if (ChaseTargetGone(ent))
                return;

        targ = ent->client->resp.last_chase_target = ent->client->chase_target;

        if (ent->client->chase_mode == 1)
        {
                ent->client->desired_fov = 90;
                ent->client->ps.fov = 90;

                if (ent->client->resp.cmd_angles[PITCH] > 89)
                        ent->client->resp.cmd_angles[PITCH] = 89;
                if (ent->client->resp.cmd_angles[PITCH] < -89)
                        ent->client->resp.cmd_angles[PITCH] = -89;

                VectorCopy(targ->s.origin, ownerv);

                ownerv[2] += targ->viewheight;

                VectorCopy(ent->client->ps.viewangles, angles);
                AngleVectors (angles, forward, right, NULL);
                VectorNormalize(forward);
                VectorMA(ownerv, -150, forward, o);

// not sure if this should be left in... -FB
//              if (o[2] < targ->s.origin[2] + 20)  
//                      o[2] = targ->s.origin[2] + 20;

                // jump animation lifts
                if (!targ->groundentity)
                        o[2] += 16;

                PRETRACE();
                trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);        
                POSTTRACE();
 
                VectorCopy(trace.endpos, goal);

                VectorMA(goal, 2, forward, goal);

                // pad for floors and ceilings
                VectorCopy(goal, o);
                o[2] += 6;
                PRETRACE();
                trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
                POSTTRACE();
                if (trace.fraction < 1) {
                        VectorCopy(trace.endpos, goal);
                        goal[2] -= 6;
                }

                VectorCopy(goal, o);
                o[2] -= 6;
                PRETRACE();
                trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
                POSTTRACE();
                if (trace.fraction < 1) {
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

                VectorCopy(ent->client->resp.cmd_angles, ent->client->ps.viewangles);
        }
                else // chase_mode == 2
        {
                VectorCopy(targ->s.origin, ownerv);
                VectorCopy(targ->client->v_angle, angles);

                AngleVectors (angles, forward, right, NULL);
                VectorNormalize(forward);
                VectorMA(ownerv, 16, forward, o);

                o[2] += targ->viewheight;

                VectorCopy(o, ent->s.origin);

                ent->client->ps.fov = targ->client->ps.fov;
                ent->client->desired_fov = targ->client->ps.fov;

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
                        VectorAdd(targ->client->v_angle,
                                targ->client->ps.kick_angles,
                                angles);
                        VectorCopy(angles, ent->client->ps.viewangles);
                        VectorCopy(angles, ent->client->v_angle);
                }
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
                                //Black Cross - Begin
                                if (teamplay->value && limchasecam->value &&
                                          ent->client->resp.team != NOTEAM &&
                                          ent->client->resp.team != e->client->resp.team)
                                        continue;
                                //Black Cross - End
                if (e->solid != SOLID_NOT || e->deadflag == DEAD_DEAD)
                        break;
        } while (e != ent->client->chase_target);

        ent->client->chase_target = e;
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
                                //Black Cross - Begin
                                if (teamplay->value && limchasecam->value &&
                                                ent->client->resp.team != NOTEAM &&
                                                ent->client->resp.team != e->client->resp.team)
                                        continue;
                                //Black Cross - End
                if (e->solid != SOLID_NOT || e->deadflag == DEAD_DEAD)
                        break;
        } while (e != ent->client->chase_target);

        ent->client->chase_target = e;
}

void GetChaseTarget(edict_t *ent)
{
        int i, found, searched;
        edict_t *e, *start_target;

        start_target = ent->client->resp.last_chase_target;

        if (start_target == NULL)
        {
                start_target = g_edicts + 1;
        }
                else
        {
                if (start_target < (g_edicts + 1) ||
                         start_target > (g_edicts + game.maxclients))
                {
                        gi.dprintf("Warning: start_target ended up out of range\n");
                }
        }

        i = (start_target - g_edicts) + 1;
        found = searched = 0;
        do
        {
                searched++;
                i--;
                if (i < 1)
                        i = game.maxclients;
                e = g_edicts + i;
                if (!e->inuse)
                        continue;
                                //Black Cross - Begin
                                if (teamplay->value && limchasecam->value &&
                                                ent->client->resp.team != NOTEAM &&
                                                ent->client->resp.team != e->client->resp.team)
                                        continue;
                                //Black Cross - End
                if (e->solid != SOLID_NOT || e->deadflag == DEAD_DEAD)
                {
                        found = 1;
                        break;
                }
        } while ((e != (start_target + 1)) && searched < 100);

        if (searched >= 100)
        {
                gi.dprintf("Warning: prevented loop in GetChaseTarget\n");
        }

        if (found)
        {
                ent->client->chase_target = e;
        }
}
