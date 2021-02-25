/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mpodmedic.c
  Description: AI/Physics for Pod Medic

\**********************************************************/

#include "g_local.h"
#include "b_mpod.h"

void medicLaserGun (edict_t *self);
void medicRocketLauncher (edict_t *self);
void find_node_hide (edict_t *self);
void medic_pain (edict_t *self, edict_t *other, float kick, int damage);
void laser_heal(edict_t *self);
void medic_closed(edict_t *self);
void medic_duck_down (edict_t *self);
void medic_duck_hold (edict_t *self);
void medic_duck_up (edict_t *self);
void medic_run (edict_t *self);
void medic_check_health (edict_t *self);
void fire_teargas (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void BOT_close (edict_t *self, float dist);

#define POD_CLOSED   8
#define POD_WALK    16

static int	sound_idle;
static int	sound_open;
static int	sound_step;
static int	sound_sight;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int	sound_gib;

void podmedic_step (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_STATIC, 0);
}

void medic_open_sound(edict_t *self)
{
   gi.sound (self, CHAN_WEAPON, sound_open, 1, ATTN_NORM, 0);	
}

mframe_t medic_frames_open1 [] =
{
   BOT_close, 0, NULL,
	BOT_move, 0, medic_open_sound,
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

mframe_t medic_frames_close1 [] =
{
	BOT_stand, 0, NULL,
};

mmove_t medic_move_open1 = {FRAME_open101, FRAME_open112, medic_frames_open1, medic_run};

mframe_t medic_frames_stand [] =
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
	BOT_stand, 0, medic_check_health,
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
mmove_t medic_move_stand = {FRAME_stand01, FRAME_stand30, medic_frames_stand, NULL};

void medic_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_stand;
}


mframe_t medic_frames_fidget [] =
{
	BOT_move, 0, medic_duck_down,
	BOT_move, 0, medic_duck_hold,
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
	BOT_move, 0,  medic_duck_up
};
mmove_t medic_move_fidget = {FRAME_duck01, FRAME_duck20, medic_frames_fidget, medic_stand};

void medic_fidget (edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_fidget;
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

mframe_t medic_frames_walk [] =
{
	BOT_walk, 8,  podmedic_step,
	BOT_walk, 7,  NULL,
	BOT_walk, 4,  NULL,
	BOT_walk, 5,  medic_check_health,
	BOT_walk, 4,  NULL
};
mmove_t medic_move_walk = {FRAME_walk01, FRAME_walk05, medic_frames_walk, NULL};

void medic_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &medic_move_walk;
}

mframe_t medic_frames_run [] =
{
	BOT_run, 30, podmedic_step,
	BOT_run, 20, NULL,
	BOT_run, 10, NULL,
	BOT_run,  7, medic_check_health,
	BOT_run,  5, NULL
};
mmove_t medic_move_run = {FRAME_run01, FRAME_run05, medic_frames_run, NULL};

void medic_run (edict_t *self)
{
   if (self->spawnflags & POD_CLOSED)
      {
      self->spawnflags &= ~POD_CLOSED;
      self->monsterinfo.currentmove = &medic_move_open1; 
      self->monsterinfo.stand = medic_stand;
      return;
      }

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &medic_move_stand;
	else
		self->monsterinfo.currentmove = &medic_move_run;
}


mframe_t medic_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, 1,  NULL,
	BOT_move, -1, NULL
};
mmove_t medic_move_pain1 = {FRAME_pain101, FRAME_pain107, medic_frames_pain1, medic_run};

mframe_t medic_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -3, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -2, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 2,  NULL,
	BOT_move, 5,  NULL
};
mmove_t medic_move_pain2 = {FRAME_pain201, FRAME_pain209, medic_frames_pain2, medic_run};

mframe_t medic_frames_cpain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -3, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -2, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 2,  NULL,
	BOT_move, 5,  NULL
};
mmove_t medic_move_cpain1 = {FRAME_cpain101, FRAME_cpain108, medic_frames_cpain1, medic_run};

