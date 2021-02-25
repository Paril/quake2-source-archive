/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mbactn.c
  Description: AI/Physics for Bact'n 

\**********************************************************/

#include "g_local.h"
#include "b_mbactn.h"

void bactn_pain (edict_t *self, edict_t *other, float kick, int damage);
void bactn_smack (edict_t *self);
void bactn_stand2 (edict_t *self);
void bactn_walk2 (edict_t *self);
void bactn_run2 (edict_t *self);
void bactn_runattack2 (edict_t *self);
void bactn_ready (edict_t *self);
void bactn_ready2 (edict_t *self);
void bactn_crouch (edict_t *self);
void fire_teargas (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rock (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);

#define BACTN_CROUCH 8
#define BACTN_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_die1;
static int	sound_die2;
static int	sound_die3;
static int	sound_gib;


mframe_t bactn_frames_action [] =
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

mframe_t bactn_frames_stand [] =
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
mmove_t bactn_move_stand = {FRAME_Stand_start, FRAME_Stand_end, bactn_frames_stand, NULL};

mmove_t bactn_move_getready = {FRAME_GetReady_start, FRAME_GetReady_end, bactn_frames_stand, bactn_ready2};
mmove_t bactn_move_ready = {FRAME_Ready_start, FRAME_Ready_end, bactn_frames_stand, NULL};
mmove_t bactn_move_readystand = {FRAME_GetReady_start, FRAME_GetReady_end, bactn_frames_stand, bactn_stand2};

mmove_t bactn_move_duck = {FRAME_Duck_start, FRAME_Duck_end, bactn_frames_stand, bactn_crouch};
mmove_t bactn_move_standup = {FRAME_DuckStand_start, FRAME_DuckStand_end, bactn_frames_stand, bactn_stand2};
mmove_t bactn_move_crouch = {FRAME_Crouch_start, FRAME_Crouch_end, bactn_frames_stand, NULL};

void bactn_crouch (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_crouch;
}


mframe_t bactn_frames_walk1 [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};
mframe_t bactn_frames_walk2 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL
};
mframe_t bactn_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL,
	BOT_walk, 0,  NULL
};

mmove_t bactn_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, bactn_frames_walk1, bactn_walk2};
mmove_t bactn_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, bactn_frames_walk2, NULL};
mmove_t bactn_move_walk3= {FRAME_Walk3_start, FRAME_Walk3_end, bactn_frames_walk3, bactn_stand2};
mframe_t bactn_frames_run1 [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 4, NULL,
	BOT_run, 8, NULL,
};
mframe_t bactn_frames_run2 [] =
{
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 0,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
};
mframe_t bactn_frames_run3 [] =
{
	BOT_run, 8, NULL,
	BOT_run, 4, NULL,
	BOT_run, 0,  NULL
};
mmove_t bactn_move_run1 = {FRAME_Run1_start, FRAME_Run1_end, bactn_frames_run1, bactn_run2};
mmove_t bactn_move_run2 = {FRAME_Run2_start, FRAME_Run2_end, bactn_frames_run2, NULL};
mmove_t bactn_move_run3 = {FRAME_Run3_start, FRAME_Run3_end, bactn_frames_run3, bactn_stand2};
mmove_t bactn_move_runattack1 = {FRAME_RunAttack1_start, FRAME_RunAttack1_end, bactn_frames_run1, bactn_runattack2};
mmove_t bactn_move_runattack2 = {FRAME_RunAttack2_start, FRAME_RunAttack2_end, bactn_frames_run2, NULL};
mmove_t bactn_move_runattack3 = {FRAME_RunAttack3_start, FRAME_RunAttack3_end, bactn_frames_run3, bactn_ready2};

void bactn_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &bactn_move_walk2)
		self->monsterinfo.currentmove = &bactn_move_walk3;
	else if (self->monsterinfo.currentmove == &bactn_move_run2)
		self->monsterinfo.currentmove = &bactn_move_run3;	
	else if (self->monsterinfo.currentmove == &bactn_move_runattack2)
		self->monsterinfo.currentmove = &bactn_move_runattack3;
	else if (self->monsterinfo.currentmove == &bactn_move_crouch)
		self->monsterinfo.currentmove = &bactn_move_ready;
	else if (self->monsterinfo.currentmove == &bactn_move_readystand)
		self->monsterinfo.currentmove = &bactn_move_standup;
	else
		self->monsterinfo.currentmove = &bactn_move_stand;
}

