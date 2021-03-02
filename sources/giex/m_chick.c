/*
==============================================================================

chick

==============================================================================
*/

#include "g_local.h"
#include "m_chick.h"

qboolean visible (edict_t *self, edict_t *other);
void ai_run_slide(edict_t *self, float distance);

void chick_stand (edict_t *self);
void chick_run (edict_t *self);
void chick_reslash(edict_t *self);
void chick_rerocket(edict_t *self);
void chick_attack1(edict_t *self);

static int	sound_missile_prelaunch;
//static int	sound_missile_launch;
static int	sound_melee_swing;
//static int	sound_melee_hit;
static int	sound_missile_reload;
static int	sound_death1;
//static int	sound_death2;
//static int	sound_fall_down;
//static int	sound_idle1;
//static int	sound_idle2;
static int	sound_pain1;
//static int	sound_pain2;
//static int	sound_pain3;
static int	sound_sight;
//static int	sound_search;

/*
void ChickMoan (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
}
*/
mframe_t chick_frames_fidget [] =
{
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL}/*ChickMoan*/,
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL},
	{ai_stand, 0,  NULL}
};
mmove_t chick_move_fidget = {FRAME_stand201, FRAME_stand230, chick_frames_fidget, chick_stand};

void chick_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.3)
		self->monsterinfo.currentmove = &chick_move_fidget;
}

mframe_t chick_frames_stand [] =
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
	{ai_stand, 0, chick_fidget},

};
mmove_t chick_move_stand = {FRAME_stand101, FRAME_stand130, chick_frames_stand, NULL};

void chick_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &chick_move_stand;
}

mframe_t chick_frames_start_run [] =
{
	{ai_run, 1,  NULL},
	{ai_run, 0,  NULL},
	{ai_run, 0,	 NULL},
	{ai_run, -1, NULL}, 
	{ai_run, -1, NULL}, 
	{ai_run, 0,  NULL},
	{ai_run, 1,  NULL},
	{ai_run, 3,  NULL},
	{ai_run, 6,	 NULL},
	{ai_run, 3,	 NULL}
};
mmove_t chick_move_start_run = {FRAME_walk01, FRAME_walk10, chick_frames_start_run, chick_run};

mframe_t chick_frames_run [] =
{
	{ai_run, 6,	NULL},
	{ai_run, 8,  NULL},
	{ai_run, 13, NULL},
	{ai_run, 5,  NULL},
	{ai_run, 7,  NULL},
	{ai_run, 4,  NULL},
	{ai_run, 11, NULL},
	{ai_run, 5,  NULL},
	{ai_run, 9,  NULL},
	{ai_run, 7,  NULL}

};

mmove_t chick_move_run = {FRAME_walk11, FRAME_walk20, chick_frames_run, NULL};

mframe_t chick_frames_walk [] =
{
	{ai_walk, 6,	 NULL},
	{ai_walk, 8,  NULL},
	{ai_walk, 13, NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 7,  NULL},
	{ai_walk, 4,  NULL},
	{ai_walk, 11, NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 9,  NULL},
	{ai_walk, 7,  NULL}
};

mmove_t chick_move_walk = {FRAME_walk11, FRAME_walk20, chick_frames_walk, NULL};

void chick_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &chick_move_walk;
}

void chick_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &chick_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == &chick_move_walk ||
		self->monsterinfo.currentmove == &chick_move_start_run)
	{
		self->monsterinfo.currentmove = &chick_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &chick_move_start_run;
	}
}

mframe_t chick_frames_pain1 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t chick_move_pain1 = {FRAME_pain101, FRAME_pain105, chick_frames_pain1, chick_run};

mframe_t chick_frames_pain2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t chick_move_pain2 = {FRAME_pain201, FRAME_pain205, chick_frames_pain2, chick_run};

mframe_t chick_frames_pain3 [] =
{
	{ai_move, 0,		NULL},
	{ai_move, 0,		NULL},
	{ai_move, -6,	NULL},
	{ai_move, 3,		NULL},
	{ai_move, 11,	NULL},
	{ai_move, 3,		NULL},
	{ai_move, 0,		NULL},
	{ai_move, 0,		NULL},
	{ai_move, 4,		NULL},
	{ai_move, 1,		NULL},
	{ai_move, 0,		NULL},
	{ai_move, -3,	NULL},
	{ai_move, -4,	NULL},
	{ai_move, 5,		NULL},
	{ai_move, 7,		NULL},
	{ai_move, -2,	NULL},
	{ai_move, 3,		NULL},
	{ai_move, -5,	NULL},
	{ai_move, -2,	NULL},
	{ai_move, -8,	NULL},
	{ai_move, 2,		NULL}
};
mmove_t chick_move_pain3 = {FRAME_pain301, FRAME_pain321, chick_frames_pain3, chick_run};

