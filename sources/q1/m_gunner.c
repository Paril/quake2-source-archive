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

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"

void SP_monster_enforcerq1 (edict_t *self);

static int	sound_pain;
static int	sound_pain2;
static int	sound_death;
static int	sound_idle;
static int	sound_open;
static int	sound_search;
static int	sound_sight;


void gunner_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void gunner_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void gunner_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void GunnerGrenade (edict_t *self);
void GunnerFire (edict_t *self);
void gunner_fire_chain(edict_t *self);
void gunner_refire_chain(edict_t *self);


void gunner_stand (edict_t *self);

mframe_t gunner_frames_fidget [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_idlesound,
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	gunner_move_fidget = {FRAME_stand31, FRAME_stand70, gunner_frames_fidget, gunner_stand};

void gunner_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &gunner_move_fidget;
}

mframe_t gunner_frames_stand [] =
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
	ai_stand, 0, gunner_fidget,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_fidget,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_fidget
};
mmove_t	gunner_move_stand = {FRAME_stand01, FRAME_stand30, gunner_frames_stand, NULL};

void gunner_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &gunner_move_stand;
}


mframe_t gunner_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 2, NULL,
	ai_walk, 6, NULL,
	ai_walk, 4, NULL,
	ai_walk, 2, NULL,
	ai_walk, 7, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 4, NULL
};
mmove_t gunner_move_walk = {FRAME_walk07, FRAME_walk19, gunner_frames_walk, NULL};

void gunner_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_walk;
}

mframe_t gunner_frames_run [] =
{
	ai_run, 26, NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 13, NULL,
	ai_run, 6,  NULL
};

mmove_t gunner_move_run = {FRAME_run01, FRAME_run08, gunner_frames_run, NULL};

void gunner_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gunner_move_stand;
	else
		self->monsterinfo.currentmove = &gunner_move_run;
}

mframe_t gunner_frames_runandshoot [] =
{
	ai_run, 32, NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 18, NULL,
	ai_run, 8,  NULL,
	ai_run, 20, NULL
};

mmove_t gunner_move_runandshoot = {FRAME_runs01, FRAME_runs06, gunner_frames_runandshoot, NULL};

void gunner_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_runandshoot;
}

mframe_t gunner_frames_pain3 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL
};
mmove_t gunner_move_pain3 = {FRAME_pain301, FRAME_pain305, gunner_frames_pain3, gunner_run};

mframe_t gunner_frames_pain2 [] =
{
	ai_move, -2, NULL,
	ai_move, 11, NULL,
	ai_move, 6,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, -1, NULL,
	ai_move, -7, NULL,
	ai_move, -2, NULL,
	ai_move, -7, NULL
};
mmove_t gunner_move_pain2 = {FRAME_pain201, FRAME_pain208, gunner_frames_pain2, gunner_run};

mframe_t gunner_frames_pain1 [] =
{
	ai_move, 2,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -5, NULL,
	ai_move, 3,	 NULL,
	ai_move, -1, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -2, NULL,
	ai_move, -2, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t gunner_move_pain1 = {FRAME_pain101, FRAME_pain118, gunner_frames_pain1, gunner_run};

void gunner_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand()&1)
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = &gunner_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = &gunner_move_pain2;
	else
		self->monsterinfo.currentmove = &gunner_move_pain1;
}

void gunner_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t gunner_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -7, NULL,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t gunner_move_death = {FRAME_death01, FRAME_death11, gunner_frames_death, gunner_dead};

void gunner_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &gunner_move_death;
}


void gunner_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	if (skill->value >= 2)
	{
		if (random() > 0.5)
			GunnerGrenade (self);
	}

	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void gunner_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void gunner_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t gunner_frames_duck [] =
{
	ai_move, 1,  gunner_duck_down,
	ai_move, 1,  NULL,
	ai_move, 1,  gunner_duck_hold,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, 0,  gunner_duck_up,
	ai_move, -1, NULL
};
mmove_t	gunner_move_duck = {FRAME_duck01, FRAME_duck08, gunner_frames_duck, gunner_run};

void gunner_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &gunner_move_duck;
}


void gunner_opengun (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_open, 1, ATTN_IDLE, 0);
}

