/*
==============================================================================

WOLF SS

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_ss.h"

void ssMachineGun (edict_t *self);



static int	sound_pain;
static int	sound_die;
static int	sound_attack;
static int	sound_sight;

mframe_t ss_frames_stand [] =
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t ss_move_stand = {FRAME_stand01, FRAME_stand17, ss_frames_stand, NULL};

void ss_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &ss_move_stand;
}

mframe_t ss_frames_walk [] =
{
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL
};
mmove_t ss_move_walk = {FRAME_roam01, FRAME_roam09, ss_frames_walk, NULL};

void ss_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &ss_move_walk;
}

mframe_t ss_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 11, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};
mmove_t ss_move_run = {FRAME_run01, FRAME_run08, ss_frames_run, NULL};

void ss_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &ss_move_stand;
	else
		self->monsterinfo.currentmove = &ss_move_run;
}


mframe_t ss_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL
};
mmove_t ss_move_pain1 = {FRAME_paina01, FRAME_paina03, ss_frames_pain1, ss_run};

mframe_t ss_frames_pain2 [] =
{
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, 0,  NULL,
	ai_move, 2,  NULL
};
mmove_t ss_move_pain2 = {FRAME_painb01, FRAME_painb07, ss_frames_pain2, ss_run};

void ss_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;


	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;


	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare


	n = rand() % 2;
	if (n == 0)
	{
		self->monsterinfo.currentmove = &ss_move_pain1;
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}
	else
	{
		self->monsterinfo.currentmove = &ss_move_pain2;
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}
}


void ssMachineGun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	int		flash_number;
	int 	damage;

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

	// Damage depends on distance
	
	switch (range (self, self->enemy))
	{
		case RANGE_MELEE:
			damage = 4;
			break;
		case RANGE_NEAR:
			damage = 3;
			break;
		case RANGE_MID:
			damage = 2;
			break;
		case RANGE_FAR:
			damage = 1;
			break;
		default:
			damage = 0;
			break;
	}

	// Damage also depends on skill
/*	
	if (skill->value == 1)
		damage = round(damage * 1.1);
	else if (skill->value == 2)
		damage = round(damage * 1.3);
	else if (skill->value == 3)
		damage = round(damage * 1.5);
*/
	monster_fire_bullet (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

void ss_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void ss_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -35);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t ss_frames_death [] =
{
	ai_move, -4, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -4, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t ss_move_death = {FRAME_death01, FRAME_death07, ss_frames_death, ss_dead};



void ss_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg1/gib.wav"), 1, ATTN_NORM, 0);
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
    self->monsterinfo.currentmove = &ss_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}


void ss_prepare_fire (edict_t *self)
{
	int		n;

	n = (rand() & 15) + 3 + 7;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

void ss_fire (edict_t *self)
{
	ssMachineGun (self);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

}

mframe_t ss_frames_attack [] =
{
	ai_charge, 2,  NULL,
	ai_charge, -2, ss_prepare_fire,
	ai_charge, 1,  NULL,
	ai_charge, 1,  ss_fire,
	ai_charge, 5,  NULL
};
mmove_t ss_move_attack = {FRAME_attack01, FRAME_attack05, ss_frames_attack, ss_run};

void ss_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &ss_move_attack;
}


/*QUAKED monster_idg1_ss (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_idg1_ss (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 11;
		SP_monster_idg2_commando (self);
		return;
	}

	sound_pain = gi.soundindex ("idg1monster/ss/pain.wav");
	sound_die = gi.soundindex ("idg1monster/ss/death.wav");	
	sound_sight = gi.soundindex ("idg1monster/ss/sight.wav");
	sound_attack = gi.soundindex ("idg1weapons/mgshot.wav");


	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg1/ss/tris.md2");
	VectorSet (self->mins, -16, -16, -35);
	VectorSet (self->maxs, 16, 16, 30);

	self->health = 100;

	self->gib_health = -40;
	self->mass = 200;

	self->pain = ss_pain;
	self->die = ss_die;

	self->monsterinfo.stand = ss_stand;
	self->monsterinfo.walk = ss_walk;
	self->monsterinfo.run = ss_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = ss_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = ss_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &ss_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
