/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_sore.c
  Description: Ore Slave scripts/AI

\**********************************************************/

#include "g_local.h"
#include "b_sore.h"

void sore_pain (edict_t *self, edict_t *other, float kick, int damage);
void sore_stand2 (edict_t *self);
void sore_walk2 (edict_t *self);
void sore_run (edict_t *self);
void sore_run2 (edict_t *self);
void sore_runaffraid2 (edict_t *self);
void sore_cower2 (edict_t *self);
void sore_talk12 (edict_t *self);
void sore_dropend (edict_t *self);
void find_node_hide(edict_t *ent);

#define SORE_WALK			16

static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_pickup;
static int	sound_drop;
static int	sound_gib;


mframe_t sore_frames_action [] =
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

mframe_t sore_frames_stand [] =
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
	BOT_stand, 0, NULL,
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
mmove_t sore_move_stand = {FRAME_Stand_start, FRAME_Stand_end, sore_frames_stand, NULL};
mmove_t sore_move_stand2 = {FRAME_Stand_start, FRAME_Stand_end, sore_frames_stand, NULL};

mmove_t sore_move_cower1 = {FRAME_Cower1_start, FRAME_Cower1_end, sore_frames_stand, sore_cower2};
mmove_t sore_move_cower2 = {FRAME_Cower2_start, FRAME_Cower2_end, sore_frames_stand, NULL};
mmove_t sore_move_cower3 = {FRAME_Cower3_start, FRAME_Cower3_end, sore_frames_stand, sore_stand2};

void sore_cower (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_cower1;
}

void sore_cower2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_cower2;
}

mframe_t sore_frames_walk1 [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};
mframe_t sore_frames_walk2 [] =
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
mframe_t sore_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 0,  NULL
};

mmove_t sore_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, sore_frames_walk1, sore_walk2};
mmove_t sore_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, sore_frames_walk2, NULL};
mmove_t sore_move_walk3	= {FRAME_Walk3_start, FRAME_Walk3_end, sore_frames_walk3, sore_stand2};

mmove_t sore_move_walk21 = {FRAME_Walk21_start, FRAME_Walk21_end, sore_frames_walk1, sore_walk2};
mmove_t sore_move_walk22 = {FRAME_Walk22_start, FRAME_Walk22_end, sore_frames_walk2, NULL};
mmove_t sore_move_walk23 = {FRAME_Walk23_start, FRAME_Walk23_end, sore_frames_walk3, sore_stand2};

mframe_t sore_frames_run1 [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 8,  NULL
};
mframe_t sore_frames_run2 [] =
{
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 0,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL
};
mframe_t sore_frames_run3 [] =
{
	BOT_run, 8,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 0,  NULL
};

mmove_t sore_move_run1 = {FRAME_Run1_start, FRAME_Run1_end, sore_frames_run1, sore_run2};
mmove_t sore_move_run2 = {FRAME_Run2_start, FRAME_Run2_end, sore_frames_run2, NULL};
mmove_t sore_move_run3 = {FRAME_Run3_start, FRAME_Run3_end, sore_frames_run3, sore_stand2};

void sore_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &sore_move_walk2)
		self->monsterinfo.currentmove = &sore_move_walk3;
	if (self->monsterinfo.currentmove == &sore_move_walk22)
		self->monsterinfo.currentmove = &sore_move_walk23;
	else if (self->monsterinfo.currentmove == &sore_move_run2)
		self->monsterinfo.currentmove = &sore_move_run3;
	else if (self->monsterinfo.currentmove == &sore_move_cower2)
		self->monsterinfo.currentmove = &sore_move_cower3;
	else
	if (self->s.modelindex2)
		self->monsterinfo.currentmove = &sore_move_stand;
	else
		self->monsterinfo.currentmove = &sore_move_stand2;
}

void sore_stand2 (edict_t *self)
{
	if (self->s.modelindex2)
		self->monsterinfo.currentmove = &sore_move_stand;
	else
		self->monsterinfo.currentmove = &sore_move_stand2;
}

void sore_walk (edict_t *self)
{
   self->enemy = NULL;
	if (self->s.modelindex)	
		self->monsterinfo.currentmove = &sore_move_walk1;   
	else
		self->monsterinfo.currentmove = &sore_move_walk21;
}

void sore_walk2 (edict_t *self)
{
	if (self->s.modelindex)	
		self->monsterinfo.currentmove = &sore_move_walk2;   
	else
		self->monsterinfo.currentmove = &sore_move_walk22;
}


void sore_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &sore_move_stand;
	else 
		self->monsterinfo.currentmove = &sore_move_run1;
}

void sore_run2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_run2;
}


