/*
==============================================================================

jorg

==============================================================================
*/

#include "g_local.h"
#include "m_boss31.h"

qboolean visible (edict_t *self, edict_t *other);
void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_idle;
static int	sound_death;
static int	sound_search1;
static int	sound_search2;
static int	sound_search3;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_firegun;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_death_hit;

void BossExplode (edict_t *self);
void MakronToss (edict_t *self);

/*
void jorg_search (edict_t *self)
{
	float r;

	r = random();

	if (r <= 0.3)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	else if (r <= 0.6)
		gi.sound (self, CHAN_VOICE, sound_search2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_search3, 1, ATTN_NORM, 0);
}
*/


void jorg_dead (edict_t *self);
void jorgBFG (edict_t *self);
void jorgMachineGun (edict_t *self);
void jorg_firebullet (edict_t *self);
void jorg_reattack1(edict_t *self);
void jorg_attack1(edict_t *self);
void jorg_idle(edict_t *self);
void jorg_step_left(edict_t *self);
void jorg_step_right(edict_t *self);
void jorg_death_hit(edict_t *self);

//
// stand
//

mframe_t jorg_frames_stand []=
{
	drone_ai_stand, 0, jorg_idle,
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
	drone_ai_stand, 19, NULL,
	drone_ai_stand, 11, jorg_step_left,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 6, NULL,
	drone_ai_stand, 9, jorg_step_right,
	drone_ai_stand, 0, NULL,		// 40
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, -2, NULL,
	drone_ai_stand, -17, jorg_step_left,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, -12, NULL,		// 50
	drone_ai_stand, -14, jorg_step_right	// 51
};
mmove_t	jorg_move_stand = {FRAME_stand01, FRAME_stand51, jorg_frames_stand, NULL};

void jorg_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM,0);
}

void jorg_death_hit (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_death_hit, 1, ATTN_NORM,0);
}


void jorg_step_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
}

void jorg_step_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
}

void jorg_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &jorg_move_stand;
}

mframe_t jorg_frames_run [] =
{
	drone_ai_run, 17,	jorg_step_left,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 8,	NULL,
	drone_ai_run, 10,	NULL,
	drone_ai_run, 33,	jorg_step_right,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 0,	NULL,
	drone_ai_run, 9,	NULL,
	drone_ai_run, 9,	NULL,
	drone_ai_run, 9,	NULL
};
mmove_t	jorg_move_run = {FRAME_walk06, FRAME_walk19, jorg_frames_run, NULL};

//
// walk
//

mframe_t jorg_frames_start_walk [] =
{
	drone_ai_walk,	5,	NULL,
	drone_ai_walk,	6,	NULL,
	drone_ai_walk,	7,	NULL,
	drone_ai_walk,	9,	NULL,
	drone_ai_walk,	15,	NULL
};
mmove_t jorg_move_start_walk = {FRAME_walk01, FRAME_walk05, jorg_frames_start_walk, NULL};

mframe_t jorg_frames_walk [] =
{
	drone_ai_walk, 17,	jorg_step_left,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 12,	NULL,
	drone_ai_walk, 8,	NULL,
	drone_ai_walk, 10,	NULL,
	drone_ai_walk, 33,	jorg_step_right,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 9,	NULL,
	drone_ai_walk, 9,	NULL,
	drone_ai_walk, 9,	NULL
};
mmove_t	jorg_move_walk = {FRAME_walk06, FRAME_walk19, jorg_frames_walk, NULL};

mframe_t jorg_frames_end_walk [] =
{
	drone_ai_walk,	11,	NULL,
	drone_ai_walk,	0,	NULL,
	drone_ai_walk,	0,	NULL,
	drone_ai_walk,	0,	NULL,
	drone_ai_walk,	8,	NULL,
	drone_ai_walk,	-8,	NULL
};
mmove_t jorg_move_end_walk = {FRAME_walk20, FRAME_walk25, jorg_frames_end_walk, NULL};

void jorg_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;

	self->monsterinfo.currentmove = &jorg_move_walk;
}

void jorg_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &jorg_move_stand;
	else
		self->monsterinfo.currentmove = &jorg_move_run;
}

