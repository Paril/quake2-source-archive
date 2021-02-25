/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_actor.c
  Description: Actor AI

\**********************************************************/

#include "g_local.h"
#include "b_actor.h"

void find_node_hide (edict_t *self);
void actor_pain (edict_t *self, edict_t *other, float kick, int damage);
void actor_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void parse_command(edict_t *ent);
void script_touch(edict_t *ent, edict_t *other);
void actor_restore(edict_t *self);
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void script_block(edict_t *self, edict_t *other);
void script_use(edict_t *self, edict_t *other, edict_t *activator);

mframe_t actor_frames_stand [] =
{
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL
};
mmove_t actor_move_stand = {FRAME_stand01, FRAME_stand40, actor_frames_stand, NULL};

void actor_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_stand;
}

void actor_chill (edict_t *self)
{
   self->enemy = NULL;
   self->monsterinfo.save = self->monsterinfo.currentmove;
	self->monsterinfo.currentmove = &actor_move_stand;
}

mframe_t actor_frames_crouch [] =
{
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL
};
mmove_t actor_move_crouch = {FRAME_crstnd01, FRAME_crstnd19, actor_frames_crouch, NULL};

void actor_crouch (edict_t *self)
{
   self->enemy = NULL;
	self->monsterinfo.currentmove = &actor_move_crouch;
}

mframe_t actor_frames_walk [] =
{
	scrBOT_run, 0,  NULL,
	scrBOT_run, 16,  NULL,
	scrBOT_run, 16,  NULL,
	scrBOT_run, 0,  NULL,
	scrBOT_run, 16,  NULL,
	scrBOT_run, 16,  NULL
};
mmove_t actor_move_walk = {FRAME_run1, FRAME_run6, actor_frames_walk, NULL};

void actor_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_walk;
}

mframe_t actor_frames_runattack [] =
{
	BOT_run, 0,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 0,  NULL,
	BOT_run, 24,  NULL,
	BOT_run, 24,  NULL
};
mmove_t actor_move_runattack = {FRAME_run1, FRAME_run6, actor_frames_runattack, NULL};

void actor_alert (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_runattack;
}

mframe_t actor_frames_run [] =
{
	scrBOT_run, 0,  NULL,
	scrBOT_run, 32,  NULL,
	scrBOT_run, 32,  NULL,
	scrBOT_run, 0,  NULL,
	scrBOT_run, 32,  NULL,
	scrBOT_run, 32,  NULL
};
mmove_t actor_move_run = {FRAME_run1, FRAME_run6, actor_frames_run, NULL};

void actor_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &actor_move_stand;
	else
		self->monsterinfo.currentmove = &actor_move_run;
}


mframe_t actor_frames_pain1 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t actor_move_pain1 = {FRAME_pain101, FRAME_pain104, actor_frames_pain1, actor_run};

mframe_t actor_frames_pain2 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t actor_move_pain2 = {FRAME_pain201, FRAME_pain204, actor_frames_pain2, actor_run};

mframe_t actor_frames_pain3 [] =
{
	BOT_move, -3, NULL,
	BOT_move, -2, NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL
};
mmove_t actor_move_pain3 = {FRAME_pain301, FRAME_pain304, actor_frames_pain3, actor_run};

void actor_dead (edict_t *self)
{
   self->s.modelindex2 = 0;
	VectorSet (self->mins, -65, -26, -24);
	VectorSet (self->maxs, -4, 24, -15);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t actor_frames_death1 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t actor_move_death1 = {FRAME_death101, FRAME_death106, actor_frames_death1, actor_dead};

mframe_t actor_frames_death2 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t actor_move_death2 = {FRAME_death201, FRAME_death206, actor_frames_death2, actor_dead};

mframe_t actor_frames_death3 [] =
{
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, -1, NULL,
	BOT_move, -1, NULL,
	BOT_move, 0, NULL,
	BOT_move, -4, NULL,
	BOT_move, 0,  NULL
};
mmove_t actor_move_death3 = {FRAME_death301, FRAME_death308, actor_frames_death3, actor_dead};

void actor_restore(edict_t *self)
{
   self->monsterinfo.currentmove = self->monsterinfo.save;
}

mframe_t actor_frames_flip [] =
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
	BOT_move, 0, NULL
};
mmove_t actor_move_flip = {FRAME_flip01, FRAME_flip12, actor_frames_flip, actor_restore};

