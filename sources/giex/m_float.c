/*
==============================================================================

floater

==============================================================================
*/

#include "g_local.h"
#include "m_float.h"


static int	sound_attack2;
//static int	sound_attack3;
static int	sound_death1;
static int	sound_idle;
static int	sound_pain1;
//static int	sound_pain2;
static int	sound_sight;


void floater_idle (edict_t *self)
{
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
	}
}


//void floater_stand1 (edict_t *self);
void floater_dead (edict_t *self);
void floater_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void floater_run (edict_t *self);
void floater_wham (edict_t *self);
void floater_zap (edict_t *self);


void floater_fire_blaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;
	int damage;

	if (!self->enemy)
		return;
	if (self->radius_dmg)
		damage = (4 + 2.0 * self->monsterinfo.skill);
	else
		damage = (2 + 1.0 * self->monsterinfo.skill);

	if ((self->s.frame == FRAME_attak104) || (self->s.frame == FRAME_attak107))
		effect = EF_HYPERBLASTER;
	else
		effect = 0;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_FLOAT_BLASTER_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, damage, 650 + 50 * self->monsterinfo.skill, MZ2_FLOAT_BLASTER_1, effect);
}


mframe_t floater_frames_stand1 [] =
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
	{ai_stand, 0, NULL}
};
mmove_t	floater_move_stand1 = {FRAME_stand101, FRAME_stand152, floater_frames_stand1, NULL};

mframe_t floater_frames_stand2 [] =
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
	{ai_stand, 0, NULL}
};
mmove_t	floater_move_stand2 = {FRAME_stand201, FRAME_stand252, floater_frames_stand2, NULL};

void floater_stand (edict_t *self)
{
	if (random() <= 0.5)		
		self->monsterinfo.currentmove = &floater_move_stand1;
	else
		self->monsterinfo.currentmove = &floater_move_stand2;
}

mframe_t floater_frames_activate [] =
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
	{ai_move,	0,	NULL},	
	{ai_move,	0,	NULL},	
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t floater_move_activate = {FRAME_actvat01, FRAME_actvat31, floater_frames_activate, NULL};

mframe_t floater_frames_attack1 [] =
{
	{ai_charge,	0,	NULL},			// Blaster attack
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	floater_fire_blaster},			// BOOM (0, -25.8, 32.5)	-- LOOP Starts
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	floater_fire_blaster},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL}			//							-- LOOP Ends
};
mmove_t floater_move_attack1 = {FRAME_attak101, FRAME_attak114, floater_frames_attack1, floater_run};

mframe_t floater_frames_attack2 [] =
{
	{ai_charge,	0,	NULL},			// Claws
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	floater_wham},			// WHAM (0, -45, 29.6)		-- LOOP Starts
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},			//							-- LOOP Ends
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL}
};
mmove_t floater_move_attack2 = {FRAME_attak201, FRAME_attak225, floater_frames_attack2, floater_run};

mframe_t floater_frames_attack3 [] =
{
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	floater_zap},		//								-- LOOP Starts
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},		//								-- LOOP Ends
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL},
	{ai_charge,	0,	NULL}
};
mmove_t floater_move_attack3 = {FRAME_attak301, FRAME_attak334, floater_frames_attack3, floater_run};

mframe_t floater_frames_death [] =
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
mmove_t floater_move_death = {FRAME_death01, FRAME_death13, floater_frames_death, floater_dead};

mframe_t floater_frames_pain1 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t floater_move_pain1 = {FRAME_pain101, FRAME_pain107, floater_frames_pain1, floater_run};

mframe_t floater_frames_pain2 [] =
{
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL},
	{ai_move,	0,	NULL}
};
mmove_t floater_move_pain2 = {FRAME_pain201, FRAME_pain208, floater_frames_pain2, floater_run};

mframe_t floater_frames_pain3 [] =
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
	{ai_move,	0,	NULL}
};
mmove_t floater_move_pain3 = {FRAME_pain301, FRAME_pain312, floater_frames_pain3, floater_run};

mframe_t floater_frames_walk [] =
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
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL},
	{ai_walk, 5, NULL}
};
mmove_t	floater_move_walk = {FRAME_stand101, FRAME_stand152, floater_frames_walk, NULL};