mframe_t medic_frames_cpain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -3, NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -2, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 2,  NULL,
	BOT_move, 5,  NULL
};
mmove_t medic_move_cpain2 = {FRAME_cpain201, FRAME_cpain205, medic_frames_cpain2, medic_run};


void MedicLaserGun (edict_t *self)
{
	edict_t *laser;
	vec3_t	start;
   int      range;

   if (self->teamchain)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, start);
	range = VectorLength (start);

   if (range > 300)
      {
      self->monsterinfo.nextattack = level.time + .5;
      return;
      }
	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_IDLE, 0);
   self->monsterinfo.nextattack = level.time + 3;
   laser = G_Spawn();
   self->teamchain = laser;
   VectorCopy(self->s.origin, laser->s.origin);
   laser->owner = self;
   laser->enemy = self->enemy;
   laser->delay = level.time + 1;
	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	laser->s.frame = 2;

	// set the color
	laser->s.skinnum = 0xd0d1d2d3;
	laser->think = laser_heal;
   laser->nextthink = level.time + FRAMETIME;
   laser->spawnflags |= 0x80000001;
	laser->svflags &= ~SVF_NOCLIENT;
	laser->dmg = 8;

	VectorSet (laser->mins, -8, -8, -8);
	VectorSet (laser->maxs, 8, 8, 8);
	gi.linkentity (laser);
}

void medic_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void medic_dead (edict_t *self)
{
	VectorSet (self->mins, -27, -27, -3);
	VectorSet (self->maxs, 27, 27, 16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t medic_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t medic_move_death1 = {FRAME_death101, FRAME_death108, medic_frames_death1, medic_dead};

// Off with his head
mframe_t medic_frames_death2 [] =
{
	BOT_move, 0,   NULL,
	BOT_move, 1,   NULL,
	BOT_move, 5,   NULL,
	BOT_move, -1,  NULL,
	BOT_move, 0,   NULL,
	BOT_move, 1,   NULL,
	BOT_move, 1,   NULL,
	BOT_move, 4,   NULL,
	BOT_move, 3,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,  NULL
};
mmove_t medic_move_death2 = {FRAME_death201, FRAME_death212, medic_frames_death2, medic_dead};

mframe_t medic_frames_death3 [] =
{
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL,
	BOT_move, 0,   NULL
};
mmove_t medic_move_death3 = {FRAME_death301, FRAME_death318, medic_frames_death3, medic_dead};


void medic_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand() % 3;
	if (n == 0)
	   {
		self->monsterinfo.currentmove = &medic_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
	   }
	else if (n == 1)
	   {
		self->monsterinfo.currentmove = &medic_move_death2;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
	   }
	else
	   {
		self->monsterinfo.currentmove = &medic_move_death3;
		gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
	   }

}


void medic_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 16;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void medic_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void medic_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 16;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t medic_frames_duck [] =
{
	BOT_move, -2, medic_duck_down,
	BOT_move, -5, medic_duck_hold,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 4,  medic_duck_up
};
mmove_t medic_move_duck = {FRAME_duck01, FRAME_duck05, medic_frames_duck, medic_run};

void medic_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (!self->enemy)
		self->enemy = attacker;

    if (self->spawnflags & POD_CLOSED)
      return;

	self->monsterinfo.currentmove = &medic_move_duck;
}


void medic_fire (edict_t *self)
{
   vec3_t forward;
   if (!self->enemy || self->enemy->health <= 0)
      return;

	MedicLaserGun (self);
   if (self->enemy->enemy && self->enemy->enemy->client)
      {
      if (visible(self, self->enemy->enemy))
			{
			VectorSubtract (self->enemy->enemy->s.origin, self->s.origin, forward);
			VectorNormalize (forward);
			fire_teargas (self, self->s.origin, forward, 0, 30, 4, 120, MOD_TEARGAS);
			}
      }
}

