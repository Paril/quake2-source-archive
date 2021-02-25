/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mboss.c
  Description: AI/Physics for Guardian

\**********************************************************/

#include "g_local.h"
#include "b_mboss1.h"

void find_node_hide (edict_t *self);
void boss1_pain (edict_t *self, edict_t *other, float kick, int damage);
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void boss1_run (edict_t *self);

#define BOSS1_WALK 16

static int	sound_sight;
static int	sound_engine;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_die;
static int	sound_idle;
static int	sound_gib;
static int	sound_step;

void boss1_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

mframe_t boss1_frames_stand [] =
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
	BOT_stand, 0, NULL
};
mmove_t boss1_move_stand = {FRAME_Stand_start, FRAME_Stand_end, boss1_frames_stand, NULL};

void boss1_stand (edict_t *self)
{
   self->monsterinfo.currentmove = &boss1_move_stand;
}

void boss1_go_green(edict_t *self)
{
	self->s.effects |= EF_FLAG2; /*EF_COLOR_SHELL | */
	self->s.renderfx = RF_TRANSLUCENT; /*RF_SHELL_GREEN*/
}

void boss1_go_normal(edict_t *self)
{
	self->s.effects &= ~EF_FLAG2; /*EF_COLOR_SHELL | */
	self->s.renderfx = 0; /*RF_SHELL_GREEN*/
}

mframe_t boss1_frames_block [] =
{
	BOT_move, 0, boss1_go_green,
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
	BOT_move, 0, boss1_go_normal,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};

mmove_t boss1_move_block = {FRAME_Block_start, FRAME_Block_end, boss1_frames_block, boss1_run};


void boss1_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t boss1_frames_walk [] =
{
	BOT_walk, 3,  boss1_step,
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
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  boss1_step,
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
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL
};
mmove_t boss1_move_walk = {FRAME_Walk_start, FRAME_Walk_end, boss1_frames_walk, NULL};

void boss1_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &boss1_move_walk;
}

mframe_t boss1_frames_run [] =
{
	BOT_run, 5,   boss1_step,
	BOT_run, 10,  NULL,
	BOT_run, 15,  NULL,
	BOT_run, 20,  NULL,
	BOT_run, 5,   boss1_step,
	BOT_run, 10,  NULL,
	BOT_run, 15,  NULL,
	BOT_run, 20,  NULL
};
mmove_t boss1_move_run = {FRAME_Run_start, FRAME_Run_end, boss1_frames_run, NULL};

void boss1_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &boss1_move_stand;
	else
		self->monsterinfo.currentmove = &boss1_move_run;
}


mframe_t boss1_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t boss1_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, boss1_frames_pain1, boss1_run};
mmove_t boss1_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, boss1_frames_pain1, boss1_run};
mmove_t boss1_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, boss1_frames_pain1, boss1_run};

void boss1_dead (edict_t *self)
{
	self->s.sound = 0;
	VectorSet (self->mins, -58, -76, -5);
	VectorSet (self->maxs, 68, 73, 50);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t boss1_frames_death [] =
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
mmove_t boss1_move_death = {FRAME_Die_start, FRAME_Die_end, boss1_frames_death, boss1_dead};

void boss1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	boss1_go_normal(self);
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/guardian/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guardian/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guardian/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guardian/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/guardian/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/guardian/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &boss1_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}


void boss1_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

   if (!self->enemy || self->enemy->health <= 0)
      return;


	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset, 40, 20, 8);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorSubtract (self->enemy->s.origin, start, forward);
	VectorNormalize (forward);   
   vectoangles(forward, self->s.angles);
   fire_blaster (self, start, forward, 15, 1600, 0x00000060 /*EF_HYPERBLASTER*/, 1);	
   self->s.angles[0] = 0;
   self->s.angles[2] = 0;
	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);

	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

void boss1_rocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   if (!self->enemy || self->enemy->health <= 0)
      return;


	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset, 40, -24, 20);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	VectorSubtract (self->enemy->s.origin, start, forward);
	VectorNormalize (forward);   
   vectoangles(forward, self->s.angles);
	monster_fire_rocket (self, start, forward, 80, 500, MZ2_BOSS2_ROCKET_2);
	gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
   self->s.angles[0] = 0;
   self->s.angles[2] = 0;
}