mmove_t sore_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, sore_frames_action, sore_run};
mmove_t sore_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, sore_frames_action, sore_run};


void sore_sight (edict_t *self, edict_t *other)
{
}

void sore_dead (edict_t *self)
{
	VectorSet (self->mins, -31, -17, -2);
	VectorSet (self->maxs, 32, 14, 10);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mmove_t sore_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, sore_frames_action, sore_dead};
mmove_t sore_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, sore_frames_action, sore_dead};

mmove_t sore_move_lookup = {FRAME_LookUp_start, FRAME_LookUp_end, sore_frames_action, sore_stand2};
mmove_t sore_move_lookdown = {FRAME_LookDown_start, FRAME_LookDown_end, sore_frames_action, sore_stand2};
mmove_t sore_move_talk11 = {FRAME_TalkStart_start, FRAME_TalkStart_end, sore_frames_action, sore_talk12};
mmove_t sore_move_talk12 = {FRAME_Talk_start, FRAME_Talk_end, sore_frames_action, sore_stand};
mmove_t sore_move_pickup = {FRAME_PickUp_start, FRAME_PickUp_end, sore_frames_action, sore_stand};
mmove_t sore_move_drop = {FRAME_Drop_start, FRAME_Drop_end, sore_frames_action, sore_dropend};

void sore_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
		self->monsterinfo.currentmove = &sore_move_death1;
		}
	else 
		{	
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sore_move_death2;
		}
}

void sore_pain (edict_t *self, edict_t *other, float kick, int damage)
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
		self->monsterinfo.currentmove = &sore_move_pain1;
		}
	else
		{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &sore_move_pain2;
		}	
}

void sore_lookup (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_lookup;
}

void sore_lookdown (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_lookdown;
}

void sore_talk (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_talk11;
}

void sore_talk12 (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_talk12;
}

void sore_pickup (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_pickup;
	self->s.modelindex2 = gi.modelindex("models/monsters/slave/derantium/ore.md2");
	gi.sound (self, CHAN_WEAPON, sound_pickup, 1, ATTN_NORM, 0);
}

void sore_drop (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_drop;
	gi.sound (self, CHAN_WEAPON, sound_drop, 1, ATTN_NORM, 0);
}

void sore_dropend (edict_t *self)
{
	self->monsterinfo.currentmove = &sore_move_stand;
	self->s.modelindex2 = 0;
}

mmove_t sore_move_decap = {FRAME_Decap_start, FRAME_Decap_end, sore_frames_action, sore_dead};

void sore_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &sore_move_decap;
	ThrowGib (self, "models/monsters/slave/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_sore (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("sore/pain1.wav");
	sound_pain2 = gi.soundindex ("sore/pain2.wav");
	sound_die1 = gi.soundindex ("sore/death1.wav");
	sound_die2 = gi.soundindex ("sore/death2.wav");

	sound_rocketshot = gi.soundindex ("sore/fire.wav");

	sound_melee_hit = gi.soundindex ("sore/melee.wav");
	sound_sight = gi.soundindex ("sore/sight.wav");
	sound_search = gi.soundindex ("sore/search.wav");
	sound_idle = gi.soundindex ("sore/idle.wav");
   */

	sound_pain1    = gi.soundindex ("monsters/slave/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/slave/die1.wav");
	sound_die2     = gi.soundindex ("monsters/slave/die2.wav");
	sound_pickup   = gi.soundindex ("monsters/slave/pickup.wav");
	sound_drop     = gi.soundindex ("monsters/slave/drop.wav");
	sound_gib	   = gi.soundindex ("monsters/slave/gib.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave/derantium/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/slave/derantium/ore.md2");
	VectorSet (self->mins, -3, -13, -1);
	VectorSet (self->maxs, 13, 13, 55);

   if (!self->health)
	   self->health = 60;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = sore_pain;
	self->die = sore_die;

   self->monsterinfo.stand = sore_stand;
	self->monsterinfo.walk = sore_walk;
	self->monsterinfo.run = sore_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = sore_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;

	self->monsterinfo.action[0] = sore_lookup;
	self->monsterinfo.action[1] = sore_lookdown;
	self->monsterinfo.action[2] = sore_talk;
	self->monsterinfo.action[3] = sore_pickup;
	self->monsterinfo.action[4] = sore_drop;
	self->monsterinfo.action[5] = sore_cower;
	self->monsterinfo.decap = sore_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);

	// No attack
	self->spawnflags |= 32;

   walkmonster_start (self);
		
   if (self->spawnflags & SORE_WALK)
      self->monsterinfo.currentmove = &sore_move_walk2;
   else
		self->monsterinfo.currentmove = &sore_move_stand;

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