mframe_t medic_frames_attack1 [] =
{
	BOT_charge, 4,  medic_fire, 
	BOT_charge, -1, NULL,
	BOT_charge, -1, NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, -1, NULL,
	BOT_charge, 1,  NULL
};
mmove_t medic_move_attack1 = {FRAME_attak101, FRAME_attak106, medic_frames_attack1, medic_run};


void medic_attack(edict_t *self)
{      
	self->monsterinfo.currentmove = &medic_move_attack1;
}



mmove_t medic_move_close1 = {FRAME_open112, FRAME_open101, medic_frames_open1, medic_closed};

mmove_t medic_move_closed = {FRAME_open101, FRAME_open101, medic_frames_open1, NULL};

void medic_open(edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_open1;
}

void medic_close(edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_close1;
}

void medic_closed(edict_t *self)
{
 	self->monsterinfo.currentmove = &medic_move_closed;
}

void medic_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

   if (level.time < self->pain_debounce_time)
	   return;

   /*
  if (!self->monsterinfo.goalnode)	
   if ((self->health < self->max_health / 4) && self->health > 0)
      {
      self->status |= STATUS_MONSTER_HIDE;
      self->monsterinfo.chaser = self->enemy;
      self->monsterinfo.goalnode = NULL;
      self->enemy = NULL;
      find_node_hide(self);
      }
   */

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() % 2;
   if (self->spawnflags & POD_CLOSED)
      {
      self->spawnflags &= ~POD_CLOSED;
      self->monsterinfo.currentmove = &medic_move_open1; 
      self->monsterinfo.stand = medic_stand;
      return;
      }
   
   if (self->monsterinfo.currentmove == &medic_move_duck)
      {
	   if (n == 0)
	      {
		   self->monsterinfo.currentmove = &medic_move_cpain1;
		   gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	      }
	   else
	      {
		   self->monsterinfo.currentmove = &medic_move_cpain2;
		   gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	      }
      return;
      }

	if (n == 0)
	   {
		self->monsterinfo.currentmove = &medic_move_pain1;
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   }
	else
	   {
		self->monsterinfo.currentmove = &medic_move_pain2;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   }
}


void laser_heal(edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;

   if (self->delay < level.time)
      {
      if (self->owner->health > 0)
         self->owner->monsterinfo.run(self->owner);
      self->owner->teamchain = NULL;
      G_FreeEdict(self);
      return;
      }          
   if (!self->enemy)
      return;

   VectorCopy (self->owner->s.origin, self->s.origin);
   self->s.origin[2] += 4;
	VectorCopy (self->s.origin, start);
	VectorCopy (self->movedir, last_movedir);
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
	VectorSubtract (point, start, self->movedir);
	VectorNormalize (self->movedir);
	if (!VectorCompare(self->movedir, last_movedir))
		self->spawnflags |= 0x80000000;

	ignore = self->owner;
	VectorMA (start, 2048, self->movedir, end);
	tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

   // heal it if we can
   if (tr.ent && tr.ent->health > 0)
      {
      tr.ent->health += self->dmg;            
      if (!tr.ent->client && (tr.ent->status & STATUS_CEC))
         {
         tr.ent->s.effects &= ~EF_FLAG1;
         tr.ent->status &= ~STATUS_CEC;
         G_FreeEdict(tr.ent->chain);
         tr.ent->chain = NULL;
         }
      if (tr.ent->health > tr.ent->max_health)
         {
         tr.ent->health = tr.ent->max_health;
         self->owner->enemy = NULL;
         gi.sound(self->owner, CHAN_VOICE, sound_attack2, 1, ATTN_NORM, 0);
         self->owner->monsterinfo.walk(self->owner);
         self->owner->teamchain = NULL;
         //if (self->owner->health > 0)
            //self->owner->monsterinfo.run(self->owner);
         G_FreeEdict(self);
         return;
         }
      }
     

	// if we hit something that's not a monster or player or is immune to lasers, we're done
	if (self->spawnflags & 0x80000000)
		{
		self->spawnflags &= ~0x80000000;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (4);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.WriteByte (self->s.skinnum);
		gi.multicast (tr.endpos, MULTICAST_PVS);
		}
	VectorCopy (tr.endpos, self->s.old_origin);
	self->nextthink = level.time + FRAMETIME;
}