void GunnerFire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int		flash_number;

	flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_bullet (self, start, aim, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

void GunnerGrenade (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	aim;
	int		flash_number;

	if (self->s.frame == FRAME_attak105)
		flash_number = MZ2_GUNNER_GRENADE_1;
	else if (self->s.frame == FRAME_attak108)
		flash_number = MZ2_GUNNER_GRENADE_2;
	else if (self->s.frame == FRAME_attak111)
		flash_number = MZ2_GUNNER_GRENADE_3;
	else // (self->s.frame == FRAME_attak114)
		flash_number = MZ2_GUNNER_GRENADE_4;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	//FIXME : do a spread -225 -75 75 225 degrees around forward
	VectorCopy (forward, aim);

	monster_fire_grenade (self, start, aim, 50, 600, flash_number);
}

mframe_t gunner_frames_attack_chain [] =
{
	/*
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	*/
	ai_charge, 0, gunner_opengun,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_attack_chain = {FRAME_attak209, FRAME_attak215, gunner_frames_attack_chain, gunner_fire_chain};

mframe_t gunner_frames_fire_chain [] =
{
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire
};
mmove_t gunner_move_fire_chain = {FRAME_attak216, FRAME_attak223, gunner_frames_fire_chain, gunner_refire_chain};

mframe_t gunner_frames_endfire_chain [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_endfire_chain = {FRAME_attak224, FRAME_attak230, gunner_frames_endfire_chain, gunner_run};

mframe_t gunner_frames_attack_grenade [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_attack_grenade = {FRAME_attak101, FRAME_attak121, gunner_frames_attack_grenade, gunner_run};

void gunner_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = &gunner_move_attack_chain;
	}
	else
	{
		if (random() <= 0.5)
			self->monsterinfo.currentmove = &gunner_move_attack_grenade;
		else
			self->monsterinfo.currentmove = &gunner_move_attack_chain;
	}
}

void gunner_fire_chain(edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_fire_chain;
}

void gunner_refire_chain(edict_t *self)
{
	if (self->enemy->health > 0)
		if ( visible (self, self->enemy) )
			if (random() <= 0.5)
			{
				self->monsterinfo.currentmove = &gunner_move_fire_chain;
				return;
			}
	self->monsterinfo.currentmove = &gunner_move_endfire_chain;
}

/*QUAKED monster_gunner (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_gunner (edict_t *self)
{
	SP_monster_enforcerq1 (self);
}


// Soldier Q1 Monster
/*
==============================================================================

Soldier Quake1 Monster

==============================================================================
*/
void soldierq1_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/idle.wav"), 1, ATTN_IDLE, 0);
}

void soldierq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/sight1.wav"), 1, ATTN_IDLE, 0);
}

void soldierq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/idle.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Soldierq1Fire (edict_t *self);
void Soldierq1_fire_chain(edict_t *self);
void Soldierq1_refire_chain(edict_t *self);


void soldierq1_stand (edict_t *self);

mframe_t soldierq1_frames_fidget [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	soldierq1_move_fidget = {0, 7, soldierq1_frames_fidget, soldierq1_stand};

void soldierq1_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &soldierq1_move_fidget;
}

mframe_t soldierq1_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	soldierq1_move_stand = {0, 7, soldierq1_frames_stand, NULL};

void soldierq1_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &soldierq1_move_stand;
}


mframe_t soldierq1_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 1, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 1, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 1, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 1, NULL,
	ai_walk, 0, NULL
};
mmove_t soldierq1_move_walk = {90, 113, soldierq1_frames_walk, NULL};

void soldierq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &soldierq1_move_walk;
}

mframe_t soldierq1_frames_run [] =
{
	ai_run, 26, NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 13, NULL,
	ai_run, 6,  NULL
};

mmove_t soldierq1_move_run = {73, 80, soldierq1_frames_run, NULL};

void soldierq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &soldierq1_move_stand;
	else
		self->monsterinfo.currentmove = &soldierq1_move_run;
}

mframe_t soldierq1_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t soldierq1_move_pain3 = {60, 72, soldierq1_frames_pain3, soldierq1_run};

mframe_t soldierq1_frames_pain2 [] =
{
	ai_move, -2, NULL,
	ai_move, 11, NULL,
	ai_move, 6,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, -1, NULL,
	ai_move, -7, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, -7, NULL,
	ai_move, -2, NULL,
	ai_move, -2, NULL,
	ai_move, -2, NULL,
	ai_move, -2, NULL,
	ai_move, -7, NULL
};
mmove_t soldierq1_move_pain2 = {46, 59, soldierq1_frames_pain2, soldierq1_run};

mframe_t soldierq1_frames_pain1 [] =
{
	ai_move, 2,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -5, NULL,
	ai_move, 3,	 NULL,
	ai_move, 3,	 NULL,
	ai_move, -1, NULL
};
mmove_t soldierq1_move_pain1 = {40, 45, soldierq1_frames_pain1, soldierq1_run};

void soldierq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand()&1)
		gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/pain1.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/pain2.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = &soldierq1_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = &soldierq1_move_pain2;
	else
		self->monsterinfo.currentmove = &soldierq1_move_pain1;
}

