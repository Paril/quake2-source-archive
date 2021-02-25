/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_shammer.c
  Description: Hammer Slave scripts/AI 

\**********************************************************/

#include "g_local.h"
#include "b_shammer.h"

void shammer_pain (edict_t *self, edict_t *other, float kick, int damage);
void shammer_stand2 (edict_t *self);
void shammer_walk2 (edict_t *self);
void shammer_run (edict_t *self);
void shammer_run2 (edict_t *self);
void shammer_runaffraid2 (edict_t *self);
void shammer_cower2 (edict_t *self);
void shammer_talk2 (edict_t *self);
void shammer_dropend (edict_t *self);
void find_node_hide(edict_t *ent);

#define SHAMMER_WALK		16

static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_putaway;
static int	sound_getham;
static int	sound_swing;
static int	sound_wipe;
static int	sound_gib;


mframe_t shammer_frames_action [] =
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

mframe_t shammer_frames_stand [] =
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
mmove_t shammer_move_stand1 = {FRAME_Stand1_start, FRAME_Stand1_end, shammer_frames_stand, NULL};
mmove_t shammer_move_stand2 = {FRAME_Stand2_start, FRAME_Stand2_end, shammer_frames_stand, NULL};

mmove_t shammer_move_cower11 = {FRAME_Cower11_start, FRAME_Cower11_end, shammer_frames_stand, shammer_cower2};
mmove_t shammer_move_cower12 = {FRAME_Cower12_start, FRAME_Cower12_end, shammer_frames_stand, NULL};
mmove_t shammer_move_cower13 = {FRAME_Cower13_start, FRAME_Cower13_end, shammer_frames_stand, shammer_stand2};

mmove_t shammer_move_cower21 = {FRAME_Cower21_start, FRAME_Cower21_end, shammer_frames_stand, shammer_cower2};
mmove_t shammer_move_cower22 = {FRAME_Cower22_start, FRAME_Cower22_end, shammer_frames_stand, NULL};
mmove_t shammer_move_cower23 = {FRAME_Cower23_start, FRAME_Cower23_end, shammer_frames_stand, shammer_stand2};

void shammer_cower (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_cower21;
	else
		self->monsterinfo.currentmove = &shammer_move_cower11;
}

void shammer_cower2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_cower22;
	else
		self->monsterinfo.currentmove = &shammer_move_cower22;
}

mframe_t shammer_frames_walk1 [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};
mframe_t shammer_frames_walk2 [] =
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
mframe_t shammer_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 0,  NULL
};

mmove_t shammer_move_walk11 = {FRAME_Walk11_start, FRAME_Walk11_end, shammer_frames_walk1, shammer_walk2};
mmove_t shammer_move_walk12 = {FRAME_Walk12_start, FRAME_Walk12_end, shammer_frames_walk2, NULL};
mmove_t shammer_move_walk13 = {FRAME_Walk13_start, FRAME_Walk13_end, shammer_frames_walk3, shammer_stand2};

mmove_t shammer_move_walk21 = {FRAME_Walk21_start, FRAME_Walk21_end, shammer_frames_walk1, shammer_walk2};
mmove_t shammer_move_walk22 = {FRAME_Walk22_start, FRAME_Walk22_end, shammer_frames_walk2, NULL};
mmove_t shammer_move_walk23 = {FRAME_Walk23_start, FRAME_Walk23_end, shammer_frames_walk3, shammer_stand2};

mframe_t shammer_frames_run1 [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 8,  NULL
};
mframe_t shammer_frames_run2 [] =
{
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 0,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL
};
mframe_t shammer_frames_run3 [] =
{
	BOT_run, 8,  NULL,
	BOT_run, 4,  NULL,
	BOT_run, 0,  NULL
};

mmove_t shammer_move_run11 = {FRAME_Run11_start, FRAME_Run11_end, shammer_frames_run1, shammer_run2};
mmove_t shammer_move_run12 = {FRAME_Run12_start, FRAME_Run12_end, shammer_frames_run2, NULL};
mmove_t shammer_move_run13 = {FRAME_Run13_start, FRAME_Run13_end, shammer_frames_run3, shammer_stand2};

mmove_t shammer_move_run21 = {FRAME_Run21_start, FRAME_Run21_end, shammer_frames_run1, shammer_run2};
mmove_t shammer_move_run22 = {FRAME_Run22_start, FRAME_Run22_end, shammer_frames_run2, NULL};
mmove_t shammer_move_run23 = {FRAME_Run23_start, FRAME_Run23_end, shammer_frames_run3, shammer_stand2};

mmove_t shammer_move_runaffraid11 = {FRAME_RunAffraid11_start, FRAME_RunAffraid11_end, shammer_frames_run1, shammer_runaffraid2};
mmove_t shammer_move_runaffraid12 = {FRAME_RunAffraid12_start, FRAME_RunAffraid12_end, shammer_frames_run2, NULL};
mmove_t shammer_move_runaffraid13 = {FRAME_RunAffraid13_start, FRAME_RunAffraid13_end, shammer_frames_run3, shammer_stand2};

