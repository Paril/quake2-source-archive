/*
==============================================================================

Raptor type thing

==============================================================================
*/

#include "g_local.h"
#include "n_raptor.h"

// STAND

void raptor_stand (edict_t *self);
void raptor_fly (edict_t *self);
void raptor_move_turn (edict_t *self);
void raptor_turn (edict_t *self);
void raptor_turn_end (edict_t *self);

int raptor_ai_check_move_forward (edict_t *self)
{
	trace_t tr;
	vec3_t forward, end;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 128, forward, end);

	tr = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

	if(tr.fraction != 1)
		return 0;

	return 1;
}

mframe_t raptor_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};

mmove_t raptor_move_stand = {FRAME_fly1, FRAME_fly12, raptor_frames_stand, raptor_fly};

void raptor_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &raptor_move_stand;
}

//
// FLY
//

mframe_t raptor_frames_turn [] =
{
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn,

	ai_move, 9,  raptor_turn,
	ai_move, 9,  raptor_turn_end
};
mmove_t raptor_move_turn1 = {FRAME_fly1, FRAME_fly12, raptor_frames_turn, raptor_fly};

mframe_t raptor_frames_fly [] =
{
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,
	ai_stand, 9, NULL,

	ai_stand, 9, NULL,
	ai_stand, 9, NULL
};
mmove_t raptor_move_fly = {FRAME_fly1, FRAME_fly12, raptor_frames_fly, raptor_fly};

void raptor_turn_end (edict_t *self)
{
	self->count = 0;
	self->s.angles[2] = 0;
}

void raptor_turn (edict_t *self)
{
	float turn;

	turn = random()*5;

	if(self->count == 1)
	{
		self->s.angles[1] += 10+turn;
		self->s.angles[2] = 30;
	}
	else
	{
		self->s.angles[1] -= 10+turn;
		self->s.angles[2] = -30;
	}
}

void raptor_move_turn (edict_t *self)
{
	float n;

	n = random();

	if(n > 0.5)
	{
		self->count = 1;
		self->s.angles[2] = 15;
	}
	else
	{
		self->count = 2;
		self->s.angles[2] = -15;
	}

	self->monsterinfo.currentmove = &raptor_move_turn1;
}

int sparrow_ai_check_move_forward (edict_t *self);

void raptor_fly (edict_t *self)
{
	float n;

	n = random();

	if(sparrow_ai_check_move_forward (self) == 1 && (n < 0.95))
		self->monsterinfo.currentmove = &raptor_move_fly;
	else
		self->monsterinfo.currentmove = &raptor_move_turn1;
}

//
// PAIN
//

void raptor_pain (edict_t *self, edict_t *other, float kick, int damage)
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

void raptor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
}

//
// SPAWN
//

void SP_npc_raptor (edict_t *self)
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

	self->s.modelindex = gi.modelindex ("models/npcs/sparrow/sparrow.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -8, -8, -4);
	VectorSet (self->maxs, 8, 8, 4);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_NOT;
	self->yaw_speed = 12;

	self->mass = 50;

	self->pain = raptor_pain;
	self->die = raptor_die;

	self->monsterinfo.stand = raptor_stand;
	self->monsterinfo.walk = raptor_fly;
	self->monsterinfo.run = raptor_fly;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	flymonster_start (self);
}

