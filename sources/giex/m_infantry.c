/*
==============================================================================

INFANTRY

==============================================================================
*/

#include "g_local.h"
#include "m_infantry.h"

void InfantryMachineGun (edict_t *self);
void ai_run_slide(edict_t *self, float distance);

static int	sound_pain1;
//static int	sound_pain2;
static int	sound_die1;
//static int	sound_die2;

//static int	sound_gunshot;
static int	sound_weapon_cock;
static int	sound_punch_swing;
static int	sound_punch_hit;
static int	sound_sight;
//static int	sound_search;
//static int	sound_idle;


mframe_t infantry_frames_stand [] =
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
	{ai_stand, 0, NULL}
};
mmove_t infantry_move_stand = {FRAME_stand50, FRAME_stand71, infantry_frames_stand, NULL};

void infantry_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_stand;
}

/*
mframe_t infantry_frames_fidget [] =
{
	{ai_stand, 1,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 3,  NULL},
	{ai_stand, 6,  NULL},
	{ai_stand, 3,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -1, NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -2, NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, -1, NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -1, NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -1, NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 1,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -1, NULL},
	{ai_stand, -1, NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, -3, NULL},
	{ai_stand, -2, NULL},
	{ai_stand, -3, NULL},
	{ai_stand, -3, NULL},
	{ai_stand, -2, NULL}
};
mmove_t infantry_move_fidget = {FRAME_stand01, FRAME_stand49, infantry_frames_fidget, infantry_stand};
*/
/*
void infantry_fidget (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_fidget;
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
*/

mframe_t infantry_frames_walk [] =
{
	{ai_walk, 5,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 6,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 5,  NULL}
};
mmove_t infantry_move_walk = {FRAME_walk03, FRAME_walk14, infantry_frames_walk, NULL};

void infantry_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_walk;
}

mframe_t infantry_frames_run [] =
{
	{ai_run, 10, NULL},
	{ai_run, 20, NULL},
	{ai_run, 5,  NULL},
	{ai_run, 7,  NULL},
	{ai_run, 30, NULL},
	{ai_run, 35, NULL},
	{ai_run, 2,  NULL},
	{ai_run, 6,  NULL}
};
mmove_t infantry_move_run = {FRAME_run01, FRAME_run08, infantry_frames_run, NULL};

void infantry_run (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_run;
	return;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &infantry_move_stand;
	else
		self->monsterinfo.currentmove = &infantry_move_run;
}


mframe_t infantry_frames_pain1 [] =
{
	{ai_move, -3, NULL},
	{ai_move, -2, NULL},
	{ai_move, -1, NULL},
	{ai_move, -2, NULL},
	{ai_move, -1, NULL},
	{ai_move, 1,  NULL},
	{ai_move, -1, NULL},
	{ai_move, 1,  NULL},
	{ai_move, 6,  NULL},
	{ai_move, 2,  NULL}
};
mmove_t infantry_move_pain1 = {FRAME_pain101, FRAME_pain110, infantry_frames_pain1, infantry_run};

mframe_t infantry_frames_pain2 [] =
{
	{ai_move, -3, NULL},
	{ai_move, -3, NULL},
	{ai_move, 0,  NULL},
	{ai_move, -1, NULL},
	{ai_move, -2, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 2,  NULL},
	{ai_move, 5,  NULL},
	{ai_move, 2,  NULL}
};
mmove_t infantry_move_pain2 = {FRAME_pain201, FRAME_pain210, infantry_frames_pain2, infantry_run};

void infantry_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.5 * self->monsterinfo.skill;
	
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	n = rand() % 2;
	if (n == 0) {
		self->monsterinfo.currentmove = &infantry_move_pain1;
	} else {
		self->monsterinfo.currentmove = &infantry_move_pain2;
	}
	self->monsterinfo.jumptime = level.time + 4.0;
}


