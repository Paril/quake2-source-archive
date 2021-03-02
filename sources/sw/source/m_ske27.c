// m_ske27.c

#include "g_local.h"
#include "m_ske27.h"

mframe_t ske27_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t ske27_move_stand = {FRAME_stand01, FRAME_stand01, ske27_frames_stand, NULL};

#define WS		3

mframe_t ske27_frames_walk [] =
{
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL,
	ai_walk, WS,	NULL
};
mmove_t ske27_move_walk = {FRAME_walk01, FRAME_walk12, ske27_frames_walk, NULL};

mframe_t ske27_frames_run_limp [] =
{
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL
};
mmove_t ske27_move_run_limp = {FRAME_limp01, FRAME_limp14, ske27_frames_run_limp, NULL};

mframe_t ske27_frames_run [] =
{
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL,
	ai_run, WS,	NULL
};
mmove_t ske27_move_run = {FRAME_walk01, FRAME_walk12, ske27_frames_run, NULL};

void ske27_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &ske27_move_stand;
}

void ske27_walk (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		ske27_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &ske27_move_walk;
}

void ske27_run (edict_t *self)
{
	if(self->health <= (self->max_health/2))
		self->monsterinfo.currentmove = &ske27_move_run_limp;
	else
		self->monsterinfo.currentmove = &ske27_move_run_limp;
}

//**************
//pain
//**************

mframe_t ske27_frames_painhigh[] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t ske27_move_painhigh = {FRAME_hithigh01, FRAME_hithigh06, ske27_frames_painhigh, ske27_run};

mframe_t ske27_frames_painlow [] =
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
	ai_move, 0,   NULL
};
mmove_t ske27_move_painlow = {FRAME_hitlow01, FRAME_hitlow07, ske27_frames_painlow, ske27_run};

void ske27_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if(random() > 0.5)
		self->monsterinfo.currentmove = &ske27_move_painlow;
	else
		self->monsterinfo.currentmove = &ske27_move_painhigh;
}


//**************
//Death
//**************

void ske27_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t ske27_frames_deatha [] =
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
	ai_move, 0,   NULL
};
mmove_t ske27_move_deatha = {FRAME_fallforward01, FRAME_fallforward12, ske27_frames_deatha, ske27_dead};

mframe_t ske27_frames_deathb [] =
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
	ai_move, 0,   NULL
};
mmove_t ske27_move_deathb = {FRAME_fallapart01, FRAME_fallapart16, ske27_frames_deathb, ske27_dead};

void ske27_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float n;

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->s.modelindex2 = 0;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = random();

	if(n < 0.5)
	{
		self->monsterinfo.currentmove = &ske27_move_deatha;
	}
	else
	{
		self->monsterinfo.currentmove = &ske27_move_deathb;
	}
}

//
//ATTACK
//
void ske27_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t	offset;
	float	r, u;
	int damage;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	offset[0] = 0.0f;
	offset[1] = 18.0f;
	offset[2] = 16.0f;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	if (skill->value > 0 && self->s.skinnum < 2)
	{
		VectorCopy (self->enemy->s.origin, end);
		predictTargPos (self, start, end, self->enemy->velocity, 2048, false);
	}
	else
		VectorCopy (self->enemy->s.origin, end);

	if (!skill->value)
		VectorMA (end, -0.25, self->enemy->velocity, end);
	else if (skill->value == 1)
		VectorMA (end, -0.2, self->enemy->velocity, end);
	else if (skill->value == 2)
		VectorMA (end, -0.1, self->enemy->velocity, end);
	else if (skill->value < 3)
	{
		if (random() < 0.5)
			VectorMA (end, -0.06, self->enemy->velocity, end);
		else
			VectorMA (end, 0.06, self->enemy->velocity, end);
	}

	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	if (skill->value < 3 && (random() > 0.5))
	{
		r = crandom () * (800/(skill->value + 1));
		u = crandom () * (300/(skill->value + 1));
	}
	else
	{
		r = 0;
		u = 0;
	}

	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	if(skill->value < 3)
	{
		damage = 20;
	}
	else
	{
		damage = 50;
	}

	monster_fire_blaster (self, start, aim, damage, BLASTER_SPEED, flash_index, EF_BLASTER);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/pistol/fire.wav"), 1, ATTN_NORM, 0);
}

void ske27_attack_refire (edict_t *self)
{
	trace_t tr;

	if (self->enemy->health <= 0)
		return;

	if(random() >= 0.5)
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->enemy->s.origin, self, MASK_MONSTERSOLID);

	if(skill->value == 3)
	{
		ske27_fire(self);
		self->monsterinfo.nextframe = FRAME_gunfire08;
	}
	else if (skill->value >= 1 && (tr.ent == self->enemy))
	{
		self->monsterinfo.nextframe = FRAME_gunfire03;
	}
	else
	{
		ske27_fire(self);
		self->monsterinfo.nextframe = FRAME_gunfire08;
	}
}

mframe_t ske27_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ske27_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, ske27_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, ske27_attack_refire
};
mmove_t ske27_move_attack = {FRAME_gunfire01, FRAME_gunfire08, ske27_frames_attack, ske27_run};

void ske27_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &ske27_move_attack;
}

//SIGHT
mframe_t ske27_frames_alert [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,

	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t ske27_move_alert = {FRAME_alert01, FRAME_alert20, ske27_frames_alert, ske27_run};

void ske27_alert (edict_t *self)
{
	self->monsterinfo.currentmove = &ske27_move_alert;
}

mframe_t ske27_frames_halt [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t ske27_move_halt = {FRAME_halt01, FRAME_halt10, ske27_frames_halt, ske27_run};

void ske27_sight (edict_t *self, edict_t *other)
{
	if(random() > 0.5)
		self->monsterinfo.currentmove = &ske27_move_halt;
	else
		self->monsterinfo.currentmove = &ske27_move_alert;
}

/*QUAKED monster_ske27 (1 .5 0) (-16 -16 -24) (16 16 32)
*/
void SP_monster_ske27 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->monsterinfo.scale = MODEL_SCALE;
	self->s.modelindex = gi.modelindex ("models/monsters/ske27droid/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 44);

	if (!self->health)
		self->health = 250;
	self->max_health = self->health;
	self->mass = 200;
	self->yaw_speed = 30;

	self->pain = ske27_pain;
	self->die = ske27_die;

	self->monsterinfo.stand = ske27_stand;
	self->monsterinfo.walk = ske27_walk;
	self->monsterinfo.run = ske27_run;
	self->monsterinfo.attack = ske27_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = ske27_sight;

	self->path_node = NO_NODES;

	gi.linkentity (self);

	self->monsterinfo.stand (self);
	walkmonster_start (self);
}
