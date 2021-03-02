#include "g_local.h"

#define SPECTATOR_DEBOUNCE			2.0

void ToggleChaseCam(edict_t *ent);

qboolean IsObserver(edict_t *ent)
{
	if (ent->client != NULL && ent->movetype == MOVETYPE_NOCLIP)
		return true;
	else
		return false;
}

// Done on disconnect, and on becoming a player
// If this isn't done on disconnect, a newly connected player
// may be considered an observer 
void markNotObserver(edict_t *ent) 
{
	ent->movetype = MOVETYPE_WALK;
	ent->svflags &= ~SVF_NOCLIENT;
}

void ObserverToPlayer(edict_t *ent)
{
	ObserverToTeam(ent, NOTEAM);
}

void ObserverToTeam(edict_t *ent, int teamToAssign)
{
	// Mark the client a non-spectator
	markNotObserver(ent);

	// put player on a team if in teamplay mode
	if (expflags & EXPERT_ENFORCED_TEAMS) {
		if (teamToAssign == NOTEAM) {
			// no team specified to assign to
			assignTeam(ent);
		} else {
			// assign to a specific team
			assignToTeam(ent, teamToAssign);
		}
	}
	
	// log the team change
	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
		gsTeamChange(ent->client->pers.netname, nameForTeam(ent->client->resp.team));

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	gi.bprintf(PRINT_HIGH, "%s left spectator mode\n", ent->client->pers.netname);

	// update client's cvar
	StuffCmd(ent, "set spectator 0 u\n");

	// debounce time to compensate for stuffcmd turnaround
	ent->client->resp.spectator_debounce = level.time + SPECTATOR_DEBOUNCE;
}

void PlayerToObserver(edict_t *ent)
{	
	// take the client out of play
	ClientLeavePlay(ent);
	
	if (expflags & EXPERT_ENFORCED_TEAMS) {
		// log the team change
		gsTeamChange(ent->client->pers.netname, GS_OBSERVER_TEAM);
	}

	// make observer
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.team = NOTEAM;
	ent->client->ps.gunindex = 0;
	ent->deadflag = DEAD_NO;
	gi.linkentity (ent);

	gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
	
	// update client's cvar
	StuffCmd(ent, "set spectator 1 u\n");

	// debounce time to compensate for stuffcmd turnaround
	ent->client->resp.spectator_debounce = level.time + SPECTATOR_DEBOUNCE;

	// enable chase cam
	ToggleChaseCam(ent);
}

// Change from a specatator to a player or player to spectator according to what
// the userinfo client cvar "specatator" is set to.
void spectatorStateChange(edict_t *ent) 
{
	// this debounce is necessary when the server changes whether the client is a player
	// or spectator, since the client's "spectator" setting will not agree until the 
	// stuffcmd to set the "specatator" cvar reaches the client.
	if (level.time > ent->client->resp.spectator_debounce) {	
		int spectator = atoi(Info_ValueForKey(ent->client->pers.userinfo, "spectator"));
		if (spectator != 0 && !IsObserver(ent) &&
			ent->client->resp.canObserve)
		{
			PlayerToObserver(ent);
		} else if (spectator == 0 && IsObserver(ent)) {
			ObserverToPlayer(ent);
		}
	}
}

// toggles observer mode
void Cmd_Observe(edict_t *ent)
{
	// player is currently an observer, make solid
	if (IsObserver(ent)) {
		ObserverToPlayer(ent);
	} else if (ent->client->resp.canObserve) {
		PlayerToObserver(ent);
	}
	// else too early in connection process, ignore

}

void ToggleChaseCam(edict_t *ent)
{
	int i;
	edict_t *e;

	// had ChaseCam enabled and now turning it off
	if (ent->client->chase_target) {
		ent->client->ps.pmove.pm_flags = 0;
		ent->client->chase_target = NULL;
		gi.cprintf(ent, PRINT_HIGH, "Chasecam disabled\n");
		return;
	}

	// enable ChaseCam, find any valid target
	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			ent->client->update_chase = true;
			break;
		}
	}

	if (!ent->client->chase_target) {
		gi.cprintf(ent, PRINT_HIGH, "No valid Chasecam targets\n");
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Chasecam enabled\n");
	}
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

	// is our chase target gone?
	if (!ent->client->chase_target->inuse) {
		// pick next target
		ChaseNext(ent);
		if (!ent->client->chase_target->inuse) {
			// no valid chase targets
			gi.cprintf(ent, PRINT_HIGH, "No more valid Chasecam targets\n");
			ToggleChaseCam(ent);
			return;
		}
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
		ent->client->ps.pmove.delta_angles[i] = 
				ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

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
		sprintf(s, "xv 180 yb -72 string2 \"Chasing %s\"",
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

	gi.cprintf(ent, PRINT_HIGH, "Switching Chasecam targets\n");

	i = ent->client->chase_target - g_edicts;
	do {
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
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

	gi.cprintf(ent, PRINT_HIGH, "Switching Chasecam targets\n");

	i = ent->client->chase_target - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

