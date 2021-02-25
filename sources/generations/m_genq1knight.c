/*
==============================================================================

QUAkE KNIGHT

==============================================================================
*/

#include "g_local.h"
#include "m_genq1knight.h"


static int	sound_sword1;
static int	sound_sword2;
static int	sound_death;
static int	sound_idle;
static int	sound_pain;
static int	sound_sight;


void knight_stand (edict_t *self);
void knight_walk (edict_t *self);
void knight_check_dist (edict_t *self);


//
// SOUNDS
//


void knight_sight_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_IDLE, 0);
	self->style ++;
}


void knight_sight (edict_t *self, edict_t *other)
{
	if(self->style <= 1)
		knight_sight_sound(self);
	knight_check_dist(self);
}

void knight_idle_sound (edict_t *self)
{
	if(random() < 0.1)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_STATIC, 0);
}

void knight_sword_sound (edict_t *self)
{
	//if(self->style)
	//	return;

	if(random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_sword1, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_sword2, 1, ATTN_IDLE, 0);
	//self->style = 1;
}

mframe_t knight_frames_kneel[] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t knight_move_kneel = {FRAME_kneel1, FRAME_kneel5, knight_frames_kneel, knight_stand};

void knight_kneel (edict_t *self)
{
	if(random() < 0.1)
	self->monsterinfo.currentmove = &knight_move_kneel;
}


mframe_t knight_frames_standing[] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t knight_move_standing = {FRAME_standing2, FRAME_standing5, knight_frames_standing, knight_walk};

void knight_standing (edict_t *self)
{
	if(random() < 0.2)
	self->monsterinfo.currentmove = &knight_move_standing;
}


//
// STAND
//

mframe_t knight_frames_stand [] =
{
	ai_stand, 0, knight_idle_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, knight_kneel,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL		

};
mmove_t knight_move_stand = {FRAME_stand1, FRAME_stand9, knight_frames_stand, knight_stand};

void knight_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &knight_move_stand;
}


//
// WALK
//

mframe_t knight_frames_walk [] =
{
	ai_walk,	3,		knight_idle_sound,
	ai_walk,	2,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	3,		knight_standing,
	ai_walk,	3,		NULL,
	ai_walk,	2,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	3,		NULL
};
mmove_t knight_move_walk = {FRAME_walk1, FRAME_walk14, knight_frames_walk, knight_walk};

void knight_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &knight_move_walk;
}


//
// RUN
//
// can be improved

void knight_check_dist (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if (range(self, self->enemy) == RANGE_MELEE)
	{
		self->monsterinfo.nextframe = FRAME_attackb1;
	}
	else
	{
		if(random() > 0.6)
			self->monsterinfo.nextframe = FRAME_runattack1;
		else
			self->monsterinfo.nextframe = FRAME_runb1;
	}
}


mframe_t knight_frames_run [] =
{
	ai_run,	16,		NULL,
	ai_run,	20,		NULL,
	ai_run,	13,		NULL,
	ai_run,	7,		NULL,
	ai_run,	16,		NULL,
	ai_run,	20,		NULL,
	ai_run,	14,		NULL,
	ai_run,	6,		knight_check_dist
};
mmove_t knight_move_run = {FRAME_runb1, FRAME_runb8, knight_frames_run, NULL};


void knight_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &knight_move_stand;
	else
		self->monsterinfo.currentmove = &knight_move_run;
}


void knight_attack (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 2);
	if (fire_hit (self, aim, (10 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_sword1, 1, ATTN_NORM, 0);
}



mframe_t knight_frames_attack_run [] =
{
	ai_charge,	20,		NULL,
	ai_charge,	20,		knight_sword_sound,
	ai_charge,	13,		NULL,
	ai_charge,	7,		NULL,
	ai_charge,	16,		NULL,
	ai_charge,	20,		knight_attack,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	6,		knight_check_dist
};
mmove_t knight_move_attack_run = {FRAME_runattack1, FRAME_runattack11, knight_frames_attack_run, knight_run};

void knight_attack_run (edict_t *self)
{
	self->monsterinfo.currentmove = &knight_move_attack_run;
}


void knight_hit_left (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 4);
	if (fire_hit (self, aim, (10 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_sword1, 1, ATTN_NORM, 0);
}

mframe_t knight_frames_attack [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	knight_hit_left,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	knight_check_dist
};
mmove_t knight_move_attack = {FRAME_attackb1, FRAME_attackb10, knight_frames_attack, knight_run};


void knight_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &knight_move_attack;
}


//
// PAIN
//

mframe_t knight_frames_pain1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t knight_move_pain1 = {FRAME_pain1, FRAME_pain3, knight_frames_pain1, knight_run};

mframe_t knight_frames_pain2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t knight_move_pain2 = {FRAME_painb1, FRAME_painb11, knight_frames_pain2, knight_run};

void knight_pain (edict_t *self, edict_t *other, float kick, int damage)
{
		
	if (level.time < self->pain_debounce_time)
		return;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->pain_debounce_time = level.time + 1.1;

	if(random() <0.85)
		self->monsterinfo.currentmove = &knight_move_pain1;
	else
		self->monsterinfo.currentmove = &knight_move_pain2;

}


//
// DEATH
//

void knight_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t knight_frames_death1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t knight_move_death1 = {FRAME_death1, FRAME_death10, knight_frames_death1, knight_dead};


mframe_t knight_frames_death2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t knight_move_death2 = {FRAME_deathb1, FRAME_deathb11, knight_frames_death2, knight_dead};



void knight_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)
	{
		if(random() <0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/gib.wav"), 1, ATTN_NORM, 0);
		
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
		ThrowHead(self, "models/monsters/q1monstr/knight/head/tris.md2", damage, GIB_ORGANIC);
		
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if(random() < 0.5)
		self->monsterinfo.currentmove = &knight_move_death1;
	else
		self->monsterinfo.currentmove = &knight_move_death2;
}

//
// SPAWN
//

/*QUAKED monster_mutant (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void SP_q1_monster_knight (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death = gi.soundindex ("q1monstr/knight/kdeath.wav");
	sound_pain = gi.soundindex ("q1monstr/knight/khurt.wav");
	sound_idle = gi.soundindex ("q1monstr/knight/idle.wav");
	sound_sight = gi.soundindex ("q1monstr/knight/ksight.wav");
	sound_sword1 = gi.soundindex ("q1monstr/knight/sword1.wav");
	sound_sword2 = gi.soundindex ("q1monstr/knight/sword2.wav");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/knight/tris.md2");
		
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 75;
	self->gib_health = -50;
	self->mass = 125;

	self->pain = knight_pain;
	self->die = knight_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = knight_stand;
	self->monsterinfo.walk = knight_walk;
	self->monsterinfo.run = knight_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = knight_attack_run; 
	self->monsterinfo.melee = knight_melee;
	self->monsterinfo.sight = knight_sight;
	self->monsterinfo.search = knight_stand;
	self->monsterinfo.idle = knight_stand;
	
	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &knight_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}