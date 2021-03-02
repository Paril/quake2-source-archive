/*
==============================================================================

FLIPPER

==============================================================================
*/

#include "g_local.h"
#include "m_flipper.h"


static int	sound_chomp;
//static int	sound_attack;
static int	sound_pain1;
//static int	sound_pain2;
static int	sound_death;
//static int	sound_idle;
//static int	sound_search;
static int	sound_sight;


void flipper_stand (edict_t *self);

mframe_t flipper_frames_stand [] =
{
	{ai_stand, 0, NULL}
};
	
mmove_t	flipper_move_stand = {FRAME_flphor01, FRAME_flphor01, flipper_frames_stand, NULL};

void flipper_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &flipper_move_stand;
}

#define FLIPPER_RUN_SPEED	24

mframe_t flipper_frames_run [] =
{
	{ai_run, FLIPPER_RUN_SPEED, NULL},	// 6
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},	// 10

	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},	// 20

	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL},
	{ai_run, FLIPPER_RUN_SPEED, NULL}		// 29
};
mmove_t flipper_move_run_loop = {FRAME_flpver06, FRAME_flpver29, flipper_frames_run, NULL};

void flipper_run_loop (edict_t *self)
{
	self->monsterinfo.currentmove = &flipper_move_run_loop;
}

mframe_t flipper_frames_run_start [] =
{
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL}
};
mmove_t flipper_move_run_start = {FRAME_flpver01, FRAME_flpver06, flipper_frames_run_start, flipper_run_loop};

void flipper_run (edict_t *self)
{
	self->monsterinfo.currentmove = &flipper_move_run_start;
}

/* Standard Swimming */ 
mframe_t flipper_frames_walk [] =
{
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL},
	{ai_walk, 4, NULL}
};
mmove_t flipper_move_walk = {FRAME_flphor01, FRAME_flphor24, flipper_frames_walk, NULL};

void flipper_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &flipper_move_walk;
}

mframe_t flipper_frames_start_run [] =
{
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, NULL},
	{ai_run, 8, flipper_run}
};
mmove_t flipper_move_start_run = {FRAME_flphor01, FRAME_flphor05, flipper_frames_start_run, NULL};

void flipper_start_run (edict_t *self)
{
	self->monsterinfo.currentmove = &flipper_move_start_run;
}

mframe_t flipper_frames_pain2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0,	NULL},
	{ai_move, 0,	NULL},
	{ai_move, 0, NULL}
};
mmove_t flipper_move_pain2 = {FRAME_flppn101, FRAME_flppn105, flipper_frames_pain2, flipper_run};

mframe_t flipper_frames_pain1 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0,	NULL},
	{ai_move, 0,	NULL},
	{ai_move, 0, NULL}
};
mmove_t flipper_move_pain1 = {FRAME_flppn201, FRAME_flppn205, flipper_frames_pain1, flipper_run};

void flipper_bite (edict_t *self)
{
	vec3_t	aim;
	int damage;

	if (self->radius_dmg)
		damage = (24 + 12 * self->monsterinfo.skill);
	else
		damage = (12 + 5 * self->monsterinfo.skill);

	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, damage, 0);
}

void flipper_preattack (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_chomp, 1, ATTN_NORM, 0);
	}
}

mframe_t flipper_frames_attack [] =
{
/*	{ai_charge, 0,	flipper_preattack,
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},*/
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	flipper_bite},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	flipper_bite},
	{ai_charge, 0,	NULL}
};
mmove_t flipper_move_attack = {FRAME_flpbit13/*01*/, FRAME_flpbit20, flipper_frames_attack, flipper_run};

void flipper_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &flipper_move_attack;
}

void flipper_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.1 * self->monsterinfo.skill;
	
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	self->monsterinfo.currentmove = &flipper_move_pain1;
	self->monsterinfo.jumptime = level.time + 3.0;
}

void flipper_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t flipper_frames_death [] =
{
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},

	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},

	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},

	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},

	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},

	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL},
	{ai_move, 0,	 NULL}
};
mmove_t flipper_move_death = {FRAME_flpdth01, FRAME_flpdth56, flipper_frames_death, flipper_dead};

void flipper_sight (edict_t *self, edict_t *other)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

void flipper_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &flipper_move_death;
}

/*QUAKED monster_flipper (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_flipper (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1		= gi.soundindex ("flipper/flppain1.wav");	
//	sound_pain2		= gi.soundindex ("flipper/flppain2.wav");	
	sound_death		= gi.soundindex ("flipper/flpdeth1.wav");	
	sound_chomp		= gi.soundindex ("flipper/flpatck1.wav");
//	sound_attack	= gi.soundindex ("flipper/flpatck2.wav");
//	sound_idle		= gi.soundindex ("flipper/flpidle1.wav");
//	sound_search	= gi.soundindex ("flipper/flpsrch1.wav");
	sound_sight		= gi.soundindex ("flipper/flpsght1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/flipper/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 32);

	self->mass = 100;
	self->health = 80 + 32 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 150 + 46 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -80 - 10 * skill->value;
	strcpy(self->monsterinfo.name, "a flipper");

	self->monsterinfo.cts_any = 0.25;
	self->monsterinfo.cts_class1 = 0.01;
	self->monsterinfo.cts_class2 = 0.04;
	self->monsterinfo.cts_class3 = 0.60;

	self->monsterinfo.item_any = 0.01;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->pain = flipper_pain;
	self->die = flipper_die;

	self->monsterinfo.stand = flipper_stand;
	self->monsterinfo.walk = flipper_walk;
	self->monsterinfo.run = flipper_start_run;
	self->monsterinfo.melee = flipper_melee;
	self->monsterinfo.sight = flipper_sight;
        self->monsterinfo.aggressive = true;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &flipper_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	swimmonster_start (self);
}
