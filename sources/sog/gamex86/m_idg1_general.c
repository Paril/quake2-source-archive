/*
==============================================================================

WOLF : GENERAL FETTGESICHT (boss with bazooka and heavy machinegun)

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_general.h"

static int	sound_sight;
static int	sound_death;
static int	sound_attack1;
static int  sound_attack2;


// STAND

void general_stand (edict_t *self);

mframe_t general_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t general_move_stand = {FRAME_stand1, FRAME_stand1, general_frames_stand, NULL};

void general_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &general_move_stand;
}


//
// WALK
//

mframe_t general_frames_walk [] =
{
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 8,  NULL
};
mmove_t general_move_walk = {FRAME_walk1, FRAME_walk4, general_frames_walk, NULL};

void general_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &general_move_walk;
}


//
// RUN
//

void general_run (edict_t *self);

mframe_t general_frames_run [] =
{
	ai_run, 12, NULL, 
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL
};
mmove_t general_move_run = {FRAME_walk1, FRAME_walk4, general_frames_run, general_run};

void general_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &general_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &general_move_run;

}

//
// PAIN
//

mframe_t general_frames_pain [] =
{
	ai_move, 0, NULL
};
mmove_t general_move_pain = {FRAME_stand1, FRAME_stand1, general_frames_pain, general_run};

void general_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

		// Lessen the chance of him going into his pain frames
	if (damage <=50)
		if (random()<0.3)
			return;

	self->monsterinfo.currentmove = &general_move_pain;
	self->pain_debounce_time = level.time + 0.5;
}


//
// ATTACK
//

void general_fire_machinegun (edict_t *self)
{

	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t offset;
		
	if(!self->enemy)
		return;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,8,-16,0);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);
	VectorSubtract (end, start, aim);
	VectorNormalize (aim);
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack2, 1, ATTN_NORM, 0);
	monster_fire_bullet (self, start, aim, 12, 1, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_INFANTRY_MACHINEGUN_1);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

}

void general_prepare_fire_machinegun (edict_t *self)
{
	int		n;

	n = (rand() & 15) + 3 + 7;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

mframe_t general_frames_attack_machinegun [] =
{
	ai_charge, 0, general_prepare_fire_machinegun,
	ai_charge, 0, NULL,
	ai_charge, 0, general_fire_machinegun,
	ai_charge, 0, NULL
};
mmove_t general_move_attack_machinegun = {FRAME_attackb1, FRAME_attackb4, general_frames_attack_machinegun, general_run};


void general_fire_rocket (edict_t *self)
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

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1.0, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	w_fire_rocket (self,start, dir, 100, 400, 100, 100);

}


mframe_t general_frames_attack_rocket [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, general_fire_rocket
};
mmove_t general_move_attack_rocket = {FRAME_attack1, FRAME_attack2, general_frames_attack_rocket, general_run};

void general_attack(edict_t *self)
{
	float f= random();
	if(f < 0.33)
		self->monsterinfo.currentmove = &general_move_attack_rocket;
	else if(f < 0.66)
		self->monsterinfo.currentmove = &general_move_attack_machinegun;	
}


//
// SIGHT
//

void general_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &general_move_attack_machinegun;
	}
}


//
// DEATH
//

void general_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -32);
	VectorSet (self->maxs, 24, 24, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t general_frames_death [] =
{
	ai_move, 0,   NULL,
	ai_move, -5, NULL,
	ai_move, -3, NULL,
	ai_move, 0, NULL
};
mmove_t general_move_death = {FRAME_death1, FRAME_death4, general_frames_death, general_dead};


void general_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->s.skinnum = 1;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &general_move_death;
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

}


//
// SPAWN
//

void SP_monster_idg1_general (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 8;
		SP_monster_idg2_super (self);
		return;
	}

	sound_sight=	gi.soundindex ("idg1monster/general/sight.wav");
	sound_death=	gi.soundindex ("idg1monster/general/death.wav");
	sound_attack1=   gi.soundindex ("idg1weapons/rlshot.wav");
	sound_attack2=   gi.soundindex ("idg1weapons/ggshot.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg1/general/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/idg1/general/weapon.md2");
	VectorSet (self->mins, -24, -24, -32);
	VectorSet (self->maxs, 24, 24, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 400;
	self->health = 3500;
	self->gib_health = -150;

	self->pain = general_pain;
	self->die = general_die;

	self->monsterinfo.stand = general_stand;
	self->monsterinfo.walk = general_walk;
	self->monsterinfo.run = general_run;
	self->monsterinfo.attack = general_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = general_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &general_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}