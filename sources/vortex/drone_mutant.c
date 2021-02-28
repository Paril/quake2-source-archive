/*
==============================================================================

mutant

==============================================================================
*/

#include "g_local.h"
#include "m_mutant.h"


static int	sound_swing;
static int	sound_hit;
static int	sound_hit2;
static int	sound_death;
static int	sound_idle;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_sight;
static int	sound_search;
static int	sound_step1;
static int	sound_step2;
static int	sound_step3;
static int	sound_thud;

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);
void mutant_jump (edict_t *self);
//
// SOUNDS
//

void mutant_step (edict_t *self)
{
	int		n;
	n = (rand() + 1) % 3;
	if (n == 0)
		gi.sound (self, CHAN_VOICE, sound_step1, 1, ATTN_NORM, 0);		
	else if (n == 1)
		gi.sound (self, CHAN_VOICE, sound_step2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_step3, 1, ATTN_NORM, 0);
}

void mutant_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void mutant_swing (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_swing, 1, ATTN_NORM, 0);
}


//
// STAND
//

mframe_t mutant_frames_stand [] =
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

	drone_ai_stand, 0, NULL
};
mmove_t mutant_move_stand = {FRAME_stand101, FRAME_stand151, mutant_frames_stand, NULL};

void mutant_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &mutant_move_stand;
}


//
// IDLE
//

void mutant_idle_loop (edict_t *self)
{
	if (random() < 0.75)
		self->monsterinfo.nextframe = FRAME_stand155;
	//if ((!(self->monsterinfo.aiflags & AI_NO_CIRCLE_STRAFE)))
	//	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
}

mframe_t mutant_frames_idle [] =
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,					// scratch loop start
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, mutant_idle_loop,		// scratch loop end
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t mutant_move_idle = {FRAME_stand152, FRAME_stand164, mutant_frames_idle, mutant_stand};

void mutant_idle (edict_t *self)
{
	self->monsterinfo.currentmove = &mutant_move_idle;
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void mutant_walk (edict_t *self);

mframe_t mutant_frames_walk [] =
{
	drone_ai_walk,	3,		NULL,
	drone_ai_walk,	1,		NULL,
	drone_ai_walk,	5,		NULL,
	drone_ai_walk,	10,		NULL,
	drone_ai_walk,	13,		NULL,
	drone_ai_walk,	10,		NULL,
	drone_ai_walk,	0,		NULL,
	drone_ai_walk,	5,		NULL,
	drone_ai_walk,	6,		NULL,
	drone_ai_walk,	16,		NULL,
	drone_ai_walk,	15,		NULL,
	drone_ai_walk,	6,		NULL
};
mmove_t mutant_move_walk = {FRAME_walk05, FRAME_walk16, mutant_frames_walk, NULL};

void mutant_walk_loop (edict_t *self)
{
	self->monsterinfo.currentmove = &mutant_move_walk;
}

mframe_t mutant_frames_start_walk [] =
{
	drone_ai_walk,	5,		NULL,
	drone_ai_walk,	5,		NULL,
	drone_ai_walk,	-2,		NULL,
	drone_ai_walk,	1,		NULL
};
mmove_t mutant_move_start_walk = {FRAME_walk01, FRAME_walk04, mutant_frames_start_walk, mutant_walk_loop};

void mutant_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &mutant_move_start_walk;
	//self->monsterinfo.currentmove = &mutant_move_walk;
}

//
// RUN
//

mframe_t mutant_frames_run [] =
{
	drone_ai_run,	30,		NULL,
	drone_ai_run,	30,		mutant_step,
	drone_ai_run,	30,		NULL,
	drone_ai_run,	30,		mutant_step,
	drone_ai_run,	30,		NULL,
	drone_ai_run,	30,		NULL
};
mmove_t mutant_move_run = {FRAME_run03, FRAME_run08, mutant_frames_run, NULL};

void mutant_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &mutant_move_stand;
	else
		self->monsterinfo.currentmove = &mutant_move_run;
}


//
// MELEE
//