void jorgBFG_refire (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && entdist(self, self->enemy) <= 768 && random() <= 0.8)
		self->s.frame = 18;

	self->monsterinfo.attack_finished = level.time + 2.0;
}

mframe_t jorg_frames_death1 [] =
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
	ai_move,	0,	NULL,		// 10
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,		// 20
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,		// 30
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,		// 40
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	MakronToss,
	ai_move,	0,	BossExplode		// 50
};
mmove_t jorg_move_death = {FRAME_death01, FRAME_death50, jorg_frames_death1, jorg_dead};

mframe_t jorg_frames_attack2 []=
{
	//ai_charge,	0,	NULL,	// 18
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	ai_charge,	0,	jorgBFG,// 24	
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	jorgBFG_refire	// 30
};
mmove_t jorg_move_attack2 = {FRAME_attak207, FRAME_attak213, jorg_frames_attack2, jorg_run};

mframe_t jorg_frames_start_attack1 [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t jorg_move_start_attack1 = {FRAME_attak101, FRAME_attak108, jorg_frames_start_attack1, jorg_attack1};

mframe_t jorg_frames_attack1[]=
{
	ai_charge,	0,	jorg_firebullet,
	ai_charge,	0,	jorg_firebullet,
	ai_charge,	0,	jorg_firebullet,
	ai_charge,	0,	jorg_firebullet,
	ai_charge,	0,	jorg_firebullet,
	ai_charge,	0,	jorg_firebullet
};
mmove_t jorg_move_attack1 = {FRAME_attak109, FRAME_attak114, jorg_frames_attack1, jorg_reattack1};

mframe_t jorg_frames_end_attack1[]=
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t jorg_move_end_attack1 = {FRAME_attak115, FRAME_attak118, jorg_frames_end_attack1, jorg_run};

void jorg_reattack1(edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && random() <= 0.9)
	{
		self->monsterinfo.currentmove = &jorg_move_attack1;
	}
	else
	{
		self->s.sound = 0;
		self->monsterinfo.currentmove = &jorg_move_end_attack1;
	}

	self->monsterinfo.attack_finished = level.time + 2.0;
}

void jorg_attack1(edict_t *self)
{
	self->monsterinfo.currentmove = &jorg_move_attack1;
}

void jorgBFG (edict_t *self)
{
	int		damage, speed;
	vec3_t	forward, start;

	gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM, 0);

	damage = 30 + 2 * self->monsterinfo.level;
	speed = 650 + 35 * self->monsterinfo.level;

	MonsterAim(self, 0.8, speed, true, MZ2_JORG_BFG_1, forward, start);
	monster_fire_bfg (self, start, forward, damage, speed, 0, 150.0, MZ2_JORG_BFG_1);
}	

void jorg_firebullet_right (edict_t *self)
{
	int		damage;
	vec3_t	forward, start;

	damage = 20 + 2*self->monsterinfo.level;

	MonsterAim(self, 0.8, 0, false, MZ2_JORG_MACHINEGUN_R1, forward, start);

	monster_fire_bullet (self, start, forward, damage, damage, 
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_JORG_MACHINEGUN_R1);
}	

void jorg_firebullet_left (edict_t *self)
{
	int		damage;
	vec3_t	forward, start;

	damage = 20 + 2*self->monsterinfo.level;

	MonsterAim(self, 0.8, 0, false, MZ2_JORG_MACHINEGUN_L1, forward, start);

	monster_fire_bullet (self, start, forward, damage, damage, 
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_JORG_MACHINEGUN_L1);
}	

void jorg_firebullet (edict_t *self)
{
	jorg_firebullet_left(self);
	jorg_firebullet_right(self);
};

void jorg_attack(edict_t *self)
{
	float	r, range;
	
	r = random();
	range = entdist(self, self->enemy);

	// medium range attack
	if (range <= 768)
	{
		if (r > 0.5)
		{
			// BFG attack
			gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM,0);
			self->monsterinfo.currentmove = &jorg_move_attack2;
		}
		else
		{
			// chaingun attack
			gi.sound (self, CHAN_VOICE, sound_attack1, 1, ATTN_NORM,0);
			self->s.sound = gi.soundindex ("boss3/w_loop.wav");
			self->monsterinfo.currentmove = &jorg_move_attack1;
		}
	}
	// long range attack
	else
	{
		if (r <= 0.33)
		{
			gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM,0);
			self->monsterinfo.currentmove = &jorg_move_attack2;
		}
		else
		{
			gi.sound (self, CHAN_VOICE, sound_attack1, 1, ATTN_NORM,0);
			self->s.sound = gi.soundindex ("boss3/w_loop.wav");
			self->monsterinfo.currentmove = &jorg_move_start_attack1;
		}
	}

	self->monsterinfo.attack_finished = level.time + 1.0;
}

