
/*
==============================================================================

Q1 ZOMBIE

==============================================================================
*/


#include "g_local.h"
#include "m_genq1zombie.h"


static int  sound_idleC;
static int	sound_idle;
static int	sound_idle2;
static int	sound_pain;
static int	sound_pain2;
static int  sound_fall;
static int  sound_gib;
static int  sound_shot;

void q1_fire_gib (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);

void q1zombie_Cidle (edict_t *self)
{
	if (random() > 0.1)
		gi.sound (self, CHAN_VOICE, sound_idleC, 1, ATTN_STATIC, 0);
}

void q1zombie_idle (edict_t *self)
{
	if (random() > 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_STATIC, 0);

	if(self->solid == SOLID_NOT)
		self->solid = SOLID_BBOX;
}

void q1zombie_idle2 (edict_t *self)
{
	if (random() > 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_STATIC, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_STATIC, 0);
}

void q1zombie_pain1(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_IDLE, 0);
	self->health = 60;
}

void q1zombie_pain2(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_IDLE, 0);
	self->health = 60;
}

void q1zombie_fall(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_fall, 1, ATTN_IDLE, 0);
	self->health = 60;
}


// STAND

void q1zombie_stand (edict_t *self);

mframe_t q1zombie_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t q1zombie_move_stand = {FRAME_stand1, FRAME_stand15, q1zombie_frames_stand, q1zombie_stand};

void q1zombie_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1zombie_move_stand;
}


//Crucified
void q1zombie_crucify (edict_t *self);

mframe_t q1zombie_frames_cruc [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, q1zombie_Cidle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
};
mmove_t q1zombie_crucified = {FRAME_cruc_1, FRAME_cruc_6, q1zombie_frames_cruc, q1zombie_crucify};

void q1zombie_crucify (edict_t *self)
{
	self->monsterinfo.currentmove = &q1zombie_crucified;
}

//
// WALK
//

mframe_t q1zombie_frames_walk [] =
{
	ai_walk, 0,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  q1zombie_idle
};
mmove_t q1zombie_move_walk = {FRAME_walk1, FRAME_walk19, q1zombie_frames_walk, NULL};

void q1zombie_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1zombie_move_walk;
}


//
// RUN
//

void q1zombie_run (edict_t *self);

mframe_t q1zombie_frames_run [] =
{
	ai_run, 1, NULL,
	ai_run, 1, NULL,
	ai_run, 0, NULL,
	ai_run, 1, NULL,
	ai_run, 2,  NULL,
	ai_run, 3, NULL,
	ai_run, 4, NULL,
	ai_run, 4, NULL,
	ai_run, 2, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 2, NULL,
	ai_run, 4, NULL,
	ai_run, 6, NULL,
	ai_run, 7, NULL,
	ai_run, 3, NULL,
	ai_run, 8, q1zombie_idle2
};
mmove_t q1zombie_move_run = {FRAME_run1, FRAME_run18, q1zombie_frames_run, q1zombie_run};

void q1zombie_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &q1zombie_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &q1zombie_move_run;
}


//
// ATTACK
//
void q1zombie_firegib (edict_t *self, vec3_t offset)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	
	switch(range (self,self->enemy))
	{
	case RANGE_MELEE:
		target[2] += self->enemy->viewheight;
		break;
	case RANGE_NEAR:
		VectorMA (target, -0.04, self->enemy->velocity, target);
		target[2] += (self->enemy->viewheight * 0.9);
		break;
	case RANGE_MID:
		VectorMA (target, -0.08, self->enemy->velocity, target);
		target[2] += (self->enemy->viewheight * 0.8);
		break;
	default:
		VectorMA (target,-0.1, self->enemy->velocity, target);
		target[2] += (rand() & self->enemy->viewheight);
		break;
	}

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_shot, 1.0, ATTN_NORM, 0);
	q1_fire_gib (self, start, aim, 10, 600);

}

void q1zombie_firegib1(edict_t *self)
{
	vec3_t offset;
	offset[0]=-8;
	offset[1]=-18;
	offset[2]=30;

	q1zombie_firegib(self,offset);
}

void q1zombie_firegib2(edict_t *self)
{
	vec3_t offset;
	offset[0]=-8;
	offset[1]=-18;
	offset[2]=30;
	q1zombie_firegib(self,offset);
}

void q1zombie_firegib3(edict_t *self)
{
	vec3_t offset;
	offset[0]=-8;
	offset[1]=22;
	offset[2]=30;
	q1zombie_firegib(self,offset);
}

mframe_t q1zombie_frames_attack1[] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1zombie_firegib1
};
mmove_t q1zombie_move_attack1 = {FRAME_atta1, FRAME_atta13, q1zombie_frames_attack1, q1zombie_run};

mframe_t q1zombie_frames_attack2[] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1zombie_firegib2
};
mmove_t q1zombie_move_attack2 = {FRAME_attb1, FRAME_attb14, q1zombie_frames_attack2, q1zombie_run};


mframe_t q1zombie_frames_attack3[] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1zombie_firegib3
};
mmove_t q1zombie_move_attack3 = {FRAME_attc1, FRAME_attc12, q1zombie_frames_attack3, q1zombie_run};


void q1zombie_attack(edict_t *self)
{
	float r=random();

	if(self->style)
		return;

	if(r < 0.3)
		self->monsterinfo.currentmove = &q1zombie_move_attack1;
	else if(r < 0.6)
		self->monsterinfo.currentmove = &q1zombie_move_attack2;
	else
		self->monsterinfo.currentmove = &q1zombie_move_attack3;
}


//
// PAIN
//