void actor_flip(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &actor_move_flip;
}

mframe_t actor_frames_salute [] =
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
	BOT_move, 0, NULL
};
mmove_t actor_move_salute = {FRAME_salute01, FRAME_salute11, actor_frames_salute, actor_restore};

void actor_salute(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &actor_move_salute;
}

mframe_t actor_frames_taunt [] =
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
	BOT_move, 0, NULL
};
mmove_t actor_move_taunt = {FRAME_taunt01, FRAME_taunt17, actor_frames_taunt, actor_restore};

void actor_taunt(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &actor_move_taunt;
}

mframe_t actor_frames_wave [] =
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
	BOT_move, 0, NULL
};
mmove_t actor_move_wave = {FRAME_wave01, FRAME_wave11, actor_frames_wave, actor_restore};

void actor_wave(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &actor_move_wave;
}

mframe_t actor_frames_point [] =
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
	BOT_move, 0, NULL
};
mmove_t actor_move_point = {FRAME_point01, FRAME_point12, actor_frames_point, actor_restore};
void actor_point(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &actor_move_point;   
}


void actor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   int	n;
// check for gib
   self->scr.commands = NULL;
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
	self->monsterinfo.currentmove = &actor_move_death1;
}

void actor_fire (edict_t *self)
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
   fire_blaster (self, start, forward, self->dmg, 1600, EF_BLASTER, 1);	

   self->s.angles[1] += 15;
   self->s.angles[0] = 0;

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_SOLDIER_BLASTER_1);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

mframe_t actor_frames_attack1 [] =
{
	BOT_charge, -1, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 0, actor_fire,
	BOT_charge, 0, NULL,
	BOT_charge, 0, NULL,
	BOT_charge, 2, NULL,
	BOT_charge, 0,  NULL
};
mmove_t actor_move_attack1 = {FRAME_attack1, FRAME_attack8, actor_frames_attack1, actor_run};

void actor_attack(edict_t *self)
{
   self->monsterinfo.nextattack = level.time + .3;
   self->monsterinfo.currentmove = &actor_move_attack1;
}

/*QUAKED monster_actor (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_actor (edict_t *self)
{
	if (deathmatch->value || !self->scriptfile)
	{
		G_FreeEdict (self);
		return;
	}
  
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/actor/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

   if (!self->health)
	   self->health = 100;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 200;

	self->pain = actor_pain;
	self->die = actor_die;

   //actor_stand;
   self->monsterinfo.stand = actor_stand;
	self->monsterinfo.walk = actor_walk;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.attack = actor_attack;
   self->monsterinfo.action[0] = actor_flip;
   self->monsterinfo.action[1] = actor_salute;
   self->monsterinfo.action[2] = actor_taunt;
   self->monsterinfo.action[3] = actor_wave;
   self->monsterinfo.action[4] = actor_point;
	self->monsterinfo.idle = NULL;
   self->status = 0;
   self->type = TYPE_SLAVE_ACTOR;
   self->touch = actor_touch;
   self->blocked = script_block;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   if (self->spawnflags)
      self->s.modelindex2 = gi.modelindex("players/slave/w_hammer.md2");
   else
      self->s.modelindex2 = gi.modelindex("players/slave/w_plasmagun.md2");
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &actor_move_stand;
   walkmonster_start (self);
   self->use = script_use;
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC;
   load_script(self);
}

void actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;


	if (level.time < self->pain_debounce_time)
		return;

   self->monsterinfo.chaser = other;
   self->status |= STATUS_MONSTER_HIDE;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() & 1;

	if (n == 0)
      self->monsterinfo.currentmove = &actor_move_pain1;
	else
	   self->monsterinfo.currentmove = &actor_move_pain2;
}


void actor_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

   if (ent->touch_debounce_time > level.time)
      return;
   ent->touch_debounce_time = level.time + .5;
   script_touch (ent, other);
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
