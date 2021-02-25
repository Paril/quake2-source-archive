/*
==============================================================================

WOLFENSTEIN PRIEST  ( FAKE HITLER )

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_priest.h"

static int	sound_pain;
static int	sound_die;
static int	sound_sight;

mframe_t idg1_priest_frames_stand [] =
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
mmove_t idg1_priest_move_stand = {FRAME_stand1, FRAME_stand10, idg1_priest_frames_stand, NULL};

void idg1_priest_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1_priest_move_stand;
}

mframe_t idg1_priest_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,

	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t idg1_priest_move_walk = {FRAME_walk1, FRAME_walk8, idg1_priest_frames_walk, NULL};

void idg1_priest_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1_priest_move_walk;
}

mframe_t idg1_priest_frames_run [] =
{
	ai_run, 8, NULL,
	ai_run, 7,  NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 7,  NULL,

	ai_run, 7,  NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL
};
mmove_t idg1_priest_move_run = {FRAME_walk1, FRAME_walk8, idg1_priest_frames_run, NULL};

void idg1_priest_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg1_priest_move_stand;
	else
		self->monsterinfo.currentmove = &idg1_priest_move_run;
}


mframe_t idg1_priest_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg1_priest_move_pain = {FRAME_pain1, FRAME_pain4, idg1_priest_frames_pain, idg1_priest_run};

void idg1_priest_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &idg1_priest_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
}



void idg1_priest_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void idg1_priest_dead (edict_t *self)
{
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t idg1_priest_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg1_priest_move_death = {FRAME_death1, FRAME_death12, idg1_priest_frames_death, idg1_priest_dead};



void idg1_priest_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
    self->monsterinfo.currentmove = &idg1_priest_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}

void idg1_priest_fire (edict_t *self)
{
	
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;

	if(!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,0,0,16);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg2monster/monkey/attack2.wav"), 1.0, ATTN_NORM, 0);
	d_fire_imp (self, start, dir, 20, 200);	
	
}

mframe_t idg1_priest_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1_priest_fire,
	ai_charge, 0, NULL,
	
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg1_priest_move_attack = {FRAME_attack1, FRAME_attack7, idg1_priest_frames_attack, idg1_priest_run};

void idg1_priest_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &idg1_priest_move_attack;
}


/*QUAKED monster_idg1_priest (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
Wolfenstein Priest (Fake Hitler)
*/
void SP_monster_idg1_priest (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_monster_idg2_skeleton (self);
		return;
	}

	sound_pain = gi.soundindex ("idg1monster/priest/pain.wav");
	sound_die = gi.soundindex ("idg1monster/priest/death.wav");	
	sound_sight = gi.soundindex ("idg1monster/priest/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg1/priest/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 32);

	self->health = 300;

	self->gib_health = -666;
	self->mass = 300;

	self->pain = idg1_priest_pain;
	self->die = idg1_priest_die;

	self->monsterinfo.stand = idg1_priest_stand;
	self->monsterinfo.walk = idg1_priest_walk;
	self->monsterinfo.run = idg1_priest_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg1_priest_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg1_priest_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg1_priest_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
