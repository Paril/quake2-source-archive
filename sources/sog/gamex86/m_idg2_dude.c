/*
==============================================================================

DUDE  ( DOOM ZOMBIE )

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_dude.h"

static int	sound_sight;
static int	sound_death;
static int	sound_pain;
static int  sound_attack;

void dude_idle (edict_t *self)
{

// No idle-sound

}



// STAND

void dude_stand (edict_t *self);

mframe_t dude_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t dude_move_stand = {FRAME_stand1, FRAME_stand3, dude_frames_stand, dude_stand};

void dude_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &dude_move_stand;
}


//
// WALK
//

mframe_t dude_frames_walk [] =
{
	ai_walk, 6,  NULL,
	ai_walk, 7,  NULL,
	ai_walk, 7,  NULL,
	ai_walk, 7,  NULL,
	ai_walk, 6,  NULL
};
mmove_t dude_move_walk = {FRAME_walk1, FRAME_walk5, dude_frames_walk, NULL};

void dude_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &dude_move_walk;
}


//
// RUN
//

void dude_run (edict_t *self);

mframe_t dude_frames_run [] =
{
	ai_run, 10, NULL, 
	ai_run, 12, NULL,
	ai_run, 10, NULL,

};
mmove_t dude_move_run = {FRAME_walk1, FRAME_walk3, dude_frames_run, dude_run};

void dude_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &dude_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &dude_move_run;
}

//
// PAIN
//

mframe_t dude_frames_pain [] =
{
	ai_move, -1, NULL,
	ai_move, 1,  NULL
};
mmove_t dude_move_pain = {FRAME_pain1, FRAME_pain2, dude_frames_pain, dude_run};

void dude_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	
	//if (self->health < (self->max_health / 2))
	//	self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;
	
	self->monsterinfo.currentmove = &dude_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->pain_debounce_time = level.time + 0.6;
}


//
// ATTACK
//

void dude_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
		
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
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack, 1, ATTN_NORM, 0);
	monster_fire_bullet (self, start, aim, 8, 1, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 1);
}


mframe_t dude_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, dude_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t dude_move_attack = {FRAME_attack1, FRAME_attack7, dude_frames_attack, dude_run};

void dude_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &dude_move_attack;
}


//
// SIGHT
//

void dude_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &dude_move_attack;
	}
}


//
// DEATH
//

static void dude_dropammoclip (edict_t *self)
{
	edict_t	*backpack;

	backpack = Drop_Item(self, FindItemByClassname("ammo_idg2_bulletsS"));
	backpack->touch = Touch_Item;
}


void dude_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 24);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t dude_frames_death [] =
{
	ai_move, -1, NULL,
	ai_move, -3, dude_dropammoclip,
	ai_move, -5, NULL,
	ai_move, -4, NULL,
	ai_move, -2, NULL,
	ai_move, -3, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
};
mmove_t dude_move_death = {FRAME_death0, FRAME_death8, dude_frames_death, dude_dead};


void dude_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_NORM, 0);

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
	self->monsterinfo.currentmove = &dude_move_death;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

}


//
// SPAWN
//

/*QUAKED monster_idg2_dude (1 .5 0) (-16 -16 0) (16 16 56) Ambush Trigger_Spawn Sight
Zombie
*/

void SP_monster_idg2_dude (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 32;
		SP_monster_idg1_sa (self);
		return;
	}
		
	sound_sight=	gi.soundindex ("idg2monster/dude/sight.wav");
	sound_pain= 	gi.soundindex ("idg2monster/dude/pain.wav");
	sound_death=	gi.soundindex ("idg2monster/dude/death.wav");
	sound_attack=   gi.soundindex ("idg2weapons/pistol.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg2/dude/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 56);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 100;
	self->health = 35;
	self->gib_health = -55;

	self->pain = dude_pain;
	self->die = dude_die;

	self->monsterinfo.stand = dude_stand;
	self->monsterinfo.walk = dude_walk;
	self->monsterinfo.run = dude_run;
	self->monsterinfo.attack = dude_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = dude_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &dude_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}