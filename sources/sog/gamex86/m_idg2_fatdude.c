/*
==============================================================================

DOOM II MANCUBUS

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_fatdude.h"

static int	sound_pain;
static int	sound_die;
static int	sound_sight;

mframe_t idg2_fatdude_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t idg2_fatdude_move_stand = {FRAME_stand1, FRAME_stand10, idg2_fatdude_frames_stand, NULL};

void idg2_fatdude_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_fatdude_move_stand;
}

mframe_t idg2_fatdude_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	
	ai_walk, 4, NULL,
	ai_walk, 5, NULL
};
mmove_t idg2_fatdude_move_walk = {FRAME_walk1, FRAME_walk7, idg2_fatdude_frames_walk, NULL};

void idg2_fatdude_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_fatdude_move_walk;
}

mframe_t idg2_fatdude_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 9,  NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 9,  NULL,

	ai_run, 9,  NULL,
	ai_run, 10, NULL
};
mmove_t idg2_fatdude_move_run = {FRAME_walk1, FRAME_walk7, idg2_fatdude_frames_run, NULL};

void idg2_fatdude_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_fatdude_move_stand;
	else
		self->monsterinfo.currentmove = &idg2_fatdude_move_run;
}


mframe_t idg2_fatdude_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_fatdude_move_pain = {FRAME_pain1, FRAME_pain4, idg2_fatdude_frames_pain, idg2_fatdude_run};

void idg2_fatdude_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &idg2_fatdude_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
}



void idg2_fatdude_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void idg2_fatdude_dead (edict_t *self)
{
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t idg2_fatdude_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_fatdude_move_death = {FRAME_death1, FRAME_death8, idg2_fatdude_frames_death, idg2_fatdude_dead};



void idg2_fatdude_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
    self->monsterinfo.currentmove = &idg2_fatdude_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}

void idg2_fatdude_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;

	if(!self->enemy)
		return;

	// first fireball, to the center

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,32,0,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,64,0,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, vec);	
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	d_fire_imp (self, start, dir, 30, 300);

	// second fireball, to the left
	
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,32,-16,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,64,-48,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, vec);	
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	d_fire_imp (self, start, dir, 30, 300);	

	// third fireball, to the right

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,32,16,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,64,48,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, vec);	
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	d_fire_imp (self, start, dir, 30, 300);	

	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg2weapons/rocket.wav"), 1.0, ATTN_NORM, 0);

}

mframe_t idg2_fatdude_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_fatdude_fire,
	ai_charge, 0, NULL,
	
	ai_charge, 0, NULL
};
mmove_t idg2_fatdude_move_attack = {FRAME_attack1, FRAME_attack6, idg2_fatdude_frames_attack, idg2_fatdude_run};

void idg2_fatdude_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &idg2_fatdude_move_attack;
}


/*QUAKED monster_idg2_fatdude (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
DOOM II Mancubus
*/
void SP_monster_idg2_fatdude (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 8;
		SP_monster_idg1_mutant (self);
		return;
	}

	sound_pain = gi.soundindex ("idg2monster/fatdude/pain.wav");
	sound_die = gi.soundindex ("idg2monster/fatdude/death.wav");	
	sound_sight = gi.soundindex ("idg2monster/fatdude/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg2/fatdude/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 32);

	self->health = 240;

	self->gib_health = -100;
	self->mass = 500;

	self->pain = idg2_fatdude_pain;
	self->die = idg2_fatdude_die;

	self->monsterinfo.stand = idg2_fatdude_stand;
	self->monsterinfo.walk = idg2_fatdude_walk;
	self->monsterinfo.run = idg2_fatdude_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg2_fatdude_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_fatdude_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_fatdude_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
