/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mpriest.c
  Description: AI/Physics for Priest

\**********************************************************/

#include "g_local.h"
#include "b_mpriest.h"

void find_node_hide (edict_t *self);
void priest_pain (edict_t *self, edict_t *other, float kick, int damage);
void fire_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void spike_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void priest_smack (edict_t *self);
void priest_stand2 (edict_t *self);
void priest_crouch2 (edict_t *self);
void priest_worship2 (edict_t *self);
void priest_meditate2 (edict_t *self);
void priest_talk2 (edict_t *self);
void priest_hand2 (edict_t *self);

#define PRIEST_CROUCH 8
#define PRIEST_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_gib;
static int	sound_step;


mframe_t priest_frames_meditate [] =
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
	BOT_move, 0, NULL
};
mmove_t priest_move_meditate1 = {FRAME_Standmedi_start, FRAME_Standmedi_end, priest_frames_meditate, priest_meditate2};
mmove_t priest_move_meditate2 = {FRAME_Meditate_start, FRAME_Meditate_end, priest_frames_meditate, NULL};
mmove_t priest_move_meditate3 = {FRAME_Medistand_start, FRAME_Medistand_end, priest_frames_meditate, priest_stand2};

mframe_t priest_frames_stand [] =
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
mmove_t priest_move_stand = {FRAME_standStart, FRAME_standEnd, priest_frames_stand, NULL};

mframe_t priest_frames_crouch [] =
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
mmove_t priest_move_crouch = {FRAME_crouchStart, FRAME_crouchEnd, priest_frames_crouch, NULL};
mmove_t priest_move_duck = {FRAME_Crouch1_start, FRAME_Crouch1_end, priest_frames_crouch, priest_crouch2};
mmove_t priest_move_getup = {FRAME_Crouch3_start, FRAME_Crouch3_end, priest_frames_crouch, priest_stand2};
mmove_t priest_move_talk1 = {FRAME_Talk1_start, FRAME_Talk1_end, priest_frames_meditate, priest_talk2};
mmove_t priest_move_talk2 = {FRAME_Talk2_start, FRAME_Talk2_end, priest_frames_meditate, NULL};
mmove_t priest_move_talk3 = {FRAME_Talk3_start, FRAME_Talk3_end, priest_frames_meditate, priest_stand2};
mmove_t priest_move_worship1 = {FRAME_Worship1_start, FRAME_Worship1_end, priest_frames_meditate, priest_worship2};
mmove_t priest_move_worship2 = {FRAME_Worship2_start, FRAME_Worship2_end, priest_frames_meditate, NULL};
mmove_t priest_move_worship3 = {FRAME_Worship3_start, FRAME_Worship3_end, priest_frames_meditate, priest_crouch2};
mmove_t priest_move_pray = {FRAME_Pray_start, FRAME_Pray_end, priest_frames_meditate, priest_stand2};


void priest_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &priest_move_crouch)
		self->monsterinfo.currentmove = &priest_move_getup;
	else if (self->monsterinfo.currentmove == &priest_move_meditate2)
		self->monsterinfo.currentmove = &priest_move_meditate3;
	else if (self->monsterinfo.currentmove == &priest_move_talk2)
		self->monsterinfo.currentmove = &priest_move_talk3;
	else
		self->monsterinfo.currentmove = &priest_move_stand;
}

void priest_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &priest_move_stand;
}

void priest_crouch (edict_t *self)
{
	if (self->monsterinfo.currentmove == &priest_move_worship2)
		self->monsterinfo.currentmove = &priest_move_worship3;
	else
		self->monsterinfo.currentmove = &priest_move_duck;
}

void priest_crouch2 (edict_t *self)
{
	self->monsterinfo.currentmove = &priest_move_crouch;
}

void priest_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}
mframe_t priest_frames_walk [] =
{
	BOT_walk, 0,  priest_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  priest_step,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 0,  NULL
};
mmove_t priest_move_walk = {FRAME_walkStart, FRAME_walkEnd, priest_frames_walk, NULL};

void priest_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &priest_move_walk;
}

