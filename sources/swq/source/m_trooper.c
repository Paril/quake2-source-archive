/*
==============================================================================

trooper

==============================================================================
*/

#include "g_local.h"
#include "m_trooper.h"


static int	sound_sight1;
static int	sound_sight2;
static int	sound_sight3;
static int	sound_fire1;

// STAND

//void AI_GrenadeLauncher (edict_t *self);
void trooper_stand (edict_t *self);

mframe_t trooper_frames_stand1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t trooper_move_stand1 = {FRAME_stand01, FRAME_stand27, trooper_frames_stand1, trooper_stand};

void trooper_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &trooper_move_stand1;
/*	if(!self->monsterinfo.team_leader || self->monsterinfo.team_leader != self)
		return;
	if(self->monsterinfo.attack_mode <= 4)
	{
		self->monsterinfo.attack_mode++;
		gi.dprintf("%i\n", self->monsterinfo.attack_mode);
	}
	else if(self->monsterinfo.attack_mode == 5)
	{
		self->monsterinfo.attack_mode = 12;
		AI_Team_Formation_V(self);
	}*/
}


//
// WALK
//

void trooper_footstep (edict_t *ent)
{
	ent->s.event = EV_FOOTSTEP;
}

mframe_t trooper_frames_start_walk [] =
{
	ai_walk, 4,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 2.5,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 1,  NULL
};
mmove_t trooper_move_start_walk = {FRAME_walk01, FRAME_walk05, trooper_frames_start_walk, NULL};

mframe_t trooper_frames_stop_walk [] =
{
	ai_walk, 4,  trooper_footstep,
	ai_walk, 4,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 6,  trooper_footstep,
	ai_walk, 3,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4, NULL,

	ai_walk, 0,  NULL
};
mmove_t trooper_move_stop_walk = {FRAME_walk18, FRAME_walk28, trooper_frames_stop_walk, NULL};

mframe_t trooper_frames_walk [] =
{
	ai_walk, 4,  trooper_footstep,
	ai_walk, 5,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 3,  trooper_footstep,
	ai_walk, 5,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 6, NULL,

	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL
};
mmove_t trooper_move_walk = {FRAME_walk06, FRAME_walk17, trooper_frames_walk, NULL};

void trooper_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &trooper_move_walk;
}


//
// RUN
//

void trooper_run (edict_t *self);

mframe_t trooper_frames_run1 [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t trooper_move_run1 = {FRAME_run05, FRAME_run12, trooper_frames_run1, NULL};

void trooper_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &trooper_move_stand1;
	else
	{
		self->monsterinfo.currentmove = &trooper_move_run1;
	}
}


//
// PAIN
//

void trooper_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

	if (skill->value == 3)
		return;		// no pain anims in nightmare
}

//
// ATTACK
//
void trooper_fire (edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int damage;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

	if (skill->value > 0)
	{
		VectorCopy (self->enemy->s.origin, end);
		predictTargPos (self, start, end, self->enemy->velocity, BLASTER_SPEED, false);
	}
	else
		VectorCopy (self->enemy->s.origin, end);

	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	if (skill->value < 3 && (random() > 0.5))
	{
		r = crandom () * (800/(skill->value + 1));
		u = crandom () * (300/(skill->value + 1));
		damage = 10+(random()*5);
	}
	else
	{
		r = 0;
		u = 0;
		damage = 30+(random()*10);
	}

	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	monster_fire_rifle (self, start, aim, damage, BLASTER_SPEED, flash_index, EF_BLASTER);
	gi.sound (self, CHAN_WEAPON, sound_fire1, 1, ATTN_NORM, 0);
}

//
// ATTACK
//

