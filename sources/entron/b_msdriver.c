/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_msdriver.c
  Description: AI/Physics for Slave Driver

\**********************************************************/

#include "g_local.h"
#include "b_msdriver.h"

void find_node_hide (edict_t *self);
void sdriver_pain (edict_t *self, edict_t *other, float kick, int damage);
void sdriver_smack1 (edict_t *self);
void sdriver_smack2 (edict_t *self);
void laser_attack2(edict_t *self);
void fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod);

#define SLAVE_EASE 8
#define SLAVE_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_gib;
static int	sound_step;

mframe_t sdriver_frames_stand [] =
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
	BOT_stand, 0, NULL
};
mmove_t sdriver_move_stand = {FRAME_Stand_start, FRAME_Stand_end, sdriver_frames_stand, NULL};
mmove_t sdriver_move_ease = {FRAME_Ease_start, FRAME_Ease_end, sdriver_frames_stand, NULL};

void sdriver_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_stand;
}

void sdriver_ease (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &sdriver_move_ease;
}

mframe_t sdriver_frames_talk [] =
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
	BOT_move, 0, NULL
};
mmove_t sdriver_move_talk1= {FRAME_Talk1_start, FRAME_Talk1_end, sdriver_frames_talk, sdriver_ease};
mmove_t sdriver_move_talk2 = {FRAME_Talk2_start, FRAME_Talk2_end, sdriver_frames_talk, sdriver_ease};
mmove_t sdriver_move_talk3 = {FRAME_Talk3_start, FRAME_Talk3_end, sdriver_frames_talk, sdriver_ease};
mmove_t sdriver_move_give = {FRAME_Give_start, FRAME_Give_end, sdriver_frames_talk, sdriver_ease};
mmove_t sdriver_move_grab = {FRAME_Grab_start, FRAME_Grab_end, sdriver_frames_talk, sdriver_stand};

void sdriver_talk1 (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_talk1;
}

void sdriver_talk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_talk2;
}

void sdriver_talk3 (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_talk3;
}

void sdriver_give (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_give;
}

void sdriver_grab (edict_t *self)
{
	self->monsterinfo.currentmove = &sdriver_move_grab;
}

void sdriver_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t sdriver_frames_walk [] =
{
	BOT_walk, 3,  sdriver_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
   BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  sdriver_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL
};
mmove_t sdriver_move_walk = {FRAME_Walk_start, FRAME_Walk_end, sdriver_frames_walk, NULL};

void sdriver_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &sdriver_move_walk;
}

mframe_t sdriver_frames_run [] =
{
	BOT_run, 3,  sdriver_step,
	BOT_run, 7,  NULL,
	BOT_run, 12, NULL,
	BOT_run, 3,  sdriver_step,
	BOT_run, 7,	 NULL,
	BOT_run, 12, NULL
};
mmove_t sdriver_move_run = {FRAME_Run_start, FRAME_Run_end, sdriver_frames_run, NULL};

void sdriver_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &sdriver_move_stand;
	else
		self->monsterinfo.currentmove = &sdriver_move_run;
}


mframe_t sdriver_frames_pain [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t sdriver_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, sdriver_frames_pain, sdriver_run};
mmove_t sdriver_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, sdriver_frames_pain, sdriver_run};
mmove_t sdriver_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, sdriver_frames_pain, sdriver_run};

void sdriver_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void sdriver_dead (edict_t *self)
{
	VectorSet (self->mins, -31, -21, 0);
	VectorSet (self->maxs, 58, 38, 11);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t sdriver_frames_death [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t sdriver_move_death = {FRAME_Die_start, FRAME_Die_end, sdriver_frames_death, sdriver_dead};

void sdriver_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
		edict_t *drop;

	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/slave_driver/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_driver/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_driver/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_driver/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_driver/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/slave_driver/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

   self->monsterinfo.currentmove = &sdriver_move_death;
   gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);

	// shogun: drop a weapon
	// Topaz: Hey, Shogun. Should we drop the gun when he gibs too?
	drop = Drop_Item (self, FindItemByClassname ("Weapon_u23k"));
	drop->touch = Touch_Item;
	drop->nextthink = level.time + 30;
	drop->think = G_FreeEdict;

}


mframe_t sdriver_frames_decap [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t sdriver_move_decap = {FRAME_Decap_start, FRAME_Decap_end, sdriver_frames_decap, sdriver_dead};

void sdriver_decap (edict_t *self, int damage)
{
	edict_t *drop;
   self->monsterinfo.currentmove = &sdriver_move_decap;
   gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	ThrowGib (self, "models/monsters/slave_driver/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	drop = Drop_Item (self, FindItemByClassname ("Weapon_u23k"));
	drop->touch = Touch_Item;
	drop->nextthink = level.time + 30;
	drop->think = G_FreeEdict;
}

void sdriver_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	offset;
	vec3_t	start;

   if (!self->enemy || self->enemy->health <= 0)
      return;

   if (self->activator)
      return;

   if (!self->enemy)
      return;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_IDLE, 0);
	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, 8, 4);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	fire_gun (self, start, forward, 10, 30, 0);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
}

