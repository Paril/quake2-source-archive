
#include "g_local.h"
#include "m_idg2_cu.h"

static int  sound_death;
static int	sound_pain;
static int	sound_sight;

void b_attack(edict_t *self);

void d_texplode (edict_t *self)
{
	if (self->s.frame == 2)
	{
		G_FreeEdict (self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

static void d_toxicball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("idg2monstr/cu/hit.wav"), 1.0, ATTN_NORM, 0);	

	// explosion sprite 
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


void d_fire_baron (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	toxicball->s.effects |= (EF_ANIM_ALLFAST); //EF_BFG
	toxicball->s.renderfx |= RF_TRANSLUCENT;

	VectorClear (toxicball->mins);
	VectorClear (toxicball->maxs);
	toxicball->s.modelindex = gi.modelindex ("sprites/dtoxic.sp2");
	toxicball->owner = self;
	toxicball->touch = d_toxicball_touch;
	toxicball->nextthink = level.time + 8000/speed;
	toxicball->think = G_FreeEdict;
	toxicball->dmg = damage;
	toxicball->s.sound = gi.soundindex ("idg2monster/cu/attack2.wav");
	toxicball->classname = "toxicball";

	gi.linkentity (toxicball);
}

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
	
	ai_walk, 5, NULL,
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
void b_run (edict_t *self);

mframe_t b_frames_run [] =
{
	ai_run, 10, b_attack,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
};

mmove_t b_move_run = {FRAME_run0, FRAME_run5, b_frames_run, NULL};

void b_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &b_move_walk;
	else
		self->monsterinfo.currentmove = &b_move_run;
}

//PAIN
mframe_t b_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
};
mmove_t b_move_pain = {FRAME_pain0, FRAME_pain2, b_frames_pain, b_run};

void b_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;


	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

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
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_NORM, 0);
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
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->s.skinnum = 2;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
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

	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg2monster/cu/attack2.wav"), 1.0, ATTN_NORM, 0);

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
mmove_t b_move_attack = {FRAME_attack0, FRAME_attack7, b_frames_attack, b_run};


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
		self->monsterinfo.currentmove = &b_move_run;
	}
}


void SP_monster_idg2_cu (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
*/

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 11;
		SP_monster_idg1_bd19 (self);
		return;
	}
	
	sound_pain =gi.soundindex ("idg2monster/cu/pain.wav");	
	sound_death=gi.soundindex ("idg2monster/cu/death.wav");	
	sound_sight=gi.soundindex ("idg2monster/cu/sight.wav");	
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/cu/tris.md2");
		
	VectorSet (self->mins, -28, -28, -16);
	VectorSet (self->maxs, 28, 28, 64);

	self->health = 600;
	self->gib_health = -80;
	self->mass = 600;
	self->pain = b_pain;
	self->die = b_die;

	self->flags |= (FL_D_MONSTER|FL_IMMUNE_SLIME);

	self->monsterinfo.stand = b_stand;
	self->monsterinfo.walk = b_walk;
	self->monsterinfo.run = b_run;
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



