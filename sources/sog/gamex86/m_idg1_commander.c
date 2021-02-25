/*
==============================================================================

WOLF : OTTO GIFTMACHER (boss with bazooka)

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_commander.h"

static int	sound_sight;
static int	sound_death;
static int	sound_attack;


// STAND

void commander_stand (edict_t *self);

mframe_t commander_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t commander_move_stand = {FRAME_stand1, FRAME_stand1, commander_frames_stand, NULL};

void commander_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &commander_move_stand;
}


//
// WALK
//

mframe_t commander_frames_walk [] =
{
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL
};
mmove_t commander_move_walk = {FRAME_walk1, FRAME_walk4, commander_frames_walk, NULL};

void commander_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &commander_move_walk;
}


//
// RUN
//

void commander_run (edict_t *self);

mframe_t commander_frames_run [] =
{
	ai_run, 12, NULL, 
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL
};
mmove_t commander_move_run = {FRAME_walk1, FRAME_walk4, commander_frames_run, commander_run};

void commander_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &commander_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &commander_move_run;

}

//
// PAIN
//


mframe_t commander_frames_pain [] =
{
	ai_move, 0, NULL
};
mmove_t commander_move_pain = {FRAME_stand1, FRAME_stand1, commander_frames_pain, commander_run};

void commander_pain (edict_t *self, edict_t *other, float kick, int damage)
{
    
	if (level.time < self->pain_debounce_time)
		return;
			
	// Lessen the chance of him going into his pain frames
	if (damage <=50)
		if (random()<0.3)
			return;

	self->monsterinfo.currentmove = &commander_move_pain;
	self->pain_debounce_time = level.time + 0.5;
}


//
// ATTACK
//

void commander_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	trace_t	tr;
	int		dist;

	if(!self->enemy)
		return;

	dist = range (self, self->enemy) ;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,8,16,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);

	switch(dist)
	{
		case RANGE_MELEE:
		{
			vec[0] += self->enemy->velocity[0] * 0.15;  
			vec[1] += self->enemy->velocity[1] * 0.15;
			vec[2] -= self->enemy->viewheight*(0.2);
			break;
		}
		case RANGE_NEAR:
		{
			vec[0] += self->enemy->velocity[0] * 0.28;  
			vec[1] += self->enemy->velocity[1] * 0.28; 
			vec[2] -= self->enemy->viewheight*(0.3);
			break;
		}
		case RANGE_MID:
		{
			vec[0] += self->enemy->velocity[0] * 0.45;  
			vec[1] += self->enemy->velocity[1] * 0.45; 
			vec[2] -= self->enemy->viewheight*(0.4);
			break;
		}
		case RANGE_FAR:
		default:
		{
			vec[0] += self->enemy->velocity[0];  
			vec[1] += self->enemy->velocity[1];
			vec[2] -= self->enemy->viewheight*(random());
			break;
		}
	}

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.sound (self, CHAN_WEAPON, sound_attack, 1.0, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	w_fire_rocket (self,start, dir, 100, 400, 100, 100);

}


mframe_t commander_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, commander_fire
};
mmove_t commander_move_attack = {FRAME_attack1, FRAME_attack2, commander_frames_attack, commander_run};

void commander_attack(edict_t *self)
{
	if(random() > 0.3)
		self->monsterinfo.currentmove = &commander_move_attack;
}


//
// SIGHT
//

void commander_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &commander_move_attack;
	}
}


//
// DEATH
//

void commander_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -32);
	VectorSet (self->maxs, 24, 24, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);

}

mframe_t commander_frames_death [] =
{
	ai_move, 0,   NULL,
	ai_move, -5, NULL,
	ai_move, -3, NULL,
	ai_move, 0, NULL
};
mmove_t commander_move_death = {FRAME_death1, FRAME_death4, commander_frames_death, commander_dead};


void commander_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	
	// hack
	// otherwise, player can't exit map

	if((strcmp("boss2",level.mapname)==0) && self->target)
		G_UseTargets(self,self->enemy);
	
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
	self->s.skinnum = 1;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &commander_move_death;
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

}


//
// SPAWN
//

void SP_monster_idg1_commander (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	sound_sight=	gi.soundindex ("idg1monster/commander/sight.wav");
	sound_death=	gi.soundindex ("idg1monster/commander/death.wav");
	sound_attack=   gi.soundindex ("idg1weapons/rlshot.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg1/commander/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/idg1/commander/weapon.md2");
	VectorSet (self->mins, -24, -24, -32);
	VectorSet (self->maxs, 24, 24, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 400;
	self->health = 2500;
	self->gib_health = -100;

	self->pain = commander_pain;
	self->die = commander_die;

	self->monsterinfo.stand = commander_stand;
	self->monsterinfo.walk = commander_walk;
	self->monsterinfo.run = commander_run;
	self->monsterinfo.attack = commander_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = commander_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &commander_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}