void jorg_dead (edict_t *self)
{
#if 0
	edict_t	*tempent;
	/*
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	*/
	
	// Jorg is on modelindex2. Do not clear him.
	VectorSet (self->mins, -60, -60, 0);
	VectorSet (self->maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->nextthink = 0;
	gi.linkentity (self);

	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->killtarget = self->killtarget;
	tempent->target = self->target;
	tempent->activator = self->enemy;
	self->killtarget = 0;
	self->target = 0;
	SP_monster_makron (tempent);
#endif
}


void jorg_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->s.sound = 0;
	self->count = 0;
	self->monsterinfo.currentmove = &jorg_move_death;
}

/*
qboolean Jorg_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	vec3_t	temp;
	float	chance;
	trace_t	tr;
	qboolean	enemy_infront;
	int			enemy_range;
	float		enemy_yaw;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);

	self->ideal_yaw = enemy_yaw;


	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
// missile attack
	if (!self->monsterinfo.attack)
		return false;
		
	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (enemy_range == RANGE_FAR)
		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.2;
	}
	else
	{
		return false;
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2*random();
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}

*/
void MakronPrecache (void);

/*QUAKED monster_jorg (1 .5 0) (-80 -80 0) (90 90 140) Ambush Trigger_Spawn Sight
*/
void init_drone_jorg (edict_t *self)
{
	sound_pain1 = gi.soundindex ("boss3/bs3pain1.wav");
	sound_pain2 = gi.soundindex ("boss3/bs3pain2.wav");
	sound_pain3 = gi.soundindex ("boss3/bs3pain3.wav");
	sound_death = gi.soundindex ("boss3/bs3deth1.wav");
	sound_attack1 = gi.soundindex ("boss3/bs3atck1.wav");
	sound_attack2 = gi.soundindex ("boss3/bs3atck2.wav");
	sound_search1 = gi.soundindex ("boss3/bs3srch1.wav");
	sound_search2 = gi.soundindex ("boss3/bs3srch2.wav");
	sound_search3 = gi.soundindex ("boss3/bs3srch3.wav");
	sound_idle = gi.soundindex ("boss3/bs3idle1.wav");
	sound_step_left = gi.soundindex ("boss3/step1.wav");
	sound_step_right = gi.soundindex ("boss3/step2.wav");
	sound_firegun = gi.soundindex ("boss3/xfire.wav");
	sound_death_hit = gi.soundindex ("boss3/d_hit.wav");

	//MakronPrecache (); - what does this do? GHz

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/boss3/jorg/tris.md2");
	VectorSet (self->mins, -80, -80, 0);
	VectorSet (self->maxs, 80, 80, 140);

	self->health = self->max_health = 10000 * self->monsterinfo.level;
	self->monsterinfo.power_armor_power = 10000 * self->monsterinfo.level;
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;
	self->mtype = M_JORG;
	self->monsterinfo.control_cost = 101;
	self->monsterinfo.cost = 300;
	self->gib_health = -2000;
	self->mass = 1000;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;

	//self->pain = jorg_pain;
	self->die = jorg_die;
	self->monsterinfo.stand = jorg_stand;
	self->monsterinfo.walk = jorg_walk;
	self->monsterinfo.run = jorg_run;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	//self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = jorg_attack;
	//self->monsterinfo.search = jorg_search;
	//self->monsterinfo.melee = NULL;
	//self->monsterinfo.sight = NULL;
	//self->monsterinfo.checkattack = Jorg_CheckAttack;
	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &jorg_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	self->nextthink = level.time + FRAMETIME;
	//walkmonster_start(self);

	G_PrintGreenText(va("A level %d jorg has spawned!", self->monsterinfo.level));
}
