//
// pinky the demon
//

#include "g_local.h"
#include "m_idg2_pinky.h"

static int  sound_death;
static int  sound_attack;
static int	sound_pain;
static int  sound_idle;
static int	sound_sight;

static void pinky_idle_sound(edict_t *self)
{	
	if(random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void pinky_search (edict_t *self)

{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM, 0);
}



static void pinky_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

//STAND

void pinky_stand (edict_t *self);
mframe_t pinky_frames_stand [] =
{
	ai_stand, 0, pinky_idle_sound,
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

	ai_stand, 0, NULL
};
mmove_t	pinky_move_stand = {FRAME_stand1, FRAME_stand21, pinky_frames_stand, NULL};

void pinky_stand (edict_t *self)
{	
	// spectre
	
	if (self->spawnflags & 8)
	{
		self->s.skinnum = 1;
		self->s.effects = EF_SPHERETRANS;
	};
	
	self->monsterinfo.currentmove = &pinky_move_stand;
}


//WALK
mframe_t pinky_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t pinky_move_walk = {FRAME_walk1, FRAME_walk5, pinky_frames_walk, NULL};

void pinky_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &pinky_move_walk;
}


// RUN
void pinky_run (edict_t *self);

mframe_t pinky_frames_run [] =
{
	ai_run, 15, NULL,
	ai_run, 15, NULL,
	ai_run, 15, NULL,
	ai_run, 15, NULL,
	ai_run, 15, NULL
};
mmove_t	pinky_move_run = {FRAME_walk1, FRAME_walk5, pinky_frames_run, NULL};

void pinky_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &pinky_move_stand;
	else
		self->monsterinfo.currentmove = &pinky_move_run;
}



//PAIN
mframe_t pinky_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL
};
mmove_t pinky_move_pain = {FRAME_pain1, FRAME_pain2, pinky_frames_pain, pinky_run};


void pinky_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;
	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
	self->monsterinfo.currentmove = &pinky_move_pain;
	self->pain_debounce_time = level.time + 3;
}

//Death

void pinky_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -30);
	VectorSet (self->maxs, 24, 24, 10);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t pinky_frames_death [] =
{
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t pinky_move_death = {FRAME_death1, FRAME_death10, pinky_frames_death, pinky_dead};


void pinky_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_NORM, 0);
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
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	if (!self->spawnflags & 8) self->s.skinnum = 2;
	self->monsterinfo.currentmove = &pinky_move_death;
}



//ATTACK

static void pinky_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, 27, 100))
		gi.sound (self, CHAN_WEAPON, sound_attack, 1, ATTN_NORM, 0);
}

mframe_t pinky_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, pinky_bite,
	ai_charge, 0, NULL
};
mmove_t pinky_move_attack = {FRAME_bite1, FRAME_bite4, pinky_frames_attack, pinky_run};


void pinky_attack(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &pinky_move_attack;
}

void SP_monster_idg2_pinky (edict_t *self)
{
	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] -= 6;
		SP_monster_idg1_dog (self);
		return;
	}
	
	sound_death=gi.soundindex ("idg2monster/pinky/death.wav");	
	sound_attack=gi.soundindex ("idg2monster/pinky/attack.wav");	
	sound_pain =gi.soundindex ("idg2monster/pinky/pain.wav");	
	sound_idle =gi.soundindex ("idg2monster/pinky/idle.wav");	
	sound_sight =gi.soundindex ("idg2monster/pinky/sight.wav");	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/pinky/tris.md2");
	VectorSet (self->mins, -20, -20, -30);
	VectorSet (self->maxs, 20, 20, 25);
	self->health = 100;
	self->gib_health = -50;
	self->mass = 120;

	self->pain = pinky_pain;
	self->die = pinky_die;

	self->monsterinfo.stand = pinky_stand;
	self->monsterinfo.walk = pinky_walk;
	self->monsterinfo.run = pinky_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = pinky_attack;
	self->monsterinfo.sight = pinky_sight;
	self->monsterinfo.search = pinky_search;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &pinky_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;


	walkmonster_start (self);
}