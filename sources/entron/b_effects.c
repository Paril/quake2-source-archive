/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_effects.c
  Description: Effects code.

\**********************************************************/

#include "g_local.h"

void SpawnBlood(vec3_t point, float damage, vec3_t dir)
{
	edict_t *blood[4];
	int i;
	for (i = 0; i < 4; i++)
		{
		blood[i] = G_Spawn();
		VectorCopy(point, blood[i]->s.origin);
		VectorScale(dir, 20 * damage, blood[i]->velocity);
		//blood[i]->s.solid = SOLID_TRIGGER;
		blood[i]->movetype = MOVETYPE_TOSS;
		blood[i]->s.modelindex = gi.modelindex("sprites/b_blood_1.sp2");
		blood[i]->nextthink = level.time + 4;
		blood[i]->think = G_FreeEdict;
		gi.linkentity (blood[i]);
		}
}

void PoolShrink(edict_t *self)
{
	self->s.frame++;
	if (self->s.frame == 11)
		G_FreeEdict(self);
	else
		{	
		self->nextthink = level.time + .2;
		gi.linkentity(self);
		}
}

void PoolExpand(edict_t *self)
{
	self->s.frame--;
	if (!self->s.frame)
		{
		self->think = PoolShrink;
		self->nextthink = level.time + 10;
		}
	else
		self->nextthink = level.time + .2;
	gi.linkentity(self);
}


void SpawnPool(vec3_t point)
{
	edict_t *blood;
	trace_t trace;
	blood = G_Spawn();
	blood->s.origin[0] = point[0];
	blood->s.origin[1] = point[1];
	blood->s.origin[2] = point[2] - 8192;
	trace = gi.trace(point, NULL, NULL, blood->s.origin, NULL, MASK_SOLID);
	blood->s.origin[0] = trace.endpos[0];
	blood->s.origin[1] = trace.endpos[1];
	blood->s.origin[2] = trace.endpos[2];
	blood->s.modelindex = gi.modelindex("models/world/blood/tris.md2");
	blood->nextthink = level.time + 1;
	blood->s.frame = 10;
	blood->think = PoolExpand;
	gi.linkentity (blood);
}