void bactn_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_stand;
}


void bactn_ready (edict_t *self)
{
	if (self->monsterinfo.currentmove == &bactn_move_run2)
		self->monsterinfo.currentmove = &bactn_move_run3;
	else if (self->monsterinfo.currentmove == &bactn_move_runattack2)
		self->monsterinfo.currentmove = &bactn_move_runattack3;
	else if (self->monsterinfo.currentmove == &bactn_move_crouch)
		self->monsterinfo.currentmove = &bactn_move_standup;
	else
		self->monsterinfo.currentmove = &bactn_move_ready;
}

void bactn_ready2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_ready;
}

void bactn_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &bactn_move_walk1;   
}

void bactn_walk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_walk2;
}


void bactn_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &bactn_move_stand;
	else if (self->monsterinfo.currentmove == &bactn_move_ready)
		self->monsterinfo.currentmove = &bactn_move_runattack1;
	else 
		self->monsterinfo.currentmove = &bactn_move_run1;
}

void bactn_run2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_run2;
}

void bactn_runattack2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_runattack2;
}


mframe_t bactn_frames_pain [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t bactn_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, bactn_frames_pain, bactn_run};
mmove_t bactn_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, bactn_frames_pain, bactn_run};
mmove_t bactn_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, bactn_frames_pain, bactn_run};


void bactn_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void bactn_dead (edict_t *self)
{
	VectorSet (self->mins, -21, -17, 0);
	VectorSet (self->maxs, 48, 12, 10);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t bactn_frames_death [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t bactn_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, bactn_frames_death, bactn_dead};
mmove_t bactn_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, bactn_frames_death, bactn_dead};
mmove_t bactn_move_death3 = {FRAME_Die3_start, FRAME_Die3_end, bactn_frames_death, bactn_dead};

void bactn_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   float n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/bactn/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/bactn/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/bactn/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/bactn/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/bactn/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/bactn/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = random();
	if (n < .33)
		{
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &bactn_move_death1;
		}
	else if (n < .66)
		{	
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &bactn_move_death2;
		}
	else
		{
		gi.sound (self, CHAN_VOICE, sound_die3, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &bactn_move_death3;
		}
}

void bactn_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	//offset[0] = self->enemy->s.origin[0];
	//offset[1] = self->enemy->s.origin[1];
	//offset[2] = self->enemy->s.origin[2] + 64;
   VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);
	VectorSet(offset, 20, -8, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	fire_rock (self, start, forward, 17, 900);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
}

void bactn_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 10 + (rand() & 15), 50);
}

void bactn_smack2 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20 + (rand() & 15), 550);
}

mframe_t bactn_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, bactn_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t bactn_move_attack1 = {FRAME_Attack1_start, FRAME_Attack1_end, bactn_frames_attack1, bactn_run};
mmove_t bactn_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, bactn_frames_attack1, bactn_run};
mmove_t bactn_move_attack3 = {FRAME_Attack3_start, FRAME_Attack3_end, bactn_frames_attack1, bactn_run};
mmove_t bactn_move_attack6 = {FRAME_Attack6_start, FRAME_Attack6_end, bactn_frames_attack1, bactn_run};

