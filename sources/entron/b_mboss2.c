/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mboss2.c
  Description: AI/Physics for Overseer 

\**********************************************************/

#include "g_local.h"
#include "b_mboss2.h"

void find_node_hide (edict_t *self);
void boss2_pain (edict_t *self, edict_t *other, float kick, int damage);
void boss2_missile (edict_t *self);
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void fire_blue_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);

#define BOSS2_WALK 16

static int	sound_sight;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_attack3;
static int	sound_die;
static int	sound_idle;
static int	sound_gib;


void boss2_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

mframe_t boss2_frames_stand [] =
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
mmove_t boss2_move_stand = {FRAME_Stand_start, FRAME_Stand_end, boss2_frames_stand, NULL};

void boss2_stand (edict_t *self)
{
   self->monsterinfo.currentmove = &boss2_move_stand;
}

mframe_t boss2_frames_walk [] =
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
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL,
	BOT_walk, 3,  NULL	
};
mmove_t boss2_move_walk = {FRAME_Walk_start, FRAME_Walk_end, boss2_frames_walk, NULL};

void boss2_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &boss2_move_walk;
}

mframe_t boss2_frames_run [] =
{
	BOT_run, 7,  NULL,
	BOT_run, 15, NULL,
	BOT_run, 20, NULL,
	BOT_run, 7,  NULL,
	BOT_run, 15, NULL,
	BOT_run, 20, NULL
};
mmove_t boss2_move_run = {FRAME_Run_start, FRAME_Run_end, boss2_frames_run, NULL};

void boss2_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &boss2_move_stand;
	else
		self->monsterinfo.currentmove = &boss2_move_run;
}


mframe_t boss2_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t boss2_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, boss2_frames_pain1, boss2_run};
mmove_t boss2_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, boss2_frames_pain1, boss2_run};
mmove_t boss2_move_pain3 = {FRAME_Pain3_start, FRAME_Pain3_end, boss2_frames_pain1, boss2_run};

void boss2_dead (edict_t *self)
{
	VectorSet (self->mins, -75, -40, -2);
	VectorSet (self->maxs, 58, 28, 24);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	fix_slope(self);
	gi.linkentity (self);
}

mframe_t boss2_frames_death [] =
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
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};
mmove_t boss2_move_death = {FRAME_Die_start, FRAME_Die_end, boss2_frames_death, boss2_dead};
mmove_t boss2_move_laugh = {FRAME_Laugh_start, FRAME_Laugh_end, boss2_frames_death, boss2_stand};
mmove_t boss2_move_talk1 = {FRAME_Talk1_start, FRAME_Talk1_end, boss2_frames_death, boss2_stand};
mmove_t boss2_move_talk2 = {FRAME_Talk2_start, FRAME_Talk2_end, boss2_frames_death, boss2_stand};
mmove_t boss2_move_talk3 = {FRAME_Talk3_start, FRAME_Talk3_end, boss2_frames_death, boss2_stand};

void boss2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, sound_gib, 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		ThrowGib (self, "models/monsters/overseer/gibs/arm_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/overseer/gibs/arm_r.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/overseer/gibs/body.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/overseer/gibs/leg_l.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/overseer/gibs/leg_r.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/overseer/gibs/head.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &boss2_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
}


void boss2_fire (edict_t *self)
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
	VectorSet(offset, 20, -10, 12);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_blue_blaster (self, start, forward, 15, 1600);	
   self->s.angles[0] = 0;
   self->s.angles[2] = 0;

	gi.sound (self, CHAN_WEAPON, sound_attack1, 1, ATTN_NORM, 0);
	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

void boss2_rocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, -9, 12);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	monster_fire_rocket (self, start, forward, 80, 1200, MZ2_BOSS2_ROCKET_2);

	VectorSet(offset, 20, 9, 12);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	monster_fire_rocket (self, start, forward, 80, 1200, MZ2_BOSS2_ROCKET_2);
   self->s.angles[0] = 0;
   self->s.angles[2] = 0;
	gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);

	//gi.WriteByte (svc_muzzleflash2);
	//gi.WriteShort (self - g_edicts);
	//gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

