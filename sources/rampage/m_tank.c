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

TANK

==============================================================================
*/

#include "g_local.h"
#include "m_tank.h"


void tank_refire_rocket (edict_t *self);
void tank_doattack_rocket (edict_t *self);
void tank_reattack_blaster (edict_t *self);

static int	sound_thud;
static int	sound_pain;
static int	sound_idle;
static int	sound_die;
static int	sound_step;
static int	sound_sight;
static int	sound_windup;
static int	sound_strike;

//
// misc
//

void tank_sight (edict_t *self, edict_t *other)
{
	gi.sound(self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


void tank_footstep (edict_t *self)
{
	gi.sound(self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
}

void tank_thud (edict_t *self)
{
	gi.sound(self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

void tank_windup (edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_windup, 1, ATTN_NORM, 0);
}

void tank_idle (edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}


//
// stand
//

mframe_t tank_frames_stand []=
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
mmove_t	tank_move_stand = {FRAME_stand01, FRAME_stand30, tank_frames_stand, NULL};
	
void tank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &tank_move_stand;
}


//
// walk
//

void tank_walk (edict_t *self);

mframe_t tank_frames_start_walk [] =
{
	ai_walk,  0, NULL,
	ai_walk,  6, NULL,
	ai_walk,  6, NULL,
	ai_walk, 11, tank_footstep
};
mmove_t	tank_move_start_walk = {FRAME_walk01, FRAME_walk04, tank_frames_start_walk, tank_walk};

mframe_t tank_frames_walk [] =
{
	ai_walk, 4,	NULL,
	ai_walk, 5,	NULL,
	ai_walk, 3,	NULL,
	ai_walk, 2,	NULL,
	ai_walk, 5,	NULL,
	ai_walk, 5,	NULL,
	ai_walk, 4,	NULL,
	ai_walk, 4,	tank_footstep,
	ai_walk, 3,	NULL,
	ai_walk, 5,	NULL,
	ai_walk, 4,	NULL,
	ai_walk, 5,	NULL,
	ai_walk, 7,	NULL,
	ai_walk, 7,	NULL,
	ai_walk, 6,	NULL,
	ai_walk, 6,	tank_footstep
};
mmove_t	tank_move_walk = {FRAME_walk05, FRAME_walk20, tank_frames_walk, NULL};

mframe_t tank_frames_stop_walk [] =
{
	ai_walk,  3, NULL,
	ai_walk,  3, NULL,
	ai_walk,  2, NULL,
	ai_walk,  2, NULL,
	ai_walk,  4, tank_footstep
};
mmove_t	tank_move_stop_walk = {FRAME_walk21, FRAME_walk25, tank_frames_stop_walk, tank_stand};

void tank_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &tank_move_walk;
}


//
// run
//

void tank_run (edict_t *self);

mframe_t tank_frames_start_run [] =
{
	ai_run,  0, NULL,
	ai_run,  6, NULL,
	ai_run,  6, NULL,
	ai_run, 11, tank_footstep
};
mmove_t	tank_move_start_run = {FRAME_walk01, FRAME_walk04, tank_frames_start_run, tank_run};

mframe_t tank_frames_run [] =
{
	ai_run, 4,	NULL,
	ai_run, 5,	NULL,
	ai_run, 3,	NULL,
	ai_run, 2,	NULL,
	ai_run, 5,	NULL,
	ai_run, 5,	NULL,
	ai_run, 4,	NULL,
	ai_run, 4,	tank_footstep,
	ai_run, 3,	NULL,
	ai_run, 5,	NULL,
	ai_run, 4,	NULL,
	ai_run, 5,	NULL,
	ai_run, 7,	NULL,
	ai_run, 7,	NULL,
	ai_run, 6,	NULL,
	ai_run, 6,	tank_footstep
};
mmove_t	tank_move_run = {FRAME_walk05, FRAME_walk20, tank_frames_run, NULL};

mframe_t tank_frames_stop_run [] =
{
	ai_run,  3, NULL,
	ai_run,  3, NULL,
	ai_run,  2, NULL,
	ai_run,  2, NULL,
	ai_run,  4, tank_footstep
};
mmove_t	tank_move_stop_run = {FRAME_walk21, FRAME_walk25, tank_frames_stop_run, tank_walk};

void tank_run (edict_t *self)
{
	if (self->enemy && self->enemy->client)
		self->monsterinfo.aiflags |= AI_BRUTAL;
	else
		self->monsterinfo.aiflags &= ~AI_BRUTAL;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &tank_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == &tank_move_walk ||
		self->monsterinfo.currentmove == &tank_move_start_run)
	{
		self->monsterinfo.currentmove = &tank_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &tank_move_start_run;
	}
}

//
// pain
//

mframe_t tank_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t tank_move_pain1 = {FRAME_pain101, FRAME_pain104, tank_frames_pain1, tank_run};

mframe_t tank_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t tank_move_pain2 = {FRAME_pain201, FRAME_pain205, tank_frames_pain2, tank_run};

mframe_t tank_frames_pain3 [] =
{
	ai_move, -7, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 2,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 3,  NULL,
	ai_move, 0,  NULL,
	ai_move, 2,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  tank_footstep
};
mmove_t	tank_move_pain3 = {FRAME_pain301, FRAME_pain316, tank_frames_pain3, tank_run};


void tank_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;

	if (damage <= 10)
		return;

	if (level.time < self->pain_debounce_time)
			return;

	if (damage <= 30)
		if (random() > 0.2)
			return;
	
	// If hard or nightmare, don't go into pain while attacking
	if ( skill->value >= 2)
	{
		if ( (self->s.frame >= FRAME_attak301) && (self->s.frame <= FRAME_attak330) )
			return;
		if ( (self->s.frame >= FRAME_attak101) && (self->s.frame <= FRAME_attak116) )
			return;
	}

	self->pain_debounce_time = level.time + 3;
	gi.sound(self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 30)
		self->monsterinfo.currentmove = &tank_move_pain1;
	else if (damage <= 60)
		self->monsterinfo.currentmove = &tank_move_pain2;
	else
		self->monsterinfo.currentmove = &tank_move_pain3;
};


