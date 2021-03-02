/*
==============================================================================

jawa2

==============================================================================
*/

#include "g_local.h"
#include "m_jawa2.h"

static int	sound_agree;
static int	sound_cry1;
static int	sound_cry2;
static int	sound_disagree;
static int	sound_lift1;
static int	sound_lift2;
static int	sound_scare;
static int	sound_talk1;
static int	sound_talk2;
static int	sound_talk3;
static int	sound_talk4;
static int	sound_talk5;
static int	sound_talk6;
static int	sound_work;

// STAND

void jawa2_stand (edict_t *self);

mframe_t jawa2_frames_stand2 [] =
{
	ai_stand, 0, NULL,
};
mmove_t jawa2_move_stand2 = {FRAME_stand1, FRAME_stand1, jawa2_frames_stand2, jawa2_stand};

mframe_t jawa2_frames_stand1 [] =
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
mmove_t jawa2_move_stand1 = {FRAME_stand1, FRAME_stand15, jawa2_frames_stand1, jawa2_stand};

mframe_t jawa2_frames_idle1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa2_move_idle1 = {FRAME_idle1, FRAME_idle6, jawa2_frames_idle1, jawa2_stand};

//********
//SLACKING
//********

void jawa2_slacking_talk (edict_t *self)
{
	float n;

	if(random() < 0.8)
	{
		return;
	}

	n = random();

	if(n > .83)
		gi.sound (self, CHAN_VOICE, sound_talk1, 1, ATTN_IDLE, 0);
	else if (n > .66)
		gi.sound (self, CHAN_VOICE, sound_talk2, 1, ATTN_IDLE, 0);
	else if (n > .49)
		gi.sound (self, CHAN_VOICE, sound_talk3, 1, ATTN_IDLE, 0);
	else if (n > .32)
		gi.sound (self, CHAN_VOICE, sound_agree, 1, ATTN_IDLE, 0);
	else if (n > .15)
		gi.sound (self, CHAN_VOICE, sound_talk5, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_talk6, 1, ATTN_IDLE, 0);
}

mframe_t jawa2_frames_drink1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, jawa2_slacking_talk
};
mmove_t jawa2_move_drink1 = {FRAME_drink1, FRAME_drink8, jawa2_frames_drink1, jawa2_stand};

mframe_t jawa2_frames_slacking1 [] =
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
	ai_stand, 0, jawa2_slacking_talk
};
mmove_t jawa2_move_slacking1 = {FRAME_slacking1, FRAME_slacking10, jawa2_frames_slacking1, jawa2_stand};

//*********
//WORK WALL
//*********

void jawa2_work_talk (edict_t *self)
{
	if(random() > 0.9)
	{
		gi.sound (self, CHAN_VOICE, sound_work, 1, ATTN_IDLE, 0);
	}
}

void jawa2_frust_talk (edict_t *self)
{
	if(random() > 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_IDLE, 0);
}

mframe_t jawa2_frames_workwall1 [] =
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
	ai_stand, 0, jawa2_work_talk
};
mmove_t jawa2_move_workwall1 = {FRAME_work1, FRAME_work15, jawa2_frames_workwall1, jawa2_stand};

//**********
//WORK FLOOR
//**********

void jawa2_workfloor_sparks (edict_t *self)
{
	vec3_t pos, forward, right, angle;

	if(random() < 0.4)
	{
		return;
	}

	VectorCopy(self->s.origin, pos);
	pos[2] -= 24;

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(pos, 8, right, pos);
	VectorMA(pos, 18, forward, pos);

	VectorCopy(forward, angle);
	angle[2] += 1;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LASER_SPARKS);
	gi.WriteByte (30);
	gi.WritePosition (pos);
	gi.WriteDir (angle);
	gi.WriteByte (0xd0d1d2d3);
	gi.multicast (pos, MULTICAST_PVS);

//0xf2f2f0f0 red
//0xd0d1d2d3 green
//0xf3f4f1f1 blue
//0xdcdddedf yellow
//0xe0e1e2e3 bitty yellow strobe


//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_BLASTER);
//	gi.WritePosition (pos);
//	gi.WriteDir (angle);
//	gi.multicast (pos, MULTICAST_PVS);
}

mframe_t jawa2_frames_workfloor1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, jawa2_workfloor_sparks,
	ai_stand, 0, jawa2_work_talk
};
mmove_t jawa2_move_workfloor1 = {FRAME_workb1, FRAME_workb6, jawa2_frames_workfloor1, jawa2_stand};

mframe_t jawa2_frames_frust1 [] =
{
	ai_stand, 0, jawa2_frust_talk,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, jawa2_workfloor_sparks
};
mmove_t jawa2_move_frust1 = {FRAME_frust1, FRAME_frust4, jawa2_frames_frust1, jawa2_stand};