vec3_t	aimangles[] =
{
	{0.0, 5.0, 0.0},
	{10.0, 15.0, 0.0},
	{20.0, 25.0, 0.0},
	{25.0, 35.0, 0.0},
	{30.0, 40.0, 0.0},
	{30.0, 45.0, 0.0},
	{25.0, 50.0, 0.0},
	{20.0, 40.0, 0.0},
	{15.0, 35.0, 0.0},
	{40.0, 35.0, 0.0},
	{70.0, 35.0, 0.0},
	{90.0, 35.0, 0.0}
};

void InfantryMachineGun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	int		flash_number;
	float spreadmult = 5 - 0.1 * self->monsterinfo.aim_frames;
	float aimmult = -0.35 + 0.0125 * self->monsterinfo.aim_frames;
	if (spreadmult < 1) {
		spreadmult = 1;
	}
	if (aimmult > 0) {
		aimmult = 0;
	}

	if (self->s.frame == FRAME_attak111) {
		flash_number = MZ2_INFANTRY_MACHINEGUN_1;
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

		if (self->enemy) {
			VectorMA (self->enemy->s.origin, aimmult, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight;
			VectorSubtract (target, start, forward);
			VectorNormalize (forward);
		} else {
			AngleVectors (self->s.angles, forward, right, NULL);
		}
		self->monsterinfo.aim_frames++;
	}
	else
	{
		flash_number = MZ2_INFANTRY_MACHINEGUN_2 + (self->s.frame - FRAME_death211);

		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

		VectorSubtract (self->s.angles, aimangles[flash_number-MZ2_INFANTRY_MACHINEGUN_2], vec);
		AngleVectors (vec, forward, NULL, NULL);
	}

	if (self->radius_dmg)
		monster_fire_bullet (self, start, forward, (int) ceil(5.6 + 2.4 * self->monsterinfo.skill), 4, DEFAULT_BULLET_HSPREAD * spreadmult, DEFAULT_BULLET_VSPREAD * spreadmult, flash_number);
	else
		monster_fire_bullet (self, start, forward, (int) ceil(2.8 + 1.2 * self->monsterinfo.skill), 4, DEFAULT_BULLET_HSPREAD * spreadmult, DEFAULT_BULLET_VSPREAD * spreadmult, flash_number);
}
void InfantrySnipe (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	int		flash_number;

	flash_number = MZ2_INFANTRY_MACHINEGUN_1;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy) {
		VectorMA (self->enemy->s.origin, -0.05, self->enemy->velocity, target);
		target[2] += self->enemy->viewheight;
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	} else {
		AngleVectors (self->s.angles, forward, right, NULL);
	}

	if (self->radius_dmg)
		monster_fire_railgun(self, start, forward, 250 + 150 * self->monsterinfo.skill, 4, flash_number);
	else
		monster_fire_railgun(self, start, forward, 150 + 75 * self->monsterinfo.skill, 4, flash_number);
}

void infantry_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t infantry_frames_death1 [] =
{
	{ai_move, -4, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, -1, NULL},
	{ai_move, -4, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, -1, NULL},
	{ai_move, 3,  NULL},
	{ai_move, 1,  NULL},
	{ai_move, 1,  NULL},
	{ai_move, -2, NULL},
	{ai_move, 2,  NULL},
	{ai_move, 2,  NULL},
	{ai_move, 9,  NULL},
	{ai_move, 9,  NULL},
	{ai_move, 5,  NULL},
	{ai_move, -3, NULL},
	{ai_move, -3, NULL}
};
mmove_t infantry_move_death1 = {FRAME_death101, FRAME_death120, infantry_frames_death1, infantry_dead};

