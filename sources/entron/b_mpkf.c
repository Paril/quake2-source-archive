/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mpkf.c
  Description: Piece Keeping Forces AI

\**********************************************************/


#include "g_local.h"
#include "b_mpkf.h"

void find_node_hide (edict_t *self);
void pkf_pain (edict_t *self, edict_t *other, float kick, int damage);
void pkf_smack (edict_t *self);
void pkf_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void GoAmbush (edict_t *ent, edict_t *enemy, float dist);
void pkf_runfire (edict_t *self);
void pkf_stand (edict_t *self);
void pkf_stand2 (edict_t *self);
void pkf_run (edict_t *self);
void pkf_fire (edict_t *self);
void pkf_grenade (edict_t *self);
void pkf_crouch (edict_t *self);
void pkf_smack (edict_t *self);
void pkf_dead (edict_t *self);
void pkf_reloaded (edict_t *self);
void pkf_reloaded2 (edict_t *self);
void pkf_step (edict_t *self);
void pound_noise(edict_t *self);
void pkf_impact(edict_t *self);

#define WARRIOR_CROUCH 8
#define WARRIOR_WALK 16

int saver;
static int	sound_sight;
static int	sound_attack;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_die1;
static int	sound_die2;
static int	sound_die3;
static int	sound_step1;
static int	sound_step2;
static int	sound_step3;
static int	sound_step4;
static int	sound_step5;
static int  sound_throw1;
static int  sound_throw2;
static int  sound_reload;
static int  sound_kick;
static int  sound_punch;
static int  sound_impact1;
static int  sound_impact2;
static int	sound_gib;

mframe_t pkf_frames_stand [] =
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
	BOT_stand, 0, NULL
};
mmove_t pkf_move_stand = {FRAME_Stand_start, FRAME_Stand_end, pkf_frames_stand, NULL};
mmove_t pkf_move_crouch = {FRAME_Crouch_start, FRAME_Crouch_end, pkf_frames_stand, NULL};
//mmove_t pkf_move_crouchduck = {FRAME_Crouch_start, FRAME_Crouch_end, pkf_frames_stand, pkf_stand};

mframe_t pkf_frames_reload [] =
{
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, NULL,
	BOT_stand, 0, pkf_reloaded,
	BOT_stand, 0, NULL
};
mmove_t pkf_move_reload1 = {FRAME_Reload_start, FRAME_Reload_end, pkf_frames_reload, pkf_stand};
mmove_t pkf_move_reload2 = {FRAME_CReload_start, FRAME_CReload_end, pkf_frames_reload, pkf_crouch};

mframe_t pkf_frames_walk [] =
{
	BOT_walk, 3, NULL,
	BOT_walk, 3, pkf_step,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, pkf_step,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL,
	BOT_walk, 3, NULL
};
mmove_t pkf_move_walk = {FRAME_Walk_start, FRAME_Walk_end, pkf_frames_walk, NULL};

mframe_t pkf_frames_run [] =
{
	BOT_run, 24, NULL,
	BOT_run, 24, pkf_step,
	BOT_run, 24, NULL,
	BOT_run, 24, NULL,
	BOT_run, 24, pkf_step,
	BOT_run, 24, NULL
};
mmove_t pkf_move_run = {FRAME_Run_start, FRAME_Run_end, pkf_frames_run, NULL};

