/*
==============================================================================

SOLDIER

==============================================================================
*/

#include "g_local.h"
#include "m_soldier.h"


static int	sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_pain_light;
static int	sound_pain;
static int	sound_pain_ss;
static int	sound_death_light;
static int	sound_death;
static int	sound_death_ss;
static int	sound_cock;

void m_soldier_stand (edict_t *self);
void m_soldier_runandshoot_continue (edict_t *self);

void m_soldier_idle (edict_t *self)
{
	if (random() > 0.8)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void m_soldier_cock (edict_t *self)
{
	if (self->s.frame == FRAME_stand322)
		gi.sound (self, CHAN_WEAPON, sound_cock, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_WEAPON, sound_cock, 1, ATTN_NORM, 0);
}

mframe_t m_soldier_frames_stand1 [] =
{
	drone_ai_stand, 0, m_soldier_idle,
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
mmove_t m_soldier_move_stand1 = {FRAME_stand101, FRAME_stand130, m_soldier_frames_stand1, m_soldier_stand};

mframe_t m_soldier_frames_stand3 [] =
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
	drone_ai_stand, 0, m_soldier_cock,
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
mmove_t m_soldier_move_stand3 = {FRAME_stand301, FRAME_stand339, m_soldier_frames_stand3, m_soldier_stand};

void m_soldier_stand (edict_t *self)
{
	if (random() > 0.5)
		self->monsterinfo.currentmove = &m_soldier_move_stand1;
	else
		self->monsterinfo.currentmove = &m_soldier_move_stand3;
}

mframe_t m_soldier_frames_run [] =
{
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL
};
mmove_t m_soldier_move_run = {FRAME_run03, FRAME_run08, m_soldier_frames_run, NULL};

void m_soldier_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		m_soldier_stand(self);
	else
		self->monsterinfo.currentmove = &m_soldier_move_run;
}

void soldier_fireblaster (edict_t *self)
{
	int		damage, speed;
	vec3_t	forward, start;
	
	if (!G_EntExists(self->enemy))
		return;

	damage = 50 + 10*self->monsterinfo.level;
	speed = 1000 + 50*self->monsterinfo.level;

	MonsterAim(self, 0.8, speed, false, MZ2_SOLDIER_BLASTER_8, forward, start);
	monster_fire_blaster(self, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BOLT, 2.0, true, MZ2_SOLDIER_BLASTER_8);
}

void soldier_firerocket (edict_t *self)
{
	int		damage, speed;
	vec3_t	forward, start;
	
	if (!G_EntExists(self->enemy))
		return;

	damage = 50 + 10*self->monsterinfo.level;
	speed = 650 + 30*self->monsterinfo.level;

	MonsterAim(self, 0.8, speed, true, MZ2_SOLDIER_BLASTER_8, forward, start);
	monster_fire_rocket (self, start, forward, damage, speed, MZ2_SOLDIER_BLASTER_8);
}

void soldier_fireshotgun (edict_t *self)
{
	int		damage;
	vec3_t	forward, start;

	if (!G_EntExists(self->enemy))
		return;

	damage = 5 + self->monsterinfo.level;
	MonsterAim(self, 0.8, 0, false, MZ2_SOLDIER_SHOTGUN_8, forward, start);
	monster_fire_shotgun(self, start, forward, damage, damage, 375, 375, 10, MZ2_SOLDIER_SHOTGUN_8);
}

void m_soldier_fire (edict_t *self)
{
	if (!G_EntExists(self->enemy))
		return;

	if (self->mtype == M_SOLDIER)
		soldier_fireblaster(self);
	else if (self->mtype == M_SOLDIERLT)
		soldier_firerocket(self);
	else if (self->mtype == M_SOLDIERSS)
		soldier_fireshotgun(self);
}

mframe_t m_soldier_frames_runandshoot [] =
{
	drone_ai_run, 25, NULL,	//109
	drone_ai_run,  25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, m_soldier_fire, //112
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, m_soldier_fire,	//117
	drone_ai_run,  25, NULL,
	drone_ai_run, 25, m_soldier_cock,	//119
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, NULL,
	drone_ai_run, 25, m_soldier_runandshoot_continue	//122
};
mmove_t m_soldier_move_runandshoot = {FRAME_runs01, FRAME_runs14, m_soldier_frames_runandshoot, NULL};

void m_soldier_runandshoot_continue (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.9) 
		&& (entdist(self, self->enemy) <= 512))
	{
		self->monsterinfo.currentmove = &m_soldier_move_runandshoot;
		self->monsterinfo.attack_finished = level.time + 2;
		return;
	}

	self->monsterinfo.currentmove = &m_soldier_move_run;
}

void m_soldier_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &m_soldier_move_runandshoot;
}

void m_soldier_attack1_refire1 (edict_t *self)
{
	// continue firing if the enemy is still close, or we are standing ground
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.9)
		&& ((entdist(self, self->enemy) <= 512) || (self->monsterinfo.aiflags & AI_STAND_GROUND)))
	{
		self->s.frame = FRAME_attak102;
		self->monsterinfo.attack_finished = level.time + 2;
	}
}

mframe_t m_soldier_frames_attack1 [] =
{
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  m_soldier_fire,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  m_soldier_attack1_refire1,
	ai_charge, 0,  NULL,
	ai_charge, 0,  m_soldier_cock,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t m_soldier_move_attack1 = {FRAME_attak101, FRAME_attak112, m_soldier_frames_attack1, m_soldier_run};

void m_soldier_attack (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &m_soldier_move_attack1;
		return;
	}

	if ((entdist(self, self->enemy) < 128) && (random() <= 0.8))
		self->monsterinfo.currentmove = &m_soldier_move_attack1;
	else
		self->monsterinfo.currentmove = &m_soldier_move_runandshoot;

	self->monsterinfo.attack_finished = level.time + 2;
}

