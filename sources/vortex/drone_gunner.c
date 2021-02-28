/*
==============================================================================

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"


static int	sound_pain;
static int	sound_pain2;
static int	sound_death;
static int	sound_idle;
static int	sound_open;
static int	sound_search;
static int	sound_sight;
static int	sound_thud;

void mygunner_continue (edict_t *self);
void mygunner_refire_chain(edict_t *self);
void mygunner_fire_chain(edict_t *self);
void mygunner_delay (edict_t *self);
void gunner_attack_grenade (edict_t *self);
void gunner_refire_grenade (edict_t *self);
void drone_ai_walk (edict_t *self, float dist);

void mygunneridlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void mygunnersearch (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}

void mygunnerstand (edict_t *self);

mframe_t mygunnerframes_fidget [] =
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, mygunneridlesound,
	drone_ai_stand, 0, NULL,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t	mygunnermove_fidget = {FRAME_stand31, FRAME_stand70, mygunnerframes_fidget, mygunnerstand};

void mygunnerfidget (edict_t *self)
{
//	if (self->monsterinfo.aiflags & drone_ai_stand_GROUND)
	//	return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &mygunnermove_fidget;
}

mframe_t mygunnerframes_stand [] =
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
	drone_ai_stand, 0, mygunnerfidget,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, mygunnerfidget,

	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, mygunnerfidget
};
mmove_t	mygunnermove_stand = {FRAME_stand01, FRAME_stand30, mygunnerframes_stand, NULL};

void mygunnerstand (edict_t *self)
{
		self->monsterinfo.currentmove = &mygunnermove_stand;
}

mframe_t gunner_frames_walk [] =
{
	drone_ai_walk, 0, NULL,
	drone_ai_walk, 3, NULL,
	drone_ai_walk, 4, NULL,
	drone_ai_walk, 5, NULL,
	drone_ai_walk, 7, NULL,
	drone_ai_walk, 2, NULL,
	drone_ai_walk, 6, NULL,
	drone_ai_walk, 4, NULL,
	drone_ai_walk, 2, NULL,
	drone_ai_walk, 7, NULL,
	drone_ai_walk, 5, NULL,
	drone_ai_walk, 7, NULL,
	drone_ai_walk, 4, NULL
};
mmove_t gunner_move_walk = {FRAME_walk07, FRAME_walk19, gunner_frames_walk, NULL};

void gunner_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &gunner_move_walk;
}

mframe_t mygunnerframes_run [] =
{
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL
};

mmove_t mygunnermove_run = {FRAME_run01, FRAME_run08, mygunnerframes_run, NULL};

void mygunnerrun (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &mygunnermove_stand;
	else
		self->monsterinfo.currentmove = &mygunnermove_run;
}

void myGunnerGrenade (edict_t *self)
{
	int		damage, speed, flash_number;
	vec3_t	forward, start;

	// sanity check
	if (!self->enemy || !self->enemy->inuse)
		return;

	damage = 28 + 6*self->monsterinfo.level;
	speed = 600 + 30*self->monsterinfo.level;

	if (speed > 900)
		speed = 900;

	if (self->s.frame == FRAME_attak105)
		flash_number = MZ2_GUNNER_GRENADE_1;
	else
		flash_number = MZ2_GUNNER_GRENADE_4;

	MonsterAim(self, 0.9, speed, false, flash_number, forward, start);
	monster_fire_grenade(self, start, forward, damage, speed, flash_number);
}

mframe_t mygunner_frames_attack_grenade_start [] =
{
	ai_charge, 0, NULL,					// 108
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL					// 111
};
mmove_t mygunner_move_attack_grenade_start = {FRAME_attak101, FRAME_attak104, mygunner_frames_attack_grenade_start, gunner_attack_grenade};

mframe_t mygunner_frames_attack_grenade_end [] =
{
	ai_charge, 0, NULL,					// 122
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, mygunner_delay		// 128
};
mmove_t mygunner_move_attack_grenade_end = {FRAME_attak115, FRAME_attak121, mygunner_frames_attack_grenade_end, mygunnerrun};

mframe_t mygunner_frames_attack_grenade [] =
{
	ai_charge, 0, myGunnerGrenade,		// 112
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, myGunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, myGunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
	//ai_charge, 0, myGunnerGrenade		// 121
};
mmove_t mygunner_move_attack_grenade = {FRAME_attak105, FRAME_attak113, mygunner_frames_attack_grenade, gunner_refire_grenade};

void gunner_refire_grenade (edict_t *self)
{
	// continue firing unless enemy is no longer valid or out of range
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.8)
		&& (entdist(self, self->enemy) <= 384))
		self->monsterinfo.currentmove = &mygunner_move_attack_grenade;
	else
		self->monsterinfo.currentmove = &mygunner_move_attack_grenade_end;

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 0.5;
}

void gunner_attack_grenade (edict_t *self)
{
	// continue attack sequence unless enemy is no longer valid
	if (G_ValidTarget(self, self->enemy, true))
		self->monsterinfo.currentmove = &mygunner_move_attack_grenade;
	else
		mygunnerrun(self);
}
/*
mframe_t mygunner_frames_attack_grenade [] =
{
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, myGunnerGrenade,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, myGunnerGrenade,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0, NULL
};
mmove_t mygunner_move_attack_grenade = {FRAME_attak101, FRAME_attak110, mygunner_frames_attack_grenade, mygunnerrun};
*/
mframe_t mygunner_frames_runandshoot [] =
{
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, myGunnerGrenade,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL
};