mframe_t priest_frames_run [] =
{
	BOT_run, 8,  priest_step,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL,
	BOT_run, 8,  priest_step,
	BOT_run, 16, NULL,
	BOT_run, 16, NULL
};
mmove_t priest_move_run = {FRAME_Run_start, FRAME_Run_end, priest_frames_run, NULL};

void priest_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &priest_move_stand;
	else
		self->monsterinfo.currentmove = &priest_move_run;
}


mframe_t priest_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t priest_move_pain1 = {FRAME_pain1Start, FRAME_pain1End, priest_frames_pain1, priest_run};

mframe_t priest_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t priest_move_pain2 = {FRAME_pain2Start, FRAME_pain2End, priest_frames_pain2, priest_run};

void priest_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void priest_dead (edict_t *self)
{
	VectorSet (self->mins, -32, -11, -5);
	VectorSet (self->maxs, 27, 24, 12);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t priest_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t priest_move_death1 = {FRAME_death1Start, FRAME_death1End, priest_frames_death1, priest_dead};
mmove_t priest_move_death2 = {FRAME_Die_start, FRAME_Die_end, priest_frames_death1, priest_dead};

void priest_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/priest/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/priest/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/priest/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/priest/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/priest/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/priest/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand() & 1;
	if (n)
		{
		self->monsterinfo.currentmove = &priest_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		}
	else
		{
		self->monsterinfo.currentmove = &priest_move_death2;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
		}
}

void priest_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, 0, 0);
   vectoangles(forward, self->s.angles);
   self->s.angles[2] = 0;
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	//G_ProjectSource (self->s.origin, offset, forward, NULL, start);

   fire_fireball (self, start, forward, 45, 750, 70, 15);

   self->s.angles[1] += 15;
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_fireball (self, start, forward, 15, 1000, 70, 15);

   self->s.angles[1] -= 30;
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_fireball (self, start, forward, 15, 1000, 70, 15);

   self->s.angles[1] += 15;
   self->s.angles[0] = 0;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);

	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_BOSS2_ROCKET_2);
	//gi.multicast (self->s.origin, MULTICAST_PVS);

/*
   if (self->health > 0)
      self->monsterinfo.run(self);
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
*/
}

mframe_t priest_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, priest_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 1, NULL,
	BOT_charge, 2, NULL,
	BOT_charge, 6, NULL,
	BOT_charge, 4, NULL,
	BOT_charge, 2, NULL,
	BOT_charge, 0,  NULL
};
mmove_t priest_move_attack1 = {FRAME_attak1Start, FRAME_attak1End, priest_frames_attack1, priest_run};

mframe_t priest_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, priest_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t priest_move_attack2 = {FRAME_attak2Start, FRAME_attak2End, priest_frames_attack2, priest_run};

void priest_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
      {
		gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &priest_move_attack2;
      self->monsterinfo.nextattack = level.time + 1;
      }
	else
      {
      self->monsterinfo.nextattack = level.time + 3;
		self->monsterinfo.currentmove = &priest_move_attack1;
      }
}

void priest_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void priest_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void priest_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}
/*
mframe_t priest_frames_duck [] =
{
	BOT_move, -2, priest_duck_down,
	BOT_move, -5, priest_duck_hold,
	BOT_move, 0,  NULL,
	BOT_move, 4,  priest_duck_up
};
mmove_t priest_move_duck = {FRAME_duckStart, FRAME_duckEnd, priest_frames_duck, priest_run};
*/
mframe_t priest_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 16, NULL,
	BOT_move_left, 16, NULL,
	BOT_move_left, 16, NULL,
	BOT_move_left, 16, NULL,
	BOT_move_left, 5, NULL,
	BOT_move_left, 2, NULL,
	BOT_move_left, 0, NULL
};
mmove_t priest_move_strafeleft = {FRAME_leftStart, FRAME_leftEnd, priest_frames_strafeleft, priest_run};

mframe_t priest_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 16, NULL,
	BOT_move_right, 16, NULL,
	BOT_move_right, 16, NULL,
	BOT_move_right, 16, NULL,
	BOT_move_right, 5, NULL,
	BOT_move_right, 2, NULL,
	BOT_move_right, 0, NULL
};
mmove_t priest_move_straferight = {FRAME_rightStart, FRAME_rightEnd, priest_frames_straferight, priest_run};

