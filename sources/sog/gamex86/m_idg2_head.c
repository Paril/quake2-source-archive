/*
==============================================================================

idg2_head (cacodemon)

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_head.h"

static int	sound_sight;
static int	sound_pain;
static int	sound_bite;
static int	sound_shoot;
static int	sound_death;

void d_texplode (edict_t *self);  // from m_idg2_cu.c

//
// Cacodemon fireball
//

static void idg2_head_fireball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg,(ent->dmg*2), 0, MOD_Q1_RL);
	}
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("idg2monster/monkey/hit.wav"), 1.0, ATTN_NORM, 0);	
	// explosion sprite 
	gi.unlinkentity(ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/headexp.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = d_texplode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}

void idg2_head_fireball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*toxicball;
	toxicball = G_Spawn();
	VectorCopy (start, toxicball->s.origin);
	VectorCopy (aimdir, toxicball->movedir);
	vectoangles (aimdir, toxicball->s.angles);
	VectorScale (aimdir, speed, toxicball->velocity);
	toxicball->movetype = MOVETYPE_FLYMISSILE;
	toxicball->clipmask = MASK_SHOT;
	toxicball->solid = SOLID_BBOX;
	toxicball->s.effects |= (EF_ANIM_ALLFAST|EF_HYPERBLASTER);
	toxicball->s.renderfx |= RF_TRANSLUCENT;
	VectorClear (toxicball->mins);
	VectorClear (toxicball->maxs);
	toxicball->s.modelindex = gi.modelindex ("sprites/headball.sp2");
	toxicball->owner = self;
	toxicball->touch = idg2_head_fireball_touch;
	toxicball->nextthink = level.time + 8000/speed;
	toxicball->think = G_FreeEdict;
	toxicball->dmg = damage;
	toxicball->s.sound = gi.soundindex ("idg2monster/monkey/attack2.wav");
	toxicball->classname = "fireball";
	gi.linkentity (toxicball);
}

void idg2_head_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


mframe_t idg2_head_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL	
};
mmove_t	idg2_head_move_stand = {FRAME_stand1, FRAME_stand4, idg2_head_frames_stand, NULL};

void idg2_head_stand (edict_t *self)
{	self->monsterinfo.currentmove = &idg2_head_move_stand;
}

mframe_t idg2_head_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t	idg2_head_move_walk = {FRAME_stand1, FRAME_stand4, idg2_head_frames_walk, NULL};

void idg2_head_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_head_move_walk;
}


mframe_t idg2_head_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};
mmove_t idg2_head_move_run = {FRAME_stand1, FRAME_stand4, idg2_head_frames_run, NULL};

void idg2_head_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_head_move_walk;
	else
		self->monsterinfo.currentmove = &idg2_head_move_run;
}

mframe_t idg2_head_frames_pain [] =
{	
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL
};
mmove_t idg2_head_move_pain = {FRAME_pain1, FRAME_pain3, idg2_head_frames_pain, idg2_head_run};

void idg2_head_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;

	if(!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,0,4,-12);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);	
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_WEAPON, sound_shoot, 1.0, ATTN_NORM, 0);
	idg2_head_fireball (self, start, dir, 40, 200);


}


mframe_t idg2_head_frames_attack2 [] =
{
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, idg2_head_fire,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL
};
mmove_t idg2_head_move_attack2 = {FRAME_attack1, FRAME_attack7, idg2_head_frames_attack2, idg2_head_run};



void idg2_head_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (5 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_bite, 1, ATTN_NORM, 0);
}


mframe_t idg2_head_frames_attack1 [] =
{
		ai_charge, 0, NULL,		
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, idg2_head_bite,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		
};
mmove_t idg2_head_move_attack1 = {FRAME_attack1, FRAME_attack7, idg2_head_frames_attack1, idg2_head_run};



void idg2_head_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_head_move_attack2;
}

void idg2_head_melee(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &idg2_head_move_attack1;
}

void idg2_head_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &idg2_head_move_pain;
}


void idg2_head_dead (edict_t *self)
{
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void idg2_head_deadthink (edict_t *self)

{

	if (!self->groundentity && level.time < self->timestamp)

	{

		self->nextthink = level.time + FRAMETIME;

		return;

	}

	idg2_head_dead(self);

}

void idg2_head_falling_dead (edict_t *self)

{
	self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, -8);
	self->movetype = MOVETYPE_TOSS;

	self->think = idg2_head_deadthink;
	self->nextthink = level.time + FRAMETIME;

	self->timestamp = level.time + 15;
	gi.linkentity (self);
}


mframe_t idg2_head_frames_death [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_head_move_death = {FRAME_death1, FRAME_death6, idg2_head_frames_death, idg2_head_falling_dead};




void idg2_head_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

		ThrowHead (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &idg2_head_move_death;
}
	

/*QUAKED monster_idg2_head (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_idg2_head (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_sight = gi.soundindex ("idg2monster/head/sight.wav");
	sound_pain = gi.soundindex ("idg2monster/head/pain.wav");
	sound_bite = gi.soundindex ("idg2monster/monkey/attack1.wav");
	sound_shoot = gi.soundindex ("idg2monster/monkey/attack2.wav");
	sound_death = gi.soundindex ("idg2monster/head/death.wav");
	
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/head/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 400;
	self->gib_health = -70;
	self->mass = 50;

	self->pain = idg2_head_pain;
	self->die = idg2_head_die;

	self->monsterinfo.stand = idg2_head_stand;
	self->monsterinfo.walk = idg2_head_walk;
	self->monsterinfo.run = idg2_head_run;
	self->monsterinfo.attack = idg2_head_attack;
	self->monsterinfo.melee = idg2_head_melee;
	self->monsterinfo.sight = idg2_head_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_head_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
