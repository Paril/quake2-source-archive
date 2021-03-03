/*
==============================================================================

BERSERK

==============================================================================
*/

#include "g_local.h"
#include "m_berserk.h"

void ai_run_slide(edict_t *self, float distance);

static int sound_pain;
static int sound_die;
//static int sound_idle;
static int sound_punch;
static int sound_sight;
//static int sound_search;

void berserk_sight (edict_t *self, edict_t *other) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

/*void berserk_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}
*/

void berserk_fidget (edict_t *self);
mframe_t berserk_frames_stand [] =
{
	{ai_stand, 0, berserk_fidget},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}
};
mmove_t berserk_move_stand = {FRAME_stand1, FRAME_stand5, berserk_frames_stand, NULL};

void berserk_stand (edict_t *self) {
	self->monsterinfo.currentmove = &berserk_move_stand;
}

mframe_t berserk_frames_stand_fidget [] =
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
	{ai_stand, 0, NULL}
};
mmove_t berserk_move_stand_fidget = {FRAME_standb1, FRAME_standb20, berserk_frames_stand_fidget, berserk_stand};

void berserk_fidget (edict_t *self) {
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() > 0.15)
		return;

	self->monsterinfo.currentmove = &berserk_move_stand_fidget;
//	gi.sound (self, CHAN_WEAPON, sound_idle, 1, ATTN_IDLE, 0);
}


mframe_t berserk_frames_walk [] =
{
	{ai_walk, 9.1, NULL},
	{ai_walk, 6.3, NULL},
	{ai_walk, 4.9, NULL},
	{ai_walk, 6.7, NULL},
	{ai_walk, 6.0, NULL},
	{ai_walk, 8.2, NULL},
	{ai_walk, 7.2, NULL},
	{ai_walk, 6.1, NULL},
	{ai_walk, 4.9, NULL},
	{ai_walk, 4.7, NULL},
	{ai_walk, 4.7, NULL},
	{ai_walk, 4.8, NULL}
};
mmove_t berserk_move_walk = {FRAME_walkc1, FRAME_walkc11, berserk_frames_walk, NULL};

void berserk_walk (edict_t *self) {
	self->monsterinfo.currentmove = &berserk_move_walk;
}

/*

  *****************************
  SKIPPED THIS FOR NOW!
  *****************************

   Running -> Arm raised in air

void()	berserk_runb1	=[	$r_att1 ,	berserk_runb2	] {ai_run(21);};
void()	berserk_runb2	=[	$r_att2 ,	berserk_runb3	] {ai_run(11);};
void()	berserk_runb3	=[	$r_att3 ,	berserk_runb4	] {ai_run(21);};
void()	berserk_runb4	=[	$r_att4 ,	berserk_runb5	] {ai_run(25);};
void()	berserk_runb5	=[	$r_att5 ,	berserk_runb6	] {ai_run(18);};
void()	berserk_runb6	=[	$r_att6 ,	berserk_runb7	] {ai_run(19);};
// running with arm in air : start loop
void()	berserk_runb7	=[	$r_att7 ,	berserk_runb8	] {ai_run(21);};
void()	berserk_runb8	=[	$r_att8 ,	berserk_runb9	] {ai_run(11);};
void()	berserk_runb9	=[	$r_att9 ,	berserk_runb10	] {ai_run(21);};
void()	berserk_runb10	=[	$r_att10 ,	berserk_runb11	] {ai_run(25);};
void()	berserk_runb11	=[	$r_att11 ,	berserk_runb12	] {ai_run(18);};
void()	berserk_runb12	=[	$r_att12 ,	berserk_runb7	] {ai_run(19);};
// running with arm in air : end loop
*/


mframe_t berserk_frames_run1 [] =
{
	{ai_run, 32, NULL},
	{ai_run, 17, NULL},
	{ai_run, 32, NULL},
	{ai_run, 36, NULL},
	{ai_run, 27, NULL},
	{ai_run, 26, NULL}
};
mmove_t berserk_move_run1 = {FRAME_run1, FRAME_run6, berserk_frames_run1, NULL};

void berserk_run (edict_t *self) {
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &berserk_move_stand;
	else
		self->monsterinfo.currentmove = &berserk_move_run1;
}


