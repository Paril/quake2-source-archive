/*
==============================================================================

brain

==============================================================================
*/

#include "g_local.h"
#include "m_brain.h"


static int	sound_chest_open;
//static int	sound_tentacles_extend;
static int	sound_tentacles_retract;
static int	sound_death;
//static int	sound_idle1;
//static int	sound_idle2;
//static int	sound_idle3;
static int	sound_pain1;
//static int	sound_pain2;
static int	sound_sight;
static int	sound_search;
static int	sound_melee1;
//static int	sound_melee2;
static int	sound_melee3;


void brain_search (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
	}
}


void brain_run (edict_t *self);
void brain_dead (edict_t *self);


//
// STAND
//

mframe_t brain_frames_stand [] =
{
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},

	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},

	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL}
};
mmove_t brain_move_stand = {FRAME_stand01, FRAME_stand30, brain_frames_stand, NULL};

void brain_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &brain_move_stand;
}


//
// IDLE
//

mframe_t brain_frames_idle [] =
{
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},

	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},

	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL},
	{ai_stand,	0,	NULL}
};
mmove_t brain_move_idle = {FRAME_stand31, FRAME_stand60, brain_frames_idle, brain_stand};

void brain_idle (edict_t *self)
{
//	gi.sound (self, CHAN_AUTO, sound_idle3, 1, ATTN_IDLE, 0);
	self->monsterinfo.currentmove = &brain_move_idle;
}


//
// WALK
//
mframe_t brain_frames_walk1 [] =
{
	{ai_walk,	7,	NULL},
	{ai_walk,	2,	NULL},
	{ai_walk,	3,	NULL},
	{ai_walk,	3,	NULL},
	{ai_walk,	1,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	9,	NULL},
	{ai_walk,	-4,	NULL},
	{ai_walk,	-1,	NULL},
	{ai_walk,	2,	NULL}
};
mmove_t brain_move_walk1 = {FRAME_walk101, FRAME_walk111, brain_frames_walk1, NULL};

// walk2 is FUBAR, do not use
#if 0
void brain_walk2_cycle (edict_t *self)
{
	if (random() > 0.1)
		self->monsterinfo.nextframe = FRAME_walk220;
}

mframe_t brain_frames_walk2 [] =
{
	{ai_walk,	3,	NULL},
	{ai_walk,	-2,	NULL},
	{ai_walk,	-4,	NULL},
	{ai_walk,	-3,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	1,	NULL},
	{ai_walk,	12,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	-3,	NULL},
	{ai_walk,	0,	NULL},

	{ai_walk,	-2,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	1,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	10,	NULL},		// Cycle Start

	{ai_walk,	-1,	NULL},
	{ai_walk,	7,	NULL},
	{ai_walk,	0,	NULL},
	{ai_walk,	3,	NULL},
	{ai_walk,	-3,	NULL},
	{ai_walk,	2,	NULL},
	{ai_walk,	4,	NULL},
	{ai_walk,	-3,	NULL},
	{ai_walk,	2,	NULL},
	{ai_walk,	0,	NULL},

	{ai_walk,	4,	brain_walk2_cycle},
	{ai_walk,	-1,	NULL},
	{ai_walk,	-1,	NULL},
	{ai_walk,	-8,	NULL},		
	{ai_walk,	0,	NULL},
	{ai_walk,	1,	NULL},
	{ai_walk,	5,	NULL},
	{ai_walk,	2,	NULL},
	{ai_walk,	-1,	NULL},
	{ai_walk,	-5,	NULL}
};
mmove_t brain_move_walk2 = {FRAME_walk201, FRAME_walk240, brain_frames_walk2, NULL};
#endif

void brain_walk (edict_t *self)
{
//	if (random() <= 0.5)
		self->monsterinfo.currentmove = &brain_move_walk1;
//	else
//		self->monsterinfo.currentmove = &brain_move_walk2;
}



