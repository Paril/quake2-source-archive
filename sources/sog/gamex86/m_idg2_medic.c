/*
==============================================================================

IDG2 MEDIC  ( DOOM II ARCHVILE )

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_medic.h"

qboolean visible (edict_t *self, edict_t *other);

edict_t *medic_FindDeadMonster (edict_t *self);    // from m_medic.c

void ED_CallSpawn (edict_t *ent);

static int sound_sight;
static int sound_death;
static int sound_pain;
static int sound_attack;
static int sound_attack2;
static int sound_resurrect;

//
// FLAME STUFF
//

void idg2_medic_flame_anim (edict_t *self)
{
	if(self->s.frame >= 2)
		self->think = G_FreeEdict;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

void idg2_medic_flame (edict_t *self)
{
	edict_t *ent;
	
	ent = G_Spawn();
	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[0] += ((rand() % 40) - 20);
	ent->s.origin[1] += ((rand() % 40) - 20);
	ent->s.origin[2] += rand() % 30;	
	ent->solid = SOLID_BBOX;
	ent->movetype = MOVETYPE_NONE;
	ent->s.renderfx |= RF_TRANSLUCENT;
	VectorClear (ent->mins);
	VectorClear (ent->maxs);
	ent->s.modelindex = gi.modelindex ("sprites/idg2med.sp2");
	ent->s.frame = 0;
	ent->owner = self;
	ent->nextthink = level.time + FRAMETIME;
	ent->think = idg2_medic_flame_anim;
	ent->classname = "flame";
	
	gi.linkentity (ent);	
}



void idg2_medic_idle (edict_t *self)
{

	edict_t	*ent;

	ent = medic_FindDeadMonster(self);

	if (ent)

	{

		self->enemy = ent;

		self->enemy->owner = self;

		self->monsterinfo.aiflags |= AI_MEDIC;

		FoundTarget (self);

	}

}


void idg2_medic_search (edict_t *self)
{

	edict_t	*ent;

	if (!self->oldenemy)

	{

		ent = medic_FindDeadMonster(self);

		if (ent)

		{

			self->oldenemy = self->enemy;

			self->enemy = ent;

			self->enemy->owner = self;

			self->monsterinfo.aiflags |= AI_MEDIC;

			FoundTarget (self);

		}

	}

}


// STAND

void idg2_medic_stand (edict_t *self);

mframe_t idg2_medic_frames_stand [] =
{
	ai_stand, 0, idg2_medic_idle,
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
mmove_t idg2_medic_move_stand = {FRAME_stand1, FRAME_stand10, idg2_medic_frames_stand, NULL};

void idg2_medic_stand (edict_t *self)
{
	self->s.effects &= ~EF_HYPERBLASTER;

	self->monsterinfo.currentmove = &idg2_medic_move_stand;
}


//
// WALK
//

mframe_t idg2_medic_frames_walk [] =
{
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL,
	ai_walk, 8, NULL
};
mmove_t idg2_medic_move_walk = {FRAME_run1, FRAME_run7, idg2_medic_frames_walk, NULL};

void idg2_medic_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_medic_move_walk;
}


//
// RUN
//

void idg2_medic_run (edict_t *self);

mframe_t idg2_medic_frames_run [] =
{
	ai_run, 14, NULL, 
	ai_run, 14, NULL,
	ai_run, 14, NULL,
	ai_run, 14, NULL, 
	ai_run, 14, NULL,
	ai_run, 14, NULL,
	ai_run, 14, NULL
};
mmove_t idg2_medic_move_run = {FRAME_run1, FRAME_run7, idg2_medic_frames_run, NULL};

void idg2_medic_run (edict_t *self)
{
	
	self->s.effects &= ~EF_HYPERBLASTER;

	if (!(self->monsterinfo.aiflags & AI_MEDIC))

	{

		edict_t	*ent;


		ent = medic_FindDeadMonster(self);

		if (ent)

		{

			self->oldenemy = self->enemy;

			self->enemy = ent;

			self->enemy->owner = self;

			self->monsterinfo.aiflags |= AI_MEDIC;

			FoundTarget (self);

			return;

		}

	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &idg2_medic_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &idg2_medic_move_run;
}

//
// PAIN
//

mframe_t idg2_medic_frames_pain [] =
{
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, 2,  NULL,
	ai_move, 1,  NULL
};
mmove_t idg2_medic_move_pain = {FRAME_pain1, FRAME_pain4, idg2_medic_frames_pain, idg2_medic_run};

void idg2_medic_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (skill->value == 3)

		return;		// no pain anims in nightmare

	
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	self->monsterinfo.currentmove = &idg2_medic_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->pain_debounce_time = level.time + 0.6;
}


//
// RESURRECT
//

void idg2_medic_burn (edict_t *self)
{
	if(!self->enemy)
		return;
		
	idg2_medic_flame (self->enemy);
	idg2_medic_flame (self->enemy);
}


void idg2_medic_resurrect (edict_t *self)
{
	if (!self->enemy->inuse)

		return;
		
	self->enemy->monsterinfo.aiflags |= AI_RESURRECTING;

	self->enemy->spawnflags = 0;

	self->enemy->monsterinfo.aiflags = 0;

	self->enemy->target = NULL;

	self->enemy->targetname = NULL;

	self->enemy->combattarget = NULL;

	self->enemy->deathtarget = NULL;

	self->enemy->owner = self;

	ED_CallSpawn (self->enemy);

	self->enemy->owner = NULL;

	if (self->enemy->think)

	{

		self->enemy->nextthink = level.time;

		self->enemy->think (self->enemy);

	}
		

}

void idg2_medic_complete_resurrection (edict_t *self)
{
	self->enemy->monsterinfo.aiflags &= ~AI_RESURRECTING;
}


mframe_t idg2_medic_frames_resurrect [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, idg2_medic_burn,
	ai_move, 0, idg2_medic_burn,
	ai_move, 0, idg2_medic_resurrect,
	ai_move, 0, idg2_medic_burn,
	ai_move, 0, idg2_medic_burn,
	ai_move, 0, idg2_medic_complete_resurrection,
};
mmove_t idg2_medic_move_resurrect = {FRAME_res1, FRAME_res10, idg2_medic_frames_resurrect, idg2_medic_run};

//
// ATTACK
//



void idg2_medic_start_fire (edict_t *self)
{

		
	if(!self->enemy)
		return;

	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack, 1, ATTN_NORM, 0);
	
	self->s.effects |= EF_HYPERBLASTER;
}


void idg2_medic_release_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	edict_t *ent;

	vec3_t  point,v;
	
	if(!self->enemy)
		return;
	
	ent = self->enemy;
	
	//if (!visible(self, ent));
	//	return;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ_BLASTER], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack2, 1, ATTN_NORM, 0);

	monster_fire_bullet (self, start, aim, 30, 0, 10, 10, MZ_SHOTGUN);

	// do our special form of knockback here
	
	VectorCopy(self->enemy->s.origin,point);
	
	point[2] -= 30;
	
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, 400, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;	
	
	self->s.effects &= ~EF_HYPERBLASTER;

}

mframe_t idg2_medic_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_medic_start_fire,
	ai_charge, 0, idg2_medic_burn,
	ai_charge, 0, idg2_medic_burn,
	ai_charge, 0, idg2_medic_burn,
	ai_charge, 0, idg2_medic_release_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg2_medic_move_attack = {FRAME_attack1, FRAME_attack8, idg2_medic_frames_attack, idg2_medic_run};

void idg2_medic_attack(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_MEDIC)

		self->monsterinfo.currentmove = &idg2_medic_move_resurrect;

	else

		self->monsterinfo.currentmove = &idg2_medic_move_attack;
}



qboolean idg2_medic_checkattack (edict_t *self)

{

	if (self->monsterinfo.aiflags & AI_MEDIC)

	{

		idg2_medic_attack(self);

		return true;

	}



	return M_CheckAttack (self);

}

//
// SIGHT
//

void idg2_medic_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


//
// DEATH
//


void idg2_medic_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t idg2_medic_frames_death [] =
{
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
};
mmove_t idg2_medic_move_death = {FRAME_death1, FRAME_death8, idg2_medic_frames_death, idg2_medic_dead};


void idg2_medic_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	self->s.effects &= ~EF_BLASTER;

	// if we had a pending patient, free him up for another medic

	if ((self->enemy) && (self->enemy->owner == self))

		self->enemy->owner = NULL;

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
	self->monsterinfo.currentmove = &idg2_medic_move_death;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

}


//
// SPAWN
//

/*QUAKED monster_idg2_medic (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
Archvile
*/

void SP_monster_idg2_medic (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 11;
		SP_monster_idg1_ss (self);
		return;
	}

	sound_sight=	gi.soundindex ("idg2monster/medic/sight.wav");
	sound_pain= 	gi.soundindex ("idg2monster/medic/pain.wav");
	sound_death=	gi.soundindex ("idg2monster/medic/death.wav");
	sound_attack=   gi.soundindex ("idg1weapons/ftshot.wav");
	sound_attack2=  gi.soundindex ("idg1weapons/fthit.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/idg2/medic/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	
	self->mass = 200;
	self->health = 500;
	self->gib_health = -100;

	self->pain = idg2_medic_pain;
	self->die = idg2_medic_die;

	self->monsterinfo.stand = idg2_medic_stand;
	self->monsterinfo.walk = idg2_medic_walk;
	self->monsterinfo.run = idg2_medic_run;
	self->monsterinfo.attack = idg2_medic_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_medic_sight;
	
	self->monsterinfo.idle = idg2_medic_idle;

	self->monsterinfo.search = idg2_medic_search;

	self->monsterinfo.checkattack = idg2_medic_checkattack;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_medic_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}