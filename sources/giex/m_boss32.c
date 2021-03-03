/*
==============================================================================

Makron -- Final Boss

==============================================================================
*/

#include "g_local.h"
#include "m_boss32.h"

qboolean visible (edict_t *self, edict_t *other);

void MakronRailgun (edict_t *self);
void MakronSaveloc (edict_t *self);
void MakronHyperblaster (edict_t *self);
void makron_step_left (edict_t *self);
void makron_step_right (edict_t *self);
void makronBFG (edict_t *self);
void makron_attack (edict_t *self);
void makron_dead (edict_t *self);
void makron_attackRail(edict_t *self);
void makron_attackHb(edict_t *self);
void makron_attackBfg(edict_t *self);

static int	sound_pain4;
//static int	sound_pain5;
//static int	sound_pain6;
static int	sound_death;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_attack_bfg;
static int	sound_brainsplorch;
static int	sound_prerailgun;
static int	sound_popup;
/*static int	sound_taunt1;
static int	sound_taunt2;
static int	sound_taunt3;*/
static int	sound_hit;
/*
void makron_taunt (edict_t *self)
{
	float r;

	r=random();
	if (r <= 0.3)
		gi.sound (self, CHAN_AUTO, sound_taunt1, 1, ATTN_NONE, 0);
	else if (r <= 0.6)
		gi.sound (self, CHAN_AUTO, sound_taunt2, 1, ATTN_NONE, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_taunt3, 1, ATTN_NONE, 0);
}
*/
//
// stand
//

mframe_t makron_frames_stand []=
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
	{ai_stand, 0, NULL},		// 10
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},		// 20
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},		// 30
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},		// 40
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},		// 50
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}		// 60
};
mmove_t	makron_move_stand = {FRAME_stand201, FRAME_stand260, makron_frames_stand, NULL};
	
void makron_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &makron_move_stand;
}

mframe_t makron_frames_run [] =
{
	{ai_run, 6.75,	makron_step_left},
	{ai_run, 27,		NULL},
	{ai_run, 18,		NULL},
	{ai_run, 18,		NULL},
	{ai_run, 18,		makron_step_right},
	{ai_run, 13.5,	NULL},
	{ai_run, 27,		NULL},
	{ai_run, 20.25,	NULL},
	{ai_run, 13.5,	NULL},
	{ai_run, 27,		NULL}
};
mmove_t	makron_move_run = {FRAME_walk204, FRAME_walk213, makron_frames_run, NULL};

void makron_hit (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_AUTO, sound_hit, 1, ATTN_NORM,0);
	}
}

void makron_popup (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_popup, 1, ATTN_NORM,0);
	}
}

void makron_step_left (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
	}
}

void makron_step_right (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
	}
}

void makron_brainsplorch (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_brainsplorch, 1, ATTN_NORM,0);
	}
}

void makron_prerailgun (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_prerailgun, 1, ATTN_NORM,0);
	}
}


mframe_t makron_frames_walk [] =
{
	{ai_walk, 4.5,	makron_step_left},
	{ai_walk, 18,	NULL},
	{ai_walk, 12,	NULL},
	{ai_walk, 12,	NULL},
	{ai_walk, 12,	makron_step_right},
	{ai_walk, 9,		NULL},
	{ai_walk, 18,	NULL},
	{ai_walk, 13.5,	NULL},
	{ai_walk, 9,	NULL},
	{ai_walk, 18,	NULL}
};
mmove_t	makron_move_walk = {FRAME_walk204, FRAME_walk213, makron_frames_run, NULL};

void makron_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &makron_move_walk;
}

void makron_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &makron_move_stand;
	else
		self->monsterinfo.currentmove = &makron_move_run;
}

mframe_t makron_frames_pain6 [] =
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
	{ai_move,	0,	NULL},		// 10
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	makron_popup},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},		// 20
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}/*makron_taunt*/,
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t makron_move_pain6 = {FRAME_pain601, FRAME_pain627, makron_frames_pain6, makron_run};

mframe_t makron_frames_pain5 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t makron_move_pain5 = {FRAME_pain501, FRAME_pain504, makron_frames_pain5, makron_run};

mframe_t makron_frames_pain4 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t makron_move_pain4 = {FRAME_pain401, FRAME_pain404, makron_frames_pain4, makron_run};

