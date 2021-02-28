/*
==============================================================================

chick

==============================================================================
*/

#include "g_local.h"
#include "m_chick.h"

qboolean visible (edict_t *self, edict_t *other);

void mychick_stand (edict_t *self);
void mychick_run (edict_t *self);
void mychick_reslash(edict_t *self);
void mychick_rerocket(edict_t *self);
void mychick_attack1(edict_t *self);
void mychick_continue (edict_t *self);
void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

static int	sound_missile_prelaunch;
static int	sound_missile_launch;
static int	sound_melee_swing;
static int	sound_melee_hit;
static int	sound_missile_reload;
static int	sound_death1;
static int	sound_death2;
static int	sound_fall_down;
static int	sound_idle1;
static int	sound_idle2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_sight;
static int	sound_search;

void myChickMoan (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
}

mframe_t mychick_frames_fidget [] =
{
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  myChickMoan,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL,
	drone_ai_stand, 0,  NULL
};
mmove_t mychick_move_fidget = {FRAME_stand201, FRAME_stand230, mychick_frames_fidget, mychick_stand};

void mychick_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.3)
		self->monsterinfo.currentmove = &mychick_move_fidget;
}

mframe_t mychick_frames_stand [] =
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
	drone_ai_stand, 0, mychick_fidget,

};
mmove_t mychick_move_stand = {FRAME_stand101, FRAME_stand130, mychick_frames_stand, NULL};

void mychick_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &mychick_move_stand;
}

mframe_t chick_frames_walk [] =
{
	drone_ai_walk, 6,	 NULL,
	drone_ai_walk, 8,  NULL,
	drone_ai_walk, 13, NULL,
	drone_ai_walk, 5,  NULL,
	drone_ai_walk, 7,  NULL,
	drone_ai_walk, 4,  NULL,
	drone_ai_walk, 11, NULL,
	drone_ai_walk, 5,  NULL,
	drone_ai_walk, 9,  NULL,
	drone_ai_walk, 7,  NULL
};

mmove_t chick_move_walk = {FRAME_walk11, FRAME_walk20, chick_frames_walk, NULL};

void chick_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &chick_move_walk;
}

mframe_t mychick_frames_start_run [] =
{
	drone_ai_run, 1,  NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0,	 NULL,
	drone_ai_run, -1, NULL, 
	drone_ai_run, -1, NULL, 
	drone_ai_run, 0,  NULL,
	drone_ai_run, 1,  NULL,
	drone_ai_run, 3,  NULL,
	drone_ai_run, 6,	 NULL,
	drone_ai_run, 3,	 NULL
};
mmove_t mychick_move_start_run = {FRAME_walk01, FRAME_walk10, mychick_frames_start_run, mychick_run};

mframe_t mychick_frames_run [] =
{
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25,  NULL,
	drone_ai_run, 25,  NULL

};
mmove_t mychick_move_run = {FRAME_walk11, FRAME_walk20, mychick_frames_run, NULL};

void mychick_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &mychick_move_stand;
		return;
	}
	self->monsterinfo.currentmove = &mychick_move_run;
}

void mychick_dead (edict_t *self)
{
//	gi.dprintf("mychick_dead()\n");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t mychick_frames_death2 [] =
{
	ai_move, -6, NULL,
	ai_move, 0,  NULL,
	ai_move, -1,  NULL,
	ai_move, -5, NULL,
	ai_move, 0, NULL,
	ai_move, -1,  NULL,
	ai_move, -2,  NULL,
	ai_move, 1,  NULL,
	ai_move, 10, NULL,
	ai_move, 2,  NULL,
	ai_move, 3,  NULL,
	ai_move, 1,  NULL,
	ai_move, 2, NULL,
	ai_move, 0,  NULL,
	ai_move, 3,  NULL,
	ai_move, 3,  NULL,
	ai_move, 1,  NULL,
	ai_move, -3,  NULL,
	ai_move, -5, NULL,
	ai_move, 4, NULL,
	ai_move, 15, NULL,
	ai_move, 14, NULL,
	ai_move, 1, NULL
};
mmove_t mychick_move_death2 = {FRAME_death201, FRAME_death223, mychick_frames_death2, mychick_dead};

mframe_t mychick_frames_death1 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, -7, NULL,
	ai_move, 4,  NULL,
	ai_move, 11, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
	
};
mmove_t mychick_move_death1 = {FRAME_death101, FRAME_death112, mychick_frames_death1, mychick_dead};

void mychick_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

	// reduce lag by removing the entity right away