mframe_t brain_frames_defense [] =
{
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
mmove_t brain_move_defense = {FRAME_defens01, FRAME_defens08, brain_frames_defense, NULL};

mframe_t brain_frames_pain3 [] =
{
	{ai_move,	-2,	NULL},
	{ai_move,	2,	NULL},
	{ai_move,	1,	NULL},
	{ai_move,	3,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	-4,	NULL}
};
mmove_t brain_move_pain3 = {FRAME_pain301, FRAME_pain306, brain_frames_pain3, brain_run};

mframe_t brain_frames_pain2 [] =
{
{ai_move,	-2,	NULL},
{ai_move,	0,	NULL},
{ai_move,	0,	NULL},
{ai_move,	0,	NULL},
{ai_move,	0,	NULL},
{ai_move,	3,	NULL},
{ai_move,	1,	NULL},
{ai_move,	-2,	NULL}
};
mmove_t brain_move_pain2 = {FRAME_pain201, FRAME_pain208, brain_frames_pain2, brain_run};

mframe_t brain_frames_pain1 [] =
{
{ai_move,	-6,	NULL},
{ai_move,	-2,	NULL},
{ai_move,	-6,	NULL},
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
{ai_move,	2,	NULL},
{ai_move,	0,	NULL},
{ai_move,	2,	NULL},
{ai_move,	1,	NULL},
{ai_move,	7,	NULL},
{ai_move,	0,	NULL},
{ai_move,	3,	NULL},
{ai_move,	-1,	NULL}
};
mmove_t brain_move_pain1 = {FRAME_pain101, FRAME_pain121, brain_frames_pain1, brain_run};


//
// DUCK
//

void brain_duck_down (edict_t *self)
{
if (self->monsterinfo.aiflags & AI_DUCKED)
	return;
self->monsterinfo.aiflags |= AI_DUCKED;
self->maxs[2] -= 32;
self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void brain_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void brain_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t brain_frames_duck [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	-2,	brain_duck_down},
	{ai_move,	17,	brain_duck_hold},
	{ai_move,	-3,	NULL},
	{ai_move,	-1,	brain_duck_up},
	{ai_move,	-5,	NULL},
	{ai_move,	-6,	NULL},
	{ai_move,	-6,	NULL}
};
mmove_t brain_move_duck = {FRAME_duck01, FRAME_duck08, brain_frames_duck, brain_run};

void brain_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.pausetime = level.time + eta + 0.5;
	self->monsterinfo.currentmove = &brain_move_duck;
}


mframe_t brain_frames_death2 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	9,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t brain_move_death2 = {FRAME_death201, FRAME_death205, brain_frames_death2, brain_dead};

mframe_t brain_frames_death1 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	-2,	NULL},
	{ai_move,	9,	NULL},
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
mmove_t brain_move_death1 = {FRAME_death101, FRAME_death118, brain_frames_death1, brain_dead};


//
// MELEE
//

void brain_swing_right (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_melee1, 1, ATTN_NORM, 0);
	}
}

void brain_hit_right (edict_t *self)
{
	vec3_t	aim;
	int damage;

	if (self->radius_dmg)
		damage = (110 + 16 * self->monsterinfo.skill);
	else
		damage = (55 + 8 * self->monsterinfo.skill);

	VectorSet (aim, MELEE_DISTANCE, self->maxs[0], 8);
	if (fire_hit (self, aim, damage, 40)) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
		}
	}
}

void brain_swing_left (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_melee1, 1, ATTN_NORM, 0);
	}
}

void brain_hit_left (edict_t *self)
{
	vec3_t	aim;
	int damage;

	if (self->radius_dmg)
		damage = (110 + 16 * self->monsterinfo.skill);
	else
		damage = (55 + 8 * self->monsterinfo.skill);

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, damage, 40)) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
		}
	}
}

