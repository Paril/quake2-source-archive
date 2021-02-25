/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_slave.c
  Description: Generic Slave scripts/AI

\**********************************************************/

#include "g_local.h"
#include "b_slave.h"

void slave_pain (edict_t *self, edict_t *other, float kick, int damage);
void slave_stand2 (edict_t *self);
void slave_walk2 (edict_t *self);
void slave_run (edict_t *self);
void slave_runaffraid2 (edict_t *self);
void slave_cower2 (edict_t *self);
void slave_sit2 (edict_t *self);
void slave_wall (edict_t *self);
void slave_talk12 (edict_t *self);
void slave_talk22 (edict_t *self);
void slave_talk32 (edict_t *self);
void slave_talk42 (edict_t *self);
void find_node_hide(edict_t *ent);

#define SLAVE_CROUCH		8
#define SLAVE_WALK		16
#define SLAVE_WALL		64
#define SLAVE_CHAINED	128

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_pain4;
static int	sound_die1;
static int	sound_die2;
static int	sound_die3;
static int	sound_die4;
static int	sound_eat;
static int	sound_doodle;
static int	sound_gib;


mframe_t slave_frames_action [] =
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
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};

mframe_t slave_frames_stand [] =
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
mmove_t slave_move_stand = {FRAME_Stand_start, FRAME_Stand_end, slave_frames_stand, NULL};

mmove_t slave_move_cower1 = {FRAME_Cower1_start, FRAME_Cower1_end, slave_frames_stand, slave_cower2};
mmove_t slave_move_cower2 = {FRAME_Cower2_start, FRAME_Cower2_end, slave_frames_stand, NULL};
mmove_t slave_move_cower3 = {FRAME_Cower3_start, FRAME_Cower3_end, slave_frames_stand, slave_stand2};

mmove_t slave_move_sit1 = {FRAME_SitDown_start, FRAME_SitDown_end, slave_frames_stand, slave_sit2};
mmove_t slave_move_sit2 = {FRAME_Sit_start, FRAME_Sit_end, slave_frames_stand, NULL};
mmove_t slave_move_sit3 = {FRAME_StandUp_start, FRAME_StandUp_end, slave_frames_stand, slave_stand2};

void slave_cower (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_cower1;
}

void slave_cower2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_cower2;
}

void slave_sit (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_sit1;
}

void slave_sit2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_sit2;
}

mframe_t slave_frames_walk1 [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};
mframe_t slave_frames_walk2 [] =
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
mframe_t slave_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 0,  NULL
};

mmove_t slave_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, slave_frames_walk1, slave_walk2};
mmove_t slave_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, slave_frames_walk2, NULL};
mmove_t slave_move_walk3= {FRAME_Walk3_start, FRAME_Walk3_end, slave_frames_walk3, slave_stand2};

mframe_t slave_frames_run1 [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 8,  NULL,
	BOT_run, 12, NULL
};
mframe_t slave_frames_run2 [] =
{
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 0,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL
};

mmove_t slave_move_runaffraid1 = {FRAME_RunAffraid1_start, FRAME_RunAffraid1_end, slave_frames_run1, slave_run};
mmove_t slave_move_runaffraid2 = {FRAME_RunAffraid2_start, FRAME_RunAffraid2_end, slave_frames_run2, NULL};
mmove_t slave_move_run = {FRAME_Run_start, FRAME_Run_end, slave_frames_run2, NULL};

void slave_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &slave_move_walk2)
		self->monsterinfo.currentmove = &slave_move_walk3;
	else if (self->monsterinfo.currentmove == &slave_move_sit2)
		self->monsterinfo.currentmove = &slave_move_sit3;
	else if (self->monsterinfo.currentmove == &slave_move_cower2)
		self->monsterinfo.currentmove = &slave_move_cower3;
	else
		self->monsterinfo.currentmove = &slave_move_stand;
}

void slave_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_stand;
}

void slave_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &slave_move_walk1;   
}

void slave_walk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_walk2;
}


void slave_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &slave_move_stand;
	else 
		self->monsterinfo.currentmove = &slave_move_run;
}

void slave_runaffraid (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &slave_move_stand;
	else 
		self->monsterinfo.currentmove = &slave_move_runaffraid1;
}

void slave_runaffraid2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_runaffraid2;
}


mmove_t slave_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, slave_frames_action, slave_runaffraid};
mmove_t slave_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, slave_frames_action, slave_runaffraid};
mmove_t slave_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, slave_frames_action, slave_wall};


void slave_sight (edict_t *self, edict_t *other)
{
}