//
// attacks
//

void TankBlaster (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	int		flash_number;

	int damage = 30;
	if (skill->value > 3)
		damage *= skill->value / 2;

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else  if (self->s.frame == FRAME_attak116)
		flash_number = MZ2_TANK_BLASTER_3;
	else if (self->s.frame >= FRAME_attak501)
		flash_number = MZ2_TANK_BLASTER2_1 + (int)((self->s.frame - FRAME_attak501) / 3);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);
	
	VectorCopy (self->enemy->s.origin, end);
	predict_shot(self, MONSTER_GUARD_BOLT_SPEED, end);

	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, damage, 800, flash_number, EF_BLASTER);
}	

void TankStrike (edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_strike, 1, ATTN_NORM, 0);
}	

void TankRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;

	int damage = 50;
	if (skill->value > 3)
		damage *= skill->value;

	if (self->s.frame == FRAME_attak324)
		flash_number = MZ2_TANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak327)
		flash_number = MZ2_TANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak330)
		flash_number = MZ2_TANK_ROCKET_3;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, damage, 550, flash_number);
}	

void TankMachineGun (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;

	int damage = 20;
	if (skill->value > 3)
		damage *= skill->value * 0.5;

	if(self->s.frame < FRAME_attak501)
		flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak406);
	else
		flash_number = MZ2_TANK_MACHINEGUN2_1 + (self->s.frame - FRAME_attak501);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);
	//gi.bprintf(PRINT_HIGH, "TANK MACHINEGUN: flash offset = %s\n", vtos(monster_flash_offset[flash_number]));
	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, vec);
		vectoangles (vec, vec);
		dir[0] = vec[0];
	}
	else
	{
		dir[0] = 0;
	}

	if (self->s.frame < FRAME_attak501)
	{
		if (self->s.frame <= FRAME_attak415)
			dir[1] = self->s.angles[1] - 8 * (self->s.frame - FRAME_attak411);
		else
			dir[1] = self->s.angles[1] + 8 * (self->s.frame - FRAME_attak419);
		dir[2] = 0;
	}
	else
	{
		dir[0] = vec[0];
		dir[1] = vec[1];
		dir[2] = vec[2];
	}
	

	
	AngleVectors (dir, forward, NULL, NULL);

	monster_fire_bullet (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD*5, DEFAULT_BULLET_VSPREAD * 5, flash_number);
}	