mframe_t boss1_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, -1,  NULL,
	BOT_charge, 0, boss1_fire,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t boss1_move_attack1 = {FRAME_Attack_start, FRAME_Attack_end, boss1_frames_attack1, boss1_run};

mframe_t boss1_frames_attack2 [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, boss1_rocket,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t boss1_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, boss1_frames_attack2, boss1_run};

void boss1_attack(edict_t *self)
{
	if (range (self, self->enemy) > RANGE_NEAR)
      {
	   self->monsterinfo.currentmove = &boss1_move_attack2;
      self->monsterinfo.nextattack = level.time + 3;
      }
	else
      {
		self->monsterinfo.pausetime = level.time + 2;
      self->monsterinfo.nextattack = level.time + 5;
		self->monsterinfo.currentmove = &boss1_move_attack1;
      }
}

mframe_t boss1_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL
};
mmove_t boss1_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, boss1_frames_strafeleft, boss1_run};

mframe_t boss1_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL
};
mmove_t boss1_move_straferight = {FRAME_Right_start, FRAME_Right_end, boss1_frames_straferight, boss1_run};

void boss1_dodge (edict_t *self, edict_t *attacker, float eta)
{
	vec3_t	offset, forward, right, end;
   trace_t  trace;

	if (!self->enemy)
		self->enemy = attacker;

	if (random() < .7)
		{
		self->monsterinfo.currentmove = &boss1_move_block;
		return;
		}

   VectorSet(offset, 0, 90, 0);
   AngleVectors (self->s.angles, forward, right, NULL);

   if (random() < .5)
      {
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &boss1_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &boss1_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &boss1_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &boss1_move_straferight;
}

void boss1_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

	if (level.time < self->pain_debounce_time)
		return;

   if (!self->s.skinnum && self->health < self->max_health / 3)
 		{
      self->s.skinnum = 1;
		gi.linkentity(self);
		}


	self->pain_debounce_time = level.time + 3;
	
	if (self->s.renderfx == RF_TRANSLUCENT)
		{
		self->health += damage / 4;
		return;
		}

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = random();

	if (n < .33)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss1_move_pain1;
      }
	else if (n < .66)
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss1_move_pain2;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss1_move_pain3;
      }

}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_boss1 (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }
   /*
	sound_pain1 = gi.soundindex ("monsters/boss1/pain1.wav");
	sound_pain2 = gi.soundindex ("monsters/boss1/pain2.wav");
	sound_die1 = gi.soundindex ("hover/hovdeth1.wav");
	sound_die2 = gi.soundindex ("hover/hovdeth2.wav");

	sound_rocketshot = gi.soundindex ("monsters/boss1/shoot.wav");

	sound_melee_hit = gi.soundindex ("gladiator/melee1.wav");
	sound_sight = gi.soundindex ("monsters/boss1/sight.wav");
	sound_search = gi.soundindex ("priest/search.wav");
	sound_idle = gi.soundindex ("priest/idle.wav");
   */

	
	sound_attack1  = gi.soundindex ("monsters/guardian/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/guardian/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/guardian/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/guardian/pain2.wav");
	sound_pain3	   = gi.soundindex ("monsters/guardian/pain3.wav");
	sound_die      = gi.soundindex ("monsters/guardian/die.wav");
	sound_sight    = gi.soundindex ("monsters/guardian/sight.wav");
	sound_gib		= gi.soundindex ("monsters/guardian/gib.wav");
	sound_step		= gi.soundindex ("monsters/guardian/step.wav");

   self->s.sound  = gi.soundindex ("monsters/guardian/engine.wav");;

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/guardian/tris.md2");
	VectorSet (self->mins, -44, -46, 0);
	VectorSet (self->maxs, 40, 59, 110);

	if (!self->health)
      self->health = 1800;
	if (!self->gib_health)
		self->gib_health = -420;
	self->mass = 1800;

	self->pain = boss1_pain;
	self->die = boss1_die;

   self->monsterinfo.stand = boss1_stand;
	self->monsterinfo.walk = boss1_walk;
	self->monsterinfo.run = boss1_run;
	self->monsterinfo.dodge = boss1_dodge;
	self->monsterinfo.attack = boss1_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = boss1_sight;
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

   if (self->spawnflags & BOSS1_WALK)
		self->monsterinfo.currentmove = &boss1_move_walk;
   else
		self->monsterinfo.currentmove = &boss1_move_stand;
	
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_METALIC | TYPE_MONSTER_FLAT;
}