/*
==============================================================================

Gonk!

==============================================================================
*/

#include "g_local.h"
#include "n_gonk.h"

// STAND

void gonk_stand (edict_t *self);
void gonk_walk (edict_t *self);
void gonk_run (edict_t *self);
void gonk_turn_end (edict_t *self);
void gonk_turn (edict_t *self);
void gonk_move_turn (edict_t *self);

void gonk_gonk (edict_t *self)
{
	float n;

	n = random();

	if(n > 0.8)
	{
//		gi.dprintf("gonk\n");
	}
}

void gonk_check_walk (edict_t *self)
{
	float n;

	n = random();

	if(n > 0.9)
	{
		gonk_walk(self);
	}
}

int gonk_ai_check_move_forward (edict_t *self)
{
	trace_t tr;
	vec3_t forward, end;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 24, forward, end);

	tr = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

	if(tr.fraction != 1)
		return 0;

	return 1;
}

mframe_t gonk_frames_stand [] =
{
	ai_stand, 0, gonk_gonk,
	ai_stand, 0, NULL,
	ai_stand, 0, gonk_check_walk
};
mmove_t gonk_move_stand = {FRAME_stand1, FRAME_stand3, gonk_frames_stand, NULL};

void gonk_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &gonk_move_stand;
}

//
//WALK/RUN
//

mframe_t gonk_frames_turn [] =
{
	ai_move, 0.1, gonk_turn,
	ai_move, 0.4, gonk_turn,
	ai_move, 0.3, gonk_turn,
	ai_move, 0.4, gonk_turn,
	ai_move, 0.3, gonk_turn,
	ai_move, 0.3, gonk_turn,
	ai_move, 0,   gonk_turn,
	ai_move, 0.4, gonk_turn,
	ai_move, 0.3, gonk_turn,
	ai_move, 0.4, gonk_turn,

	ai_move, 0.3, gonk_turn,
	ai_move, 0.4, gonk_turn
};
mmove_t gonk_move_turn1 = {FRAME_walk1, FRAME_walk12, gonk_frames_turn, gonk_walk};

mframe_t gonk_frames_walk[] =
{
	ai_move, 0.1, NULL,
	ai_move, 0.4, NULL,
	ai_move, 0.3, NULL,
	ai_move, 0.4, NULL,
	ai_move, 0.3, NULL,
	ai_move, 0.3, NULL,
	ai_move, 0,   NULL,
	ai_move, 0.4, NULL,
	ai_move, 0.3, NULL,
	ai_move, 0.4, NULL,

	ai_move, 0.3, NULL,
	ai_move, 0.4, NULL
};
mmove_t gonk_move_walk = {FRAME_walk1, FRAME_walk12, gonk_frames_walk, gonk_walk};

void gonk_turn_end (edict_t *self)
{
	self->count = 0;
}

void gonk_turn (edict_t *self)
{
	float turn;

	turn = random()*5;

	if(self->count == 1)
	{
		self->s.angles[1] += 2+turn;
	}
	else
	{
		self->s.angles[1] -= 2+turn;
	}
}

void gonk_move_turn (edict_t *self)
{
	float n;

	n = random();

	if(n > 0.5)
	{
		self->count = 1;
	}
	else
	{
		self->count = 2;
	}

	self->monsterinfo.currentmove = &gonk_move_turn1;
}


void gonk_walk (edict_t *self)
{
	float n;

	n = random();

	if(gonk_ai_check_move_forward (self) == 1 && (n < 0.95))
		self->monsterinfo.currentmove = &gonk_move_walk;
	else
		gonk_move_turn(self);
}

void gonk_run (edict_t *self)
{
	float n;

	n = random();

	if(gonk_ai_check_move_forward (self) == 1 && (n < 0.95))
		self->monsterinfo.currentmove = &gonk_move_walk;
	else
		gonk_move_turn(self);
}

//
// PAIN
//

void gonk_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

//	gi.sound (self, CHAN_VOICE, sound_wowbeep, 1, ATTN_NORM, 0);
}

//
// DEATH
//

void gonk_dead (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t gonk_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t gonk_move_death = {FRAME_die1, FRAME_die10, gonk_frames_death1, gonk_dead};

void gonk_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

/*	n = random();
	if (n >= 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);
*/
	self->monsterinfo.currentmove = &gonk_move_death;
}

//
// SPAWN
//

void SP_npc_gonk (edict_t *self)
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

	self->s.modelindex = gi.modelindex ("models/npcs/gonk/gonk.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -8, -8, -24);
	VectorSet (self->maxs, 8, 8, 0);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 12;

	self->mass = 50;

	self->pain = gonk_pain;
	self->die = gonk_die;

	self->monsterinfo.stand = gonk_stand;
	self->monsterinfo.walk = gonk_walk;
	self->monsterinfo.run = gonk_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