mframe_t pkf_frames_fire [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, pkf_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t pkf_move_fire = {FRAME_Fire_start, FRAME_Fire_end, pkf_frames_fire, pkf_run};

mframe_t pkf_frames_throw [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, pkf_grenade,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t pkf_move_throw = {FRAME_Throw_start, FRAME_Throw_end, pkf_frames_throw, pkf_run};

mframe_t pkf_frames_smack [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  pkf_smack,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t pkf_move_smack = {FRAME_Smack_start, FRAME_Smack_end, pkf_frames_smack, pkf_run};

mframe_t pkf_frames_kick [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, pkf_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t pkf_move_kick = {FRAME_Kick_start, FRAME_Kick_end, pkf_frames_kick, pkf_run};


mframe_t pkf_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t pkf_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, pkf_frames_pain1, pkf_run};
mmove_t pkf_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, pkf_frames_pain1, pkf_run};
mmove_t pkf_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, pkf_frames_pain1, pkf_run};

mframe_t pkf_frames_grenade [] =
{
	BOT_move, -3, NULL,
	BOT_move, -6, pkf_impact,
	BOT_move, -7, NULL,
	BOT_move, -6, NULL,
	BOT_move, -5, NULL,
	BOT_move, -1, NULL,
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
mmove_t pkf_move_grenade = {FRAME_Grenade_start, FRAME_Grenade_end, pkf_frames_grenade, pkf_run};

mframe_t pkf_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  pkf_impact,
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
mmove_t pkf_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, pkf_frames_death1, pkf_dead};

mframe_t pkf_frames_death2 [] =
{
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, pkf_impact,
	BOT_move, 0, NULL
};
mmove_t pkf_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, pkf_frames_death2, pkf_dead};

mframe_t pkf_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 7, NULL,
	BOT_move_left, 0, NULL
};
mmove_t pkf_move_strafeleft = {FRAME_RLeft_start, FRAME_RLeft_end, pkf_frames_strafeleft, pkf_run};

mframe_t pkf_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 7, NULL,
	BOT_move_right, 0, NULL
};
mmove_t pkf_move_straferight = {FRAME_RRight_start, FRAME_RRight_end, pkf_frames_straferight, pkf_run};

mframe_t pkf_frames_straferight2 [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 15, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 5, NULL,
	BOT_move_right, 3, NULL
};
mmove_t pkf_move_straferight2 = {FRAME_Right_start, FRAME_Right_end, pkf_frames_straferight2, pkf_run};

mframe_t pkf_frames_strafeleft2 [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 15, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 5, NULL,
	BOT_move_left, 3, NULL
};
mmove_t pkf_move_strafeleft2 = {FRAME_Left_start, FRAME_Left_end, pkf_frames_strafeleft2, pkf_run};

mframe_t pkf_frames_decap [] =
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
	BOT_move, 0, pkf_impact,
	BOT_move, 0, NULL
};
mmove_t pkf_move_decap = {FRAME_Decap_start, FRAME_Decap_end, pkf_frames_decap, pkf_dead};


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void pkf_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &pkf_move_stand;
   self->maxs[2] = 60;
}


void pkf_crouch (edict_t *self)
{
   //self->enemy = NULL;
	self->monsterinfo.currentmove = &pkf_move_crouch;
}

void pkf_reloaded (edict_t *self)
{
   self->monsterinfo.shots = 16;
}

void pkf_reload (edict_t *self)
{
   if (self->monsterinfo.shots <= 0)
      {
	   gi.sound (self, CHAN_WEAPON, sound_reload, 1, ATTN_NORM, 0);
      if (random() < .5)
         self->monsterinfo.currentmove = &pkf_move_reload1;
      else
         {
         self->monsterinfo.currentmove = &pkf_move_reload2;
         self->maxs[2] = 32;
         }
      }
   else
      self->monsterinfo.currentmove = &pkf_move_crouch;
   self->status |= STATUS_MONSTER_NOSHOOT;
   self->monsterinfo.nextcover = 0;
   self->monsterinfo.covers = 0;
   self->monsterinfo.nextambush = level.time + 1 + (random() * 3);
}

void pkf_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &pkf_move_walk;
}

void pkf_impact(edict_t *self)
{
	if (rand() & 1)
		gi.sound (self, CHAN_BODY, sound_impact1, 1, ATTN_NORM, 0);
   else
		gi.sound (self, CHAN_BODY, sound_impact2, 1, ATTN_NORM, 0);
}

