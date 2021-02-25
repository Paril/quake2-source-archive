/*
==============================================================================

WOLF OFFICER

==============================================================================
*/

#include "g_local.h"
#include "m_idg1_officer.h"

static int	sound_sight;
static int	sound_death;
static int	sound_attack;


void officer_idle (edict_t *self)
{

// No idle-sound

}



// STAND

void officer_stand (edict_t *self);

mframe_t officer_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t officer_move_stand = {FRAME_stand1, FRAME_stand17, officer_frames_stand, officer_stand};

void officer_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &officer_move_stand;
}


//
// WALK
//

mframe_t officer_frames_walk [] =
{
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 2,  NULL
};
mmove_t officer_move_walk = {FRAME_roam1, FRAME_roam7, officer_frames_walk, NULL};

void officer_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &officer_move_walk;
}


//
// RUN
//

void officer_run (edict_t *self);

mframe_t officer_frames_run [] =
{
	ai_run, 11, NULL, 
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};
mmove_t officer_move_run = {FRAME_run1, FRAME_run4, officer_frames_run, officer_run};

void officer_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &officer_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &officer_move_run;
}

//
// PAIN
//

mframe_t officer_frames_pain [] =
{
	ai_move, -2, NULL,
	ai_move, 2,  NULL,
};
mmove_t officer_move_pain = {FRAME_pain1, FRAME_pain2, officer_frames_pain, officer_run};


void officer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->monsterinfo.currentmove = &officer_move_pain;
	self->pain_debounce_time = level.time + 0.6;
}


//
// ATTACK
//

void officer_fire (edict_t *self)
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


void officer_attack_refire (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.2)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attack1;
	else
		self->monsterinfo.nextframe = FRAME_attack7;
}


mframe_t officer_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, officer_fire,
	ai_charge, 0, officer_attack_refire,
	ai_charge, 0, NULL
};
mmove_t officer_move_attack = {FRAME_attack1, FRAME_attack7, officer_frames_attack, officer_run};

void officer_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &officer_move_attack;
}


//
// SIGHT
//

void officer_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &officer_move_attack;
	}
}


//
// DEATH
//

static void officer_dropammoclip (edict_t *self)
{
	edict_t	*backpack;

	backpack = Drop_Item(self, FindItemByClassname("ammo_idg1_bulletsS"));
	backpack->touch = Touch_Item;
}


void officer_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -31);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t officer_frames_death [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, officer_dropammoclip,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t officer_move_death = {FRAME_death1, FRAME_death7, officer_frames_death, officer_dead};


void officer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->monsterinfo.currentmove = &officer_move_death;
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

}


//
// SPAWN
//

void SP_monster_idg1_officer (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		self->s.origin[2] -= 32;
		SP_monster_idg2_sarge (self);
		return;
	}

	sound_sight=	gi.soundindex ("idg1monster/officer/sight.wav");
	sound_death=	gi.soundindex ("idg1monster/officer/death.wav");
	sound_attack=   gi.soundindex ("idg1weapons/pshot.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg1/officer/tris.md2");
	VectorSet (self->mins, -16, -16, -31);
	VectorSet (self->maxs, 16, 16, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 100;
	self->health = 60;
	self->gib_health = -55;

	self->pain = officer_pain;
	self->die = officer_die;

	self->monsterinfo.stand = officer_stand;
	self->monsterinfo.walk = officer_walk;
	self->monsterinfo.run = officer_run;
	self->monsterinfo.attack = officer_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = officer_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &officer_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}