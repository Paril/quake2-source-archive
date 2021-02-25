/*
==============================================================================

CHTON !!!!!! HE LIVES !

==============================================================================
*/

#include "g_local.h"
#include "m_genq1chton.h"

void q1_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

static int	sound_pain;
static int	sound_death;
static int	sound_sight;
static int  sound_throw;
static int  sound_rise;


static void chton_rise_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_rise, 1, ATTN_NONE, 0);
	//gi.dprintf("RISE\n");
}

static void chton_sight_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

static void chton_sight_sound2 (edict_t *self)
{
	if(random() < 0.1)
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


void chton_stand (edict_t *self);
void chton_rise (edict_t *self);
static void chton_check_attack(edict_t *self);



mframe_t chton_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_sight_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chton_check_attack,
	ai_stand, 0, NULL
};
mmove_t chton_move_stand = {FRAME_walk1, FRAME_walk31, chton_frames_stand, NULL};


void chton_stand(edict_t *self)
{
	self->monsterinfo.currentmove = &chton_move_stand;
}

void chton_frameend(edict_t *self)
{//	gi.dprintf("RISE:%d\n",self->s.frame);
}
void chton_framemid(edict_t *self)
{//	gi.dprintf("RISE:%d\n",self->s.frame);
}
void chton_framestart(edict_t *self)
{//	gi.dprintf("RISE:%d\n",self->s.frame);
}


mframe_t chton_frames_rise [] =
{
	ai_move, 0, chton_rise_sound,
	ai_move, 0, NULL,
	ai_move, 0, chton_sight_sound,
	ai_move, 0, chton_framestart,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, chton_framemid,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, chton_frameend,
	ai_move, 0, chton_stand   
};

mmove_t	chton_move_rise = {FRAME_rise1, FRAME_rise17, chton_frames_rise, NULL}; //chton_rise

void chton_rise (edict_t *self)
{
//	gi.dprintf("RISE CHTON !\n");
	self->monsterinfo.currentmove = &chton_move_rise;
}



mframe_t chton_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chton_sight_sound2,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chton_check_attack,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chton_check_attack
};
mmove_t chton_move_walk = {FRAME_walk1, FRAME_walk31, chton_frames_walk, NULL};

void chton_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &chton_move_walk;
}

void chton_run (edict_t *self);
mframe_t chton_frames_run [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chton_sight_sound2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chton_check_attack,
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
	ai_charge, 0, chton_check_attack,
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
	ai_charge, 0, chton_check_attack
};
mmove_t chton_move_run = {FRAME_walk1, FRAME_walk31, chton_frames_walk, chton_run};

void chton_run (edict_t *self)
{
	self->monsterinfo.currentmove = &chton_move_run;
}


mframe_t chton_frames_shock1 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 chton_check_attack
};
mmove_t chton_frames_move_shock1 = {FRAME_shocka1, FRAME_shocka10, chton_frames_shock1, chton_walk};


mframe_t chton_frames_shock2 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  chton_check_attack
};
mmove_t chton_frames_move_shock2 = {FRAME_shockb1, FRAME_shockb6, chton_frames_shock2, chton_walk};

mframe_t chton_frames_shock3 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 chton_check_attack
};
mmove_t chton_frames_move_shock3 = {FRAME_shockc1, FRAME_shockc10, chton_frames_shock3, chton_walk};


void chton_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

/*	if(stricmp(level.mapname,"qe1m7")==0)
	if(stricmp(other->classname,"target_q1_bolt")==0)
	{
		gi.dprintf("HIT BY BOLT");
		self->pain_debounce_time = level.time + 5;
	}
*/
	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	

	if(damage > 25)
	{
		if (self->health < 1500)
		{
			self->monsterinfo.currentmove = &chton_frames_move_shock1;
			self->pain_debounce_time = level.time + 6;
		
		}
		else if (self->health < 1000)
		{
			self->monsterinfo.currentmove = &chton_frames_move_shock2;
			self->pain_debounce_time = level.time + 6;
		}
		else if (self->health < 500)
		{
			self->monsterinfo.currentmove = &chton_frames_move_shock3;
			self->pain_debounce_time = level.time + 6;
		}
	}
}