void Soldierq1_Dead (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void SoldBack_Touch(edict_t *pack,edict_t *other,cplane_t *plane,csurface_t *surface) 
{
  int i;

  if (!other || !other->client/* || pack->owner==other*/) 
	  return;

	gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/am_pkup.wav"), 1, ATTN_NORM, 0);
	gi.cprintf (other, PRINT_HIGH, "You got 5 shells.\n");

	other->client->pers.inventory[ITEM_INDEX(FindItem ("Shells"))] += 5;

  G_FreeEdict(pack);
}

void G_FreeSoldPack(edict_t *pack) 
{
  G_FreeEdict(pack);
}

void Throw_Soldier_Backpack(edict_t *monster) 
{
	vec3_t forward,up;
	edict_t *pack;
	int i;

	if (gi.pointcontents(monster->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
		return;

	pack = G_Spawn();
	pack->owner = monster;
	pack->s.modelindex=gi.modelindex("models/items/pack/tris.md2");
	pack->s.effects |= EF_ROTATE;
	pack->s.renderfx |= RF_GLOW;
	pack->movetype = MOVETYPE_TOSS;
	pack->solid = SOLID_TRIGGER;
	VectorSet (pack->mins, -16, -16, -16);
	VectorSet (pack->maxs, 16, 16, 16);
	VectorCopy (monster->s.origin, pack->s.origin);
	VectorSet (pack->velocity, 1, 1, 1);
	AngleVectors (monster->s.angles, forward, NULL, up);
	VectorMA (pack->velocity, 100+crandom()*10.0, forward, pack->velocity);
	VectorMA (pack->velocity, 200+crandom()*10.0, up, pack->velocity);

	pack->touch = SoldBack_Touch;

	pack->think = G_FreeSoldPack;
	pack->nextthink = level.time + 15.0;

	gi.linkentity (pack);
}


mframe_t soldierq1_frames_death [] =
{
	ai_move, 0,	 nogib,
	ai_move, 0,	 NULL,
	ai_move, 0,	 Throw_Soldier_Backpack,
	ai_move, -7, NULL,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t soldierq1_move_death = {18, 28, soldierq1_frames_death, Soldierq1_Dead};

void soldierq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/death1.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &soldierq1_move_death;
}

static int shotgun_flash [] = {MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_SHOTGUN_8};

void Soldierq1Fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int flash_index;
	int flash_number;
	flash_number = 0;

	flash_index = shotgun_flash[flash_number];

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_shotgun (self, start, aim, 2, 3, 100, 100, 12, flash_index);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/sattck1.wav"), 1, ATTN_IDLE, 0);
}

void Soldierq1LoadIn (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/lock4.wav"), 1, ATTN_IDLE, 0);
}


void soldierq1_whichway (edict_t *self);

mframe_t soldierq1_frames_fire_chain [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Soldierq1Fire,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL
};
mmove_t soldierq1_move_fire_chain = {81, 89, soldierq1_frames_fire_chain, soldierq1_whichway};

mframe_t soldierq1_frames_fire_reload [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, Soldierq1LoadIn,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	soldierq1_move_fire_reload = {29, 39, soldierq1_frames_fire_reload, soldierq1_run};

void soldierq1_whichway (edict_t *self)
{
	if (self->enemy->health > 0)
		if ( visible (self, self->enemy) )
			if (random() <= 0.5)
			{
				self->monsterinfo.currentmove = &soldierq1_move_fire_chain;
				return;
			}
	self->monsterinfo.currentmove = &soldierq1_move_fire_reload;
}

void soldierq1_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &soldierq1_move_fire_chain;
}

/*QUAKED monster_soldierq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldierq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/sold_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 30;
	self->gib_health = -70;
	self->mass = 200;
	self->reload_shots = 3;

	self->pain = soldierq1_pain;
	self->die = soldierq1_die;

	self->monsterinfo.stand = soldierq1_stand;
	self->monsterinfo.walk = soldierq1_walk;
	self->monsterinfo.run = soldierq1_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = soldierq1_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = soldierq1_sight;
	self->monsterinfo.search = soldierq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &soldierq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}

/*
======================================
======================================

Orge Quake1

======================================
======================================
*/
void ogreq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("ogre_q1/ogwake.wav"), 1, ATTN_IDLE, 0);
}

void ogreq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("ogre_q1/ogidle.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Ogreq1Fire (edict_t *self);
void Ogreq1_fire_chain(edict_t *self);
void Ogreq1_refire_chain(edict_t *self);


void ogreq1_stand (edict_t *self);

mframe_t ogreq1_frames_fidget [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	ogreq1_move_fidget = {0, 8, ogreq1_frames_fidget, ogreq1_stand};

void ogreq1_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &ogreq1_move_fidget;
}

mframe_t ogreq1_frames_stand [] =
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
mmove_t	ogreq1_move_stand = {0, 8, ogreq1_frames_stand, NULL};

void ogreq1_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &ogreq1_move_stand;
}


mframe_t ogreq1_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 2, NULL,
	ai_walk, 6, NULL,
	ai_walk, 4, NULL,
	ai_walk, 2, NULL,
	ai_walk, 7, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL
};
mmove_t ogreq1_move_walk = {9, 24, ogreq1_frames_walk, NULL};

void ogreq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &ogreq1_move_walk;
}

mframe_t ogreq1_frames_run [] =
{
	ai_run, 13, NULL,
	ai_run, 5,  NULL,
	ai_run, 3,  NULL,
	ai_run, 4,  NULL,
	ai_run, 9, NULL,
	ai_run, 8, NULL,
	ai_run, 7, NULL,
	ai_run, 10,  NULL
};

mmove_t ogreq1_move_run = {25, 32, ogreq1_frames_run, NULL};

