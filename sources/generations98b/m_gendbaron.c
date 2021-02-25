
#include "g_local.h"
#include "m_gendbaron.h"

static int  sound_death;
static int	sound_pain;
static int	sound_sight;

void d_fire_baron (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void b_attack(edict_t *self);

void b_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


//STAND

void b_stand (edict_t *self);

mframe_t b_frames_stand [] =
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
	ai_stand, 0, NULL
};

mmove_t	b_move_stand = {FRAME_stand0, FRAME_stand19, b_frames_stand, NULL};

void b_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &b_move_stand;
}

//WALK

mframe_t b_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	
	ai_walk, 5, b_attack,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t b_move_walk = {FRAME_walk0, FRAME_walk19, b_frames_walk, NULL};

void b_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &b_move_walk;
}

//RUN
/*void b_run (edict_t *self);

mframe_t b_frames_run [] =
{
	ai_run, 10, b_attack,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
};

mmove_t b_move_run = {FRAME_run0, FRAME_run5, b_frames_run, b_run};

void b_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &b_move_walk;
	else
		self->monsterinfo.currentmove = &b_move_run;
}*/

//PAIN
mframe_t b_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
};
mmove_t b_move_pain = {FRAME_pain0, FRAME_pain2, b_frames_pain, b_walk};

void b_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	

	if(damage > 25)
	{
		self->monsterinfo.currentmove = &b_move_pain;
		self->pain_debounce_time = level.time + 3;
	}
}

//Death

void b_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t b_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -6, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL
};
mmove_t b_move_death = {FRAME_death0, FRAME_death8, b_frames_death, b_dead};


void b_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->monsterinfo.currentmove = &b_move_death;
}



//ATTACK

void b_fire(edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	int		dist;

	if(!self->enemy)
		return;

	dist = range (self, self->enemy) ;
	VectorSet (offset, 8, 12, 32);
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start); 
	VectorCopy (self->enemy->s.origin, vec);

	switch(dist)
	{
		case RANGE_MELEE:
		{
			vec[0] += self->enemy->velocity[0] * 0.2;  
			vec[1] += self->enemy->velocity[1] * 0.2;
			break;
		}
		case RANGE_NEAR:
		{
			vec[0] += self->enemy->velocity[0] * 0.3;  
			vec[1] += self->enemy->velocity[1] * 0.3; 
			break;
		}
		case RANGE_MID:
		{
			vec[0] += self->enemy->velocity[0] * 0.5;  
			vec[1] += self->enemy->velocity[1] * 0.5; 
			break;
		}
		case RANGE_FAR:
		default:
		{
			vec[0] += self->enemy->velocity[0];  
			vec[1] += self->enemy->velocity[1];
			break;
		}
	}

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("dmonstr/ffire.wav"), 1.0, ATTN_NORM, 0);

/*	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);*/
	d_fire_baron (self,start, dir, 40, 360);
}

mframe_t b_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, -1,NULL,
	ai_charge, -3,  b_fire,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL, //b_attack
};
mmove_t b_move_attack = {FRAME_attack0, FRAME_attack7, b_frames_attack, b_walk};


void b_attack(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if (infront(self,self->enemy) && visible(self,self->enemy))
		self->monsterinfo.currentmove = &b_move_attack;
	else
	{
		if(random() < 0.5)
			gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &b_move_walk;
	}
}


void SP_d_monster_baron (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
*/
	sound_pain =gi.soundindex ("dmonstr/baron/pain.wav");	
	sound_death=gi.soundindex ("dmonstr/baron/death.wav");	
	sound_sight=gi.soundindex ("dmonstr/baron/sight.wav");	
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/dmonster/baron/tris.md2");
		
	VectorSet (self->mins, -28, -28, -16);
	VectorSet (self->maxs, 28, 28, 64);

	self->health = 1100;
	self->gib_health = -80;
	self->mass = 600;
	self->pain = b_pain;
	self->die = b_die;

	self->flags |= (FL_D_MONSTER|FL_IMMUNE_SLIME);

	self->monsterinfo.stand = b_stand;
	self->monsterinfo.walk = b_walk;
	self->monsterinfo.run = b_walk; //b_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = b_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = b_sight;
	self->monsterinfo.search = b_stand;

	gi.linkentity (self);
	self->monsterinfo.currentmove = &b_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}



