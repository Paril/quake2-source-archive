/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mzombie.c
  Description: AI/Physics for Zombie Priest

\**********************************************************/

#include "g_local.h"
#include "b_mzombie.h"

void zombie_pain (edict_t *self, edict_t *other, float kick, int damage);
void zombie_spawnbooks (edict_t *self);
void zombie_blue_explode (edict_t *ent, int size);
void zombie_crouch2 (edict_t *self);
void zombie_talk2 (edict_t *self);
void zombie_walk2 (edict_t *self);
void zombie_stand2 (edict_t *self);
void zombie_spread2 (edict_t *self);

#define ZOMBIE_CROUCH 8
#define ZOMBIE_WALK 16

static int	sound_sight;
static int	sound_step;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_gib;


void zombie_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}


mframe_t zombie_frames_stand [] =
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

mframe_t zombie_frames_action [] =
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
	BOT_move, 0, NULL
};

mmove_t zombie_move_stand = {FRAME_Stand_start, FRAME_Stand_end, zombie_frames_stand, NULL};
mmove_t zombie_move_crouch1 = {FRAME_Crouch1_start, FRAME_Crouch1_end, zombie_frames_stand, zombie_crouch2};
mmove_t zombie_move_crouch2 = {FRAME_Crouch2_start, FRAME_Crouch2_end, zombie_frames_stand, NULL};
mmove_t zombie_move_talk = {FRAME_Talk_start, FRAME_Talk_end, zombie_frames_action, NULL};
mmove_t zombie_move_laugh = {FRAME_Laugh_start, FRAME_Laugh_end, zombie_frames_action, NULL};
mmove_t zombie_move_spread1 = {FRAME_Spread1_start, FRAME_Spread1_end, zombie_frames_action, zombie_spread2};
mmove_t zombie_move_spread2 = {FRAME_Spread2_start, FRAME_Spread2_end, zombie_frames_action, NULL};
mmove_t zombie_move_spread3 = {FRAME_Spread3_start, FRAME_Spread3_end, zombie_frames_action, zombie_stand2};
mframe_t zombie_frames_walk1 [] =
{
	BOT_walk, 1,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 3,  NULL
};

mframe_t zombie_frames_walk2 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  zombie_step,
   BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  zombie_step,
	BOT_walk, 3,  NULL
};

mframe_t zombie_frames_walk3 [] =
{
	BOT_walk, 3,  NULL,
	BOT_walk, 2,  NULL,
	BOT_walk, 1,  NULL
};

mmove_t zombie_move_walk1 = {FRAME_Walk1_start, FRAME_Walk1_end, zombie_frames_walk1, zombie_walk2};
mmove_t zombie_move_walk2 = {FRAME_Walk2_start, FRAME_Walk2_end, zombie_frames_walk2, NULL};
mmove_t zombie_move_walk3 = {FRAME_Walk3_start, FRAME_Walk3_end, zombie_frames_walk3, zombie_stand2};

void zombie_stand (edict_t *self)
{
	if (self->monsterinfo.currentmove == &zombie_move_walk2)
		self->monsterinfo.currentmove = &zombie_move_walk3;
	else if (self->monsterinfo.currentmove == &zombie_move_spread2)
		self->monsterinfo.currentmove = &zombie_move_spread3;
	else
		{
		self->monsterinfo.currentmove = &zombie_move_stand;
		VectorSet (self->mins, -16, -16, 0);
		VectorSet (self->maxs, 5, 16, 54);
		}
}

void zombie_stand2 (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_stand;
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 5, 16, 54);
}

void zombie_crouch (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_crouch1;
}

void zombie_crouch2 (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_crouch2;
}


void zombie_walk (edict_t *self)
{
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 5, 16, 54);
	self->monsterinfo.currentmove = &zombie_move_walk1;
}

void zombie_walk2 (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_walk2;
}