void ogreq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &ogreq1_move_stand;
	else
		self->monsterinfo.currentmove = &ogreq1_move_run;
}

mframe_t ogreq1_frames_pain5 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogreq1_move_pain5 = {97, 111, ogreq1_frames_pain5, ogreq1_run};

mframe_t ogreq1_frames_pain4 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogreq1_move_pain4 = {81, 96, ogreq1_frames_pain4, ogreq1_run};

mframe_t ogreq1_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL
};
mmove_t ogreq1_move_pain3 = {67, 71, ogreq1_frames_pain3, ogreq1_run};

mframe_t ogreq1_frames_pain2 [] =
{
	ai_move, -2, NULL,
	ai_move, 11, NULL,
	ai_move, 6,	 NULL
};
mmove_t ogreq1_move_pain2 = {72, 74, ogreq1_frames_pain2, ogreq1_run};

mframe_t ogreq1_frames_pain1 [] =
{
	ai_move, 2,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -5, NULL,
	ai_move, 3,	 NULL,
	ai_move, 3,	 NULL,
	ai_move, -1, NULL
};
mmove_t ogreq1_move_pain1 = {75, 80, ogreq1_frames_pain1, ogreq1_run};

void ogreq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("ogre_q1/ogpain1.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = &ogreq1_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = &ogreq1_move_pain2;
	else if (damage <= 45)
		self->monsterinfo.currentmove = &ogreq1_move_pain4;
	else if (damage <= 60)
		self->monsterinfo.currentmove = &ogreq1_move_pain5;
	else
		self->monsterinfo.currentmove = &ogreq1_move_pain1;
}

void ogreq1_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}



void OgrePack_Touch(edict_t *pack,edict_t *other,cplane_t *plane,csurface_t *surface) 
{
  int i;

  if (!other || !other->client/* || pack->owner==other*/) 
	  return;

	gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/am_pkup.wav"), 1, ATTN_NORM, 0);
	gi.cprintf (other, PRINT_HIGH, "You got 2 rockets.\n");

	other->client->pers.inventory[ITEM_INDEX(FindItem ("Rockets"))] += 5;

  G_FreeEdict(pack);
}

void G_FreeOgrePack(edict_t *pack) 
{
  G_FreeEdict(pack);
}

void Throw_Ogre_Backpack(edict_t *monster) 
{
	vec3_t forward,up;
	edict_t *pack;
	int i;

	if (gi.pointcontents(monster->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
		return;

	pack = G_Spawn();
	pack->owner = monster;
	pack->s.modelindex=gi.modelindex("models/items/pack/tris.md2");
	pack->s.effects |= EF_ROTATE;
	pack->s.renderfx |= RF_GLOW;
	pack->movetype = MOVETYPE_TOSS;
	pack->solid = SOLID_TRIGGER;
	VectorSet (pack->mins, -16, -16, -16);
	VectorSet (pack->maxs, 16, 16, 16);
	VectorCopy (monster->s.origin, pack->s.origin);
	VectorSet (pack->velocity, 1, 1, 1);
	AngleVectors (monster->s.angles, forward, NULL, up);
	VectorMA (pack->velocity, 100+crandom()*10.0, forward, pack->velocity);
	VectorMA (pack->velocity, 200+crandom()*10.0, up, pack->velocity);

	pack->touch = OgrePack_Touch;

	pack->think = G_FreeOgrePack;
	pack->nextthink = level.time + 15.0;

	gi.linkentity (pack);
}



mframe_t ogreq1_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 Throw_Ogre_Backpack,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogreq1_move_death = {112, 125, ogreq1_frames_death, ogreq1_dead};

void ogreq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("ogreq1/ogdth.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &ogreq1_move_death;
}

void Ogreq1_FireGrenade (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int flash_index;
	int flash_number;
	flash_number = 0;

	flash_index = shotgun_flash[flash_number];

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	//VectorMA (target, -0.2, self->enemy->velocity, target);
	//target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_grenade (self, start, aim, 25, 400, flash_index);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/grenlf1a.wav"), 1, ATTN_IDLE, 0);
}

void Ogre_ChainsawHit (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	fire_hit (self, aim, (4 + (rand() % 9)), 0);		// Slower attack
}

void Ogreq1_ChainsawSound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("ogre_q1/ogsawatk.wav"), 1, ATTN_IDLE, 0);
}


void ogreq1_whichway (edict_t *self);

mframe_t ogreq1_frames_fire_chain_prefire [] =
{
	ai_charge,   0, NULL
};
mmove_t ogreq1_move_fire_chain_prefire = {33, 33, ogreq1_frames_fire_chain_prefire, ogreq1_whichway};