void chick_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.1 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}

	if (random() < 0.2 + 0.01 * self->monsterinfo.skill)
		return;

	if (damage <= 10 + self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &chick_move_pain1;
		self->monsterinfo.jumptime = level.time + 2.0;
	} else if (damage <= 25 + 3 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &chick_move_pain2;
		self->monsterinfo.jumptime = level.time + 4.0;
	} else {
		self->monsterinfo.currentmove = &chick_move_pain3;
		self->monsterinfo.jumptime = level.time + 6.0;
	}
}

void chick_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t chick_frames_death2 [] =
{
	{ai_move, -6, NULL},
	{ai_move, 0,  NULL},
	{ai_move, -1,  NULL},
	{ai_move, -5, NULL},
	{ai_move, 0, NULL},
	{ai_move, -1,  NULL},
	{ai_move, -2,  NULL},
	{ai_move, 1,  NULL},
	{ai_move, 10, NULL},
	{ai_move, 2,  NULL},
	{ai_move, 3,  NULL},
	{ai_move, 1,  NULL},
	{ai_move, 2, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 3,  NULL},
	{ai_move, 3,  NULL},
	{ai_move, 1,  NULL},
	{ai_move, -3,  NULL},
	{ai_move, -5, NULL},
	{ai_move, 4, NULL},
	{ai_move, 15, NULL},
	{ai_move, 14, NULL},
	{ai_move, 1, NULL}
};
mmove_t chick_move_death2 = {FRAME_death201, FRAME_death223, chick_frames_death2, chick_dead};

mframe_t chick_frames_death1 [] =
{
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, -7, NULL},
	{ai_move, 4,  NULL},
	{ai_move, 11, NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL},
	{ai_move, 0,  NULL}
	
};
mmove_t chick_move_death1 = {FRAME_death101, FRAME_death112, chick_frames_death1, chick_dead};

void chick_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int n;
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

	self->monsterinfo.currentmove = &chick_move_death1;
	gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
}


void chick_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void chick_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void chick_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t chick_frames_duck [] =
{
	{ai_move, 0, chick_duck_down},
	{ai_move, 1, NULL},
	{ai_move, 4, chick_duck_hold},
	{ai_move, -4,  NULL},
	{ai_move, -5,  chick_duck_up},
	{ai_move, 3, NULL},
	{ai_move, 1,  NULL}
};
mmove_t chick_move_duck = {FRAME_duck01, FRAME_duck07, chick_frames_duck, chick_run};

void chick_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &chick_move_duck;
}

void ChickSlash (edict_t *self) {
	vec3_t	aim;
	int damage;
	if (self->radius_dmg)
		damage = (100 + 12 * self->monsterinfo.skill);
	else
		damage = (50 + 6 * self->monsterinfo.skill);

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 10);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_melee_swing, 1, ATTN_NORM, 0);
	}
	fire_hit (self, aim, damage, 100);
}


void ChickRocket (edict_t *self) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int damage;

	if (!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_CHICK_ROCKET_1], forward, right, start);

	VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, vec);
//	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	if (self->radius_dmg)
		damage = (80 + 14 * self->monsterinfo.skill);
	else
		damage = (40 + 7 * self->monsterinfo.skill);

	monster_fire_rocket (self, start, dir, damage, 650 + 10 * self->monsterinfo.skill, MZ2_CHICK_ROCKET_1);
}

void Chick_PreAttack1 (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_missile_prelaunch, 1, ATTN_NORM, 0);
	}
}

void ChickReload (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_missile_reload, 1, ATTN_NORM, 0);
	}
}


mframe_t chick_frames_start_attack1 [] =
{
	{ai_run_circle, 10,	Chick_PreAttack1},
/*	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},*/
	{ai_run_circle, 12,	NULL},
	{ai_run_circle, 14,	NULL},
	{ai_run_circle, 16,  NULL},
	{ai_run_circle, 17,	NULL},
	{ai_run_circle, 18,	NULL},
	{ai_run_circle, 19,	NULL},
/*	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},*/
	{ai_run_circle, 19,	chick_attack1}
};
mmove_t chick_move_start_attack1 = {FRAME_attak101, FRAME_attak108 /*113*/, chick_frames_start_attack1, NULL};


mframe_t chick_frames_attack1 [] =
{
	{ai_run_circle, 19,	ChickRocket},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 19,	ChickReload},
	{ai_run_circle, 19,	NULL},
/*	{ai_charge, 5,	NULL},
	{ai_charge, 6,	NULL},
	{ai_charge, 6,	NULL},
	{ai_charge, 4,	NULL},*/
	{ai_run_circle, 19,	chick_rerocket}

};
mmove_t chick_move_attack1 = {FRAME_attak114, FRAME_attak123 /*127*/, chick_frames_attack1, NULL};

mframe_t chick_frames_end_attack1 [] =
{
	{ai_run_circle, 19,	NULL},
	{ai_run_circle, 17,	NULL},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 13,	NULL},
	{ai_run_circle, 11,  NULL}
};
mmove_t chick_move_end_attack1 = {FRAME_attak128, FRAME_attak132, chick_frames_end_attack1, chick_run};