void pkf_run (edict_t *self)
{
   self->maxs[2] = 60;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &pkf_move_stand;
	else
		self->monsterinfo.currentmove = &pkf_move_run;
}

void pkf_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void pkf_step (edict_t *self)
{
   switch (rand() % 5)
      {
      case 0:
         gi.sound (self, CHAN_BODY, sound_step1, 1, ATTN_STATIC, 0);
         break;
      case 1:
         gi.sound (self, CHAN_BODY, sound_step2, 1, ATTN_STATIC, 0);
         break;
      case 2:
         gi.sound (self, CHAN_BODY, sound_step3, 1, ATTN_STATIC, 0);
         break;
      case 3:
         gi.sound (self, CHAN_BODY, sound_step4, 1, ATTN_STATIC, 0);
         break;
      case 4:
         gi.sound (self, CHAN_BODY, sound_step5, 1, ATTN_STATIC, 0);
         break;	
      }
}

void pkf_dead (edict_t *self)
{
	VectorSet (self->mins, -28, -16, -1);
	VectorSet (self->maxs, 42, 15, 13);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

void pkf_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	edict_t *drop;

	self->s.modelindex2 = self->s.modelindex3 = self->s.modelindex4 = 0;
	if (self->health <= self->gib_health)
		{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/pkf/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/pkf/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/pkf/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/pkf/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/pkf/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/pkf/gibs/head.md2", damage, GIB_METALLIC);
		return;
		}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	
	if (rand() & 1)
		{
		drop = Drop_Item (self, FindItem("Plasma Gun"));
		drop->touch = Touch_Item;
		drop->nextthink = level.time + 30;
		drop->think = G_FreeEdict;
		}
	else
		{
		drop = Drop_Item (self, FindItem("PKF Ammo Pack"));
		drop->touch = Touch_Item;
		drop->nextthink = level.time + 30;
		drop->think = G_FreeEdict;
		}

	if (rand() & 1)
      {
      self->monsterinfo.currentmove = &pkf_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
      }
	else
      {
      self->monsterinfo.currentmove = &pkf_move_death2;
      gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
      }
}