mframe_t ogreq1_frames_fire_slash [] =
{
	ai_stand, 0, Ogreq1_ChainsawSound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	ogreq1_move_fire_slash = {34, 46, ogreq1_frames_fire_slash, ogreq1_run};

mframe_t ogreq1_frames_fire_smash [] =
{
	ai_stand, 0, Ogreq1_ChainsawSound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, Ogre_ChainsawHit,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	ogreq1_move_fire_smash = {48, 60, ogreq1_frames_fire_smash, ogreq1_run};


mframe_t ogreq1_frames_grenade [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, Ogreq1_FireGrenade,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	ogreq1_move_grenade = {61, 66, ogreq1_frames_grenade, ogreq1_run};



void ogreq1_whichway (edict_t *self)
{
	if (self->enemy->health > 0)
		if ( visible (self, self->enemy) )
			if (random() <= 0.5)
			{
				self->monsterinfo.currentmove = &ogreq1_move_fire_smash;
				return;
			}
	self->monsterinfo.currentmove = &ogreq1_move_fire_slash;
}

void ogreq1_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &ogreq1_move_grenade;
}

void ogreq1_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &ogreq1_move_fire_chain_prefire;
}

/*QUAKED monster_ogreq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_ogreq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/ogre_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 200;
	self->gib_health = -70;
	self->mass = 200;
	self->reload_shots = 3;

	self->pain = ogreq1_pain;
	self->die = ogreq1_die;

	self->monsterinfo.stand = ogreq1_stand;
	self->monsterinfo.walk = ogreq1_walk;
	self->monsterinfo.run = ogreq1_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = ogreq1_attack;
	self->monsterinfo.melee = ogreq1_melee;
	self->monsterinfo.sight = ogreq1_sight;
	self->monsterinfo.search = ogreq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &ogreq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}








































/*
==============================================================================

Enforcer Quake1 Monster

==============================================================================
*/
void enforcerq1_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("enforcer_q1/idle1.wav"), 1, ATTN_IDLE, 0);
}

void enforcerq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/sight3.wav"), 1, ATTN_IDLE, 0);
}

void enforcerq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("soldier_q1/idle1.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Enforcerq1Fire (edict_t *self);
void Enforcerq1_fire_chain(edict_t *self);
void Enforcerq1_refire_chain(edict_t *self);


void enforcerq1_stand (edict_t *self);

mframe_t enforcerq1_frames_fidget [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	enforcerq1_move_fidget = {0, 6, enforcerq1_frames_fidget, enforcerq1_stand};

void enforcerq1_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &enforcerq1_move_fidget;
}

mframe_t enforcerq1_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	enforcerq1_move_stand = {0, 6, enforcerq1_frames_stand, NULL};

void enforcerq1_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &enforcerq1_move_stand;
}


mframe_t enforcerq1_frames_walk [] =
{
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 3, NULL
};
mmove_t enforcerq1_move_walk = {7, 22, enforcerq1_frames_walk, NULL};

void enforcerq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &enforcerq1_move_walk;
}

mframe_t enforcerq1_frames_run [] =
{
	ai_run, 22, NULL,
	ai_run, 8,  NULL,
	ai_run, 7,  NULL,
	ai_run, 8,  NULL,
	ai_run, 13, NULL,
	ai_run, 12, NULL,
	ai_run, 11, NULL,
	ai_run, 7,  NULL
};

mmove_t enforcerq1_move_run = {23, 30, enforcerq1_frames_run, NULL};

void enforcerq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &enforcerq1_move_stand;
	else
		self->monsterinfo.currentmove = &enforcerq1_move_run;
}

mframe_t enforcerq1_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t enforcerq1_move_pain3 = {66, 69, enforcerq1_frames_pain3, enforcerq1_run};

mframe_t enforcerq1_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL
};
mmove_t enforcerq1_move_pain2 = {70, 74, enforcerq1_frames_pain2, enforcerq1_run};

mframe_t enforcerq1_frames_pain1 [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL
};
mmove_t enforcerq1_move_pain1 = {75, 82, enforcerq1_frames_pain1, enforcerq1_run};

