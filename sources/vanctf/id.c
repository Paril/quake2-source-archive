#include "g_local.h"
#include "id.h"

// does a simple trace to see if e1 can see e2.
static qboolean IsRoughlyVisible(edict_t *e1, edict_t *e2)
{
	trace_t tr;

	tr = gi.trace(e1->s.origin, NULL, NULL, e2->s.origin, e1, CONTENTS_SOLID|CONTENTS_MONSTER);
	return (tr.ent == e2);
//	if (tr.ent == e2)
//		return (true);
//	return (false);
}

// only calcs forward
static void AngleVectorForward(vec3_t angles, vec3_t forward)
{
	float		angle;
	static float		sp, sy, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
//	angle = angles[ROLL] * (M_PI*2 / 360);
//	sr = sin(angle);
//	cr = cos(angle);

	forward[0] = cp*cy;
	forward[1] = cp*sy;
	forward[2] = -sp;
}


void id_update_sbar(edict_t *ent) {
	vec3_t	forward, dir;
	edict_t	*e;
	float	bestdot = 0, dot;
	int		best, i;

	// loop thru all people and find out how 
	// far they are off of our line of sight
	AngleVectorForward(ent->client->v_angle, forward);
	for (i = 1; i <= maxclients->value; i++)
	{
		e = &g_edicts[i];
		// slot has to be used
		if (!e->inuse)
			continue;
		if (e->client->resp.spectator)
			continue;
		// other person has to be in my PVS
		if (!gi.inPVS(ent->s.origin, e->s.origin))
			continue;
		// other person has to be visible to me
		if (!IsRoughlyVisible(ent, e))
			continue;

		// figure out how far they are off of our line of sight
		VectorSubtract(e->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		dot = DotProduct(forward, dir);

		// they have to be closer to line of sight than everyone previous
		if (dot > bestdot/* && IsRoughlyVisible(ent, e)*/) 
		{
			bestdot = dot;
			best = i;
		}
	}

	// if the best player is very close to the line of sight,
	// then use him/her, otherwise clear it
	if (bestdot > 0.95) 
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_PLAYERSKINS + (best - 1);
	else
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
}

void Cmd_Id_f (edict_t *ent)
{
	ent->client->resp.id_state = !ent->client->resp.id_state;
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
	}
}


/*
void id_update_sbar(edict_t *ent)
{
	int sighted;
	sighted = SightedPlayer(ent);
	if (!sighted)
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	else
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_PLAYERSKINS + (sighted - 1);

}
*/