/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_sbarrow.c
  Description: Wheel Barrow Slave scripts/AI 

\**********************************************************/

#include "g_local.h"
#include "b_sbarrow.h"

void sbarrow_pain (edict_t *self, edict_t *other, float kick, int damage);
void sbarrow_stand2 (edict_t *self);
void sbarrow_walk2 (edict_t *self);
void sbarrow_run (edict_t *self);
void sbarrow_run2 (edict_t *self);
void sbarrow_runaffraid2 (edict_t *self);
void sbarrow_cower2 (edict_t *self);
void find_node_hide(edict_t *ent);

#define SBARROW_WALK		16

static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_dump;
static int	sound_gib;


mframe_t sbarrow_frames_action [] =
{
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};

mframe_t sbarrow_frames_stand [] =
{
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL
};
mmove_t sbarrow_move_stand = {FRAME_Stand_start, FRAME_Stand_end, sbarrow_frames_stand, NULL};

mmove_t sbarrow_move_cower1 = {FRAME_Cower1_start, FRAME_Cower1_end, sbarrow_frames_stand, sbarrow_cower2};
mmove_t sbarrow_move_cower2 = {FRAME_Cower2_start, FRAME_Cower2_end, sbarrow_frames_stand, NULL};
mmove_t sbarrow_move_cower3 = {FRAME_Cower3_start, FRAME_Cower3_end, sbarrow_frames_stand, sbarrow_stand2};

void sbarrow_cower (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_cower1;
}

void sbarrow_cower2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_cower2;
}

mframe_t sbarrow_frames_walk1 [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};
mframe_t sbarrow_frames_walk2 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL
};
mframe_t sbarrow_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 0,  NULL
};

mmove_t sbarrow_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, sbarrow_frames_walk1, sbarrow_walk2};
mmove_t sbarrow_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, sbarrow_frames_walk2, NULL};
mmove_t sbarrow_move_walk3	= {FRAME_Walk3_start, FRAME_Walk3_end, sbarrow_frames_walk3, sbarrow_stand2};

mframe_t sbarrow_frames_run1 [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 8,  NULL
};
mframe_t sbarrow_frames_run2 [] =
{
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 0,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL
};
mframe_t sbarrow_frames_run3 [] =
{
	BOT_run, 8,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 0,  NULL
};

mmove_t sbarrow_move_runaffraid1 = {FRAME_RunAffraid1_start, FRAME_RunAffraid1_end, sbarrow_frames_run1, sbarrow_runaffraid2};
mmove_t sbarrow_move_runaffraid2 = {FRAME_RunAffraid2_start, FRAME_RunAffraid2_end, sbarrow_frames_run2, NULL};
mmove_t sbarrow_move_runaffraid3 = {FRAME_RunAffraid3_start, FRAME_RunAffraid3_end, sbarrow_frames_run3, sbarrow_stand2};
mmove_t sbarrow_move_run1 = {FRAME_Run1_start, FRAME_Run1_end, sbarrow_frames_run1, sbarrow_run2};
mmove_t sbarrow_move_run2 = {FRAME_Run2_start, FRAME_Run2_end, sbarrow_frames_run2, NULL};
mmove_t sbarrow_move_run3 = {FRAME_Run3_start, FRAME_Run3_end, sbarrow_frames_run3, sbarrow_stand2};

void sbarrow_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &sbarrow_move_walk2)
		self->monsterinfo.currentmove = &sbarrow_move_walk3;
	else if (self->monsterinfo.currentmove == &sbarrow_move_run2)
		self->monsterinfo.currentmove = &sbarrow_move_run3;
	else if (self->monsterinfo.currentmove == &sbarrow_move_runaffraid2)
		self->monsterinfo.currentmove = &sbarrow_move_runaffraid3;
	else if (self->monsterinfo.currentmove == &sbarrow_move_cower2)
		self->monsterinfo.currentmove = &sbarrow_move_cower3;
	else
		self->monsterinfo.currentmove = &sbarrow_move_stand;
}

void sbarrow_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_stand;
}

void sbarrow_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &sbarrow_move_walk1;   
}

void sbarrow_walk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_walk2;
}


void sbarrow_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &sbarrow_move_stand;
	else 
		self->monsterinfo.currentmove = &sbarrow_move_run1;
}

void sbarrow_run2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_run2;
}

void sbarrow_runaffraid (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &sbarrow_move_stand;
	else 
		self->monsterinfo.currentmove = &sbarrow_move_runaffraid1;
}

void sbarrow_runaffraid2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_runaffraid2;
}


mmove_t sbarrow_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, sbarrow_frames_action, sbarrow_runaffraid};
mmove_t sbarrow_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, sbarrow_frames_action, sbarrow_runaffraid};


void sbarrow_sight (edict_t *self, edict_t *other)
{
}