void enforcerq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand()&1)
		gi.sound (self, CHAN_VOICE, gi.soundindex ("enforcer_q1/pain1.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, gi.soundindex ("enforcer_q1/pain2.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = &enforcerq1_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = &enforcerq1_move_pain2;
	else
		self->monsterinfo.currentmove = &enforcerq1_move_pain1;
}

void enforcerq1_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void SoldEnforcer_Touch(edict_t *pack,edict_t *other,cplane_t *plane,csurface_t *surface) 
{
  int i;

  if (!other || !other->client/* || pack->owner==other*/) 
	  return;

	gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/am_pkup.wav"), 1, ATTN_NORM, 0);
	gi.cprintf (other, PRINT_HIGH, "You got 5 cells.\n");

	other->client->pers.inventory[ITEM_INDEX(FindItem ("Cells"))] += 5;

  G_FreeEdict(pack);
}

void G_FreeEnforcerPack(edict_t *pack) 
{
  G_FreeEdict(pack);
}

void Throw_Enforcer_Backpack(edict_t *monster) 
{
	vec3_t forward,up;
	edict_t *pack;
	int i;

	if (gi.pointcontents(monster->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
		return;

	pack = G_Spawn();
	pack->owner = monster;
	pack->s.modelindex=gi.modelindex("models/items/pack/tris.md2");
	pack->s.effects |= EF_ROTATE;
	pack->s.renderfx |= RF_GLOW;
	pack->movetype = MOVETYPE_TOSS;
	pack->solid = SOLID_TRIGGER;
	VectorSet (pack->mins, -16, -16, -16);
	VectorSet (pack->maxs, 16, 16, 16);
	VectorCopy (monster->s.origin, pack->s.origin);
	VectorSet (pack->velocity, 1, 1, 1);
	AngleVectors (monster->s.angles, forward, NULL, up);
	VectorMA (pack->velocity, 100+crandom()*10.0, forward, pack->velocity);
	VectorMA (pack->velocity, 200+crandom()*10.0, up, pack->velocity);

	pack->touch = SoldEnforcer_Touch;

	pack->think = G_FreeEnforcerPack;
	pack->nextthink = level.time + 15.0;

	gi.linkentity (pack);
}



mframe_t enforcerq1_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 Throw_Enforcer_Backpack,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t enforcerq1_move_death = {41, 54, enforcerq1_frames_death, enforcerq1_dead};

void enforcerq1_whichway (edict_t *self);

void enforcerq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("enforcer_q1/death1.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &enforcerq1_move_death;
}

void Enforcerq1Fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int flash_index;
	int flash_number;
	flash_number = 0;

	flash_index = shotgun_flash[flash_number];

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	//monster_fire_shotgun (self, start, aim, 2, 3, 100, 100, 12, flash_index);
	monster_fire_blaster (self, start, aim, 8, 600, flash_index, EF_HYPERBLASTER);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("enforcer_q1/enfire.wav"), 1, ATTN_IDLE, 0);
}

void enforcerq1_secondfire (edict_t *self);

mframe_t enforcerq1_frames_start_chain [] =
{
	ai_charge,   0, NULL,
	ai_charge,   1, NULL,
	ai_charge,   1, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL
};
mmove_t enforcerq1_move_start_fire = {31, 35, enforcerq1_frames_start_chain, enforcerq1_whichway};

mframe_t enforcerq1_frames_fire_1 [] =
{
	ai_stand, 0, Enforcerq1Fire,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	enforcerq1_move_fire_1 = {36, 40, enforcerq1_frames_fire_1, enforcerq1_secondfire};

mframe_t enforcerq1_frames_fire_2 [] =
{
	ai_stand, 0, Enforcerq1Fire,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	enforcerq1_move_fire_2 = {36, 40, enforcerq1_frames_fire_2, enforcerq1_run};

void enforcerq1_whichway (edict_t *self)
{
	self->monsterinfo.currentmove = &enforcerq1_move_fire_1;
}

void enforcerq1_secondfire (edict_t *self)
{
	self->monsterinfo.currentmove = &enforcerq1_move_fire_2;
}

void enforcerq1_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &enforcerq1_move_start_fire;
}

/*QUAKED monster_soldierq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_enforcerq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/enforcer_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 80;
	self->gib_health = -70;
	self->mass = 175;

	self->pain = enforcerq1_pain;
	self->die = enforcerq1_die;

	self->monsterinfo.stand = enforcerq1_stand;
	self->monsterinfo.walk = enforcerq1_walk;
	self->monsterinfo.run = enforcerq1_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = enforcerq1_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = enforcerq1_sight;
	self->monsterinfo.search = enforcerq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &enforcerq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}














/*
==============================================================================

Knight Quake1 Monster

==============================================================================
*/
void knightq1_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/idle.wav"), 1, ATTN_IDLE, 0);
}

void knightq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/ksight.wav"), 1, ATTN_IDLE, 0);
}

void knightq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/idle.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Knightq1_Slash (edict_t *self);

void knightq1_stand (edict_t *self);

mframe_t knightq1_frames_stand [] =
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
mmove_t	knightq1_move_stand = {0, 8, knightq1_frames_stand, NULL};

void knightq1_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &knightq1_move_stand;
}


mframe_t knightq1_frames_walk [] =
{
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 3, NULL
};
mmove_t knightq1_move_walk = {53, 66, knightq1_frames_walk, NULL};

void knightq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &knightq1_move_walk;
}

mframe_t knightq1_frames_run [] =
{
	ai_run, 30, NULL,
	ai_run, 23,  NULL,
	ai_run, 10,  NULL,
	ai_run, 8,  NULL,
	ai_run, 14, NULL,
	ai_run, 16, NULL,
	ai_run, 19, NULL,
	ai_run, 21,  NULL
};

mmove_t knightq1_move_run = {9, 16, knightq1_frames_run, NULL};

void knightq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &knightq1_move_stand;
	else
		self->monsterinfo.currentmove = &knightq1_move_run;
}

mframe_t knightq1_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t knightq1_move_pain3 = {28, 30, knightq1_frames_pain3, knightq1_run};

mframe_t knightq1_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t knightq1_move_pain2 = {31, 41, knightq1_frames_pain2, knightq1_run};

