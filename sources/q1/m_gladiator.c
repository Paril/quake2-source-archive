/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
==============================================================================

GLADIATOR

==============================================================================
*/

#include "g_local.h"
#include "m_gladiator.h"


static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_gun;
static int	sound_cleaver_swing;
static int	sound_cleaver_hit;
static int	sound_cleaver_miss;
static int	sound_idle;
static int	sound_search;
static int	sound_sight;


void gladiator_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void gladiator_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void gladiator_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}

void gladiator_cleaver_swing (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_cleaver_swing, 1, ATTN_NORM, 0);
}

mframe_t gladiator_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t gladiator_move_stand = {FRAME_stand1, FRAME_stand7, gladiator_frames_stand, NULL};

void gladiator_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_stand;
}


mframe_t gladiator_frames_walk [] =
{
	ai_walk, 15, NULL,
	ai_walk, 7,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 12, NULL,
	ai_walk, 8,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 8,  NULL
};
mmove_t gladiator_move_walk = {FRAME_walk1, FRAME_walk16, gladiator_frames_walk, NULL};

void gladiator_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_walk;
}


mframe_t gladiator_frames_run [] =
{
	ai_run, 23,	NULL,
	ai_run, 14,	NULL,
	ai_run, 14,	NULL,
	ai_run, 21,	NULL,
	ai_run, 12,	NULL,
	ai_run, 13,	NULL
};
mmove_t gladiator_move_run = {FRAME_run1, FRAME_run6, gladiator_frames_run, NULL};

void gladiator_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gladiator_move_stand;
	else
		self->monsterinfo.currentmove = &gladiator_move_run;
}


void GaldiatorMelee (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	if (fire_hit (self, aim, (20 + (rand() %5)), 300))
		gi.sound (self, CHAN_AUTO, sound_cleaver_hit, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_cleaver_miss, 1, ATTN_NORM, 0);
}

mframe_t gladiator_frames_attack_melee [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, gladiator_cleaver_swing,
	ai_charge, 0, NULL,
	ai_charge, 0, GaldiatorMelee,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, gladiator_cleaver_swing,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GaldiatorMelee,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gladiator_move_attack_melee = {FRAME_melee1, FRAME_melee17, gladiator_frames_attack_melee, gladiator_run};

void gladiator_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_attack_melee;
}


void GladiatorGun (edict_t *self)
{
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GLADIATOR_RAILGUN_1], forward, right, start);

	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	monster_fire_railgun (self, start, dir, 50, 100, MZ2_GLADIATOR_RAILGUN_1);
}

mframe_t gladiator_frames_attack_gun [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GladiatorGun,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gladiator_move_attack_gun = {FRAME_attack1, FRAME_attack9, gladiator_frames_attack_gun, gladiator_run};

void gladiator_attack(edict_t *self)
{
	float	range;
	vec3_t	v;

	// a small safe zone
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	range = VectorLength(v);
	if (range <= (MELEE_DISTANCE + 32))
		return;

	// charge up the railgun
	gi.sound (self, CHAN_WEAPON, sound_gun, 1, ATTN_NORM, 0);
	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;
	self->monsterinfo.currentmove = &gladiator_move_attack_gun;
}


mframe_t gladiator_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t gladiator_move_pain = {FRAME_pain1, FRAME_pain6, gladiator_frames_pain, gladiator_run};

mframe_t gladiator_frames_pain_air [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t gladiator_move_pain_air = {FRAME_painup1, FRAME_painup7, gladiator_frames_pain_air, gladiator_run};

void gladiator_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
	{
		if ((self->velocity[2] > 100) && (self->monsterinfo.currentmove == &gladiator_move_pain))
			self->monsterinfo.currentmove = &gladiator_move_pain_air;
		return;
	}

	self->pain_debounce_time = level.time + 3;

	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (self->velocity[2] > 100)
		self->monsterinfo.currentmove = &gladiator_move_pain_air;
	else
		self->monsterinfo.currentmove = &gladiator_move_pain;
	
}


void gladiator_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t gladiator_frames_death [] =
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
mmove_t gladiator_move_death = {FRAME_death1, FRAME_death22, gladiator_frames_death, gladiator_dead};

void gladiator_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
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
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &gladiator_move_death;
}


/*QUAKED monster_gladiator (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
*/
void SP_monster_gladiator (edict_t *self)
{
	SP_monster_zombieq1 (self);
}
























/*
==============================================================================

Dog Q1

==============================================================================
*/

//
// SOUNDS
//

void dog_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("dog_q1/dsight.wav"), 1, ATTN_NORM, 0);
}

void dog_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("dog_q1/idle.wav"), 1, ATTN_NORM, 0);
}


//
// STAND
//

mframe_t dog_frames_stand [] =
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
mmove_t dog_move_stand = {69, 77, dog_frames_stand, NULL};

void dog_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &dog_move_stand;
}

//
// WALK
//

void dog_walk (edict_t *self);

mframe_t dog_frames_walk [] =
{
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL,
	ai_walk,	8,		NULL
};
mmove_t dog_move_walk = {78, 85, dog_frames_walk, NULL};

void dog_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &dog_move_walk;
}


//
// RUN
//

mframe_t dog_frames_run [] =
{
	ai_run,	16,		NULL,
	ai_run,	32,		NULL,
	ai_run,	32,		NULL,
	ai_run,	20,		NULL,
	ai_run,	64,		NULL,
	ai_run,	32,		NULL,
	ai_run,	16,		NULL,
	ai_run,	32,		NULL,
	ai_run,	32,		NULL,
	ai_run,	20,		NULL,
	ai_run,	64,		NULL,
	ai_run,	32,		NULL
};
mmove_t dog_move_run = {48, 59, dog_frames_run, NULL};