mframe_t zombie_frames_run [] =
{
	BOT_run, 3,  NULL,
	BOT_run, 5,  NULL,
	BOT_run, 8,  zombie_step,
	BOT_run, 12, NULL,
	BOT_run, 15, NULL,
	BOT_run, 3,  NULL,
	BOT_run, 5,  zombie_step,
	BOT_run, 12, NULL
};
mmove_t zombie_move_run = {FRAME_Walk2_start, FRAME_Walk2_end, zombie_frames_run, NULL};

void zombie_run (edict_t *self)
{
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 5, 16, 54);
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &zombie_move_stand;
	else
		self->monsterinfo.currentmove = &zombie_move_run;
}


mframe_t zombie_frames_pain [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, 1, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};
mmove_t zombie_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, zombie_frames_pain, zombie_run};
mmove_t zombie_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, zombie_frames_pain, zombie_run};

void zombie_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void zombie_dead (edict_t *self)
{
	VectorSet (self->mins, -13, -14, -2);
	VectorSet (self->maxs, 53, 11, 12);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t zombie_frames_death [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t zombie_move_death = {FRAME_Die_start, FRAME_Die_end, zombie_frames_death, zombie_dead};

void zombie_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// check for gib
	if (self->health <= self->gib_health)
		{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/zombie_priest/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/zombie_priest/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/zombie_priest/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/zombie_priest/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/zombie_priest/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/zombie_priest/gibs/head.md2", damage, GIB_METALLIC);
		return;
		}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

   self->monsterinfo.currentmove = &zombie_move_death;
   gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}

void zombie_rail_think (edict_t *self)
{
	vec3_t end;
	trace_t	tr;
	edict_t *explode;

	if (!self->enemy || self->enemy->health <= 0)
		{
		G_FreeEdict(self);
		return;
		}
	if (!self->owner || self->owner->health <= 0)
		{
		G_FreeEdict(self);
		return;
		}

	self->nextthink = level.time + FRAMETIME;
	VectorMA(self->s.origin, 100, self->pos1, end);
	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
	VectorCopy(end, self->s.origin);
	end[0] = tr.endpos[0];
	end[1] = tr.endpos[1];
	end[2] = tr.endpos[2] - 8192;

	tr = gi.trace (tr.endpos, NULL, NULL, end, self, MASK_SHOT);

	end[0] = tr.endpos[0];
	end[1] = tr.endpos[1];
	end[2] = tr.endpos[2] + 80;
	tr.endpos[2] += 2;
	explode = G_Spawn();
	VectorCopy(tr.endpos, explode->s.origin);
	zombie_blue_explode (explode, 1);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (end);
	gi.WritePosition (tr.endpos);
	gi.multicast (tr.endpos, MULTICAST_PHS);

	if (tr.fraction < 1)
		{
		if (tr.ent->takedamage)
			T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 20, 130, 0, 0);
		else
			{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SPARKS);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (self->pos1);
			gi.multicast (tr.endpos, MULTICAST_PVS);
			}
		}	
}


void zombie_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	offset;
	vec3_t	start;
	edict_t	*rail;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_IDLE, 0);
	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, 0, 16);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	rail = G_Spawn();
	rail->classname = "Rail\n";
	rail->owner = self;
	rail->enemy = self->enemy;
	rail->think = zombie_rail_think;
	rail->nextthink = level.time + FRAMETIME;
	gi.linkentity(rail);
	VectorCopy (forward, rail->pos1);
	VectorCopy (start, rail->s.origin);
}

mframe_t zombie_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  zombie_fire,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t zombie_move_attack1 = {FRAME_Attack1_start, FRAME_Attack1_end, zombie_frames_attack1, zombie_run};

