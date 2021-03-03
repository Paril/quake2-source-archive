/*
==============================================================================

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"


static int	sound_pain;
//static int	sound_pain2;
static int	sound_death;
//static int	sound_idle;
static int	sound_open;
static int	sound_search;
static int	sound_sight;

/*
void gunner_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
*/
void gunner_search (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
	}
}


qboolean visible (edict_t *self, edict_t *other);
void GunnerGrenade (edict_t *self);
void GunnerFire (edict_t *self);
void gunner_fire_chain(edict_t *self);
void gunner_refire_chain(edict_t *self);


void gunner_stand (edict_t *self);

mframe_t gunner_frames_fidget [] =
{
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}/*gunner_idlesound*/,
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
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, gunner_fidget},

	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, gunner_fidget},

	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, gunner_fidget}
};
mmove_t	gunner_move_stand = {FRAME_stand01, FRAME_stand30, gunner_frames_stand, NULL};

void gunner_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &gunner_move_stand;
}


mframe_t gunner_frames_walk [] =
{
	{ai_walk, 0, NULL},
	{ai_walk, 3, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 7, NULL},
	{ai_walk, 2, NULL},
	{ai_walk, 6, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 2, NULL},
	{ai_walk, 7, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 7, NULL},
	{ai_walk, 4, NULL}
};
mmove_t gunner_move_walk = {FRAME_walk07, FRAME_walk19, gunner_frames_walk, NULL};

void gunner_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_walk;
}

mframe_t gunner_frames_run [] =
{
	{ai_run, 26, NULL},
	{ai_run, 9,  NULL},
	{ai_run, 9,  NULL},
	{ai_run, 9,  NULL},
	{ai_run, 15, NULL},
	{ai_run, 10, NULL},
	{ai_run, 13, NULL},
	{ai_run, 6,  NULL}
};

mmove_t gunner_move_run = {FRAME_run01, FRAME_run08, gunner_frames_run, NULL};

void gunner_run (edict_t *self) {
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gunner_move_stand;
	else
		self->monsterinfo.currentmove = &gunner_move_run;
}

void gunner_checkrerun (edict_t *self) {
	if ((random() < 0.5 + 0.01 * self->monsterinfo.skill) && (self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy)) {
		self->s.frame -= 7;
	} else {
		self->monsterinfo.aim_frames = 0;
		gunner_run(self);
	}
}

mframe_t gunner_frames_runandshoot [] =
{
	{ai_run_circle, 15, GunnerFire},
	{ai_run_circle, 15, GunnerFire},
	{ai_run_circle, 15, GunnerFire},
	{ai_run_circle, 15, GunnerFire},
	{ai_run_circle, 15, GunnerFire},
	{ai_run_circle, 15, GunnerFire}
};

mmove_t gunner_move_runandshoot = {FRAME_runs01, FRAME_runs06, gunner_frames_runandshoot, gunner_checkrerun};

void gunner_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_runandshoot;
}

mframe_t gunner_frames_pain3 [] =
{
	{ai_move, -3, NULL},
	{ai_move, 1,	 NULL},
	{ai_move, 1,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 1,	 NULL}
};
mmove_t gunner_move_pain3 = {FRAME_pain301, FRAME_pain305, gunner_frames_pain3, gunner_run};

mframe_t gunner_frames_pain2 [] =
{
	{ai_move, -2, NULL},
	{ai_move, 11, NULL},
	{ai_move, 6,	 NULL},
	{ai_move, 2,	 NULL},
	{ai_move, -1, NULL},
	{ai_move, -7, NULL},
	{ai_move, -2, NULL},
	{ai_move, -7, NULL}
};
mmove_t gunner_move_pain2 = {FRAME_pain201, FRAME_pain208, gunner_frames_pain2, gunner_run};

mframe_t gunner_frames_pain1 [] =
{
	{ai_move, 2,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, -5, NULL},
	{ai_move, 3,	 NULL},
	{ai_move, -1, NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 1,	 NULL},
	{ai_move, 1,	 NULL},
	{ai_move, 2,	 NULL},
	{ai_move, 1,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, -2, NULL},
	{ai_move, -2, NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL}
};
mmove_t gunner_move_pain1 = {FRAME_pain101, FRAME_pain118, gunner_frames_pain1, gunner_run};