void dog_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &dog_move_stand;
	else
		self->monsterinfo.currentmove = &dog_move_run;
}


//
// MELEE
//

void dog_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (2 + (rand() %6)), 0))
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("dog_q1/dattack1.wav"), 1, ATTN_NORM, 0);
}

void dog_check_refire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = 0;
}

mframe_t dog_frames_attack [] =
{
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	10,	dog_bite,
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	10,	dog_check_refire
};
mmove_t dog_move_attack = {0, 7, dog_frames_attack, dog_run};

void dog_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &dog_move_attack;
}


//
// ATTACK
//

void dog_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->health <= 0)
	{
		self->touch = NULL;
		return;
	}

	if (other->takedamage)
	{
		if (VectorLength(self->velocity) > 400)
		{
			vec3_t	point;
			vec3_t	normal;
			int		damage;

			VectorCopy (self->velocity, normal);
			VectorNormalize(normal);
			VectorMA (self->s.origin, self->maxs[0], normal, point);
			damage = 10 + 10 * random();
			T_Damage (other, self, self, self->velocity, point, normal, damage, damage, 0, MOD_UNKNOWN);
		}
	}

	if (!M_CheckBottom (self))
	{
		if (self->groundentity)
		{
			self->monsterinfo.nextframe = 66;
			self->touch = NULL;	
			gi.sound (self, CHAN_VOICE, gi.soundindex ("fiend_q1/dland2.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	self->touch = NULL;
}

void dog_jump_takeoff (edict_t *self)
{
	vec3_t	forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);
	self->s.origin[2] += 1;
	VectorScale (forward, 600, self->velocity);
	self->velocity[2] = 250;
	self->groundentity = NULL;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->monsterinfo.attack_finished = level.time + 3;
	self->touch = dog_jump_touch;
}

void dog_check_landing (edict_t *self)
{
	if (self->groundentity)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("fiend_q1/dland2.wav"), 1, ATTN_NORM, 0);
		self->monsterinfo.attack_finished = 0;
		self->monsterinfo.aiflags &= ~AI_DUCKED;
		return;
	}

	if (level.time > self->monsterinfo.attack_finished)
		self->monsterinfo.nextframe = 65;
	else
		self->monsterinfo.nextframe = 66;
}

mframe_t dog_frames_jump [] =
{
	ai_charge,	 0,	NULL,
	ai_charge,	17,	NULL,
	ai_charge,	15,	dog_jump_takeoff,
	ai_charge,	15,	NULL,
	ai_charge,	15,	NULL,
	ai_charge,	 0,	dog_check_landing,
	ai_charge,	 3,	NULL,
	ai_charge,	 0,	NULL,
	ai_charge,	 3,	NULL
};
mmove_t dog_move_jump = {60, 68, dog_frames_jump, dog_run};

void dog_jump (edict_t *self)
{
	self->monsterinfo.currentmove = &dog_move_jump;
}


//
// CHECKATTACK
//

qboolean dog_check_melee (edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
		return true;
	return false;
}

qboolean dog_check_jump (edict_t *self)
{
	vec3_t	v;
	float	distance;

	if (self->absmin[2] > (self->enemy->absmin[2] + 0.75 * self->enemy->size[2]))
		return false;

	if (self->absmax[2] < (self->enemy->absmin[2] + 0.25 * self->enemy->size[2]))
		return false;

	v[0] = self->s.origin[0] - self->enemy->s.origin[0];
	v[1] = self->s.origin[1] - self->enemy->s.origin[1];
	v[2] = 0;
	distance = VectorLength(v);

	if (distance < 100)
		return false;
	if (distance > 100)
	{
		if (random() < 0.9)
			return false;
	}

	return true;
}

qboolean dog_checkattack (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return false;

	if (dog_check_melee(self))
	{
		self->monsterinfo.attack_state = AS_MELEE;
		return true;
	}

	if (dog_check_jump(self))
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		// FIXME play a jump sound here
		return true;
	}

	return false;
}


//
// PAIN
//

mframe_t dog_frames_pain1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	-12,	NULL,
	ai_move,	-12,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-10,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t dog_move_pain1 = {32, 47, dog_frames_pain1, dog_run};

void dog_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 6;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	r = random();
	gi.sound (self, CHAN_VOICE, gi.soundindex ("dog_q1/dpain1.wav"), 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &dog_move_pain1;
}


//
// DEATH
//

void dog_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);

	M_FlyCheck (self);
}

mframe_t dog_frames_death1 [] =
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
mmove_t dog_move_death1 = {17, 25, dog_frames_death1, dog_dead};

void dog_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
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

	gi.sound (self, CHAN_VOICE, gi.soundindex ("dog_q1/ddeath.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->s.skinnum = 1;

	self->monsterinfo.currentmove = &dog_move_death1;
}


//
// SPAWN
//

/*QUAKED monster_dog (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_dog (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/dog_q1/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 40);

	self->health = 25;
	self->gib_health = -120;
	self->mass = 80;

	self->pain = dog_pain;
	self->die = dog_die;

	self->monsterinfo.stand = dog_stand;
	self->monsterinfo.walk = dog_walk;
	self->monsterinfo.run = dog_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = dog_jump;
	self->monsterinfo.melee = dog_melee;
	self->monsterinfo.sight = dog_sight;
	self->monsterinfo.search = dog_search;
	self->monsterinfo.idle = dog_search;
	self->monsterinfo.checkattack = dog_checkattack;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &dog_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}


