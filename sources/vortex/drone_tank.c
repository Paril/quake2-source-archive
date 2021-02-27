/*
==============================================================================

TANK

==============================================================================
*/

#include "g_local.h"
#include "m_tank.h"


void mytank_refire_rocket (edict_t *self);
void mytank_doattack_rocket (edict_t *self);
void mytank_reattack_blaster (edict_t *self);
void mytank_meleeattack (edict_t *self);
void mytank_restrike (edict_t *self);
qboolean TeleportNearTarget (edict_t *self, edict_t *target, float dist);
void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);
void mytank_chain_refire (edict_t *self);
void mytank_attack_chain (edict_t *self);

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

void mytank_footstep (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
}

void mytank_thud (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

void mytank_windup (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_windup, 1, ATTN_NORM, 0);
}

void mytank_idle (edict_t *self)
{
	int		range;
	vec3_t	v;

	//GHz: Commanders teleport back to owner
	if ((self->s.skinnum & 2) && !(self->monsterinfo.aiflags & AI_STAND_GROUND))
	{
		VectorSubtract(self->activator->s.origin, self->s.origin, v);
		range = VectorLength (v);
		if (range > 256)
		{
			TeleportNearTarget (self, self->activator, 16);
		}
	}
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
	self->superspeed = false; //GHz: No more sliding
}


//
// stand
//

mframe_t mytank_frames_stand []=
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t	mytank_move_stand = {FRAME_stand01, FRAME_stand30, mytank_frames_stand, NULL};
	
void mytank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &mytank_move_stand;
}

void tank_walk (edict_t *self);

mframe_t tank_frames_start_walk [] =
{
	drone_ai_walk,  0, NULL,
	drone_ai_walk,  6, NULL,
	drone_ai_walk,  6, NULL,
	drone_ai_walk, 11, mytank_footstep
};
mmove_t	tank_move_start_walk = {FRAME_walk01, FRAME_walk04, tank_frames_start_walk, tank_walk};

mframe_t tank_frames_walk [] =
{
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 5,	NULL,
	drone_ai_walk, 3,	NULL,
	drone_ai_walk, 2,	NULL,
	drone_ai_walk, 5,	NULL,
	drone_ai_walk, 5,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	mytank_footstep,
	drone_ai_walk, 3,	NULL,
	drone_ai_walk, 5,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 5,	NULL,
	drone_ai_walk, 7,	NULL,
	drone_ai_walk, 7,	NULL,
	drone_ai_walk, 6,	NULL,
	drone_ai_walk, 6,	mytank_footstep
};
mmove_t	tank_move_walk = {FRAME_walk05, FRAME_walk20, tank_frames_walk, NULL};

mframe_t tank_frames_stop_walk [] =
{
	drone_ai_walk,  3, NULL,
	drone_ai_walk,  3, NULL,
	drone_ai_walk,  2, NULL,
	drone_ai_walk,  2, NULL,
	drone_ai_walk,  4, mytank_footstep
};
mmove_t	tank_move_stop_walk = {FRAME_walk21, FRAME_walk25, tank_frames_stop_walk, mytank_stand};

void tank_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &tank_move_walk;
}

//
// run
//

void mytank_run (edict_t *self);

mframe_t mytank_frames_start_run [] =
{
	drone_ai_run,  15, NULL,
	drone_ai_run,  15, NULL,
	drone_ai_run,  15, NULL,
	drone_ai_run, 15, mytank_footstep
};
mmove_t	mytank_move_start_run = {FRAME_walk01, FRAME_walk04, mytank_frames_start_run, mytank_run};

mframe_t mytank_frames_run [] =
{
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	mytank_footstep,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	NULL,
	drone_ai_run, 15,	mytank_footstep
};
mmove_t	mytank_move_run = {FRAME_walk05, FRAME_walk20, mytank_frames_run, NULL};