void m_soldier_duck_down (edict_t *self)
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

void m_soldier_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	VectorClear(self->velocity);
	gi.linkentity (self);
}

mframe_t m_soldier_frames_duck [] =
{
	ai_move, 0, m_soldier_duck_down,
	ai_move, 0,	NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  m_soldier_duck_up
};
mmove_t m_soldier_move_duck = {FRAME_duck01, FRAME_duck04, m_soldier_frames_duck, m_soldier_run};

void m_soldier_jump_takeoff (edict_t *self)
{
	vec3_t	v;

	gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	VectorSubtract(self->monsterinfo.dir, self->s.origin, v);
	v[2] = 0;
	VectorNormalize(v);
	VectorScale(v, -200, self->velocity);
	self->velocity[2] = 400;
	self->monsterinfo.pausetime = level.time + 2.0; // maximum duration of jump
}

void m_soldier_jump_hold (edict_t *self)
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

mframe_t m_soldier_frames_jump [] =
{
	ai_move, 0, m_soldier_jump_takeoff,
	ai_move, 0,	m_soldier_jump_hold,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t m_soldier_move_jump = {FRAME_duck01, FRAME_duck05, m_soldier_frames_jump, m_soldier_run};

void m_soldier_jump (edict_t *self)
{
	if (self->groundentity)
		self->monsterinfo.currentmove = &m_soldier_move_jump;
}

void m_soldier_dodge (edict_t *self, edict_t *attacker, vec3_t dir, int radius)
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
		self->monsterinfo.currentmove = &m_soldier_move_duck;
		self->monsterinfo.dodge_time = level.time + 2.0;
	}
	else
	{
		m_soldier_jump(self);
		self->monsterinfo.dodge_time = level.time + 3.0;
	}
}

void m_soldier_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t m_soldier_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   m_soldier_fire,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   m_soldier_fire,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t m_soldier_move_death1 = {FRAME_death101, FRAME_death136, m_soldier_frames_death1, m_soldier_dead};

mframe_t m_soldier_frames_death2 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t m_soldier_move_death2 = {FRAME_death201, FRAME_death235, m_soldier_frames_death2, m_soldier_dead};

mframe_t m_soldier_frames_death3 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
};
mmove_t m_soldier_move_death3 = {FRAME_death301, FRAME_death345, m_soldier_frames_death3, m_soldier_dead};

mframe_t m_soldier_frames_death4 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t m_soldier_move_death4 = {FRAME_death401, FRAME_death453, m_soldier_frames_death4, m_soldier_dead};

mframe_t m_soldier_frames_death5 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t m_soldier_move_death5 = {FRAME_death501, FRAME_death524, m_soldier_frames_death5, m_soldier_dead};

mframe_t m_soldier_frames_death6 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t m_soldier_move_death6 = {FRAME_death601, FRAME_death610, m_soldier_frames_death6, m_soldier_dead};

void m_soldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int n;

	// notify the owner that the monster is dead
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

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->takedamage = DAMAGE_YES;
	self->deadflag = DEAD_DEAD;

	n = GetRandom(1, 6);
	switch (n)
	{
	case 1: self->monsterinfo.currentmove = &m_soldier_move_death1; break;
	case 2: self->monsterinfo.currentmove = &m_soldier_move_death2; break;
	case 3: self->monsterinfo.currentmove = &m_soldier_move_death3; break;
	case 4: self->monsterinfo.currentmove = &m_soldier_move_death4; break;
	case 5: self->monsterinfo.currentmove = &m_soldier_move_death5; break;
	case 6: self->monsterinfo.currentmove = &m_soldier_move_death6; break;
	}

	DroneList_Remove(self);

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void init_drone_soldier (edict_t *self)
{
	// NOTE: monster's pain, think, and touch functions are set-up elsewhere

	self->s.modelindex = gi.modelindex ("models/monsters/soldier/tris.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	sound_idle =	gi.soundindex ("soldier/solidle1.wav");
	sound_sight1 =	gi.soundindex ("soldier/solsght1.wav");
	sound_sight2 =	gi.soundindex ("soldier/solsrch1.wav");
	sound_cock =	gi.soundindex ("infantry/infatck3.wav");

	self->mass = 100;

	//4.2 don't override previous mtype
	if (!self->mtype)
		self->mtype = M_SOLDIER;

	self->monsterinfo.control_cost = 33;
	self->monsterinfo.cost = 50;

	// set health
	self->health = M_SOLDIER_INITIAL_HEALTH+M_SOLDIER_ADDON_HEALTH*self->monsterinfo.level;
	self->max_health = self->health;
	self->gib_health = -150;

	// set armor
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = M_SOLDIER_INITIAL_ARMOR+M_SOLDIER_ADDON_ARMOR*self->monsterinfo.level;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;

	// set AI jump parameters
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.jumpup = 64;

	self->die = m_soldier_die;

	self->monsterinfo.stand = m_soldier_stand;
	self->monsterinfo.run = m_soldier_run;
	self->monsterinfo.dodge = m_soldier_dodge;
	self->monsterinfo.attack = m_soldier_attack;

	gi.linkentity (self);
	self->nextthink = level.time + FRAMETIME;
	self->monsterinfo.currentmove = &m_soldier_move_stand1;
}