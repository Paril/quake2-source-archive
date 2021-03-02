#include <math.h>
#include "g_local.h"

void predictTargPos (edict_t *self, vec3_t source, vec3_t targPos, vec3_t targVel, float speed, qboolean feet)
{
	float		eta, dist, height;
	vec3_t		dir, tPos, finalPos, enemyvel;
	trace_t		tr;
	int			content_mask = MASK_SHOT | MASK_WATER;

	// targPos = enemy's position
	// tPos = calculated enemy's position
	// source == self->s.origin

	if (!visible(self, self->enemy))
	{
		// enemy is not visible
		// so use location of last known sighting
		VectorCopy (self->monsterinfo.last_sighting, targPos);

		// clear target velocity
		VectorClear (targVel);
	}

	VectorSubtract (targPos, source, dir);	
	eta = (VectorLength (dir) / speed);

	VectorCopy (targPos, tPos);
	VectorCopy (targVel, enemyvel);
	enemyvel[2] = 0;

	VectorMA (tPos, eta, enemyvel, finalPos);
	
	tPos[2] -= 20;
	finalPos[2] -= 20;

	//gi.bprintf (PRINT_MEDIUM, "Aiming\n");
	tr = gi.trace (tPos, NULL, NULL, finalPos, self->enemy, content_mask);
	if (tr.fraction < 1.0)
	{
		//gi.bprintf (PRINT_MEDIUM, "Hit wall %f\n", tr.fraction);

		finalPos[2] += 20;
		tPos[2] += 20;

		VectorSubtract (finalPos, tPos, dir);
		dist = VectorLength (dir);
		dist *= tr.fraction;

		VectorNormalize (dir);
	
		VectorMA (targPos, dist, dir, targPos);
		return;				
	}	
	
	VectorSubtract (targPos, source, dir);
	dist = VectorLength (dir);

	//if ((eta > 0.45) && (eta < 1.3) && (skill->value > 1) )
	if ( (dist > 225) && (dist < 650) && (skill->value > 1) )
	{
		eta = eta * 0.20;
	}

	height = targPos[2] - (source[2]);// + 24);
	//gi.bprintf (PRINT_MEDIUM, "dsfheight difference = %f, tz = %f, sz = %f\n", height, targPos[2], source[2]);

	if ( (height < 5) && (self->enemy->groundentity) && (skill->value > 1) && (feet) )
	{

		//gi.bprintf (PRINT_MEDIUM, "Aim at feet\n");
		VectorCopy (targPos, tPos);
		//if (dist < 64)
		//	tPos[2] -= (self->enemy->viewheight + 26);
		//else
			tPos[2] -= (self->enemy->viewheight + 26);
	}
	else
	{
		//gi.bprintf (PRINT_MEDIUM, "Aim at head\n");
		VectorCopy (targPos, tPos);	
	}

	VectorMA (tPos, eta, enemyvel, targPos);
	
}

qboolean visible_from_weapon (edict_t *ent, vec3_t source)
{

	vec3_t	spot1;
	trace_t	trace;

	VectorCopy (ent->enemy->s.origin, spot1);
	spot1[2] += ent->viewheight;

	//return true;

	trace = gi.trace (source, vec3_origin, vec3_origin, spot1, ent, MASK_OPAQUE);
	
	if (trace.fraction == 1.0)
		return true;

	if (trace.ent->svflags & SVF_MONSTER)
		return true;

	if (trace.ent->client)
		return true;

	return false;


}
