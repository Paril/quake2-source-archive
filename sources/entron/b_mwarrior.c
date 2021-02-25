/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mwarrior.c
  Description: AI/Physics for Warrior

\**********************************************************/

#include "g_local.h"
#include "b_mwarrior.h"

void find_node_hide (edict_t *self);
void warrior_pain (edict_t *self, edict_t *other, float kick, int damage);
void warrior_smack (edict_t *self);
void warrior_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void GoAmbush (edict_t *ent, edict_t *enemy, float dist);
void warrior_runfire (edict_t *self);

#define WARRIOR_CROUCH 8
#define WARRIOR_WALK 16

static int	sound_sight;
static int	sound_attack;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;
static int  sound_reload;
static int  sound_gib;

mframe_t warrior_frames_stand [] =
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
mmove_t warrior_move_stand = {FRAME_Stand_start, FRAME_Stand_end, warrior_frames_stand, NULL};

void warrior_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &warrior_move_stand;
}

mframe_t warrior_frames_crouch [] =
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
mmove_t warrior_move_crouch = {FRAME_Crouch_start, FRAME_Crouch_end, warrior_frames_crouch, NULL};

void warrior_crouch (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &warrior_move_crouch;
}

void warrior_reload (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_reload, 1, ATTN_NORM, 0);
   self->status |= STATUS_MONSTER_NOSHOOT;
   self->monsterinfo.currentmove = &warrior_move_crouch;
   self->monsterinfo.nextcover = 0;
   self->monsterinfo.covers = 0;
   self->monsterinfo.shots = 16;
   self->monsterinfo.nextambush = level.time + 1 + (random() * 3);
}

mframe_t warrior_frames_walk [] =
{
	BOT_walk, 0,  NULL,
	BOT_walk, 10,  NULL,
	BOT_walk, 10,  NULL,
	BOT_walk, 0,  NULL,
	BOT_walk, 10,  NULL,
	BOT_walk, 10,  NULL
};
mmove_t warrior_move_walk = {FRAME_Run_start, FRAME_Run_end, warrior_frames_walk, NULL};

void warrior_walk (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &warrior_move_walk;
}

mframe_t warrior_frames_run [] =
{
	BOT_run, 24,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 12, NULL,
	BOT_run, 24,  NULL,
	BOT_run, 16, NULL,
	BOT_run, 12, NULL
};
mmove_t warrior_move_run = {FRAME_Run_start, FRAME_Run_end, warrior_frames_run, NULL};

mframe_t warrior_frames_runattack [] =
{
	BOT_run, 24, NULL,
	BOT_run, 16, warrior_runfire,
	BOT_run, 12, warrior_runfire,
	BOT_run, 24, warrior_runfire,
	BOT_run, 16, warrior_runfire,
	BOT_run, 12, NULL
};
mmove_t warrior_move_runattack = {FRAME_Runattack_start, FRAME_Runattack_end, warrior_frames_runattack, NULL};

void warrior_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &warrior_move_stand;
	else
		self->monsterinfo.currentmove = &warrior_move_run;
}


mframe_t warrior_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t warrior_move_pain1 = {FRAME_Pain1_start, FRAME_Pain1_end, warrior_frames_pain1, warrior_run};

mframe_t warrior_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t warrior_move_pain2 = {FRAME_Pain2_start, FRAME_Pain2_end, warrior_frames_pain2, warrior_run};

