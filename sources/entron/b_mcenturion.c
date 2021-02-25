/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mcenturion.c
  Description: AI/Physics for Centurion 

\**********************************************************/

#include "g_local.h"
#include "b_mcenturion.h"

void find_node_hide (edict_t *self);
void centurion_pain (edict_t *self, edict_t *other, float kick, int damage);
void fire_gauss (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void gauss_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void centurion_smack (edict_t *self);

#define CENTURION_CROUCH 8
#define CENTURION_WALK 16

static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_step;
static int	sound_gib;

void centurion_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

mframe_t centurion_frames_stand [] =
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
mmove_t centurion_move_stand = {FRAME_standStart, FRAME_standEnd, centurion_frames_stand, NULL};

void centurion_stand (edict_t *self)
{
   self->monsterinfo.currentmove = &centurion_move_stand;
}

mframe_t centurion_frames_crouch [] =
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
	BOT_stand, 0, NULL
};

mmove_t centurion_move_crouch = {FRAME_crouchStart, FRAME_crouchEnd, centurion_frames_crouch, NULL};

void centurion_crouch (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &centurion_move_crouch;
}

mframe_t centurion_frames_walk [] =
{
	BOT_walk, 0,  centurion_step,
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
	BOT_walk, 0,  NULL,
	BOT_walk, 0,  centurion_step,
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
	BOT_walk, 0,  NULL
};
mmove_t centurion_move_walk = {FRAME_walkStart, FRAME_walkEnd, centurion_frames_walk, NULL};

void centurion_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &centurion_move_walk;
}

mframe_t centurion_frames_run [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 26, centurion_step,
	BOT_run, 26, NULL,
	BOT_run, 0, NULL,
	BOT_run, 26, centurion_step,
	BOT_run, 0,  NULL
};
mmove_t centurion_move_run = {FRAME_runStart, FRAME_runEnd, centurion_frames_run, NULL};

void centurion_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &centurion_move_stand;
	else
		self->monsterinfo.currentmove = &centurion_move_run;
}


mframe_t centurion_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t centurion_move_pain1 = {FRAME_pain1Start, FRAME_pain1End, centurion_frames_pain1, centurion_run};

mframe_t centurion_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t centurion_move_pain2 = {FRAME_pain2Start, FRAME_pain2End, centurion_frames_pain2, centurion_run};

void centurion_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void centurion_dead (edict_t *self)
{
	VectorSet (self->mins, -55, -54, -2);
	VectorSet (self->maxs, 40, 61, 29);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t centurion_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t centurion_move_death1 = {FRAME_death1Start, FRAME_death1End, centurion_frames_death1, centurion_dead};

void centurion_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/centurion/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/centurion/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/centurion/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/centurion/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/centurion/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/centurion/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &centurion_move_death1;

	n = rand() & 1;
	if (n)
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
}

void centurion_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, 8, 2);
   vectoangles(forward, self->s.angles);
   self->s.angles[2] = 0;
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

   fire_gauss (self, start, forward, 80, 1500, 120, 120);
   //fire_shotgun (self, start, forward, 20, 140, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, 6, MOD_SSHOTGUN);
   self->s.angles[0] = 0;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (self-g_edicts);
	//gi.WriteByte (MZ_SSHOTGUN);
	//gi.multicast (start, MULTICAST_PVS);
}

mframe_t centurion_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, centurion_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t centurion_move_attack1 = {FRAME_attak1Start, FRAME_attak1End, centurion_frames_attack1, centurion_run};

mframe_t centurion_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, centurion_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t centurion_move_attack2 = {FRAME_attak2Start, FRAME_attak2End, centurion_frames_attack2, centurion_run};

void centurion_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
      {
		self->monsterinfo.currentmove = &centurion_move_attack2;
      self->monsterinfo.nextattack = level.time + 1;
		gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
      }
	else
      {
      self->monsterinfo.nextattack = level.time + 2;
		self->monsterinfo.currentmove = &centurion_move_attack1;
      }
}

void centurion_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void centurion_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void centurion_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t centurion_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL
	};
mmove_t centurion_move_strafeleft = {FRAME_leftStart, FRAME_leftEnd, centurion_frames_strafeleft, centurion_run};

mframe_t centurion_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL
};
mmove_t centurion_move_straferight = {FRAME_rightStart, FRAME_rightEnd, centurion_frames_straferight, centurion_run};

void centurion_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &centurion_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &centurion_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &centurion_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &centurion_move_straferight;
}


void centurion_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &centurion_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &centurion_move_pain2;
      }

}

void centurion_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 20 + (rand() & 15), 50);
}

void blue_explode_think(edict_t *self)
{
   if (self->s.frame < self->status)
      self->s.frame++;
   else
      self->think = G_FreeEdict;
   self->nextthink = level.time + FRAMETIME;               
}

void blue_explode (edict_t *ent)
{
   float k = random();
   ent->s.modelindex = gi.modelindex ("models/objects/flash1/tris.md2");
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
   gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/xpld_wat.wav"), 1, ATTN_NORM, 0);
   ent->touch = NULL;
   ent->solid = SOLID_NOT;
   ent->think = blue_explode_think;
   ent->nextthink = level.time + FRAMETIME;
   VectorClear(ent->velocity);
   ent->s.sound = 0;
   gi.linkentity(ent);
}


void fire_gauss (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
   rocket->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
   rocket->touch = gauss_touch;

	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
   rocket->s.angles[1] += 90;
   rocket->nextthink = level.time + 8000/speed;
	rocket->owner = self;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "gauss";
	gi.linkentity (rocket);
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_centurion (edict_t *self)
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
	sound_step     = gi.soundindex ("monsters/centurion/step.wav");
	sound_sight    = gi.soundindex ("monsters/centurion/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/centurion/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/centurion/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/centurion/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/centurion/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/centurion/die1.wav");
	sound_die2     = gi.soundindex ("monsters/centurion/die2.wav");
	sound_gib      = gi.soundindex ("monsters/centurion/gib.wav");
   

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/centurion/tris.md2");
	VectorSet (self->mins, -30, -40, 0);
	VectorSet (self->maxs, 23, 35, 84);

	if (!self->health)
      self->health = 600;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 600;

	self->pain = centurion_pain;
	self->die = centurion_die;

   self->monsterinfo.stand = centurion_stand;
	self->monsterinfo.walk = centurion_walk;
	self->monsterinfo.run = centurion_run;
	self->monsterinfo.dodge = centurion_dodge;
	self->monsterinfo.attack = centurion_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = centurion_sight;
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
	
	self->monsterinfo.currentmove = &centurion_move_stand;
   walkmonster_start (self);

   if (self->spawnflags & CENTURION_CROUCH)
		self->monsterinfo.currentmove = &centurion_move_crouch;
   else if (self->spawnflags & CENTURION_WALK)
		self->monsterinfo.currentmove = &centurion_move_walk;
   else
		self->monsterinfo.currentmove = &centurion_move_stand;
	
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_METALIC;
   gi.linkentity(self);
}
