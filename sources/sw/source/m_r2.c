/*
==============================================================================

r2 unit

==============================================================================
*/

#include "g_local.h"
#include "m_r2.h"


static int	sound_confused;
static int	sound_happybeep;
static int	sound_happy;
static int	sound_sad1;
static int	sound_sad2;
static int	sound_walk;
static int	sound_danger;
static int	sound_die;
static int	sound_warn;
static int	sound_wowbeep;

// STAND

void r2_stand (edict_t *self);

mframe_t r2_frames_stand [] =
{
	ai_stand, 0, NULL,
};
mmove_t r2_move_stand = {FRAME_stand1, FRAME_stand1, r2_frames_stand, r2_stand};

void r2_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &r2_move_stand;
}

//
// WALK
//

mframe_t r2_frames_walk1 [] =
{
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL
};
mmove_t r2_move_walk1 = {FRAME_walk1b, FRAME_walk2b, r2_frames_walk1, NULL};

void r2_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &r2_move_walk1;
}

//
// RUN
//

mframe_t r2_frames_run [] =
{
	ai_run, 4,  NULL,
	ai_run, 4,  NULL
};
mmove_t r2_move_run = {FRAME_walk1b, FRAME_walk2b, r2_frames_run, NULL};

void r2_run (edict_t *self)
{
	self->monsterinfo.currentmove = &r2_move_run;
}

//
// PAIN
//

mframe_t r2_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL
};
mmove_t r2_move_pain1 = {FRAME_protest1, FRAME_protest12, r2_frames_pain1, r2_run};

void r2_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

	gi.sound (self, CHAN_VOICE, sound_wowbeep, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &r2_move_pain1;
}

//
// SIGHT
//

void r2_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_danger, 1, ATTN_NORM, 0);
}

//
// DEATH
//

void r2_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void r2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;

	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);

	self->s.frame = FRAME_walk1b;
	r2_dead(self);
}


//
// SPAWN
//

void SP_monster_r2 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_confused = gi.soundindex ("r2-unit/confusion.wav");
	sound_happybeep = gi.soundindex ("r2-unit/happybeep.wav");
	sound_happy = gi.soundindex ("r2-unit/r2happy1.wav");
	sound_sad1 = gi.soundindex ("r2-unit/r2d21.wav");
	sound_sad2 = gi.soundindex ("r2-unit/wow-oh!.wav");
	sound_walk = gi.soundindex ("r2-unit/r2d2walk.wav");
	sound_danger = gi.soundindex ("r2-unit/r2dangr.wav");
	sound_die = gi.soundindex ("r2-unit/r2hit.wav");
	sound_warn = gi.soundindex ("r2-unit/r2warn.wav");
	sound_wowbeep = gi.soundindex ("r2-unit/wow-beep.wav");

	self->s.skinnum = 0;
	self->health = 30+(random() * 15);
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/monsters/r2unit/tris.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 10;

	self->mass = 50;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	self->pain = r2_pain;
	self->die = r2_die;

	self->monsterinfo.stand = r2_stand;
	self->monsterinfo.walk = r2_walk;
	self->monsterinfo.run = r2_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = r2_sight;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