mframe_t sdriver_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sdriver_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sdriver_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sdriver_fire,
	BOT_charge, 0,  NULL
};
mmove_t sdriver_move_attack1 = {FRAME_Attack1_start, FRAME_Attack1_end, sdriver_frames_attack1, sdriver_run};

mframe_t sdriver_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, -3, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, sdriver_smack1,
	BOT_charge, 5, NULL,
	BOT_charge, 0,  NULL
};
mmove_t sdriver_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, sdriver_frames_attack2, sdriver_run};

mframe_t sdriver_frames_attack3 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, -3, NULL,
	BOT_charge, 0, sdriver_smack2,
	BOT_charge, 5, NULL,
	BOT_charge, 0,  NULL
};
mmove_t sdriver_move_attack3 = {FRAME_Attack3_start, FRAME_Attack3_end, sdriver_frames_attack3, sdriver_run};

void sdriver_attack(edict_t *self)
{
   if (range (self, self->enemy) != RANGE_MELEE)
      {
      self->monsterinfo.currentmove = &sdriver_move_attack1;
      self->monsterinfo.nextattack = level.time + 3;
      return;
      }
   self->monsterinfo.nextattack = level.time + 1;
	if (rand() & 1 || self->enemy->type & TYPE_SLAVE_ACTOR)
      {
		gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
      self->monsterinfo.nextattack = level.time + 1;
		self->monsterinfo.currentmove = &sdriver_move_attack2;
      }
	else
      {
		self->monsterinfo.currentmove = &sdriver_move_attack3;
      self->monsterinfo.nextattack = level.time + 1;
      }
}


mframe_t sdriver_frames_strafeleft [] =
{
	BOT_move_left, 10, NULL,
	BOT_move_left, 20, NULL,
	BOT_move_left, 20, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL
};
mmove_t sdriver_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, sdriver_frames_strafeleft, sdriver_run};

mframe_t sdriver_frames_straferight [] =
{
	BOT_move_right, 10, NULL,
	BOT_move_right, 20, NULL,
	BOT_move_right, 20, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL
};
mmove_t sdriver_move_straferight = {FRAME_Right_start, FRAME_Right_end, sdriver_frames_straferight, sdriver_run};

void sdriver_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &sdriver_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &sdriver_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &sdriver_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &sdriver_move_straferight;
}

void sdriver_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &sdriver_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &sdriver_move_pain2;
      }
}

void sdriver_smack1 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20 + (rand() & 15), 0);
}

void sdriver_smack2 (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 40 + (rand() & 15), 550))
	gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_sdriver (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("monsters/slave_driver/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/slave_driver/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/slave_driver/attack2.wav");
	sound_attack3  = gi.soundindex ("monsters/slave_driver/attack3.wav");
	sound_pain1    = gi.soundindex ("monsters/slave_driver/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave_driver/pain2.wav");
	sound_die      = gi.soundindex ("monsters/slave_driver/die.wav");
	sound_gib      = gi.soundindex ("monsters/slave_driver/gib.wav");
	sound_step     = gi.soundindex ("monsters/slave_driver/step.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave_driver/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 54);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 280;

	self->pain = sdriver_pain;
	self->die = sdriver_die;

	self->monsterinfo.stand = sdriver_stand;
	self->monsterinfo.walk = sdriver_walk;
	self->monsterinfo.run = sdriver_run;
	self->monsterinfo.dodge = sdriver_dodge;
	self->monsterinfo.attack = sdriver_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = sdriver_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->monsterinfo.action[0] = sdriver_ease;
   self->monsterinfo.action[1] = sdriver_grab;
   self->monsterinfo.action[2] = sdriver_give;
   self->monsterinfo.action[3] = sdriver_talk1;
   self->monsterinfo.action[4] = sdriver_talk2;
   self->monsterinfo.action[5] = sdriver_talk3;
	self->monsterinfo.decap = sdriver_decap;
   self->status = 0;
   self->type = 0;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
   walkmonster_start (self);
	
   if (self->spawnflags & SLAVE_EASE)		
      self->monsterinfo.currentmove = &sdriver_move_ease;
   else if (self->spawnflags & SLAVE_WALK)
      self->monsterinfo.currentmove = &sdriver_move_walk;
	else
		self->monsterinfo.currentmove = &sdriver_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD;
}

