/*
==============================================================================

DR. SCHABBS  ( WOLFENSTEIN EPISODE 2 BOSS )

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_doctor.h"

static int	sound_pain;
static int	sound_die;
static int	sound_attack;
static int	sound_sight;

//
// Dr Schabbs injection needle
//

void idg1_needle_think (edict_t *self)
{
	
	// animation of rotating needle
	
	if(self->s.frame >= 11)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	

static void idg1_needle_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		// FIX MOD
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL);
	}
	
	// green explosion sprite 
	
	gi.unlinkentity(ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/dtexp.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = d_texplode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}


void idg1_fire_needle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*needle;

	needle = G_Spawn();
	VectorCopy (start, needle->s.origin);
	VectorCopy (dir, needle->movedir);
	vectoangles (dir, needle->s.angles);
	VectorScale (dir, speed, needle->velocity);
	needle->movetype = MOVETYPE_FLYMISSILE;
	needle->clipmask = MASK_SHOT;
	needle->solid = SOLID_BBOX;
	needle->s.frame = 0;
	needle->s.renderfx |= RF_TRANSLUCENT;

	VectorClear (needle->mins);
	VectorClear (needle->maxs);
	needle->s.modelindex = gi.modelindex ("models/monsters/idg1/doctor/shot/tris.md2");
	needle->owner = self;
	needle->touch = idg1_needle_touch;
	needle->nextthink = level.time + FRAMETIME;
	needle->think = idg1_needle_think;
	needle->dmg = damage;
	//needle->s.sound = gi.soundindex ("weapons/rockfly.wav");
	needle->classname = "needle";
	
	gi.linkentity (needle);
}


mframe_t doctor_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t doctor_move_stand = {FRAME_stand1, FRAME_stand6, doctor_frames_stand, NULL};

void doctor_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &doctor_move_stand;
}

mframe_t doctor_frames_walk [] =
{
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL
};
mmove_t doctor_move_walk = {FRAME_walk1, FRAME_walk8, doctor_frames_walk, NULL};

void doctor_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &doctor_move_walk;
}

mframe_t doctor_frames_run [] =
{
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL,
	ai_run, 5, NULL
};
mmove_t doctor_move_run = {FRAME_walk1, FRAME_walk8, doctor_frames_run, NULL};

void doctor_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &doctor_move_stand;
	else
		self->monsterinfo.currentmove = &doctor_move_run;
}


mframe_t doctor_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t doctor_move_pain = {FRAME_pain1, FRAME_pain5, doctor_frames_pain, doctor_run};

void doctor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

		// Lessen the chance of him going into his pain frames
	if (damage <=50)
		if (random()<0.3)
			return;

	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
	self->monsterinfo.currentmove = &doctor_move_pain;
	self->pain_debounce_time = level.time + 3;
}


void doctor_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void doctor_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t doctor_frames_death [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL
};
mmove_t doctor_move_death = {FRAME_death1, FRAME_death6, doctor_frames_death, doctor_dead};



void doctor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg1/gib.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
    self->monsterinfo.currentmove = &doctor_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}

void doctor_fire (edict_t *self)
{
	vec3_t	forward, right, start;
	vec3_t	end, dir;

	if(!self->enemy)
		return;

	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_NORM, 0);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_FLYER_BLASTER_1], forward, right, start);
	start[2]+=32;

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);
	VectorNormalize (dir);
	idg1_fire_needle(self,start, dir, 30, 300);
}


mframe_t doctor_frames_attack [] =
{
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  doctor_fire,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t doctor_move_attack = {FRAME_attack1, FRAME_attack5, doctor_frames_attack, doctor_run};

void doctor_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &doctor_move_attack;
}


/*QUAKED monster_idg1_doctor (1 .5 0) (-16 -16 -32) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_idg1_doctor (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_monster_idg2_spiderchild (self);
		return;
	}

	sound_pain = gi.soundindex ("idg1monster/doctor/pain.wav");
	sound_die = gi.soundindex ("idg1monster/doctor/death.wav");	
	sound_sight = gi.soundindex ("idg1monster/doctor/sight.wav");
	sound_attack = gi.soundindex ("idg1weapons/knifeair.wav");


	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg1/doctor/tris.md2");
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 800;

	self->gib_health = -80;
	self->mass = 200;

	self->pain = doctor_pain;
	self->die = doctor_die;

	self->monsterinfo.stand = doctor_stand;
	self->monsterinfo.walk = doctor_walk;
	self->monsterinfo.run = doctor_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = doctor_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = doctor_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &doctor_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
