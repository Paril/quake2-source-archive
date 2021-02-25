/*
==============================================================================

Wolfenstein dog

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_dog.h"


static int	sound_pain;
static int	sound_death;
static int	sound_attack;
static int	sound_sight;


void idg1dog_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

//STAND

mframe_t idg1dog_frames_stand [] =
{
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
mmove_t	idg1dog_move_stand = {FRAME_stand1, FRAME_stand9, idg1dog_frames_stand, NULL};

void idg1dog_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1dog_move_stand;
}

// WALK

mframe_t idg1dog_frames_walk [] =
{
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL
};
mmove_t idg1dog_move_walk = {FRAME_walk1, FRAME_walk8, idg1dog_frames_walk, NULL};

void idg1dog_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1dog_move_walk;
}

// RUN

mframe_t idg1dog_frames_run [] =
{
	ai_run, 12, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 16, NULL,
	ai_run, 22, NULL,
	ai_run, 20, NULL,
	ai_run, 12, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 16, NULL,
	ai_run, 22, NULL,
	ai_run, 20, NULL
};
mmove_t idg1dog_move_run = {FRAME_run1, FRAME_run12, idg1dog_frames_run, NULL};

void idg1dog_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg1dog_move_walk;
	else
		self->monsterinfo.currentmove = &idg1dog_move_run;
}

// PAIN

mframe_t idg1dog_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t idg1dog_move_pain = {FRAME_pain1, FRAME_pain6, idg1dog_frames_pain, idg1dog_run};


void idg1dog_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
	return;

	self->pain_debounce_time = level.time + 1;
	
	if (skill->value == 3)
		return;

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_IDLE, 0);
	
	self->monsterinfo.currentmove = &idg1dog_move_pain;
}

// MELEE

static void idg1dog_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->maxs[0], 8);
	if (fire_hit (self, aim, (random() + random() + random())*8, 100))
		gi.sound (self, CHAN_WEAPON, sound_attack, 1, ATTN_NORM, 0);
}


static void idg1dog_checkrefire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 2) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attack1;
}


mframe_t idg1dog_frames_attack []=
{
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, idg1dog_bite,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, idg1dog_checkrefire
};
mmove_t idg1dog_move_attack = {FRAME_attack1, FRAME_attack8, idg1dog_frames_attack, idg1dog_run};

void idg1dog_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1dog_move_attack;
}		




//
// CHECKATTACK
//

static qboolean idg1dog_check_melee (edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
		return true;
	return false;
}



qboolean idg1dog_checkattack (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return false;

	if (idg1dog_check_melee(self))
	{
		self->monsterinfo.attack_state = AS_MELEE;
		return true;
	}

	return false;
}


// DEAD

void idg1dog_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}


mframe_t idg1dog_frames_death [] =
{
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
mmove_t idg1dog_move_death = {FRAME_death1, FRAME_death9, idg1dog_frames_death, idg1dog_dead};


void idg1dog_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg1/gib.wav"), 1, ATTN_NORM, 0);
		
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);

		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	
    self->monsterinfo.currentmove = &idg1dog_move_death;
}		



//
// SPAWN
//

/*QUAKED monster_idg1_dog (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_idg1_dog (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] += 6;
		SP_monster_idg2_pinky (self);
		return;
	}	
	
	sound_attack = gi.soundindex ("idg1monster/dog/attack.wav");
	sound_death = gi.soundindex ("idg1monster/dog/death.wav");
	sound_sight = gi.soundindex ("idg1monster/dog/sight.wav");
	sound_pain = gi.soundindex ("idg1monster/dog/pain.wav");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg1/dog/tris.md2");
	VectorSet (self->mins, -32, -16, -24);
	VectorSet (self->maxs, 32, 16, 32);

	self->health = 25;
	self->gib_health = -60;
	self->mass = 80;

	self->pain = idg1dog_pain;
	self->die = idg1dog_die;

	self->monsterinfo.stand = idg1dog_stand;
	self->monsterinfo.walk = idg1dog_walk;
	self->monsterinfo.run = idg1dog_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = idg1dog_melee;
	self->monsterinfo.sight = idg1dog_sight;
	self->monsterinfo.search = idg1dog_stand;
	self->monsterinfo.idle = idg1dog_stand;
	self->monsterinfo.checkattack = idg1dog_checkattack;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &idg1dog_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}