#ifdef OLD_NOLAG_STYLE
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

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	//level.total_monsters--;

	n = rand() % 2;
	if (n == 0)
	{
		self->monsterinfo.currentmove = &mychick_move_death1;
		gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	}
	else
	{
		self->monsterinfo.currentmove = &mychick_move_death2;
		gi.sound (self, CHAN_VOICE, sound_death2, 1, ATTN_NORM, 0);
	}

	DroneList_Remove(self);
	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void mychick_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	if (!self->groundentity)
		return;

	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] = 0;
	self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void mychick_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	VectorClear(self->velocity);
	gi.linkentity (self);
}

mframe_t mychick_frames_duck [] =
{
	ai_move, 0, mychick_duck_down,
	ai_move, 0,  NULL,
	ai_move, 0,  mychick_duck_up,
	ai_move, 0, NULL,
	ai_move, 0,  NULL
};
mmove_t mychick_move_duck = {FRAME_duck03, FRAME_duck07, mychick_frames_duck, mychick_run};

void mychick_jump_takeoff (edict_t *self)
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

void mychick_jump_hold (edict_t *self)
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

mframe_t mychick_frames_leap [] =
{
	ai_move, 0, mychick_jump_takeoff,
	ai_move, 0, NULL,
	ai_move, 0, mychick_jump_hold,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0, NULL,
	ai_move, 0,  NULL
};
mmove_t mychick_move_leap = {FRAME_duck01, FRAME_duck07, mychick_frames_leap, mychick_run};

void mychick_leap (edict_t *self)
{
	if (self->groundentity)
		self->monsterinfo.currentmove = &mychick_move_leap;
}

void mychick_dodge (edict_t *self, edict_t *attacker, vec3_t dir, int radius)
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
		self->monsterinfo.currentmove = &mychick_move_duck;
		self->monsterinfo.dodge_time = level.time + 2.0;
	}
	else
	{
		mychick_leap(self);
		self->monsterinfo.dodge_time = level.time + 3.0;
	}
}

void fire_meteor (edict_t *self, vec3_t end, int damage, int radius, int speed);

void myChickMeteor (edict_t *self)
{
	int damage, speed;

	if (!G_EntExists(self->enemy))
		return;

	//slvl = self->monsterinfo.level;

	//if (slvl > 15)
	//	slvl = 15;

	damage = 500;//200 + 30 * slvl;
	speed = 1000;//650 + 35 * slvl;

	fire_meteor(self, self->enemy->s.origin, damage, 200, speed);

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void myChickSlash (edict_t *self)
{
	vec3_t	aim;
	
	if (self->monsterinfo.bonus_flags & BF_UNIQUE_FIRE)
	{
		myChickMeteor(self);
		return;
	}

	if (!G_EntExists(self->enemy))
		return;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 10);
	gi.sound (self, CHAN_WEAPON, sound_melee_swing, 1, ATTN_NORM, 0);
	fire_hit (self, aim, (10 + (rand() %6)), 100);
}

void fire_fireball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float damage_radius, int speed, int flames, int flame_damage);

void myChickFireball (edict_t *self)
{
	int slvl, damage, speed, flame_damage;
	vec3_t	forward, start;

	if (!G_EntExists(self->enemy))
		return;

	slvl = self->monsterinfo.level;

	if (slvl > 15)
		slvl = 15;

	damage = 50 + 15 * slvl;
	flame_damage = 2 * slvl;
	speed = 650 + 35 * slvl;

	MonsterAim(self, 0.9, speed, true, MZ2_CHICK_ROCKET_1, forward, start);
	fire_fireball(self, start, forward, damage, 125.0, speed, 5, flame_damage);

	gi.sound (self, CHAN_ITEM, gi.soundindex("spells/firecast.wav"), 1, ATTN_NORM, 0);
}

void myChickRocket (edict_t *self)
{
	int		damage, speed;
	vec3_t	forward, start;

	if (self->monsterinfo.bonus_flags & BF_UNIQUE_FIRE)
	{
		myChickFireball(self);
		return;
	}

	if (!G_EntExists(self->enemy))
		return;

	damage = 50 + 15*self->monsterinfo.level;
	if ( self->activator && self->activator->client )
	{
		speed = 200 + 30*self->monsterinfo.level;
	}
	else
	{
		speed = 450;	
	}

	MonsterAim(self, 1, speed, true, MZ2_CHICK_ROCKET_1, forward, start);
	monster_fire_rocket (self, start, forward, damage, speed, MZ2_CHICK_ROCKET_1);
}

