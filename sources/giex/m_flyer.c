/*
==============================================================================

flyer

==============================================================================
*/

#include "g_local.h"
#include "m_flyer.h"

qboolean visible (edict_t *self, edict_t *other);
void ai_run_slide(edict_t *self, float distance);

static int	nextmove;			// Used for start/stop frames

static int	sound_sight;
//static int	sound_idle;
static int	sound_pain1;
//static int	sound_pain2;
//static int	sound_slash;
static int	sound_sproing;
static int	sound_die;


void flyer_check_melee(edict_t *self);
void flyer_loop_melee (edict_t *self);
void flyer_melee (edict_t *self);
void flyer_setstart (edict_t *self);
void flyer_stand (edict_t *self);
void flyer_nextmove (edict_t *self);

/*
void flyer_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
*/
void flyer_pop_blades (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sproing, 1, ATTN_NORM, 0);
	}
}


mframe_t flyer_frames_stand [] =
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
	{ai_stand, 0, NULL}
};
mmove_t	flyer_move_stand = {FRAME_stand01, FRAME_stand45, flyer_frames_stand, NULL};


mframe_t flyer_frames_walk [] =
{
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL}
};
mmove_t	flyer_move_walk = {FRAME_stand01, FRAME_stand45, flyer_frames_walk, NULL};

mframe_t flyer_frames_run [] =
{
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL},
	{ai_run, 19, NULL}
};
mmove_t	flyer_move_run = {FRAME_stand01, FRAME_stand45, flyer_frames_run, NULL};

void flyer_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &flyer_move_stand;
	else
		self->monsterinfo.currentmove = &flyer_move_run;
}

void flyer_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &flyer_move_walk;
}

void flyer_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &flyer_move_stand;
}

mframe_t flyer_frames_start [] =
{
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	flyer_nextmove}
};
mmove_t flyer_move_start = {FRAME_start01, FRAME_start06, flyer_frames_start, NULL};

mframe_t flyer_frames_stop [] =
{
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	NULL},
		{ai_move, 0,	flyer_nextmove}
};
mmove_t flyer_move_stop = {FRAME_stop01, FRAME_stop07, flyer_frames_stop, NULL};

void flyer_stop (edict_t *self)
{
		self->monsterinfo.currentmove = &flyer_move_stop;
}

void flyer_start (edict_t *self)
{
		self->monsterinfo.currentmove = &flyer_move_start;
}


mframe_t flyer_frames_rollright [] =
{
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
mmove_t flyer_move_rollright = {FRAME_rollr01, FRAME_rollr09, flyer_frames_rollright, NULL};

mframe_t flyer_frames_rollleft [] =
{
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
mmove_t flyer_move_rollleft = {FRAME_rollf01, FRAME_rollf09, flyer_frames_rollleft, NULL};

mframe_t flyer_frames_pain3 [] =
{	
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL}
};
mmove_t flyer_move_pain3 = {FRAME_pain301, FRAME_pain304, flyer_frames_pain3, flyer_run};

mframe_t flyer_frames_pain2 [] =
{
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL}
};
mmove_t flyer_move_pain2 = {FRAME_pain201, FRAME_pain204, flyer_frames_pain2, flyer_run};

mframe_t flyer_frames_pain1 [] =
{
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
mmove_t flyer_move_pain1 = {FRAME_pain101, FRAME_pain109, flyer_frames_pain1, flyer_run};

mframe_t flyer_frames_defense [] = 
{
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},		// Hold this frame
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL}
};
mmove_t flyer_move_defense = {FRAME_defens01, FRAME_defens06, flyer_frames_defense, NULL};

mframe_t flyer_frames_bankright [] =
{
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL}
};
mmove_t flyer_move_bankright = {FRAME_bankr01, FRAME_bankr07, flyer_frames_bankright, NULL};

mframe_t flyer_frames_bankleft [] =
{
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL},
		{ai_move, 0, NULL}
};
mmove_t flyer_move_bankleft = {FRAME_bankl01, FRAME_bankl07, flyer_frames_bankleft, NULL};

void flyer_detonate(edict_t *self);
mframe_t flyer_frames_start_melee [] =
{
	{ai_charge, 26, flyer_pop_blades},
		{ai_charge, 26, NULL},
		{ai_charge, 26, NULL},
		{ai_charge, 26, NULL},
		{ai_charge, 26, NULL},
		{ai_charge, 26, NULL}
};
mmove_t flyer_move_start_melee = {FRAME_attak101, FRAME_attak106, flyer_frames_start_melee, flyer_detonate};

void flyer_fire (edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;

	if (!self->enemy)
		return;

	if ((self->s.frame == FRAME_attak204) || (self->s.frame == FRAME_attak207) || (self->s.frame == FRAME_attak210))
		effect = EF_HYPERBLASTER;
	else
		effect = 0;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	if (self->radius_dmg)
		monster_fire_blaster (self, start, dir, 4 + (int) ceil(2.0 * self->monsterinfo.skill), 1000, MZ2_HOVER_BLASTER_1, effect);
	else
		monster_fire_blaster (self, start, dir, 2 + (int) ceil(1.0 * self->monsterinfo.skill), 1000, MZ2_HOVER_BLASTER_1, effect);
}

void flyer_fireleft (edict_t *self)
{
	flyer_fire (self, MZ2_FLYER_BLASTER_1);
}

void flyer_fireright (edict_t *self)
{
	flyer_fire (self, MZ2_FLYER_BLASTER_2);
}

void flyer_detonate(edict_t *self);
void flyer_checkreblast(edict_t *self) {
	if ((self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy)) {
		if (range (self, self->enemy) == RANGE_MELEE) {
			self->monsterinfo.currentmove = &flyer_move_start_melee;
		} else {
			self->s.frame -= 8;
		}
	}
}

