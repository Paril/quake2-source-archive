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
//static int	sound_idle;
static int	sound_die;
static int	sound_step;
static int	sound_sight;
static int	sound_windup;
static int	sound_strike;

//
// misc
//

void tank_footstep (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
	}
}

void tank_thud (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

void tank_windup (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_windup, 1, ATTN_NORM, 0);
	self->s.frame += 5;
}

/*
void tank_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
*/

//
// stand
//

mframe_t tank_frames_stand []=
{
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}
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
	{ai_walk,  0, NULL},
	{ai_walk,  6, NULL},
	{ai_walk,  6, NULL},
	{ai_walk, 11, tank_footstep}
};
mmove_t	tank_move_start_walk = {FRAME_walk01, FRAME_walk04, tank_frames_start_walk, tank_walk};

mframe_t tank_frames_walk [] =
{
	{ai_walk,  4,	NULL},
	{ai_walk,  5,	NULL},
	{ai_walk,  3,	NULL},
	{ai_walk,  2,	NULL},
	{ai_walk,  5,	NULL},
	{ai_walk,  5,	NULL},
	{ai_walk,  4,	NULL},
	{ai_walk,  4,	tank_footstep},
	{ai_walk,  3,	NULL},
	{ai_walk,  5,	NULL},
	{ai_walk,  4,	NULL},
	{ai_walk,  5,	NULL},
	{ai_walk,  7,	NULL},
	{ai_walk,  7,	NULL},
	{ai_walk,  6,	NULL},
	{ai_walk,  6,	tank_footstep}
};
mmove_t	tank_move_walk = {FRAME_walk05, FRAME_walk20, tank_frames_walk, NULL};

mframe_t tank_frames_stop_walk [] =
{
	{ai_walk,  3, NULL},
	{ai_walk,  3, NULL},
	{ai_walk,  2, NULL},
	{ai_walk,  2, NULL},
	{ai_walk,  4, tank_footstep}
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
	{ai_run,  6, NULL},
	{ai_run, 12, NULL},
	{ai_run, 12, NULL},
	{ai_run, 22, tank_footstep}
};
mmove_t	tank_move_start_run = {FRAME_walk01, FRAME_walk04, tank_frames_start_run, tank_run};

mframe_t tank_frames_run [] =
{
	{ai_run, 12,	NULL},
	{ai_run, 15,	NULL},
	{ai_run,  9,	NULL},
	{ai_run,  8,	NULL},
	{ai_run, 15,	NULL},
	{ai_run, 15,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	tank_footstep},
	{ai_run,  9,	NULL},
	{ai_run, 15,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 15,	NULL},
	{ai_run, 21,	NULL},
	{ai_run, 21,	NULL},
	{ai_run, 18,	NULL},
	{ai_run, 18,	tank_footstep}
};
mmove_t	tank_move_run = {FRAME_walk05, FRAME_walk20, tank_frames_run, NULL};

mframe_t tank_frames_stop_run [] =
{
	{ai_run,  9, NULL},
	{ai_run,  9, NULL},
	{ai_run,  6, NULL},
	{ai_run,  6, NULL},
	{ai_run, 12, tank_footstep}
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
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t tank_move_pain1 = {FRAME_pain101, FRAME_pain104, tank_frames_pain1, tank_run};

mframe_t tank_frames_pain2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t tank_move_pain2 = {FRAME_pain201, FRAME_pain205, tank_frames_pain2, tank_run};

mframe_t tank_frames_pain3 [] =
{
	{ai_move, -7, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 2,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 3,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 2,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  tank_footstep}
};
mmove_t	tank_move_pain3 = {FRAME_pain301, FRAME_pain316, tank_frames_pain3, tank_run};


void tank_pain (edict_t *self, edict_t *other, float kick, int damage) {
	if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;

	if (damage <= 5 + 2.5 * self->monsterinfo.skill)
		return;

	if (level.time < self->pain_debounce_time)
			return;

	if (damage <= 50 + 5 * self->monsterinfo.skill)
		if (random() > 0.3)
			return;

	self->pain_debounce_time = level.time + (3 + 0.1 * self->monsterinfo.skill);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}

	if (damage <= 40 + 4 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &tank_move_pain1;
		self->monsterinfo.jumptime = level.time + 3.0;
	} else if (damage <= 60 + 6 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &tank_move_pain2;
		self->monsterinfo.jumptime = level.time + 5.0;
	} else {
		self->monsterinfo.currentmove = &tank_move_pain3;
		self->monsterinfo.jumptime = level.time + 7.0;
	}
}


