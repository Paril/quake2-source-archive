/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_msten.c
  Description: AI/Physics for Sten Dragon. 

\**********************************************************/

#include "g_local.h"
#include "b_msten.h"

void find_node_hide (edict_t *self);
void sten_pain (edict_t *self, edict_t *other, float kick, int damage);
void sten_smack1 (edict_t *self);      // bite
void sten_smack2 (edict_t *self);      // claw
void sten_smack3 (edict_t *self);      // pounce

#define STEN_CROUCH 8
#define STEN_WALK 16

static int	sound_step;
static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_gib;


void sten_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t sten_frames_stand [] =
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
	BOT_stand, 0, NULL
};
mmove_t sten_move_stand = {FRAME_Stand_start, FRAME_Stand_end, sten_frames_stand, NULL};

void sten_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &sten_move_stand;
}

mframe_t sten_frames_crouch [] =
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
	BOT_stand, 0, NULL
};

mmove_t sten_move_crouch = {FRAME_Crouch_start, FRAME_Crouch_end, sten_frames_crouch, NULL};

void sten_crouch (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &sten_move_crouch;
}


mframe_t sten_frames_cpain [] =
{
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};

mmove_t sten_move_cpain = {FRAME_Cpain_start, FRAME_Cpain_end, sten_frames_cpain, NULL};


mframe_t sten_frames_walk [] =
{
	BOT_walk, 0,  sten_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL,
	BOT_walk, 0,  sten_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL
};
mmove_t sten_move_walk = {FRAME_Walk_start, FRAME_Walk_end, sten_frames_walk, NULL};

void sten_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &sten_move_walk;
}

mframe_t sten_frames_run [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 26, sten_step,
	BOT_run, 26, NULL,
	BOT_run, 0, NULL,
	BOT_run, 26, sten_step,
};
mmove_t sten_move_run = {FRAME_Run_start, FRAME_Run_end, sten_frames_run, NULL};

void sten_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &sten_move_stand;
	else
		self->monsterinfo.currentmove = &sten_move_run;
}


mframe_t sten_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t sten_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, sten_frames_pain1, sten_run};

mframe_t sten_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t sten_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, sten_frames_pain2, sten_run};

void sten_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void sten_dead (edict_t *self)
{
	self->mins[2] = -1;
	self->maxs[2] = 15;
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t sten_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t sten_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, sten_frames_death1, sten_dead};

mframe_t sten_frames_death2 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t sten_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, sten_frames_death2, sten_dead};

void sten_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/sten/gibs/leg_f_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/sten/gibs/leg_f_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/sten/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/sten/gibs/leg_b_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/sten/gibs/leg_b_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/sten/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &sten_move_death1;

	n = rand() & 1;
	if (n)
      {
	   self->monsterinfo.currentmove = &sten_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
      }
	else
      {
	   self->monsterinfo.currentmove = &sten_move_death2;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
      }
}


mframe_t sten_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sten_smack1,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t sten_move_attack1 = {FRAME_Bite_start, FRAME_Bite_end, sten_frames_attack1, sten_run};

mframe_t sten_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sten_smack2,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t sten_move_attack2 = {FRAME_Claw_start, FRAME_Claw_end, sten_frames_attack2, sten_run};

mframe_t sten_frames_attack3 [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 15, NULL,
	BOT_charge, 35, NULL,
	BOT_charge, 44, NULL,
	BOT_charge, 42, NULL,
	BOT_charge, 24, NULL,
	BOT_charge, 13, sten_smack3,
   BOT_charge, 3, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t sten_move_attack3 = {FRAME_Pounce_start, FRAME_Pounce_end, sten_frames_attack3, sten_run};

void sten_attack(edict_t *self)
{   
   self->monsterinfo.nextattack = level.time + 1;
	if (range (self, self->enemy) > RANGE_NEAR)
      return;
	if (range (self, self->enemy) == RANGE_MELEE)
      {
      if (rand() & 1)
         {
	      self->monsterinfo.currentmove = &sten_move_attack2;
			gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
         self->monsterinfo.nextattack = level.time + 1;
         }
	   else
         {
         self->monsterinfo.nextattack = level.time + 2;
		   self->monsterinfo.currentmove = &sten_move_attack1;
			gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
         }
      }
   else
      {
      self->monsterinfo.nextattack = level.time + 2;
      if ((rand() & 3) != 3)
         return;
		gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
      self->monsterinfo.currentmove = &sten_move_attack3;
      }
}

void sten_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] = 16;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void sten_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void sten_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 24;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}
/*
void sten_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &sten_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &sten_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &sten_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &sten_move_straferight;
}
*/

void sten_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &sten_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &sten_move_pain2;
      }

}

void sten_smack1 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20, 50);
}

void sten_smack2 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20, 50);
}


void sten_smack3 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 40, 50);
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_sten (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

   /*
	sound_pain1 = gi.soundindex ("makron/pain1.wav");
	sound_pain2 = gi.soundindex ("makron/pain2.wav");
	sound_die1 = gi.soundindex ("gladiator/glddeth2.wav");
	sound_die2 = gi.soundindex ("hover/hovdeth2.wav");

	sound_rocketshot = gi.soundindex ("priest/fire.wav");

	sound_melee_hit = gi.soundindex ("gladiator/melee1.wav");
	sound_sight = gi.soundindex ("gunner/gunsrch1.wav");
	sound_search = gi.soundindex ("priest/search.wav");
	sound_step = gi.soundindex ("mutant/step3.wav");
	sound_idle = gi.soundindex ("priest/idle.wav");
   */

	sound_step     = gi.soundindex ("monsters/sten/step.wav");
	sound_sight    = gi.soundindex ("monsters/sten/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/sten/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/sten/attack2.wav");
	sound_attack3  = gi.soundindex ("monsters/sten/attack3.wav");
	sound_pain1    = gi.soundindex ("monsters/sten/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/sten/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/sten/die1.wav");
	sound_die2     = gi.soundindex ("monsters/sten/die2.wav");
	sound_gib      = gi.soundindex ("monsters/sten/gib.wav");
   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/sten/tris.md2");
	VectorSet (self->mins, -24, -16, -2);
	VectorSet (self->maxs, 24, 10, 24);

	if (!self->health)
      self->health = 140;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 50;

	self->pain = sten_pain;
	self->die = sten_die;

   self->monsterinfo.stand = sten_stand;
	self->monsterinfo.walk = sten_walk;
	self->monsterinfo.run = sten_run;
	//self->monsterinfo.dodge = sten_dodge;
	self->monsterinfo.attack = sten_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = sten_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;


   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & STEN_CROUCH)
		self->monsterinfo.currentmove = &sten_move_crouch;
   else if (self->spawnflags & STEN_WALK)
		self->monsterinfo.currentmove = &sten_move_walk;
   else
		self->monsterinfo.currentmove = &sten_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_FLAT;
   gi.linkentity(self);
}