mframe_t flyer_frames_attack2 [] =
{
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, flyer_fireleft},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, flyer_fireright},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, flyer_fireleft},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, flyer_fireright},
		{ai_run_circle, 21, flyer_checkreblast},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL},
		{ai_run_circle, 21, NULL}
};
mmove_t flyer_move_attack2 = {FRAME_attak201, FRAME_attak217, flyer_frames_attack2, flyer_run};


mframe_t flyer_frames_end_melee [] =
{
		{ai_charge, 0, NULL},
		{ai_charge, 0, NULL},
		{ai_charge, 0, NULL}
};
mmove_t flyer_move_end_melee = {FRAME_attak119, FRAME_attak121, flyer_frames_end_melee, flyer_run};


void flyer_detonate(edict_t *self) {
	int damage;

	if (self->radius_dmg)
		damage = (150 + 26 * self->monsterinfo.skill);
	else
		damage = (75 + 13 * self->monsterinfo.skill);
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= 2;
	}

	T_RadiusDamage(NULL, self, self, damage, damage * 0.5, self, 120 + 5 * self->monsterinfo.skill, true, MOD_R_SPLASH);

	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	BecomeExplosion1(self);
	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY)) {
		if (self->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
			removeStealth();
		}
		level.killed_monsters++;
	}
}

void flyer_loop_melee (edict_t *self)
{
	flyer_detonate(self);
}



void flyer_attack (edict_t *self)
{
/*	if (random() <= 0.5)	
		self->monsterinfo.currentmove = &flyer_move_attack1;
	else */
	self->monsterinfo.currentmove = &flyer_move_attack2;
}

void flyer_sight (edict_t *self, edict_t *other)
{
	if (random() < (0.2 + 0.06 * self->monsterinfo.skill))
		flyer_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

void flyer_setstart (edict_t *self)
{
	nextmove = ACTION_run;
	self->monsterinfo.currentmove = &flyer_move_start;
}

void flyer_nextmove (edict_t *self)
{
	if (nextmove == ACTION_attack1)
		self->monsterinfo.currentmove = &flyer_move_start_melee;
	else if (nextmove == ACTION_attack2)
		self->monsterinfo.currentmove = &flyer_move_attack2;
	else if (nextmove == ACTION_run)
		self->monsterinfo.currentmove = &flyer_move_run;
}

void flyer_melee (edict_t *self)
{
//	flyer.nextmove = ACTION_attack1;
//	self->monsterinfo.currentmove = &flyer_move_stop;
	self->monsterinfo.currentmove = &flyer_move_start_melee;
}


void flyer_check_melee(edict_t *self)
{
	if (self->enemy && (range (self, self->enemy) == RANGE_MELEE)) {
		flyer_detonate(self);
//		T_Damage (self, self, self, self->velocity, self->s.origin, self->velocity, 10000, 0, DAMAGE_NO_PROTECTION, MOD_ROCKET);
/*		if (random() <= 0.8)
			self->monsterinfo.currentmove = &flyer_move_loop_melee;
		else
			self->monsterinfo.currentmove = &flyer_move_end_melee;
*/	} else {
		self->monsterinfo.currentmove = &flyer_move_end_melee;
	}
}

void flyer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.1 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	n = rand() % 3;
	if (n == 0) {
		self->monsterinfo.currentmove = &flyer_move_pain1;
	} else if (n == 1) {
		self->monsterinfo.currentmove = &flyer_move_pain2;
	} else {
		self->monsterinfo.currentmove = &flyer_move_pain3;
	}
	self->monsterinfo.jumptime = level.time + 3.0;
}


void flyer_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	flyer_detonate(self);
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	BecomeExplosion1(self);
}
	

/*QUAKED monster_flyer (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_flyer (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	// fix a map bug in jail5.bsp
	if (!Q_strcasecmp(level.mapname, "jail5") && (self->s.origin[2] == -104))
	{
		self->targetname = self->target;
		self->target = NULL;
	}

	sound_sight = gi.soundindex ("flyer/flysght1.wav");
//	sound_idle = gi.soundindex ("flyer/flysrch1.wav");
	sound_pain1 = gi.soundindex ("flyer/flypain1.wav");
//	sound_pain2 = gi.soundindex ("flyer/flypain2.wav");
//	sound_slash = gi.soundindex ("flyer/flyatck2.wav");
	sound_sproing = gi.soundindex ("flyer/flyatck1.wav");
	sound_die = gi.soundindex ("flyer/flydeth1.wav");

	gi.soundindex ("flyer/flyatck3.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/flyer/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->s.sound = gi.soundindex ("flyer/flyidle1.wav");

	self->mass = 50;
	self->health = 65 + 12 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 280 + 160 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -100;
	strcpy(self->monsterinfo.name, "a flyer");
	self->yaw_speed = 20 + skill->value;

	self->pain = flyer_pain;
	self->die = flyer_die;

	self->monsterinfo.cts_any = 0.52;
	self->monsterinfo.cts_class1 = 0.03;
	self->monsterinfo.cts_class2 = 0.08;
	self->monsterinfo.cts_class3 = 0.25;

	self->monsterinfo.item_any = 0.02;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->monsterinfo.stand = flyer_stand;
	self->monsterinfo.walk = flyer_walk;
	self->monsterinfo.run = flyer_run;
	self->monsterinfo.attack = flyer_attack;
	self->monsterinfo.melee = flyer_melee;
	self->monsterinfo.sight = flyer_sight;
	self->monsterinfo.idle = NULL;
	self->monsterinfo.aggressive = true;
	self->monsterinfo.prefered_range = 0;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &flyer_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
