/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mguard.c
  Description: AI/Physics for Guard 

\**********************************************************/

#include "g_local.h"
#include "b_mguard.h"

void find_node_hide (edict_t *self);
void guard_pain (edict_t *self, edict_t *other, float kick, int damage);
void guard_smack1 (edict_t *self);
void guard_smack2 (edict_t *self);
void guard_stand2	(edict_t *self);
void guard_crouch2 (edict_t *self);
void guard_walk2 (edict_t *self);

#define GUARD_CROUCH 8
#define GUARD_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_gib;
static int	sound_step;


void guard_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t guard_frames_action [] =
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

mframe_t guard_frames_stand [] =
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
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL
};
mmove_t guard_move_stand = {FRAME_Stand_start, FRAME_Stand_end, guard_frames_stand, NULL};
mmove_t guard_move_idle1 = {FRAME_Idle1_start, FRAME_Idle1_end, guard_frames_stand, guard_stand2};
mmove_t guard_move_idle2 = {FRAME_Idle2_start, FRAME_Idle2_end, guard_frames_stand, guard_stand2};
mmove_t guard_move_idle3 = {FRAME_Idle3_start, FRAME_Idle3_end, guard_frames_stand, guard_stand2};
mmove_t guard_move_crouch1 = {FRAME_Crouch1_start, FRAME_Crouch1_end, guard_frames_stand, guard_crouch2};
mmove_t guard_move_crouch2 = {FRAME_Crouch2_start, FRAME_Crouch2_end, guard_frames_stand, NULL};
mmove_t guard_move_crouch3 = {FRAME_Crouch3_start, FRAME_Crouch3_end, guard_frames_stand, guard_stand2};

mmove_t guard_move_talk = {FRAME_Talk_start, FRAME_Talk_end, guard_frames_action, guard_stand2};
mmove_t guard_move_button = {FRAME_Button_start, FRAME_Button_end, guard_frames_action, guard_stand2};
mmove_t guard_move_point = {FRAME_Point_start, FRAME_Point_end, guard_frames_action, guard_stand2};




mframe_t guard_frames_walk1 [] =
{
	BOT_walk, 1, NULL,
	BOT_walk, 2, NULL,
	BOT_walk, 3, guard_step,
	BOT_walk, 4, NULL
};
mframe_t guard_frames_walk2 [] =
{
	BOT_walk, 0, NULL,
	BOT_walk, 4, NULL,
	BOT_walk, 4, NULL,
	BOT_walk, 4, guard_step,
	BOT_walk, 4, NULL,
	BOT_walk, 4, NULL,
	BOT_walk, 0, NULL,
	BOT_walk, 4, NULL,
	BOT_walk, 4, NULL,
	BOT_walk, 4, guard_step,
	BOT_walk, 4, NULL,
	BOT_walk, 4, NULL
};
mframe_t guard_frames_walk3 [] =
{
	BOT_walk, 4, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 2, guard_step,
	BOT_walk, 1, NULL
};
mmove_t guard_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, guard_frames_walk1, guard_walk2};
mmove_t guard_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, guard_frames_walk2, NULL};
mmove_t guard_move_walk3 = {FRAME_Walk3_start, FRAME_Walk3_end, guard_frames_walk3, guard_stand2};

void guard_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &guard_move_walk2)
		self->monsterinfo.currentmove = &guard_move_walk3;
	else if (self->monsterinfo.currentmove == &guard_move_crouch2)
		self->monsterinfo.currentmove = &guard_move_crouch3;
	else
		self->monsterinfo.currentmove = &guard_move_stand;
}

void guard_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_stand;
}

void guard_crouch (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_crouch1;
}

void guard_crouch2 (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_crouch2;
}

void guard_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_walk1;
}

void guard_walk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_walk2;
}

mframe_t guard_frames_run [] =
{
	BOT_run, 0,  guard_step,
	BOT_run, 24, NULL,
	BOT_run, 24, NULL,
	BOT_run, 0,  guard_step,
	BOT_run, 24, NULL,
	BOT_run, 24, NULL
};
mmove_t guard_move_run = {FRAME_Run_start, FRAME_Run_end, guard_frames_run, NULL};

void guard_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &guard_move_stand;
	else
		self->monsterinfo.currentmove = &guard_move_run;
}


mframe_t guard_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t guard_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, guard_frames_pain1, guard_run};

mframe_t guard_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, 0, NULL,
	BOT_move, -1, NULL
};
mmove_t guard_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, guard_frames_pain2, guard_run};

void guard_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void guard_dead (edict_t *self)
{
	VectorSet (self->mins, -43, -24, -3);
	VectorSet (self->maxs, 34, 10, 8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t guard_frames_death [] =
{
	BOT_move, -4, NULL,
	BOT_move, -2, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t guard_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, guard_frames_death, guard_dead};
mmove_t guard_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, guard_frames_death, guard_dead};

void guard_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/guard/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guard/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guard/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guard/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guard/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/guard/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	

	if (rand() & 1)
      {
      self->monsterinfo.currentmove = &guard_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
      }
	else
      {
      self->monsterinfo.currentmove = &guard_move_death2;
      gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
      }
}