void gunner_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 4 + 0.1 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}

	if (damage <= 20 + 2 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &gunner_move_pain3;
		self->monsterinfo.jumptime = level.time + 3.0;
	} else if (damage <= 50 + 5 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &gunner_move_pain2;
		self->monsterinfo.jumptime = level.time + 5.0;
	} else {
		self->monsterinfo.currentmove = &gunner_move_pain1;
		self->monsterinfo.jumptime = level.time + 7.0;
	}
}

void gunner_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t gunner_frames_death [] =
{
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, -7, NULL},
	{ai_move, -3, NULL},
	{ai_move, -5, NULL},
	{ai_move, 8,	 NULL},
	{ai_move, 6,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL}
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
	if (self->enemy && random() > (0.6 - 0.1 * self->monsterinfo.skill) && infront_aim(self, self->enemy))
		GunnerGrenade (self);

	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 0.3;
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
	if (self->enemy && random() > (0.8 - 0.1 * self->monsterinfo.skill) && infront_aim(self, self->enemy))
		GunnerGrenade (self);

	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t gunner_frames_duck [] =
{
	{ai_move, 1,  gunner_duck_down},
	{ai_move, 1,  NULL},
	{ai_move, 1,  gunner_duck_hold},
	{ai_move, 0,  NULL},
	{ai_move, -1, NULL},
	{ai_move, -1, NULL},
	{ai_move, 0,  gunner_duck_up},
	{ai_move, -1, NULL}
};
mmove_t	gunner_move_duck = {FRAME_duck01, FRAME_duck08, gunner_frames_duck, gunner_run};

void gunner_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.65)
		return;

	M_ChangeYaw(self);

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &gunner_move_duck;
}

void gunner_opengun (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_open, 1, ATTN_IDLE, 0);
	}
}

void GunnerFire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int		flash_number;
	int damage;
	float spreadmult = 5 - 0.1 * self->monsterinfo.aim_frames;
	float aimmult = -0.35 + 0.0125 * self->monsterinfo.aim_frames;
	if (spreadmult < 1) {
		spreadmult = 1;
	}
	if (aimmult > 0) {
		aimmult = 0;
	}

	if (!self->enemy)
		return;
	if (self->radius_dmg)
		damage = (int) ceil(8.4 + 2.4 * self->monsterinfo.skill);
	else
		damage = (int) ceil(4.2 + 1.2 * self->monsterinfo.skill);

	flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, aimmult, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_bullet (self, start, aim, damage, 4, DEFAULT_BULLET_HSPREAD * spreadmult, DEFAULT_BULLET_VSPREAD * spreadmult, flash_number);
	self->monsterinfo.aim_frames++;
}
/*
void GunnerReFire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int		flash_number;
	int damage;

	if (!self->enemy)
		return;

	if (self->radius_dmg)
		damage = (int) ceil(10 + 2.2 * self->monsterinfo.skill);
	else
		damage = (int) ceil(5.0 + 1.1 * self->monsterinfo.skill);

	flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, (-0.4 + 0.09 * self->monsterinfo.skill), self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_bullet (self, start, aim, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

mframe_t gunner_frames_attack_chain [] =
{
	/*
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	//
	{ai_charge, 0, gunner_opengun},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}
};
mmove_t gunner_move_attack_chain = {FRAME_attak209, FRAME_attak215, gunner_frames_attack_chain, gunner_fire_chain};

mframe_t gunner_frames_fire_chain [] =
{
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire},
	{ai_charge,   0, GunnerFire}
};
mmove_t gunner_move_fire_chain = {FRAME_attak216, FRAME_attak223, gunner_frames_fire_chain, gunner_refire_chain};

mframe_t gunner_frames_refire_chain [] =
{
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire},
	{ai_charge,   0, GunnerReFire}
};
mmove_t gunner_move_refire_chain = {FRAME_attak216, FRAME_attak223, gunner_frames_refire_chain, gunner_refire_chain};

mframe_t gunner_frames_endfire_chain [] =
{
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}
};
mmove_t gunner_move_endfire_chain = {FRAME_attak224, FRAME_attak230, gunner_frames_endfire_chain, gunner_run};
*/
void GunnerGrenade (edict_t *self) {
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int		flash_number;
	int damage;

	if (!self->enemy)
		return;

	if (self->radius_dmg)
		damage = (int) ceil(150 + 18 * self->monsterinfo.skill);
	else
		damage = (int) ceil(75 + 9 * self->monsterinfo.skill);

	if (!(self->monsterinfo.aiflags & AI_DUCKED)) {
		if (!visible(self, self->enemy)) {
			gunner_run(self);
			return;
		}
		if (range (self, self->enemy) == RANGE_MELEE) {
			self->monsterinfo.currentmove = &gunner_move_runandshoot;
		}
	}

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

//	VectorCopy (forward, aim);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*(1200 - 18 * self->monsterinfo.skill);
	u = crandom()*(600 - 9 * self->monsterinfo.skill);

// Consider distance to target
	u += (int) ceil((VectorLength(aim) + 1.2 * aim[2]) * 0.25);
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	monster_fire_grenade (self, start, aim, damage, 600 + 15 * self->monsterinfo.skill, flash_number);
}

