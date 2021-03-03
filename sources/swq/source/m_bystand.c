// m_bystand.c

#include "g_local.h"
#include "m_bystand.h"

void find_action_point (edict_t *self, int flags, qboolean run);

void maleworker_AI_stand (edict_t *self)
{
	if(self->sflags == 10)
		return;

	if(random() > 0.001)
	{
		if(random() > 0.5)
		{
			find_action_point(self, 2, 0);
		}
		else
		{
			find_action_point(self, 4, 0);
		}
	}
}

mframe_t maleworker_frames_stand [] =
{
	ai_stand, 0, maleworker_AI_stand,
	ai_stand, 0, maleworker_AI_stand,
	ai_stand, 0, maleworker_AI_stand,
	ai_stand, 0, maleworker_AI_stand
};
mmove_t maleworker_move_stand = {FRAME_slackingA, FRAME_slackingB, maleworker_frames_stand, NULL};

void maleworker_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &maleworker_move_stand;

	// randomize on startup
	if (level.time < 1.0)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));
}


//
// WALK
//

mframe_t maleworker_frames_walk [] =
{
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4,  NULL,

	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL
};
mmove_t maleworker_move_walk = {FRAME_walkA, FRAME_walkB, maleworker_frames_walk, NULL};

void maleworker_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &maleworker_move_walk;
}


//
// RUN
//

mframe_t maleworker_frames_run [] =
{
	ai_run, 4,  NULL,
	ai_run, 15, NULL,
	ai_run, 15, NULL,
	ai_run, 8,  NULL,
	ai_run, 20, NULL,
	ai_run, 15, NULL
};
mmove_t maleworker_move_run = {FRAME_runA, FRAME_runB, maleworker_frames_run, NULL};

void maleworker_run (edict_t *self)
{
	if ((level.time < self->pain_debounce_time) && (!self->enemy))
	{
		if (self->movetarget)
			maleworker_walk(self);
		else
			maleworker_stand(self);
		return;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		maleworker_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &maleworker_move_run;
}

//
// PAIN
//

mframe_t maleworker_frames_pain [] =
{
	ai_move, -5, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL
};
mmove_t maleworker_move_pain = {FRAME_painA, FRAME_painB, maleworker_frames_pain, maleworker_run};

mframe_t maleworker_frames_pain2 [] =
{
	ai_move, -4, NULL,
	ai_move, 4,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t maleworker_move_pain2 = {FRAME_painA, FRAME_painB, maleworker_frames_pain2, maleworker_run};

void maleworker_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;
	float r;

//	if (self->health < (self->max_health / 2))
//		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	r=random();

	self->enemy = other;

	if(self->health < 25)
	{
		if(r < 0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain25_1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain25_2.wav"), 1, ATTN_NORM, 0);
	}
	else if(self->health < 50)
	{
		if(r < 0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain50_1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain50_2.wav"), 1, ATTN_NORM, 0);
	}
	if(self->health < 75)
	{
		if(r < 0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain75_1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain75_2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(r < 0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain100_1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/pain100_2.wav"), 1, ATTN_NORM, 0);
	}

	n = rand() % 3;
	if (n == 0)
		self->monsterinfo.currentmove = &maleworker_move_pain;
	else if (n == 1)
		self->monsterinfo.currentmove = &maleworker_move_pain;
}

void maleworker_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t maleworker_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -13, NULL,
	ai_move, 14,  NULL,
	ai_move, 0,   NULL,
	ai_move, 14,  NULL
};
mmove_t maleworker_move_death1 = {FRAME_dieA, FRAME_dieB, maleworker_frames_death1, maleworker_dead};

mframe_t maleworker_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 7,   NULL,
	ai_move, 0,   NULL,
	ai_move, -6,  NULL,
	ai_move, -5,  NULL,
	ai_move, 1,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 7,   NULL,
	ai_move, 0,   NULL,

	ai_move, -6,  NULL,
	ai_move, -5,  NULL,
	ai_move, 1,   NULL,
	ai_move, 0,   NULL,
	ai_move, -6,  NULL,
	ai_move, -5,  NULL,
	ai_move, 1,   NULL,
	ai_move, 0,   NULL
};
mmove_t maleworker_move_death2 = {FRAME_diebA, FRAME_diebB, maleworker_frames_death2, maleworker_dead};

void maleworker_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	float r;

	self->s.modelindex2 = 0;

// check for gib
	if (self->health <= -80)
	{
//		gi.sound (self, CHAN_VOICE, maleworker.sound_gib, 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death

	r = random();

	if(r<0.25)
		gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/death1.wav"), 1, ATTN_NORM, 0);
	else if(r<0.5)
		gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/death2.wav"), 1, ATTN_NORM, 0);
	else if(r<0.75)
		gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/death3.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, gi.soundindex("player/male/death4.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand() % 2;
	if (n == 0)
		self->monsterinfo.currentmove = &maleworker_move_death1;
	else
		self->monsterinfo.currentmove = &maleworker_move_death2;
}

//
// ATTACK
//

mframe_t maleworker_frames_attack [] =
{
	ai_charge, -2,  NULL,
	ai_charge, -2,  NULL,
	ai_charge, 3,   NULL,
	ai_charge, 2,   NULL,
	ai_charge, -2,  NULL,
	ai_charge, -2,  NULL,
	ai_charge, 3,   NULL,
	ai_charge, -2,  NULL,
	ai_charge, -2,  NULL,
	ai_charge, 2,   NULL,

	ai_charge, -2,  NULL

};
mmove_t maleworker_move_attack = {FRAME_swingA, FRAME_swingB, maleworker_frames_attack, maleworker_run};

void maleworker_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &maleworker_move_attack;
}

/*QUAKED npc_maleworker (1 .5 0) (-16 -16 -24) (16 16 32)
*/
void SP_npc_maleworker (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/npcs/mworker/mworker.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	if (!self->health)
		self->health = 20;
	self->mass = 200;

	self->pain = maleworker_pain;
	self->die = maleworker_die;

	self->monsterinfo.stand = maleworker_stand;
	self->monsterinfo.walk = maleworker_walk;
	self->monsterinfo.run = maleworker_run;
	self->monsterinfo.attack = maleworker_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	if(self->spawnflags & 4)
	{
		self->monsterinfo.aiflags |= AI_NPC_VIOLENT;
	}
	else
	{
		self->monsterinfo.aiflags |= AI_NPC_PASSIVE;
	}

	gi.linkentity (self);

	self->monsterinfo.currentmove = &maleworker_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