mframe_t makron_frames_death2 [] =
{
	{ai_move,	-15,	NULL},
	{ai_move,	3,	NULL},
	{ai_move,	-12,	NULL},
	{ai_move,	0,	makron_step_left},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			// 10
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	11,	NULL},
	{ai_move,	12,	NULL},
	{ai_move,	11,	makron_step_right},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			// 20
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			// 30
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	5,	NULL},
	{ai_move,	7,	NULL},
	{ai_move,	6,	makron_step_left},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	-1,	NULL},
	{ai_move,	2,	NULL},			// 40
	{ai_move,	0,	NULL},			
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			
	{ai_move,	0,	NULL},			// 50
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	-6,	NULL},
	{ai_move,	-4,	NULL},
	{ai_move,	-6,	makron_step_right},
	{ai_move,	-4,	NULL},
	{ai_move,	-4,	makron_step_left},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			// 60
	{ai_move,	0,	NULL},			
	{ai_move,	0,	NULL},
	{ai_move,	-2,	NULL},
	{ai_move,	-5,	NULL},
	{ai_move,	-3,	makron_step_right},
	{ai_move,	-8,	NULL},
	{ai_move,	-3,	makron_step_left},
	{ai_move,	-7,	NULL},
	{ai_move,	-4,	NULL},
	{ai_move,	-4,	makron_step_right},			// 70
	{ai_move,	-6,	NULL},			
	{ai_move,	-7,	NULL},
	{ai_move,	0,	makron_step_left},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},			// 80
	{ai_move,	0,	NULL},			
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	-2,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	2,	NULL},
	{ai_move,	0,	NULL},			// 90
	{ai_move,	27,	makron_hit},			
	{ai_move,	26,	NULL},
	{ai_move,	0,	makron_brainsplorch},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}			// 95
};
mmove_t makron_move_death2 = {FRAME_death201, FRAME_death295, makron_frames_death2, makron_dead};

mframe_t makron_frames_death3 [] =
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
	{ai_move,	0,	NULL}
};
mmove_t makron_move_death3 = {FRAME_death301, FRAME_death320, makron_frames_death3, NULL};

mframe_t makron_frames_sight [] =
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
	{ai_move,	0,	NULL}
};
mmove_t makron_move_sight= {FRAME_active01, FRAME_active13, makron_frames_sight, makron_run};

mframe_t makron_frames_jetpack [] =
{
	{ai_jetpack,		75,	NULL},
	{ai_jetpack,		75,	NULL},
	{ai_jetpack,		75,	NULL},
	{ai_jetpack,		75,	NULL},
	{ai_jetpack,		75,	NULL},
	{ai_jetpack,		75,	NULL},
	{ai_stopjetpack,	75,	NULL},
};
mmove_t makron_move_jetpack = {FRAME_active01, FRAME_active07, makron_frames_jetpack, makron_attack};

void makronBFG (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int damage;

	if (!self->enemy)
		return;

	if (self->radius_dmg)
		damage = 250 + 20 * self->monsterinfo.skill;
	else
		damage = 175 + 10 * self->monsterinfo.skill;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_BFG], forward, right, start);

	VectorMA(self->enemy->s.origin, 0.3, self->enemy->velocity, vec);
//	VectorCopy (self->enemy->s.origin, vec);
//	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_attack_bfg, 1, ATTN_NORM, 0);
	}
	monster_fire_bfg (self, start, dir, damage, 600 + 8 * self->monsterinfo.skill, 150 + 1 * self->monsterinfo.skill, 200 + 1 * self->monsterinfo.skill, MZ2_MAKRON_BFG);
}	


mframe_t makron_frames_attack3 []=
{
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	makronBFG},		// FIXME: BFG Attack here
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	makron_attack}
};
mmove_t makron_move_attack3 = {FRAME_attak301, FRAME_attak308, makron_frames_attack3, makron_run};

mframe_t makron_frames_attack4[]=
{
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	MakronHyperblaster},		// fire
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	NULL},
	{ai_run_circle,	22,	makron_attack}
};
mmove_t makron_move_attack4 = {FRAME_attak401, FRAME_attak426, makron_frames_attack4, makron_run};

mframe_t makron_frames_attack5[]=
{
	{ai_run_circle,	20,	makron_prerailgun},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	MakronSaveloc},
	{ai_run_circle,	20,	MakronRailgun},		// Fire railgun
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	NULL},
	{ai_run_circle,	20,	makron_attack}
};
mmove_t makron_move_attack5 = {FRAME_attak501, FRAME_attak516, makron_frames_attack5, makron_run};

void MakronSaveloc (edict_t *self)
{
	if (!self->enemy)
		return;
	VectorMA(self->enemy->s.origin, 0.3, self->enemy->velocity, self->pos1);
//	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;
}

// FIXME: He's not firing from the proper Z
void MakronRailgun (edict_t *self)
{
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;
	int damage;

	if (self->radius_dmg)
		damage = 230 + 24 * self->monsterinfo.skill;
	else
		damage = 115 + 12 * self->monsterinfo.skill;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_RAILGUN_1], forward, right, start);

	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	monster_fire_railgun (self, start, dir, damage, 100, MZ2_MAKRON_RAILGUN_1);
}

