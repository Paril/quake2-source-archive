#include "g_local.h"
#include "m_gendimp.h"

static int  sound_dead1;
static int  sound_dead2;
static int	sound_pain;
static int  sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_melee;

void i_attack(edict_t *self);
void d_fire_imp (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);


static void i_idle_sound(edict_t *self)
{	
	if(random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

static void i_sight (edict_t *self, edict_t *other)
{
	if(random() < 0.4)
		gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);
}

//STAND

void i_stand (edict_t *self);
mframe_t i_frames_stand [] =
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
mmove_t	i_move_stand = {FRAME_stand0, FRAME_stand19, i_frames_stand, NULL};

void i_stand (edict_t *self)
{	self->monsterinfo.currentmove = &i_move_stand;
}


//WALK
mframe_t i_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, i_attack,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, i_idle_sound,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, i_attack,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, i_attack
};
mmove_t i_move_walk = {FRAME_walk0, FRAME_walk19, i_frames_walk, NULL};

void i_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &i_move_walk;
}

//RUN
void i_run (edict_t *self);
mframe_t i_frames_run [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,

	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,

	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,

	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t	i_move_run = {FRAME_stand0, FRAME_stand19, i_frames_run, i_run};

void i_run (edict_t *self)
{	self->monsterinfo.currentmove = &i_move_stand;
}

/*
void i_run (edict_t *self);

mframe_t i_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, i_attack
};

mmove_t i_move_run = {FRAME_run0, FRAME_run5, i_frames_run, i_run};

void i_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &i_move_walk;
	else
		self->monsterinfo.currentmove = &i_move_run;
}
*/

//PAIN
mframe_t i_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t i_move_pain = {FRAME_pain0, FRAME_pain3, i_frames_pain, i_walk};

void i_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;
	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
	self->monsterinfo.currentmove = &i_move_pain;
	self->pain_debounce_time = level.time + 2;
}

//Death

void i_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -16);
	VectorSet (self->maxs, 24, 24, -4);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t i_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -6, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,

	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t i_move_death = {FRAME_die0, FRAME_die9, i_frames_death, i_dead};


void i_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	if(random() > 0.5)
		gi.sound (self, CHAN_VOICE, sound_dead1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_dead2, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->monsterinfo.currentmove = &i_move_death;
}



//ATTACK

void i_fire(edict_t *self)
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

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,6,4,2);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);

	switch(dist)
	{
		case RANGE_MELEE:
		{
			vec[0] += self->enemy->velocity[0] * 0.15;  
			vec[1] += self->enemy->velocity[1] * 0.15;
			break;
		}
		case RANGE_NEAR:
		{
			vec[0] += self->enemy->velocity[0] * 0.28;  
			vec[1] += self->enemy->velocity[1] * 0.28; 
			break;
		}
		case RANGE_MID:
		{
			vec[0] += self->enemy->velocity[0] * 0.45;  
			vec[1] += self->enemy->velocity[1] * 0.45; 
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
	vec[2] = self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("dweap/rocket.wav"), 1.0, ATTN_NORM, 0);
	d_fire_imp (self, start, dir, 20, 520);
}

mframe_t i_frames_attack1 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 1,NULL,
	ai_charge, -3, i_fire,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 0, NULL,
	ai_charge, 2, i_attack
};
mmove_t i_move_attack1 = {FRAME_attack0, FRAME_attack6, i_frames_attack1,i_walk};

static void i_rip (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (15 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_melee, 1, ATTN_NORM, 0);
}

mframe_t i_frames_attack2 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 1,NULL,
	ai_charge, -3, i_rip,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 0, NULL,
	ai_charge, 2, i_attack
};
mmove_t i_move_attack2 = {FRAME_attack0, FRAME_attack6, i_frames_attack2,i_walk};


void i_attack(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	//if (infront(self,self->enemy) && visible(self,self->enemy))
	//{
		if (range (self, self->enemy) == RANGE_MELEE)
			self->monsterinfo.currentmove = &i_move_attack2;
		else if(random() > 0.5)
			self->monsterinfo.currentmove = &i_move_attack1;
		else
			self->monsterinfo.currentmove = &i_move_walk;
	/*}
	else
		self->monsterinfo.currentmove = &i_move_walk;*/
}



void SP_d_monster_imp (edict_t *self)
{
	sound_dead1=gi.soundindex ("dmonstr/imp/dead1.wav");	
	sound_dead2=gi.soundindex ("dmonstr/imp/dead2.wav");	
	sound_pain =gi.soundindex ("dmonstr/spain.wav");	
	sound_idle =gi.soundindex ("dmonstr/imp/idle.wav");	
	sound_sight1 =gi.soundindex ("dmonstr/imp/sight1.wav");	
	sound_sight2 =gi.soundindex ("dmonstr/imp/sight2.wav");	
	sound_melee =gi.soundindex ("dmonstr/imp/melee.wav");	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/dmonster/imp/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 24);
	self->health = 40;
	self->gib_health = -40;
	self->mass = 80;

	self->pain = i_pain;
	self->die = i_die;

	self->flags |= FL_D_MONSTER;

	self->monsterinfo.stand = i_stand;
	self->monsterinfo.walk = i_walk;
	self->monsterinfo.run = i_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = i_attack;
	self->monsterinfo.melee = i_rip;
	self->monsterinfo.sight = i_sight;
	self->monsterinfo.search = i_stand;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &i_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}