mframe_t q1zombie_frames_pain1 [] =
{
	ai_move, 0, q1zombie_pain1,
	ai_move, 3, NULL,
	ai_move, 1, NULL,
	ai_move, -1,NULL,
	ai_move, -3,NULL,
	ai_move, -1,NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t q1zombie_move_pain1 = {FRAME_paina1, FRAME_paina12, q1zombie_frames_pain1, q1zombie_run};

mframe_t q1zombie_frames_pain2 [] =
{
	ai_move, 0, q1zombie_pain2,
	ai_move, -2, NULL,
	ai_move, -8, NULL,
	ai_move, -6,NULL,
	ai_move, -2,NULL,
	ai_move, 0,NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, q1zombie_fall,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t q1zombie_move_pain2 = {FRAME_painb1, FRAME_painb28, q1zombie_frames_pain2, q1zombie_run};

mframe_t q1zombie_frames_pain3 [] =
{
	ai_move, 0, q1zombie_pain2,
	ai_move, 0, NULL,
	ai_move, -3,NULL,
	ai_move, -1,NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 1, NULL,
	ai_move, 1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t q1zombie_move_pain3 = {FRAME_painc1, FRAME_painc18, q1zombie_frames_pain3, q1zombie_run};

mframe_t q1zombie_frames_pain4 [] =
{
	ai_move, 0, q1zombie_pain1,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,NULL,
	ai_move, 0,NULL,
	ai_move, 0,NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t q1zombie_move_pain4 = {FRAME_paind1, FRAME_paind13, q1zombie_frames_pain4, q1zombie_run};

// fall and stand up functions

void q1zombie_down( edict_t *self)
{
	q1zombie_fall(self);
	self->nextthink += 5;
}

void q1zombie_fallagain ( edict_t *self);

mframe_t q1zombie_frames_pain5 [] =
{
	ai_move, 0, q1zombie_pain2,
	ai_move, -8, NULL,
	ai_move, -5, NULL,
	ai_move, -3,NULL,
	ai_move, -1,NULL,
	ai_move, -2,NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, 0, q1zombie_down,
	ai_move, 0, NULL,
	ai_move, 0, q1zombie_idle,
	ai_move, 0, q1zombie_fallagain,
	ai_move, 0, NULL,  
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 5, NULL,
	ai_move, 3, NULL,
	ai_move, 1, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t q1zombie_move_pain5 = {FRAME_paine1, FRAME_paine30, q1zombie_frames_pain5, q1zombie_run};

void q1zombie_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;
	
	self->health = 60;
	if(self->wait > level.time)
	{
		self->dmg += damage;
		if(self->dmg > 30)
		{
			self->monsterinfo.currentmove = &q1zombie_move_pain5;
			self->pain_debounce_time = level.time + 3;
			self->dmg = 0;
			self->wait = 0;
			return;
		}
	}
	else
	{
		self->wait = level.time + 1;
		self->dmg = damage;
	}

	if(damage < 15)
		return;

	if(damage >= 25)
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain5;
		self->pain_debounce_time = level.time + 3;
		return;
	}

	if(self->pain_debounce_time > level.time)
	return;
	
	r = random();

	if (r < 0.25)
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain1;
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + 1.0;
	}
	else if (r < 0.5)
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain2;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + 1.5;
	}
	else if (r < 0.75)
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain3;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + 1.1;
	}
	else
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain4;
		self->pain_debounce_time = level.time + 1.0;
	}
}

void q1zombie_fallagain ( edict_t *self)
{
	if(self->pain_debounce_time > level.time)
	{
		self->monsterinfo.currentmove = &q1zombie_move_pain5;
	} 
}

//
// SIGHT
//

void q1zombie_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM, 0);
	
	if (!self->style && (skill->value > 0) && (range(self, self->enemy) >= RANGE_MID))
		q1zombie_attack(self);
}


//
// DEATH
//

void q1zombie_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
	ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/monsters/q1monstr/zombie/head/tris.md2", damage*2, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	return;
}


//
// SPAWN
//

void SP_q1_monster_zombie (edict_t *self)
{
	if (deathmatch->value && !self->style)
	{
		G_FreeEdict (self);
		return;
	}

	sound_idleC =	gi.soundindex ("q1monstr/zombie/idle_w2.wav");
	sound_idle =	gi.soundindex ("q1monstr/zombie/z_idle.wav");
	sound_idle2 =	gi.soundindex ("q1monstr/zombie/z_idle1.wav");
	sound_pain=		gi.soundindex ("q1monstr/zombie/z_pain.wav");
	sound_pain2=    gi.soundindex ("q1monstr/zombie/z_pain1.wav");
	sound_fall=    gi.soundindex ("q1monstr/zombie/z_fall.wav");
	sound_gib=    gi.soundindex ("q1monstr/zombie/z_gib.wav");
	sound_shot=    gi.soundindex ("q1monstr/zombie/z_shot1.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/zombie/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->mass = 100;
	self->health = 60;
	self->gib_health = 0;

	self->pain = q1zombie_pain;
	self->die = q1zombie_die;

	self->flags |= FL_Q1_MONSTER; 

	self->monsterinfo.stand = q1zombie_stand;
	self->monsterinfo.walk = q1zombie_walk;
	self->monsterinfo.run = q1zombie_run;
	self->monsterinfo.attack = q1zombie_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = q1zombie_sight;

	gi.linkentity (self);

	self->monsterinfo.scale = MODEL_SCALE;

	if(self->style)
	{
		self->movetype = MOVETYPE_NONE;
		self->monsterinfo.currentmove = &q1zombie_crucified;

	}
	else
	{
		self->monsterinfo.currentmove = &q1zombie_move_stand;
		walkmonster_start (self);
	}
}