mframe_t guard_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  guard_smack1,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t guard_move_attack1 = {FRAME_Attack1_start, FRAME_Attack1_end, guard_frames_attack1, guard_run};

mframe_t guard_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  guard_smack1,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  guard_smack1,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t guard_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, guard_frames_attack2, guard_run};
mmove_t guard_move_attack3 = {FRAME_Attack3_start, FRAME_Attack3_end, guard_frames_attack1, guard_run};

mframe_t guard_frames_attack3 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, -3, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 6,  NULL,
	BOT_charge, 5,  NULL,
	BOT_charge, 9,  guard_smack2,
	BOT_charge, 0,  NULL,
	BOT_charge, 1,  NULL,
	BOT_charge, 1,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t guard_move_attack4 = {FRAME_Polejump_start, FRAME_Polejump_end, guard_frames_attack3, guard_run};

void guard_attack(edict_t *self)
{
   self->monsterinfo.nextattack = level.time + 1;
   if (range (self, self->enemy) != RANGE_MELEE)
      return;
   self->monsterinfo.nextattack = level.time + 1;
	switch (rand() & 3)
		{
		case 0:
			self->monsterinfo.currentmove = &guard_move_attack1;
			break;
		case 1:
			self->monsterinfo.currentmove = &guard_move_attack2;
			break;
		case 2:
			self->monsterinfo.currentmove = &guard_move_attack3;
			break;
		case 3:
			self->monsterinfo.currentmove = &guard_move_attack4;
			break;
		}
}


mframe_t guard_frames_strafeleft [] =
{
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 2, NULL
};
mmove_t guard_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, guard_frames_strafeleft, guard_run};

mframe_t guard_frames_straferight [] =
{
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 2, NULL
};
mmove_t guard_move_straferight = {FRAME_Right_start, FRAME_Right_end, guard_frames_straferight, guard_run};

void guard_dodge (edict_t *self, edict_t *attacker, float eta)
{
	vec3_t	offset, forward, right, end;
   trace_t  trace;

	if (!self->enemy)
		self->enemy = attacker;

   VectorSet(offset, 0, 90, 0);
   AngleVectors (self->s.angles, forward, right, NULL);

   if (random() < .5)
      {
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &guard_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &guard_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &guard_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &guard_move_straferight;
}

void guard_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

	if (level.time < self->pain_debounce_time)
		return;

   if (!self->s.skinnum && self->health < self->max_health / 3)
		{
      self->s.skinnum = 1;
		gi.linkentity(self);
		}	
	
	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() & 1;

	if (n == 0)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &guard_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &guard_move_pain2;
      }
}

void guard_smack1 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 20 + (rand() & 15), 30))
	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
}

void guard_smack2 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 40 + (rand() & 15), 550))
	gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
}

void guard_talk (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_talk;
}

void guard_button (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_button;
}

void guard_point (edict_t *self)
{
	self->monsterinfo.currentmove = &guard_move_point;
}

void guard_idle (edict_t *self)
{
	float n = random();
	if (n < .33)
		self->monsterinfo.currentmove = &guard_move_idle1;
	else if (n < .66)
		self->monsterinfo.currentmove = &guard_move_idle2;
	else
		self->monsterinfo.currentmove = &guard_move_idle3;
}

mmove_t guard_move_decap = {FRAME_Decap_start, FRAME_Decap_end, guard_frames_death, guard_dead};

void guard_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &guard_move_decap;
   gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
	ThrowGib (self, "models/monsters/guard/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_guard (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("monsters/guard/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/guard/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/guard/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/guard/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/guard/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/guard/die1.wav");
	sound_die2     = gi.soundindex ("monsters/guard/die2.wav");
	sound_gib      = gi.soundindex ("monsters/guard/gib.wav");
	sound_step		= gi.soundindex ("monsters/guard/step.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/guard/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/guard/staff.md2");
	VectorSet (self->mins, -6, -16, 0);
	VectorSet (self->maxs, 10, 11, 60);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 200;

	self->pain = guard_pain;
	self->die = guard_die;

   self->monsterinfo.stand = guard_stand;
	self->monsterinfo.walk = guard_walk;
	self->monsterinfo.run = guard_run;
	self->monsterinfo.dodge = guard_dodge;
	self->monsterinfo.attack = guard_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = guard_sight;
	self->monsterinfo.idle = guard_idle;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.idle_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
   self->monsterinfo.decap = guard_decap;

	self->monsterinfo.action[0] = guard_crouch;
	self->monsterinfo.action[1] = guard_button;
	self->monsterinfo.action[2] = guard_point;
	self->monsterinfo.action[3] = guard_talk;

   
	self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & GUARD_CROUCH)
		self->monsterinfo.currentmove = &guard_move_crouch1;
   else if (self->spawnflags & GUARD_WALK)
		self->monsterinfo.currentmove = &guard_move_walk1;
   else
		self->monsterinfo.currentmove = &guard_move_stand;
 	
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD;
}