mframe_t tank_frames_attack_blast [] =
{
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	monster_skip_frame,
	ai_charge, -1,	monster_skip_frame,
	ai_charge, -2,	monster_skip_frame,
	ai_charge, -1,	monster_skip_frame,
	ai_charge, -1,	monster_skip_frame,
	ai_charge, 0,	NULL,
	ai_charge, 0,	TankBlaster,		// 10
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	NULL,
	ai_charge, 0,	TankBlaster,
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	NULL,
	ai_charge, 0,	TankBlaster			// 16
};
mmove_t tank_move_attack_blast = {FRAME_attak101, FRAME_attak116, tank_frames_attack_blast, tank_reattack_blaster};

mframe_t tank_frames_reattack_blast [] =
{
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	NULL,
	ai_charge, 0,	TankBlaster,
	ai_charge, 0,	monster_skip_frame,
	ai_charge, 0,	NULL,
	ai_charge, 0,	TankBlaster			// 16
};
mmove_t tank_move_reattack_blast = {FRAME_attak111, FRAME_attak116, tank_frames_reattack_blast, tank_reattack_blaster};

mframe_t tank_frames_attack_post_blast [] =	
{
	ai_move, 0,		monster_skip_frame,				// 17
	ai_move, 0,		monster_skip_frame,
	ai_move, 2,		monster_skip_frame,
	ai_move, 3,		monster_skip_frame,
	ai_move, 2,		NULL,
	ai_move, -2,	tank_footstep		// 22
};
mmove_t tank_move_attack_post_blast = {FRAME_attak117, FRAME_attak122, tank_frames_attack_post_blast, tank_run};

void tank_reattack_blaster (edict_t *self)
{
	if (skill->value >= 2)
		if (visible (self, self->enemy))
			if (self->enemy->health > 0)
				if (random() <= 0.9 && M_CheckClearShot(self))
				{
					self->monsterinfo.currentmove = &tank_move_reattack_blast;
					return;
				}
	self->monsterinfo.currentmove = &tank_move_attack_post_blast;
}


void tank_poststrike (edict_t *self)
{
	self->enemy = NULL;
	tank_run (self);
}

mframe_t tank_frames_attack_strike [] =
{
	ai_move, 3,   monster_skip_frame,
	ai_move, 2,   monster_skip_frame,
	ai_move, 2,   monster_skip_frame,
	ai_move, 1,   monster_skip_frame,
	ai_move, 6,   monster_skip_frame,
	ai_move, 7,   NULL,
	ai_move, 9,   tank_footstep,
	ai_move, 2,   monster_skip_frame,
	ai_move, 1,   monster_skip_frame,
	ai_move, 2,   NULL,
	ai_move, 2,   tank_footstep,
	ai_move, 2,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, -2,  monster_skip_frame,
	ai_move, -2,  NULL,
	ai_move, 0,   tank_windup,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   monster_skip_frame,
	ai_move, 0,   NULL,
	ai_move, 0,   TankStrike,
	ai_move, 0,   monster_skip_frame,
	ai_move, -1,  monster_skip_frame,
	ai_move, -1,  monster_skip_frame,
	ai_move, -1,  monster_skip_frame,
	ai_move, -1,  monster_skip_frame,
	ai_move, -1,  monster_skip_frame,
	ai_move, -3,  monster_skip_frame,
	ai_move, -10, monster_skip_frame,
	ai_move, -10, monster_skip_frame,
	ai_move, -2,  monster_skip_frame,
	ai_move, -3,  NULL,
	ai_move, -2,  tank_footstep
};
mmove_t tank_move_attack_strike = {FRAME_attak201, FRAME_attak238, tank_frames_attack_strike, tank_poststrike};


