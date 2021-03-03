/*
==============================================================================

SUPERTANK

==============================================================================
*/

#include "g_local.h"
#include "m_supertank.h"

qboolean visible (edict_t *self, edict_t *other);

static int	sound_pain1;
static int	sound_pain2;
//static int	sound_pain3;
static int	sound_death;
static int	sound_search1;
//static int	sound_search2;

static	int	tread_sound;

void BossExplode (edict_t *self);

void TreadSound (edict_t *self) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, tread_sound, 1, ATTN_NORM, 0);
	}
}

void supertank_search (edict_t *self) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	}
}


void supertank_dead(edict_t *self);
void supertankRocket(edict_t *self);
void supertankReRocket(edict_t *self);
void supertankLeftGL(edict_t *self);
void supertankRightGL(edict_t *self);
void supertankMachineGun(edict_t *self);
void supertankLaser(edict_t *self);
void supertank_reattack1(edict_t *self);


//
// stand
//

mframe_t supertank_frames_stand []=
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
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}
};
mmove_t	supertank_move_stand = {FRAME_stand_1, FRAME_stand_60, supertank_frames_stand, NULL};
	
void supertank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &supertank_move_stand;
}


mframe_t supertank_frames_run [] =
{
	{ai_run, 12,	TreadSound},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL},
	{ai_run, 12,	NULL}
};
mmove_t	supertank_move_run = {FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_run, NULL};

//
// walk
//


mframe_t supertank_frames_forward [] =
{
	{ai_walk, 4,	TreadSound},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL},
	{ai_walk, 4,	NULL}
};
mmove_t	supertank_move_forward = {FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_forward, NULL};

void supertank_forward (edict_t *self)
{
		self->monsterinfo.currentmove = &supertank_move_forward;
}

void supertank_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &supertank_move_forward;
}

void supertank_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &supertank_move_stand;
	else
		self->monsterinfo.currentmove = &supertank_move_run;
}

mframe_t supertank_frames_turn_right [] =
{
	{ai_move,	0,	TreadSound},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_turn_right = {FRAME_right_1, FRAME_right_18, supertank_frames_turn_right, supertank_run};

mframe_t supertank_frames_turn_left [] =
{
	{ai_move,	0,	TreadSound},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_turn_left = {FRAME_left_1, FRAME_left_18, supertank_frames_turn_left, supertank_run};


mframe_t supertank_frames_pain3 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_pain3 = {FRAME_pain3_9, FRAME_pain3_12, supertank_frames_pain3, supertank_run};

mframe_t supertank_frames_pain2 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_pain2 = {FRAME_pain2_5, FRAME_pain2_8, supertank_frames_pain2, supertank_run};

mframe_t supertank_frames_pain1 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_pain1 = {FRAME_pain1_1, FRAME_pain1_4, supertank_frames_pain1, supertank_run};

mframe_t supertank_frames_death1 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	BossExplode}
};
mmove_t supertank_move_death = {FRAME_death_1, FRAME_death_24, supertank_frames_death1, supertank_dead};

mframe_t supertank_frames_backward[] =
{
	{ai_walk, 0,	TreadSound},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL},
	{ai_walk, 0,	NULL}
};
mmove_t	supertank_move_backward = {FRAME_backwd_1, FRAME_backwd_18, supertank_frames_backward, NULL};

mframe_t supertank_frames_attack4[]=
{
	{ai_run_circle,	0,	NULL},
	{ai_run_circle,	0,	NULL},
	{ai_run_circle,	0,	supertankLeftGL},
	{ai_run_circle,	0,	NULL},
	{ai_run_circle,	0,	supertankRightGL},
	{ai_run_circle,	0,	NULL}
};
mmove_t supertank_move_attack4 = {FRAME_attak4_1, FRAME_attak4_6, supertank_frames_attack4, supertank_run};
mframe_t supertank_frames_attack5[]=
{
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	supertankLaser},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL}
};
mmove_t supertank_move_attack5 = {FRAME_attak4_1, FRAME_attak4_6, supertank_frames_attack4, supertank_run};

mframe_t supertank_frames_attack3[]=
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t supertank_move_attack3 = {FRAME_attak3_1, FRAME_attak3_27, supertank_frames_attack3, supertank_run};

mframe_t supertank_frames_attack2[]=
{
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	supertankRocket},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	supertankRocket},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	supertankRocket},
	{ai_run_circle,	20,	supertankReRocket},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL}
};
mmove_t supertank_move_attack2 = {FRAME_attak2_1, FRAME_attak2_27, supertank_frames_attack2, supertank_run};

mframe_t supertank_frames_attack1[]= {
	{ai_run_circle,	20,	supertankMachineGun},
	{ai_run_circle,	20,	supertankMachineGun},
	{ai_run_circle,	20,	supertankMachineGun},
	{ai_run_circle,	20,	supertankMachineGun},
	{ai_run_circle,	20,	supertankMachineGun},
	{ai_run_circle,	20,	supertankMachineGun}
};
mmove_t supertank_move_attack1 = {FRAME_attak1_1, FRAME_attak1_6, supertank_frames_attack1, supertank_reattack1};