mframe_t brain_frames_attack1 [] =
{
	{ai_charge,	8,	NULL},
	{ai_charge,	3,	NULL},
	{ai_charge,	5,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	-3,	brain_swing_right},
	{ai_charge,	0,	NULL},
	{ai_charge,	-5,	NULL},
	{ai_charge,	-7,	brain_hit_right},
	{ai_charge,	0,	NULL},
	{ai_charge,	6,	brain_swing_left},
	{ai_charge,	1,	NULL},
	{ai_charge,	2,	brain_hit_left},
	{ai_charge,	-3,	NULL},
	{ai_charge,	6,	NULL},
	{ai_charge,	-1,	NULL},
	{ai_charge,	-3,	NULL},
	{ai_charge,	2,	NULL},
	{ai_charge,	-11,NULL}
};
mmove_t brain_move_attack1 = {FRAME_attak101, FRAME_attak118, brain_frames_attack1, brain_run};

void brain_chest_open (edict_t *self)
{
	self->spawnflags &= ~65536;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_BODY, sound_chest_open, 1, ATTN_NORM, 0);
	}
}

void touchSpores(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void brain_gibrain(edict_t *self)
{
	edict_t *gib;
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;

	if (!self->enemy)
		return;
//	gib = ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 50, GIB_ORGANIC);
	gib = G_Spawn();
	gi.setmodel (gib, "models/objects/gibs/sm_meat/tris.md2");
	VectorCopy (self->s.origin, gib->s.origin);
	gib->s.origin[2] += self->viewheight;
	gib->s.effects |= EF_GIB;
	gib->movetype = MOVETYPE_TOSS;
	gib->owner = self;
	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;
	gib->touch = touchSpores;
	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 2;
	gib->s.sound = gi.soundindex("giex/spells/grfly.wav");
	gib->classid = CI_BRAINGIBRAIN;
	gib->classname = "brain_gibrain";

	if (self->radius_dmg)
		gib->dmg = 32 + 4 * self->monsterinfo.skill;
	else
		gib->dmg = 16 + 2 * self->monsterinfo.skill;

//	AngleVectors (self->s.angles, forward, right, NULL);
//	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	//FIXME : do a spread -225 -75 75 225 degrees around forward
//	VectorCopy (forward, aim);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
//	VectorAdd(end, self->enemy->velocity, end);
	VectorCopy (self->s.origin, start);
	start[2] += self->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*(1200 - 25 * self->monsterinfo.skill);
	u = crandom()*(1000 - 20 * self->monsterinfo.skill);

// Consider distance to target
	u += 80 + (int) ceil((1.3 * VectorLength(aim) * 1.5 + aim[2]) * 0.9);
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	VectorScale (aim, 550 + 20 * self->monsterinfo.skill, gib->velocity);

//	gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/grcast2.wav"), 1, ATTN_NORM, 0);
//	gib->nextthink = level.time + FRAMETIME;
}

void brain_tentacle_attack (edict_t *self)
{
	vec3_t	aim;
	int i;
	int damage;

	if (self->radius_dmg)
		damage = (100 + 12 * self->monsterinfo.skill);
	else
		damage = (50 + 6 * self->monsterinfo.skill);

	for (i = 0; i < 6; i++)
		brain_gibrain(self);

	VectorSet (aim, MELEE_DISTANCE, 0, 8);
	if (fire_hit (self, aim, damage, -200) && self->monsterinfo.skill > 0)
		self->spawnflags |= 65536;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_tentacles_retract, 1, ATTN_NORM, 0);
	}
}

void brain_chest_closed (edict_t *self) {
	self->monsterinfo.attack_finished = level.time + 2.0 * random();
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
}

void brain_check_rechest(edict_t *self) {
	if ((self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy) && random() < 0.6) {
		self->s.frame -= 14;
	} else {
		brain_run(self);
	}
}


mframe_t brain_frames_attack2 [] =
{
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	brain_chest_open},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	brain_tentacle_attack},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	brain_chest_closed},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL},
	{ai_run_circle,	13,	NULL}
};
mmove_t brain_move_attack2 = {FRAME_attak201, FRAME_attak217, brain_frames_attack2, brain_check_rechest};

void brain_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &brain_move_attack1;
}
void brain_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &brain_move_attack2;
}