mmove_t mygunner_move_runandshoot = {FRAME_runs01, FRAME_runs06, mygunner_frames_runandshoot, mygunner_continue};

void mygunner_continue (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.9) 
		&& (entdist(self, self->enemy) <= 512))
		self->monsterinfo.currentmove = &mygunner_move_runandshoot;
	else
		self->monsterinfo.currentmove = &mygunnermove_run;

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 1.0;
}

void mygunner_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &mygunner_move_runandshoot;
}

void myGunnerFire (edict_t *self)
{
	int		damage, flash_number;
	vec3_t	forward, start;

	// sanity check
	if (!self->enemy || !self->enemy->inuse)
		return;

	flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);
 
	damage = 7 + self->monsterinfo.level;

	MonsterAim(self, 0.9, 0, false, flash_number, forward, start);

	monster_fire_bullet (self, start, forward, damage, damage, 
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

void mygunner_opengun (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_open, 1, ATTN_IDLE, 0);
}

mframe_t mygunner_frames_attack_chain [] =
{
	ai_charge, 0, mygunner_opengun,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t mygunner_move_attack_chain = {FRAME_attak209, FRAME_attak215, mygunner_frames_attack_chain, mygunner_fire_chain};

mframe_t mygunner_frames_fire_chain [] =
{
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire,
	ai_charge,   0, myGunnerFire
};
mmove_t mygunner_move_fire_chain = {FRAME_attak216, FRAME_attak223, mygunner_frames_fire_chain, mygunner_refire_chain};

void mygunner_delay (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse)
		return;

	// delay next attack if we're not standing ground, our enemy isn't within close range
	// (we need to get closer) and we are not a tank commander/boss
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND) && (entdist(self, self->enemy) > 256))
		self->monsterinfo.attack_finished = level.time + GetRandom(5, 20)*FRAMETIME;
}

mframe_t mygunner_frames_endfire_chain [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t mygunner_move_endfire_chain = {FRAME_attak224, FRAME_attak230, mygunner_frames_endfire_chain, mygunnerrun};

void mygunner_fire_chain(edict_t *self)
{
	self->monsterinfo.currentmove = &mygunner_move_fire_chain;
}

void mygunner_refire_chain(edict_t *self)
{
	// keep firing
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.8)
		&& entdist(self, self->enemy) > 128)
		self->monsterinfo.currentmove = &mygunner_move_fire_chain;
	else
		self->monsterinfo.currentmove = &mygunner_move_endfire_chain;

	//mygunner_delay(self);

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 0.5;
}

void gunner_stand_attack (edict_t *self)
{
	if (entdist(self, self->enemy) <= 384 && random() <= 0.8)
		self->monsterinfo.currentmove = &mygunner_move_attack_grenade;
	else
		self->monsterinfo.currentmove = &mygunner_move_attack_chain;
}

void gunner_attack (edict_t *self)
{
	float	r = random();
	float	dist = entdist(self, self->enemy);

	// short range (20% chance grenade, 80% chance run and shoot)
	if (dist <= 128)
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &mygunner_move_attack_grenade;
		else
			self->monsterinfo.currentmove = &mygunner_move_runandshoot;
	}
	// medium range (100% run and shoot)
	else if (dist <= 512)
	{
		self->monsterinfo.currentmove = &mygunner_move_runandshoot;
	}
	// long range (50% chance chaingun)
	else
	{
		if (r <= 0.5)
			self->monsterinfo.currentmove = &mygunner_move_attack_chain;
		else
			self->monsterinfo.attack_finished = level.time + 2.0;// don't attack, try to get closer
	}
}

void mygunner_attack (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		gunner_stand_attack(self);
	else
		gunner_attack(self);
}

void mygunner_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	if (!self->groundentity)
		return;

	if (self->enemy && (random() > 0.5))
		myGunnerGrenade (self);

	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] = 0;
	self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void mygunner_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	VectorClear(self->velocity);
	gi.linkentity (self);
}

