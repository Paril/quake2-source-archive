/*
==============================================================================

idg0_keen

Commander Keen (NPC)

==============================================================================
*/

#include "g_local.h"
#include "m_idg0_keen.h"
#include "g_sogutil.h"

qboolean HasGun;
qboolean PissedOff;

int sound_death;

void idg0_keen_stand (edict_t *self);
void idg0_keen_attack (edict_t *self);
void idg0_keen_fire (edict_t *self);
void idg0_keen_dead (edict_t *self);


static int Abs(int i)
{
	if (i >= 0) return i; 
	else return - i;
}; 


void idg0_keen_search_player (edict_t *self)
{
	int i;
	edict_t *ent;
	
	if (!HasGun)
		return;

	if (PissedOff)
		return;

	ent=&g_edicts[0];

	for (i=0 ; i<=maxclients->value ; i++, ent++)
	{		
		if (!ent->inuse)
			continue;

		if (!ent->client)
			continue;
		
		if (ent->client->pers.health <= 0)
			continue;

		// Measuring distance between Keen and player

		if ((Abs(ent->s.origin[0] - self->s.origin[0]) < 64) &&
			(Abs(ent->s.origin[1] - self->s.origin[1]) < 64) &&
			(Abs(ent->s.origin[2] - self->s.origin[2]) < 64))
		{
			if (!ent->client->pers.inventory[K_RAYGUN])
			{
				HasGun = false;
				ent->client->pers.inventory[K_RAYGUN]++;
				ent->client->newweapon = GetItemByIndex(K_RAYGUN);
//				safe_bprintf(PRINT_HIGH,"A special gift from Commander Keen !",ent);
				return;
			}
		}
	}


}

mframe_t idg0_keen_frames_wave [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 5
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 10
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 15
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 20
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 25
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 30
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 35
	
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 40

	ai_stand, 0, NULL,
	ai_stand, 0, idg0_keen_search_player
};
mmove_t idg0_keen_move_wave = {FRAME_wave1, FRAME_wave42, idg0_keen_frames_wave, idg0_keen_stand};

mframe_t idg0_keen_frames_guntaken [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 5

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 10

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 15

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 20

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,	// 25

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL	// 30
};
mmove_t idg0_keen_move_guntaken = {FRAME_guntaken1, FRAME_guntaken30, idg0_keen_frames_guntaken, NULL};



mframe_t idg0_keen_frames_painwg [] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg0_keen_move_painwg = {FRAME_painwg1, FRAME_painwg3, idg0_keen_frames_painwg, idg0_keen_attack};

mframe_t idg0_keen_frames_painwog [] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t idg0_keen_move_painwog = {FRAME_painwog1, FRAME_painwog3, idg0_keen_frames_painwog, idg0_keen_stand};

mframe_t idg0_keen_frames_death [] =
{
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,

	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL,
	ai_move,	0,		NULL
};
mmove_t idg0_keen_move_death = {FRAME_death1, FRAME_death10, idg0_keen_frames_death, idg0_keen_dead};

mframe_t idg0_keen_frames_attack [] =
{
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		idg0_keen_fire,

	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	ai_charge,	0,		NULL,
	
	ai_charge,	0,		NULL
};
mmove_t idg0_keen_move_attack = {FRAME_attack1, FRAME_attack11, idg0_keen_frames_attack, idg0_keen_attack};



void idg0_keen_walk (edict_t *self)
{
	if (HasGun)
		self->monsterinfo.currentmove = &idg0_keen_move_wave;
	else
		self->monsterinfo.currentmove = &idg0_keen_move_guntaken;
}

void idg0_keen_run (edict_t *self)
{
	if (PissedOff)
		self->monsterinfo.currentmove = &idg0_keen_move_attack;
	else
	{
		if (HasGun)
			self->monsterinfo.currentmove = &idg0_keen_move_wave;
		else
			self->monsterinfo.currentmove = &idg0_keen_move_guntaken;
	}
}

void idg0_keen_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	
	PissedOff = true;
	
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (HasGun)
	{	
		self->monsterinfo.aiflags &= ~AI_GOOD_GUY;
		self->monsterinfo.currentmove = &idg0_keen_move_painwg;
	}
	else
		self->monsterinfo.currentmove = &idg0_keen_move_painwog;


}

void idg0_keen_stand (edict_t *self)
{
	if (HasGun)
		self->monsterinfo.currentmove = &idg0_keen_move_wave;
	else
		self->monsterinfo.currentmove = &idg0_keen_move_guntaken;
}

void idg0_keen_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;

	if(!self->enemy)
		return;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,0,0,0);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("idg3weapons/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);
	keen_fire_raygunshot (self, start, dir, 23, 400);

}



void idg0_keen_attack (edict_t *self)
{
	if (HasGun)
		self->monsterinfo.currentmove = &idg0_keen_move_attack;
	else
		self->monsterinfo.currentmove = &idg0_keen_move_guntaken;


}


void idg0_keen_dead (edict_t *self)
{

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;

	self->svflags |= SVF_DEADMONSTER;

	self->nextthink = 0;
	gi.linkentity (self);
}

void idg0_keen_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_IDLE, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_IDLE, 0);

	self->s.skinnum = 1;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &idg0_keen_move_death;

}



/*QUAKED misc_idg0_keen (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn
Commander Keen
*/
void SP_misc_idg0_keen (edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	HasGun = true;
	PissedOff = false;

	sound_death = gi.soundindex ("misc/fhit3.wav");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg0/keen/tris.md2");

	VectorSet (self->mins, -8, -8, -24);
	VectorSet (self->maxs, 8, 8, 8);

	self->health = 100;
	
	self->gib_health = -100;
	self->mass = 100000;   // Do not move an inch

	self->pain = idg0_keen_pain;
	self->die = idg0_keen_die;

	self->monsterinfo.stand = idg0_keen_stand;
	self->monsterinfo.walk = idg0_keen_walk;
	self->monsterinfo.run = idg0_keen_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg0_keen_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg0_keen_move_wave;
	
	self->monsterinfo.scale = MODEL_SCALE;
	
	walkmonster_start (self);
}

