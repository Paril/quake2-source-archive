/*
==============================================================================

DOOM II ARACHNOTRON

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_spiderchild.h"

static int sound_pain;
static int sound_die;
static int sound_sight;
static int sound_attack;


//
// Arachnotron Plasma Shot   by Sir Psycho
//

void spiderchild_plasma_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t origin;
	int n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	
	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_UNKNOWN);
	}

	gi.sound (ent, CHAN_AUTO, gi.soundindex ("idg2weapons/phit.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLASMA_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void spiderchild_plasma_think (edict_t *self)
{
	
	// animation of white plasma
	
	if(self->s.frame >= 3)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	

void spiderchild_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *plasma;
	int r;

	plasma = G_Spawn();
	VectorCopy (start, plasma->s.origin);
	VectorCopy (dir, plasma->movedir);

	vectoangles (dir, plasma->s.angles);
	VectorScale (dir, speed, plasma->velocity);
	plasma->movetype = MOVETYPE_FLYMISSILE;
	plasma->clipmask = MASK_SHOT;
	plasma->solid = SOLID_BBOX;
	plasma->s.effects |= (EF_HYPERBLASTER|EF_ANIM_ALLFAST); 
	VectorClear (plasma->mins);
	VectorClear (plasma->maxs);

	plasma->s.modelindex = gi.modelindex ("sprites/idg2spi.sp2");
	plasma->owner = self;
	plasma->touch = spiderchild_plasma_touch;
	plasma->nextthink = level.time + FRAMETIME;
	plasma->think = spiderchild_plasma_think;
	plasma->dmg = damage;
	plasma->classname = "plasma";

	gi.linkentity (plasma);
} 

// end Sir Psycho



mframe_t idg2_spiderchild_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t idg2_spiderchild_move_stand = {FRAME_stand1, FRAME_stand10, idg2_spiderchild_frames_stand, NULL};

void idg2_spiderchild_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_spiderchild_move_stand;
}

mframe_t idg2_spiderchild_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL
};
mmove_t idg2_spiderchild_move_walk = {FRAME_walk1, FRAME_walk5, idg2_spiderchild_frames_walk, NULL};

void idg2_spiderchild_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_spiderchild_move_walk;
}

mframe_t idg2_spiderchild_frames_run [] =
{
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 12, NULL
};
mmove_t idg2_spiderchild_move_run = {FRAME_walk1, FRAME_walk5, idg2_spiderchild_frames_run, NULL};

void idg2_spiderchild_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_spiderchild_move_stand;
	else
		self->monsterinfo.currentmove = &idg2_spiderchild_move_run;
}


mframe_t idg2_spiderchild_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_spiderchild_move_pain = {FRAME_pain1, FRAME_pain4, idg2_spiderchild_frames_pain, idg2_spiderchild_run};

void idg2_spiderchild_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &idg2_spiderchild_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
}



void idg2_spiderchild_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void idg2_spiderchild_dead (edict_t *self)
{
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;

	gi.linkentity (self);

}

mframe_t idg2_spiderchild_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL
};
mmove_t idg2_spiderchild_move_death = {FRAME_death1, FRAME_death6, idg2_spiderchild_frames_death, idg2_spiderchild_dead};



void idg2_spiderchild_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
    self->monsterinfo.currentmove = &idg2_spiderchild_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}
/*
void idg2_spiderchild_prepare_fire (edict_t *self)
{
	int		n;

	n = (rand() & 15) + 3;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}
*/
void idg2_spiderchild_fire (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	vec3_t  offset;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,0,0,0);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	if (self->enemy)
	{
		VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
		target[2] += self->enemy->viewheight;
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	else
	{
		AngleVectors (self->s.angles, forward, right, NULL);
	}

	gi.sound (self, CHAN_WEAPON, sound_attack, 1.0, ATTN_NORM, 0);
	
	spiderchild_plasma (self, start, forward, 12, 300);
/*	
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
*/
}

mframe_t idg2_spiderchild_frames_attack [] =
{
	ai_charge, 0, idg2_spiderchild_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_spiderchild_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_spiderchild_fire
};
mmove_t idg2_spiderchild_move_attack = {FRAME_attack1, FRAME_attack5, idg2_spiderchild_frames_attack, idg2_spiderchild_run};

void idg2_spiderchild_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &idg2_spiderchild_move_attack;
}


/*QUAKED monster_idg2_spiderchild (1 .5 0) (-32 -32 -24) (32 32 16) Ambush Trigger_Spawn Sight
DOOM II Arachnotron
*/
void SP_monster_idg2_spiderchild (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 8;
		SP_monster_idg1_doctor (self);
		return;
	}

	sound_pain = gi.soundindex ("idg2monster/spiderchild/pain.wav");
	sound_die = gi.soundindex ("idg2monster/spiderchild/death.wav");	
	sound_sight = gi.soundindex ("idg2monster/spiderchild/sight.wav");
	sound_attack = gi.soundindex ("idg2weapons/plasma.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg2/spiderchild/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 16);

	self->health = 300;

	self->gib_health = -100;
	self->mass = 400;

	self->pain = idg2_spiderchild_pain;
	self->die = idg2_spiderchild_die;

	self->monsterinfo.stand = idg2_spiderchild_stand;
	self->monsterinfo.walk = idg2_spiderchild_walk;
	self->monsterinfo.run = idg2_spiderchild_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg2_spiderchild_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_spiderchild_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_spiderchild_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
