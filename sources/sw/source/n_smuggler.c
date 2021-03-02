// n_smuggler.c

#include "g_local.h"
#include "n_smuggler.h"

void smuggler_update_head (edict_t *self);

mframe_t smuggler_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t smuggler_move_stand = {FRAME_standA, FRAME_standB, smuggler_frames_stand, NULL};

#define WS		8

mframe_t smuggler_frames_walk [] =
{
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head,
	ai_walk, WS,	smuggler_update_head
};
mmove_t smuggler_move_walk = {FRAME_walkA, FRAME_walkB, smuggler_frames_walk, NULL};

void smuggler_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &smuggler_move_stand;
}

void smuggler_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &smuggler_move_walk;
}

mframe_t smuggler_frames_run [] =
{
	ai_run, 4,  smuggler_update_head,
	ai_run, 15, smuggler_update_head,
	ai_run, 15, smuggler_update_head,
	ai_run, 8,  smuggler_update_head,
	ai_run, 20, smuggler_update_head,
	ai_run, 15, smuggler_update_head
};
mmove_t smuggler_move_run = {FRAME_runA, FRAME_runB, smuggler_frames_run, NULL};

void smuggler_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		smuggler_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &smuggler_move_run;
}

//**************
//pain
//**************

void smuggler_pain (edict_t *self, edict_t *other, float kick, int damage)
{


}


//**************
//Death
//**************

void smuggler_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t smuggler_frames_deatha [] =
{
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head
};
mmove_t smuggler_move_deatha = {FRAME_deathA, FRAME_deathB, smuggler_frames_deatha, smuggler_dead};

mframe_t smuggler_frames_deathb [] =
{
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head
};
mmove_t smuggler_move_deathb = {FRAME_deathbA, FRAME_deathbB, smuggler_frames_deathb, smuggler_dead};

mframe_t smuggler_frames_deathc [] =
{
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head,
	ai_move, 0,   smuggler_update_head
};
mmove_t smuggler_move_deathc = {FRAME_deathcA, FRAME_deathcB, smuggler_frames_deathc, smuggler_dead};

void smuggler_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float n;

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->s.modelindex2 = 0;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = random();

	if(n < 0.33)
	{
		self->monsterinfo.currentmove = &smuggler_move_deatha;
	}
	else if(n < 0.66)
	{
		self->monsterinfo.currentmove = &smuggler_move_deathb;
	}
	else
	{
		self->monsterinfo.currentmove = &smuggler_move_deathc;
	}
}

//
//ATTACK
//
void smuggler_fire (edict_t *self)
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

	smuggler_update_head(self);

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

void smuggler_attack_refire (edict_t *self)
{
	trace_t tr;

	if (self->enemy->health <= 0)
		return;

	if(random() >= 0.5)
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->enemy->s.origin, self, MASK_MONSTERSOLID);

	if(skill->value == 3)
		self->monsterinfo.nextframe = FRAME_aimfireB;
	else if (skill->value >= 1 && (tr.ent == self->enemy))
		self->monsterinfo.nextframe = FRAME_aimfire03;
	else
		self->monsterinfo.nextframe = FRAME_aimfireB;

	smuggler_update_head(self);
}

mframe_t smuggler_frames_attack [] =
{
	ai_charge, 0, smuggler_update_head,
	ai_charge, 0, smuggler_update_head,
	ai_charge, 0, smuggler_update_head,
	ai_charge, 0, smuggler_update_head,
	ai_charge, 0, smuggler_fire,
	ai_charge, 0, smuggler_attack_refire
};
mmove_t smuggler_move_attack = {FRAME_aimfireA, FRAME_aimfireB, smuggler_frames_attack, smuggler_run};

void smuggler_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &smuggler_move_attack;
}

//SIGHT

void smuggler_sight (edict_t *self, edict_t *other)
{
}

/*QUAKED npc_smuggler (1 .5 0) (-16 -16 -24) (16 16 32)
*/

void smuggler_update_head (edict_t *self)
{
	VectorCopy(self->child->s.origin, self->child->s.old_origin);
	VectorCopy(self->s.angles, self->child->s.angles);
	self->child->s.origin[0] = self->s.origin[0];// + self->velocity[0];
	self->child->s.origin[1] = self->s.origin[1];// + self->velocity[0];
	self->child->s.origin[2] = self->s.origin[2];// + self->velocity[0];

	if(self->deadflag == DEAD_DEAD)
	{
		self->child->s.frame = self->s.frame+1;
	}
	else
	{
		self->child->s.frame = self->s.frame;
	}
}

void smuggler_head_update (edict_t *self)
{
	self->s.origin[2] += 1;
}

void SP_smuggler_head (edict_t *body)
{
	edict_t *head;

	head = G_Spawn();
	head->movetype = MOVETYPE_NONE;
	head->solid = SOLID_NOT;
	head->s.modelindex = gi.modelindex ("models/monsters/smuggler/smugheada.md2");
	VectorCopy(body->s.angles, head->s.angles);
	head->s.frame = body->s.frame;
	VectorSet (head->mins, -2, -2, -24);
	VectorSet (head->maxs, 2, 2, 32);
	head->s.skinnum = 1;

	head->s.origin[0] = body->s.origin[0];
	head->s.origin[1] = body->s.origin[1];
	head->s.origin[2] = body->s.origin[2];

	head->nextthink = level.time + 1;
	head->think = smuggler_head_update;

	gi.linkentity (head);

	body->child = head;
}

void SP_npc_smuggler (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/smuggler/nsmug.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/smuggler/blastera.md2");
//	self->s.modelindex3 = gi.modelindex ("models/monsters/smuggler/smugheada.md2");
	VectorSet (self->mins, -12, -12, -24);
	VectorSet (self->maxs, 12, 12, 32);

	if (!self->health)
		self->health = 30;
	self->mass = 200;
	self->yaw_speed = 45;

	self->pain = smuggler_pain;
	self->die = smuggler_die;

	self->monsterinfo.stand = smuggler_stand;
	self->monsterinfo.walk = smuggler_walk;
	self->monsterinfo.run = smuggler_run;
	self->monsterinfo.attack = smuggler_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = smuggler_sight;

	self->path_node = NO_NODES;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &smuggler_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);

	SP_smuggler_head (self);
}
