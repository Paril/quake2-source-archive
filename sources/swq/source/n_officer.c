// n_officer.c

#include "g_local.h"
#include "n_officer.h"

void find_near_mark (edict_t *self, float radius);
void officer_walk (edict_t *self);
void officer_stand (edict_t *self);
void ai_npc_walk (edict_t *self, float dist);
void ai_npc_stand (edict_t *self, float dist);

mframe_t officer_frames_work [] =
{
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL
};
mmove_t officer_move_work = {FRAME_work01, FRAME_work10, officer_frames_work, NULL};

mframe_t officer_frames_stand [] =
{
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,
	ai_npc_stand, 0, NULL,

	ai_npc_stand, 0, NULL
};
mmove_t officer_move_stand = {FRAME_stand01, FRAME_stand11, officer_frames_stand, NULL};

#define WS		3

mframe_t officer_frames_walk [] =
{
	ai_npc_walk, WS,  NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS,  NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS,  NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS, NULL,
	ai_npc_walk, WS,  NULL,
	ai_npc_walk, WS,  NULL,
	ai_npc_walk, WS, NULL
};
mmove_t officer_move_walk = {FRAME_walk211, FRAME_walk221, officer_frames_walk, NULL};

mframe_t officer_frames_start_walk [] =
{
	ai_npc_walk, .5,  NULL,
	ai_npc_walk, 1, NULL,
	ai_npc_walk, 1.5, NULL,
	ai_npc_walk, 2,  NULL,
	ai_npc_walk, 2.5, NULL,
	ai_npc_walk, 3, NULL,
	ai_npc_walk, 3, NULL,
	ai_npc_walk, 3,  NULL,
	ai_npc_walk, 3, NULL,
	ai_npc_walk, 3, NULL
};
mmove_t officer_move_start_walk = {FRAME_walk101, FRAME_walk110, officer_frames_start_walk, officer_walk};

mframe_t officer_frames_end_walk [] =
{
	ai_npc_stand, 3,  NULL,
	ai_npc_stand, 2.5, NULL,
	ai_npc_stand, 2, NULL,
	ai_npc_stand, 1.5,  NULL,
	ai_npc_stand, 1, NULL,
	ai_npc_stand, .5, NULL
};
mmove_t officer_move_end_walk = {FRAME_walk322, FRAME_walk327, officer_frames_end_walk, officer_stand};

void officer_stand (edict_t *self)
{
	if(!self->action_target && !self->monsterinfo.aiflags & AI_STAND_GROUND && !self->monsterinfo.pausetime)
	{
		self->action_target = G_PickTarget(self->pathtarget);
	}

	if(self->monsterinfo.pausetime >= 1000000 && !self->spawnflags & 16)
	{
//		gi.dprintf("npc_officer -- %f\n", self->monsterinfo.pausetime);
//		self->monsterinfo.pausetime = 0;
		if(self->wait)
			self->monsterinfo.pausetime = level.time + self->wait;
		else
			self->monsterinfo.pausetime = 0;
	}

//	if(self->ideal_yaw != self->s.angles[YAW])
//		M_ChangeYaw(self);

	if (self->monsterinfo.currentmove == &officer_move_stand ||
		self->monsterinfo.currentmove == &officer_move_work ||
		self->monsterinfo.currentmove == &officer_move_end_walk)
	{
		if(self->spawnflags & 8 && !self->monsterinfo.aiflags & AI_STAND_GROUND)
			self->monsterinfo.currentmove = &officer_move_work;
		else
			self->monsterinfo.currentmove = &officer_move_stand;
	}
	else
	{
		if(self->spawnflags & 8 && !self->monsterinfo.aiflags & AI_STAND_GROUND)
			self->monsterinfo.currentmove = &officer_move_work;
		else
			self->monsterinfo.currentmove = &officer_move_stand;
	}
}

void officer_walk (edict_t *self)
{
	if (self->monsterinfo.currentmove == &officer_move_walk ||
		self->monsterinfo.currentmove == &officer_move_start_walk)
	{
		self->monsterinfo.currentmove = &officer_move_walk;
	}
	else
	{
		self->monsterinfo.currentmove = &officer_move_start_walk;
	}
}

mframe_t officer_frames_run [] =
{
	ai_npc_walk, 4,  NULL,
	ai_npc_walk, 15, NULL,
	ai_npc_walk, 15, NULL,
	ai_npc_walk, 8,  NULL,
	ai_npc_walk, 20, NULL,
	ai_npc_walk, 15, NULL,
	ai_npc_walk, 8,  NULL,
	ai_npc_walk, 20, NULL
};
mmove_t officer_move_run = {FRAME_run01, FRAME_run08, officer_frames_run, NULL};

void officer_run (edict_t *self)
{
	if(!self->action_target)
		self->action_target = G_PickTarget(self->combattarget);

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		officer_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &officer_move_run;
}

void officer_pain_hide (edict_t *self)
{
	vec3_t		v;

	if(self->deathtarget)
	{
		self->goalentity = self->movetarget = G_PickTarget(self->deathtarget);
		if ((!self->movetarget) || (strcmp(self->movetarget->classname, "officer_mark") != 0))
		{
			gi.dprintf ("%s has bad target %s at %s\n", self->classname, self->deathtarget);
			self->deathtarget = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand (self);
			return;
		}

		VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
		self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
		self->monsterinfo.walk (self);
		self->deathtarget = NULL;
	}
//	else
//	{
//		find_near_mark(self, 1024);
//	}
}

void officer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}

void officer_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t officer_frames_death1 [] =
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
mmove_t officer_move_death1 = {FRAME_death01, FRAME_death16, officer_frames_death1, officer_dead};

void officer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &officer_move_death1;
}

void officer_use (edict_t *self, edict_t *other, edict_t *activator)
{
}

void officer_attack (edict_t *self)
{
}

/*QUAKED npc_officer (1 .5 0) (-16 -16 -24) (16 16 32)
*/
void SP_npc_officer (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/npcs/officer/tris.md2");
	VectorSet (self->mins, -8, -8, -24);
	VectorSet (self->maxs, 8, 8, 32);

	if (!self->health)
		self->health = 30;
	self->mass = 200;
	self->yaw_speed = 15;

	self->pain = officer_pain;
	self->die = officer_die;

	self->monsterinfo.stand = officer_stand;
	self->monsterinfo.walk = officer_walk;
	self->monsterinfo.run = officer_run;
	self->monsterinfo.attack = officer_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->path_node = NO_NODES;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &officer_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
	if(self->wait)
		self->monsterinfo.pausetime = level.time + self->wait;
	else
		self->monsterinfo.pausetime = level.time + 20;
}
