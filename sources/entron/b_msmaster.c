/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_msmaster.c
  Description: AI/Physics for Slave Master

\**********************************************************/

#include "g_local.h"
#include "b_msmaster.h"

void find_node_hide (edict_t *self);
void smaster_pain (edict_t *self, edict_t *other, float kick, int damage);
void smaster_smack (edict_t *self);
void fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod);

#define MASTER_EASE 8
#define MASTER_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int  sound_gib;
static int  sound_step;


mframe_t smaster_frames_stand [] =
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
	BOT_stand, 0, NULL
};
mmove_t smaster_move_stand = {FRAME_Stand_start, FRAME_Stand_end, smaster_frames_stand, NULL};

void smaster_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_stand;
}

mframe_t smaster_frames_ease [] =
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
	BOT_stand, 0, NULL
};
mmove_t smaster_move_ease = {FRAME_Ease_start, FRAME_Ease_end, smaster_frames_ease, NULL};

void smaster_ease (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &smaster_move_ease;
}

void smaster_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t smaster_frames_walk [] =
{
	BOT_walk, 3,  smaster_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
   BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  smaster_step,
   BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL
};
mmove_t smaster_move_walk = {FRAME_Walk_start, FRAME_Walk_end, smaster_frames_walk, NULL};

void smaster_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &smaster_move_walk;
}

mframe_t smaster_frames_run [] =
{
	BOT_run, 0,  smaster_step,
	BOT_run, 24, NULL,
	BOT_run, 24, NULL,
	BOT_run, 0,  smaster_step,
	BOT_run, 24, NULL,
	BOT_run, 24, NULL
};
mmove_t smaster_move_run = {FRAME_Run_start, FRAME_Run_end, smaster_frames_run, NULL};

void smaster_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &smaster_move_stand;
	else
		self->monsterinfo.currentmove = &smaster_move_run;
}


mframe_t smaster_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t smaster_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, smaster_frames_pain1, smaster_run};

mframe_t smaster_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t smaster_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, smaster_frames_pain2, smaster_run};

mframe_t smaster_frames_pain3 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t smaster_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, smaster_frames_pain3, smaster_run};

void smaster_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void smaster_dead (edict_t *self)
{
	VectorSet (self->mins, -34, -34, 0);
	VectorSet (self->maxs, 56, 23, 23);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t smaster_frames_death [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t smaster_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, smaster_frames_death, smaster_dead};
mmove_t smaster_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, smaster_frames_death, smaster_dead};

void smaster_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/slave_master/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_master/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_master/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_master/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/slave_master/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/slave_master/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

   if (rand() & 1)
      {
      self->monsterinfo.currentmove = &smaster_move_death1;
      gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
      }
   else
      {
      self->monsterinfo.currentmove = &smaster_move_death2;
      gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
      }
}

void smaster_fire (edict_t *self)
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
	VectorSet(offset, 20, -10, 14);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_gun (self, start, forward, 14, 4, 0);

   //fire_blaster (self, start, forward, 40, 1600, EF_BLASTER, 1);	
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_GUNNER_MACHINEGUN_2);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

mframe_t smaster_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  smaster_fire,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t smaster_move_attack1 = {FRAME_Attack1_start, FRAME_Attack1_end, smaster_frames_attack1, smaster_run};

mframe_t smaster_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  smaster_fire,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t smaster_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, smaster_frames_attack2, smaster_run};

mframe_t smaster_frames_attack3 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, -3, NULL,
	BOT_charge, 0, smaster_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t smaster_move_attack3 = {FRAME_Attack3_start, FRAME_Attack3_end, smaster_frames_attack3, smaster_run};