//
// attacks
//

void TankBlaster (edict_t *self) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	int		flash_number;
	int damage;

	if (self->s.skinnum >= 2) { //Tank commander
		if (self->radius_dmg)
			damage = 90 + 10 * self->monsterinfo.skill;
		else
			damage = 45 + 5 * self->monsterinfo.skill;
	} else { // Ordinary tank
		if (self->radius_dmg)
			damage = 60 + 8 * self->monsterinfo.skill;
		else
			damage = 30 + 4 * self->monsterinfo.skill;
	}

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else // (self->s.frame == FRAME_attak116)
		flash_number = MZ2_TANK_BLASTER_3;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy && (self->enemy->health > 0)) {
		VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, end);
//		VectorCopy (self->enemy->s.origin, end);
		end[2] += self->enemy->viewheight;
		VectorSubtract (end, start, dir);
	} else {
		VectorCopy(self->s.angles, dir);
	}

	monster_fire_blaster (self, start, dir, damage, 600 + 25 * self->monsterinfo.skill, flash_number, EF_BLASTER);
}

void TankStrike (edict_t *self) {
	int damage;

	if (self->s.skinnum >= 2) { //Tank commander
		if (self->radius_dmg)
			damage = (210 + 40 * self->monsterinfo.skill);
		else
			damage = (105 + 20 * self->monsterinfo.skill);
	} else { // Ordinary tank
		if (self->radius_dmg)
			damage = (140 + 30 * self->monsterinfo.skill);
		else
			damage = (70 + 15 * self->monsterinfo.skill);
	}

	T_RadiusDamage(NULL, self, self, damage, damage * 0.8, self, 150 + 2 * self->monsterinfo.skill, true, MOD_TANKSTRIKE);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_strike, 1, ATTN_NORM, 0);
	}
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
void TankRestrike(edict_t *self) {
	if (self->enemy && (self->enemy->health > 0)) {
		vec3_t	vec;
		float	range;
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		range = VectorLength (vec);

		if (range <= 150) {
			self->s.frame -= 11;
		} else {
			self->s.frame += 5;
		}
	}
}

void tank_attack(edict_t *self);
mframe_t tank_frames_attack_strike [] =
{
	{ai_move, 3,   monster_skip8frames},
	{ai_move, 2,   NULL},
	{ai_move, 2,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 6,   NULL},
	{ai_move, 7,   NULL},
	{ai_move, 9,   tank_footstep},
	{ai_move, 2,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 2,   monster_skip8frames},
	{ai_move, 2,   tank_footstep},
	{ai_move, 2,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -2,  NULL},
	{ai_move, -2,  NULL},
	{ai_move, 0,   tank_windup}, //windup skips 5
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   TankStrike},
	{ai_move, 0,   TankRestrike}, //Restrike skips 5
	{ai_move, -1,  NULL},
	{ai_move, -1,  NULL},
	{ai_move, -1,  NULL},
	{ai_move, -1,  NULL},
	{ai_move, -1,  NULL},
	{ai_move, -3,  monster_skip4frames},
	{ai_move, -10, NULL},
	{ai_move, -10, NULL},
	{ai_move, -2,  NULL},
	{ai_move, -3,  NULL},
	{ai_move, -2,  tank_attack}
};
mmove_t tank_move_attack_strike = {FRAME_attak201, FRAME_attak238, tank_frames_attack_strike, tank_run};

