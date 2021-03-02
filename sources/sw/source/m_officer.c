// n_officer2.c

#include "g_local.h"
#include "m_officer.h"

void find_near_mark (edict_t *self, float radius);
void officer2_walk (edict_t *self);
void officer2_stand (edict_t *self);

mframe_t officer2_frames_stand [] =
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

	ai_stand, 0, NULL
};
mmove_t officer2_move_stand = {FRAME_stand01, FRAME_stand11, officer2_frames_stand, NULL};

#define WS		3

mframe_t officer2_frames_walk [] =
{
	ai_walk, WS,  NULL,
	ai_walk, WS, NULL,
	ai_walk, WS, NULL,
	ai_walk, WS,  NULL,
	ai_walk, WS, NULL,
	ai_walk, WS, NULL,
	ai_walk, WS,  NULL,
	ai_walk, WS, NULL,
	ai_walk, WS, NULL,
	ai_walk, WS,  NULL,
	ai_walk, WS,  NULL,
	ai_walk, WS, NULL
};
mmove_t officer2_move_walk = {FRAME_walk211, FRAME_walk221, officer2_frames_walk, NULL};

mframe_t officer2_frames_start_walk [] =
{
	ai_walk, .5,  NULL,
	ai_walk, 1, NULL,
	ai_walk, 1.5, NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2.5, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL
};
mmove_t officer2_move_start_walk = {FRAME_walk101, FRAME_walk110, officer2_frames_start_walk, officer2_walk};

mframe_t officer2_frames_end_walk [] =
{
	ai_stand, 3,  NULL,
	ai_stand, 2.5, NULL,
	ai_stand, 2, NULL,
	ai_stand, 1.5,  NULL,
	ai_stand, 1, NULL,
	ai_stand, .5, NULL
};
mmove_t officer2_move_end_walk = {FRAME_walk322, FRAME_walk327, officer2_frames_end_walk, officer2_stand};

void officer2_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &officer2_move_stand ||
		self->monsterinfo.currentmove == &officer2_move_end_walk)
	{
		self->monsterinfo.currentmove = &officer2_move_stand;
	}
	else
	{
		self->monsterinfo.currentmove = &officer2_move_stand;
	}
}

void officer2_walk (edict_t *self)
{
	if (self->monsterinfo.currentmove == &officer2_move_walk ||
		self->monsterinfo.currentmove == &officer2_move_start_walk)
	{
		self->monsterinfo.currentmove = &officer2_move_walk;
	}
	else
	{
		self->monsterinfo.currentmove = &officer2_move_start_walk;
	}
}

mframe_t officer2_frames_run [] =
{
	ai_run, 4,  NULL,
	ai_run, 15, NULL,
	ai_run, 15, NULL,
	ai_run, 8,  NULL,
	ai_run, 20, NULL,
	ai_run, 15, NULL,
	ai_run, 8,  NULL,
	ai_run, 20, NULL
};
mmove_t officer2_move_run = {FRAME_run01, FRAME_run08, officer2_frames_run, NULL};

void officer2_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		officer2_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &officer2_move_run;
}

void officer2_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}

void officer2_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t officer2_frames_death1 [] =
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
mmove_t officer2_move_death1 = {FRAME_death01, FRAME_death16, officer2_frames_death1, officer2_dead};

void officer2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->s.modelindex2 = 0;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &officer2_move_death1;
}

//
//ATTACK
//
void officer2_fire (edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int damage;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

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
		damage = 10;
	}
	else
	{
		damage = 25;
	}

	monster_fire_blaster (self, start, aim, damage, BLASTER_SPEED, flash_index, EF_BLASTER);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/pistol/fire.wav"), 1, ATTN_NORM, 0);

}

void officer2_fire1 (edict_t *self)
{
	officer2_fire (self, 0);
}

mframe_t officer2_frames_attack [] =
{
	ai_charge, 0, officer2_fire1,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t officer2_move_attack = {FRAME_attack01, FRAME_attack04, officer2_frames_attack, officer2_run};

void officer2_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &officer2_move_attack;
}

//SIGHT

mframe_t officer2_frames_sight [] =
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
	ai_charge, 0, NULL
};
mmove_t officer2_move_sight = {FRAME_snap01, FRAME_snap13, officer2_frames_attack, officer2_run};

void officer2_sight (edict_t *self, edict_t *other)
{
	if(self->monsterinfo.currentmove == &officer2_move_walk ||
		self->monsterinfo.currentmove == &officer2_move_stand)
	{
		self->monsterinfo.currentmove = &officer2_move_sight;
	}
}

/*QUAKED monster_officer (1 .5 0) (-16 -16 -24) (16 16 32)
*/
void SP_monster_officer (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/officer/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/officer/pistol.md2");
	VectorSet (self->mins, -12, -12, -24);
	VectorSet (self->maxs, 12, 12, 32);

	if (!self->health)
		self->health = 30;
	self->mass = 200;
	self->yaw_speed = 15;

	self->pain = officer2_pain;
	self->die = officer2_die;

	self->monsterinfo.stand = officer2_stand;
	self->monsterinfo.walk = officer2_walk;
	self->monsterinfo.run = officer2_run;
	self->monsterinfo.attack = officer2_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = officer2_sight;

	self->path_node = NO_NODES;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &officer2_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}
