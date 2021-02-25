/*
==============================================================================

WOLF SA

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_sa.h"

static int	sound_sight;
static int	sound_death1;
static int	sound_death2;
static int	sound_death3;
static int	sound_pain;
static int	sound_attack;


void sa_idle (edict_t *self)
{

// No idle-sound

}



// STAND

void sa_stand (edict_t *self);

mframe_t sa_frames_stand [] =
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
mmove_t sa_move_stand = {FRAME_stand01, FRAME_stand20, sa_frames_stand, sa_stand};

void sa_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &sa_move_stand;
}


//
// WALK
//

mframe_t sa_frames_walk [] =
{
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL
};
mmove_t sa_move_walk = {FRAME_roam01, FRAME_roam12, sa_frames_walk, NULL};

void sa_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &sa_move_walk;
}


//
// RUN
//

void sa_run (edict_t *self);

mframe_t sa_frames_run [] =
{
	ai_run, 8, NULL, 
	ai_run, 8, NULL,
	ai_run, 7, NULL,
	ai_run, 7, NULL,
	ai_run, 5,  NULL,
	ai_run, 7, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL,
	ai_run, 8, NULL, 
	ai_run, 8, NULL,
	ai_run, 8, NULL
};
mmove_t sa_move_run = {FRAME_run01, FRAME_run11, sa_frames_run, sa_run};

void sa_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &sa_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &sa_move_run;
}

//
// PAIN
//

mframe_t sa_frames_pain1 [] =
{
	ai_move, -2, NULL,
	ai_move, 3,  NULL,
	ai_move, 1,  NULL
};
mmove_t sa_move_pain1 = {FRAME_paina01, FRAME_paina03, sa_frames_pain1, sa_run};

mframe_t sa_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 13,   NULL,
};
mmove_t sa_move_pain2 = {FRAME_painb01, FRAME_painb03, sa_frames_pain2, sa_run};


void sa_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;
	
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;
	
	r = random();

	if (r < 0.5)
	{
		self->monsterinfo.currentmove = &sa_move_pain1;
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + 0.6;
	}
	else if (r > 0.5)
	{
		self->monsterinfo.currentmove = &sa_move_pain2;
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + 0.6;
	}
}


//
// ATTACK
//

void sa_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int damage;
		
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
	
	// Damage depends on distance
	
	switch (range (self, self->enemy))
	{
		case RANGE_MELEE:
			damage = 20;
			break;
		case RANGE_NEAR:
			damage = 12;
			break;
		case RANGE_MID:
			damage = 6;
			break;
		case RANGE_FAR:
			damage = 4;
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


void sa_attack_refire (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.2)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attack01;
	else
		self->monsterinfo.nextframe = FRAME_attack10;
}


mframe_t sa_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, sa_fire,
	ai_charge, 0, sa_attack_refire,
	ai_charge, 0, NULL
};
mmove_t sa_move_attack = {FRAME_attack01, FRAME_attack10, sa_frames_attack, sa_run};

void sa_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &sa_move_attack;
}


//
// SIGHT
//

void sa_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &sa_move_attack;
	}
}


//
// DEATH
//

static void dropammoclip (edict_t *self)
{
	edict_t	*backpack;

	backpack = Drop_Item(self, FindItemByClassname("ammo_idg1_bulletsS"));
	backpack->touch = Touch_Item;
}


void sa_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, -16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}


mframe_t sa_frames_death [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, dropammoclip,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL
};
mmove_t sa_move_death = {FRAME_death01, FRAME_death06, sa_frames_death, sa_dead};


void sa_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->s.skinnum = 2;
	self->monsterinfo.currentmove = &sa_move_death;

	n = rand() % 3;
	if (n == 0)
	{
		gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	}
	else if (n == 1)
	{
		gi.sound (self, CHAN_VOICE, sound_death2, 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_death3, 1, ATTN_NORM, 0);
	}


}


//
// SPAWN
//

/*QUAKED monster_idg1_sa (1 .5 0) (-16 -16 -32) (16 16 24) Ambush Trigger_Spawn Sight Corpse
Wolfenstein SA (Brown guard with pistol)
*/

void SP_monster_idg1_sa (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 32;
		SP_monster_idg2_dude (self);
		return;
	}

	sound_sight=	gi.soundindex ("idg1monster/sa/sight.wav");
	sound_pain= 	gi.soundindex ("idg1monster/sa/pain.wav");
	sound_death1=	gi.soundindex ("idg1monster/sa/death1.wav");
	sound_death2=	gi.soundindex ("idg1monster/sa/death2.wav");
	sound_death3=	gi.soundindex ("idg1monster/sa/death3.wav");
	sound_attack=   gi.soundindex ("idg1weapons/pshot.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg1/sa/tris.md2");
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 100;
	self->health = 35;
	self->gib_health = -55;

	self->pain = sa_pain;
	self->die = sa_die;

	self->monsterinfo.stand = sa_stand;
	self->monsterinfo.walk = sa_walk;
	self->monsterinfo.run = sa_run;
	self->monsterinfo.attack = sa_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = sa_sight;



	gi.linkentity (self);

	self->monsterinfo.currentmove = &sa_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	
	walkmonster_start (self);

	// Dead Guard
	
	if (self->spawnflags & 8)
	{
		self->health = -1;
		self->deadflag = DEAD_DEAD;
		self->takedamage = DAMAGE_YES;
		self->s.skinnum = 2;
		VectorSet (self->mins, -16, -16, -24);
		VectorSet (self->maxs, 16, 16, -8);
		self->movetype = MOVETYPE_TOSS;
		self->svflags |= SVF_DEADMONSTER;
		self->monsterinfo.currentmove = NULL;
		self->s.frame = FRAME_death06;
		gi.linkentity (self);
	}

}