void chick_rerocket(edict_t *self)
{
	if (!self->enemy)
		return;
	if (self->enemy->health > 0) {
		if (range (self, self->enemy) > RANGE_MELEE) {
			if ( visible (self, self->enemy) ) {
				if (random() <= 0.6 + 0.01 * self->monsterinfo.skill) {
					self->monsterinfo.currentmove = &chick_move_attack1;
					return;
				}
			}
		}
	}
	self->monsterinfo.currentmove = &chick_move_end_attack1;
}

void chick_attack1(edict_t *self)
{
	self->monsterinfo.currentmove = &chick_move_attack1;
}

mframe_t chick_frames_slash [] =
{
	{ai_charge, 1,	NULL},
	{ai_charge, 7,	ChickSlash},
	{ai_charge, -7,	NULL},
	{ai_charge, 1,	NULL},
	{ai_charge, -1,	NULL},
	{ai_charge, 1,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 1,	NULL},
	{ai_charge, -2,	chick_reslash}
};
mmove_t chick_move_slash = {FRAME_attak204, FRAME_attak212, chick_frames_slash, NULL};

mframe_t chick_frames_end_slash [] =
{
	{ai_charge, -6,	NULL},
	{ai_charge, -1,	NULL},
	{ai_charge, -6,	NULL},
	{ai_charge, 0,	NULL}
};
mmove_t chick_move_end_slash = {FRAME_attak213, FRAME_attak216, chick_frames_end_slash, chick_run};


void chick_reslash(edict_t *self) {
	if (!self->enemy)
		return;
	if (self->enemy->health > 0) {
		if (range (self, self->enemy) == RANGE_MELEE) {
			if (random() <= 0.9) {
				self->monsterinfo.currentmove = &chick_move_slash;
				return;
			} else {
				self->monsterinfo.currentmove = &chick_move_end_slash;
				return;
			}
		}
	}
	self->monsterinfo.currentmove = &chick_move_end_slash;
}

void chick_slash(edict_t *self) {
	if (!self->enemy)
		return;
	self->monsterinfo.currentmove = &chick_move_slash;
}


mframe_t chick_frames_start_slash [] =
{
	{ai_charge, 1,	NULL},
	{ai_charge, 8,	NULL},
	{ai_charge, 3,	NULL}
};
mmove_t chick_move_start_slash = {FRAME_attak201, FRAME_attak203, chick_frames_start_slash, chick_slash};



void chick_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &chick_move_start_slash;
}


void chick_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &chick_move_start_attack1;
}

void chick_sight(edict_t *self, edict_t *other)
{
	if (random() < (0.3 + 0.07 * self->monsterinfo.skill))
		chick_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

/*QUAKED monster_chick (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_chick (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	sound_missile_prelaunch	= gi.soundindex ("chick/chkatck1.wav");	
//	sound_missile_launch	= gi.soundindex ("chick/chkatck2.wav");	
	sound_melee_swing		= gi.soundindex ("chick/chkatck3.wav");	
//	sound_melee_hit			= gi.soundindex ("chick/chkatck4.wav");	
	sound_missile_reload	= gi.soundindex ("chick/chkatck5.wav");	
	sound_death1			= gi.soundindex ("chick/chkdeth1.wav");	
//	sound_death2			= gi.soundindex ("chick/chkdeth2.wav");	
//	sound_fall_down			= gi.soundindex ("chick/chkfall1.wav");	
//	sound_idle1				= gi.soundindex ("chick/chkidle1.wav");	
//	sound_idle2				= gi.soundindex ("chick/chkidle2.wav");	
	sound_pain1				= gi.soundindex ("chick/chkpain1.wav");	
//	sound_pain2				= gi.soundindex ("chick/chkpain2.wav");	
//	sound_pain3				= gi.soundindex ("chick/chkpain3.wav");	
	sound_sight				= gi.soundindex ("chick/chksght1.wav");	
//	sound_search			= gi.soundindex ("chick/chksrch1.wav");	

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 56);

	self->mass = 200;
	self->health = 168 + 30 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 330 + 178 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -140 - 18 * skill->value;
	strcpy(self->monsterinfo.name, "a cyber chick");

	self->pain = chick_pain;
	self->die = chick_die;

	self->monsterinfo.cts_any = 0.18;
	self->monsterinfo.cts_class1 = 0.08;
	self->monsterinfo.cts_class2 = 0.19;
	self->monsterinfo.cts_class3 = 0.41;

	self->monsterinfo.item_any = 0.08;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->monsterinfo.stand = chick_stand;
	self->monsterinfo.walk = chick_walk;
	self->monsterinfo.run = chick_run;
	self->monsterinfo.dodge = NULL; //chick_dodge;
	self->monsterinfo.attack = chick_attack;
	self->monsterinfo.melee = chick_melee;
	self->monsterinfo.sight = chick_sight;
	self->monsterinfo.aggressive = true;
	self->monsterinfo.prefered_range = 320;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &chick_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