void pkf_runfire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;    

   if (--self->monsterinfo.shots <= 0)
		{	
		self->monsterinfo.currentmove = &pkf_move_run;
      return;
		}

   if (!self->enemy || self->enemy->health <= 0)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 47, 3, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   pkf_plasma (self, start, forward, 1, 800);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void pkf_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;    

   if (--self->monsterinfo.shots <= 0) 
      {
      self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
      return;
      }
	
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

   if (!self->enemy || self->enemy->health <= 0)
      return;

   if (!visible (self, self->enemy))
      {
      self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
      return;
      }

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 47, 3, 2);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   pkf_plasma (self, start, forward, 15, 800);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void pkf_grenade (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
   float    range;
   //trace_t  trace;

   if (!self->enemy || self->enemy->health <= 0)
      return;

   VectorCopy(self->enemy->s.origin, start);
   start[2] += 180;
	//trace = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);	
   //if (trace.fraction < 1.0)
      //return;
   VectorSubtract (start, self->s.origin, forward);
   range = VectorLength(forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, -8, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_grenade (self, start, forward, 100, range, 3, 100);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
   if (rand() & 1)
      gi.sound (self, CHAN_WEAPON, sound_throw1, 1, ATTN_NORM, 0);
   else
      gi.sound (self, CHAN_WEAPON, sound_throw2, 1, ATTN_NORM, 0);
}

void pkf_attack(edict_t *self)
{
   vec3_t forward;
   float range;
   self->monsterinfo.nextattack = level.time + 2.4;
   self->monsterinfo.pausetime = level.time + 1; //2
   VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
   range = VectorLength(forward);
	if (range < MELEE_DISTANCE)
      {
		switch (rand() & 1)
			{
			case 0:
				self->monsterinfo.currentmove = &pkf_move_smack;
				break;
			default:
				self->monsterinfo.currentmove = &pkf_move_kick;
			}
		return;
      }

   if (range >= 300)
      {
      if (random() < .25)
         self->monsterinfo.currentmove = &pkf_move_throw;
      else
         self->monsterinfo.currentmove = &pkf_move_fire;
      return;
      }
   self->monsterinfo.currentmove = &pkf_move_fire;
}


void pkf_dodge (edict_t *self, edict_t *attacker, float eta)
{
	vec3_t	offset, forward, right, end;
   trace_t  trace;
   float k = random();

   if (self->monsterinfo.currentmove == &pkf_move_straferight)
      return;
   if (self->monsterinfo.currentmove == &pkf_move_strafeleft)
      return;

	//if (!self->enemy)
		//self->enemy = attacker;

   VectorSet(offset, 0, 90, 0);
   AngleVectors (self->s.angles, forward, right, NULL);
   if (k < .3)
      {
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
         {
         if (random() < .5)
            self->monsterinfo.currentmove = &pkf_move_straferight;
         else
            self->monsterinfo.currentmove = &pkf_move_straferight2;
         return;
         }
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
         if (random() < .5)
            self->monsterinfo.currentmove = &pkf_move_strafeleft;
         else
            self->monsterinfo.currentmove = &pkf_move_strafeleft2;
      return;         
      }
   if (k < .6 || self->monsterinfo.shots < 4)
      {
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
         {
         if (random() < .5)
            self->monsterinfo.currentmove = &pkf_move_strafeleft;
         else
            self->monsterinfo.currentmove = &pkf_move_strafeleft2;
         return;
         }
      offset[1] = 90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
         if (random() < .5)
            self->monsterinfo.currentmove = &pkf_move_straferight;
         else
            self->monsterinfo.currentmove = &pkf_move_straferight2;
      return;
      }
}

void pkf_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;
   static int i = 0;
   self->status |= STATUS_MONSTER_COMMUNICATE;
 
   if (level.time < self->pain_debounce_time)
		return;

   self->pain_debounce_time = level.time + 3;

	if (!self->s.skinnum && self->health < self->max_health / 2)
		{
      self->s.skinnum = 1;
		gi.linkentity(self);
		}
	else if (self->s.skinnum == 1 && self->health < self->max_health / 3)
		{
      self->s.skinnum = 2;
		gi.linkentity(self);
		}

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = random();

	if (self->enemy && self->enemy->health > 0)
		if (range (self, self->enemy) <= RANGE_NEAR)
			{
			self->monsterinfo.goalambush = NULL;
			self->status &= ~STATUS_MONSTER_AMBUSH;
			GoAmbush(self, self->enemy, 24);
	      }


	if (kick > 90 && other)
		{
		float range;
		vec3_t forward;
		VectorSubtract (other->s.origin, self->s.origin, forward);
		range = VectorLength(forward);
		self->s.angles[YAW] = vectoyaw(forward);
		self->monsterinfo.currentmove = &pkf_move_grenade;
		return;
		}
	if (n < .33)
		{
      gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &pkf_move_pain1;
		}
	else if (n < .66)
		{
      gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &pkf_move_pain2;
		}
	else
		{
      gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &pkf_move_pain3;
		}
}

void pkf_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 20 + (rand() & 4), 740))
		{
		if (self->monsterinfo.currentmove == &pkf_move_smack)
			gi.sound (self, CHAN_WEAPON, sound_punch, 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_WEAPON, sound_kick, 1, ATTN_NORM, 0);
		}
}

void pkf_plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	   {
		G_FreeEdict (self);
		return;
	   }

	if (other->takedamage)
      T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_ENERGY, MOD_PLASMA);
	else
	   {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SCREEN_SPARKS);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	   }
	G_FreeEdict (self);
}

void pkf_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	//bolt->s.effects |= EF_COLOR_SHELL;
	//bolt->s.renderfx |= RF_SHELL_GREEN;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/nail/tris.md2");
	//bolt->s.sound = gi.soundindex ("weapons/plasmagun/fire.wav");
	bolt->owner = self;
	bolt->touch = pkf_plasma_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	gi.linkentity (bolt);

   check_dodge (self, start, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
		{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
		}	
}	