// Off with his head
mframe_t infantry_frames_death2 [] =
{
	{ai_move, 0,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 5,   NULL},
	{ai_move, -1,  NULL},
	{ai_move, 0,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 1,   NULL},
	{ai_move, 4,   NULL},
	{ai_move, 3,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -2,  InfantryMachineGun},
	{ai_move, -2,  InfantryMachineGun},
	{ai_move, -3,  InfantryMachineGun},
	{ai_move, -1,  InfantryMachineGun},
	{ai_move, -2,  InfantryMachineGun},
	{ai_move, 0,   InfantryMachineGun},
	{ai_move, 2,   InfantryMachineGun},
	{ai_move, 2,   InfantryMachineGun},
	{ai_move, 3,   InfantryMachineGun},
	{ai_move, -10, InfantryMachineGun},
	{ai_move, -7,  InfantryMachineGun},
	{ai_move, -8,  InfantryMachineGun},
	{ai_move, -6,  NULL},
	{ai_move, 4,   NULL},
	{ai_move, 0,   NULL}
};
mmove_t infantry_move_death2 = {FRAME_death201, FRAME_death225, infantry_frames_death2, infantry_dead};

mframe_t infantry_frames_death3 [] =
{
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL},
	{ai_move, -6,  NULL},
	{ai_move, -11, NULL},
	{ai_move, -3,  NULL},
	{ai_move, -11, NULL},
	{ai_move, 0,   NULL},
	{ai_move, 0,   NULL}
};
mmove_t infantry_move_death3 = {FRAME_death301, FRAME_death309, infantry_frames_death3, infantry_dead};


void infantry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
	n = rand() % 3;
	if (n == 0) {
		self->monsterinfo.currentmove = &infantry_move_death1;
	} else if (n == 1) {
		self->monsterinfo.currentmove = &infantry_move_death2;
	} else {
		self->monsterinfo.currentmove = &infantry_move_death3;
	}
}


void infantry_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void infantry_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void infantry_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t infantry_frames_duck [] =
{
	{ai_move, -2, infantry_duck_down},
	{ai_move, -5, infantry_duck_hold},
	{ai_move, 3,  NULL},
	{ai_move, 4,  infantry_duck_up},
	{ai_move, 0,  NULL}
};
mmove_t infantry_move_duck = {FRAME_duck01, FRAME_duck05, infantry_frames_duck, infantry_run};

void infantry_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &infantry_move_duck;
}


void infantry_cock_gun (edict_t *self)
{
	int		n;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_weapon_cock, 1, ATTN_NORM, 0);
	}
	n = (rand() & 15) + 10;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
	self->monsterinfo.aim_frames = 0;
	self->s.frame += 4;
}

void infantry_fire (edict_t *self)
{
	InfantryMachineGun (self);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else {
		self->s.frame--;
//		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		self->monsterinfo.attack_finished = level.time + 0.6 + 0.8 * random();
	}
}

mframe_t infantry_frames_attack1 [] =
{
/*	{ai_run_slide, 4,  NULL},
	{ai_run_slide, -1, NULL},*/
	{ai_run_circle, 18, NULL},
	{ai_run_circle, 18,  infantry_cock_gun},
	{ai_run_circle, 19, NULL},
	{ai_run_circle, 19,  NULL},
	{ai_run_circle, 22,  NULL},
	{ai_run_circle, 22,  NULL},
	{ai_run_circle, 23, NULL},
	{ai_run_circle, 22, NULL},
	{ai_run_circle, 18,  infantry_fire},
	{ai_run_circle, 22,  NULL},
	{ai_run_circle, 23, NULL},
	{ai_run_circle, 22, NULL},
	{ai_run_circle, 21, NULL}
};
mmove_t infantry_move_attack1 = {FRAME_attak103, FRAME_attak115, infantry_frames_attack1, infantry_run};


void infantry_start_snipe (edict_t *self)
{
//	gi.dprintf("I C J00!\n");
	gi.sound (self->enemy, CHAN_WEAPON, sound_weapon_cock, 1, ATTN_NORM, 0);
	self->monsterinfo.pausetime = level.time + 2.0;
//	self->monsterinfo.currentmove = &infantry_move_snipe;
	M_CheckAttack(self);
}
void infantry_snipe_delay (edict_t *self)
{
	M_ChangeYaw(self);
	if (level.time < self->monsterinfo.pausetime) {
		self->s.frame--;
	}
}
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void infantry_snipe (edict_t *self)
{
	vec3_t	origin, angles;
//	gi.dprintf("SNIPAH!\n");

	InfantrySnipe(self);
	SelectSpawnPoint (self, origin, angles);
	VectorCopy(origin, self->s.origin);
	VectorCopy(angles, self->s.angles);
//	self->enemy = NULL;
}