void tank_attack_all_mb(edict_t *self)
{
	TankMachineGun(self);
	TankBlaster(self);
}
void tank_attack_all_mbf(edict_t *self)
{
	TankMachineGun(self);
	TankBlaster(self);
	tank_footstep(self);
}

void tank_attack_all_m(edict_t *self)
{
	TankMachineGun(self);
}
void tank_attack_all_mf(edict_t *self)
{
	TankMachineGun(self);
	tank_footstep(self);
}

void tank_attack_all_mbr(edict_t *self)
{
	TankMachineGun(self);
	TankBlaster(self);
	TankRocket(self);
}
mframe_t tank_frames_attack_pre_rocket [] =
{
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,			// 10

	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 1,  tank_attack_all_m,
	ai_charge, 2,  tank_attack_all_mb,
	ai_charge, 7,  tank_attack_all_m,
	ai_charge, 7,  tank_attack_all_m,
	ai_charge, 7,  tank_attack_all_mbf,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,			// 20

	ai_charge, -3, tank_attack_all_m,
	NULL, -3, tank_attack_all_mb  //adding a frame fixed the crashing caused by moving from FRAME_attak3** to FRAME_attak5** 
									// ??? wtf ??? setting ai to NULL stopped the monster from moving
};
mmove_t tank_move_attack_pre_rocket = {FRAME_attak501, FRAME_attak521, tank_frames_attack_pre_rocket, tank_doattack_rocket};

mframe_t tank_frames_attack_fire_rocket [] =
{
	ai_charge, -3, tank_attack_all_m,			// Loop Start	22 
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mbr,		// 24 //rocket 
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mbr, //rocket
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, -1, tank_attack_all_mbr		// 30	Loop End // rocket
};
mmove_t tank_move_attack_fire_rocket = {FRAME_attak522, FRAME_attak530, tank_frames_attack_fire_rocket, tank_refire_rocket};

mframe_t tank_frames_attack_post_rocket [] =
{	
	ai_charge, 0,  tank_attack_all_m,			// 31
	ai_charge, -1, tank_attack_all_m,
	ai_charge, -1, tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 2,  tank_attack_all_m,
	ai_charge, 3,  tank_attack_all_mb,
	ai_charge, 4,  tank_attack_all_m,
	ai_charge, 2,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,			// 40

	ai_charge, 0,  tank_attack_all_m,
	ai_charge, -9, tank_attack_all_mb,
	ai_charge, -8, tank_attack_all_m,
	ai_charge, -7, tank_attack_all_m,
	ai_charge, -1, tank_attack_all_mb,
	ai_charge, -1, tank_attack_all_mf,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  tank_attack_all_m,			// 50

	ai_charge, 0,  tank_attack_all_mb,
	ai_charge, 0,  tank_attack_all_m,
	ai_charge, 0,  NULL
};
mmove_t tank_move_attack_post_rocket = {FRAME_attak531, FRAME_attak553, tank_frames_attack_post_rocket, tank_run};

mframe_t tank_frames_attack_chain [] =
{
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, NULL,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	NULL,      0, TankMachineGun,
	ai_charge, 0, NULL,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, monster_skip_frame,
	ai_charge, 0, NULL
};
mmove_t tank_move_attack_chain = {FRAME_attak401, FRAME_attak429, tank_frames_attack_chain, tank_run};

void tank_refire_rocket (edict_t *self)
{

	// Only on hard or nightmare
	if ( skill->value >= 2 )
		if (self->enemy->health > 0)
			if (visible(self, self->enemy) )
				if (random() <= 0.4 && M_CheckClearShot(self))
				{
					self->monsterinfo.currentmove = &tank_move_attack_fire_rocket;
					return;
				}
	self->monsterinfo.currentmove = &tank_move_attack_post_rocket;
}

void tank_doattack_rocket (edict_t *self)
{
	self->monsterinfo.currentmove = &tank_move_attack_fire_rocket;
}