void myChickRail (edict_t *self)
{
	int		damage;
	vec3_t	forward, start;

	//if (!self->activator->client)
	//	damage = 50 + 20*self->monsterinfo.level;
	//else
		damage = 50 + 10*self->monsterinfo.level;

	MonsterAim(self, 0.33, 0, false, MZ2_CHICK_ROCKET_1, forward, start);
	monster_fire_railgun (self, start, forward, damage, damage, MZ2_GLADIATOR_RAILGUN_1);
}	

void mychick_PreAttack1 (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_missile_prelaunch, 1, ATTN_NORM, 0);
}

void myChickReload (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_missile_reload, 1, ATTN_NORM, 0);
}

mframe_t mychick_frames_start_attack1 [] =
{
	ai_charge, 0,	mychick_PreAttack1,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 4,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -3,  NULL,
	ai_charge, 3,	NULL,
	ai_charge, 5,	NULL,
	ai_charge, 7,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	mychick_attack1
};
mmove_t mychick_move_start_attack1 = {FRAME_attak101, FRAME_attak113, mychick_frames_start_attack1, NULL};


mframe_t mychick_frames_attack1 [] =
{
	ai_charge, 0,	myChickRocket,//myChickRail,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	myChickReload,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	mychick_rerocket

};
mmove_t mychick_move_attack1 = {FRAME_attak114, FRAME_attak121, mychick_frames_attack1, mychick_rerocket};

mframe_t mychick_frames_end_attack1 [] =
{
	ai_charge, -3,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -6,	NULL,
	ai_charge, -4,	NULL,
	ai_charge, -2,  NULL
};
mmove_t mychick_move_end_attack1 = {FRAME_attak128, FRAME_attak132, mychick_frames_end_attack1, mychick_run};

void mychick_rerocket(edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true)) 
	{
		if (random() <= 0.8 && (entdist(self, self->enemy) <= 512 || (self->monsterinfo.aiflags & AI_STAND_GROUND)))
			self->monsterinfo.currentmove = &mychick_move_attack1;
		else
			self->monsterinfo.currentmove = &mychick_move_end_attack1;
	}
	else
		self->monsterinfo.currentmove = &mychick_move_end_attack1;

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 0.5;
}

//GHz START
mframe_t mychick_frames_runandshoot [] =
{
	drone_ai_run, 20,	myChickRocket,
	drone_ai_run, 20,	NULL,
	drone_ai_run, 20,	NULL,
	drone_ai_run, 20,	NULL,
	drone_ai_run, 20,	NULL,
	drone_ai_run, 20,	NULL,
	drone_ai_run, 20,	NULL
};
mmove_t mychick_move_runandshoot = {FRAME_attak120, FRAME_attak126, mychick_frames_runandshoot, mychick_continue};

void mychick_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &mychick_move_runandshoot;
}

void mychick_continue (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.9) 
		&& (entdist(self, self->enemy) <= 512)) 
	{
		self->monsterinfo.currentmove = &mychick_move_runandshoot;
		
		// don't call the attack function again for awhile!
		self->monsterinfo.attack_finished = level.time + 1;
		return;
	}

	self->monsterinfo.currentmove = &mychick_move_run;
}
//GHz END

void mychick_attack1(edict_t *self)
{
	self->monsterinfo.currentmove = &mychick_move_attack1;
}

mframe_t mychick_frames_slash [] =
{
	ai_charge, 1,	NULL,
	ai_charge, 7,	myChickSlash,
	ai_charge, -7,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, -1,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, -2,	mychick_reslash
};
mmove_t mychick_move_slash = {FRAME_attak204, FRAME_attak212, mychick_frames_slash, NULL};

mframe_t mychick_frames_end_slash [] =
{
	ai_charge, -6,	NULL,
	ai_charge, -1,	NULL,
	ai_charge, -6,	NULL,
	ai_charge, 0,	NULL
};
mmove_t mychick_move_end_slash = {FRAME_attak213, FRAME_attak216, mychick_frames_end_slash, mychick_run};


void mychick_reslash(edict_t *self)
{
	if (self->enemy->health > 0)
	{
		if (entdist (self, self->enemy) == 32)
			if (random() <= 0.9)
			{				
				self->monsterinfo.currentmove = &mychick_move_slash;
				return;
			}
			else
			{
				self->monsterinfo.currentmove = &mychick_move_end_slash;
				return;
			}
	}
	self->monsterinfo.currentmove = &mychick_move_end_slash;
}

