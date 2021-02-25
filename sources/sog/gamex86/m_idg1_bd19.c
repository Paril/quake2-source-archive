/*
==============================================================================

WOLF : HANS GROSSE

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_bd19.h"

static int	sound_sight;
static int	sound_death;
static int	sound_attack;


void grosse_idle (edict_t *self)
{

// No idle-sound

}



// STAND

void grosse_stand (edict_t *self);

mframe_t grosse_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t grosse_move_stand = {FRAME_stand01, FRAME_stand07, grosse_frames_stand, grosse_stand};

void grosse_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &grosse_move_stand;
}


//
// WALK
//

mframe_t grosse_frames_walk [] =
{
	ai_walk, 7,  NULL,
	ai_walk, 9,  NULL,
	ai_walk, 11,  NULL,
	ai_walk, 12,  NULL,
	ai_walk, 11,  NULL,
	ai_walk, 9,  NULL,
	ai_walk, 8,  NULL
};
mmove_t grosse_move_walk = {FRAME_walk01, FRAME_walk07, grosse_frames_walk, NULL};

void grosse_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &grosse_move_walk;
}


//
// RUN
//

void grosse_run (edict_t *self);

mframe_t grosse_frames_run [] =
{
	ai_run, 13, NULL, 
	ai_run, 15, NULL,
	ai_run, 18, NULL,
	ai_run, 20, NULL,
	ai_run, 19, NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL
};
mmove_t grosse_move_run = {FRAME_walk01, FRAME_walk07, grosse_frames_run, grosse_run};

void grosse_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &grosse_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &grosse_move_run;

}

mframe_t grosse_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t grosse_move_pain = {FRAME_stand01, FRAME_stand05, grosse_frames_pain, grosse_run};


//
// PAIN
//

void grosse_pain (edict_t *self, edict_t *other, float kick, int damage)
{
    
	if (level.time < self->pain_debounce_time)
		return;

		// Lessen the chance of him going into his pain frames
	if (damage <=50)
		if (random()<0.3)
			return;

	self->monsterinfo.currentmove = &grosse_move_pain;
	self->pain_debounce_time = level.time + 0.5;
}


//
// ATTACK
//

void grosse_fire (edict_t *self)
{

	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t  offset;
		
	if(!self->enemy)
		return;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,8,16,0);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack, 1, ATTN_NORM, 0);
	monster_fire_bullet (self, start, aim, 12, 1, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_INFANTRY_MACHINEGUN_1);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

}

void grosse_prepare_fire (edict_t *self)
{
	int		n;

	n = (rand() & 15) + 5;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

mframe_t grosse_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, grosse_prepare_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, grosse_fire,
	ai_charge, 0, NULL
};
mmove_t grosse_move_attack = {FRAME_attack01, FRAME_attack07, grosse_frames_attack, grosse_run};

void grosse_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &grosse_move_attack;
}


//
// SIGHT
//

void grosse_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &grosse_move_attack;
	}
}


//
// DEATH
//

void grosse_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -43);
	VectorSet (self->maxs, 24, 24, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t grosse_frames_death [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL
};
mmove_t grosse_move_death = {FRAME_death01, FRAME_death06, grosse_frames_death, grosse_dead};


void grosse_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg1/gib.wav"), 1, ATTN_NORM, 0);

		for (n= 0; n < 2; n++)
        {
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
     		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
		}
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &grosse_move_death;

}


//
// SPAWN
//

void SP_monster_idg1_bd19 (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 11;
		SP_monster_idg2_cu (self);
		return;
	}	
	
	sound_sight=	gi.soundindex ("idg1monster/bd19/sight.wav");
	sound_death=	gi.soundindex ("idg1monster/bd19/death.wav");
	sound_attack=   gi.soundindex ("idg1weapons/ggshot.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg1/bd19/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/idg1/bd19/weapon.md2");
	VectorSet (self->mins, -24, -24, -43);
	VectorSet (self->maxs, 24, 24, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 400;
	self->health = 2000;
	self->gib_health = -100;

	self->pain = grosse_pain;
	self->die = grosse_die;

	self->monsterinfo.stand = grosse_stand;
	self->monsterinfo.walk = grosse_walk;
	self->monsterinfo.run = grosse_run;
	self->monsterinfo.attack = grosse_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = grosse_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &grosse_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}