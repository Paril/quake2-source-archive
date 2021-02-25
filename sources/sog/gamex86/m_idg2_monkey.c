#include "g_local.h"
#include "m_idg2_monkey.h"

static int  sound_dead1;
static int  sound_dead2;
static int	sound_pain;
static int  sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_melee;

void i_attack(edict_t *self);
void d_texplode (edict_t *self);   // from m_idg2_cu.c


static void d_fireball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	ent->s.modelindex = gi.modelindex ("sprites/dfexp.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = d_texplode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}

void d_fire_imp (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	toxicball->s.modelindex = gi.modelindex ("sprites/dfire.sp2");
	toxicball->owner = self;
	toxicball->touch = d_fireball_touch;
	toxicball->nextthink = level.time + 8000/speed;
	toxicball->think = G_FreeEdict;
	toxicball->dmg = damage;
	toxicball->s.sound = gi.soundindex ("idg2monster/monkey/attack2.wav");
	toxicball->classname = "fireball";
	gi.linkentity (toxicball);
}

static void i_idle_sound(edict_t *self)
{	
	if(random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void imp_search (edict_t *self)

{

	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM, 0);

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
	ai_stand, 0, i_idle_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	i_move_stand = {FRAME_stand1, FRAME_stand8, i_frames_stand, NULL};

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
	ai_walk, 5, NULL
};
mmove_t i_move_walk = {FRAME_walk1, FRAME_walk6, i_frames_walk, NULL};

void i_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &i_move_walk;
}

void i_run (edict_t *self);


//GET UP !!!
mframe_t i_frames_rise [] =
{
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
mmove_t i_move_rise = {FRAME_rise1, FRAME_rise9, i_frames_rise, i_run};
 
	

//RUN
mframe_t i_frames_run [] =
{
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL
};
mmove_t	i_move_run = {FRAME_walk1, FRAME_walk6, i_frames_run, NULL};

void i_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &i_move_walk;
	else
	{
		if (self->monsterinfo.currentmove == &i_move_stand)
			self->monsterinfo.currentmove = &i_move_rise;
		else
			self->monsterinfo.currentmove = &i_move_run;
	};
}



//PAIN
mframe_t i_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
};
mmove_t i_move_pain1 = {FRAME_paina1, FRAME_paina2, i_frames_pain1, i_run};

mframe_t i_frames_pain2 [] =
{
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, 2,  NULL,
	ai_move, 1,  NULL
};
mmove_t i_move_pain2 = {FRAME_painb1, FRAME_painb4, i_frames_pain2, i_run};


void i_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;
	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	
	if(random() > 0.5)
		self->monsterinfo.currentmove = &i_move_pain1;
	else
		self->monsterinfo.currentmove = &i_move_pain2;
	self->pain_debounce_time = level.time + 3;
}

//Death

void i_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, 0);
	VectorSet (self->maxs, 24, 24, 30);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t i_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, 0,  NULL
};
mmove_t i_move_death = {FRAME_die1, FRAME_die8, i_frames_death, i_dead};


void i_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	if(random() > 0.5)
		gi.sound (self, CHAN_VOICE, sound_dead1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_dead2, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
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

	if(!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,6,4,26);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg2monster/monkey/attack2.wav"), 1.0, ATTN_NORM, 0);
	d_fire_imp (self, start, dir, 20, 300);
}

mframe_t i_frames_attack1 [] =
{
	ai_charge, 1,NULL,
	ai_charge, -3, i_fire,
	ai_charge, 2, NULL
};
mmove_t i_move_attack1 = {FRAME_attack1, FRAME_attack3, i_frames_attack1,i_run};

static void i_rip (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (5 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_melee, 1, ATTN_NORM, 0);
}

mframe_t i_frames_attack2 [] =
{
	ai_charge, 1, NULL,
	ai_charge, -3,NULL,
	ai_charge, 2, NULL,
	ai_charge, 0, i_rip,
	ai_charge, 0, NULL
};
mmove_t i_move_attack2 = {FRAME_slash1, FRAME_slash5, i_frames_attack2, i_run};

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
	//	if (range (self, self->enemy) == RANGE_MELEE)
	//		self->monsterinfo.currentmove = &i_move_attack2;
		if(random() > 0.5)
			self->monsterinfo.currentmove = &i_move_attack1;
	/*}
	else
		self->monsterinfo.currentmove = &i_move_walk;*/
}

void i_attack2(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &i_move_attack2;
}

void SP_monster_idg2_monkey (edict_t *self)
{
	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 8;
		SP_monster_idg1_sa (self);
		return;
	}
	
	sound_dead1=gi.soundindex ("idg2monster/monkey/dead1.wav");	
	sound_dead2=gi.soundindex ("idg2monster/monkey/dead2.wav");	
	sound_pain =gi.soundindex ("idg2monster/monkey/pain.wav");	
	sound_idle =gi.soundindex ("idg2monster/monkey/idle.wav");	
	sound_sight1 =gi.soundindex ("idg2monster/monkey/sight1.wav");	
	sound_sight2 =gi.soundindex ("idg2monster/monkey/sight2.wav");	
	sound_melee =gi.soundindex ("idg2monster/monkey/attack1.wav");	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/monkey/tris.md2");
	VectorSet (self->mins, -20, -20, 0);
	VectorSet (self->maxs, 20, 20, 50);
	self->health = 40;
	self->gib_health = -40;
	self->mass = 80;

	self->pain = i_pain;
	self->die = i_die;

	//self->flags |= FL_D_MONSTER;

	self->monsterinfo.stand = i_stand;
	self->monsterinfo.walk = i_walk;
	self->monsterinfo.run = i_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = i_attack;
	self->monsterinfo.melee = i_attack2;
	self->monsterinfo.sight = i_sight;
	self->monsterinfo.search = imp_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &i_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}