void warrior_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void warrior_dead (edict_t *self)
{
	VectorSet (self->mins, -65, -26, -24);
	VectorSet (self->maxs, -4, 24, -15);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t warrior_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t warrior_move_death1 = {FRAME_Die1_start, FRAME_Die1_end, warrior_frames_death1, warrior_dead};

mframe_t warrior_frames_death2 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t warrior_move_death2 = {FRAME_Die2_start, FRAME_Die2_end, warrior_frames_death2, warrior_dead};

void warrior_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
      self->deadflag = DEAD_DEAD;
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);		
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_METALLIC);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	

	if (rand() & 1)
      {
      self->monsterinfo.currentmove = &warrior_move_death1;
		gi.sound (self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
      }
	else
      {
      self->monsterinfo.currentmove = &warrior_move_death2;
      gi.sound (self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
      }
}

void warrior_runfire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;    

   if (self->monsterinfo.shots-- < 0)
      return;

   if (!self->enemy || self->enemy->health <= 0)
      return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, -8, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   warrior_plasma (self, start, forward, 1, 800);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void warrior_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;    

   if (self->monsterinfo.shots-- < 0)
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

	VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, -8, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   warrior_plasma (self, start, forward, 1, 800);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void warrior_grenade (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
   float    range;
   trace_t  trace;

   if (!self->enemy || self->enemy->health <= 0)
      return;

   VectorCopy(self->enemy->s.origin, start);
   start[2] += 180;
	trace = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);	
   if (trace.fraction < 1.0)
      return;
   VectorSubtract (start, self->s.origin, forward);
   range = VectorLength(forward);
	VectorNormalize (forward);   
	VectorSet(offset, 20, -8, 10);
   vectoangles(forward, self->s.angles);
   AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
   fire_grenade (self, start, forward, 100, range, 3, 100);
   //warrior_plasma (self, start, forward, 1, 800);
   self->s.angles[2] = 0;
   self->s.angles[0] = 0;
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_GUNNER_GRENADE_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

mframe_t warrior_frames_attack [] =
{
	BOT_charge, 0, warrior_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t warrior_move_attack = {FRAME_Attack_start, FRAME_Attack_end, warrior_frames_attack, warrior_run};

mframe_t warrior_frames_attack2 [] =
{
	BOT_charge, 0, warrior_grenade,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL
};
mmove_t warrior_move_attack2 = {FRAME_Attack_start, FRAME_Attack_end, warrior_frames_attack2, warrior_run};


void warrior_attack(edict_t *self)
{
   self->monsterinfo.nextattack = level.time + 1.4;
   self->monsterinfo.pausetime = level.time + 1;
   if (random() < .2)
      self->monsterinfo.currentmove = &warrior_move_attack2;
   else if (random() < .7)
      self->monsterinfo.currentmove = &warrior_move_attack;
   else
      self->monsterinfo.currentmove = &warrior_move_runattack;
}


mframe_t warrior_frames_strafeleft [] =
{
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
	BOT_move_left, 10, NULL,
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
mmove_t warrior_move_strafeleft = {FRAME_Left_start, FRAME_Left_end, warrior_frames_strafeleft, warrior_run};

mframe_t warrior_frames_straferight [] =
{
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
	BOT_move_right, 10, NULL,
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
mmove_t warrior_move_straferight = {FRAME_Right_start, FRAME_Right_end, warrior_frames_straferight, warrior_run};

void warrior_dodge (edict_t *self, edict_t *attacker, float eta)
{
	vec3_t	offset, forward, right, end;
   trace_t  trace;
   float k = random();

   if (self->monsterinfo.currentmove == &warrior_move_straferight)
      return;
   if (self->monsterinfo.currentmove == &warrior_move_strafeleft)
      return;

	if (!self->enemy)
		self->enemy = attacker;

   VectorSet(offset, 0, 90, 0);
   AngleVectors (self->s.angles, forward, right, NULL);
   if (k < .3)
      {
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &warrior_move_straferight;
         return;
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
	      self->monsterinfo.currentmove = &warrior_move_strafeleft;
      return;         
      }
   if (k < .6 || self->monsterinfo.shots < 4)
      {
      offset[1] = -90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
         {
	      self->monsterinfo.currentmove = &warrior_move_strafeleft;
         return;
         }
      offset[1] = 90;
	   G_ProjectSource (self->s.origin, offset, forward, right, end);
      trace = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
      if (trace.fraction == 1)
   	   self->monsterinfo.currentmove = &warrior_move_straferight;
      return;
      }
   self->monsterinfo.currentmove = &warrior_move_runattack;
}

void warrior_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

   self->status |= STATUS_MONSTER_COMMUNICATE;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() & 1;

	if (n == 0)
      {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &warrior_move_pain1;
      }
	else
      {
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	   self->monsterinfo.currentmove = &warrior_move_pain2;
      }
}

void warrior_plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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

void warrior_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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
	bolt->s.effects |= EF_COLOR_SHELL;
	bolt->s.renderfx |= RF_SHELL_GREEN;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/nail/tris.md2");
	//bolt->s.sound = gi.soundindex ("weapons/plasmagun/fire.wav");
	bolt->owner = self;
	bolt->touch = warrior_plasma_touch;
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

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void SP_monster_warrior (edict_t *self)
{
	if (deathmatch->value)
	   {
		G_FreeEdict (self);
		return;
	   }

	sound_sight    = gi.soundindex ("warrior/sight.wav");
	sound_attack   = gi.soundindex ("warrior/attack.wav");
	sound_pain1    = gi.soundindex ("warrior/pain1.wav");
	sound_pain2    = gi.soundindex ("warrior/pain2.wav");
	sound_die1     = gi.soundindex ("warrior/die1.wav");
	sound_die2     = gi.soundindex ("warrior/die2.wav");
   sound_reload   = gi.soundindex ("weapons/assim/reload.wav");
   sound_gib		= gi.soundindex ("warrior/gib.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/warrior/tris.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/warrior/weapon.md2");
	VectorSet (self->mins, -16, -12, -24);
	VectorSet (self->maxs, 16, 16, 32);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 200;

	self->pain = warrior_pain;
	self->die = warrior_die;

   if (self->spawnflags & WARRIOR_CROUCH)
      self->monsterinfo.stand = warrior_crouch;         
   else if (self->spawnflags & WARRIOR_WALK)
      self->monsterinfo.stand = warrior_walk;
   else
      self->monsterinfo.stand = warrior_stand;
	self->monsterinfo.walk = warrior_walk;
	self->monsterinfo.run = warrior_run;
	self->monsterinfo.dodge = warrior_dodge;
	self->monsterinfo.attack = warrior_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.reload = warrior_reload;
	self->monsterinfo.sight = warrior_sight;
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
	
	self->monsterinfo.currentmove = &warrior_move_stand;
   walkmonster_start (self);
   self->monsterinfo.shots = 16;
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC | TYPE_MONSTER_AMBUSH;
}