void tank_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	float	r;
	if (!M_CheckClearShot(self))
	{
		self->monsterinfo.currentmove = &tank_move_run;
		self->s.frame = 0;
		self->monsterinfo.nextframe = 0;
		self->monsterinfo.attack_finished = 0;
		return;
	}
	if (self->enemy->health < 0)
	{
		self->monsterinfo.currentmove = &tank_move_attack_strike;
		self->monsterinfo.aiflags &= ~AI_BRUTAL;
		return;
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	r = random();

	
	self->monsterinfo.currentmove = &tank_move_attack_pre_rocket;
	self->pain_debounce_time = level.time + 5.0;	// no pain for a while
	
	if (range <= 125)
	{
		if (r < 0.4)
			self->monsterinfo.currentmove = &tank_move_attack_chain;
		else 
			self->monsterinfo.currentmove = &tank_move_attack_blast;
	}
	else if (range <= 250)
	{
		if (r < 0.5)
			self->monsterinfo.currentmove = &tank_move_attack_chain;
		else
			self->monsterinfo.currentmove = &tank_move_attack_blast;
	}
	else
	{
		if (r < 0.33)
			self->monsterinfo.currentmove = &tank_move_attack_chain;
		else if (r < 0.66)
		{
			self->monsterinfo.currentmove = &tank_move_attack_pre_rocket;
			self->pain_debounce_time = level.time + 5.0;	// no pain for a while
		}
		else
			self->monsterinfo.currentmove = &tank_move_attack_blast;
	}
}


//
// death
//

void tank_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, -7);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEAD;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t tank_frames_death1 [] =
{
	ai_move, -7,  NULL,
	ai_move, -2,  NULL,
	ai_move, -2,  NULL,
	ai_move, 1,   NULL,
	ai_move, 3,   NULL,
	ai_move, 6,   NULL,
	ai_move, 1,   NULL,
	ai_move, 1,   NULL,
	ai_move, 2,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -2,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -3,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -4,  NULL,
	ai_move, -6,  NULL,
	ai_move, -4,  NULL,
	ai_move, -5,  NULL,
	ai_move, -7,  NULL,
	ai_move, -15, tank_thud,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t	tank_move_death = {FRAME_death101, FRAME_death132, tank_frames_death1, tank_dead};

void tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gib_target(self, 100, (GIB_BIG | GIB_MECH), point);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound(self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &tank_move_death;
	
}


//
// monster_tank
//

/*QUAKED monster_tank (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
/*QUAKED monster_tank_commander (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_tank (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	VectorSet (self->mins, -32, -32, -16);
	VectorSet (self->maxs, 32, 32, 72);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	sound_pain = gi.soundindex ("tank/tnkpain2.wav");
	sound_thud = gi.soundindex ("tank/tnkdeth2.wav");
	sound_idle = gi.soundindex ("tank/tnkidle1.wav");
	sound_die = gi.soundindex ("tank/death.wav");
	sound_step = gi.soundindex ("tank/step.wav");
	sound_windup = gi.soundindex ("tank/tnkatck4.wav");
	sound_strike = gi.soundindex ("tank/tnkatck5.wav");
	sound_sight = gi.soundindex ("tank/sight1.wav");

	gi.soundindex ("tank/tnkatck1.wav");
	gi.soundindex ("tank/tnkatk2a.wav");
	gi.soundindex ("tank/tnkatk2b.wav");
	gi.soundindex ("tank/tnkatk2c.wav");
	gi.soundindex ("tank/tnkatk2d.wav");
	gi.soundindex ("tank/tnkatk2e.wav");
	gi.soundindex ("tank/tnkatck3.wav");

	if (strcmp(self->classname, "monster_tank_commander") == 0)
	{
		self->health = 1000;
		self->gib_health = -225;
	}
	else
	{
		self->health = 750;
		self->gib_health = -200;
	}

	self->mass = 500;

	self->pain = tank_pain;
	self->die = tank_die;
	self->monsterinfo.stand = tank_stand;
	self->monsterinfo.walk = tank_walk;
	self->monsterinfo.run = tank_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = tank_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = tank_sight;
	self->monsterinfo.idle = tank_idle;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &tank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);

	if (strcmp(self->classname, "monster_tank_commander") == 0)
		self->s.skinnum = 2;
}
