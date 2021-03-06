/*
==============================================================================

Fly: Heliocopty monster from Cutman's stage

==============================================================================
*/

#include "m_fly.h"
#include "g_local.h"

qboolean visible (edict_t *self, edict_t *other);

static int	nextmove;			// Used for start/stop frames

//static int	sound_sight;
//static int	sound_idle;
//static int	sound_pain1;
//static int	sound_pain2;
//static int	sound_slash;
//static int	sound_sproing;
//static int	sound_die;


void fly_check_melee(edict_t *self);
void fly_loop_melee (edict_t *self);
void fly_melee (edict_t *self);
void fly_setstart (edict_t *self);
void fly_stand (edict_t *self);
void fly_nextmove (edict_t *self);


void fly_sight (edict_t *self, edict_t *other)
{
//	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void fly_idle (edict_t *self)
{
//	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void fly_pop_blades (edict_t *self)
{
//	gi.sound (self, CHAN_VOICE, sound_sproing, 1, ATTN_NORM, 0);
}


mframe_t fly_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	fly_move_stand = {FRAME_spin1, FRAME_spin5, fly_frames_stand, NULL};


mframe_t fly_frames_walk [] =
{
	ai_walk, 15, NULL,
	ai_walk, 15, NULL,
	ai_walk, 15, NULL,
	ai_walk, 15, NULL,
	ai_walk, 15, NULL
};
mmove_t	fly_move_walk = {FRAME_spin1, FRAME_spin5, fly_frames_walk, NULL};

mframe_t fly_frames_run [] =
{
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL
};
mmove_t	fly_move_run = {FRAME_spin1, FRAME_spin5, fly_frames_run, NULL};

void fly_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &fly_move_stand;
	else
		self->monsterinfo.currentmove = &fly_move_run;
}

void fly_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &fly_move_walk;
}

void fly_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &fly_move_stand;
}

mframe_t fly_frames_start [] =
{
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	fly_nextmove
};
mmove_t fly_move_start = {FRAME_spin1, FRAME_spin5, fly_frames_start, NULL};

mframe_t fly_frames_stop [] =
{
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	NULL,
		ai_move, 0,	fly_nextmove
};
mmove_t fly_move_stop = {FRAME_spin1, FRAME_spin5, fly_frames_stop, NULL};

void fly_stop (edict_t *self)
{
		self->monsterinfo.currentmove = &fly_move_stop;
}

void fly_start (edict_t *self)
{
		self->monsterinfo.currentmove = &fly_move_start;
}


mframe_t fly_frames_pain1 [] =
{
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL
};
mmove_t fly_move_pain1 = {FRAME_pain1, FRAME_pain5, fly_frames_pain1, fly_run};


void fly_slash (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 0);
	fire_hit (self, aim, 5, 300);
	//gi.sound (self, CHAN_WEAPON, sound_slash, 1, ATTN_NORM, 0);
}

mframe_t fly_frames_start_melee [] =
{
		ai_charge, 0, NULL
};
mmove_t fly_move_start_melee = {FRAME_stand1, FRAME_stand1, fly_frames_start_melee, fly_loop_melee};

mframe_t fly_frames_end_melee [] =
{
		ai_charge, 0, NULL
};
mmove_t fly_move_end_melee = {FRAME_stand1, FRAME_stand1, fly_frames_end_melee, fly_run};


mframe_t fly_frames_loop_melee [] =
{
		ai_charge, 50, NULL,
		ai_charge, 50, NULL,
		ai_charge, 50, NULL,
		ai_charge, 0, fly_slash,
		ai_charge, 0, NULL

};
mmove_t fly_move_loop_melee = {FRAME_spin1, FRAME_spin5, fly_frames_loop_melee, fly_check_melee};

void fly_loop_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &fly_move_loop_melee;
}



/*void fly_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &flyer_move_attack2;
}*/

void fly_setstart (edict_t *self)
{
	nextmove = ACTION_run;
	self->monsterinfo.currentmove = &fly_move_start;
}

void fly_nextmove (edict_t *self)
{
	if (nextmove == ACTION_attack1)
		self->monsterinfo.currentmove = &fly_move_start_melee;
//	else if (nextmove == ACTION_attack2)
//		self->monsterinfo.currentmove = &fly_move_attack2;
	else if (nextmove == ACTION_run)
		self->monsterinfo.currentmove = &fly_move_run;
}

void fly_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &fly_move_start_melee;
}

void fly_check_melee(edict_t *self)
{
	if (range (self, self->enemy) == 200)
		self->monsterinfo.currentmove = &fly_move_loop_melee;
	else
		self->monsterinfo.currentmove = &fly_move_end_melee;
}

void fly_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	self->monsterinfo.currentmove = &fly_move_pain1;
}


void fly_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}


void SP_monster_fly (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	/*sound_sight = gi.soundindex ("flyer/flysght1.wav");
	sound_idle = gi.soundindex ("flyer/flysrch1.wav");
	sound_pain1 = gi.soundindex ("flyer/flypain1.wav");
	sound_pain2 = gi.soundindex ("flyer/flypain2.wav");
	sound_slash = gi.soundindex ("flyer/flyatck2.wav");
	sound_sproing = gi.soundindex ("flyer/flyatck1.wav");
	sound_die = gi.soundindex ("flyer/flydeth1.wav");*/

	//gi.soundindex ("flyer/flyatck3.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/fly/tris.md2");
	VectorSet (self->mins, -12, -12, -16);
	VectorSet (self->maxs, 12, 12, 16);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	//self->s.sound = gi.soundindex ("flyer/flyidle1.wav");

	self->health = 10;
	self->mass = 50;

	self->pain = fly_pain;
	self->die = fly_die;

	self->monsterinfo.stand = fly_stand;
	self->monsterinfo.walk = fly_walk;
	self->monsterinfo.run = fly_run;
	//self->monsterinfo.attack = fly_attack;
	self->monsterinfo.melee = fly_melee;
	self->monsterinfo.sight = fly_sight;
	self->monsterinfo.idle = fly_idle;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &fly_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}