void mytank_run (edict_t *self)
{
	if (self->enemy && self->enemy->client)
		self->monsterinfo.aiflags |= AI_BRUTAL;
	else
		self->monsterinfo.aiflags &= ~AI_BRUTAL;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &mytank_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == &mytank_move_start_run)
	{
		self->monsterinfo.currentmove = &mytank_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &mytank_move_start_run;
	}
}

//
// attacks
//

void myTankRail (edict_t *self)
{
	int		flash_number, damage;
	vec3_t	forward, start;

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else
		flash_number = MZ2_TANK_BLASTER_3;

	damage = 50 + 5*self->monsterinfo.level;

	MonsterAim(self, 0.5, 0, false, flash_number, forward, start);
	monster_fire_railgun(self, start, forward, damage, damage, MZ2_GLADIATOR_RAILGUN_1);
}

void myTankBlaster (edict_t *self)
{
	int		flash_number, speed, damage;
	vec3_t	forward, start;

	// alternate attack for commander
	if (self->s.skinnum & 2)
	{
		myTankRail(self);
		return;
	}

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else
		flash_number = MZ2_TANK_BLASTER_3;

	damage = 35 + 15*self->monsterinfo.level;
	speed = 650 /*+ 50*self->monsterinfo.level*/; // speed should NEVER scale.

	MonsterAim(self, 1, speed, false, flash_number, forward, start);
	monster_fire_blaster(self, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BOLT, 2.0, true, flash_number);
}	

void myTankStrike (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_strike, 1, ATTN_NORM, 0);
}	

void myTankRocket (edict_t *self)
{
	int		flash_number, damage, speed;
	vec3_t	forward, start;

	// sanity check
	if (!self->enemy || !self->enemy->inuse)
		return;

	if (self->s.frame == FRAME_attak324)
		flash_number = MZ2_TANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak327)
		flash_number = MZ2_TANK_ROCKET_2;
	else
		flash_number = MZ2_TANK_ROCKET_3;

	damage = 20 + 10*self->monsterinfo.level;
	if( self->activator && self->activator->client )
	{
		speed = 800 + 30*self->monsterinfo.level;	
	}
	else
	{
		speed = 880;
	}	

	MonsterAim(self, 1, speed, true, flash_number, forward, start);

	monster_fire_rocket (self, start, forward, damage, speed, flash_number);
}	

void myTankMachineGun (edict_t *self)
{
	vec3_t	forward, start;
	int		flash_number, damage;

	// sanity check
	if (!self->enemy || !self->enemy->inuse)
		return;

	flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak406);

	damage = 5 + 2*self->monsterinfo.level;

	MonsterAim(self, 0.6, 0, false, flash_number, forward, start);

	monster_fire_bullet (self, start, forward, damage, damage, 
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	

mframe_t mytank_frames_attack_blast [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myTankBlaster,		// 10
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myTankBlaster,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myTankBlaster			// 16
};
mmove_t mytank_move_attack_blast = {FRAME_attak101, FRAME_attak116, mytank_frames_attack_blast, mytank_reattack_blaster};

mframe_t mytank_frames_reattack_blast [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myTankBlaster,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myTankBlaster			// 16
};
mmove_t mytank_move_reattack_blast = {FRAME_attak111, FRAME_attak116, mytank_frames_reattack_blast, mytank_reattack_blaster};

void mytank_delay (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse)
		return;

	// delay next attack if we're not standing ground, our enemy isn't within rocket range
	// (we need to get closer) and we are not a tank commander/boss
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND) && (entdist(self, self->enemy) > 512) 
		&& (self->monsterinfo.control_cost < 100))
		self->monsterinfo.attack_finished = level.time + GetRandom(20, 30)*FRAMETIME;
}

mframe_t mytank_frames_attack_post_blast [] =	
{
	ai_move, 0,		NULL,				// 17
	ai_move, 0,		NULL,
	ai_move, 0,		NULL,
	ai_move, 0,		NULL,
	ai_move, 0,		NULL,//mytank_delay,
	ai_move, 0,	mytank_footstep		// 22
};
mmove_t mytank_move_attack_post_blast = {FRAME_attak117, FRAME_attak122, mytank_frames_attack_post_blast, mytank_run};

