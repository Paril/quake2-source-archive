/*
==========================================================

idg2_skull

The DOOM Lost Soul

==========================================================
*/

#include "g_local.h"
#include "m_idg2_skull.h"

static int sound_attack;

static int sound_pain;

static int sound_death;

mframe_t idg2_skull_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t idg2_skull_move_stand = {FRAME_stand1, FRAME_stand6, idg2_skull_frames_stand, NULL};


void idg2_skull_sight (edict_t *self, edict_t *other)
{
}


void idg2_skull_stand (edict_t *self)
{	
	self->monsterinfo.currentmove = &idg2_skull_move_stand;
}


mframe_t idg2_skull_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t idg2_skull_move_walk = {FRAME_stand1, FRAME_stand6, idg2_skull_frames_walk, NULL};


void idg2_skull_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skull_move_walk;
}


mframe_t idg2_skull_frames_run [] =
{
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL
};
mmove_t idg2_skull_move_run = {FRAME_stand1, FRAME_stand6, idg2_skull_frames_run, NULL};


void idg2_skull_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_skull_move_walk;
	else
		self->monsterinfo.currentmove = &idg2_skull_move_run;
}


void idg2_skull_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (5 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_attack, 1, ATTN_NORM, 0);
}


mframe_t idg2_skull_frames_attack [] =
{
	ai_charge, 15, NULL,
	ai_charge, 15, NULL,
	ai_charge, 15, NULL,
	ai_charge, 15, idg2_skull_bite,
	ai_charge, 15, NULL
};
mmove_t idg2_skull_move_attack = {FRAME_attack1, FRAME_attack5, idg2_skull_frames_attack, idg2_skull_run};

void idg2_skull_attack (edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &idg2_skull_move_attack;
}


mframe_t idg2_skull_frames_charge [] =
{
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL
};
mmove_t idg2_skull_move_charge = {FRAME_attack1, FRAME_attack2, idg2_skull_frames_charge, idg2_skull_attack};


void idg2_skull_charge (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skull_move_charge;
	gi.sound (self, CHAN_VOICE, sound_attack, 1.0, ATTN_NORM, 0);
}



mframe_t idg2_skull_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_skull_move_pain = {FRAME_pain1, FRAME_pain4, idg2_skull_frames_pain, idg2_skull_run};

void idg2_skull_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &idg2_skull_move_pain;
}

void idg2_skull_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}

/*QUAKED monster_idg2_skull (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_idg2_skull (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain = gi.soundindex ("idg2monster/skull/pain.wav");
	sound_attack = gi.soundindex ("idg2monster/skull/attack.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg2/skull/tris.md2");
	self->s.effects |= EF_ROCKET;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 70;
	self->mass = 50;

	self->pain = idg2_skull_pain;
	self->die = idg2_skull_die;

	self->monsterinfo.stand = idg2_skull_stand;
	self->monsterinfo.walk = idg2_skull_walk;
	self->monsterinfo.run = idg2_skull_run;
	self->monsterinfo.attack = idg2_skull_charge;
	self->monsterinfo.melee = idg2_skull_attack;
	self->monsterinfo.sight = idg2_skull_sight;
	self->monsterinfo.idle = NULL;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_skull_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