void slave_dead (edict_t *self)
{
	VectorSet (self->mins, -31, -19, -2);
	VectorSet (self->maxs, 39, 20, 9);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

void slave_dead2 (edict_t *self)
{
	VectorSet (self->mins, -29, -17, -2);
	VectorSet (self->maxs, 16, 18, 39);
	//self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

void slave_dead3 (edict_t *self)
{
	VectorSet (self->mins, -10, -26, -1);
	VectorSet (self->maxs, 29, 26, 55);
	//self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mmove_t slave_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, slave_frames_action, slave_dead};
mmove_t slave_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, slave_frames_action, slave_dead};
mmove_t slave_move_death3 = {FRAME_Die3_start, FRAME_Die3_end, slave_frames_action, slave_dead2};
mmove_t slave_move_death4 = {FRAME_Die4_start, FRAME_Die4_end, slave_frames_action, slave_dead3};

mmove_t slave_move_lookup = {FRAME_LookUp_start, FRAME_LookUp_end, slave_frames_action, slave_stand2};
mmove_t slave_move_lookdown = {FRAME_LookDown_start, FRAME_LookDown_end, slave_frames_action, slave_stand2};
mmove_t slave_move_talk11 = {FRAME_TalkStart_start, FRAME_TalkStart_end, slave_frames_action, slave_talk12};
mmove_t slave_move_talk12 = {FRAME_Talk1_start, FRAME_Talk1_end, slave_frames_action, NULL};
mmove_t slave_move_talk21 = {FRAME_TalkStart_start, FRAME_TalkStart_end, slave_frames_action, slave_talk22};
mmove_t slave_move_talk22 = {FRAME_Talk2_start, FRAME_Talk2_end, slave_frames_action, NULL};
mmove_t slave_move_talk31 = {FRAME_TalkStart_start, FRAME_TalkStart_end, slave_frames_action, slave_talk32};
mmove_t slave_move_talk32 = {FRAME_Talk3_start, FRAME_Talk3_end, slave_frames_action, NULL};
mmove_t slave_move_talk41 = {FRAME_TalkStart_start, FRAME_TalkStart_end, slave_frames_action, slave_talk42};
mmove_t slave_move_talk42 = {FRAME_Talk4_start, FRAME_Talk4_end, slave_frames_action, NULL};
mmove_t slave_move_getfood = {FRAME_GetFood_start, FRAME_GetFood_end, slave_frames_action, slave_sit2};
mmove_t slave_move_eat = {FRAME_Eat_start, FRAME_Eat_end, slave_frames_action, slave_sit2};
mmove_t slave_move_doodle = {FRAME_Doodle_start, FRAME_Doodle_end, slave_frames_action, slave_sit2};
mmove_t slave_move_button = {FRAME_Button_start, FRAME_Button_end, slave_frames_action, slave_stand2};
mmove_t slave_move_scream = {FRAME_Scream_start, FRAME_Scream_end, slave_frames_action, slave_stand2};
mmove_t slave_move_shoved = {FRAME_Shoved_start, FRAME_Shoved_end, slave_frames_action, slave_walk};
mmove_t slave_move_wall = {FRAME_Wall_start, FRAME_Wall_end, slave_frames_action, NULL};
mmove_t slave_move_chained = {FRAME_Chained_start, FRAME_Chained_end, slave_frames_action, NULL};

void slave_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

	if (self->spawnflags & SLAVE_WALL)
		{
		gi.sound (self, CHAN_VOICE, sound_die3, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_death3;
		return;
		};
	if (self->spawnflags & SLAVE_CHAINED)
		{
		gi.sound (self, CHAN_VOICE, sound_die4, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_death4;
		return;
		}
	if (rand() & 1)
		{
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_death1;
		}
	else 
		{	
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_death2;
		}
}

void slave_pain (edict_t *self, edict_t *other, float kick, int damage)
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

	if (self->spawnflags & SLAVE_WALL)
		{
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_pain3;
		return;
		}
	if (self->spawnflags & SLAVE_CHAINED)
		{
		gi.sound (self, CHAN_VOICE, sound_pain4, 1, ATTN_NORM, 0);
		return;
		}

	self->status |= STATUS_MONSTER_HIDE;
	self->monsterinfo.chaser = other;
   self->monsterinfo.goalnode = NULL;
   self->enemy = NULL;
   find_node_hide(self);

	if (rand() & 1)
		{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_pain1;
		}
	else
		{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &slave_move_pain2;
		}	
}

void slave_lookup (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_lookup;
}

void slave_lookdown (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_lookdown;
}

void slave_talk1 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk11;
}

void slave_talk12 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk12;
}