void TankCheckRocketTarget (edict_t *self) {
	if (self->enemy && visible(self, self->enemy)) {
		VectorCopy(self->enemy->s.origin, self->pos1);
		self->pos1[2] += self->enemy->viewheight;
	}
}
/*
void TankCheckRocketEnd (edict_t *self) {
	if (!visible(self, self->enemy)) {
		self->pos1[0] = -1;
		self->pos1[1] = -1;
		self->pos1[2] = -1;
	}
}
*/
void TankRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;
	int damage;
	if (self->s.skinnum >= 2) { //Tank commander
		if (self->radius_dmg)
			damage = 200 + 36 * self->monsterinfo.skill;
		else
			damage = 100 + 18 * self->monsterinfo.skill;
	} else { // Ordinary tank
		if (self->radius_dmg)
			damage = 150 + 24 * self->monsterinfo.skill;
		else
			damage = 75 + 12 * self->monsterinfo.skill;
	}

	if (self->s.frame == FRAME_attak324)
		flash_number = MZ2_TANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak327)
		flash_number = MZ2_TANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak330)
		flash_number = MZ2_TANK_ROCKET_3;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy && visible(self, self->enemy)) {
		VectorCopy(self->enemy->s.origin, self->pos1);
		VectorMA(self->enemy->s.origin, 0.5, self->enemy->velocity, vec);
		self->pos1[2] += self->enemy->viewheight;
		vec[2] += self->enemy->viewheight;
	} else {
		//VectorCopy(self->s.angles, vec);
		VectorCopy(self->pos1, vec);
	}
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, damage, 650 + 15 * self->monsterinfo.skill, flash_number);
}
/*
void TankMachineGun (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;
	int damage;
	if (self->s.skinnum >= 2) { //Tank commander
		if (self->radius_dmg)
			damage = 12 + 3.0 * self->monsterinfo.skill;
		else
			damage = 6 + 1.5 * self->monsterinfo.skill;
	} else { // Ordinary tank
		if (self->radius_dmg)
			damage = 9 + 2.0 * self->monsterinfo.skill;
		else
			damage = 4.5 + 1.0 * self->monsterinfo.skill;
	}

	flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak406);

	AngleVectors (self->s.angles, forward, right, NULL);
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
		vec[1] = self->s.angles[1];
		vec[2] = 0;
		dir[0] = 0;
	}
	if (self->s.frame <= FRAME_attak415)
		dir[1] = self->s.angles[1] - (2 - 0.063 * self->monsterinfo.skill) * (self->s.frame - FRAME_attak411);
	else
		dir[1] = self->s.angles[1] + (2 - 0.063 * self->monsterinfo.skill) * (self->s.frame - FRAME_attak419);
	dir[2] = vec[2];

	AngleVectors (dir, forward, NULL, NULL);

	monster_fire_bullet (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	
*/

mframe_t tank_frames_attack_blast [] =
{
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	TankBlaster},		// 10
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	TankBlaster},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	NULL},
	{ai_run_circle, 16,	TankBlaster}		// 16
};
mmove_t tank_move_attack_blast = {FRAME_attak101, FRAME_attak116, tank_frames_attack_blast, tank_reattack_blaster};

mframe_t tank_frames_reattack_blast [] =
{
	{ai_run_circle, 18,	NULL},
	{ai_run_circle, 18,	NULL},
	{ai_run_circle, 18,	TankBlaster},
	{ai_run_circle, 18,	TankBlaster},
	{ai_run_circle, 18,	NULL},
	{ai_run_circle, 18,	TankBlaster}		// 16
};
mmove_t tank_move_reattack_blast = {FRAME_attak111, FRAME_attak116, tank_frames_reattack_blast, tank_reattack_blaster};

mframe_t tank_frames_attack_post_blast [] =
{
	{ai_move, 0,		NULL},				// 17
	{ai_move, 0,		NULL},
	{ai_move, 2,		NULL},
	{ai_move, 3,		NULL},
	{ai_move, 2,		NULL},
	{ai_move, -2,	tank_footstep}		// 22
};
mmove_t tank_move_attack_post_blast = {FRAME_attak117, FRAME_attak122, tank_frames_attack_post_blast, tank_run};

void tank_reattack_blaster (edict_t *self) {
	vec3_t	vec;
	float	range;

	if (self->enemy && self->enemy->health > 0) {
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		range = VectorLength (vec);

		if (range <= 150) {
			self->monsterinfo.currentmove = &tank_move_attack_strike;
			return;
		} else {
			if (visible (self, self->enemy)) {
				if (random() < (0.5 + 0.013 * self->monsterinfo.skill)) {
					self->monsterinfo.currentmove = &tank_move_reattack_blast;
					return;
				}
			}
		}
	}
	self->monsterinfo.currentmove = &tank_move_attack_post_blast;
	self->monsterinfo.attack_finished = level.time + 0.3 + 1.2 * random();
}

/*
void tank_poststrike (edict_t *self) {
	self->enemy = NULL;
	tank_run (self);
}
*/