void trooper_fire2 (edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t 	target;
	vec3_t 	vel;
	vec3_t	tv;
	float 	dist;
	float	r, u;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	AngleVectors (self->s.angles, forward, right, NULL);

	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*1000;
	u = crandom()*500;
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	VectorCopy(self->enemy->s.origin, tv);
	tv[2] += self->enemy->viewheight;

	if(self->deadflag == DEAD_NO)
	{
		VectorSubtract(self->s.origin, self->enemy->s.origin, vel);

		dist = VectorLength(vel);

		VectorCopy(self->enemy->velocity, vel);
		if (vel[2] > 0)
			vel[2] = 0;

		VectorMA (self->enemy->s.origin, dist / 1000, vel, target);
		target[2] += self->enemy->viewheight - 8;

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	monster_fire_rifle (self, start, forward, 10, BLASTER_SPEED, flash_index, EF_BLASTER);
	gi.sound (self, CHAN_WEAPON, sound_fire1, 1, ATTN_NORM, 0);
}

// ATTACK1

void trooper_fire1 (edict_t *self)
{
	trooper_fire (self, 0);
}

void trooper_attack1_refire (edict_t *self)
{
	trace_t tr;

	if (self->enemy->health <= 0)
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->enemy->s.origin, self, MASK_MONSTERSOLID);

	if(tr.ent == self->enemy && random() > 0.5)
		self->monsterinfo.nextframe = FRAME_fire03;
	else if(tr.ent == self->enemy && skill->value >= 1)
		self->monsterinfo.nextframe = FRAME_fire01;
}

mframe_t trooper_frames_attack1 [] =
{
	ai_hold, 0,  trooper_fire1,
	ai_hold, 0,  NULL,
	ai_hold, 0,  trooper_attack1_refire,
	ai_hold, 0,  NULL,
};
mmove_t trooper_move_attack1 = {FRAME_fire01, FRAME_fire04, trooper_frames_attack1, trooper_run};

void trooper_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &trooper_move_attack1;
}

//
//  DUCK
//
void trooper_duck_up (edict_t *self);
void trooper_duck_down (edict_t *self);

void trooper_duck_refire (edict_t *self)
{
	trace_t tr;

	if (self->enemy->health <= 0)
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->enemy->s.origin, self, MASK_MONSTERSOLID);

	if(tr.ent == self->enemy && random() > 0.5)
		self->monsterinfo.nextframe = FRAME_duck3;
	else if(tr.ent == self->enemy && skill->value >= 1)
		self->monsterinfo.nextframe = FRAME_duck3;
}

mframe_t trooper_frames_duck1 [] =
{
	ai_hold, 0,  trooper_duck_down,
	ai_hold, 0,  trooper_fire1,
	ai_hold, 0,  NULL,
	ai_hold, 0,  trooper_duck_refire,
	ai_hold, 0,  NULL,
	ai_hold, 0,  trooper_duck_up,
};
mmove_t trooper_move_duck1 = {FRAME_duckA, FRAME_duckB, trooper_frames_duck1, trooper_run};

void trooper_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1000;
	gi.linkentity (self);
}

void trooper_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

void trooper_duck_hold (edict_t *self)
{
	if(random() > 0.9)
		trooper_fire (self, 0);
	if (self->monsterinfo.attack_mode == AM_DUCKED)
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
}

void trooper_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if(skill->value < 2)
		return;

	if(self->monsterinfo.attack_mode == AM_DUCKED)
		self->monsterinfo.currentmove = &trooper_move_duck1;
	else if (random() > 0.2 && skill->value == 2)
	{
		self->monsterinfo.currentmove = &trooper_move_duck1;
	}
	else
	{
		self->monsterinfo.currentmove = &trooper_move_duck1;
	}
}

//
// SIGHT
//

void trooper_sight(edict_t *self, edict_t *other)
{
	int n;

	AI_AlertStroggs (self);

	n = rand() % 3;
	if(n == 0)
		gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else if(n == 1)
		gi.sound (self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);
	else if(n == 3)
		gi.sound (self, CHAN_VOICE, sound_sight3, 1, ATTN_NORM, 0);
}

//
// DEATH
//

void trooper_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t trooper_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t trooper_move_death1 = {FRAME_death01, FRAME_death33, trooper_frames_death1, trooper_dead};

void trooper_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gitem_t	*item;

