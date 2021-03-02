#include "g_local.h"

static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}
edict_t *Matrix_FindTarget (edict_t *self, int range)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;


	ent = NULL;
	ignore = self;
	while ((ent = findradius(ent, self->s.origin, range)) != NULL)
	{
		if (ent == self)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		tr = gi.trace (self->s.origin, NULL, NULL, ent->s.origin, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
			while(1);
			{
				if (!tr.ent)
				break;

			// hurt it if we can
			//	if ((tr.ent->takedamage) && (tr.ent != self->owner))
				return ent;

				if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
					break;
			}

	}
	
	return NULL;

}

void Matrix_SniperZoom (edict_t *ent)
{


	edict_t	*target = NULL;
	edict_t *blip = NULL;
	vec3_t	targetdir, blipdir, dir, AngleToTarget;
	vec_t	speed;
	int		dist, i;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}
		
	if (target != NULL)
	{
	VectorNormalize(targetdir);
	vectoangles (targetdir, AngleToTarget);
	
	for(i=0;i<2;i++)
	{
		dir[i] = fabs(ent->client->v_angle[i] -  AngleToTarget[i]);
		if(dir[i]>180)
		dir[i] = 360 - dir[i];
	}
	dir[2] = 0;
	

//	gi.cprintf (ent, PRINT_HIGH, "YAW = %i   ",dir[YAW]);
	dist = sqrt((dir[0]*dir[0]) + (dir[1] * dir [1]));
	
	dist = (dist+5) *2;
	if(dist < 90)
		ent->client->ps.fov = dist;
	else
		ent->client->ps.fov = 90;


	}

}

void MatrixOlympics (edict_t *ent)
{
	int i, sum;
	qboolean already;
	
	sum=0;
	already = false;

	if(ent->deadflag)
		return;

	for (i=0;i<2;i++)
	{
		sum+= ent->velocity[i]*ent->velocity[i];
	}
	sum = sqrt(sum);
if (sum>490 && !ent->MatrixJumping)
	{
	//every 5th of a second
	i =(level.time * 10);
	if (i % 2 != 0)
	SpawnShadow (ent);
	already = true;
	}
	//gi.cprintf (ent, PRINT_HIGH, "XY = %i   ",sum);
//  XY speed

	/*sum=0;
	for (i=0;i<3;i++)
	{
		sum+= ent->velocity[i]*ent->velocity[i];
	}
	sum = sqrt(sum);*/
	//gi.cprintf (ent, PRINT_HIGH, "XYZ = %i   ",sum);
// Actual Speed

	sum = ent->velocity[2];
	if(sum > 290 && !already && !ent->MatrixJumping)
		ent->MatrixJumping = true;
	//gi.cprintf (ent, PRINT_HIGH, "Z = %i\n",sum);
// Z speed
}


void MatrixSniperHud(edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t *blip = NULL;
	vec3_t	targetdir, blipdir, dir, AngleToTarget, forward, blag, offset, right, start;
	vec_t	speed;
	trace_t	tr;
	int		dist, i;
	
	
	while ((blip = findradius(blip, ent->s.origin, 8000)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;

		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}
		
	if (target != NULL)
	{
	VectorNormalize(targetdir);
	vectoangles (targetdir, AngleToTarget);
	
		
		
			if(fabs(AngleToTarget[PITCH]) > fabs(AngleToTarget[YAW]))
			{
			if(ent->client->v_angle[PITCH] > AngleToTarget[PITCH])
			ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schdown");
			if(ent->client->v_angle[PITCH] < AngleToTarget[PITCH])
			ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schup");
			}
			else
			{
			if(ent->client->v_angle[YAW] < AngleToTarget[YAW])
			ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schleft");
			if(ent->client->v_angle[YAW] > AngleToTarget[YAW])
			ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schright");
			}
		
		AngleVectors (ent->client->v_angle, forward, NULL, NULL);
		VectorMA(ent->s.origin, 1500, forward, blag);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		tr = gi.trace(start, vec3_origin, vec3_origin, blag, ent, MASK_MONSTERSOLID);
		
		if (tr.fraction <1 && tr.ent)
			if(tr.ent->takedamage)
				ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schhit");
		
	}
	else 
		ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("blank");
	
	

	if(ent->deadflag)
		ent->client->ps.stats[STAT_SNIPER_OK] = 0;
	else	
		ent->client->ps.stats[STAT_SNIPER_OK] = 1;
	
}