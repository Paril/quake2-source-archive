/*
==============================================================================

JAWA

==============================================================================
*/

#include "g_local.h"
#include "m_jawa.h"


static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_attack4;
static int	sound_cry1;
static int	sound_cry2;
static int	sound_scare;

// STAND

void jawa_stand (edict_t *self);

mframe_t jawa_frames_stand2 [] =
{
	ai_stand, 0, NULL,
};
mmove_t jawa_move_stand2 = {FRAME_stand1, FRAME_stand1, jawa_frames_stand2, jawa_stand};

mframe_t jawa_frames_stand1 [] =
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
	ai_stand, 0, NULL
};
mmove_t jawa_move_stand1 = {FRAME_stand1, FRAME_stand15, jawa_frames_stand1, jawa_stand};

void jawa_stand (edict_t *self)
{
	if(random() > 0.01)
		self->monsterinfo.currentmove = &jawa_move_stand2;
	else
		self->monsterinfo.currentmove = &jawa_move_stand1;
}

//
// WALK
//

mframe_t jawa_frames_walk1 [] =
{
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL
};
mmove_t jawa_move_walk1 = {FRAME_rungun1, FRAME_rungun6, jawa_frames_walk1, NULL};

void jawa_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &jawa_move_walk1;
}

//
// RUN
//

void jawa_run (edict_t *self);

mframe_t jawa_frames_run1 [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t jawa_move_run1 = {FRAME_rungun1, FRAME_rungun6, jawa_frames_run1, NULL};

void jawa_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &jawa_move_stand1;
	else
	{
		self->monsterinfo.currentmove = &jawa_move_run1;
	}
}

//
// PAIN
//

mframe_t jawa_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL
};
mmove_t jawa_move_pain1 = {FRAME_paingun1, FRAME_paingun4, jawa_frames_pain1, jawa_run};

mframe_t jawa_frames_pain2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t jawa_move_pain2 = {FRAME_paingunb1, FRAME_paingunb5, jawa_frames_pain2, jawa_run};

void jawa_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

	if(random() >= 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

		if(random() >= 0.5)
			self->monsterinfo.currentmove = &jawa_move_pain1;
		else
			self->monsterinfo.currentmove = &jawa_move_pain2;
}


//
// ATTACK
//

void jawa_fire (edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t 	target;
	vec3_t 	vel;
	vec3_t	tv;
	float 	dist;
	float	r, u;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	AngleVectors (self->s.angles, forward, right, NULL);

	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*1000;
	u = crandom()*500;
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	VectorCopy(self->enemy->s.origin, tv);
	tv[2] += self->enemy->viewheight;

	if(self->deadflag == DEAD_NO)
	{
		VectorSubtract(self->s.origin, self->enemy->s.origin, vel);

		dist = VectorLength(vel);

		VectorCopy(self->enemy->velocity, vel);
		if (vel[2] > 0)
			vel[2] = 0;

		VectorMA (self->enemy->s.origin, dist / 1000, vel, target);
		target[2] += self->enemy->viewheight - 8;

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	monster_fire_blaster (self, start, forward, 5, BLASTER_SPEED, flash_index, EF_BLASTER);
}

// ATTACK1

void jawa_fire1 (edict_t *self)
{
	jawa_fire (self, 0);
}

void jawa_attack1_refire (edict_t *self)
{
	trace_t tr;

	if (self->enemy->health <= 0)
		return;

	if(random() >= 0.7)
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->enemy->s.origin, self, MASK_SOLID);

	if ((skill->value >= 1) && (tr.fraction == 1))
		self->monsterinfo.nextframe = FRAME_firegun1;
	else
		self->monsterinfo.nextframe = FRAME_firegun5;
}

mframe_t jawa_frames_attack1 [] =
{
	ai_charge, 0,  jawa_fire1,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  jawa_attack1_refire,
	ai_charge, 0,  NULL
};
mmove_t jawa_move_attack1 = {FRAME_firegun1, FRAME_firegun5, jawa_frames_attack1, jawa_run};

void jawa_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &jawa_move_attack1;
}


//
// SIGHT
//

void jawa_sight(edict_t *self, edict_t *other)
{
	int n;

	n = rand() % 4;
	if(n == 0)
		gi.sound (self, CHAN_VOICE, sound_attack1, 1, ATTN_NORM, 0);
	else if(n == 1)
		gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM, 0);
	else if(n == 2)
		gi.sound (self, CHAN_VOICE, sound_attack3, 1, ATTN_NORM, 0);
	else if(n == 3)
		gi.sound (self, CHAN_VOICE, sound_attack4, 1, ATTN_NORM, 0);
}

//
// DEATH
//

void jawa_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t jawa_frames_death1 [] =
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
mmove_t jawa_move_death1 = {FRAME_diegun1, FRAME_diegun9, jawa_frames_death1, jawa_dead};

mframe_t jawa_frames_death2 [] =
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

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t jawa_move_death2 = {FRAME_diegunb1, FRAME_diegunb13, jawa_frames_death2, jawa_dead};

void jawa_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

	n = random();
	if (n >= 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);

	n = random();
	if (n >= 0.5)
		self->monsterinfo.currentmove = &jawa_move_death1;
	else
		self->monsterinfo.currentmove = &jawa_move_death2;
}


//
// SPAWN
//
void SP_monster_jawa (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_attack1 = gi.soundindex ("jawa/j_attack1.wav");
	sound_attack2 = gi.soundindex ("jawa/j_attack2.wav");
	sound_attack3 = gi.soundindex ("jawa/j_attack3.wav");
	sound_attack4 = gi.soundindex ("jawa/j_attack4.wav");
	sound_cry1 = gi.soundindex ("jawa/j_cry1.wav");
	sound_cry2 = gi.soundindex ("jawa/j_cry2.wav");
	sound_scare = gi.soundindex ("jawa/j_scare.wav");

	self->s.skinnum = 0;
	self->health = 30+(random() * 15);
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/monsters/jawa/guard.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 45;

	self->mass = 50;

	self->pain = jawa_pain;
	self->die = jawa_die;

	self->monsterinfo.stand = jawa_stand;
	self->monsterinfo.walk = jawa_walk;
	self->monsterinfo.run = jawa_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = jawa_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = jawa_sight;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

