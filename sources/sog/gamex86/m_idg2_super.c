
#include "g_local.h"
#include "m_idg2_super.h"

static int  sound_death;
static int	sound_sight;
static int  sound_walk1;
static int  sound_walk2;

void BossExplode (edict_t *self);    // from m_supertank.c

void cd_attack(edict_t *self);

static void cd_walk1_sound(edict_t *self)
{	gi.sound (self, CHAN_VOICE, sound_walk1, 1, ATTN_IDLE, 0);
}

static void cd_walk2_sound(edict_t *self)
{	gi.sound (self, CHAN_VOICE, sound_walk2, 1, ATTN_IDLE, 0);
}

static void cd_idle_sound(edict_t *self)
{	
	if(random() < 0.1)
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_IDLE, 0);
}

static void cd_sight_sound(edict_t *self)
{	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


//STAND

void cd_stand (edict_t *self);

mframe_t cd_frames_stand [] =
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
	ai_stand, 0, cd_idle_sound,
	ai_stand, 0, NULL
};

mmove_t	cd_move_stand = {FRAME_stand02, FRAME_stand40, cd_frames_stand, NULL};

void cd_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &cd_move_stand;
}

//WALK

mframe_t cd_frames_walk [] =
{
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, cd_attack,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, cd_walk2_sound,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, cd_attack,
	ai_walk, 8, cd_walk1_sound,
};
mmove_t cd_move_walk = {FRAME_walk01, FRAME_walk12, cd_frames_walk, NULL};

void cd_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &cd_move_walk;
}

//RUN
void cd_run (edict_t *self);

mframe_t cd_frames_run [] =
{
	ai_charge, 18, NULL,
	ai_charge, 24, NULL,
	ai_charge, 28, cd_walk2_sound,
	ai_charge, 18, NULL,
	ai_charge, 24, NULL,
	ai_charge, 28, cd_walk1_sound
};

mmove_t cd_move_run = {FRAME_run01, FRAME_run06, cd_frames_run, cd_attack};

void cd_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &cd_move_walk;
	else
		self->monsterinfo.currentmove = &cd_move_run;
}


/*
======================================

======================================
*/
mframe_t cd_frames_look [] =
{
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, cd_walk2_sound,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, cd_walk1_sound,
};
mmove_t cd_move_look = {FRAME_walk01, FRAME_walk12, cd_frames_look, cd_attack};

void cd_look (edict_t *self)
{
	self->monsterinfo.currentmove = &cd_move_look;
}





//SALUTE- After he kills you

mframe_t cd_frames_salute [] = 
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, cd_sight_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t cd_move_salute = {FRAME_salute01,FRAME_salute11,cd_frames_salute,cd_walk};

void cyberdemon_salute (edict_t *self)
{
	self->monsterinfo.currentmove = &cd_move_salute;
}

//Taunt - When he first sees you
mframe_t cd_frames_taunt [] = 
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
};
mmove_t cd_move_taunt = {FRAME_taunt01,FRAME_taunt17,cd_frames_taunt,cd_walk};

void cd_sight (edict_t *self, edict_t *other)
{
	cd_sight_sound(self);
	self->monsterinfo.currentmove = &cd_move_taunt;
	//cd_attack(self);
}

//WAVE- when u try to run away

mframe_t cd_frames_wave [] = 
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, cd_sight_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t cd_move_wave = {FRAME_wave01,FRAME_wave11,cd_frames_wave,cd_walk};

//cant find uses for Flip and Point


//PAIN
mframe_t cd_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t cd_move_pain1 = {FRAME_pain101, FRAME_pain104, cd_frames_pain1, cd_walk};

mframe_t cd_frames_pain2 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t cd_move_pain2 = {FRAME_pain201, FRAME_pain204, cd_frames_pain2, cd_walk};

mframe_t cd_frames_pain3 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t cd_move_pain3 = {FRAME_pain301, FRAME_pain304, cd_frames_pain3, cd_walk};

void cd_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	gi.sound (self, CHAN_VOICE, sound_sight, 1.0, ATTN_NORM, 0);	

	if(damage > 30)
	{
		if (self->health < 3000)
		{
			self->monsterinfo.currentmove = &cd_move_pain1;
			self->pain_debounce_time = level.time + 6;
		
		}
		else if (self->health < 2000)
		{
			self->monsterinfo.currentmove = &cd_move_pain2;
			self->pain_debounce_time = level.time + 6;
		}
		else if (self->health < 1000)
		{
			self->monsterinfo.currentmove = &cd_move_pain3;
			self->pain_debounce_time = level.time + 6;
		}
	}
}