mmove_t shammer_move_runaffraid21 = {FRAME_RunAffraid21_start, FRAME_RunAffraid21_end, shammer_frames_run1, shammer_runaffraid2};
mmove_t shammer_move_runaffraid22 = {FRAME_RunAffraid22_start, FRAME_RunAffraid22_end, shammer_frames_run2, NULL};
mmove_t shammer_move_runaffraid23 = {FRAME_RunAffraid23_start, FRAME_RunAffraid23_end, shammer_frames_run3, shammer_stand2};

void shammer_stand (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		{
		if (self->monsterinfo.currentmove == &shammer_move_walk22)
			self->monsterinfo.currentmove = &shammer_move_walk23;
		else if (self->monsterinfo.currentmove == &shammer_move_runaffraid22)
			self->monsterinfo.currentmove = &shammer_move_runaffraid23;
		else if (self->monsterinfo.currentmove == &shammer_move_run22)
			self->monsterinfo.currentmove = &shammer_move_run23;
		else if (self->monsterinfo.currentmove == &shammer_move_cower22)
			self->monsterinfo.currentmove = &shammer_move_cower23;
		else
			self->monsterinfo.currentmove = &shammer_move_stand2;
		}
	else
		{
		if (self->monsterinfo.currentmove == &shammer_move_walk12)
			self->monsterinfo.currentmove = &shammer_move_walk13;
		else if (self->monsterinfo.currentmove == &shammer_move_runaffraid12)
			self->monsterinfo.currentmove = &shammer_move_runaffraid13;
		else if (self->monsterinfo.currentmove == &shammer_move_run12)
			self->monsterinfo.currentmove = &shammer_move_run13;
		else if (self->monsterinfo.currentmove == &shammer_move_cower12)
			self->monsterinfo.currentmove = &shammer_move_cower13;
		else
			self->monsterinfo.currentmove = &shammer_move_stand1;
		}
}

void shammer_stand2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_stand2;
	else
		self->monsterinfo.currentmove = &shammer_move_stand1;
}


void shammer_walk (edict_t *self)
{
   self->enemy = NULL;
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_walk21;   
	else
		self->monsterinfo.currentmove = &shammer_move_walk11;
}

void shammer_walk2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_walk22;
	else
		self->monsterinfo.currentmove = &shammer_move_walk12;
}


void shammer_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &shammer_move_stand1;
	else 
		{
		if (self->flipping & FLIP_WEAPON)
			self->monsterinfo.currentmove = &shammer_move_run21;   
		else
			self->monsterinfo.currentmove = &shammer_move_run11;
		}
}

void shammer_run2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_run22;
	else
		self->monsterinfo.currentmove = &shammer_move_run12;
}

void shammer_runaffraid (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &shammer_move_stand1;
	else 
		{
		if (self->flipping & FLIP_WEAPON)
			self->monsterinfo.currentmove = &shammer_move_runaffraid21;   
		else
			self->monsterinfo.currentmove = &shammer_move_runaffraid11;
		}
}

void shammer_runaffraid2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_runaffraid22;
	else
		self->monsterinfo.currentmove = &shammer_move_runaffraid12;
}


mmove_t shammer_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, shammer_frames_action, shammer_runaffraid};
mmove_t shammer_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, shammer_frames_action, shammer_runaffraid};


void shammer_sight (edict_t *self, edict_t *other)
{
}

void shammer_dead (edict_t *self)
{
	VectorSet (self->mins, -31, -17, -1);
	VectorSet (self->maxs, 32, 14, 10);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mmove_t shammer_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, shammer_frames_action, shammer_dead};
mmove_t shammer_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, shammer_frames_action, shammer_dead};

mmove_t shammer_move_lookup = {FRAME_LookUp_start, FRAME_LookUp_end, shammer_frames_action, shammer_stand2};
mmove_t shammer_move_lookdown = {FRAME_LookDown_start, FRAME_LookDown_end, shammer_frames_action, shammer_stand2};
mmove_t shammer_move_gethammer = {FRAME_GetHammer_start, FRAME_GetHammer_end, shammer_frames_action, shammer_stand2};
mmove_t shammer_move_putaway = {FRAME_PutAway_start, FRAME_PutAway_end, shammer_frames_action, shammer_stand2};
mmove_t shammer_move_swing = {FRAME_Swing_start, FRAME_Swing_end, shammer_frames_action, shammer_stand2};
mmove_t shammer_move_wipe = {FRAME_Wipe_start, FRAME_Wipe_end, shammer_frames_action, shammer_stand2};

mmove_t shammer_move_talk11 = {FRAME_Talk11_start, FRAME_Talk11_end, shammer_frames_action, shammer_talk2};
mmove_t shammer_move_talk12 = {FRAME_Talk12_start, FRAME_Talk12_end, shammer_frames_action, NULL};
mmove_t shammer_move_talk13 = {FRAME_Talk13_start, FRAME_Talk13_end, shammer_frames_action, shammer_stand2};