void mytank_reattack_blaster (edict_t *self)
{
	float r, range;

	if (G_ValidTarget(self, self->enemy, true))
	{
		r = random();
		range = entdist(self, self->enemy);

		// medium range = 80% chance to continue attack
		if (range <= 512)
		{
			if (r <= 0.8)
				self->monsterinfo.currentmove = &mytank_move_reattack_blast;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_post_blast;
		}
		// long range = 50% chance to continue attack
		else
		{
			if (r <= 0.5)
				self->monsterinfo.currentmove = &mytank_move_reattack_blast;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_post_blast;
		}
	}
	else
	{
		self->monsterinfo.currentmove = &mytank_move_attack_post_blast;
	}

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 2.0 + random();
}


void mytank_poststrike (edict_t *self)
{
	self->enemy = NULL;
	mytank_run (self);
}

mframe_t mytank_frames_attack_strike [] =
{
	ai_move, 3,   NULL,
	ai_move, 2,   NULL,
	ai_move, 2,   NULL,
	ai_move, 1,   NULL,
	ai_move, 6,   NULL,
	ai_move, 7,   NULL,
	ai_move, 9,   mytank_footstep,
	ai_move, 2,   NULL,
	ai_move, 1,   NULL,
	ai_move, 2,   NULL,
	ai_move, 2,   mytank_footstep,
	ai_move, 2,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -2,  NULL,
	ai_move, -2,  NULL,
	ai_move, 0,   mytank_windup,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   myTankStrike,
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
	ai_move, -2,  mytank_footstep
};
mmove_t mytank_move_attack_strike = {FRAME_attak201, FRAME_attak238, mytank_frames_attack_strike, mytank_poststrike};

mframe_t mytank_frames_strike [] =
{
	//ai_move, 0,   mytank_windup,
	//ai_move, 0,   NULL,
	//ai_move, 0,   NULL,
	ai_move, 0,   mytank_windup,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   mytank_meleeattack,
};
mmove_t mytank_move_strike = {FRAME_attak222, FRAME_attak226, mytank_frames_strike, mytank_restrike};

mframe_t mytank_frames_post_strike [] =
{
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
	ai_move, -3,  NULL,//mytank_delay,
	ai_move, -2,  mytank_footstep
};
mmove_t mytank_move_post_strike = {FRAME_attak227, FRAME_attak238, mytank_frames_post_strike, mytank_run};

mframe_t mytank_frames_attack_pre_rocket [] =
{
	ai_charge, 0,  NULL,
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
	ai_charge, 7,  mytank_footstep,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 20

	ai_charge, -3, NULL
};
mmove_t mytank_move_attack_pre_rocket = {FRAME_attak301, FRAME_attak321, mytank_frames_attack_pre_rocket, mytank_doattack_rocket};

mframe_t mytank_frames_attack_fire_rocket [] =
{
	ai_charge, 0, NULL,			// Loop Start	22 
	ai_charge, 0,  NULL,
	ai_charge, 0,  myTankRocket,		// 24
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  myTankRocket,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0, myTankRocket		// 30	Loop End
};
mmove_t mytank_move_attack_fire_rocket = {FRAME_attak322, FRAME_attak330, mytank_frames_attack_fire_rocket, mytank_refire_rocket};

mframe_t mytank_frames_attack_post_rocket [] =
{	
	ai_charge, 0,  NULL,			// 31
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 40

	ai_charge, 0,  NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, mytank_footstep,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,			// 50

	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  mytank_delay
};
mmove_t mytank_move_attack_post_rocket = {FRAME_attak331, FRAME_attak353, mytank_frames_attack_post_rocket, mytank_run};

mframe_t mytank_frames_attack_chain_start [] =
{
	ai_charge, 0, NULL,	// 168
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL	// 172
};
mmove_t mytank_move_attack_chain_start = {FRAME_attak401, FRAME_attak405, mytank_frames_attack_chain_start, mytank_attack_chain};