mframe_t infantry_frames_snipe [] =
{
	{ai_charge, 0,  infantry_start_snipe},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0,  NULL},
	{ai_charge, 0,  NULL},
	{ai_charge, 0,  NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}, //extra
	{ai_charge, 0, infantry_snipe_delay},
	{ai_charge, 0, infantry_snipe},
//	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}
};
mmove_t infantry_move_snipe = {FRAME_attak101, FRAME_attak115, infantry_frames_snipe, infantry_run};


void infantry_swing (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_punch_swing, 1, ATTN_NORM, 0);
	}
}

void infantry_smack (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, (25 + (rand() % 5) + 2 * self->monsterinfo.skill), 50)) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_WEAPON, sound_punch_hit, 1, ATTN_NORM, 0);
		}
	}
}

mframe_t infantry_frames_attack2 [] =
{
	{ai_charge, 3, NULL},
	{ai_charge, 6, NULL},
	{ai_charge, 0, infantry_swing},
	{ai_charge, 8, NULL},
	{ai_charge, 5, NULL},
	{ai_charge, 8, infantry_smack},
	{ai_charge, 6, NULL},
	{ai_charge, 3, NULL},
};
mmove_t infantry_move_attack2 = {FRAME_attak201, FRAME_attak208, infantry_frames_attack2, infantry_run};

void infantry_attack(edict_t *self)
{
	if (self->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
		self->monsterinfo.currentmove = &infantry_move_snipe;
	} else {
		self->monsterinfo.currentmove = &infantry_move_attack1;
	}
}

void infantry_sight (edict_t *self, edict_t *other)
{
	if (self->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
		infantry_attack(self);
	} else {
		if (random() < (0.6 + 0.02 * self->monsterinfo.skill))
			infantry_attack(self);
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
		}
	}
}

/*QUAKED monster_infantry (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_infantry (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	sound_pain1 = gi.soundindex ("infantry/infpain1.wav");
//	sound_pain2 = gi.soundindex ("infantry/infpain2.wav");
	sound_die1 = gi.soundindex ("infantry/infdeth1.wav");
//	sound_die2 = gi.soundindex ("infantry/infdeth2.wav");

//	sound_gunshot = gi.soundindex ("infantry/infatck1.wav");
	sound_weapon_cock = gi.soundindex ("infantry/infatck3.wav");
	sound_punch_swing = gi.soundindex ("infantry/infatck2.wav");
	sound_punch_hit = gi.soundindex ("infantry/melee2.wav");
	
	sound_sight = gi.soundindex ("infantry/infsght1.wav");
//	sound_search = gi.soundindex ("infantry/infsrch1.wav");
//	sound_idle = gi.soundindex ("infantry/infidle1.wav");
	

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->mass = 200;
	self->health = 120 + 15 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 375 + 159 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -120 - 18 * skill->value;
	strcpy(self->monsterinfo.name, "an infantry soldier");

	self->pain = infantry_pain;
	self->die = infantry_die;

	self->monsterinfo.stand = infantry_stand;
	self->monsterinfo.walk = infantry_walk;
	self->monsterinfo.run = infantry_run;
	self->monsterinfo.dodge = NULL; // infantry_dodge;
	self->monsterinfo.attack = infantry_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = infantry_sight;
	self->monsterinfo.idle = NULL/*infantry_fidget*/;
	self->monsterinfo.aggressive = true;
	self->monsterinfo.prefered_range = 120;

	self->monsterinfo.cts_any = 0.28;
	self->monsterinfo.cts_class1 = 0.06;
	self->monsterinfo.cts_class2 = 0.14;
	self->monsterinfo.cts_class3 = 0.40;

	self->monsterinfo.item_any = 0.07;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &infantry_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
