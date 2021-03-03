/*
==============================================================================

bird

==============================================================================
*/

#include "g_local.h"
#include "m_bird.h"

// STAND

void bird_stand (edict_t *self);
void path_near (edict_t *self);

mframe_t bird_frames_stand [] =
{
	ai_stand, 1, NULL,
	ai_stand, 1, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, -1, NULL,
	ai_stand, -1, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};

mmove_t bird_move_stand = {FRAME_fly1, FRAME_fly8, bird_frames_stand, bird_stand};

void bird_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &bird_move_stand;
}

//
// WALK
//
void bird_walk (edict_t *self);

void bird_path_near (edict_t *self)
{
	vec3_t		v;
	edict_t		*next;
	float			dist;
	edict_t		*goal;

	VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
	dist = VectorLength(v);
	if(dist > 128)
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

void bird_ai_walk (edict_t *self, float dist)
{
	float cur_yaw;
	float ideal_yaw;
	float diff;
	vec3_t v;
	vec3_t v2;
	vec3_t new_org;

	M_ChangeYaw (self);
	path_near(self);
	ideal_yaw = self->ideal_yaw;
	cur_yaw = anglemod(self->s.angles[YAW]);
	diff = ideal_yaw-cur_yaw;
	if(diff < 5 && diff > -5)
		ai_walk(self, dist);
	else
	{
//		ai_walk(self, 15);
		VectorSubtract(self->last_move_ent->s.origin, self->s.origin, v);
		VectorNormalize2 (v, v2);
		VectorMA(self->s.origin, 4, v2, new_org);
		new_org[2] = self->s.origin[2];
		VectorCopy(new_org, self->s.origin);
	}

//	gi.dprintf("%f\n", diff);
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
/*
mframe_t bird_frames_walk [] =
{
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL
};
mmove_t bird_move_walk = {FRAME_fly1, FRAME_fly8, bird_frames_walk, NULL};
*/

mframe_t bird_frames_walk [] =
{
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL,
	bird_ai_walk, 4,  NULL
};
mmove_t bird_move_walk = {FRAME_fly1, FRAME_fly8, bird_frames_walk, bird_walk};

void bird_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &bird_move_walk;
}

//
// RUN
//

void bird_run (edict_t *self);

mframe_t bird_frames_run [] =
{
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL,
	ai_run, 6, NULL
};
mmove_t bird_move_run = {FRAME_fly1, FRAME_fly8, bird_frames_run, NULL};

void bird_run (edict_t *self)
{
	self->monsterinfo.currentmove = &bird_move_run;
}

//
// PAIN
//

void bird_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;
}


//
// ATTACK
//

void bird_attack(edict_t *self)
{
}


//
// SIGHT
//

void bird_sight(edict_t *self, edict_t *other)
{
}

//
// DEATH
//

void bird_dead (edict_t *self);

void bird_dead2 (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t bird_frames_death1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t bird_move_death1 = {FRAME_fall1, FRAME_fall7, bird_frames_death1, bird_dead};

mframe_t bird_frames_death2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t bird_move_death2 = {FRAME_die1, FRAME_die8, bird_frames_death2, bird_dead2};

void bird_deadthink (edict_t *self)
{
	if (!self->groundentity && level.time < self->timestamp)
	{
		self->nextthink = level.time + FRAMETIME;
		return;
	}
//	BecomeExplosion1(self);
	self->monsterinfo.currentmove = &bird_move_death2;
}

void bird_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -4);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_TOSS;
	self->think = bird_deadthink;
	self->nextthink = level.time + FRAMETIME;
	self->timestamp = level.time + 15;
	gi.linkentity (self);
}

void bird_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &bird_move_death1;
}


//
// SPAWN
//

void SP_monster_bird (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.skinnum = 0;
	self->health = 5;
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/monsters/crittera/tris.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 12;

	self->mass = 50;

	self->pain = bird_pain;
	self->die = bird_die;

	self->monsterinfo.stand = bird_stand;
	self->monsterinfo.walk = bird_walk;
	self->monsterinfo.run = bird_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = bird_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = bird_sight;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

//	walkmonster_start (self);
	flymonster_start (self);
}