mframe_t mytank_frames_attack_chain_end [] =
{
	ai_charge, 0, NULL,	// 192
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, mytank_delay	// 196
};
mmove_t mytank_move_attack_chain_end = {FRAME_attak425, FRAME_attak429, mytank_frames_attack_chain_end, mytank_run};

mframe_t mytank_frames_attack_chain [] =
{
	ai_charge,		0, myTankMachineGun,	// 173
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun,
	ai_charge,      0, myTankMachineGun		// 191
};
mmove_t mytank_move_attack_chain = {FRAME_attak406, FRAME_attak424, mytank_frames_attack_chain, mytank_chain_refire};

void mytank_attack_chain (edict_t *self)
{
	// continue attack sequence unless enemy is no longer valid
	if (G_ValidTarget(self, self->enemy, true))
		self->monsterinfo.currentmove = &mytank_move_attack_chain;
	else
		mytank_run(self);
}

void mytank_chain_refire (edict_t *self)
{
	float	r, range;

	// is enemy still valid?
	if (G_ValidTarget(self, self->enemy, true))
	{
		r = random();
		range = entdist(self, self->enemy);

		// medium range = 50% chance to continue attack
		if (range <= 512)
		{
			if (r <= 0.5)
				self->monsterinfo.currentmove = &mytank_move_attack_chain;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_chain_end;

		}
		// long range = 80% chance to continue attack
		else if (r <= 0.8)
			self->monsterinfo.currentmove = &mytank_move_attack_chain;
		// end attack sequence
		else
			self->monsterinfo.currentmove = &mytank_move_attack_chain_end;
	}
	else
	{
		self->monsterinfo.currentmove = &mytank_move_attack_chain_end;
	}
	
	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 2.0 + random();
}

void mytank_restrike (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.6 || !self->enemy->client)
		&& (entdist(self, self->enemy) < 128))
	{
		self->monsterinfo.currentmove = &mytank_move_strike;
	}
	else
	{
		self->monsterinfo.currentmove = &mytank_move_post_strike;
	}

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 0.5 + random();
}

void mytank_refire_rocket (edict_t *self)
{
	float	r, range;
	
	if (G_ValidTarget(self, self->enemy, true))
	{
		r = random();
		range = entdist(self, self->enemy);

		// medium range
		if (range <= 512)
		{
			if (r <= 0.8)
				self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_post_rocket;
		}
		// long range
		else
		{
			if (r <= 0.5)
				self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_post_rocket;
		}
	}
	// end attack sequence
	else
	{
		self->monsterinfo.currentmove = &mytank_move_attack_post_rocket;
	}

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 2.5 + random();
}

void mytank_doattack_rocket (edict_t *self)
{
	self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
}

void mytank_melee (edict_t *self)
{
	
}