mframe_t bactn_frames_attack2 [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, bactn_smack2,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t bactn_move_attack4 = {FRAME_Attack4_start, FRAME_Attack4_end, bactn_frames_attack2, bactn_run};

mframe_t bactn_frames_attack3 [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, bactn_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t bactn_move_attack5 = {FRAME_Attack5_start, FRAME_Attack5_end, bactn_frames_attack3, bactn_run};

void bactn_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
      {
		switch (rand() & 3)
			{
			case 0:
				gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
				self->monsterinfo.currentmove = &bactn_move_attack1;
				self->monsterinfo.nextattack = level.time + 1;	
				break;
			case 1:
				gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
				self->monsterinfo.currentmove = &bactn_move_attack2;
				self->monsterinfo.nextattack = level.time + 1;	
				break;
			case 2:
				gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
				self->monsterinfo.currentmove = &bactn_move_attack3;
				self->monsterinfo.nextattack = level.time + 1;	
				break;
			case 3:
				gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
				self->monsterinfo.currentmove = &bactn_move_attack4;
				self->monsterinfo.nextattack = level.time + 1;	
				break;
			}
      }
	else
      {
		gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
      self->monsterinfo.nextattack = level.time + 2;
		self->monsterinfo.currentmove = &bactn_move_attack5;
      }
}


mframe_t bactn_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 20, NULL,
	BOT_move_left, 16, NULL,
	BOT_move_left, 0, NULL
};
mmove_t bactn_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, bactn_frames_strafeleft, bactn_run};

mframe_t bactn_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 20, NULL,
	BOT_move_right, 16, NULL,
	BOT_move_right, 0, NULL
};
mmove_t bactn_move_straferight = {FRAME_Right_start, FRAME_Right_end, bactn_frames_straferight, bactn_run};

void bactn_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &bactn_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &bactn_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &bactn_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &bactn_move_straferight;
}

void bactn_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

   if (self->health < self->max_health / 3)
 		{
      self->s.skinnum = 1;
		gi.linkentity(self);
		}
   
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = random();

	if (n < .33)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &bactn_move_pain1;
      }
	else if (n < .66)
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &bactn_move_pain2;
      }
	else 
      {
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &bactn_move_pain3;
      }

}


mmove_t bactn_move_wave1 = {FRAME_Wave1_start, FRAME_Wave1_end, bactn_frames_action, NULL};
mmove_t bactn_move_wave2 = {FRAME_Wave2_start, FRAME_Wave2_end, bactn_frames_action, NULL};
mmove_t bactn_move_wave3 = {FRAME_Wave3_start, FRAME_Wave3_end, bactn_frames_action, NULL};
mmove_t bactn_move_wave4 = {FRAME_Wave4_start, FRAME_Wave4_end, bactn_frames_action, NULL};
mmove_t bactn_move_button = {FRAME_Button_start, FRAME_Button_end, bactn_frames_action, NULL};
mmove_t bactn_move_bow = {FRAME_Bow_start, FRAME_Bow_end, bactn_frames_action, NULL};
mmove_t bactn_move_talk1 = {FRAME_Talk1_start, FRAME_Talk1_end, bactn_frames_action, NULL};
mmove_t bactn_move_talk2 = {FRAME_Talk2_start, FRAME_Talk2_end, bactn_frames_action, NULL};
mmove_t bactn_move_laugh = {FRAME_Laugh_start, FRAME_Laugh_end, bactn_frames_action, NULL};
mmove_t bactn_move_breath = {FRAME_Breath_start, FRAME_Breath_end, bactn_frames_action, NULL};

void bactn_wave1 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_wave1;
}

void bactn_wave2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_wave2;
}

void bactn_wave3 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_wave3;
}

void bactn_wave4 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_wave4;
}

void bactn_button (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_button;
}

void bactn_bow (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_bow;
}

void bactn_talk1 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_talk1;
}

void bactn_talk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_talk2;
}

void bactn_laugh (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_laugh;
}

void bactn_breath (edict_t *self)
{
	self->monsterinfo.currentmove = &bactn_move_breath;
}


/*
mmove_t bactn_move_decap = {FRAME_decapStart, FRAME_decapEnd, bactn_frames_death1, bactn_dead};

void bactn_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &bactn_move_decap;
	gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	ThrowGib (self, "models/monsters/bactn/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}
*/

static void rock_hit (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   float	   points;
	vec3_t	v;
	vec3_t	dir;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	   {
		G_FreeEdict (ent);
		return;
	   }

	if (!other->takedamage || (ent->owner->team_data == other->team_data))
		return;

	VectorAdd (other->mins, other->maxs, v);
	VectorMA (other->s.origin, 0.5, v, v);
	VectorSubtract (ent->s.origin, v, v);
	points = ent->dmg - 0.5 * VectorLength (v);
	VectorSubtract (other->s.origin, ent->s.origin, dir);
	T_Damage (other, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, 0);
	ent->touch = NULL;
}

