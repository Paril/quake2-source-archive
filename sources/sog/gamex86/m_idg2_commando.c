/*
==============================================================================

DOOM II COMMANDO

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_commando.h"

static int	sound_pain;
static int	sound_die;
static int	sound_sight;

mframe_t idg2_commando_frames_stand [] =
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
mmove_t idg2_commando_move_stand = {FRAME_stand1, FRAME_stand10, idg2_commando_frames_stand, NULL};

void idg2_commando_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_commando_move_stand;
}

mframe_t idg2_commando_frames_walk [] =
{
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL
};
mmove_t idg2_commando_move_walk = {FRAME_walk1, FRAME_walk5, idg2_commando_frames_walk, NULL};

void idg2_commando_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_commando_move_walk;
}

mframe_t idg2_commando_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL
};
mmove_t idg2_commando_move_run = {FRAME_walk1, FRAME_walk5, idg2_commando_frames_run, NULL};

void idg2_commando_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_commando_move_stand;
	else
		self->monsterinfo.currentmove = &idg2_commando_move_run;
}


mframe_t idg2_commando_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_commando_move_pain = {FRAME_pain1, FRAME_pain4, idg2_commando_frames_pain, idg2_commando_run};

void idg2_commando_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &idg2_commando_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
}



void idg2_commando_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void idg2_commando_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t idg2_commando_frames_death [] =
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
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_commando_move_death = {FRAME_death1, FRAME_death13, idg2_commando_frames_death, idg2_commando_dead};



void idg2_commando_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
    self->monsterinfo.currentmove = &idg2_commando_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}

void idg2_commando_fire (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	int		flash_number;

	flash_number = MZ2_INFANTRY_MACHINEGUN_1;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
		target[2] += self->enemy->viewheight;
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	else
	{
		AngleVectors (self->s.angles, forward, right, NULL);
	}

	monster_fire_bullet (self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

mframe_t idg2_commando_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_commando_fire,
	
	ai_charge, 0, idg2_commando_fire,
	ai_charge, 0, idg2_commando_fire,
	ai_charge, 0, idg2_commando_fire,
	ai_charge, 0, idg2_commando_fire,
	ai_charge, 0, idg2_commando_fire,
	
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg2_commando_move_attack = {FRAME_attack1, FRAME_attack14, idg2_commando_frames_attack, idg2_commando_run};

void idg2_commando_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &idg2_commando_move_attack;
}


/*QUAKED monster_idg2_commando (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
DOOM II Commando
*/
void SP_monster_idg2_commando (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 11;
		SP_monster_idg1_ss (self);
		return;
	}
	
	sound_pain = gi.soundindex ("idg2monster/sarge/pain.wav");
	sound_die = gi.soundindex ("idg2monster/sarge/death.wav");	
	sound_sight = gi.soundindex ("idg2monster/sarge/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg2/commando/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 120;

	self->gib_health = -60;
	self->mass = 200;

	self->pain = idg2_commando_pain;
	self->die = idg2_commando_die;

	self->monsterinfo.stand = idg2_commando_stand;
	self->monsterinfo.walk = idg2_commando_walk;
	self->monsterinfo.run = idg2_commando_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg2_commando_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_commando_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_commando_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