void mytank_meleeattack (edict_t *self)
{
	int damage;
	trace_t tr;
	edict_t *other=NULL;
	vec3_t	v;

	// tank must be on the ground to punch
	if (!self->groundentity)
		return;

	self->lastsound = level.framenum;

	damage = 100+20*self->monsterinfo.level;
	gi.sound (self, CHAN_AUTO, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	
	while ((other = findradius(other, self->s.origin, 128)) != NULL)
	{
		if (!G_ValidTarget(self, other, true))
			continue;
		// bosses don't have to be facing their enemy, others do
		//if ((self->monsterinfo.control_cost < 3) && !nearfov(self, other, 0, 60))//!infront(self, other))
		//	continue;

		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage (other, self, self, v, tr.endpos, tr.plane.normal, damage, 200, 0, MOD_TANK_PUNCH);
		//other->velocity[2] += 200;//damage / 2;
	}
}

/*
void mytank_meleeattack (edict_t *self)
{
	int			damage;
	vec3_t		v;
	edict_t		*other = NULL;
	trace_t		tr;

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;
	if (self->enemy->health <= 0)
		return;

	damage = 100 + 20*self->monsterinfo.level;

	while ((other = findradius(other, self->s.origin, 256)) != NULL)
	{
		if (other == self)
			continue;
		if (!other->inuse)
			continue;
		if (!other->takedamage)
			continue;
		if (other->solid == SOLID_NOT)
			continue;
		if (!other->groundentity)
			continue;
		if (OnSameTeam(self, other))
			continue;
		if (!visible(self, other))
			continue;
		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage (other, self, self, v, other->s.origin, tr.plane.normal, damage, damage, 0, MOD_UNKNOWN);
		other->velocity[2] += damage / 2;
	}
	myTankStrike(self);
}
*/


/*
qboolean TeleportNearTarget (edict_t *self, edict_t *target)
{
	int		x;
	vec3_t	forward, right, start, point, dir;
	trace_t tr;

	//GHz: Get starting position and relative angles
	VectorCopy(target->s.origin, start);
	start[2]++;
	AngleVectors(self->s.angles, forward, right, NULL);

	for (x = 0; x < 4; x++)
	{
		//GHz: Get direction
		switch (x)
		{
		case 0: VectorCopy(forward, dir);break;
		case 1: VectorCopy(right, dir);break;
		case 2: VectorInverse(forward);VectorCopy(forward, dir);break;
		case 3: VectorInverse(right);VectorCopy(right, dir);break;
		}

		//GHz: Check target for valid spot
		VectorMA(start, 75, dir, start);
		tr = gi.trace(start, self->mins, self->maxs, start, target, MASK_SHOT);
		if (!(tr.contents & MASK_SHOT))
		{
			//GHz: Check for spot for landing
			VectorCopy(start, point);
			point[2] -= 32;
			tr = gi.trace(start, NULL, NULL, point, NULL, MASK_SHOT);
			if (tr.fraction != 1.0)
			{
				//self->s.event = EV_PLAYER_TELEPORT;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BOSSTPORT);
				gi.WritePosition (self->s.origin);
				gi.multicast (self->s.origin, MULTICAST_PVS);

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BOSSTPORT);
				gi.WritePosition (start);
				gi.multicast (start, MULTICAST_PVS);

				VectorCopy(start, self->s.origin);
				return true;
			}
		}
	}
	return false;
}
*/

void commander_attack (edict_t *self)
{
	float r = random();
	float range = entdist(self, self->enemy);

	// short range attack
	if (range <= 128 && r <= 0.6)
	{
		self->monsterinfo.currentmove = &mytank_move_strike;
	}
	else
	{
		// try to teleport to enemy if we are not standing ground
		if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		{
			if (TeleportNearTarget(self, self->enemy, 16.0))
			{
				if (r <= 0.5)
				{
					self->monsterinfo.currentmove = &mytank_move_strike;
					self->monsterinfo.attack_finished = level.time + 0.5;
					return;
				}

				// recalculate enemy distance
				range = entdist(self, self->enemy);
			}
		}

		// medium range attack
		if (range <= 512)
		{
			if (r <= 0.2)
				self->monsterinfo.currentmove = &mytank_move_attack_blast;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
		}
		// long range attack
		else
		{
			self->monsterinfo.currentmove = &mytank_move_attack_blast;
		}
	}

	// don't call attack function for awhile
	self->monsterinfo.attack_finished = level.time + 2.0;
}

void tank_attack (edict_t *self)
{
	float r = random();
	float range = entdist(self, self->enemy);

	//gi.dprintf("%d tank_attack()\n", level.framenum);

	// short range attack (60% strike, then 20% blaster, 80% rocket)
	if (range <= 128)
	{
		if ((!self->enemy->client || r <= 0.6) && self->groundentity)
		{
			self->monsterinfo.currentmove = &mytank_move_strike;
		}
		else
		{
			if (r <= 0.2)
				self->monsterinfo.currentmove = &mytank_move_attack_blast;
			else
				self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
		}
	}
	// medium range attack (20% chain, 40% blaster, 40% rocket)
	else if (range <= 512)
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &mytank_move_attack_chain;
		else if (r <= 0.6)
			self->monsterinfo.currentmove = &mytank_move_attack_blast;
		else
			self->monsterinfo.currentmove = &mytank_move_attack_fire_rocket;
	}
	// long range attack (20% blaster, 80% chain)
	else
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &mytank_move_attack_blast;
		else
			self->monsterinfo.currentmove = &mytank_move_attack_chain;
	}

	// don't call attack function for awhile
	self->monsterinfo.attack_finished = level.time + 2.0;
}