mframe_t jawa2_frames_sweat1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa2_move_sweat1 = {FRAME_sweat1, FRAME_sweat6, jawa2_frames_sweat1, jawa2_stand};

void jawa2_stand (edict_t *self)
{
	float n;

	n = random();

	if(self->spawnflags & 1)
	{
		self->monsterinfo.currentmove = &jawa2_move_workwall1;
	}
	else if(self->spawnflags & 8)
	{
		if(n > 0.07)
			self->monsterinfo.currentmove = &jawa2_move_workfloor1;
		else if (n < 0.03)
			self->monsterinfo.currentmove = &jawa2_move_frust1;
		else
			self->monsterinfo.currentmove = &jawa2_move_sweat1;
	}
	else if(self->spawnflags & 4)
	{
		if(n > 0.3)
			self->monsterinfo.currentmove = &jawa2_move_slacking1;
		else if (n < 0.03)
			self->monsterinfo.currentmove = &jawa2_move_drink1;
	}
	else
	{
		if(n < 0.01)
			self->monsterinfo.currentmove = &jawa2_move_stand1;
		else if (n < 0.03)
			self->monsterinfo.currentmove = &jawa2_move_idle1;
		else
			self->monsterinfo.currentmove = &jawa2_move_stand2;
	}
}

//
// WALK
//

mframe_t jawa2_frames_walk1 [] =
{
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,

	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL
};
mmove_t jawa2_move_walk1 = {FRAME_walk1, FRAME_walk12, jawa2_frames_walk1, NULL};

void jawa2_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &jawa2_move_walk1;
}

//
// RUN
//

void jawa2_run (edict_t *self);

mframe_t jawa2_frames_run1 [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t jawa2_move_run1 = {FRAME_run1, FRAME_run6, jawa2_frames_run1, NULL};

void jawa2_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &jawa2_move_stand1;
	else
	{
		self->monsterinfo.currentmove = &jawa2_move_run1;
	}
}

//
// PAIN
//

mframe_t jawa2_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t jawa2_move_pain1 = {FRAME_pain1, FRAME_pain3, jawa2_frames_pain1, jawa2_run};

void jawa2_pain (edict_t *self, edict_t *other, float kick, int damage)
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

	self->monsterinfo.currentmove = &jawa2_move_pain1;
}


//
// ATTACK
//

void jawa2_attack(edict_t *self)
{
}


//
// SIGHT
//

void jawa2_sight(edict_t *self, edict_t *other)
{
}

//
// DEATH
//

void jawa2_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t jawa2_frames_death1 [] =
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
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t jawa2_move_death1 = {FRAME_drama1, FRAME_drama19, jawa2_frames_death1, jawa2_dead};

mframe_t jawa2_frames_death2 [] =
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
mmove_t jawa2_move_death2 = {FRAME_nodrama1, FRAME_nodrama9, jawa2_frames_death2, jawa2_dead};

void jawa2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
		self->monsterinfo.currentmove = &jawa2_move_death1;
	else
		self->monsterinfo.currentmove = &jawa2_move_death2;
}


//
// SPAWN
//

void SP_monster_jawa2 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_agree = gi.soundindex ("jawa/j_agree.wav");
	sound_cry1 = gi.soundindex ("jawa/j_cry1.wav");
	sound_cry2 = gi.soundindex ("jawa/j_cry2.wav");
	sound_disagree = gi.soundindex ("jawa/j_disagr.wav");
	sound_lift1 = gi.soundindex ("jawa/j_lift1.wav");
	sound_lift2 = gi.soundindex ("jawa/j_lift2.wav");
	sound_scare = gi.soundindex ("jawa/j_scare.wav");
	sound_talk1 = gi.soundindex ("jawa/j_talk1.wav");
	sound_talk2 = gi.soundindex ("jawa/j_talk2.wav");
	sound_talk3 = gi.soundindex ("jawa/j_talk3.wav");
	sound_talk4 = gi.soundindex ("jawa/j_talk4.wav");
	sound_talk5 = gi.soundindex ("jawa/j_talk5.wav");
	sound_talk6 = gi.soundindex ("jawa/j_talk6.wav");
	sound_work = gi.soundindex ("jawa/j_work.wav");

	self->s.skinnum = 0;
	self->health = 30+(random() * 15);
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/monsters/jawa/worker.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 45;

	self->mass = 50;

	self->pain = jawa2_pain;
	self->die = jawa2_die;

	self->monsterinfo.stand = jawa2_stand;
	self->monsterinfo.walk = jawa2_walk;
	self->monsterinfo.run = jawa2_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = jawa2_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = jawa2_sight;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

