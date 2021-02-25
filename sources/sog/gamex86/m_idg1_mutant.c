//
// WOLF Mutant
//

#include "g_local.h"
#include "m_idg1_mutant.h"

static int  sound_dead;
static int  sound_melee;
static int  sound_attack;


static void idg1mutant_sight (edict_t *self, edict_t *other)
{
	//No sight sound (no warning !!!)
}

static void idg1mutant_search (edict_t *self)
{
}

//STAND

void idg1mutant_stand (edict_t *self);
mframe_t idg1mutant_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	idg1mutant_move_stand = {FRAME_stand1, FRAME_stand6, idg1mutant_frames_stand, NULL};

void idg1mutant_stand (edict_t *self)
{	self->monsterinfo.currentmove = &idg1mutant_move_stand;
}


//WALK
mframe_t idg1mutant_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t idg1mutant_move_walk = {FRAME_run1, FRAME_run4, idg1mutant_frames_walk, NULL};

void idg1mutant_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg1mutant_move_walk;
}



//RUN

void idg1mutant_run (edict_t *self);

mframe_t idg1mutant_frames_run [] =
{
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL,
	ai_run, 9, NULL
};
mmove_t	idg1mutant_move_run = {FRAME_run1, FRAME_run4, idg1mutant_frames_run, NULL};

void idg1mutant_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg1mutant_move_stand;
	else
		self->monsterinfo.currentmove = &idg1mutant_move_run;
}



//PAIN

void idg1mutant_pain (edict_t *self, edict_t *other, float kick, int damage)
{
// Never in pain
	self->monsterinfo.currentmove = &idg1mutant_move_run;
}

//Death

void idg1mutant_dead (edict_t *self)
{
	VectorSet (self->mins, -24, -24, -30);
	VectorSet (self->maxs, 24, 24, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t idg1mutant_frames_death [] =
{
	ai_move, -1, NULL,
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, 0,  NULL

};
mmove_t idg1mutant_move_death = {FRAME_death1, FRAME_death6, idg1mutant_frames_death, idg1mutant_dead};


void idg1mutant_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg1/gib.wav"), 1, ATTN_NORM, 0);
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
	gi.sound (self, CHAN_VOICE, sound_dead, 1, ATTN_NORM, 0);
	// self->s.skinnum = 1;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &idg1mutant_move_death;
}



//ATTACK

void idg1mutant_fire(edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int		damage;
		
	if(!self->enemy)
		return;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_SOLDIER_SHOTGUN_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*1000;
	u = crandom()*500;
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	switch (range (self, self->enemy))
	{
		case RANGE_MELEE:
			damage = 28;
			break;
		case RANGE_NEAR:
			damage = 16;
			break;
		case RANGE_MID:
			damage = 8;
			break;
		case RANGE_FAR:
			damage = 6;
			break;
		default:
			damage = 0;
			break;
	}

	// Damage also depends on skill
/*	
	if (skill->value == 1)
		damage = round(damage * 1.1);
	else if (skill->value == 2)
		damage = round(damage * 1.3);
	else if (skill->value == 3)
		damage = round(damage * 1.5);
*/
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack, 1, ATTN_NORM, 0);
	monster_fire_bullet (self, start, aim, damage, 1, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 1);
}

mframe_t idg1mutant_frames_attack1 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1mutant_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1mutant_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1mutant_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg1mutant_move_attack1 = {FRAME_fire1, FRAME_fire9, idg1mutant_frames_attack1, idg1mutant_run};

static void idg1mutant_slash (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (10 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_melee, 1, ATTN_NORM, 0);
}

mframe_t idg1mutant_frames_attack2 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1mutant_slash,
	ai_charge, 0, NULL,
	ai_charge, 0, idg1mutant_slash,
	ai_charge, 0, NULL
};
mmove_t idg1mutant_move_attack2 = {FRAME_slash1, FRAME_slash6, idg1mutant_frames_attack2, idg1mutant_run};

void idg1mutant_attack(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if(random() > 0.3)
		self->monsterinfo.currentmove = &idg1mutant_move_attack1;
}

void idg1mutant_attack2(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &idg1mutant_move_attack2;
}

void SP_monster_idg1_mutant (edict_t *self)
{
	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 8;
		SP_monster_idg2_fatdude (self);
		return;
	}
	
	sound_dead=gi.soundindex ("idg1monster/mutant/death.wav");		
	sound_melee =gi.soundindex ("idg1weapons/knifeair.wav");	
	sound_attack =gi.soundindex ("idg1weapons/pshot.wav");		

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg1/mutant/tris.md2");
	VectorSet (self->mins, -20, -20, -30);
	VectorSet (self->maxs, 20, 20, 20);
	self->health = 60;
	self->gib_health = -40;
	self->mass = 80;

	self->pain = idg1mutant_pain;
	self->die = idg1mutant_die;

	self->monsterinfo.stand = idg1mutant_stand;
	self->monsterinfo.walk = idg1mutant_walk;
	self->monsterinfo.run = idg1mutant_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg1mutant_attack;
	self->monsterinfo.melee = idg1mutant_attack2;
	self->monsterinfo.sight = idg1mutant_sight;
	self->monsterinfo.search = idg1mutant_search;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg1mutant_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}