/*
mframe_t zombie_frames_attack2 [] =
{
	BOT_charge, 0, zombie_spawnbooks,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t zombie_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, zombie_frames_attack2, zombie_run};
*/
void zombie_attack(edict_t *self)
{
   //self->monsterinfo.nextattack = level.time + 2;
	if (rand() & 1)
      {
		self->monsterinfo.currentmove = &zombie_move_attack1;
      self->monsterinfo.nextattack = level.time + 2;
      }
	else
      {
      self->monsterinfo.nextattack = level.time + 2;
		self->monsterinfo.currentmove = &zombie_move_attack1;
      }
}


mframe_t zombie_frames_strafeleft [] =
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
mmove_t zombie_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, zombie_frames_strafeleft, zombie_run};

mframe_t zombie_frames_straferight [] =
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
mmove_t zombie_move_straferight = {FRAME_Right_start, FRAME_Right_end, zombie_frames_straferight, zombie_run};

void zombie_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &zombie_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &zombie_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &zombie_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &zombie_move_straferight;
}

void zombie_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &zombie_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &zombie_move_pain2;
      }
}

mmove_t zombie_move_sleep = {FRAME_Sleep, FRAME_Sleep, zombie_frames_action, NULL};
mmove_t zombie_move_wakeup = {FRAME_Wakeup_start, FRAME_Wakeup_end, zombie_frames_action, zombie_stand};
mmove_t zombie_move_meditate = {FRAME_Meditate_start, FRAME_Meditate_end, zombie_frames_action, NULL};
mmove_t zombie_move_medistand = {FRAME_Medistand_start, FRAME_Medistand_end, zombie_frames_action, zombie_stand};
mmove_t zombie_move_spell = {FRAME_Spread1_start, FRAME_Spread1_end, zombie_frames_action, zombie_spread2};

void zombie_sleep (edict_t *self)
{
	VectorSet (self->mins, -50, -11, -7);
	VectorSet (self->maxs, 6, 11, 12);
	self->monsterinfo.currentmove = &zombie_move_sleep;
}

void zombie_wakeup (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_wakeup;
}

void zombie_meditate (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_meditate;
}

void zombie_medistand (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_medistand;
}

void zombie_spell (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_spread1;
}

void zombie_spread2 (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_spread2;
}

void zombie_talk (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_laugh;
}

void zombie_laugh (edict_t *self)
{
	self->monsterinfo.currentmove = &zombie_move_talk;
}

void zombie_blue_explode_think(edict_t *self)
{
   if (self->s.frame < self->status)
      self->s.frame++;
   else
      self->think = G_FreeEdict;
   self->nextthink = level.time + FRAMETIME;               
}

void zombie_blue_explode (edict_t *ent, int size)
{
   float k = random();
	if (!size)
		ent->s.modelindex = gi.modelindex ("models/objects/flash1/tris.md2");
	else
		ent->s.modelindex = gi.modelindex ("models/objects/flash1/small.md2");
   ent->s.renderfx |= RF_TRANSLUCENT;
   ent->s.effects |= EF_FLAG2;
   if (k < .3)   
      {
      ent->s.frame = 0;
      ent->status = 14;
      }
   else if (k < .6)
      {
      ent->s.frame = 15;
      ent->status = 29;
      }
   else 
      {
      ent->s.frame = 30;
      ent->status = 48;
      }
   ent->think = zombie_blue_explode_think;
   ent->nextthink = level.time + FRAMETIME;
   gi.linkentity(ent);
}


void zombie_bookattack (edict_t *self)
{
	vec3_t forward, end;
	trace_t tr;

	if (!self->enemy || self->enemy->health <= 0)
		{
		G_FreeEdict(self);
		return;
		}
	if (!self->owner || self->owner->health <= 0)
		{
		G_FreeEdict(self);
		return;
		}

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorMA(self->s.origin, 4096, forward, end);
	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);

	if (tr.fraction < 1)
		{
		if (tr.ent->takedamage)
			T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 2, 10, 0, 0);
		else
			{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SPARKS);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (forward);
			gi.multicast (tr.endpos, MULTICAST_PVS);
			}
		}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(tr.endpos);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	//check_dodge (self, self->s.origin, forward, 800);
	//self->owner->s.frame = FRAME_Attack1_end - 2;
	self->nextthink = level.time + FRAMETIME;
	if (--self->delay <= 0)
		self->think = G_FreeEdict;
}

