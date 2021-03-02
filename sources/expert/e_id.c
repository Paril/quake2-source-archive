#include "g_local.h"

/*
 * Player ID system
 * Adapted from CTF ID system in Quake 2 Threewave CTF 1.02
 */

void Cmd_ID_f (edict_t *ent)
{
	// FIXME no op - only server can turn off playerid
	return;
/*
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
*/
}

void SetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	ent->client->ps.stats[STAT_PLAYER_ID] = 0;

	if (!(expflags & EXPERT_PLAYERID)) 
	{
		return;
	}

	// see if we are looking directly at a player
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	// Note don't id dead bodies
	if (tr.fraction < 1 && tr.ent &&
	    tr.ent->client && !tr.ent->deadflag) {
		ent->client->ps.stats[STAT_PLAYER_ID] = 
			CS_PLAYERNAMES + (ent - g_edicts - 1);
			//CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	// find the player in our line-of-sight who 
	// is nearest our facing vector
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	who = g_edicts + 1;
	for (i = 1; i <= maxclients->value; i++) {
		// Note don't id dead bodies or observers
		if (!who->inuse || IsObserver(who) || who->deadflag)
		{
			who = who + 1;
			continue;
		}
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
		who = who + 1;
	}
	if (bd > 0.82)
		ent->client->ps.stats[STAT_PLAYER_ID] = 
			CS_PLAYERNAMES + (best - g_edicts - 1);
			//CS_PLAYERSKINS + (best - g_edicts - 1);
}