void slave_talk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk21;
}

void slave_talk22 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk22;
}

void slave_talk3 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk31;
}

void slave_talk32 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk32;
}

void slave_talk4 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk41;
}

void slave_talk42 (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_talk42;
}

void slave_getfood (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_getfood;
}

void slave_eat (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_eat;
	gi.sound (self, CHAN_VOICE, sound_eat, 1, ATTN_NORM, 0);
}

void slave_doodle (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_doodle;
	gi.sound (self, CHAN_BODY, sound_doodle, 1, ATTN_NORM, 0);
}

void slave_button (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_button;
}

void slave_scream (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_scream;
}

void slave_shoved (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_shoved;
}

void slave_wall (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_wall;
}

void slave_chained (edict_t *self)
{
	self->monsterinfo.currentmove = &slave_move_chained;
}


mmove_t slave_move_decap = {FRAME_Decap_start, FRAME_Decap_end, slave_frames_action, slave_dead};

void slave_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &slave_move_decap;
	ThrowGib (self, "models/monsters/slave/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_slave (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("slave/pain1.wav");
	sound_pain2 = gi.soundindex ("slave/pain2.wav");
	sound_die1 = gi.soundindex ("slave/death1.wav");
	sound_die2 = gi.soundindex ("slave/death2.wav");

	sound_rocketshot = gi.soundindex ("slave/fire.wav");

	sound_melee_hit = gi.soundindex ("slave/melee.wav");
	sound_sight = gi.soundindex ("slave/sight.wav");
	sound_search = gi.soundindex ("slave/search.wav");
	sound_idle = gi.soundindex ("slave/idle.wav");
   */

	sound_pain1    = gi.soundindex ("monsters/slave/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave/pain2.wav");
	sound_pain3    = gi.soundindex ("monsters/slave/pain3.wav");
	sound_pain4    = gi.soundindex ("monsters/slave/pain4.wav");
	sound_die1     = gi.soundindex ("monsters/slave/die1.wav");
	sound_die2     = gi.soundindex ("monsters/slave/die2.wav");
	sound_die3     = gi.soundindex ("monsters/slave/die3.wav");
	sound_die4     = gi.soundindex ("monsters/slave/die4.wav");
	sound_eat		= gi.soundindex ("monsters/slave/eat.wav");
	sound_doodle	= gi.soundindex ("monsters/slave/doodle.wav");
	sound_gib	   = gi.soundindex ("monsters/slave/gib.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave/tris.md2");
	VectorSet (self->mins, -3, -13, -1);
	VectorSet (self->maxs, 13, 13, 59);

   if (!self->health)
	   self->health = 60;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = slave_pain;
	self->die = slave_die;

   self->monsterinfo.stand = slave_stand;
	self->monsterinfo.walk = slave_walk;
	self->monsterinfo.run = slave_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = slave_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;

	self->monsterinfo.action[0]  = slave_lookup;
	self->monsterinfo.action[1]  = slave_lookdown;
	self->monsterinfo.action[2]  = slave_talk1;
	self->monsterinfo.action[3]  = slave_talk2;
	self->monsterinfo.action[4]  = slave_talk3;
	self->monsterinfo.action[5]  = slave_talk4;
	self->monsterinfo.action[6]  = slave_getfood;
	self->monsterinfo.action[7]  = slave_eat;
	self->monsterinfo.action[8]  = slave_doodle;
	self->monsterinfo.action[9]  = slave_button;
	self->monsterinfo.action[10] = slave_scream;
	self->monsterinfo.action[11] = slave_shoved;
	self->monsterinfo.action[12] = slave_wall;
	self->monsterinfo.action[13] = slave_chained;
	self->monsterinfo.action[14] = slave_cower;
	self->monsterinfo.action[15] = slave_sit;
	self->monsterinfo.decap = slave_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);

	// No attack
	self->spawnflags |= 32;

   walkmonster_start (self);
		
   if (self->spawnflags & SLAVE_CROUCH)
      self->monsterinfo.currentmove = &slave_move_sit2;
   else if (self->spawnflags & SLAVE_WALK)
      self->monsterinfo.currentmove = &slave_move_walk2;
   else if (self->spawnflags & SLAVE_WALL)
      self->monsterinfo.currentmove = &slave_move_wall;
   else if (self->spawnflags & SLAVE_CHAINED)
      self->monsterinfo.currentmove = &slave_move_chained;
   else
		self->monsterinfo.currentmove = &slave_move_stand;

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