mframe_t mygunner_frames_duck [] =
{
	ai_move, 0,  mygunner_duck_down,
	ai_move, 0,  NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,  mygunner_duck_up,
};
mmove_t	mygunner_move_duck = {FRAME_duck03, FRAME_duck07, mygunner_frames_duck, mygunnerrun};

void mygunner_jump_takeoff (edict_t *self)
{
	vec3_t	v;

	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	VectorSubtract(self->monsterinfo.dir, self->s.origin, v);
	v[2] = 0;
	VectorNormalize(v);
	VectorScale(v, -200, self->velocity);
	self->velocity[2] = 400;
	self->monsterinfo.pausetime = level.time + 2.0; // maximum duration of jump
}

void mygunner_jump_hold (edict_t *self)
{
	vec3_t	v;

	if (G_EntExists(self->monsterinfo.attacker))
	{
		// face the attacker
		VectorSubtract(self->monsterinfo.attacker->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);
		M_ChangeYaw(self);
	}
	// check for landing or jump timeout
	if (self->groundentity || (level.time > self->monsterinfo.pausetime))
	{
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		VectorClear(self->velocity);
	}
	else
	{
		// we're still in the air
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}
}

mframe_t mygunner_frames_leap [] =
{
	ai_move,	0,	mygunner_jump_takeoff,
	ai_move,	0,	NULL,
	ai_move,	0,	mygunner_jump_hold,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mygunner_move_leap = {FRAME_duck01, FRAME_duck08, mygunner_frames_leap, mygunnerrun};

void mygunner_leap (edict_t *self)
{
	if (self->groundentity)
		self->monsterinfo.currentmove = &mygunner_move_leap;
}

void mygunner_dodge (edict_t *self, edict_t *attacker, vec3_t dir, int radius)
{
	if (random() > 0.9)
		return;
	if (level.time < self->monsterinfo.dodge_time)
		return;
	if (OnSameTeam(self, attacker))
		return;

	if (!self->enemy && G_EntIsAlive(attacker))
		self->enemy = attacker;
	if (!radius)
	{
		self->monsterinfo.currentmove = &mygunner_move_duck;
		self->monsterinfo.dodge_time = level.time + 2.0;
	}
	else
	{
		mygunner_leap(self);
		self->monsterinfo.dodge_time = level.time + 3.0;
	}
}

void mygunnerdead (edict_t *self)
{
	//gi.dprintf("mygunnerdead()\n");

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t mygunnerframes_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -7, NULL,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t mygunnermove_death = {FRAME_death01, FRAME_death11, mygunnerframes_death, mygunnerdead};

void mygunnerdie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

// check for gib
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

	DroneList_Remove(self);

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &mygunnermove_death;

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void init_drone_gunner (edict_t *self)
{
	sound_death = gi.soundindex ("gunner/death1.wav");	
	sound_pain = gi.soundindex ("gunner/gunpain2.wav");	
	sound_pain2 = gi.soundindex ("gunner/gunpain1.wav");	
	sound_idle = gi.soundindex ("gunner/gunidle1.wav");	
	sound_open = gi.soundindex ("gunner/gunatck1.wav");	
	sound_search = gi.soundindex ("gunner/gunsrch1.wav");	
	sound_sight = gi.soundindex ("gunner/sight1.wav");
	sound_thud = gi.soundindex ("player/land1.wav");

	gi.soundindex ("gunner/gunatck2.wav");
	gi.soundindex ("gunner/gunatck3.wav");

	self->monsterinfo.control_cost = M_GUNNER_CONTROL_COST;
	self->monsterinfo.cost = M_GUNNER_COST;
	self->s.modelindex = gi.modelindex ("models/monsters/gunner/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	//if (self->activator && self->activator->client)
	self->health = 120 + 75*self->monsterinfo.level;
	//else self->health = 100 + 30*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 200;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.jumpup = 64;
	
	if (random() > 0.5)
		self->item = FindItemByClassname("ammo_bullets");
	else
		self->item = FindItemByClassname("ammo_grenades");

	self->die = mygunnerdie;

	self->monsterinfo.stand = mygunnerstand;
	self->monsterinfo.run = mygunnerrun;
	self->monsterinfo.dodge = mygunner_dodge;
	self->monsterinfo.attack = mygunner_attack;
	self->monsterinfo.walk = gunner_walk;

	//K03 Begin
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;

	//if (self->activator && self->activator->client)
		self->monsterinfo.power_armor_power = 60 + 45*self->monsterinfo.level;
	//else self->monsterinfo.power_armor_power = 100 + 50*self->monsterinfo.level;

	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;
	self->mtype = M_GUNNER;
	//K03 End

	gi.linkentity (self);

	self->monsterinfo.currentmove = &mygunnermove_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start (self);
	self->nextthink = level.time + FRAMETIME;
}
