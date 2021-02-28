/*
==============================================================================

TANK

==============================================================================
*/

#include "g_local.h"
#include "m_tank.h"


void bosstank_refire_rocket (edict_t *self);
void bosstank_doattack_rocket (edict_t *self);
void bosstank_reattack_blaster (edict_t *self);

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

void bosstank_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


void bosstank_footstep (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
}

void bosstank_thud (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

void bosstank_windup (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_windup, 1, ATTN_NORM, 0);
}

void bosstank_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}


//
// stand
//

mframe_t bosstank_frames_stand []=
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
mmove_t	bosstank_move_stand = {FRAME_stand01, FRAME_stand30, bosstank_frames_stand, NULL};
	
void bosstank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &bosstank_move_stand;
}


//
// walk
//

void bosstank_walk (edict_t *self);

mframe_t bosstank_frames_start_walk [] =
{
	ai_walk,  15, NULL,
	ai_walk,  15, NULL,
	ai_walk,  15, NULL,
	ai_walk, 15, bosstank_footstep
};
mmove_t	bosstank_move_start_walk = {FRAME_walk01, FRAME_walk04, bosstank_frames_start_walk, bosstank_walk};

mframe_t bosstank_frames_walk [] =
{
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	bosstank_footstep,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	NULL,
	ai_walk, 15,	bosstank_footstep
};
mmove_t	bosstank_move_walk = {FRAME_walk05, FRAME_walk20, bosstank_frames_walk, NULL};

mframe_t bosstank_frames_stop_walk [] =
{
	ai_walk,  15, NULL,
	ai_walk,  15, NULL,
	ai_walk,  15, NULL,
	ai_walk,  15, NULL,
	ai_walk,  15, bosstank_footstep
};
mmove_t	bosstank_move_stop_walk = {FRAME_walk21, FRAME_walk25, bosstank_frames_stop_walk, bosstank_stand};

void bosstank_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &bosstank_move_walk;
}


//
// run
//

void bosstank_run (edict_t *self);

mframe_t bosstank_frames_start_run [] =
{
	ai_run,  15, NULL,
	ai_run,  15, NULL,
	ai_run,  15, NULL,
	ai_run, 15, bosstank_footstep
};
mmove_t	bosstank_move_start_run = {FRAME_walk01, FRAME_walk04, bosstank_frames_start_run, bosstank_run};

mframe_t bosstank_frames_run [] =
{
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	bosstank_footstep,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	NULL,
	ai_run, 15,	bosstank_footstep
};
mmove_t	bosstank_move_run = {FRAME_walk05, FRAME_walk20, bosstank_frames_run, NULL};

mframe_t bosstank_frames_stop_run [] =
{
	ai_run,  15, NULL,
	ai_run,  15, NULL,
	ai_run,  15, NULL,
	ai_run,  15, NULL,
	ai_run,  15, bosstank_footstep
};
mmove_t	bosstank_move_stop_run = {FRAME_walk21, FRAME_walk25, bosstank_frames_stop_run, bosstank_walk};

void bosstank_run (edict_t *self)
{
	if (self->enemy && self->enemy->client)
		self->monsterinfo.aiflags |= AI_BRUTAL;
	else
		self->monsterinfo.aiflags &= ~AI_BRUTAL;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &bosstank_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == &bosstank_move_walk ||
		self->monsterinfo.currentmove == &bosstank_move_start_run)
	{
		self->monsterinfo.currentmove = &bosstank_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &bosstank_move_start_run;
	}
}

//
// pain
//

mframe_t bosstank_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t bosstank_move_pain1 = {FRAME_pain101, FRAME_pain104, bosstank_frames_pain1, bosstank_run};

mframe_t bosstank_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t bosstank_move_pain2 = {FRAME_pain201, FRAME_pain205, bosstank_frames_pain2, bosstank_run};

mframe_t bosstank_frames_pain3 [] =
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
	ai_move, 0,  bosstank_footstep
};
mmove_t	bosstank_move_pain3 = {FRAME_pain301, FRAME_pain316, bosstank_frames_pain3, bosstank_run};


void bosstank_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 30)
		self->monsterinfo.currentmove = &bosstank_move_pain1;
	else if (damage <= 60)
		self->monsterinfo.currentmove = &bosstank_move_pain2;
	else
		self->monsterinfo.currentmove = &bosstank_move_pain3;
};


//
// attacks
//

void BossTankBlaster (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	int		flash_number;

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else // (self->s.frame == FRAME_attak116)
		flash_number = MZ2_TANK_BLASTER_3;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, 45, 1500, flash_number, EF_BLASTER);
}	

void BossTankStrike (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_strike, 1, ATTN_NORM, 0);
}	

void BossBossTankStrikeBoom (edict_t *self)
{
	BossTankStrike(self);

	T_RadiusDamage (self, self, 600, self, 1600, MOD_UNKNOWN);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);
}

void BossTankRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;

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

	monster_fire_rocket (self, start, dir, 125, 1200, flash_number);
}	