void chton_dead (edict_t *self)
{
	VectorSet (self->mins, -64, -64, -64);
	VectorSet (self->maxs, 64, 64, 64);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t chton_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t chton_move_death = {FRAME_death1, FRAME_death9, chton_frames_death, chton_dead};


void chton_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
		ThrowHead (self, "models/objects/q1gibs/q1gib2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	self->monsterinfo.currentmove = &chton_move_death;
}

//void door_use (edict_t *self, edict_t *other, edict_t *activator);;
void chton_bolt(edict_t *self, int no)
{
	switch(no)
	{
	case 1:

		gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
		self->monsterinfo.currentmove = &chton_frames_move_shock1;
		self->pain_debounce_time = level.time + 6;
		break;
	case 2:
		gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
		self->monsterinfo.currentmove = &chton_frames_move_shock2;
		self->pain_debounce_time = level.time + 6;
		break;
	case 3:
		gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
		self->monsterinfo.currentmove = &chton_frames_move_shock3;
		self->pain_debounce_time = level.time + 6;
		self->health = -50;
		gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
		self->deadflag = DEAD_DEAD;
		self->takedamage = DAMAGE_NO;
		self->monsterinfo.currentmove = &chton_move_death;
		monster_death_use(self);
		break;
	}
}


void chton_attack_left (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	int		dist;
	qboolean high=false;

	if(!self->enemy)
		return;

	dist = range (self, self->enemy) ;

	AngleVectors (self->s.angles, forward, right, NULL);
	//gi.dprintf("ENEMY:%f\nSELF:%f\nDIFF:%f\n",self->enemy->s.origin[2],self->s.origin[2],
//		self->enemy->s.origin[2] - (self->s.origin[2] + 50.00));	
	
	if((self->enemy->s.origin[2] - (self->s.origin[2] + 50.00)) > 100)
		high=true;

	VectorSet (offset, 36, 160, 200);
	G_ProjectSource (self->s.origin, offset , forward, right, start); //monster_flash_offset[MZ2_BOSS2_ROCKET_1]
	VectorCopy (self->enemy->s.origin, vec);

	switch(dist)
	{
		case RANGE_MELEE:
		{
			vec[0] += self->enemy->velocity[0] * 0.12;  
			vec[1] += self->enemy->velocity[1] * 0.12;
			if(!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_NEAR:
		{
			vec[0] += self->enemy->velocity[0] * 0.25;  
			vec[1] += self->enemy->velocity[1] * 0.25; 
			if(!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_MID:
		{
			vec[0] += self->enemy->velocity[0] * 0.4;  
			vec[1] += self->enemy->velocity[1] * 0.4; 
			if(!high)
				vec[2] -= (self->enemy->viewheight*(4 * random()));
			else
				vec[2] += self->enemy->viewheight;

			break;
		}
		case RANGE_FAR:
		default:
		{
			vec[0] += self->enemy->velocity[0];  
			vec[1] += self->enemy->velocity[1];
			if(!high)
				vec[2] -= (self->enemy->viewheight*(2 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
	}


	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	q1_fire_lavaball (self,start, dir, 100, 750, 120, 100);
}	

void chton_attack_right (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	int		dist;
	qboolean high=false;
	
	if(!self->enemy)
		return;

	dist = range (self, self->enemy);
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet (offset, 36, -160, 200);
	G_ProjectSource (self->s.origin,offset, forward, right, start);//monster_flash_offset[MZ2_BOSS2_ROCKET_4]

	if((self->enemy->s.origin[2] - (self->s.origin[2] + 50.00)) > 100)
		high=true;

	VectorCopy (self->enemy->s.origin, vec);
	
	switch(dist)
	{
		case RANGE_MELEE:
		{
			vec[0] += self->enemy->velocity[0] * 0.12;  
			vec[1] += self->enemy->velocity[1] * 0.12;
			if(!high)
			vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_NEAR:
		{
			vec[0] += self->enemy->velocity[0] * 0.25;  
			vec[1] += self->enemy->velocity[1] * 0.25; 
			if(!high)
			vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_MID:
		{
			vec[0] += self->enemy->velocity[0] * 0.4;  
			vec[1] += self->enemy->velocity[1] * 0.4; 
			if(!high)
				vec[2] -= (self->enemy->viewheight*(4 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
		case RANGE_FAR:
		default:
		{
			vec[0] += self->enemy->velocity[0];  
			vec[1] += self->enemy->velocity[1];
			if(!high)
				vec[2] -= (self->enemy->viewheight*(2 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
	}

	
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_4);
	gi.multicast (start, MULTICAST_PVS);

	q1_fire_lavaball (self,start, dir, 100, 750, 120, 100);
}	


mframe_t chton_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, chton_sight_sound2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chton_attack_left,
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
	ai_charge, 0, chton_attack_right,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chton_check_attack
};
mmove_t chton_move_attack = {FRAME_attack1, FRAME_attack23, chton_frames_attack, chton_walk};

void chton_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &chton_move_attack;
}

void chton_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_IDLE, 0);

/*	if(!self->enemy)
		gi.dprintf("CHTON SEES FIRST ENEMY\n");
	else
		gi.dprintf("CHTON SEES OLD ENEMY\n");
*/
	//	self->monsterinfo.currentmove = &chton_move_rise;
	//chton_attack(self);
}

static void chton_check_attack (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;
	self->monsterinfo.currentmove = &chton_move_attack;
}


qboolean chton_checkattack (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return false;

	else if (visible(self,self->enemy))
		return true;
}


void SP_q1_monster_chton (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain =gi.soundindex ("q1monstr/boss/pain.wav");	
	sound_death=gi.soundindex ("q1monstr/boss/death.wav");	
	sound_rise =gi.soundindex ("q1monstr/boss/out1.wav");	
	sound_throw =gi.soundindex ("q1monstr/boss/throw.wav");	
	sound_sight  =gi.soundindex ("q1monstr/boss/sight1.wav");	
	
	if(strcmp(level.mapname,"qe1m7")==0)
		self->movetype = MOVETYPE_NONE;
	else
		self->movetype = MOVETYPE_STEP;

	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/boss/tris.md2");
	
	VectorSet (self->mins, -128, -128, -24);
	VectorSet (self->maxs, 128, 128, 226);

	if(skill->value ==0)
		self->health = 3000;
	else if(skill->value ==1)
		self->health = 4000;
	else
		self->health = 5000;

	self->gib_health = -100;
	self->mass = 1500;
	self->style = 0;

	self->flags |= (FL_Q1_MONSTER|FL_IMMUNE_LAVA);

	self->pain = chton_pain;
	self->die = chton_die;

	self->monsterinfo.stand = chton_stand; 
	self->monsterinfo.walk = chton_walk;
	self->monsterinfo.run = chton_run; 
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = chton_attack;
	self->monsterinfo.melee = NULL; 
	self->monsterinfo.sight = chton_sight;
	self->monsterinfo.search = NULL;
	self->monsterinfo.checkattack = chton_checkattack;

	self->s.renderfx |= RF_FULLBRIGHT;
	
	gi.linkentity (self);

	self->monsterinfo.scale = MODEL_SCALE;
	self->monsterinfo.currentmove = &chton_move_rise;

	walkmonster_start (self);
	
}



