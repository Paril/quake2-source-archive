/*
==============================================================================

Makron -- Final Boss

==============================================================================
*/

#include "g_local.h"
#include "m_boss32.h"

qboolean visible (edict_t *self, edict_t *other);

void MakronRailgun (edict_t *self);
void MakronHyperblaster (edict_t *self);
void makron_step_left (edict_t *self);
void makron_step_right (edict_t *self);
void makronBFG (edict_t *self);
void makron_dead (edict_t *self);

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

static int	sound_pain4;
static int	sound_pain5;
static int	sound_pain6;
static int	sound_death;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_attack_bfg;
static int	sound_brainsplorch;
static int	sound_prerailgun;
static int	sound_popup;
static int	sound_taunt1;
static int	sound_taunt2;
static int	sound_taunt3;
static int	sound_hit;

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

//
// stand
//

mframe_t makron_frames_stand []=
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,		// 10
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,		// 20
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,		// 30
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,		// 40
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,		// 50
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL		// 60
};
mmove_t	makron_move_stand = {FRAME_stand201, FRAME_stand260, makron_frames_stand, NULL};
	
void makron_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &makron_move_stand;
}

mframe_t makron_frames_run [] =
{
	drone_ai_run, 3,	makron_step_left,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 8,	NULL,
	drone_ai_run, 8,	NULL,
	drone_ai_run, 8,	makron_step_right,
	drone_ai_run, 6,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 9,	NULL,
	drone_ai_run, 6,	NULL,
	drone_ai_run, 12,	NULL
};
mmove_t	makron_move_run = {FRAME_walk204, FRAME_walk213, makron_frames_run, NULL};

void makron_hit (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_hit, 1, ATTN_NONE,0);
}

void makron_popup (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_popup, 1, ATTN_NONE,0);
}

void makron_step_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
}

void makron_step_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
}

void makron_brainsplorch (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_brainsplorch, 1, ATTN_NORM,0);
}

void makron_prerailgun (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_prerailgun, 1, ATTN_NORM,0);
}


mframe_t makron_frames_walk [] =
{
	drone_ai_walk, 3,	makron_step_left,
	drone_ai_walk, 12,	NULL,
	drone_ai_walk, 8,	NULL,
	drone_ai_walk, 8,	NULL,
	drone_ai_walk, 8,	makron_step_right,
	drone_ai_walk, 6,	NULL,
	drone_ai_walk, 12,	NULL,
	drone_ai_walk, 9,	NULL,
	drone_ai_walk, 6,	NULL,
	drone_ai_walk, 12,	NULL
};
mmove_t	makron_move_walk = {FRAME_walk204, FRAME_walk213, makron_frames_walk, NULL};

void makron_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;

	self->monsterinfo.currentmove = &makron_move_walk;
}

void makron_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &makron_move_stand;
	else
		self->monsterinfo.currentmove = &makron_move_run;
}

mframe_t makron_frames_death2 [] =
{
	ai_move,	-15,	NULL,
	ai_move,	3,	NULL,
	ai_move,	-12,	NULL,
	ai_move,	0,	makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 10
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	11,	NULL,
	ai_move,	12,	NULL,
	ai_move,	11,	makron_step_right,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 20
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 30
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	5,	NULL,
	ai_move,	7,	NULL,
	ai_move,	6,	makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-1,	NULL,
	ai_move,	2,	NULL,			// 40
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,			// 50
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-6,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	-6,	makron_step_right,
	ai_move,	-4,	NULL,
	ai_move,	-4,	makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 60
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	-5,	NULL,
	ai_move,	-3,	makron_step_right,
	ai_move,	-8,	NULL,
	ai_move,	-3,	makron_step_left,
	ai_move,	-7,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	-4,	makron_step_right,			// 70
	ai_move,	-6,	NULL,			
	ai_move,	-7,	NULL,
	ai_move,	0,	makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 80
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	2,	NULL,
	ai_move,	0,	NULL,			// 90
	ai_move,	27,	makron_hit,			
	ai_move,	26,	NULL,
	ai_move,	0,	makron_brainsplorch,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL			// 95
};
mmove_t makron_move_death2 = {FRAME_death201, FRAME_death295, makron_frames_death2, makron_dead};

mframe_t makron_frames_death3 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t makron_move_death3 = {FRAME_death301, FRAME_death320, makron_frames_death3, NULL};