void sbarrow_dead (edict_t *self)
{
	VectorSet (self->mins, -10, -19, -1);
	VectorSet (self->maxs, 33, 16, 30);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

void sbarrow_dead2 (edict_t *self)
{
	VectorSet (self->mins, -20, -16, -2);
	VectorSet (self->maxs, 17, 18, 32);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

void sbarrow_dead3 (edict_t *self)
{
	VectorSet (self->mins, -31, -14, -2);
	VectorSet (self->maxs, 19, 17, 11);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;	
	gi.linkentity (self);
}

mmove_t sbarrow_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, sbarrow_frames_action, sbarrow_dead};
mmove_t sbarrow_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, sbarrow_frames_action, sbarrow_dead2};

mmove_t sbarrow_move_lookup = {FRAME_LookUp_start, FRAME_LookUp_end, sbarrow_frames_action, sbarrow_stand2};
mmove_t sbarrow_move_lookdown = {FRAME_LookDown_start, FRAME_LookDown_end, sbarrow_frames_action, sbarrow_stand2};
mmove_t sbarrow_move_talk = {FRAME_Talk_start, FRAME_Talk_end, sbarrow_frames_action, sbarrow_stand};
mmove_t sbarrow_move_dump = {FRAME_Dump_start, FRAME_Dump_end, sbarrow_frames_action, sbarrow_stand};

void sbarrow_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	if (self->health <= self->gib_health)
		{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/slave/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/slave/gibs/head.md2", damage, GIB_METALLIC);
		return;
		}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (rand() & 1)
		{
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sbarrow_move_death1;
		}
	else 
		{	
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sbarrow_move_death2;
		}
}

void sbarrow_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (!(self->s.skinnum & 1))
		if (self->health < self->max_health / 2)
			{
			self->s.skinnum++;
			gi.linkentity(self);
			}
   
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	self->status |= STATUS_MONSTER_HIDE;
	self->monsterinfo.chaser = other;
   self->monsterinfo.goalnode = NULL;
   self->enemy = NULL;
   find_node_hide(self);

	if (rand() & 1)
		{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sbarrow_move_pain1;
		}
	else
		{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sbarrow_move_pain2;
		}	
}

void sbarrow_lookup (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_lookup;
}

void sbarrow_lookdown (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_lookdown;
}

void sbarrow_talk (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_talk;
}

void sbarrow_dump (edict_t *self)
{
	self->monsterinfo.currentmove = &sbarrow_move_dump;
	gi.sound (self, CHAN_WEAPON, sound_dump, 1, ATTN_NORM, 0);
}

mmove_t sbarrow_move_decap = {FRAME_Decap_start, FRAME_Decap_end, sbarrow_frames_action, sbarrow_dead3};

void sbarrow_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &sbarrow_move_decap;
	ThrowGib (self, "models/monsters/slave/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_sbarrow (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("sbarrow/pain1.wav");
	sound_pain2 = gi.soundindex ("sbarrow/pain2.wav");
	sound_die1 = gi.soundindex ("sbarrow/death1.wav");
	sound_die2 = gi.soundindex ("sbarrow/death2.wav");

	sound_rocketshot = gi.soundindex ("sbarrow/fire.wav");

	sound_melee_hit = gi.soundindex ("sbarrow/melee.wav");
	sound_sight = gi.soundindex ("sbarrow/sight.wav");
	sound_search = gi.soundindex ("sbarrow/search.wav");
	sound_idle = gi.soundindex ("sbarrow/idle.wav");
   */

	sound_pain1    = gi.soundindex ("monsters/slave/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/slave/die1.wav");
	sound_die2     = gi.soundindex ("monsters/slave/die2.wav");
	sound_dump     = gi.soundindex ("monsters/slave/dump.wav");
	sound_gib	   = gi.soundindex ("monsters/slave/gib.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave/barrow/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/slave/barrow/barrow.md2");
	VectorSet (self->mins, -4, -16, 0);
	VectorSet (self->maxs, 16, 16, 56);

   if (!self->health)
	   self->health = 60;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = sbarrow_pain;
	self->die = sbarrow_die;

   self->monsterinfo.stand = sbarrow_stand;
	self->monsterinfo.walk = sbarrow_walk;
	self->monsterinfo.run = sbarrow_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = sbarrow_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;

	self->monsterinfo.action[0] = sbarrow_lookup;
	self->monsterinfo.action[1] = sbarrow_lookdown;
	self->monsterinfo.action[2] = sbarrow_talk;
	self->monsterinfo.action[3] = sbarrow_dump;
	self->monsterinfo.action[4] = sbarrow_cower;
	self->monsterinfo.decap = sbarrow_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);

	// No attack
	self->spawnflags |= 32;

   walkmonster_start (self);
		
   if (self->spawnflags & SBARROW_WALK)
      self->monsterinfo.currentmove = &sbarrow_move_walk2;
   else
		self->monsterinfo.currentmove = &sbarrow_move_stand;

	if (self->team_data == 1)
		self->s.skinnum = 2;
	if (self->team_data == 2)
		self->s.skinnum = 4;
	if (self->team_data == 3)
		self->s.skinnum = 6;

	gi.linkentity(self);
	self->team_data = 0;
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_SLAVE_ACTOR;
}
