/*
==============================================================================

Vehicles

==============================================================================
*/

#include "g_local.h"
#include <math.h>

// STAND

void skimmer_stand (edict_t *self);
void skimmer_walk (edict_t *self);
void skimmer_run (edict_t *self);

mframe_t skimmer_frames_stand [] =
{
	ai_stand, 0, NULL,
};
mmove_t skimmer_move_stand = {0, 0, skimmer_frames_stand, skimmer_stand};

void skimmer_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &skimmer_move_stand;
//	self->s.frame = 0;
}

//
// WALK
//

void path_near (edict_t *self)
{
	vec3_t		v;
	edict_t		*next;
	float			dist;
	edict_t		*goal;

	VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
	dist = VectorLength(v);
	if(dist > 194)
		return;

	goal = self->movetarget;
	self->last_move_ent = self->movetarget;

	if (goal->pathtarget)
	{
		char *savetarget;

		savetarget = goal->target;
		goal->target = goal->pathtarget;
		G_UseTargets (goal, self);
		goal->target = savetarget;
	}

	if (goal->target)
		next = G_PickTarget(goal->target);
	else
		next = NULL;

	if ((next) && (next->spawnflags & 1))
	{
		VectorCopy (next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= self->mins[2];
		VectorCopy (v, self->s.origin);
		next = G_PickTarget(next->target);
	}

	self->goalentity = self->movetarget = next;

	VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw (v);

	if (goal->wait)
	{
		self->monsterinfo.pausetime = level.time + goal->wait;
		self->monsterinfo.stand (self);
		return;
	}

	if (!self->movetarget)
	{
		self->monsterinfo.pausetime = level.time + 100000000;
		self->monsterinfo.stand (self);
	}
	else
	{
		VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
	}
}

void skimmer_ai_walk (edict_t *self, float dist)
{
	float cur_yaw;
	float ideal_yaw;
	float diff;
	vec3_t v;
	vec3_t v2;
	vec3_t new_org;

	path_near(self);
	VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	ideal_yaw = self->ideal_yaw;
	cur_yaw = anglemod(self->s.angles[YAW]);
	diff = ideal_yaw-cur_yaw;
	M_ChangeYaw (self);
	if(diff < 5 && diff > -5)
	{
		ai_walk(self, dist);
	}
	else
	{
//		ai_walk(self, 35);
		VectorSubtract(self->last_move_ent->s.origin, self->s.origin, v);
		VectorNormalize2 (v, v2);
		VectorMA(self->s.origin, 20, v2, new_org);
		new_org[2] = self->s.origin[2];
		VectorCopy(new_org, self->s.origin);
	}

	if(diff > 5)
	{
		self->s.angles[ROLL] -= 90/diff;
	}
	else if(diff < -5)
	{
		self->s.angles[ROLL] -= 90/diff;
	}
	else
	{
		if(self->s.angles[ROLL] > 7)
			self->s.angles[ROLL] -= 7;
		else if (self->s.angles[ROLL] < -7)
			self->s.angles[ROLL] += 7;
		else
			self->s.angles[ROLL] -= self->s.angles[ROLL];
	}
}

void Check_Collide (edict_t *self)
{
	vec3_t	start, end;
	vec3_t	forward;
	trace_t	tr;
	vec3_t normal;

	AngleVectors (self->s.angles, forward, NULL, NULL);

	VectorMA(self->s.origin, 32, forward, end);
	VectorMA(self->s.origin, 16, forward, start);

	tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_ALL);

	if(tr.fraction != 1)
	{
		if(tr.ent)
		{
			VectorNormalize2(forward, normal);
			T_Damage (tr.ent, self, self, forward, tr.endpos, normal, 30, 1000, 1, 1);
		}
	}
}

mframe_t skimmer_frames_walk [] =
{
	skimmer_ai_walk, 50,  Check_Collide,
	skimmer_ai_walk, 50,  Check_Collide,
	skimmer_ai_walk, 50,  Check_Collide,
	skimmer_ai_walk, 50,  Check_Collide,
	skimmer_ai_walk, 50,  Check_Collide
};
mmove_t skimmer_move_walk = {0, 4, skimmer_frames_walk, NULL};

void skimmer_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &skimmer_move_walk;
}

//
// RUN
//

void skimmer_run (edict_t *self);

mframe_t skimmer_frames_run [] =
{
	skimmer_ai_walk, 50, NULL,
	skimmer_ai_walk, 50, NULL,
	skimmer_ai_walk, 50, NULL,
	skimmer_ai_walk, 50, NULL,
	skimmer_ai_walk, 50, NULL
};
mmove_t skimmer_move_run = {0, 4, skimmer_frames_run, NULL};

void skimmer_run (edict_t *self)
{
	self->monsterinfo.currentmove = &skimmer_move_run;
}

//
// PAIN
//

void skimmer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}


//
// ATTACK
//

void skimmer_attack(edict_t *self)
{
}

//
// DEATH
//

void skimmer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}

//
// SPAWN
//

void SP_misc_skimmer (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.skinnum = 0;
	self->health = 90;
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/ships/car/tris.md2");
	self->monsterinfo.scale = 1;
	VectorSet (self->mins, -24, -24, -16);
	VectorSet (self->maxs, 24, 24, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 10;
	self->takedamage = DAMAGE_AIM;

	self->mass = 50;

	self->pain = skimmer_pain;
	self->die = skimmer_die;

	self->monsterinfo.stand = skimmer_stand;
	self->monsterinfo.walk = skimmer_walk;
	self->monsterinfo.run = skimmer_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	flymonster_start (self);
*/
}