mframe_t makron_frames_sight [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t makron_move_sight= {FRAME_active01, FRAME_active13, makron_frames_sight, makron_run};

void makronBFG (edict_t *self)
{
	vec3_t	forward, start;
	int		damage, speed;

	damage = 30 + 2 * self->monsterinfo.level;
	speed = 650 + 35 * self->monsterinfo.level;

	MonsterAim(self, 0.8, speed, true, MZ2_MAKRON_BFG, forward, start);
	monster_fire_bfg (self, start, forward, damage, speed, 0, 150.0, MZ2_MAKRON_BFG);
}	


mframe_t makron_frames_attack3 []=
{
	//ai_charge,	0,	NULL,			// 201
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	ai_charge,	0,	makronBFG,		// 204
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL			// 208
};
mmove_t makron_move_attack3 = {FRAME_attak304, FRAME_attak308, makron_frames_attack3, makron_run};

mframe_t makron_frames_attack4[]=
{
	//ai_charge,	0,	NULL,					// 209
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	ai_charge,	0,	MakronHyperblaster,		// fire 213
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	MakronHyperblaster,		// fire
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t makron_move_attack4 = {FRAME_attak405, FRAME_attak426, makron_frames_attack4, makron_run};

mframe_t makron_frames_attack5[]=
{
	ai_charge,	0,	makron_prerailgun,	// 235
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	MakronRailgun,		// 243 - Fire railgun
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL				// 250
};
mmove_t makron_move_attack5 = {FRAME_attak501, FRAME_attak516, makron_frames_attack5, makron_run};

// FIXME: He's not firing from the proper Z
void MakronRailgun (edict_t *self)
{
	vec3_t	forward, start;
	int		damage;

	damage = 150 + 35 * self->monsterinfo.level;

	MonsterAim(self, 0.5, 0, false, MZ2_MAKRON_RAILGUN_1, forward, start);

	monster_fire_railgun (self, start, forward, damage, 100, MZ2_MAKRON_RAILGUN_1);
}

// FIXME: This is all wrong. He's not firing at the proper angles.
void MakronHyperblaster (edict_t *self)
{
	vec3_t	forward, start;
	int		damage, speed, flash_number;

	flash_number = MZ2_MAKRON_BLASTER_1 + (self->s.frame - FRAME_attak405);

	damage = 50 + 10*self->monsterinfo.level;
	speed = 1000 + 50*self->monsterinfo.level;

	MonsterAim(self, 0.8, 1500, false, flash_number, forward, start);
	monster_fire_blaster(self, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BOLT, 2.0, true, flash_number);
}	

void makron_sight(edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = &makron_move_sight;
};

void makron_attack(edict_t *self)
{
	float	r, range;

	r = random();
	range = entdist(self, self->enemy);

	// medium range
	if (range <= 768)
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &makron_move_attack5; // railgun
		else if (r <= 0.6)
			self->monsterinfo.currentmove = &makron_move_attack4; // hyperblaster
		else
			self->monsterinfo.currentmove = &makron_move_attack3; // bfg	
	}
	// long range
	else
	{
		if (r <= 0.3)
			self->monsterinfo.currentmove = &makron_move_attack3; // bfg	
		else
			self->monsterinfo.currentmove = &makron_move_attack5; // railgun
	}

	self->monsterinfo.attack_finished = level.time + GetRandom(1, 3);
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
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->s.frame = 346;
	ent->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	ent->think = makron_torso_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	ent->s.sound = gi.soundindex ("makron/spine.wav");
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
	edict_t *tempent;

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
		//self->deadflag = DEAD_DEAD;
		M_Remove(self, false, false);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->s.origin[1] -= 84;
	makron_torso (tempent);

	self->monsterinfo.currentmove = &makron_move_death2;
	
}

//
// monster_makron
//

void MakronPrecache (void)
{
	sound_pain4 = gi.soundindex ("makron/pain3.wav");
	sound_pain5 = gi.soundindex ("makron/pain2.wav");
	sound_pain6 = gi.soundindex ("makron/pain1.wav");
	sound_death = gi.soundindex ("makron/death.wav");
	sound_step_left = gi.soundindex ("makron/step1.wav");
	sound_step_right = gi.soundindex ("makron/step2.wav");
	sound_attack_bfg = gi.soundindex ("makron/bfg_fire.wav");
	sound_brainsplorch = gi.soundindex ("makron/brain1.wav");
	sound_prerailgun = gi.soundindex ("makron/rail_up.wav");
	sound_popup = gi.soundindex ("makron/popup.wav");
	sound_taunt1 = gi.soundindex ("makron/voice4.wav");
	sound_taunt2 = gi.soundindex ("makron/voice3.wav");
	sound_taunt3 = gi.soundindex ("makron/voice.wav");
	sound_hit = gi.soundindex ("makron/bhit.wav");

	gi.modelindex ("models/monsters/boss3/rider/tris.md2");
}

/*QUAKED monster_makron (1 .5 0) (-30 -30 0) (30 30 90) Ambush Trigger_Spawn Sight
*/
void init_drone_makron (edict_t *self)
{
	MakronPrecache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	VectorSet (self->mins, -30, -30, 0);
	VectorSet (self->maxs, 30, 30, 90);

	self->health = self->max_health = 5000 * self->monsterinfo.level;
	self->monsterinfo.power_armor_power = 5000 * self->monsterinfo.level;
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;
	self->gib_health = -2000;
	self->mass = 500;
	self->mtype = M_MAKRON;
	self->monsterinfo.control_cost = 101;
	self->monsterinfo.cost = 300;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;

	//self->pain = makron_pain;
	self->die = makron_die;
	self->monsterinfo.stand = makron_stand;
	self->monsterinfo.walk = makron_walk;
	self->monsterinfo.run = makron_run;
	//self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = makron_attack;
	//self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = makron_sight;
	//self->monsterinfo.checkattack = Makron_CheckAttack;

	gi.linkentity (self);
	
//	self->monsterinfo.currentmove = &makron_move_stand;
	self->monsterinfo.currentmove = &makron_move_sight;
	self->monsterinfo.scale = MODEL_SCALE;
	self->nextthink = level.time + FRAMETIME;

	//walkmonster_start(self);
}


/*
=================
MakronSpawn

=================
*/
void MakronSpawn (edict_t *self)
{
	vec3_t		forward;
	edict_t		*makron;

	makron = SpawnDrone(self->activator, 33, true);
	VectorCopy(self->s.origin, makron->s.origin);
	gi.linkentity(makron);

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale(forward, 400, makron->velocity);
	makron->velocity[2] = 200;
	makron->groundentity = NULL;
	makron->gib_health = 0;// don't leave behind a body
	makron->monsterinfo.walk(makron);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
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

	ent = G_Spawn ();
	ent->activator = self->activator;
	ent->nextthink = level.time + 0.8;
	ent->think = MakronSpawn;
	//ent->target = self->target; GHz - for what?
	VectorCopy (self->s.origin, ent->s.origin);
	VectorCopy(self->s.angles, ent->s.angles);
}