void mytank_attack (edict_t *self)
{
	if (self->s.skinnum & 2)
		commander_attack(self);
	else
		tank_attack(self);
}

void mytank_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	//mytank_attack(self);
}

//
// death
//

void mytank_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, -0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t mytank_frames_death1 [] =
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
	ai_move, -15, mytank_thud,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t	mytank_move_death = {FRAME_death101, FRAME_death132, mytank_frames_death1, mytank_dead};

void mytank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

#ifdef OLD_NOLAG_STYLE
	// reduce lag by removing the entity right away
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}
#endif

	// check for gibbed body
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		//self->deadflag = DEAD_DEAD;
#ifdef OLD_NOLAG_STYLE
		M_Remove(self, false, false);
#else
		if (nolag->value)
			M_Remove(self, false, true);
		else
			M_Remove(self, false, false);
#endif
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	DroneList_Remove(self);

	// begin death sequence
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &mytank_move_death;
	
	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

//
// monster_tank
//

/*QUAKED monster_tank (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
/*QUAKED monster_mytank_commander (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
void init_drone_tank (edict_t *self)
{
//	if (deathmatch->value)
//	{
//		G_FreeEdict (self);
//		return;
//	}

	self->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	VectorSet (self->mins, -24, -24, -16);
	VectorSet (self->maxs, 24, 24, 64);
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

//	if (self->activator && self->activator->client)
	self->health = 150 + 95*self->monsterinfo.level;
	//else self->health = 100 + 65*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -200;

	//if (self->activator && self->activator->client)
	self->monsterinfo.power_armor_power = 200 + 95*self->monsterinfo.level;
	//else self->monsterinfo.power_armor_power = 200 + 105*self->monsterinfo.level;

	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;

	self->monsterinfo.control_cost = M_TANK_CONTROL_COST;
	self->monsterinfo.cost = M_TANK_COST;
	self->mtype = M_TANK;
	
	if (random() > 0.5)
		self->item = FindItemByClassname("ammo_bullets");
	else
		self->item = FindItemByClassname("ammo_rockets");

	self->mass = 500;

	//self->pain = mytank_pain;
	self->die = mytank_die;
	//self->touch = mytank_touch;
	self->monsterinfo.stand = mytank_stand;
	self->monsterinfo.walk = tank_walk;
	self->monsterinfo.run = mytank_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = mytank_attack;
	self->monsterinfo.melee = mytank_melee;
	self->monsterinfo.sight = mytank_sight;
	self->monsterinfo.idle = mytank_idle;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	//self->monsterinfo.melee = 1;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &mytank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start(self);
	self->nextthink = level.time + FRAMETIME;

//	self->activator->num_monsters += self->monsterinfo.control_cost;
}

void init_drone_commander (edict_t *self)
{
	init_drone_tank(self);

	// modify health and armor
	if (invasion->value < 2)
		self->health = 950 + 675*self->monsterinfo.level;
	else
		self->health = 1500 + 750*self->monsterinfo.level;

	self->max_health = self->health;
	self->monsterinfo.power_armor_power = 675*self->monsterinfo.level;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;

	self->monsterinfo.control_cost = 101;
	self->monsterinfo.cost = M_COMMANDER_COST;
	self->mtype = M_COMMANDER;
	self->s.skinnum = 2;

	if (!invasion->value)
		G_PrintGreenText(va("A level %d tank commander has spawned!", self->monsterinfo.level));
}