void fire_rock (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*rock;

	rock = G_Spawn();
	VectorCopy (start, rock->s.origin);
	VectorScale (aimdir, speed, rock->velocity);
	VectorSet (rock->avelocity, 300, 300, 300);
	rock->movetype = MOVETYPE_FLYARC;
	rock->clipmask = MASK_SHOT;
	rock->solid = SOLID_BBOX;
	//rock->s.effects |= EF_GRENADE;
	rock->s.modelindex = gi.modelindex ("models/objects/rock/tris.md2");
	rock->owner = self;
	rock->touch = rock_hit;
	rock->nextthink = level.time + 5;
	rock->think = G_FreeEdict;
	rock->dmg = damage;
	gi.linkentity (rock);
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_bactn (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("bactn/pain1.wav");
	sound_pain2 = gi.soundindex ("bactn/pain2.wav");
	sound_die1 = gi.soundindex ("bactn/death1.wav");
	sound_die2 = gi.soundindex ("bactn/death2.wav");

	sound_rocketshot = gi.soundindex ("bactn/fire.wav");

	sound_melee_hit = gi.soundindex ("bactn/melee.wav");
	sound_sight = gi.soundindex ("bactn/sight.wav");
	sound_search = gi.soundindex ("bactn/search.wav");
	sound_idle = gi.soundindex ("bactn/idle.wav");
   */

	sound_sight    = gi.soundindex ("monsters/bactn/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/bactn/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/bactn/attack2.wav");
	sound_attack3  = gi.soundindex ("monsters/bactn/attack3.wav");
	sound_pain1    = gi.soundindex ("monsters/bactn/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/bactn/pain2.wav");
	sound_pain3    = gi.soundindex ("monsters/bactn/pain3.wav");
	sound_die1     = gi.soundindex ("monsters/bactn/die1.wav");
	sound_die2     = gi.soundindex ("monsters/bactn/die2.wav");
	sound_die3     = gi.soundindex ("monsters/bactn/die3.wav");
	sound_gib	   = gi.soundindex ("monsters/bactn/gib.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/bactn/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/bactn/staff.md2");
	VectorSet (self->mins, -3, -16, 0);
	VectorSet (self->maxs, 16, 13, 60);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = bactn_pain;
	self->die = bactn_die;

   self->monsterinfo.stand = bactn_stand;
	self->monsterinfo.walk = bactn_walk;
	self->monsterinfo.run = bactn_run;
	self->monsterinfo.dodge = bactn_dodge;
	self->monsterinfo.attack = bactn_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = bactn_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
	//self->monsterinfo.action[0] = bactn_meditate;
	//self->monsterinfo.action[1] = bactn_medistand;

	//self->monsterinfo.decap = bactn_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	self->monsterinfo.action[0]  = bactn_stand;
	self->monsterinfo.action[1]  = bactn_ready;
	self->monsterinfo.action[2]  = bactn_crouch;
	self->monsterinfo.action[3]  = bactn_wave1;
	self->monsterinfo.action[4]  = bactn_wave2;
	self->monsterinfo.action[5]  = bactn_wave3;
	self->monsterinfo.action[6]  = bactn_wave4;
	self->monsterinfo.action[7]  = bactn_button;
	self->monsterinfo.action[8]  = bactn_bow;
	self->monsterinfo.action[9]  = bactn_talk1;
	self->monsterinfo.action[10] = bactn_talk2;
	self->monsterinfo.action[11] = bactn_laugh;
	self->monsterinfo.action[12] = bactn_breath;

	gi.linkentity (self);

   walkmonster_start (self);
		
   if (self->spawnflags & BACTN_CROUCH)
      self->monsterinfo.currentmove = &bactn_move_crouch;
   else if (self->spawnflags & BACTN_WALK)
      self->monsterinfo.currentmove = &bactn_move_walk2;
   else
		self->monsterinfo.currentmove = &bactn_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD;
}