void MakronHyperblaster (edict_t *self)
{
	vec3_t	v;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;
	int damage;

	if (!self->enemy)
		return;

	if (self->radius_dmg)
		damage = 40 + 6 * self->monsterinfo.skill;
	else
		damage = 20 + 3 * self->monsterinfo.skill;

	flash_number = MZ2_MAKRON_BLASTER_1 + (self->s.frame - FRAME_attak405);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);
	VectorMA(self->enemy->s.origin, 0.3, self->enemy->velocity, vec);
//	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, vec);
	vectoangles (vec, vec);
	dir[0] = vec[0];

	if (self->s.frame <= FRAME_attak413)
		dir[1] = self->s.angles[1] - (1.5 - 0.01 * self->monsterinfo.skill) * (self->s.frame - FRAME_attak413);
	else
		dir[1] = self->s.angles[1] + (1.5 - 0.01 * self->monsterinfo.skill) * (self->s.frame - FRAME_attak421);

	dir[2] = 0;

	AngleVectors (dir, forward, NULL, NULL);

	monster_fire_blaster (self, start, forward, damage, 600 + 15 * self->monsterinfo.skill, MZ2_MAKRON_BLASTER_1, EF_BLASTER);
}


void makron_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames
	if (damage <= 40 + 4 * self->monsterinfo.skill)
		if (random()<0.2 + 0.02 * self->monsterinfo.skill)
			return;

	self->pain_debounce_time = level.time + (6 + 0.3 * self->monsterinfo.skill);


	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain4, 1, ATTN_NORM,0);
	}
	if (damage <= 60 + 10 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &makron_move_pain4;
	} else if (damage <= 110 + 20 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &makron_move_pain5;
	} else {
		self->monsterinfo.currentmove = &makron_move_pain6;
	}
	self->monsterinfo.jumptime = level.time + 3.0;
}

void makron_sight(edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = &makron_move_sight;
}

void makron_attackBfg(edict_t *self)
{
	if (random() < 0.5)
		return;
	self->monsterinfo.currentmove = &makron_move_attack3; // BFG
}
void makron_attackHb(edict_t *self)
{
	if (random() < 0.25)
		return;
	self->monsterinfo.currentmove = &makron_move_attack4; // HB
}
void makron_attackRail(edict_t *self)
{
	self->monsterinfo.currentmove = &makron_move_attack5; // Rail
}

void makron_attack(edict_t *self)
{
	//vec3_t	vec;
	//float	range;
	float	r;

	if ((self->monsterinfo.currentmove == &makron_move_attack3) ||
		(self->monsterinfo.currentmove == &makron_move_attack4) ||
		(self->monsterinfo.currentmove == &makron_move_attack5)) {
		return;
	}
	if ((level.time > self->monsterinfo.jetpacktime + 2.5) && (random() < 0.3)) {
		self->monsterinfo.currentmove = &makron_move_jetpack; // Jetpack
	} else {
		r = random();

		//VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		//range = VectorLength (vec);


		if (r <= 0.3)
			self->monsterinfo.currentmove = &makron_move_attack3; // BFG
		else if (r <= 0.6)
			self->monsterinfo.currentmove = &makron_move_attack4; // HB
		else
			self->monsterinfo.currentmove = &makron_move_attack5; // Rail
	}
}

/*
---
Makron Torso. This needs to be spawned in
---
*/

void makron_torso_think (edict_t *self)
{
	if (++self->s.frame < 365)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 346;
		self->nextthink = level.time + FRAMETIME;
	}
}

void makron_torso (edict_t *ent)
{
	ent->svflags |= SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->s.frame = 346;
	ent->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	ent->think = makron_torso_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	ent->s.sound = gi.soundindex ("makron/spine.wav");
	ent->gib_health = -450 - 60 * ent->monsterinfo.skill;
	gi.linkentity (ent);
}


//
// death
//