void knightq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/khurt.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = &knightq1_move_pain3;
	else //if (damage <= 25)
		self->monsterinfo.currentmove = &knightq1_move_pain2;
	//else
		//self->monsterinfo.currentmove = &enforcerq1_move_pain1;
}

void knightq1_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t knightq1_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t knightq1_move_death = {76, 85, knightq1_frames_death, knightq1_dead};

void knightq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/kdeath.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &knightq1_move_death;
}

void Knightq1_Slash (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	fire_hit (self, aim, (7 + (rand() % 13)), 0);		// Slower attack
}

void Knightq1_Sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("knight_q1/sword2.wav"), 1, ATTN_NORM, 0);
}
void knight_check_refire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = 43;
}

mframe_t knightq1_frames_start_chain [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Knightq1_Sound,
	ai_charge,   0, Knightq1_Slash,
	ai_charge,   0, Knightq1_Slash,
	ai_charge,   0, Knightq1_Slash,
	ai_charge,   0, Knightq1_Slash,
	ai_charge,   0, knight_check_refire
};
mmove_t knightq1_move_start_fire = {43, 52, knightq1_frames_start_chain, knightq1_run};


void knightq1_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &knightq1_move_start_fire;
}

/*QUAKED monster_soldierq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_knightq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/knight_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 175;
	self->gib_health = -70;
	self->mass = 175;

	self->pain = knightq1_pain;
	self->die = knightq1_die;

	self->monsterinfo.stand = knightq1_stand;
	self->monsterinfo.walk = knightq1_walk;
	self->monsterinfo.run = knightq1_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = knightq1_attack;
	self->monsterinfo.sight = knightq1_sight;
	self->monsterinfo.search = knightq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &knightq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}











/*
==============================================================================

Scrag/Wizard Quake1 Monster

==============================================================================
*/
void scragq1_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard/widle2.wav"), 1, ATTN_IDLE, 0);
}

void scragq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard_q1/wsight.wav"), 1, ATTN_IDLE, 0);
}

void scragq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard_q1/widle2.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Scragq1_Shoot (edict_t *self);

void scragq1_stand (edict_t *self);

mframe_t scragq1_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t	scragq1_move_stand = {0, 14, scragq1_frames_stand, NULL};

void scragq1_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &scragq1_move_stand;
}


mframe_t scragq1_frames_walk [] =
{
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL
};
mmove_t scragq1_move_walk = {15, 28, scragq1_frames_walk, NULL};

void scragq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &scragq1_move_walk;
}

mframe_t scragq1_frames_run [] =
{
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL
};

mmove_t scragq1_move_run = {15, 28, scragq1_frames_run, NULL};

void scragq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &scragq1_move_stand;
	else
		self->monsterinfo.currentmove = &scragq1_move_run;
}

mframe_t scragq1_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t scragq1_move_pain3 = {42, 45, scragq1_frames_pain3, scragq1_run};

void scragq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard_q1/wpain.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &scragq1_move_pain3;
}

void scragq1_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t scragq1_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t scragq1_move_death = {46, 53, scragq1_frames_death, scragq1_dead};

void scragq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard_q1/wdeath.wav"), 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &scragq1_move_death;
}

void Scragq1_Fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int flash_index;
	int flash_number;
	flash_number = 0;

	flash_index = shotgun_flash[flash_number];

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_blaster (self, start, aim, 12, 850, flash_index, EF_BLASTER);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("wizard_q1/wattack.wav"), 1, ATTN_IDLE, 0);
}

mframe_t scragq1_frames_start_chain [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Scragq1_Fire,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Scragq1_Fire,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Scragq1_Fire,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Scragq1_Fire,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL
};
mmove_t scragq1_move_start_fire = {29, 41, scragq1_frames_start_chain, scragq1_run};


void scragq1_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &scragq1_move_start_fire;
}

/*QUAKED monster_scragq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_scragq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/scrag_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 80;
	self->gib_health = -70;
	self->mass = 175;

	self->pain = scragq1_pain;
	self->die = scragq1_die;

	self->monsterinfo.stand = scragq1_stand;
	self->monsterinfo.walk = scragq1_walk;
	self->monsterinfo.run = scragq1_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = scragq1_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = scragq1_sight;
	self->monsterinfo.search = scragq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &scragq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}



























/*
==============================================================================

Zombie Quake1 Monster

==============================================================================
*/
void zombieq1_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/z_idle1.wav"), 1, ATTN_IDLE, 0);
}

void zombieq1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/z_idle.wav"), 1, ATTN_IDLE, 0);
}

void zombieq1_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/idle_w2.wav"), 1, ATTN_IDLE, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void Zombieq1_Shoot (edict_t *self);

void zombieq1_stand (edict_t *self);

mframe_t zombieq1_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t	zombieq1_move_stand = {0, 14, zombieq1_frames_stand, NULL};

void zombieq1_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &zombieq1_move_stand;
}


mframe_t zombieq1_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 1, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL
};
mmove_t zombieq1_move_walk = {16, 33, zombieq1_frames_walk, NULL};