mmove_t shammer_move_talk21 = {FRAME_Talk21_start, FRAME_Talk21_end, shammer_frames_action, shammer_talk2};
mmove_t shammer_move_talk22 = {FRAME_Talk22_start, FRAME_Talk22_end, shammer_frames_action, NULL};
mmove_t shammer_move_talk23 = {FRAME_Talk23_start, FRAME_Talk23_end, shammer_frames_action, shammer_stand2};

void shammer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
		self->monsterinfo.currentmove = &shammer_move_death1;
		}
	else 
		{	
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &shammer_move_death2;
		}
}

void shammer_pain (edict_t *self, edict_t *other, float kick, int damage)
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
		self->monsterinfo.currentmove = &shammer_move_pain1;
		}
	else
		{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &shammer_move_pain2;
		}	
}

void shammer_lookup (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_lookup;
}

void shammer_lookdown (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_lookdown;
}

void shammer_gethammer (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_gethammer;
	self->flipping |= FLIP_WEAPON;
	gi.sound (self, CHAN_WEAPON, sound_getham, 1, ATTN_NORM, 0);
}

void shammer_putaway (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_putaway;
	self->flipping &= ~FLIP_WEAPON;
	gi.sound (self, CHAN_WEAPON, sound_putaway, 1, ATTN_NORM, 0);
}

void shammer_swing (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_swing;
	gi.sound (self, CHAN_WEAPON, sound_swing, 1, ATTN_NORM, 0);
}

void shammer_wipe (edict_t *self)
{
	self->monsterinfo.currentmove = &shammer_move_wipe;
	gi.sound (self, CHAN_BODY, sound_wipe, 1, ATTN_NORM, 0);
}

void shammer_talk (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_talk21;
	else
		self->monsterinfo.currentmove = &shammer_move_talk11;
}

void shammer_talk2 (edict_t *self)
{
	if (self->flipping & FLIP_WEAPON)
		self->monsterinfo.currentmove = &shammer_move_talk22;
	else
		self->monsterinfo.currentmove = &shammer_move_talk12;
}


mmove_t shammer_move_decap = {FRAME_Decap_start, FRAME_Decap_end, shammer_frames_action, shammer_dead};

void shammer_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &shammer_move_decap;
	ThrowGib (self, "models/monsters/slave/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_shammer (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("shammer/pain1.wav");
	sound_pain2 = gi.soundindex ("shammer/pain2.wav");
	sound_die1 = gi.soundindex ("shammer/death1.wav");
	sound_die2 = gi.soundindex ("shammer/death2.wav");

	sound_rocketshot = gi.soundindex ("shammer/fire.wav");

	sound_melee_hit = gi.soundindex ("shammer/melee.wav");
	sound_sight = gi.soundindex ("shammer/sight.wav");
	sound_search = gi.soundindex ("shammer/search.wav");
	sound_idle = gi.soundindex ("shammer/idle.wav");
   */

	sound_pain1    = gi.soundindex ("monsters/slave/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/slave/die1.wav");
	sound_die2     = gi.soundindex ("monsters/slave/die2.wav");
	sound_putaway  = gi.soundindex ("monsters/slave/putaway.wav");
	sound_getham	= gi.soundindex ("monsters/slave/getham.wav");
	sound_swing    = gi.soundindex ("monsters/slave/swing.wav");
	sound_wipe     = gi.soundindex ("monsters/slave/wipe.wav");
	sound_gib	   = gi.soundindex ("monsters/slave/gib.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave/hammer/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/slave/hammer/hammer.md2");
	VectorSet (self->mins, -3, -13, -1);
	VectorSet (self->maxs, 13, 13, 57);

   if (!self->health)
	   self->health = 60;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = shammer_pain;
	self->die = shammer_die;

   self->monsterinfo.stand = shammer_stand;
	self->monsterinfo.walk = shammer_walk;
	self->monsterinfo.run = shammer_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = shammer_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;

	self->monsterinfo.action[0] = shammer_lookup;
	self->monsterinfo.action[1] = shammer_lookdown;
	self->monsterinfo.action[2] = shammer_gethammer;
	self->monsterinfo.action[3] = shammer_putaway;
	self->monsterinfo.action[4] = shammer_swing;
	self->monsterinfo.action[5] = shammer_wipe;
	self->monsterinfo.action[6] = shammer_talk;
	self->monsterinfo.action[7] = shammer_cower;
	self->monsterinfo.decap = shammer_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);

	// No attack
	self->spawnflags |= 32;

   walkmonster_start (self);
		
   if (self->spawnflags & SHAMMER_WALK)
      self->monsterinfo.currentmove = &shammer_move_walk12;
   else
		self->monsterinfo.currentmove = &shammer_move_stand1;

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