//Death

mframe_t cd_frames_death1 [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -6, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,
	ai_move, 0, BossExplode
};
mmove_t cd_move_death1 = {FRAME_death101, FRAME_death106, cd_frames_death1, NULL};

mframe_t cd_frames_death2 [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 6, NULL,
	ai_move, 3, NULL,
	ai_move, 1, NULL,
	ai_move, 0, BossExplode
};
mmove_t cd_move_death2 = {FRAME_death201, FRAME_death206, cd_frames_death2, NULL};

mframe_t cd_frames_death3 [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, -4, NULL,
	ai_move, 4, NULL,
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, 0, NULL,
	ai_move, 0, BossExplode
};
mmove_t cd_move_death3 = {FRAME_death301, FRAME_death308, cd_frames_death3, NULL};

void cd_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float r = random();

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	if(r > 0.66)	
		self->monsterinfo.currentmove = &cd_move_death1;
	else if(r > 0.33)
		self->monsterinfo.currentmove = &cd_move_death2;
	else
		self->monsterinfo.currentmove = &cd_move_death3;
}


/*
======================================
Check firing func
======================================
*/


//ATTACK

void cd_fire(edict_t *self)
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
	VectorSet(offset,8,-36,50);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);


	tr = gi.trace (self->s.origin, 0, 0, start, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
//gi.dprintf("CYBER: CANNOT FIRE WALL IN BETWEEN\n");
		self->delay = level.time + 1.0;
		cd_walk(self);
		return;
	}

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

	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg2weapons/rocket.wav"), 1.0, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	d_fire_rocket (self,start, dir, 100, 620, 100, 100);
	self->style++;
}

mframe_t cd_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, -5, cd_fire,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL 
};
mmove_t cd_move_attack = {FRAME_attack01, FRAME_attack08, cd_frames_attack, cd_walk}; 


void cd_attack(edict_t *self)
{
	static  vec3_t	v;
	static  float	len;
	float r = random();
	trace_t tr;
	vec3_t spot1,spot2;

	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(self->style <3)
	{
		self->wait = level.time + 3.0;
		self->monsterinfo.currentmove = &cd_move_attack;
		return;
	}

	
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy (self->enemy->s.origin, spot2);
	spot2[2] += self->enemy->viewheight;
	tr = gi.trace (spot1, 0, 0, spot2, self, MASK_SHOT);

	if ((tr.ent->client) &&
		(tr.ent == self->enemy) &&
		infront(self,self->enemy) && 
		visible(self,self->enemy) &&
		self->delay < level.time  &&
		self->wait < level.time &&
		len < 512)
	{
		self->style = 0;
		self->monsterinfo.currentmove = &cd_move_attack;
		return;
	}
	else if(len < 512 || (random() < 0.3))
	{
		self->monsterinfo.currentmove = &cd_move_look;
	}
	else
	{
		if (tr.ent != self->enemy || !tr.ent->client)
		{
			self->monsterinfo.currentmove = &cd_move_look;
			return;
		}
		self->monsterinfo.currentmove = &cd_move_run;
	}
	
}


void SP_monster_idg2_super (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
*/
	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 8;
		SP_monster_idg1_commander (self);
		return;
	}

	sound_death =gi.soundindex ("idg2monster/super/death.wav");	
	sound_sight=gi.soundindex ("idg2monster/super/sight.wav");	
	sound_walk1 =gi.soundindex ("boss3/step1.wav");	
	sound_walk2=gi.soundindex ("boss3/step2.wav");	

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/super/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/idg2/super/weapon.md2");
	
	VectorSet (self->mins, -40, -40, -24);
	VectorSet (self->maxs, 40, 40, 92);

	self->health = 4000;
	self->gib_health = -666;
	self->mass = 800;
	self->style = 0;

	self->pain = cd_pain;
	self->die = cd_die;

	self->flags |= (FL_D_MONSTER|FL_IMMUNE_LAVA|FL_IMMUNE_SLIME|FL_NO_KNOCKBACK);

	self->monsterinfo.stand = cd_stand;
	self->monsterinfo.walk = cd_walk;
	self->monsterinfo.run = cd_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = cd_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = cd_sight;
	self->monsterinfo.search = cd_look; //stand;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &cd_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}