void zombieq1_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &zombieq1_move_walk;
}

mframe_t zombieq1_frames_run [] =
{
	ai_run, 0, NULL,
	ai_run, 1, NULL,
	ai_run, 1, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 1, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 1, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL
};

mmove_t zombieq1_move_run = {35, 51, zombieq1_frames_run, NULL};

void zombieq1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &zombieq1_move_stand;
	else
		self->monsterinfo.currentmove = &zombieq1_move_run;
}

mframe_t zombieq1_frames_pain4 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_pain4 = {149, 161, zombieq1_frames_pain4, zombieq1_run};

mframe_t zombieq1_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_pain2 = {103, 130, zombieq1_frames_pain2, zombieq1_run};

mframe_t zombieq1_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_pain1 = {91, 102, zombieq1_frames_pain1, zombieq1_run};

void zombieq1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/z_pain1.wav"), 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (random() < 0.5)
		self->monsterinfo.currentmove = &zombieq1_move_pain1;
	else
		self->monsterinfo.currentmove = &zombieq1_move_pain2;
}

void Zombie_Splat (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/z_fall.wav"), 1, ATTN_NORM, 0);
}
void zombieq1_dead (edict_t *self);

void zombieq1_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t zombieq1_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 Zombie_Splat,
	ai_move, 0,	 zombieq1_duck_hold,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_death = {162, 173, zombieq1_frames_death, zombieq1_dead};

mframe_t zombieq1_frames_revive [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_revive = {174, 191, zombieq1_frames_revive, zombieq1_run}; // 18

void zombieq1_dead (edict_t *self)
{
	self->health = 60;
	self->monsterinfo.currentmove = &zombieq1_move_revive;
	self->movetype = MOVETYPE_STEP;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->takedamage = DAMAGE_YES;
//	VectorSet (self->mins, -16, -16, -24);
//	VectorSet (self->maxs, 16, 16, -8);
//	self->movetype = MOVETYPE_TOSS;
//	self->svflags |= SVF_DEADMONSTER;
//	self->nextthink = 0;
//	gi.linkentity (self);
}
void zombieq1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, gi.soundindex ("zombie_q1/z_pain.wav"), 1, ATTN_NORM, 0);
	//self->deadflag = DEAD_NO;
	//self->takedamage = DAMAGE_NO;
	self->monsterinfo.currentmove = &zombieq1_move_death;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->monsterinfo.pausetime = level.time + 5;
}

void Zombieq1_Fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	aim;
	int		flash_number;

	flash_number = MZ2_GUNNER_GRENADE_4;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (forward, aim);

	monster_throw_gib (self, start, aim, 8, 1000, flash_number, EF_GIB);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/ax1.wav"), 1, ATTN_IDLE, 0);
}

void zombieq1_whichatk (edict_t *self);

mframe_t zombieq1_frames_start_chain [] =
{
	ai_charge,   0, NULL
};
mmove_t zombieq1_move_start_fire = {52, 52, zombieq1_frames_start_chain, zombieq1_whichatk};

mframe_t zombieq1_frames_throw_1 [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Zombieq1_Fire,
	ai_charge,   0, NULL
};
mmove_t zombieq1_move_fire_1 = {53, 64, zombieq1_frames_throw_1, zombieq1_run};

mframe_t zombieq1_frames_throw_2 [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Zombieq1_Fire
};
mmove_t zombieq1_move_fire_2 = {65, 78, zombieq1_frames_throw_2, zombieq1_run};

mframe_t zombieq1_frames_throw_3 [] =
{
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, Zombieq1_Fire
};
mmove_t zombieq1_move_fire_3 = {79, 90, zombieq1_frames_throw_3, zombieq1_run};

void zombieq1_whichatk (edict_t *self)
{
	if (random() < 0.5)
		self->monsterinfo.currentmove = &zombieq1_move_fire_1;
	else if (random() < 1.0)
		self->monsterinfo.currentmove = &zombieq1_move_fire_2;
	else if (random() < 1.5)
		self->monsterinfo.currentmove = &zombieq1_move_fire_3;
}

void zombieq1_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &zombieq1_move_start_fire;
}

void zombie_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;

	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void zombie_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t zombieq1_frames_duck [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 zombie_duck_down,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 zombie_duck_up,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t zombieq1_move_duck = {131, 148, zombieq1_frames_duck, zombieq1_run};

void zombieq1_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &zombieq1_move_duck;
}

/*QUAKED monster_zombieq1 (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_zombieq1 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/zombie_q1/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 60;
	self->gib_health = -70;
	self->mass = 175;

	self->pain = zombieq1_pain;
	self->die = zombieq1_die;

	self->monsterinfo.stand = zombieq1_stand;
	self->monsterinfo.walk = zombieq1_walk;
	self->monsterinfo.run = zombieq1_run;
	self->monsterinfo.dodge = zombieq1_dodge;
	self->monsterinfo.attack = zombieq1_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = zombieq1_sight;
	self->monsterinfo.search = zombieq1_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &zombieq1_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}