void BossTankMachineGun (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;

	flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak406);

	dir[0] = self->s.angles[0];
	dir[1] = self->s.angles[1];
	dir[2] = self->s.angles[2];

	AngleVectors (dir, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

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

	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		VectorMA (vec, 0, self->enemy->velocity, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, forward);
		VectorNormalize (forward);
	}


	monster_fire_bullet (self, start, forward, 25, 20, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	


mframe_t bosstank_frames_attack_blast [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -1,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -1,	NULL,
	ai_charge, -1,	NULL,
	ai_charge, 0,	BossTankBlaster,
	ai_charge, 0,	BossTankBlaster,		// 10
	ai_charge, 0,	NULL,
	ai_charge, 0,	BossTankBlaster,
	ai_charge, 0,	BossTankBlaster,
	ai_charge, 0,	NULL,
	ai_charge, 0,	BossTankBlaster,
	ai_charge, 0,	BossTankBlaster			// 16
};
mmove_t bosstank_move_attack_blast = {FRAME_attak101, FRAME_attak116, bosstank_frames_attack_blast, bosstank_reattack_blaster};

mframe_t bosstank_frames_reattack_blast [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	BossTankBlaster,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	BossTankBlaster			// 16
};
mmove_t bosstank_move_reattack_blast = {FRAME_attak111, FRAME_attak116, bosstank_frames_reattack_blast, bosstank_reattack_blaster};

mframe_t bosstank_frames_attack_post_blast [] =	
{
	ai_move, 0,		NULL,				// 17
	ai_move, 0,		NULL,
	ai_move, 2,		NULL,
	ai_move, 3,		NULL,
	ai_move, 2,		NULL,
	ai_move, -2,	bosstank_footstep		// 22
};
mmove_t bosstank_move_attack_post_blast = {FRAME_attak117, FRAME_attak122, bosstank_frames_attack_post_blast, bosstank_run};

void bosstank_reattack_blaster (edict_t *self)
{
	if (skill->value >= 2)
		if (visible (self, self->enemy))
			if (self->enemy->health > 0)
				if (random() <= 0.6)
				{
					self->monsterinfo.currentmove = &bosstank_move_reattack_blast;
					return;
				}
	self->monsterinfo.currentmove = &bosstank_move_attack_post_blast;
}
void BossBossTankStrikeRefire (edict_t *self);

mframe_t bosstank_frames_attack3 [] =
{
	ai_move, 0,   bosstank_windup,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   BossBossTankStrikeBoom,
	ai_move, 0,   NULL,
	ai_move, -1,  BossBossTankStrikeRefire,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -3,  NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -2,  NULL,
	ai_move, -3,  NULL,
	ai_move, -2,  bosstank_footstep
};
mmove_t bosstank_move_attack3 = {FRAME_attak219, FRAME_attak238, bosstank_frames_attack3, bosstank_run};


void BossBossTankStrikeRefire (edict_t *self)
{
		if (visible (self, self->enemy))
			if (self->enemy->health > 0)
				if (random() <= 0.6)
				{
					if (range (self, self->enemy) <= RANGE_NEAR)
					{
						self->s.frame = 95;
						return;
					}
				}
	self->monsterinfo.currentmove = &bosstank_move_start_run;
}


void bosstank_poststrike (edict_t *self)
{
	self->enemy = NULL;
	bosstank_run (self);
}

mframe_t bosstank_frames_attack_strike [] =
{
	ai_move, 3,   NULL,
	ai_move, 2,   NULL,
	ai_move, 2,   NULL,
	ai_move, 1,   NULL,
	ai_move, 6,   NULL,
	ai_move, 7,   NULL,
	ai_move, 9,   bosstank_footstep,
	ai_move, 2,   NULL,
	ai_move, 1,   NULL,
	ai_move, 2,   NULL,
	ai_move, 2,   bosstank_footstep,
	ai_move, 2,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -2,  NULL,
	ai_move, -2,  NULL,
	ai_move, 0,   bosstank_windup,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   BossTankStrike,
	ai_move, 0,   NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, -3,  NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -2,  NULL,
	ai_move, -3,  NULL,
	ai_move, -2,  bosstank_footstep
};
mmove_t bosstank_move_attack_strike = {FRAME_attak201, FRAME_attak238, bosstank_frames_attack_strike, bosstank_poststrike};

void bosstank_GoToFireRocket (edict_t *self)
{
	self->s.frame = 136;
}

mframe_t bosstank_frames_attack_pre_rocket [] =
{
/*	ai_charge, 0,  bosstank_GoToFireRocket,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 10

	ai_charge, 0,  NULL,
	ai_charge, 1,  NULL,
	ai_charge, 2,  NULL,
	ai_charge, 7,  NULL,
	ai_charge, 7,  NULL,
	ai_charge, 7,  bosstank_footstep,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,*/
	ai_charge, 0,  NULL,			// 20

	ai_charge, -3, NULL
};
mmove_t bosstank_move_attack_pre_rocket = {135, FRAME_attak321, bosstank_frames_attack_pre_rocket, bosstank_doattack_rocket};

mframe_t bosstank_frames_attack_fire_rocket [] =
{
	ai_charge, -3, NULL,			// Loop Start	22 
	ai_charge, 0,  NULL,
	ai_charge, 0,  BossTankRocket,		// 24
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  BossTankRocket,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, -1, BossTankRocket		// 30	Loop End
};
mmove_t bosstank_move_attack_fire_rocket = {FRAME_attak322, FRAME_attak330, bosstank_frames_attack_fire_rocket, bosstank_refire_rocket};

mframe_t bosstank_frames_attack_post_rocket [] =
{	
	ai_charge, 0,  NULL,			// 31
	ai_charge, -1, NULL,
	ai_charge, -1, NULL,
	ai_charge, 0,  NULL,
	ai_charge, 2,  NULL,
	ai_charge, 3,  NULL,
	ai_charge, 4,  NULL,
	ai_charge, 2,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 40

	ai_charge, 0,  NULL,
	ai_charge, -9, NULL,
	ai_charge, -8, NULL,
	ai_charge, -7, NULL,
	ai_charge, -1, NULL,
	ai_charge, -1, bosstank_footstep,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 50

	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t bosstank_move_attack_post_rocket = {FRAME_attak331, FRAME_attak353, bosstank_frames_attack_post_rocket, bosstank_run};

mframe_t bosstank_frames_attack_chain [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge,      0, BossTankMachineGun,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t bosstank_move_attack_chain = {FRAME_attak401, FRAME_attak429, bosstank_frames_attack_chain, bosstank_run};

void bosstank_refire_rocket (edict_t *self)
{
	// Only on hard or nightmare
	if ( skill->value >= 2 )
		if (self->enemy->health > 0)
			if (visible(self, self->enemy) )
				if (random() <= 0.4)
				{
					self->monsterinfo.currentmove = &bosstank_move_attack_fire_rocket;
					return;
				}
	self->monsterinfo.currentmove = &bosstank_move_attack_post_rocket;
}

void bosstank_doattack_rocket (edict_t *self)
{
	self->monsterinfo.currentmove = &bosstank_move_attack_fire_rocket;
}

void bosstank_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	float	r;

	if (self->enemy->health < 0)
	{
		self->enemy = NULL;
		bosstank_stand (self);
		return;
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	r = random();

	if (range <= 125)
	{
		if (r < 0.4)
			self->monsterinfo.currentmove = &bosstank_move_attack_chain;
		else if (r < 0.7)
			self->monsterinfo.currentmove = &bosstank_move_attack3;
		else 
			self->monsterinfo.currentmove = &bosstank_move_attack_blast;
	}
	else if (range <= 250)
	{
		if (r < 0.5)
			self->monsterinfo.currentmove = &bosstank_move_attack_chain;
		else if (r < 0.6)
			self->monsterinfo.currentmove = &bosstank_move_attack3;
		else
			self->monsterinfo.currentmove = &bosstank_move_attack_blast;
	}
	else if (range >= 700)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BOSSTPORT);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		_VectorCopy (self->enemy->s.origin, self->s.origin);

		self->s.origin[0] += 45;
	}
	else
	{
		if (r < 0.33)
			self->monsterinfo.currentmove = &bosstank_move_attack_chain;
		else if (r < 0.66)
			self->monsterinfo.currentmove = &bosstank_move_attack_fire_rocket;
		else
			self->monsterinfo.currentmove = &bosstank_move_attack_blast;
	}
}


//
// death
//

void bosstank_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, -0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t bosstank_frames_death1 [] =
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
	ai_move, -15, bosstank_thud,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t	bosstank_move_death = {FRAME_death101, FRAME_death132, bosstank_frames_death1, bosstank_dead};

void bosstank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
/*	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2");
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2");
		ThrowGib (self, "models/objects/gibs/chest/tris.md2");
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

	self->monsterinfo.currentmove = &bosstank_move_death;*/
	G_FreeEdict(self);
	if (((int)(stroggflags->value) & SF_RPG_MODE) && attacker->client)
	{
		AddExp(attacker, rndnum (3000, 6000));
		Check_Levelup(attacker);
	}
	
}

/*QUAKED monster_boss_tank (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_boss_tank (edict_t *self)
{
	/*if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

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
	self->s.effects |= EF_DOUBLE;

	self->health = 32000;
	self->gib_health = -600;

	self->mass = 1500;

	self->pain = bosstank_pain;
	self->die = bosstank_die;
	self->ismonster = 1;
	self->monsterinfo.stand = bosstank_stand;
	self->monsterinfo.walk = bosstank_walk;
	self->monsterinfo.run = bosstank_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = bosstank_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = bosstank_sight;
	self->monsterinfo.idle = bosstank_idle;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &bosstank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);

	self->s.skinnum = 2;
}