void berserk_attack_spike (edict_t *self) {
	static	vec3_t	aim = {MELEE_DISTANCE, 0, -24};
	int damage;
	if (self->radius_dmg)
		damage = (40 + 12 * self->monsterinfo.skill);
	else
		damage = (20 + 6 * self->monsterinfo.skill);

	fire_hit (self, aim, damage, 400); // Faster attack -- upwards and backwards
}


void berserk_swing (edict_t *self) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_punch, 1, ATTN_NORM, 0);
	}
}

mframe_t berserk_frames_attack_spike [] =
{
/*		ai_charge, 0, NULL,*/
		{ai_charge, 0, NULL},
		{ai_charge, 0, berserk_swing},
		{ai_charge, 0, berserk_attack_spike},
		{ai_charge, 0, NULL},
		{ai_charge, 0, NULL},
		{ai_charge, 0, NULL},
		{ai_charge, 0, NULL}
};
mmove_t berserk_move_attack_spike = {FRAME_att_c2 /*1*/, FRAME_att_c8, berserk_frames_attack_spike, berserk_run};


void berserk_attack_club (edict_t *self) {
	vec3_t	aim;
	int damage;
	if (self->radius_dmg)
		damage = (60 + 14 * self->monsterinfo.skill);
	else
		damage = (30 + 7 * self->monsterinfo.skill);

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	fire_hit (self, aim, damage, 400);		// Slower attack
}

mframe_t berserk_frames_attack_club [] =
{
/*	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,*/
	{ai_charge, 0, NULL},
	{ai_charge, 0, berserk_swing},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, berserk_attack_club},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL},
	{ai_charge, 0, NULL}
};
mmove_t berserk_move_attack_club = {FRAME_att_c12 /*9*/, FRAME_att_c20, berserk_frames_attack_club, berserk_run};


void berserk_strike (edict_t *self) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int damage;

	if (!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_CHICK_ROCKET_1], forward, right, start);
	start[2] -= (self->viewheight / 2.0);

	VectorMA(self->enemy->s.origin, 0.3, self->enemy->velocity, vec);
//	VectorCopy (self->enemy->s.origin, vec);
	vec[2] -= 32;
//	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	if (self->radius_dmg)
		damage = (80 + 12 * self->monsterinfo.skill);
	else
		damage = (40 + 6 * self->monsterinfo.skill);

	monster_fire_greenmissile(self, start, dir, damage, 550 + 6 * self->monsterinfo.skill, MZ2_CHICK_ROCKET_1);
	self->monsterinfo.attack_finished = level.time + 0.8 + random();

/*	static	vec3_t	aim = {MELEE_DISTANCE, 0, 12};
	int damage;
	if (self->radius_dmg)
		damage = (80 + 28 * self->monsterinfo.skill);
	else
		damage = (40 + 14 * self->monsterinfo.skill);
	fire_hit (self, aim, damage, 400);*/
}

void berserk_restrike (edict_t *self) {
	if ((self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy)) {
		self->s.frame -= 15;
	}
}


mframe_t berserk_frames_attack_strike [] =
{
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, berserk_swing},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, berserk_strike},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 10, NULL},
	{ai_run, 19.7, NULL},
	{ai_run, 23.6, berserk_restrike}
};

mmove_t berserk_move_attack_strike = {FRAME_att_c21, FRAME_att_c34, berserk_frames_attack_strike, berserk_run};


void berserk_attack(edict_t *self) {
	self->monsterinfo.currentmove = &berserk_move_attack_strike;
}
void berserk_melee (edict_t *self) {
	if ((rand() % 2) == 0)
		self->monsterinfo.currentmove = &berserk_move_attack_spike;
	else
		self->monsterinfo.currentmove = &berserk_move_attack_club;
}