mframe_t tank_frames_attack_pre_rocket [] =
{
/*	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},
	{ai_run_circle, 0,  NULL},*/			// 10

	{ai_run_circle, 10,  TankCheckRocketTarget},
	{ai_run_circle, 11,  TankCheckRocketTarget},
	{ai_run_circle, 11,  TankCheckRocketTarget},
	{ai_run_circle, 12,  TankCheckRocketTarget},
	{ai_run_circle, 12,  TankCheckRocketTarget},
	{ai_run_circle, 13,  tank_footstep},
	{ai_run_circle, 13,  TankCheckRocketTarget},
	{ai_run_circle, 14,  TankCheckRocketTarget},
	{ai_run_circle, 15,  TankCheckRocketTarget},
	{ai_run_circle, 15,  TankCheckRocketTarget},		// 20

	{ai_run_circle, 16, TankCheckRocketTarget}
};
mmove_t tank_move_attack_pre_rocket = {FRAME_attak311 /*301*/, FRAME_attak321, tank_frames_attack_pre_rocket, tank_doattack_rocket};

mframe_t tank_frames_attack_fire_rocket [] =
{
	{ai_run_circle, 16, TankCheckRocketTarget},	// Loop Start	22
	{ai_run_circle, 16,  TankCheckRocketTarget},
	{ai_run_circle, 16,  TankRocket},		// 24
	{ai_run_circle, 16,  TankCheckRocketTarget},
	{ai_run_circle, 16,  TankCheckRocketTarget},
	{ai_run_circle, 16,  TankRocket},
	{ai_run_circle, 16,  TankCheckRocketTarget},
	{ai_run_circle, 16,  TankCheckRocketTarget},
	{ai_run_circle, 16, TankRocket}		// 30	Loop End
};
mmove_t tank_move_attack_fire_rocket = {FRAME_attak322, FRAME_attak330, tank_frames_attack_fire_rocket, tank_refire_rocket};

mframe_t tank_frames_attack_post_rocket [] =
{
	{ai_run_circle, 16,  NULL},			// 31
	{ai_run_circle, 16, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15,  NULL},
	{ai_run_circle, 15,  NULL},
	{ai_run_circle, 14,  NULL},
	{ai_run_circle, 14,  NULL},
	{ai_run_circle, 14,  NULL},
	{ai_run_circle, 13,  NULL},
	{ai_run_circle, 13,  NULL},			// 40

	{ai_run_circle, 13,  NULL},
	{ai_run_circle, 12, NULL},
	{ai_run_circle, 12, NULL},
	{ai_run_circle, 12, NULL},
	{ai_run_circle, 11, NULL},
	{ai_run_circle, 11, tank_footstep},
	{ai_run_circle, 11,  NULL},
	{ai_run_circle, 10,  NULL},
	{ai_run_circle, 10,  NULL},
	{ai_run_circle, 10,  NULL},			// 50

	{ai_run_circle, 9,  NULL},
	{ai_run_circle, 9,  NULL},
	{ai_run_circle, 9,  NULL}
};
mmove_t tank_move_attack_post_rocket = {FRAME_attak331, FRAME_attak353, tank_frames_attack_post_rocket, tank_run};
/*
mframe_t tank_frames_attack_chain [] =
{
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, TankMachineGun},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}
};
mmove_t tank_move_attack_chain = {FRAME_attak401, FRAME_attak429, tank_frames_attack_chain, tank_run};
*/

void tank_refire_rocket (edict_t *self)
{
	if (self->enemy && (self->enemy->health > 0)) {
		float	range;
		vec3_t	vec;
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		range = VectorLength (vec);

		if (range <= 150) {
			self->monsterinfo.currentmove = &tank_move_attack_strike;
			return;
		} else {
			if (visible(self, self->enemy) ) {
				if (random() < (0.5 + 0.015 * self->monsterinfo.skill)) {
					self->monsterinfo.currentmove = &tank_move_attack_fire_rocket;
					return;
				}
			} else if ((self->pos1[0] != self->pos2[0]) || (self->pos1[1] != self->pos2[1]) || (self->pos1[2] != self->pos2[2])) {
				//if (random() < (0.5 + 0.015 * self->monsterinfo.skill)) {
					VectorCopy(self->pos1, self->pos2);
				//}
				self->monsterinfo.currentmove = &tank_move_attack_fire_rocket;
				return;
			}
		}
	}
	self->monsterinfo.currentmove = &tank_move_attack_post_rocket;
	self->monsterinfo.attack_finished = level.time + 0.3 + 1.2 * random();
}

void tank_doattack_rocket (edict_t *self) {
	self->monsterinfo.currentmove = &tank_move_attack_fire_rocket;
}