void medic_check_health (edict_t *self)
{
   if (!(level.framenum & 7))
      return;
   self->health += 10;
   if (self->status & STATUS_CEC)
      {
      self->s.effects &= ~EF_FLAG1;
      self->status &= ~STATUS_CEC;
      G_FreeEdict (self->chain);
      self->chain = NULL;
      }
   if (self->health > self->max_health)
      self->health = self->max_health;
   
   else if (self->health < self->max_health / 3)
      {
      gi.sound(self, CHAN_BODY, gi.soundindex("world/spark1.wav"), 1, ATTN_NORM, 0);
      gi.WriteByte (svc_temp_entity);
	   gi.WriteByte (TE_SPARKS);
	   gi.WritePosition (self->s.origin);
      gi.WriteDir (self->movedir);
	   gi.multicast (self->s.origin, MULTICAST_PVS);
      }
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_podmedic (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

   /*
	sound_pain1 = gi.soundindex ("pod/pain1.wav");
	sound_pain2 = gi.soundindex ("pod/pain2.wav");
	sound_die1 = gi.soundindex ("pod/death1.wav");
	sound_die2 = gi.soundindex ("pod/death2.wav");

	sound_rocketshot = gi.soundindex ("world/deactivated.wav");
	sound_lasershot = gi.soundindex ("tank/tnkatck3.wav");
	sound_open = gi.soundindex ("pod/open.wav");
	
	sound_sight = gi.soundindex ("pod/sight.wav");
	sound_search = gi.soundindex ("pod/search.wav");
	sound_idle = gi.soundindex ("pod/idle.wav");   
   */

	sound_idle     = gi.soundindex ("monsters/pod/idle.wav");
	sound_open     = gi.soundindex ("monsters/pod/open.wav");
	sound_sight    = gi.soundindex ("monsters/pod/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/pod/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/pod/attack2.wav");
	sound_pain1    = gi.soundindex ("monsters/pod/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/pod/pain2.wav");
	sound_die1     = gi.soundindex ("monsters/pod/die1.wav");
	sound_die2     = gi.soundindex ("monsters/pod/die2.wav");  
	sound_step     = gi.soundindex ("monsters/pod/step.wav");  
	sound_gib		= gi.soundindex ("monsters/pod/gib.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/pod/tris.md2");
	VectorSet (self->mins, -24, -24, 0);
	VectorSet (self->maxs, 24, 24, 32);

	if (!self->health)
      self->health = 200;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 130;

	self->pain = medic_pain;
	self->die = medic_die;

   self->monsterinfo.stand = medic_stand;
	self->monsterinfo.walk = medic_walk;
	self->monsterinfo.run = medic_run;
	self->monsterinfo.dodge = medic_dodge;
	self->monsterinfo.attack = medic_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = medic_sight;
	self->monsterinfo.idle = medic_fidget;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type |= TYPE_MONSTER_TEAMMATE | TYPE_MONSTER_FLAT | TYPE_MONSTER_METALIC;
   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & POD_CLOSED)
		self->monsterinfo.currentmove = &medic_move_closed;
   else if (self->spawnflags & POD_WALK)
		self->monsterinfo.currentmove = &medic_move_walk;
   else
		self->monsterinfo.currentmove = &medic_move_stand;
   
	self->s.skinnum = 0;
	self->monsterinfo.scale = MODEL_SCALE;
}