/*// check for gib
	if (self->health <= self->gib_health)
	{
		VectorSet (self->mins, -4, -4, -4);
		VectorSet (self->maxs, 4, 4, -4);
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		if(self->s.modelindex3)
			ThrowGib (self, "models/monsters/trooper/tris3.md2", damage, GIB_TROOPERGIB);
		if(self->s.modelindex4)
			ThrowGib (self, "models/monsters/trooper/tris4.md2", damage, GIB_TROOPERGIB);
		ThrowGib (self, "models/monsters/trooper/tris6.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/monsters/trooper/tris6.md2", damage, GIB_ORGANIC);
		if(self->s.modelindex2)
		{
			ThrowGib (self, "models/monsters/trooper/tris1.md2", damage, GIB_TROOPERGIB);
			ThrowHead (self, "models/monsters/trooper/tris5.md2", damage, GIB_ORGANIC);
		}
		else
			ThrowHead (self, "models/monsters/trooper/tris5.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		self->takedamage = DAMAGE_NO;

		self->s.modelindex2 = 0;
		self->s.modelindex3 = 0;

		return;
	}*/

	if (self->deadflag == DEAD_DEAD)
		return;

	item = FindItem("Trooper_Rifle");
	if(item)
	{
		Drop_Item (self, item);
	}

//	self->monsterinfo.team_leader = NULL;
//	self->monsterinfo.team_wing1 = NULL;
//	self->monsterinfo.team_wing2 = NULL;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->svflags |= SVF_DEADMONSTER;

//	if (attacker->client)
//		self->monsterinfo.killer = attacker;
//	else
//		self->monsterinfo.killer = attacker->enemy;

	self->s.frame = 0;
	self->s.modelindex = gi.modelindex("models/monsters/trooper/tris2.md2");
	self->s.modelindex2 = gi.modelindex("models/monsters/trooper/tris1.md2");
	self->s.modelindex3 = gi.modelindex("models/monsters/trooper/tris3.md2");
	self->s.modelindex4 = gi.modelindex("models/monsters/trooper/tris4.md2");

	if(random() > 0.6)
	{
		self->monsterinfo.currentmove = &trooper_move_death1;
		return;
	}

	if (fabs((self->s.origin[2] + self->viewheight) - point[2]) <= 4)
	{
		// head shot
		self->s.modelindex2 = 0; //losing his head
		ThrowGib (self, "models/monsters/trooper/gib1.md2", damage, GIB_TROOPERGIB);
	}
	else
	{
		if(random() > 0.5)
		{
			self->s.modelindex3 = 0; //losing his right arm
			ThrowGib (self, "models/monsters/trooper/gib3.md2", damage, GIB_TROOPERGIB);
		}
		else
		{
			self->s.modelindex4 = 0; //losing his left arm
			ThrowGib (self, "models/monsters/trooper/gib4.md2", damage, GIB_TROOPERGIB);
		}
	}

	self->monsterinfo.currentmove = &trooper_move_death1;
}


//
// SPAWN
//

void spawn_head (edict_t *self)
{
	edict_t *head;

	head = G_Spawn();
	VectorCopy (self->s.origin, head->s.origin);
	VectorCopy (self->movedir, head->movedir);
	vectoangles (self->s.angles, head->s.angles);
	head->movetype = MOVETYPE_NOCLIP;
	head->solid = SOLID_NOT;
	VectorClear (head->mins);
	VectorClear (head->maxs);
	head->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	head->owner = self;
	head->classname = "testhead";
}

void SP_monster_trooper (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_sight1 = gi.soundindex ("trooper/sight1.wav");
	sound_sight2 = gi.soundindex ("trooper/sight2.wav");
	sound_sight3 = gi.soundindex ("trooper/sight3.wav");
	sound_fire1 = gi.soundindex ("trooper/fire1.wav");

	self->s.skinnum = 0;
	if(skill->value < 3)
	{
		self->health = 40+(random() * 20);
		self->max_health = self->health;
	}
	else 
	{
		self->health = 70+(random() * 30); //the 30 is to be like trooper armor
		self->max_health = self->health;
	}
	self->gib_health = -self->health;

	self->s.modelindex = gi.modelindex ("models/monsters/trooper/craptris.md2");
//	self->s.modelindex2 = gi.modelindex ("models/monsters/trooper/weapon1.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 45;

	self->mass = 120;

	self->pain = trooper_pain;
	self->die = trooper_die;

	self->monsterinfo.stand = trooper_stand;
	self->monsterinfo.walk = trooper_walk;
	self->monsterinfo.run = trooper_run;
	self->monsterinfo.dodge = trooper_dodge;
	self->monsterinfo.attack = trooper_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = trooper_sight;

	self->path_node = NO_NODES;
	self->moveflags |= MOVE_NEVER_JUMP;// | MOVE_GAP_CHECKING;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