void smaster_attack(edict_t *self)
{
   if (range (self, self->enemy) == RANGE_MELEE)
      {
		gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &smaster_move_attack3;
      self->monsterinfo.nextattack = level.time + 1;
      return;
      }
   self->monsterinfo.nextattack = level.time + 3;
	if (rand() & 1)
      {
		self->monsterinfo.pausetime = level.time + 2;
      self->monsterinfo.currentmove = &smaster_move_attack1;
      self->monsterinfo.nextattack = level.time + 4;
      }
	else
      {
		self->monsterinfo.pausetime = level.time + 2;
      self->monsterinfo.nextattack = level.time + 4;
		self->monsterinfo.currentmove = &smaster_move_attack2;
      }
}

void smaster_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (!self->enemy)
		self->enemy = attacker;
}

void smaster_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &smaster_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &smaster_move_pain2;
      }
}

void smaster_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20 + (rand() & 15), 0);
}

mframe_t smaster_frames_action [] =
{
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};

mmove_t smaster_move_ease2 = {FRAME_Ease_start, FRAME_Ease_end, smaster_frames_action, NULL};
mmove_t smaster_move_ponder = {FRAME_Ponder_start, FRAME_Ponder_end, smaster_frames_action, smaster_ease};
mmove_t smaster_move_talk = {FRAME_Talk_start, FRAME_Talk_end, smaster_frames_action, smaster_ease};
mmove_t smaster_move_talk2 = {FRAME_Talk2_start, FRAME_Talk2_end, smaster_frames_action, smaster_ease};
mmove_t smaster_move_talk3 = {FRAME_Talk3_start, FRAME_Talk3_end, smaster_frames_action, smaster_ease};
mmove_t smaster_move_laugh = {FRAME_Laugh_start, FRAME_Laugh_end, smaster_frames_action, smaster_ease};
mmove_t smaster_move_key = {FRAME_Key_start, FRAME_Key_end, smaster_frames_action, smaster_ease};

void smaster_ease2 (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_ease2;
}

void smaster_ponder (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_ponder;
}

void smaster_talk (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_talk;
}

void smaster_talk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_talk2;
}

void smaster_talk3 (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_talk3;
}

void smaster_laugh (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_laugh;
}

void smaster_key (edict_t *self)
{
	self->monsterinfo.currentmove = &smaster_move_key;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_smaster (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("monsters/slave_master/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/slave_master/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/slave_master/attack2.wav");
	sound_attack3  = gi.soundindex ("monsters/slave_master/attack3.wav");
	sound_pain1    = gi.soundindex ("monsters/slave_master/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/slave_master/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/slave_master/die1.wav");
	sound_die2     = gi.soundindex ("monsters/slave_master/die2.wav");
	sound_gib      = gi.soundindex ("monsters/slave_master/gib.wav");
	sound_step     = gi.soundindex ("monsters/slave_master/step.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/slave_master/tris.md2");
	VectorSet (self->mins, -28, -24, 0);
	VectorSet (self->maxs, 32, 16, 72);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 340;

	self->pain = smaster_pain;
	self->die = smaster_die;

   self->monsterinfo.stand = smaster_stand;
	self->monsterinfo.walk = smaster_walk;
	self->monsterinfo.run = smaster_run;
	self->monsterinfo.dodge = smaster_dodge;
	self->monsterinfo.attack = smaster_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = smaster_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;

	self->monsterinfo.action[0] = smaster_ease2;
	self->monsterinfo.action[1] = smaster_ponder;
	self->monsterinfo.action[2] = smaster_talk;
	self->monsterinfo.action[3] = smaster_talk2;
	self->monsterinfo.action[4] = smaster_talk3;
	self->monsterinfo.action[5] = smaster_laugh;
	self->monsterinfo.action[6] = smaster_key;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & MASTER_EASE)		
      self->monsterinfo.currentmove = &smaster_move_ease;
   else if (self->spawnflags & MASTER_WALK)
      self->monsterinfo.currentmove = &smaster_move_walk;
	else
		self->monsterinfo.currentmove = &smaster_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD | TYPE_MONSTER_FLAT;
}