void priest_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &priest_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &priest_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &priest_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &priest_move_straferight;
}

void priest_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

   if (!self->s.skinnum && self->health < self->max_health / 3)
		{
      self->s.skinnum = 1;
		gi.linkentity(self);
		}
   
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() & 1;

	if (n == 0)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &priest_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &priest_move_pain2;
      }

}


void fire_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
   if (damage == 15)
      {
      rocket->s.modelindex = gi.modelindex ("models/objects/spike/tris.md2");
	   rocket->s.effects |= EF_GRENADE;
	   rocket->touch = spike_touch;
      }
   else
      {
	   rocket->s.modelindex = gi.modelindex ("models/objects/fireball/tris.md2");
	   rocket->s.effects |= EF_ROCKET;
      rocket->s.renderfx = RF_TRANSLUCENT;
	   rocket->touch = rocket_touch;
      }
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
   rocket->avelocity[2] = 180;
   rocket->nextthink = level.time + 8000/speed;
	rocket->owner = self;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "fireball";
	gi.linkentity (rocket);
}

void priest_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20 + (rand() & 15), 50);
}


void priest_meditate(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_meditate1;
}

void priest_meditate2(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_meditate2;
}

void priest_talk(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_talk1;
}

void priest_talk2(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_talk2;
}

void priest_pray(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_pray;
}

void priest_worship(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_worship1;
}

void priest_worship2(edict_t *self)
{
   self->monsterinfo.currentmove = &priest_move_worship2;
}



mmove_t priest_move_decap = {FRAME_decapStart, FRAME_decapEnd, priest_frames_death1, priest_dead};

void priest_decap (edict_t *self, int damage)
{
   self->monsterinfo.currentmove = &priest_move_decap;
	gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	ThrowGib (self, "models/monsters/priest/gibs/head.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_priest (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

   /*
	sound_pain1 = gi.soundindex ("priest/pain1.wav");
	sound_pain2 = gi.soundindex ("priest/pain2.wav");
	sound_die1 = gi.soundindex ("priest/death1.wav");
	sound_die2 = gi.soundindex ("priest/death2.wav");

	sound_rocketshot = gi.soundindex ("priest/fire.wav");

	sound_melee_hit = gi.soundindex ("priest/melee.wav");
	sound_sight = gi.soundindex ("priest/sight.wav");
	sound_search = gi.soundindex ("priest/search.wav");
	sound_idle = gi.soundindex ("priest/idle.wav");
   */

	sound_sight    = gi.soundindex ("monsters/priest/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/priest/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/priest/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/priest/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/priest/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/priest/die1.wav");
	sound_die2     = gi.soundindex ("monsters/priest/die2.wav");
	sound_gib	   = gi.soundindex ("monsters/priest/gib.wav");
	sound_step	   = gi.soundindex ("monsters/priest/step.wav");

   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/priest/tris.md2");
	VectorSet (self->mins, -12, -11, 0);
	VectorSet (self->maxs, 11, 21, 60);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 180;

	self->pain = priest_pain;
	self->die = priest_die;

   self->monsterinfo.stand = priest_stand;
	self->monsterinfo.walk = priest_walk;
	self->monsterinfo.run = priest_run;
	self->monsterinfo.dodge = priest_dodge;
	self->monsterinfo.attack = priest_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = priest_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
	self->monsterinfo.action[0] = priest_meditate;
	self->monsterinfo.action[1] = priest_crouch;
	self->monsterinfo.action[2] = priest_worship;
	self->monsterinfo.action[3] = priest_pray;
	self->monsterinfo.action[4] = priest_talk;

	self->monsterinfo.decap = priest_decap;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);

   walkmonster_start (self);
		
   if (self->spawnflags & PRIEST_CROUCH)
      self->monsterinfo.currentmove = &priest_move_crouch;
   else if (self->spawnflags & PRIEST_WALK)
      self->monsterinfo.currentmove = &priest_move_walk;
   else
		self->monsterinfo.currentmove = &priest_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD;
}
