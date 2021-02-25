/*
==============================================================================

idg2_actor

The DOOM spacemarine, acting in the background
(which is really cool, I might add)

Spawnflags :
0 = Medic
4 = Medic with chainsaw
8 = Wounded marine, lying down
16 = Marine on guard with chaingun
32 = Dead corpse

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_actor.h"

static int sound_chainsaw_up;

static int sound_chainsaw_working;

static int sound_pain1;

static int sound_pain2;

static int sound_pain3;

static int sound_death;

void idg2_actor_stand (edict_t *self);
void idg2_actor_dead (edict_t *self);
void idg2_actor_walk (edict_t *self);
void idg2_actor_run (edict_t *self);

void idg2_actor_chainsaw_start (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_chainsaw_up, 0.7, ATTN_IDLE, 0);
}

void idg2_actor_chainsaw_working (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_chainsaw_working, 0.7, ATTN_IDLE, 0);
}

void idg2_actor_scream (edict_t *self)
{
	int r;
	
	r = 1 + (rand()&2);

	if (r = 1)

		gi.sound (self, CHAN_VOICE, sound_pain1, 0.5, ATTN_IDLE, 0);

	else if (r = 2)

		gi.sound (self, CHAN_VOICE, sound_pain2, 0.5, ATTN_IDLE, 0);

	else 

		gi.sound (self, CHAN_VOICE, sound_pain3, 0.5, ATTN_IDLE, 0);

}

mframe_t idg2_actor_frames_stand [] =
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
mmove_t idg2_actor_move_stand = {FRAME_stand01, FRAME_stand40, idg2_actor_frames_stand, idg2_actor_stand};

mframe_t idg2_actor_frames_bed [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL	
};
mmove_t idg2_actor_move_bed = {FRAME_bed1, FRAME_bed6, idg2_actor_frames_bed, idg2_actor_stand};

mframe_t idg2_actor_frames_walk [] =
{
	ai_walk,	9,	NULL,
	ai_walk,	9,	NULL,
	ai_walk,	8,	NULL,
	ai_walk,	8,	NULL,
	ai_walk,	8,	NULL,
	ai_walk,	9,	NULL
};
mmove_t idg2_actor_move_walk = {FRAME_run1, FRAME_run6, idg2_actor_frames_walk, idg2_actor_walk};
mmove_t idg2_actor_move_run = {FRAME_run1, FRAME_run6, idg2_actor_frames_walk, idg2_actor_run};

mframe_t idg2_actor_frames_saw [] =
{
	ai_stand,	0,	idg2_actor_chainsaw_start,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	idg2_actor_chainsaw_working,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg2_actor_move_saw = {FRAME_attack1, FRAME_attack8, idg2_actor_frames_saw, idg2_actor_stand};


mframe_t idg2_actor_frames_point [] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg2_actor_move_point = {FRAME_point01, FRAME_point12, idg2_actor_frames_point, idg2_actor_stand};

mframe_t idg2_actor_frames_pain1 [] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	idg2_actor_scream,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg2_actor_move_pain1 = {FRAME_pain201, FRAME_pain204, idg2_actor_frames_pain1, idg2_actor_stand};

mframe_t idg2_actor_frames_pain2 [] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	idg2_actor_scream,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg2_actor_move_pain2 = {FRAME_bedpain1, FRAME_bedpain4, idg2_actor_frames_pain1, idg2_actor_stand};

mframe_t idg2_actor_frames_death1 [] =
{
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL
};
mmove_t idg2_actor_move_death1 = {FRAME_death201, FRAME_death206, idg2_actor_frames_death1, idg2_actor_dead};

mframe_t idg2_actor_frames_death2 [] =
{
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL
};
mmove_t idg2_actor_move_death2 = {FRAME_beddeath1, FRAME_beddeath6, idg2_actor_frames_death2, idg2_actor_dead};

void idg2_actor_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_actor_move_walk;
}

void idg2_actor_run (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_actor_move_run;

}

void idg2_actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (self->spawnflags & 8)
		self->monsterinfo.currentmove = &idg2_actor_move_pain2;
	else
		self->monsterinfo.currentmove = &idg2_actor_move_pain1;

}

void idg2_actor_stand (edict_t *self)
{
	if (self->spawnflags & 8)
	{	
		if (random() < 0.05)
			self->monsterinfo.currentmove = &idg2_actor_move_pain2;
		else
			self->monsterinfo.currentmove = &idg2_actor_move_bed;
	}
	else if (random() < 0.5)
	
		self->monsterinfo.currentmove = &idg2_actor_move_stand;
	
	else if ((self->spawnflags & 4) && (random() > 0.5))
		
		self->monsterinfo.currentmove = &idg2_actor_move_saw;
	
	else
		
		self->monsterinfo.currentmove = &idg2_actor_move_point;

	
}


void idg2_actor_dead (edict_t *self)
{

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;

	self->svflags |= SVF_DEADMONSTER;

	self->nextthink = 0;
	gi.linkentity (self);
}

void idg2_actor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	self->s.modelindex2 = 0;

	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_IDLE, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/idg2head/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_IDLE, 0);



	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (self->spawnflags & 8)
		self->monsterinfo.currentmove = &idg2_actor_move_death2;
	else
		self->monsterinfo.currentmove = &idg2_actor_move_death1;

}

/*QUAKED misc_idg2_actor (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn MEDIC_SAW MARINE_BED MARINE_GUARD CORPSE
*/
void SP_misc_idg2_actor (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}


	sound_chainsaw_up = gi.soundindex ("idg2weapons/sawup.wav");
	sound_chainsaw_working = gi.soundindex ("idg2weapons/sawon.wav");
	sound_pain1 = gi.soundindex ("idg2monster/actor/pain1.wav");
	sound_pain2 = gi.soundindex ("idg2monster/actor/pain2.wav");
	sound_pain3 = gi.soundindex ("idg2monster/actor/pain3.wav");
	sound_death = gi.soundindex ("idg2monster/actor/death.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/objects/idg2actor/tris.md2");

	if (self->spawnflags & 4)
	{
		self->s.modelindex2 = gi.modelindex("models/objects/idg2actor/chainsaw.md2");
	}

	if (self->spawnflags & 16)
	{
		self->s.modelindex2 = gi.modelindex("models/objects/idg2actor/chaingun.md2");
	}
	
	if (self->spawnflags & 8)
	{
		VectorSet (self->mins, -16, -16, -24);
		VectorSet (self->maxs, 16, 16, -8);
	}
	else
	{
		VectorSet (self->mins, -16, -16, -24);
		VectorSet (self->maxs, 16, 16, 32);
	};
	
	self->health = 100;

	self->gib_health = -50;
	self->mass = 300;

	self->pain = idg2_actor_pain;
	self->die = idg2_actor_die;

	self->monsterinfo.stand = idg2_actor_stand;
	self->monsterinfo.walk = idg2_actor_walk;
	self->monsterinfo.run = idg2_actor_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_actor_move_stand;
	


	self->monsterinfo.scale = MODEL_SCALE;
	
	walkmonster_start (self);

	if ((self->spawnflags & 8) || (self->spawnflags & 16) || (self->spawnflags & 32))
		self->s.skinnum = 0;
	else
		self->s.skinnum = 1;

	// Corpse

	if (self->spawnflags & 32)
	{
		self->health = -1;
		self->deadflag = DEAD_DEAD;
		self->takedamage = DAMAGE_YES;
		self->monsterinfo.currentmove = NULL;
		VectorSet (self->mins, -16, -16, -32);
		VectorSet (self->maxs, 16, 16, -16);
		self->movetype = MOVETYPE_TOSS;
		self->svflags |= SVF_DEADMONSTER;
		self->monsterinfo.currentmove = NULL;
		self->s.frame = FRAME_death206;
		gi.linkentity (self);
	}

}