void zombie_spawnbooks (edict_t *self)
{
	edict_t *book[4], *blue_mushroom;
	vec3_t forward, right, offset;	
	int x;

	if (!self->enemy || self->enemy->health <= 0)
		return;

	blue_mushroom = G_Spawn();
	VectorCopy (self->s.origin, blue_mushroom->s.origin);
	blue_mushroom->classname = "BlueExplode\n";
	zombie_blue_explode(blue_mushroom, 0);
   AngleVectors (self->s.angles, forward, right, NULL);
	for (x = 0; x < 4; x++)
		{
		book[x] = G_Spawn();
		book[x]->classname = "Boook\n";
		book[x]->owner = self;
		book[x]->enemy = self->enemy;
		book[x]->think = zombie_bookattack;
		book[x]->nextthink = level.time + .5;
		book[x]->s.modelindex = gi.modelindex("models/objects/books/tris2.md2");
		book[x]->delay = 7;
		book[x]->s.renderfx = RF_FULLBRIGHT;
		book[x]->avelocity[1] = 180;
		book[x]->velocity[2] = 70;
		book[x]->movetype = MOVETYPE_FLY;
		}
	VectorSet(offset, 0, -30, -30);
	G_ProjectSource (self->s.origin, offset, forward, right, book[0]->s.origin);
	VectorSet(offset, 30, 0, -30);
	G_ProjectSource (self->s.origin, offset, forward, right, book[1]->s.origin);
	VectorSet(offset, 0, 30, -30);
	G_ProjectSource (self->s.origin, offset, forward, right, book[2]->s.origin);
	VectorSet(offset, -30, 0, -30);
	G_ProjectSource (self->s.origin, offset, forward, right, book[3]->s.origin);
	gi.linkentity(book[0]);
	gi.linkentity(book[1]);
	gi.linkentity(book[2]);	
	gi.linkentity(book[3]);	
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_zombie (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("monsters/zombie_priest/sight.wav");
	sound_step		= gi.soundindex ("monsters/zombie_priest/step.wav");
	sound_attack1  = gi.soundindex ("monsters/zombie_priest/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/zombie_priest/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/zombie_priest/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/zombie_priest/pain2.wav");
	sound_die      = gi.soundindex ("monsters/zombie_priest/die.wav");
	sound_gib      = gi.soundindex ("monsters/zombie_priest/gib.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/zombie_priest/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 5, 16, 54);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 220;

	self->pain = zombie_pain;
	self->die = zombie_die;

	self->monsterinfo.stand = zombie_stand;
	self->monsterinfo.walk = zombie_walk;
	self->monsterinfo.run = zombie_run;
	self->monsterinfo.dodge = zombie_dodge;
	self->monsterinfo.attack = zombie_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = zombie_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
	self->monsterinfo.action[0] = zombie_sleep;
	self->monsterinfo.action[1] = zombie_wakeup;
	self->monsterinfo.action[2] = zombie_meditate;
	self->monsterinfo.action[3] = zombie_medistand;
	self->monsterinfo.action[4] = zombie_spell;
	self->monsterinfo.action[5] = zombie_talk;
	self->monsterinfo.action[6] = zombie_laugh;
	self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);
	
   if (self->spawnflags & ZOMBIE_CROUCH)
		self->monsterinfo.currentmove = &zombie_move_crouch1;
   else if (self->spawnflags & ZOMBIE_WALK)
		self->monsterinfo.currentmove = &zombie_move_walk1;
	else
		self->monsterinfo.currentmove = &zombie_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_DROPBLOOD;
}

