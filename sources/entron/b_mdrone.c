/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mdrone.c
  Description: AI/Physics for Drone 

\**********************************************************/

#include "g_local.h"
#include "b_mdrone.h"

void find_node_hide (edict_t *self);
void drone_pain (edict_t *self, edict_t *other, float kick, int damage);
void drone_smack (edict_t *self);
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);

#define DRONE_WALK 16

static int	sound_engine;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_die1;
static int	sound_die2;
static int	sound_sight;
static int	sound_idle;
static int	sound_gib;


mframe_t drone_frames_stand [] =
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
mmove_t drone_move_stand = {FRAME_Stand_start, FRAME_Stand_end, drone_frames_stand, NULL};

void drone_stand (edict_t *self)
{
   self->monsterinfo.currentmove = &drone_move_stand;
}


mframe_t drone_frames_walk [] =
{
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
mmove_t drone_move_walk = {FRAME_Walk_start, FRAME_Walk_end, drone_frames_walk, NULL};

void drone_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &drone_move_walk;
}

mframe_t drone_frames_run [] =
{
	BOT_run, 24,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 17,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 17,  NULL
};
mmove_t drone_move_run = {FRAME_Run_start, FRAME_Run_end, drone_frames_run, NULL};

void drone_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &drone_move_stand;
	else
		self->monsterinfo.currentmove = &drone_move_run;
}


mframe_t drone_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t drone_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, drone_frames_pain1, drone_run};

mframe_t drone_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t drone_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, drone_frames_pain2, drone_run};

void drone_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void drone_dead (edict_t *self)
{
	VectorSet (self->mins, -27, -8, 0);
	VectorSet (self->maxs, 26, 7, 14);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t drone_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t drone_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, drone_frames_death1, drone_dead};


void drone_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/drone/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/drone/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/drone/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/drone/gibs/tail.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/drone/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &drone_move_death1;
	gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
}


void drone_fire (edict_t *self)
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
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_blaster (self, start, forward, 15, 1600, 0x00000060 /*EF_HYPERBLASTER*/, 1);	
   self->s.angles[0] = 0;
   self->s.angles[2] = 0;
	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);

	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

mframe_t drone_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, drone_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0,  NULL
};
mmove_t drone_move_attack1 = {FRAME_Attack_start, FRAME_Attack_end, drone_frames_attack1, drone_run};

mframe_t drone_frames_attack2 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, drone_smack,
	BOT_charge, 0, NULL,
	BOT_charge, 0, drone_smack,
	BOT_charge, 0,  NULL
};
mmove_t drone_move_attack2 = {FRAME_Spin_start, FRAME_Spin_end, drone_frames_attack2, drone_run};

void drone_attack(edict_t *self)
{
   self->monsterinfo.nextattack = level.time + 1;
	if (range (self, self->enemy) == RANGE_MELEE)
      {
	   self->monsterinfo.currentmove = &drone_move_attack2;
      self->monsterinfo.nextattack = level.time + .5;
      }
	else
      {
      self->monsterinfo.nextattack = level.time + 2;
		self->monsterinfo.currentmove = &drone_move_attack1;
      }
}

void drone_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void drone_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void drone_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t drone_frames_strafeleft [] =
{
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL,
	BOT_move_left, 25, NULL
};
mmove_t drone_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, drone_frames_strafeleft, drone_run};

mframe_t drone_frames_straferight [] =
{
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL,
	BOT_move_right, 25, NULL
};
mmove_t drone_move_straferight = {FRAME_Right_start, FRAME_Right_end, drone_frames_straferight, drone_run};

void drone_dodge (edict_t *self, edict_t *attacker, float eta)
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
	      self->monsterinfo.currentmove = &drone_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &drone_move_strafeleft;
      return;         
      }
   offset[1] = -90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
      {
	   self->monsterinfo.currentmove = &drone_move_strafeleft;
      return;
      }
   offset[1] = 90;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
   trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
   if (trace.fraction == 1)
	   self->monsterinfo.currentmove = &drone_move_straferight;
}

void drone_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	   self->monsterinfo.currentmove = &drone_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &drone_move_pain2;
      }

}

void drone_smack (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 20 + (rand() & 15), 50))
	gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);
}

/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_GATTLING;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
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
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (hyper)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_drone (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }
   /*
	sound_pain1 = gi.soundindex ("monsters/drone/pain1.wav");
	sound_pain2 = gi.soundindex ("monsters/drone/pain2.wav");
	sound_die1 = gi.soundindex ("hover/hovdeth1.wav");
	sound_die2 = gi.soundindex ("hover/hovdeth2.wav");

	sound_rocketshot = gi.soundindex ("monsters/drone/shoot.wav");

	sound_melee_hit = gi.soundindex ("gladiator/melee1.wav");
	sound_sight = gi.soundindex ("monsters/drone/sight.wav");
	sound_search = gi.soundindex ("priest/search.wav");
	sound_idle = gi.soundindex ("priest/idle.wav");
   */

	
	sound_sight    = gi.soundindex ("monsters/drone/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/drone/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/drone/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/drone/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/drone/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/drone/die1.wav");
	sound_die2     = gi.soundindex ("monsters/drone/die2.wav");  
	sound_gib      = gi.soundindex ("monsters/drone/gib.wav");  
   self->s.sound  = gi.soundindex ("monsters/drone/engine.wav");;

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/drone/tris.md2");
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 84);

	if (!self->health)
      self->health = 100;
	if (!self->gib_health)
		self->gib_health = -120;
	self->mass = 70;

	self->pain = drone_pain;
	self->die = drone_die;

   self->monsterinfo.stand = drone_stand;
	self->monsterinfo.walk = drone_walk;
	self->monsterinfo.run = drone_run;
	self->monsterinfo.dodge = drone_dodge;
	self->monsterinfo.attack = drone_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = drone_sight;
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

   if (self->spawnflags & DRONE_WALK)
		self->monsterinfo.currentmove = &drone_move_walk;
   else
		self->monsterinfo.currentmove = &drone_move_stand;
 	
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_METALIC | TYPE_MONSTER_FLAT;
   gi.linkentity(self);
}