mframe_t supertank_frames_end_attack1[]=
{
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL}
};
mmove_t supertank_move_end_attack1 = {FRAME_attak1_7, FRAME_attak1_20, supertank_frames_end_attack1, supertank_run};

void supertank_reattack1(edict_t *self) {
	if (self->enemy && visible(self, self->enemy) && infront(self, self->enemy)) {
		if (random() < 0.9) {
			self->monsterinfo.currentmove = &supertank_move_attack1;
			return;
		}
	}
	self->monsterinfo.aim_frames = 0;
	self->monsterinfo.currentmove = &supertank_move_end_attack1;
	self->monsterinfo.attack_finished = level.time + 0.3 + 1.2 * random();
}
void supertankReRocket(edict_t *self) {
	if (self->enemy && visible(self, self->enemy) && infront(self, self->enemy) && (random() < 0.7)) {
		self->s.frame -= 10;
	}
}

void supertank_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames
	if (damage <= 20 + 2 * self->monsterinfo.skill)
		if (random() < 0.2 + 0.01 * self->monsterinfo.skill)
			return;

	// Don't go into pain if he's firing his rockets
	if ( (self->s.frame >= FRAME_attak2_1) && (self->s.frame <= FRAME_attak2_14) )
		return;

	self->pain_debounce_time = level.time + (7 + 0.2 * self->monsterinfo.skill);

	if (damage <= 20 + 2 * self->monsterinfo.skill) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM,0);
		}
		self->monsterinfo.currentmove = &supertank_move_pain1;
		self->monsterinfo.jumptime = level.time + 4.0;
	} else if (damage <= 30 + 3 * self->monsterinfo.skill) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM,0);
		}
		self->monsterinfo.currentmove = &supertank_move_pain2;
		self->monsterinfo.jumptime = level.time + 6.0;
	} else {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM,0);
		}
		self->monsterinfo.currentmove = &supertank_move_pain3;
		self->monsterinfo.jumptime = level.time + 8.0;
	}
}


void supertankRocket (edict_t *self) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;
	int damage;
	if (self->radius_dmg)
		damage = 240 + 44 * self->monsterinfo.skill;
	else
		damage = 120 + 22 * self->monsterinfo.skill;

	if (!self->enemy)
		return;

	if (self->s.frame == FRAME_attak2_8)
		flash_number = MZ2_SUPERTANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak2_11)
		flash_number = MZ2_SUPERTANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak2_14)
		flash_number = MZ2_SUPERTANK_ROCKET_3;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, damage, 900 + 10 * self->monsterinfo.skill, flash_number);
}

void supertankLeftGL(edict_t *self) {
	vec3_t	forward, right, up;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;
	float	range;
	int damage;
	if (self->radius_dmg)
		damage = (16 + 3 * self->monsterinfo.skill);
	else
		damage = (8 + 1.5 * self->monsterinfo.skill);

	if (!self->enemy)
		return;

	range = sqrt(pow(self->s.origin[0] - self->enemy->s.origin[0], 2) + pow(self->s.origin[1] - self->enemy->s.origin[1], 2));

	flash_number = MZ2_SUPERTANK_ROCKET_1;

	AngleVectors (self->s.angles, forward, right, up);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, vec);
	VectorMA(vec, range * 0.1, up, vec);

	VectorMA(start, -9, right, start);
	start[2] -= 26;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_incendiary(self, start, dir, (int) (damage * 0.2), damage, 750 + 10 * self->monsterinfo.skill, flash_number);
}	
void supertankRightGL(edict_t *self) {
	vec3_t	forward, right, up;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;
	float	range;
	int damage;
	if (self->radius_dmg)
		damage = (40 + 8 * self->monsterinfo.skill);
	else
		damage = (20 + 4 * self->monsterinfo.skill);

	if (!self->enemy)
		return;

	range = sqrt(pow(self->s.origin[0] - self->enemy->s.origin[0], 2) + pow(self->s.origin[1] - self->enemy->s.origin[1], 2));

	flash_number = MZ2_SUPERTANK_ROCKET_1;

	AngleVectors (self->s.angles, forward, right, up);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, vec);
	VectorMA(vec, range * 0.1, up, vec);

	VectorMA(start, 67, right, start);
	start[2] -= 26;

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_incendiary(self, start, dir, (int) (damage * 0.2), damage, 750 + 10 * self->monsterinfo.skill, flash_number);
}	