void makron_dead (edict_t *self)
{
	VectorSet (self->mins, -60, -60, 0);
	VectorSet (self->maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void makron_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	edict_t *tempent;

	int		n;

	self->s.sound = 0;
	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1 /*4*/; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

/*	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->s.origin[1] -= 84;
	makron_torso (tempent);*/

	self->monsterinfo.currentmove = &makron_move_death2;
	
}

qboolean Makron_CheckAttack (edict_t *self) {
	vec3_t	temp;
	float	chance;
	//qboolean	enemy_infront;
	int			enemy_range;
	float		enemy_yaw;

	if (!self->enemy)
		return false;
/*	if (self->enemy->health > 0) {
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
*/
	//enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);

	self->ideal_yaw = enemy_yaw;

	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}

// missile attack
	if (!self->monsterinfo.attack)
		return false;

	if (level.time < self->monsterinfo.attack_finished)
		return false;

//	if (enemy_range == RANGE_FAR)
//		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND) {
		chance = 0.6;
	} else if (enemy_range == RANGE_MELEE) {
		chance = 0.85;
	} else if (enemy_range == RANGE_NEAR) {
		chance = 0.6;
	} else if (enemy_range == RANGE_MID) {
		chance = 0.4;
	} else {
		chance = 0.2;
	}

	if (random () < chance) {
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 0.3*random();
		return true;
	}

	if (self->flags & FL_FLY) {
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}


//
// monster_makron
//

void MakronPrecache (void)
{
	sound_pain4 = gi.soundindex ("makron/pain3.wav");
//	sound_pain5 = gi.soundindex ("makron/pain2.wav");
//	sound_pain6 = gi.soundindex ("makron/pain1.wav");
	sound_death = gi.soundindex ("makron/death.wav");
	sound_step_left = gi.soundindex ("makron/step1.wav");
	sound_step_right = gi.soundindex ("makron/step2.wav");
	sound_attack_bfg = gi.soundindex ("makron/bfg_fire.wav");
	sound_brainsplorch = gi.soundindex ("makron/brain1.wav");
	sound_prerailgun = gi.soundindex ("makron/rail_up.wav");
	sound_popup = gi.soundindex ("makron/popup.wav");
/*	sound_taunt1 = gi.soundindex ("makron/voice4.wav");
	sound_taunt2 = gi.soundindex ("makron/voice3.wav");
	sound_taunt3 = gi.soundindex ("makron/voice.wav");*/
	sound_hit = gi.soundindex ("makron/bhit.wav");

	gi.modelindex ("models/monsters/boss3/rider/tris.md2");
}

/*QUAKED monster_makron (1 .5 0) (-30 -30 0) (30 30 90) Ambush Trigger_Spawn Sight
*/
void SP_monster_makron (edict_t *self)
{
	MakronPrecache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	VectorSet (self->mins, -30, -30, 0);
	VectorSet (self->maxs, 30, 30, 90);

	self->mass = 500;
	if (deathmatch->value)
		self->health = 1700 + 490 * skill->value;
	else
		self->health = 2500 + 360 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 3450 + 2670 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -300 - 30 * skill->value;
	strcpy(self->monsterinfo.name, "Makron");

	self->pain = makron_pain;
	self->die = makron_die;
	self->monsterinfo.stand = makron_stand;
	self->monsterinfo.walk = makron_walk;
	self->monsterinfo.run = makron_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = makron_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = makron_sight;
	self->monsterinfo.checkattack = Makron_CheckAttack;

	self->monsterinfo.cts_any = 1;
	self->monsterinfo.cts_class1 = 0.20;
	self->monsterinfo.cts_class2 = 0.40;
	self->monsterinfo.cts_class3 = 0.70;

	self->monsterinfo.item_any = 0.95;
	self->monsterinfo.itemmult_class1 = 2.0;
	self->monsterinfo.itemmult_class2 = 0.9;
	self->monsterinfo.itemmult_class3 = 0.7;
	self->monsterinfo.itemmult_class4 = 0.5;

	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 250;

	gi.linkentity (self);

//	self->monsterinfo.currentmove = &makron_move_stand;
	self->monsterinfo.currentmove = &makron_move_sight;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}


/*
=================
MakronSpawn

=================
*/
void MakronSpawn (edict_t *self)
{
	vec3_t		vec;
	edict_t		*player;

	SP_monster_makron (self);

	// jump at player
	player = level.sight_client;
	if (!player)
		return;

	VectorSubtract (player->s.origin, self->s.origin, vec);
	self->s.angles[YAW] = vectoyaw(vec);
	VectorNormalize (vec);
	VectorMA (vec3_origin, 400, vec, self->velocity);
	self->velocity[2] = 200;
	self->groundentity = NULL;
}

/*
=================
MakronToss

Jorg is just about dead, so set up to launch Makron out
=================
*/
void MakronToss (edict_t *self)
{
	edict_t	*ent;
	char command[32];

	sprintf(command, "%d", self->monsterinfo.skill);
	gi.cvar_forceset("skill", command);

	ent = G_Spawn ();
	ent->nextthink = level.time + 0.8;
	ent->think = MakronSpawn;
	ent->radius_dmg = self->radius_dmg;
	ent->target = self->target;
	VectorCopy (self->s.origin, ent->s.origin);
}