mframe_t gunner_frames_attack_grenade [] =
{
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, GunnerGrenade},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, GunnerGrenade},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, GunnerGrenade},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, GunnerGrenade},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL},
	{ai_run_circle, 15, NULL}
};
mmove_t gunner_move_attack_grenade = {FRAME_attak101, FRAME_attak121, gunner_frames_attack_grenade, gunner_run};

void gunner_attack(edict_t *self) {
	if (!self->enemy)
		return;
	if (range (self, self->enemy) <= RANGE_CLOSE) {
		self->monsterinfo.currentmove = &gunner_move_runandshoot;
	} else {
		if (random() < 0.6)
			self->monsterinfo.currentmove = &gunner_move_attack_grenade;
		else
			self->monsterinfo.currentmove = &gunner_move_runandshoot;
	}
}

void gunner_sight (edict_t *self, edict_t *other) {
	if (random() < (0.3 + 0.04 * self->monsterinfo.skill))
		gunner_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}
/*
void gunner_fire_chain(edict_t *self) {
	if (!self->enemy)
		return;
	self->monsterinfo.currentmove = &gunner_move_fire_chain;
}

void gunner_refire_chain(edict_t *self) {
	float chance = 0.4 + 0.01 * self->monsterinfo.skill;
	if (chance > 0.6)
		chance = 0.6;

	if (self->enemy && (self->enemy->health > 0)) {
		if ( visible (self, self->enemy) ) {
			if (random() <= chance)
			{
				self->monsterinfo.attack_finished = level.time + 0.5;
				self->monsterinfo.currentmove = &gunner_move_refire_chain;
				return;
			}
		}
	}
	self->monsterinfo.currentmove = &gunner_move_endfire_chain;
}
*/
/*QUAKED monster_gunner (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_gunner (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	sound_death = gi.soundindex ("gunner/death1.wav");	
	sound_pain = gi.soundindex ("gunner/gunpain2.wav");	
//	sound_pain2 = gi.soundindex ("gunner/gunpain1.wav");	
//	sound_idle = gi.soundindex ("gunner/gunidle1.wav");	
	sound_open = gi.soundindex ("gunner/gunatck1.wav");	
	sound_search = gi.soundindex ("gunner/gunsrch1.wav");	
	sound_sight = gi.soundindex ("gunner/sight1.wav");	

	gi.soundindex ("gunner/gunatck2.wav");
	gi.soundindex ("gunner/gunatck3.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/gunner/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 175;
	self->gib_health = -70;
	self->mass = 200;
	self->health = 325 + 55 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 960 + 690 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -160 - 25 * skill->value;
	strcpy(self->monsterinfo.name, "a gunner");

	self->monsterinfo.cts_any = 0.31;
	self->monsterinfo.cts_class1 = 0.07;
	self->monsterinfo.cts_class2 = 0.16;
	self->monsterinfo.cts_class3 = 0.37;

	self->monsterinfo.item_any = 0.15;
	self->monsterinfo.itemmult_class1 = 1.2;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 0.9;
	self->monsterinfo.itemmult_class4 = 0.8;

	self->pain = gunner_pain;
	self->die = gunner_die;

	self->monsterinfo.stand = gunner_stand;
	self->monsterinfo.walk = gunner_walk;
	self->monsterinfo.run = gunner_run;
	self->monsterinfo.dodge = gunner_dodge;
	self->monsterinfo.attack = gunner_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = gunner_sight;
	self->monsterinfo.search = gunner_search;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 200;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &gunner_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	self->monsterinfo.pausetime = 0;

	walkmonster_start (self);
}
