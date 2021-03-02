/*
==============================================================================

GLADIATOR

==============================================================================
*/

#include "g_local.h"
#include "m_gladiator.h"

void ai_run_slide(edict_t *self, float distance);

static int	sound_pain1;
//static int	sound_pain2;
static int	sound_die;
static int	sound_gun;
static int	sound_cleaver_swing;
static int	sound_cleaver_hit;
static int	sound_cleaver_miss;
//static int	sound_idle;
static int	sound_search;
static int	sound_sight;

/*
void gladiator_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
*/
void gladiator_search (edict_t *self) {
	//gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}

void gladiator_step (edict_t *self) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_search, 0.1, ATTN_NORM, 0);
	}
}
void gladiator_runstep (edict_t *self) {
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_search, 0.3, ATTN_NORM, 0);
	}
}

void gladiator_cleaver_swing (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_cleaver_swing, 1, ATTN_NORM, 0);
	}
}

mframe_t gladiator_frames_stand [] =
{
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL}
};
mmove_t gladiator_move_stand = {FRAME_stand1, FRAME_stand7, gladiator_frames_stand, NULL};

void gladiator_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_stand;
}


mframe_t gladiator_frames_walk [] =
{
	{ai_walk, 15, NULL},
	{ai_walk, 7,  NULL},
	{ai_walk, 6,  NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 2,  gladiator_step},
	{ai_walk, 0,  NULL},
	{ai_walk, 2,  NULL},
	{ai_walk, 8,  NULL},
	{ai_walk, 12, NULL},
	{ai_walk, 8,  NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 5,  NULL},
	{ai_walk, 2,  gladiator_step},
	{ai_walk, 2,  NULL},
	{ai_walk, 1,  NULL},
	{ai_walk, 8,  NULL}
};
mmove_t gladiator_move_walk = {FRAME_walk1, FRAME_walk16, gladiator_frames_walk, NULL};

void gladiator_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_walk;
}


mframe_t gladiator_frames_run [] =
{
	{ai_run, 23,	NULL},
	{ai_run, 14,	gladiator_runstep},
	{ai_run, 14,	NULL},
	{ai_run, 21,	NULL},
	{ai_run, 12,	gladiator_runstep},
	{ai_run, 13,	NULL}
};
mmove_t gladiator_move_run = {FRAME_run1, FRAME_run6, gladiator_frames_run, NULL};

void gladiator_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gladiator_move_stand;
	else
		self->monsterinfo.currentmove = &gladiator_move_run;
}


void GaldiatorMelee (edict_t *self)
{
	vec3_t	aim;
	int damage;

	if (self->radius_dmg)
		damage = (80 + 20 * self->monsterinfo.skill);
	else
		damage = (40 + 10 * self->monsterinfo.skill);
	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	if (fire_hit (self, aim, damage, 300)) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_AUTO, sound_cleaver_hit, 1, ATTN_NORM, 0);
		}
	} else {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self, CHAN_AUTO, sound_cleaver_miss, 1, ATTN_NORM, 0);
		}
	}
}

mframe_t gladiator_frames_attack_melee [] =
{
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, gladiator_cleaver_swing},
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, GaldiatorMelee},
	{ai_run_slide, 15, NULL},
	{ai_run_slide, 25, NULL},
	{ai_run_slide, 25, NULL},
	{ai_run_slide, 25, gladiator_cleaver_swing},
	{ai_run_slide, 25, NULL},
	{ai_run_slide, 25, NULL},
	{ai_run_slide, 25, GaldiatorMelee},
	{ai_run_slide, 15, NULL},
	{ai_run_slide, 5, NULL},
	{ai_run_slide, 5, NULL}
};
mmove_t gladiator_move_attack_melee = {FRAME_melee1, FRAME_melee17, gladiator_frames_attack_melee, gladiator_run};

void gladiator_melee(edict_t *self) {
	self->monsterinfo.currentmove = &gladiator_move_attack_melee;
}


void GladiatorGun (edict_t *self);
void GladiatorReGun (edict_t *self) {
	if ((self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy) && (random() < 0.4 + 0.008 * self->monsterinfo.skill)) {
		VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, self->pos1);
		self->pos1[2] += self->enemy->viewheight;
		self->s.frame -= 10;
	}
}

mframe_t gladiator_frames_attack_gun [] =
{
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, GladiatorGun},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, NULL},
	{ai_run_circle,	14, GladiatorReGun}
};
mmove_t gladiator_move_attack_gun = {FRAME_attack1, FRAME_attack9, gladiator_frames_attack_gun, gladiator_run};

