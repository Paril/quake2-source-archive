/*
==============================================================================

Gonk!

==============================================================================
*/

#include "g_local.h"
#include "n_mse.h"

// STAND

void MSE_stand (edict_t *self);

mframe_t MSE_frames_stand [] =
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
	ai_stand, 0, NULL
};

mmove_t MSE_move_stand = {FRAME_walk1, FRAME_walk12, MSE_frames_stand, MSE_stand};

void MSE_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &MSE_move_stand;
}

//
// PAIN
//

void MSE_pain (edict_t *self, edict_t *other, float kick, int damage)
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

void MSE_dead (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t MSE_frames_death1 [] =
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
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t MSE_move_death = {FRAME_sideshot1, FRAME_sideshot11, MSE_frames_death1, MSE_dead};

void MSE_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->monsterinfo.currentmove = &MSE_move_death;
}

//
// SPAWN
//

void SP_npc_MSE (edict_t *self)
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

	self->s.modelindex = gi.modelindex ("models/npcs/mousedroid/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/npcs/mousedroid/fwheels.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -8, -8, 0);
	VectorSet (self->maxs, 8, 8, 16);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 12;

	self->mass = 50;

	self->pain = MSE_pain;
	self->die = MSE_die;

	self->monsterinfo.stand = MSE_stand;
	self->monsterinfo.walk = MSE_stand;
	self->monsterinfo.run = MSE_stand;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