void supertankMachineGun (edict_t *self) {
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;
	int damage;
	float spreadmult = 5 - 0.1 * self->monsterinfo.aim_frames;
	float aimmult = -0.25 + 0.0125 * self->monsterinfo.aim_frames;
	if (spreadmult < 1) {
		spreadmult = 1;
	}
	if (aimmult > 0) {
		aimmult = 0;
	}

	if (self->radius_dmg)
		damage = (int) (20 + 2 * self->monsterinfo.skill);
	else
		damage = (int) (10 + 1.0 * self->monsterinfo.skill);

	flash_number = MZ2_SUPERTANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak1_1);

	//FIXME!!!
	dir[0] = 0;
	dir[1] = self->s.angles[1];
	dir[2] = 0;

	AngleVectors (dir, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy) {
		VectorCopy (self->enemy->s.origin, vec);
		VectorMA (vec, aimmult, self->enemy->velocity, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, forward);
		VectorNormalize (forward);
	}

	monster_fire_bullet (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD * spreadmult, DEFAULT_BULLET_VSPREAD * spreadmult, flash_number);
	self->monsterinfo.aim_frames++;
}
void supertankLaser(edict_t *self) {
	if (self->enemy && visible(self, self->enemy) && infront(self, self->enemy) && (random() < 0.9)) {
		self->s.frame -= 1;
	}
}

void supertank_attack(edict_t *self) {
	vec3_t	vec;
	float	range;
	float	r = random();

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	// Attack 1 == Chaingun
	// Attack 2 == Rocket Launcher

	if (range <= 80) {
		self->monsterinfo.currentmove = &supertank_move_attack1;
	} else { // fire rockets more often at distance
		if (r < 0.2)
			self->monsterinfo.currentmove = &supertank_move_attack1;
		else if (r < 0.7)
			self->monsterinfo.currentmove = &supertank_move_attack2;
		else
			self->monsterinfo.currentmove = &supertank_move_attack4;
	}
}


//
// death
//

void supertank_dead (edict_t *self) {
	VectorSet (self->mins, -60, -60, 0);
	VectorSet (self->maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}


void BossExplode (edict_t *self)
{
	vec3_t	org;
	int		n;

	self->think = BossExplode;
	VectorCopy (self->s.origin, org);
	org[2] += 24 + (rand()&15);
	switch (self->count++)
	{
	case 0:
		org[0] -= 24;
		org[1] -= 24;
		break;
	case 1:
		org[0] += 24;
		org[1] += 24;
		break;
	case 2:
		org[0] += 24;
		org[1] -= 24;
		break;
	case 3:
		org[0] -= 24;
		org[1] += 24;
		break;
	case 4:
		org[0] -= 48;
		org[1] -= 48;
		break;
	case 5:
		org[0] += 48;
		org[1] += 48;
		break;
	case 6:
		org[0] -= 48;
		org[1] += 48;
		break;
	case 7:
		org[0] += 48;
		org[1] -= 48;
		break;
	case 8:
		self->s.sound = 0;
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 500, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 500, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", 500, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (org);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + 0.1;
}


void supertank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->count = 0;
	self->monsterinfo.currentmove = &supertank_move_death;
}

//
// monster_supertank
//

/*QUAKED monster_supertank (1 .5 0) (-64 -64 0) (64 64 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_supertank (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	sound_pain1 = gi.soundindex ("bosstank/btkpain1.wav");
	sound_pain2 = gi.soundindex ("bosstank/btkpain2.wav");
//	sound_pain3 = gi.soundindex ("bosstank/btkpain3.wav");
	sound_death = gi.soundindex ("bosstank/btkdeth1.wav");
	sound_search1 = gi.soundindex ("bosstank/btkunqv1.wav");
//	sound_search2 = gi.soundindex ("bosstank/btkunqv2.wav");

//	self->s.sound = gi.soundindex ("bosstank/btkengn1.wav");
	tread_sound = gi.soundindex ("bosstank/btkengn1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss1/tris.md2");
	VectorSet (self->mins, -64, -64, 0);
	VectorSet (self->maxs, 64, 64, 112);

	if (deathmatch->value) {
		self->health = 2500 + 650 * skill->value; // Only spawns in Hunt, lots of health ;)
		self->monsterinfo.level = 4200 + 2700 * skill->value;
	} else {
		self->health = 2000 + 570 * skill->value;
		self->monsterinfo.level = 3100 + 2250 * skill->value;
	}
	self->gib_health = -500;
	self->mass = 800;
	self->max_health = self->health;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -2000;
	strcpy(self->monsterinfo.name, "a super tank");
	self->yaw_speed = 15;

	self->pain = supertank_pain;
	self->die = supertank_die;
	self->monsterinfo.stand = supertank_stand;
	self->monsterinfo.walk = supertank_walk;
	self->monsterinfo.run = supertank_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = supertank_attack;
	self->monsterinfo.search = supertank_search;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 320;

	self->monsterinfo.cts_any = 0.95;
	self->monsterinfo.cts_class1 = 0.18;
	self->monsterinfo.cts_class2 = 0.28;
	self->monsterinfo.cts_class3 = 0.58;

	self->monsterinfo.item_any = 0.60;
	self->monsterinfo.itemmult_class1 = 1.8;
	self->monsterinfo.itemmult_class2 = 0.95;
	self->monsterinfo.itemmult_class3 = 0.8;
	self->monsterinfo.itemmult_class4 = 0.6;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &supertank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