void brain_sight (edict_t *self, edict_t *other)
{
	if (random() < (0.3 + 0.07 * self->monsterinfo.skill)) {
		if (range(self, other) == RANGE_MELEE)
			brain_melee(self);
		else
			brain_attack(self);
	}
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

//
// RUN
//

mframe_t brain_frames_run [] =
{
	{ai_run,	27,	NULL},
	{ai_run,	6,	NULL},
	{ai_run,	9,	NULL},
	{ai_run,	9,	NULL},
	{ai_run,	3,	NULL},
	{ai_run,	0,	NULL},
	{ai_run,	0,	NULL},
	{ai_run,	30,	NULL},
	{ai_run,	-12,	NULL},
	{ai_run,	-3,	NULL},
	{ai_run,	6,	NULL}
};
mmove_t brain_move_run = {FRAME_walk101, FRAME_walk111, brain_frames_run, NULL};

void brain_run (edict_t *self)
{
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &brain_move_stand;
	else
		self->monsterinfo.currentmove = &brain_move_run;
}


void brain_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.15 * self->monsterinfo.skill;

	r = random();
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	if (r < 0.33) {
		self->monsterinfo.currentmove = &brain_move_pain1;
	} else if (r < 0.66) {
		self->monsterinfo.currentmove = &brain_move_pain2;
	} else {
		self->monsterinfo.currentmove = &brain_move_pain3;
	}
	self->monsterinfo.jumptime = level.time + 4.0;
}

void brain_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}



void brain_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	self->s.effects = 0;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;

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
	if (random() <= 0.5)
		self->monsterinfo.currentmove = &brain_move_death1;
	else
		self->monsterinfo.currentmove = &brain_move_death2;
}

/*QUAKED monster_brain (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_brain (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	sound_chest_open = gi.soundindex ("brain/brnatck1.wav");
//	sound_tentacles_extend = gi.soundindex ("brain/brnatck2.wav");
	sound_tentacles_retract = gi.soundindex ("brain/brnatck3.wav");
	sound_death = gi.soundindex ("brain/brndeth1.wav");
//	sound_idle1 = gi.soundindex ("brain/brnidle1.wav");
//	sound_idle2 = gi.soundindex ("brain/brnidle2.wav");
//	sound_idle3 = gi.soundindex ("brain/brnlens1.wav");
	sound_pain1 = gi.soundindex ("brain/brnpain1.wav");
//	sound_pain2 = gi.soundindex ("brain/brnpain2.wav");
	sound_sight = gi.soundindex ("brain/brnsght1.wav");
	sound_search = gi.soundindex ("brain/brnsrch1.wav");
	sound_melee1 = gi.soundindex ("brain/melee1.wav");
//	sound_melee2 = gi.soundindex ("brain/melee2.wav");
	sound_melee3 = gi.soundindex ("brain/melee3.wav");

	self->movetype = MOVETYPE_STEP;
	self->yaw_speed = 18;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/brain/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->mass = 400;
	self->health = 440 + 55 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	self->monsterinfo.power_armor_power = (450 + 180 * skill->value) * getMonsterBonus(self);
	self->monsterinfo.level = 960 + 715 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -190 - 24 * skill->value;
	strcpy(self->monsterinfo.name, "a brain");

	self->pain = brain_pain;
	self->die = brain_die;

	self->monsterinfo.cts_any = 0.28;
	self->monsterinfo.cts_class1 = 0.09;
	self->monsterinfo.cts_class2 = 0.21;
	self->monsterinfo.cts_class3 = 0.42;

	self->monsterinfo.item_any = 0.12;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->monsterinfo.stand = brain_stand;
	self->monsterinfo.walk = brain_walk;
	self->monsterinfo.run = brain_run;
	self->monsterinfo.dodge = NULL; //brain_dodge;
	self->monsterinfo.attack = brain_attack;
	self->monsterinfo.melee = brain_melee;
	self->monsterinfo.sight = brain_sight;
	self->monsterinfo.search = brain_search;
	self->monsterinfo.idle = brain_idle;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 120;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &brain_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