mframe_t floater_frames_run [] =
{
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL},
	{ai_run, 13, NULL}
};
mmove_t	floater_move_run = {FRAME_stand101, FRAME_stand152, floater_frames_run, NULL};

void floater_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &floater_move_stand1;
	else
		self->monsterinfo.currentmove = &floater_move_run;
}

void floater_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &floater_move_walk;
}

void floater_wham (edict_t *self)
{
	static	vec3_t	aim = {MELEE_DISTANCE, 0, 0};
	int damage;

	if (self->radius_dmg)
		damage = 80 + 8 * self->monsterinfo.skill;
	else
		damage = 40 + 4 * self->monsterinfo.skill;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
	}
	fire_hit (self, aim, damage, -50);
}

void floater_zap (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	origin;
	vec3_t	dir;
	vec3_t	offset;
	int damage;

	if (!self->enemy)
		return;

	if (self->radius_dmg)
		damage = 100 + 12 * self->monsterinfo.skill;
	else
		damage = 50 + 6 * self->monsterinfo.skill;

	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);

	AngleVectors (self->s.angles, forward, right, NULL);
	//FIXME use a flash and replace these two lines with the commented one
	VectorSet (offset, 18.5, -0.9, 10);
	G_ProjectSource (self->s.origin, offset, forward, right, origin);
//	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, origin);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
	}

	//FIXME use the flash, Luke
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (32);
	gi.WritePosition (origin);
	gi.WriteDir (dir);
	gi.WriteByte (1);	//sparks
	gi.multicast (origin, MULTICAST_PVS);

	T_Damage (self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, damage, -10, DAMAGE_ENERGY, MOD_UNKNOWN);
}

void floater_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &floater_move_attack1;
}

void floater_sight (edict_t *self, edict_t *other)
{
	if (random() < (0.25 + 0.06 * self->monsterinfo.skill))
		floater_attack(self);
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}


void floater_melee(edict_t *self)
{
	if (random() < 0.5)		
		self->monsterinfo.currentmove = &floater_move_attack3;
	else
		self->monsterinfo.currentmove = &floater_move_attack2;
}


void floater_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.1 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	self->monsterinfo.currentmove = &floater_move_pain1;
	self->monsterinfo.jumptime = level.time + 4.0;
}

void floater_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

void floater_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	BecomeExplosion1(self);
}

/*QUAKED monster_floater (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_floater (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_attack2 = gi.soundindex ("floater/fltatck2.wav");
//	sound_attack3 = gi.soundindex ("floater/fltatck3.wav");
	sound_death1 = gi.soundindex ("floater/fltdeth1.wav");
	sound_idle = gi.soundindex ("floater/fltidle1.wav");
	sound_pain1 = gi.soundindex ("floater/fltpain1.wav");
//	sound_pain2 = gi.soundindex ("floater/fltpain2.wav");
	sound_sight = gi.soundindex ("floater/fltsght1.wav");

	gi.soundindex ("floater/fltatck1.wav");

	self->s.sound = gi.soundindex ("floater/fltsrch1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/float/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 32);

	self->mass = 300;
	self->health = 300 + 28 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 278 + 168 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -100;
	strcpy(self->monsterinfo.name, "a floater");

	self->monsterinfo.cts_any = 0.25;
	self->monsterinfo.cts_class1 = 0.02;
	self->monsterinfo.cts_class2 = 0.07;
	self->monsterinfo.cts_class3 = 0.20;

	self->monsterinfo.item_any = 0.06;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	self->pain = floater_pain;
	self->die = floater_die;

	self->monsterinfo.stand = floater_stand;
	self->monsterinfo.walk = floater_walk;
	self->monsterinfo.run = floater_run;
//	self->monsterinfo.dodge = floater_dodge;
	self->monsterinfo.attack = floater_attack;
	self->monsterinfo.melee = floater_melee;
	self->monsterinfo.sight = floater_sight;
	self->monsterinfo.idle = floater_idle;
        self->monsterinfo.aggressive = true;

	gi.linkentity (self);

	if (random() <= 0.5)
		self->monsterinfo.currentmove = &floater_move_stand1;
	else
		self->monsterinfo.currentmove = &floater_move_stand2;

	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