void pkf_decap (edict_t *self, int damage)
{
	edict_t *drop;
	self->s.modelindex2 = self->s.modelindex3 = self->s.modelindex4;
   self->monsterinfo.currentmove = &pkf_move_decap;
	if (rand() & 1)
		{
		drop = Drop_Item (self, FindItem("Plasma Gun"));
		drop->touch = Touch_Item;
		drop->nextthink = level.time + 30;
		drop->think = G_FreeEdict;
		}
	else
		{
		drop = Drop_Item (self, FindItem("PKF Ammo Pack"));
		drop->touch = Touch_Item;
		drop->nextthink = level.time + 30;
		drop->think = G_FreeEdict;
		}
   gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
	ThrowGib (self, "models/monsters/pkf/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_pkf (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("monsters/pkf/sight.wav");
	sound_attack   = gi.soundindex ("monsters/pkf/attack.wav");
	sound_pain1    = gi.soundindex ("monsters/pkf/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/pkf/pain2.wav");
	sound_pain3    = gi.soundindex ("monsters/pkf/pain3.wav");
	sound_die1     = gi.soundindex ("monsters/pkf/die1.wav");
	sound_die2     = gi.soundindex ("monsters/pkf/die2.wav");
	sound_die3     = gi.soundindex ("monsters/pkf/die3.wav");
   sound_step1    = gi.soundindex ("monsters/pkf/step1.wav");
   sound_step2    = gi.soundindex ("monsters/pkf/step2.wav");
   sound_step3    = gi.soundindex ("monsters/pkf/step3.wav");
   sound_step4    = gi.soundindex ("monsters/pkf/step4.wav");
   sound_step5    = gi.soundindex ("monsters/pkf/step5.wav");
   sound_throw1   = gi.soundindex ("monsters/pkf/throw1.wav");
   sound_throw2   = gi.soundindex ("monsters/pkf/throw2.wav");
	sound_kick     = gi.soundindex ("monsters/pkf/kick.wav");
	sound_punch    = gi.soundindex ("monsters/pkf/punch.wav");
	sound_impact1  = gi.soundindex ("monsters/pkf/impact1.wav");
	sound_impact2  = gi.soundindex ("monsters/pkf/impact2.wav");
	sound_gib		= gi.soundindex ("monsters/pkf/gib.wav");
   sound_reload   = gi.soundindex ("weapons/reload.wav");
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/pkf/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/pkf/plasmagun.md2");
	self->s.modelindex3 = gi.modelindex("models/monsters/pkf/backpack.md2");
	if (self->style)
		self->s.modelindex4 = gi.modelindex("models/monsters/pkf/armor.md2");
	VectorSet (self->mins, -9, -15, -1);
	VectorSet (self->maxs, 12, 15, 60);

   if (!self->health)
	   self->health = self->max_health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 240;

	self->pain = pkf_pain;
	self->die = pkf_die;

   self->monsterinfo.stand = pkf_stand;
	self->monsterinfo.walk = pkf_walk;
	self->monsterinfo.run = pkf_run;
	self->monsterinfo.dodge = pkf_dodge;
	self->monsterinfo.attack = pkf_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.reload = pkf_reload;
	self->monsterinfo.sight = pkf_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
	self->monsterinfo.decap = pkf_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & WARRIOR_CROUCH)
		self->monsterinfo.currentmove = &pkf_move_crouch;
   else if (self->spawnflags & WARRIOR_WALK)
		self->monsterinfo.currentmove = &pkf_move_walk;
   else
		self->monsterinfo.currentmove = &pkf_move_stand;

	if (!self->team_data)
		self->team_data = 1;
	self->monsterinfo.shots = 16;
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_AMBUSH | TYPE_MONSTER_DROPBLOOD;
   saver = 0;
}