/*
void() 	berserk_atke1	=[	$r_attb1,	berserk_atke2	] {ai_run(9);};
void() 	berserk_atke2	=[	$r_attb2,	berserk_atke3	] {ai_run(6);};
void() 	berserk_atke3	=[	$r_attb3,	berserk_atke4	] {ai_run(18.4);};
void() 	berserk_atke4	=[	$r_attb4,	berserk_atke5	] {ai_run(25);};
void() 	berserk_atke5	=[	$r_attb5,	berserk_atke6	] {ai_run(14);};
void() 	berserk_atke6	=[	$r_attb6,	berserk_atke7	] {ai_run(20);};
void() 	berserk_atke7	=[	$r_attb7,	berserk_atke8	] {ai_run(8.5);};
void() 	berserk_atke8	=[	$r_attb8,	berserk_atke9	] {ai_run(3);};
void() 	berserk_atke9	=[	$r_attb9,	berserk_atke10	] {ai_run(17.5);};
void() 	berserk_atke10	=[	$r_attb10,	berserk_atke11	] {ai_run(17);};
void() 	berserk_atke11	=[	$r_attb11,	berserk_atke12	] {ai_run(9);};
void() 	berserk_atke12	=[	$r_attb12,	berserk_atke13	] {ai_run(25);};
void() 	berserk_atke13	=[	$r_attb13,	berserk_atke14	] {ai_run(3.7);};
void() 	berserk_atke14	=[	$r_attb14,	berserk_atke15	] {ai_run(2.6);};
void() 	berserk_atke15	=[	$r_attb15,	berserk_atke16	] {ai_run(19);};
void() 	berserk_atke16	=[	$r_attb16,	berserk_atke17	] {ai_run(25);};
void() 	berserk_atke17	=[	$r_attb17,	berserk_atke18	] {ai_run(19.6);};
void() 	berserk_atke18	=[	$r_attb18,	berserk_run1	] {ai_run(7.8);};
*/


mframe_t berserk_frames_pain1 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t berserk_move_pain1 = {FRAME_painc1, FRAME_painc4, berserk_frames_pain1, berserk_run};


mframe_t berserk_frames_pain2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t berserk_move_pain2 = {FRAME_painb1, FRAME_painb20, berserk_frames_pain2, berserk_run};

void berserk_pain (edict_t *self, edict_t *other, float kick, int damage) {
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.1 * self->monsterinfo.skill;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}

	if (damage < 20 + 5 * self->monsterinfo.skill) {
		self->monsterinfo.currentmove = &berserk_move_pain1;
		self->monsterinfo.jumptime = level.time + 3.0;
	} else {
		self->monsterinfo.currentmove = &berserk_move_pain2;
		self->monsterinfo.jumptime = level.time + 6.0;
	}
}


void berserk_dead (edict_t *self) {
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}


mframe_t berserk_frames_death1 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
	
};
mmove_t berserk_move_death1 = {FRAME_death1, FRAME_death13, berserk_frames_death1, berserk_dead};


mframe_t berserk_frames_death2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t berserk_move_death2 = {FRAME_deathc1, FRAME_deathc8, berserk_frames_death2, berserk_dead};


void berserk_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

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

	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (damage >= 50)
		self->monsterinfo.currentmove = &berserk_move_death1;
	else
		self->monsterinfo.currentmove = &berserk_move_death2;
}


/*QUAKED monster_berserk (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_berserk (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	// pre-caches
	sound_pain  = gi.soundindex ("berserk/berpain2.wav");
	sound_die   = gi.soundindex ("berserk/berdeth2.wav");
//	sound_idle  = gi.soundindex ("berserk/beridle1.wav");
	sound_punch = gi.soundindex ("berserk/attack.wav");
//	sound_search = gi.soundindex ("berserk/bersrch1.wav");
	sound_sight = gi.soundindex ("berserk/sight.wav");

	self->s.modelindex = gi.modelindex("models/monsters/berserk/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->monsterinfo.skill = skill->value;
	self->mass = 250;
	self->health = 270 + 40 * self->monsterinfo.skill;
	self->max_health = self->health;
	self->monsterinfo.level = 755 + 322 * self->monsterinfo.skill;
	self->gib_health = -200 - 22 * skill->value;
	strcpy(self->monsterinfo.name, "a berserker");

	self->pain = berserk_pain;
	self->die = berserk_die;

	self->monsterinfo.stand = berserk_stand;
	self->monsterinfo.walk = berserk_walk;
	self->monsterinfo.run = berserk_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = berserk_attack;
	self->monsterinfo.melee = berserk_melee;
	self->monsterinfo.sight = berserk_sight;
//	self->monsterinfo.search = berserk_search;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 0;

	self->monsterinfo.cts_any = 0.12;
	self->monsterinfo.cts_class1 = 0.05;
	self->monsterinfo.cts_class2 = 0.15;
	self->monsterinfo.cts_class3 = 0.45;
	self->monsterinfo.item_any = 0.08;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->monsterinfo.currentmove = &berserk_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	gi.linkentity (self);

	walkmonster_start (self);
}