void mychick_slash(edict_t *self)
{
	self->monsterinfo.currentmove = &mychick_move_slash;
}


mframe_t mychick_frames_start_slash [] =
{	
	ai_charge, 1,	NULL,
	ai_charge, 8,	NULL,
	ai_charge, 3,	NULL
};
mmove_t mychick_move_start_slash = {FRAME_attak201, FRAME_attak203, mychick_frames_start_slash, mychick_slash};



void mychick_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &mychick_move_start_slash;
}

void chick_fire_attack (edict_t *self)
{
	float r = random();
	float range = entdist(self, self->enemy);

	// medium-long range
	if (range <= 768)
	{
		// 30% chance to cast meteor, 70% chance to shoot fireballs
		if (r <= 0.3)
		{
			self->monsterinfo.currentmove = &mychick_move_slash;
		}
		else
		{
			if (self->monsterinfo.aiflags & AI_STAND_GROUND)
				self->monsterinfo.currentmove = &mychick_move_attack1;
			else
				mychick_runandshoot(self);
		}
	}
	// long range
	else
		self->monsterinfo.currentmove = &mychick_move_slash;// always cast meteor

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 1;
}

void mychick_attack(edict_t *self)
{
	if (self->monsterinfo.bonus_flags & BF_UNIQUE_FIRE)
	{
		chick_fire_attack(self);
		return;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &mychick_move_attack1;
	else
		mychick_runandshoot(self);

	// don't call the attack function again for awhile!
	self->monsterinfo.attack_finished = level.time + 1;
}

void mychick_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

mframe_t mychick_frames_jump [] =
{
	drone_ai_run, 0,	NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0, NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0,  NULL,
	drone_ai_run, 0,  NULL

};
mmove_t mychick_move_jump = {FRAME_walk11, FRAME_walk20, mychick_frames_jump, NULL};

/*QUAKED monster_chick (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void init_drone_bitch (edict_t *self)
{
	sound_missile_prelaunch	= gi.soundindex ("chick/chkatck1.wav");	
	sound_missile_launch	= gi.soundindex ("chick/chkatck2.wav");	
	sound_melee_swing		= gi.soundindex ("chick/chkatck3.wav");	
	sound_melee_hit			= gi.soundindex ("chick/chkatck4.wav");	
	sound_missile_reload	= gi.soundindex ("chick/chkatck5.wav");	
	sound_death1			= gi.soundindex ("chick/chkdeth1.wav");	
	sound_death2			= gi.soundindex ("chick/chkdeth2.wav");	
	sound_fall_down			= gi.soundindex ("chick/chkfall1.wav");	
	sound_idle1				= gi.soundindex ("chick/chkidle1.wav");	
	sound_idle2				= gi.soundindex ("chick/chkidle2.wav");	
	sound_pain1				= gi.soundindex ("chick/chkpain1.wav");	
	sound_pain2				= gi.soundindex ("chick/chkpain2.wav");	
	sound_pain3				= gi.soundindex ("chick/chkpain3.wav");	
	sound_sight				= gi.soundindex ("chick/chksght1.wav");	
	sound_search			= gi.soundindex ("chick/chksrch1.wav");	

	self->mtype = M_CHICK;
	self->monsterinfo.control_cost = M_CHICK_CONTROL_COST;
	self->monsterinfo.cost = M_CHICK_COST;

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 56);
	
	if (self->activator && self->activator->client)
		self->health = 120 + 75*self->monsterinfo.level;
	else
		self->health = 80 + 55*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 200;

	if (random() > 0.5)
		self->item = FindItemByClassname("ammo_rails");
	else
		self->item = FindItemByClassname("ammo_rockets");
	
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;

	//if (self->activator && self->activator->client)
		self->monsterinfo.power_armor_power = 60 + 50*self->monsterinfo.level;
	//else self->monsterinfo.power_armor_power = 20*self->monsterinfo.level;

	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;

//	self->pain = mychick_pain;
	self->die = mychick_die;

	self->monsterinfo.stand = mychick_stand;
	self->monsterinfo.walk = chick_walk;
	self->monsterinfo.run = mychick_run;
//	self->monsterinfo.jump = mychick_jump;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.dodge = mychick_dodge;
	self->monsterinfo.attack = mychick_attack;
	//self->monsterinfo.melee = mychick_melee;
	self->monsterinfo.sight = mychick_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &mychick_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start (self);
	self->nextthink = level.time + FRAMETIME;
}