void tank_attack(edict_t *self) {
	vec3_t	vec;
	float	range;
	float	r;
/*
	if (self->enemy->health < 0)
	{
		self->monsterinfo.currentmove = &tank_move_attack_strike;
		self->monsterinfo.aiflags &= ~AI_BRUTAL;
		return;
	}
*/
	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	r = random();

	if (range <= 150) {
/*		if (r < (0.4 - 0.02 * self->monsterinfo.skill))
			self->monsterinfo.currentmove = &tank_move_attack_chain;
		else
			self->monsterinfo.currentmove = &tank_move_attack_blast;
*/
		self->monsterinfo.currentmove = &tank_move_attack_strike;
//		self->monsterinfo.attack_finished = level.time + 3;
	} else if (range <= 300) {
		if (r < (0.7 - 0.006 * self->monsterinfo.skill))
			self->monsterinfo.currentmove = &tank_move_attack_blast;
		else {
			TankCheckRocketTarget(self);
			self->monsterinfo.currentmove = &tank_move_attack_pre_rocket;
			self->pain_debounce_time = level.time + 5.0;	// no pain for a while
		}
	} else {
		if (r < (0.5 - 0.01 * self->monsterinfo.skill)) {
			self->monsterinfo.currentmove = &tank_move_attack_blast;
		} else {
			TankCheckRocketTarget(self);
			self->monsterinfo.currentmove = &tank_move_attack_pre_rocket;
			self->pain_debounce_time = level.time + 5.0;	// no pain for a while
		}
	}
}

void tank_sight (edict_t *self, edict_t *other)
{
	if (random() < (0.5 + 0.05 * self->monsterinfo.skill))
		tank_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}


//
// death
//

void tank_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, -0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t tank_frames_death1 [] =
{
	{ai_move, -7,  NULL},
	{ai_move, -2,  NULL},
	{ai_move, -2,  NULL},
	{ai_move, 1,   NULL},
	{ai_move, 3,   NULL},
	{ai_move, 6,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 2,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -2,  NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -3,  NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -4,  NULL},
	{ai_move, -6,  NULL},
	{ai_move, -4,  NULL},
	{ai_move, -5,  NULL},
	{ai_move, -7,  NULL},
	{ai_move, -15, tank_thud},
	{ai_move, -5,  NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL}
};
mmove_t	tank_move_death = {FRAME_death101, FRAME_death132, tank_frames_death1, tank_dead};

void tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1 /*4*/; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
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
/*	if (deathmatch->value)
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
//	sound_idle = gi.soundindex ("tank/tnkidle1.wav");
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

	if (self->classid == CI_M_TANKCOMM) {
		self->health = 1700 + 350 * skill->value;
		self->max_health = self->health;
		self->monsterinfo.level = 2520 + 1780 * skill->value;
		self->gib_health = -600 - 125 * skill->value;
		strcpy(self->monsterinfo.name, "a tank commander");

		self->monsterinfo.cts_any = 0.75;
		self->monsterinfo.cts_class1 = 0.14;
		self->monsterinfo.cts_class2 = 0.30;
		self->monsterinfo.cts_class3 = 0.60;

		self->monsterinfo.item_any = 0.5;
		self->monsterinfo.itemmult_class1 = 1.6;
		self->monsterinfo.itemmult_class2 = 0.9;
		self->monsterinfo.itemmult_class3 = 0.7;
		self->monsterinfo.itemmult_class4 = 0.5;
	} else {
		self->health = 1400 + 275 * skill->value;
		self->max_health = self->health;
		self->monsterinfo.level = 2190 + 1850 * skill->value;
		self->gib_health = -500 - 100 * skill->value;
		strcpy(self->monsterinfo.name, "a tank");

		self->monsterinfo.cts_any = 0.60;
		self->monsterinfo.cts_class1 = 0.10;
		self->monsterinfo.cts_class2 = 0.25;
		self->monsterinfo.cts_class3 = 0.50;

		self->monsterinfo.item_any = 0.4;
		self->monsterinfo.itemmult_class1 = 1.4;
		self->monsterinfo.itemmult_class2 = 0.95;
		self->monsterinfo.itemmult_class3 = 0.7;
		self->monsterinfo.itemmult_class4 = 0.5;
	}

	self->monsterinfo.skill = skill->value;
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
	self->monsterinfo.idle = NULL/*tank_idle*/;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 320;
	self->yaw_speed = 25;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &tank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);

	if (self->classid == CI_M_TANKCOMM)
		self->s.skinnum = 2;
}
