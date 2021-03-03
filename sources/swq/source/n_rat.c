/*
==============================================================================

Rat

==============================================================================
*/

#include "g_local.h"
#include "n_rat.h"

void rat_walk (edict_t *self);
void rat_run (edict_t *self);
void rat_stand (edict_t *self);
void rat_eat_check (edict_t *self);
void rat_head_up (edict_t *self);
void rat_head_down (edict_t *self);
void rat_move_turn (edict_t *self);
void rat_turn (edict_t *self);
void rat_turn_end (edict_t *self);

//
// STAND
//

mframe_t rat_frames_eat [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL
};
mmove_t rat_move_eat = {FRAME_eat3, FRAME_eat13, rat_frames_eat, rat_eat_check};

void rat_eat_check (edict_t *self)
{
	float n;

	n = random();

	if(n >= 0.6)
		self->monsterinfo.nextframe = FRAME_eat1;
//		self->monsterinfo.currentmove = &rat_move_eat;
	else
		rat_head_up(self);
}

mframe_t rat_frames_headup [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t rat_move_headup = {FRAME_headup1, FRAME_headup3, rat_frames_headup, rat_stand};

mframe_t rat_frames_headdn [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, rat_walk
};
mmove_t rat_move_headdn = {FRAME_headdn1, FRAME_headdn2, rat_frames_headdn, rat_walk};

void rat_head_up (edict_t *self)
{
	self->monsterinfo.currentmove = &rat_move_headup;
}
void rat_head_down (edict_t *self)
{
	self->monsterinfo.currentmove = &rat_move_headdn;
}


//
// RAT AI
//
int rat_ai_check_move_forward (edict_t *self)
{
	trace_t tr;
	vec3_t forward, end;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 64, forward, end);

	tr = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

	if(tr.fraction != 1)
		return 0;

	return 1;
}

void rat_ai_check_stop (edict_t *self)
{
	if(random() > 0.6)
//		rat_head_up(self);
		self->monsterinfo.currentmove = &rat_move_eat;
}

void rat_ai_check_move (edict_t *self)
{
	float n;

	n = random();

	if(n >= 0.5)
		rat_head_down (self);
}

//
// STAND
//

mframe_t rat_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, rat_ai_check_move
};
mmove_t rat_move_stand = {FRAME_idle1, FRAME_idle30, rat_frames_stand, rat_stand};

void rat_stand (edict_t *self)
{
//	rat_ai_check_move(self);
	self->monsterinfo.currentmove = &rat_move_stand;
//	rat_walk(self);
}

//
// WALK/RUN
//

mframe_t rat_frames_turn [] =
{
	ai_move, 0,  rat_turn,
	ai_move, 2,  rat_turn,
	ai_move, 3,  rat_turn,
	ai_move, 1,  rat_turn,
	ai_move, 0,  rat_turn_end
};
mmove_t rat_move_turn1 = {FRAME_run1, FRAME_run5, rat_frames_turn, rat_walk};

mframe_t rat_frames_run1 [] =
{
	ai_move, 0,  NULL,
	ai_move, 3,  NULL,
	ai_move, 9,  NULL,
	ai_move, 7,  NULL,
	ai_move, 4,  rat_ai_check_stop
};
mmove_t rat_move_run1 = {FRAME_run1, FRAME_run5, rat_frames_run1, rat_walk};

void rat_turn_end (edict_t *self)
{
	self->count = 0;
}

void rat_turn (edict_t *self)
{
	float turn;

	turn = random()*5;

	if(self->count == 1)
	{
		self->s.angles[1] += 17.5+turn;
	}
	else
	{
		self->s.angles[1] -= 17.5+turn;
	}
}

void rat_move_turn (edict_t *self)
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

	self->monsterinfo.currentmove = &rat_move_turn1;
}

void rat_walk (edict_t *self)
{
	float n;

	n = random();

	if(rat_ai_check_move_forward (self) == 1 && (n < 0.95))
		self->monsterinfo.currentmove = &rat_move_run1;
	else
		rat_move_turn(self);
}

void rat_run (edict_t *self)
{
	float n;

	n = random();

	if(rat_ai_check_move_forward (self) == 1 && (n < 0.95))
		self->monsterinfo.currentmove = &rat_move_run1;
	else
		self->monsterinfo.currentmove = &rat_move_turn1;
}

//
// PAIN
//

mframe_t rat_frames_pain [] =
{
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL
};
mmove_t rat_move_pain = {FRAME_pain1, FRAME_pain4, rat_frames_pain, rat_stand};

void rat_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

//	gi.sound (self, CHAN_VOICE, sound_wowbeep, 1, ATTN_NORM, 0);

	self->monsterinfo.currentmove = &rat_move_pain;
}

//
// DEATH
//

void rat_dead (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t rat_frames_death2 [] =
{
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
mmove_t rat_move_death2 = {FRAME_death201, FRAME_death209, rat_frames_death2, rat_dead};

mframe_t rat_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t rat_move_death1 = {FRAME_death101, FRAME_death108, rat_frames_death1, rat_dead};

void rat_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float n;

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

	n = random();
	if (n >= 0.5)
		self->monsterinfo.currentmove = &rat_move_death1;
	else
		self->monsterinfo.currentmove = &rat_move_death2;
}

//
// SPAWN
//

void SP_npc_rat (edict_t *self)
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

	self->s.modelindex = gi.modelindex ("models/npcs/rat/rat.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -8, -8, -4);
	VectorSet (self->maxs, 8, 8, 4);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 12;

	self->mass = 50;

	self->pain = rat_pain;
	self->die = rat_die;

	self->monsterinfo.stand = rat_stand;
	self->monsterinfo.walk = rat_walk;
	self->monsterinfo.run = rat_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);

	self->monsterinfo.run (self);
}