void GladiatorGun (edict_t *self) {
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;

	if (!self->enemy)
		return;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GLADIATOR_RAILGUN_1], forward, right, start);

	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	if (self->radius_dmg)
		monster_fire_railgun (self, start, dir, 130 + 30 * self->monsterinfo.skill, 100, MZ2_GLADIATOR_RAILGUN_1);
	else
		monster_fire_railgun (self, start, dir, 75 + 15 * self->monsterinfo.skill, 100, MZ2_GLADIATOR_RAILGUN_1);

	if ((self->enemy->health > 0) && visible(self, self->enemy) && (random() < (0.1 + 0.015 * self->monsterinfo.skill))) {
		VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, self->pos1);
		self->pos1[2] += self->enemy->viewheight;
		self->s.frame -= 5;
	}
}

void gladiator_attack(edict_t *self) {
//	float	range;
//	vec3_t	v;

	if (!self->enemy)
		return;

// a small safe zone
//	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
//	range = VectorLength(v);
//	if (range <= (MELEE_DISTANCE + 32))
//		return;

	// charge up the railgun
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_gun, 1, ATTN_NORM, 0);
	}
//	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	VectorMA(self->enemy->s.origin, 0.4, self->enemy->velocity, self->pos1);
	self->pos1[2] += self->enemy->viewheight;
	self->monsterinfo.currentmove = &gladiator_move_attack_gun;
}

void gladiator_sight (edict_t *self, edict_t *other) {
	if (random() < (0.4 + 0.045 * self->monsterinfo.skill))
		gladiator_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

mframe_t gladiator_frames_pain [] = {
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t gladiator_move_pain = {FRAME_pain1, FRAME_pain6, gladiator_frames_pain, gladiator_run};

mframe_t gladiator_frames_pain_air [] = {
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t gladiator_move_pain_air = {FRAME_painup1, FRAME_painup7, gladiator_frames_pain_air, gladiator_run};

void gladiator_pain (edict_t *self, edict_t *other, float kick, int damage){
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time) {
		if ((self->velocity[2] > 100) && (self->monsterinfo.currentmove == &gladiator_move_pain))
			self->monsterinfo.currentmove = &gladiator_move_pain_air;
		return;
	}

	self->pain_debounce_time = level.time + 4 + 0.15 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	self->monsterinfo.jumptime = level.time + 4.0;

	if (self->velocity[2] > 100)
		self->monsterinfo.currentmove = &gladiator_move_pain_air;
	else
		self->monsterinfo.currentmove = &gladiator_move_pain;
	
}


void gladiator_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t gladiator_frames_death [] =
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
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t gladiator_move_death = {FRAME_death1, FRAME_death22, gladiator_frames_death, gladiator_dead};

void gladiator_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &gladiator_move_death;
}


/*QUAKED monster_gladiator (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
*/
void SP_monster_gladiator (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/


	sound_pain1 = gi.soundindex ("gladiator/pain.wav");	
//	sound_pain2 = gi.soundindex ("gladiator/gldpain2.wav");	
	sound_die = gi.soundindex ("gladiator/glddeth2.wav");	
	sound_gun = gi.soundindex ("gladiator/railgun.wav");
	sound_cleaver_swing = gi.soundindex ("gladiator/melee1.wav");
	sound_cleaver_hit = gi.soundindex ("gladiator/melee2.wav");
	sound_cleaver_miss = gi.soundindex ("gladiator/melee3.wav");
//	sound_idle = gi.soundindex ("gladiator/gldidle1.wav");
	sound_search = gi.soundindex ("gladiator/gldsrch1.wav");
	sound_sight = gi.soundindex ("gladiator/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/gladiatr/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 64);

	self->mass = 400;
	self->health = 560 + 90 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 1380 + 930 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -230 - 30 * skill->value;
	strcpy(self->monsterinfo.name, "a gladiator");

	self->pain = gladiator_pain;
	self->die = gladiator_die;

	self->monsterinfo.stand = gladiator_stand;
	self->monsterinfo.walk = gladiator_walk;
	self->monsterinfo.run = gladiator_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = gladiator_attack;
	self->monsterinfo.melee = gladiator_melee;
	self->monsterinfo.sight = gladiator_sight;
	self->monsterinfo.idle = NULL;
	self->monsterinfo.search = gladiator_search;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 250;

	self->monsterinfo.cts_any = 0.55;
	self->monsterinfo.cts_class1 = 0.10;
	self->monsterinfo.cts_class2 = 0.20;
	self->monsterinfo.cts_class3 = 0.40;

	self->monsterinfo.item_any = 0.32;
	self->monsterinfo.itemmult_class1 = 1.2;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 0.9;
	self->monsterinfo.itemmult_class4 = 0.8;

	gi.linkentity (self);
	self->monsterinfo.currentmove = &gladiator_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