mframe_t boss2_frames_attack1 [] =
{
	BOT_charge, 0, boss2_fire,
	BOT_charge, -1,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t boss2_move_attack1 = {FRAME_Attack_start, FRAME_Attack_end, boss2_frames_attack1, boss2_run};

mframe_t boss2_frames_attack2 [] =
{
	BOT_charge, 0, boss2_rocket,
	BOT_charge, -1,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL,
	BOT_charge, 0,  NULL
};
mmove_t boss2_move_attack2 = {FRAME_Attack2_start, FRAME_Attack2_end, boss2_frames_attack2, boss2_run};

mframe_t boss2_frames_attack3 [] =
{
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, boss2_missile,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t boss2_move_attack3 = {FRAME_Attack3_start, FRAME_Attack3_end, boss2_frames_attack3, boss2_run};

void boss2_attack(edict_t *self)
{
	float x = range (self, self->enemy);
   self->monsterinfo.nextattack = level.time + 1;
	if (x == RANGE_MELEE)
      {
	   self->monsterinfo.currentmove = &boss2_move_attack3;
      self->monsterinfo.nextattack = level.time + .5;
      }
	else if (x > RANGE_NEAR)
      {
      self->monsterinfo.nextattack = level.time + 3;
		self->monsterinfo.currentmove = &boss2_move_attack2;
      }
	else
      {
		self->monsterinfo.pausetime = level.time + 2;
      self->monsterinfo.nextattack = level.time + 4;
		self->monsterinfo.currentmove = &boss2_move_attack1;
      }
}


void boss2_dodge (edict_t *self, edict_t *attacker, float eta)
{

	//vec3_t	offset, forward, right, end;
   //trace_t  trace;

	if (!self->enemy)
		self->enemy = attacker;

   //VectorSet(offset, 0, 90, 0);
   //AngleVectors (self->s.angles, forward, right, NULL);
}

void boss2_pain (edict_t *self, edict_t *other, float kick, int damage)
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
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = random();

	if (n < .33)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss2_move_pain1;
      }
	else if (n < .66)
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss2_move_pain2;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &boss2_move_pain3;
      }

}

void boss2_missile (edict_t *self)
{
	vec3_t	aim;
	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, 20 + (rand() & 15), 50))
	gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
}


void boss2_laugh (edict_t *self)
{
   self->monsterinfo.currentmove = &boss2_move_laugh;
}

void boss2_talk1 (edict_t *self)
{
   self->monsterinfo.currentmove = &boss2_move_talk1;
}

void boss2_talk2 (edict_t *self)
{
   self->monsterinfo.currentmove = &boss2_move_talk2;
}

void boss2_talk3(edict_t *self)
{
   self->monsterinfo.currentmove = &boss2_move_talk3;
}

void fire_blue_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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
	bolt->spawnflags = 1;
	bolt->s.effects |= EF_COLOR_SHELL | EF_BOOMER;
	bolt->s.renderfx |= RF_SHELL_BLUE;
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

void SP_monster_boss2 (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }
   /*
	sound_pain1 = gi.soundindex ("monsters/boss2/pain1.wav");
	sound_pain2 = gi.soundindex ("monsters/boss2/pain2.wav");
	sound_die1 = gi.soundindex ("hover/hovdeth1.wav");
	sound_die2 = gi.soundindex ("hover/hovdeth2.wav");

	sound_rocketshot = gi.soundindex ("monsters/boss2/shoot.wav");

	sound_melee_hit = gi.soundindex ("gladiator/melee1.wav");
	sound_sight = gi.soundindex ("monsters/boss2/sight.wav");
	sound_search = gi.soundindex ("priest/search.wav");
	sound_idle = gi.soundindex ("priest/idle.wav");
   */

	
	sound_sight		= gi.soundindex ("monsters/overseer/sight.wav");
	sound_attack1  = gi.soundindex ("monsters/overseer/attack1.wav");
	sound_attack2  = gi.soundindex ("monsters/overseer/attack2.wav");
	sound_attack3  = gi.soundindex ("monsters/overseer/attack3.wav");
	sound_pain1    = gi.soundindex ("monsters/overseer/pain1.wav");
	sound_pain2    = gi.soundindex ("monsters/overseer/pain2.wav");
	sound_pain3	   = gi.soundindex ("monsters/overseer/pain3.wav");
	sound_die	   = gi.soundindex ("monsters/overseer/die.wav");
	sound_gib		= gi.soundindex ("monsters/overseer/gib.wav");
   self->s.sound  = gi.soundindex ("monsters/overseer/engine.wav");;

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/overseer/tris.md2");
	VectorSet (self->mins, -25, -21, 0);
	VectorSet (self->maxs, 19, 21, 75);

	if (!self->health)
      self->health = 1800;
	if (!self->gib_health)
		self->gib_health = -420;
	self->mass = 800;

	self->pain = boss2_pain;
	self->die = boss2_die;

   self->monsterinfo.stand = boss2_stand;
	self->monsterinfo.walk = boss2_walk;
	self->monsterinfo.run = boss2_run;
	self->monsterinfo.dodge = boss2_dodge;
	self->monsterinfo.attack = boss2_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = boss2_sight;
	self->monsterinfo.idle = NULL;
   self->monsterinfo.nextattack = level.time + random() * 4;
   self->monsterinfo.pausetime = level.time + random() * 4;
   self->monsterinfo.search_time = level.time + random() * 4;
   self->monsterinfo.trail_time = level.time + random() * 4;
   self->status = 0;
   self->type = 0;
	self->monsterinfo.action[0] = boss2_laugh;
	self->monsterinfo.action[1] = boss2_talk1;
	self->monsterinfo.action[2] = boss2_talk2;
	self->monsterinfo.action[3] = boss2_talk3;
   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
   walkmonster_start (self);

   if (self->spawnflags & BOSS2_WALK)
		self->monsterinfo.currentmove = &boss2_move_walk;
   else
		self->monsterinfo.currentmove = &boss2_move_stand;

	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_METALIC | TYPE_MONSTER_FLAT;
   gi.linkentity(self);
}