void mutant_hit_left (edict_t *self)
{
	int damage;
	vec3_t	aim;

	if (!self->activator->client)
		damage = 25*self->monsterinfo.level;
	else damage = 10*self->monsterinfo.level;

	VectorSet (aim, 100, self->mins[0], 8);
	if (fire_hit (self, aim, damage, 100))
		gi.sound (self, CHAN_WEAPON, sound_hit, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_WEAPON, sound_swing, 1, ATTN_NORM, 0);
}

void mutant_hit_right (edict_t *self)
{
	int damage;
	vec3_t	aim;

	if (!self->activator->client)
		damage = 16*self->monsterinfo.level;
	else damage = 12*self->monsterinfo.level;

	VectorSet (aim, 100, self->maxs[0], 8);
	if (fire_hit (self, aim, damage, 100))
		gi.sound (self, CHAN_WEAPON, sound_hit2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_WEAPON, sound_swing, 1, ATTN_NORM, 0);
}

void mutant_check_refire (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (entdist(self, self->enemy) < 100) && (random() < 0.8))
		self->monsterinfo.nextframe = FRAME_attack11;
}

mframe_t mutant_frames_attack [] =
{
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	ai_charge,	0,	mutant_hit_left,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mutant_hit_right,
	ai_charge,	0,	mutant_check_refire
};
mmove_t mutant_move_attack = {FRAME_attack11, FRAME_attack15, mutant_frames_attack, mutant_run};

void mutant_melee (edict_t *self)
{
	//self->monsterinfo.currentmove = &mutant_move_attack;
}

//
// ATTACK
//
float monster_increaseDamageByTalent(edict_t *owner, float damage);

void mutant_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		damage, knockback;
	vec3_t	point, normal;

	
	if (self->health <= 0)
	{
		self->touch = NULL;
		return;
	}

	if (G_EntExists(other))
	{
		VectorCopy (self->velocity, normal);
		VectorNormalize(normal);
		VectorMA (self->s.origin, self->maxs[0], normal, point);
	
		//if (!self->activator->client)
		//	damage = 40*self->monsterinfo.level;
		//else 
			damage = 30 + 7*self->monsterinfo.level;
			damage = monster_increaseDamageByTalent(self->activator, damage);

		knockback = damage;
		if (knockback > 500)
			knockback = 500;
		T_Damage (other, self, self, self->velocity, point, normal, damage, knockback, 0, MOD_UNKNOWN);
	}

	if (!M_CheckBottom(self))
	{
		if (self->groundentity)
			self->touch = NULL;
		return;
	}

	self->touch = NULL;
}

void mutant_jump_takeoff (edict_t *self)
{
	int	speed = 700;
	vec3_t	forward, start;

	//4.5 monster bonus flags
	if (self->monsterinfo.bonus_flags & BF_FANATICAL)
		speed *= 2;

	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	self->lastsound = level.framenum;

	self->groundentity = NULL;

	MonsterAim(self, -1, speed, false, 0, forward, start);
	self->velocity[0] += forward[0] * speed;
	self->velocity[1] += forward[1] * speed;
	self->velocity[2] += forward[2] * speed;

	//self->touch = mutant_jump_touch;
	self->monsterinfo.pausetime = level.time + 2; // maximum duration of jump
	self->velocity[2] += 300;

	self->s.frame = FRAME_attack05; // skip to airborne jump frame
	self->monsterinfo.aiflags |= AI_HOLD_FRAME; // hold this frame
}

void mutant_check_landing (edict_t *self)
{
	//if (!self->enemy)
	//	return;

	// check for landing or jump timeout
	if (self->groundentity || (self->waterlevel > 1)
		|| (level.time > self->monsterinfo.pausetime))
	{
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		//VectorClear(self->velocity);
	}
	else
	{
		// we're still in the air
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}

}

mframe_t mutant_frames_jump [] =
{
	ai_charge,	 0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mutant_jump_takeoff,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mutant_check_landing,
	ai_charge,	 0,	NULL,
//	ai_charge,	 0,	NULL,
//	ai_charge,	 0,	NULL
};
mmove_t mutant_move_jump = {FRAME_attack01, FRAME_attack06, mutant_frames_jump, mutant_run};

void mutant_jump (edict_t *self)
{
	self->monsterinfo.currentmove = &mutant_move_jump;
}

void mutant_attack (edict_t *self)
{
	float	dist;

	// can only attack with our feet on the ground unless we're in the water
	if (!self->groundentity && !self->waterlevel)
		return;

	dist = entdist(self, self->enemy);

	// attack enemy if he's visible and in range
	if (visible(self, self->enemy))
	{
		if (dist <= 100 && random() < 0.8)
			self->monsterinfo.currentmove = &mutant_move_attack;
		else if (dist <= 512)
			mutant_jump(self);
	}
}

//
// DEATH
//

void mutant_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);

//	M_FlyCheck (self);
}

mframe_t mutant_frames_death1 [] =
{
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
mmove_t mutant_move_death1 = {FRAME_death101, FRAME_death109, mutant_frames_death1, mutant_dead};

mframe_t mutant_frames_death2 [] =
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
	ai_move,	0,	NULL
};
mmove_t mutant_move_death2 = {FRAME_death201, FRAME_death210, mutant_frames_death2, mutant_dead};

void mutant_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

#ifdef OLD_NOLAG_STYLE
	// reduce lag by removing the entity right away
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}
#endif

	//GHz: Check for gibbed body
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		//self->deadflag = DEAD_DEAD;
#ifdef OLD_NOLAG_STYLE
		M_Remove(self, false, false);
#else
		if (nolag->value)
			M_Remove(self, false, true);
		else
			M_Remove(self, false, false);
#endif
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	//GHz: Begin death sequence
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->s.skinnum = 1;

	if (random() < 0.5)
		self->monsterinfo.currentmove = &mutant_move_death1;
	else
		self->monsterinfo.currentmove = &mutant_move_death2;
	
	DroneList_Remove(self);

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}

}


//
// SPAWN
//

/*QUAKED monster_mutant (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void init_drone_mutant (edict_t *self)
{
	sound_swing = gi.soundindex ("mutant/mutatck1.wav");
	sound_hit = gi.soundindex ("mutant/mutatck2.wav");
	sound_hit2 = gi.soundindex ("mutant/mutatck3.wav");
	sound_death = gi.soundindex ("mutant/mutdeth1.wav");
	sound_idle = gi.soundindex ("mutant/mutidle1.wav");
	0;//sound_pain1 = gi.soundindex ("mutant/mutpain1.wav");
	0;//sound_pain2 = gi.soundindex ("mutant/mutpain2.wav");
	sound_sight = gi.soundindex ("mutant/mutsght1.wav");
	0;//sound_search = gi.soundindex ("mutant/mutsrch1.wav");
	sound_step1 = gi.soundindex ("mutant/step1.wav");
	sound_step2 = gi.soundindex ("mutant/step2.wav");
	sound_step3 = gi.soundindex ("mutant/step3.wav");
	sound_thud = gi.soundindex ("mutant/thud1.wav");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/mutant/tris.md2");
	//VectorSet (self->mins, -32, -32, -24);
	//VectorSet (self->maxs, 32, 32, 48);
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 32);

	//if (self->activator && self->activator->client)
	self->health = 135 + 75*self->monsterinfo.level;
	//else
		//self->health = 400 + 160*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 300;

//	self->pain = mutant_pain;
	self->die = mutant_die;

	self->monsterinfo.stand = mutant_stand;
	self->monsterinfo.walk = mutant_walk;
	self->monsterinfo.run = mutant_run;
//	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = mutant_attack;
	self->monsterinfo.melee = mutant_melee;
	self->monsterinfo.sight = mutant_sight;
//	self->monsterinfo.search = mutant_search;
	//self->monsterinfo.idle = mutant_idle;
//	self->monsterinfo.checkattack = mutant_checkattack;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
//	self->monsterinfo.melee = 1;

	//K03 Begin
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = 0;
	self->monsterinfo.control_cost = M_MUTANT_CONTROL_COST;
	self->monsterinfo.cost = M_MUTANT_COST;
	self->mtype = M_MUTANT;
	//K03 End

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &mutant_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
	self->nextthink = level.time + FRAMETIME;
//	walkmonster_start (self);

	//self->nextthink = level.time + 0.1;
	//self->activator->num_monsters += self->monsterinfo.control_cost;
}
