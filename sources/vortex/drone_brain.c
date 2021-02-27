/*
==============================================================================

brain

==============================================================================
*/

#include "g_local.h"
#include "m_brain.h"


static int	sound_chest_open;
static int	sound_tentacles_extend;
static int	sound_tentacles_retract;
static int	sound_death;
static int	sound_idle1;
static int	sound_idle2;
static int	sound_idle3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_sight;
static int	sound_search;
static int	sound_melee1;
static int	sound_melee2;
static int	sound_melee3;
static int	sound_thud;

void mybrain_attack (edict_t *self);
void mybrain_attack3 (edict_t *self);
void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

void mybrain_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	mybrain_attack(self);
}

void mybrain_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


void mybrain_run (edict_t *self);
void mybrain_dead (edict_t *self);


//
// STAND
//

mframe_t mybrain_frames_stand [] =
{
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,

	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,

	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL
};
mmove_t mybrain_move_stand = {FRAME_stand01, FRAME_stand30, mybrain_frames_stand, NULL};

void mybrain_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &mybrain_move_stand;
}


//
// IDLE
//

mframe_t mybrain_frames_idle [] =
{
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,

	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,

	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL,
	drone_ai_stand,	0,	NULL
};
mmove_t mybrain_move_idle = {FRAME_stand31, FRAME_stand60, mybrain_frames_idle, mybrain_stand};

void mybrain_idle (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_idle3, 1, ATTN_IDLE, 0);
	self->monsterinfo.currentmove = &mybrain_move_idle;
}

mframe_t brain_frames_walk1 [] =
{
	drone_ai_walk,	7,	NULL,
	drone_ai_walk,	2,	NULL,
	drone_ai_walk,	3,	NULL,
	drone_ai_walk,	3,	NULL,
	drone_ai_walk,	1,	NULL,
	drone_ai_walk,	0,	NULL,
	drone_ai_walk,	0,	NULL,
	drone_ai_walk,	9,	NULL,
	drone_ai_walk,	-4,	NULL,
	drone_ai_walk,	-1,	NULL,
	drone_ai_walk,	2,	NULL
};
mmove_t brain_move_walk1 = {FRAME_walk101, FRAME_walk111, brain_frames_walk1, NULL};

void mybrain_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &brain_move_walk1;
}

mframe_t mybrain_frames_defense [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_defense = {FRAME_defens01, FRAME_defens08, mybrain_frames_defense, NULL};

//
// DUCK
//

void mybrain_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	if (!self->groundentity)
		return;

	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] = 0;
	self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void mybrain_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	VectorClear(self->velocity);
	gi.linkentity (self);
}

void mybrain_jump_takeoff (edict_t *self)
{
	vec3_t	v;

	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	VectorSubtract(self->monsterinfo.dir, self->s.origin, v);
	v[2] = 0;
	VectorNormalize(v);
	VectorScale(v, -200, self->velocity);
	self->velocity[2] = 400;
	self->monsterinfo.pausetime = level.time + 5.0; // maximum duration of jump

//	gi.dprintf("mybrain_jump_takeoff()\n");
}

void mybrain_jump_hold (edict_t *self)
{
	vec3_t	v;

	if (G_EntIsAlive(self->monsterinfo.attacker))
	{
		// face the attacker
		VectorSubtract(self->monsterinfo.attacker->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);
		M_ChangeYaw(self);
	}

	// check for landing or jump timeout
	if (self->waterlevel || self->groundentity || (level.time > self->monsterinfo.pausetime))
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t mybrain_frames_duck [] =
{
	ai_move,	0,	mybrain_duck_down,
	ai_move,	0,	NULL,//mybrain_duck_down,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	mybrain_duck_up,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_duck = {FRAME_duck01, FRAME_duck08, mybrain_frames_duck, mybrain_run};

mframe_t mybrain_frames_jump [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	mybrain_jump_takeoff,
	ai_move,	0,	mybrain_jump_hold,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_jump = {FRAME_duck01, FRAME_duck08, mybrain_frames_jump, mybrain_run};

void mybrain_jumpattack_takeoff (edict_t *self)
{
	int	speed = 800;
	vec3_t	forward, start;

	if (!G_EntExists(self->enemy))
		return;

	//4.5 monster bonus flags
	if (self->monsterinfo.bonus_flags & BF_FANATICAL)
		speed *= 2;

	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);

	// launch towards our enemy
	MonsterAim(self, -1, speed, false, 0, forward, start);
	VectorScale (forward, speed, self->velocity);
	self->velocity[2] = 300;

	// jump timeout
	self->monsterinfo.pausetime = level.time + 5.0;

	// shrink our bbox vertically because we're ducking
	self->maxs[2] = 0;
	self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void mybrain_jumpattack_landing (edict_t *self)
{
	// expand the bbox again, we're standing up
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);

	// attack right away if enemy is valid, visible, infront, and is moving
	if (G_EntIsAlive(self->enemy) && visible(self, self->enemy) 
		&& infront(self, self->enemy) && self->enemy->movetype)
		mybrain_attack3(self);
}

void mybrain_jumpattack_hold (edict_t *self)
{
	int value=0;

	// we are on ground
	if (self->groundentity)
		value = 1;
	// we are in the water or passed jump timeout value
	else if (self->waterlevel || level.time > self->monsterinfo.pausetime)
		value = 2;

	// discontinue holding this frame
	if (value)
	{
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;

		if (value == 1)
			gi.sound (self, CHAN_WEAPON, sound_thud, 1, ATTN_NORM, 0);
	}
	
	else
	{
		vec3_t v;

		// hold this frame
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

		// face the enemy if he's alive and visible
		if (G_EntIsAlive(self->enemy) && visible(self, self->enemy))
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			M_ChangeYaw(self);
		}
	}
}


mframe_t mybrain_frames_jumpattack [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	mybrain_jumpattack_takeoff,
	ai_move,	0,	mybrain_jumpattack_hold,
	ai_move,	0,	NULL,
	ai_move,	0,	mybrain_jumpattack_landing,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_jumpattack = {FRAME_duck01, FRAME_duck08, mybrain_frames_jumpattack, mybrain_run};

void mybrain_jump (edict_t *self)
{
	if (self->groundentity)
		self->monsterinfo.currentmove = &mybrain_move_jump;
}

void mybrain_dodge (edict_t *self, edict_t *attacker, vec3_t dir, int radius)
{
	if (random() > 0.9)
		return;
	if (level.time < self->monsterinfo.dodge_time)
		return;
	if (OnSameTeam(self, attacker))
		return;

	if (!self->enemy && G_EntIsAlive(attacker))
		self->enemy = attacker;
	if (!radius)
	{
		self->monsterinfo.currentmove = &mybrain_move_duck;
		self->monsterinfo.dodge_time = level.time + 2.0;
	}
	else
	{
		mybrain_jump(self);
		self->monsterinfo.dodge_time = level.time + 3.0;
	}
}

mframe_t mybrain_frames_death2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	9,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_death2 = {FRAME_death201, FRAME_death205, mybrain_frames_death2, mybrain_dead};

mframe_t mybrain_frames_death1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	9,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t mybrain_move_death1 = {FRAME_death101, FRAME_death118, mybrain_frames_death1, mybrain_dead};


//
// MELEE
//

void mybrain_swing_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_melee1, 1, ATTN_NORM, 0);
}

void mybrain_hit_right (edict_t *self)
{
	int		damage;

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;

	if (!self->activator->client)
		damage = 120 + 80*self->monsterinfo.level;
	else 
		damage = 120 + 55*self->monsterinfo.level;
	
	if (M_MeleeAttack(self, MELEE_DISTANCE, damage, 0))
		gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

void mybrain_swing_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_melee2, 1, ATTN_NORM, 0);
}

void mybrain_hit_left (edict_t *self)
{
	int		damage;

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;

	if (!self->activator->client)
		damage = 120 + 80*self->monsterinfo.level;
	else 
		damage = 120 + 80*self->monsterinfo.level;

	if (M_MeleeAttack(self, MELEE_DISTANCE, damage, 0))
		gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

mframe_t mybrain_frames_attack1 [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_swing_right,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_hit_right,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_swing_left,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_hit_left,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,NULL
};
mmove_t mybrain_move_attack1 = {FRAME_attak101, FRAME_attak118, mybrain_frames_attack1, mybrain_run};

void mybrain_chest_open (edict_t *self)
{
	//self->spawnflags &= ~65536;
	//self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	gi.sound (self, CHAN_BODY, sound_chest_open, 1, ATTN_NORM, 0);
}

void mybrain_tentacle_attack (edict_t *self)
{
	int		damage;

	if (!self->activator->client)
		damage = 100 + 45*self->monsterinfo.level;
	else 
		damage = 100 + 35*self->monsterinfo.level;

	M_MeleeAttack(self, MELEE_DISTANCE, damage, 0);

	gi.sound (self, CHAN_WEAPON, sound_tentacles_retract, 1, ATTN_NORM, 0);
}

void mybrain_chest_closed (edict_t *self)
{
	//self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->spawnflags & 65536)
	{
		self->spawnflags &= ~65536;
		self->monsterinfo.currentmove = &mybrain_move_attack1;
	}
}

mframe_t mybrain_frames_attack2 [] =
{
	ai_charge,	0,	mybrain_chest_open,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_tentacle_attack,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_chest_closed,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t mybrain_move_attack2 = {FRAME_attak205, FRAME_attak217, mybrain_frames_attack2, mybrain_run};

#define BRAIN_INITIAL_PULL			-65 // from 60.
#define BRAIN_ADDON_PULL			0 // from -2, pull shouldn't scale.
#define BRAIN_INITIAL_TENTACLE_DMG	30
#define BRAIN_ADDON_TENTACLE_DMG	3 // from 6, give some life for the player.

void mybrain_suxor (edict_t *self)
{
	int		damage, range, pull;
	vec3_t	start, v, end;
	trace_t	tr;

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;

	self->lastsound = level.framenum;

	G_EntMidPoint(self->enemy, start);
	VectorSubtract(start, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);
	range = VectorLength(v);
	VectorNormalize(v);
	VectorMA(self->s.origin, 512, v, end);
	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);
	if (G_EntExists(tr.ent) && (tr.ent == self->enemy))
	{
		damage = BRAIN_INITIAL_TENTACLE_DMG + BRAIN_ADDON_TENTACLE_DMG*self->monsterinfo.level;
		pull = BRAIN_INITIAL_PULL + BRAIN_ADDON_PULL*self->monsterinfo.level;

		if (tr.ent->groundentity)
			pull *= 2;

		if (range > 64)
			T_Damage(tr.ent, self, self, v, tr.endpos, tr.plane.normal, 0, pull, 0, MOD_UNKNOWN);
		else
			T_Damage(tr.ent, self, self, v, tr.endpos, tr.plane.normal, damage, pull, 0, MOD_UNKNOWN);
	}
}

void mybrain_continue (edict_t *self)
{
	float	dist, chance;
	vec3_t	start, forward;

	if (!G_ValidTarget(self, self->enemy, true))
		return;

	// higher chance to continue attack if our enemy is close
	dist = entdist(self, self->enemy);
	chance = 1-dist/1024.0;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, self->maxs[1]+8, forward , start);

	if (G_IsClearPath(self->enemy, MASK_SHOT, start, self->enemy->s.origin) 
		&& (random() <= chance) && (dist <= 512))
		self->monsterinfo.nextframe = FRAME_attak206;
	mybrain_suxor(self);
}

void mybrain_delay (edict_t *self)
{
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		self->monsterinfo.attack_finished = level.time + random();
}

mframe_t mybrain_frames_attack3 [] =
{
	ai_charge,	0,	mybrain_chest_open,
	ai_charge,	0,	mybrain_suxor,
	ai_charge,	0,	mybrain_suxor,
	ai_charge,	0,	mybrain_suxor,
	ai_charge,	0,	mybrain_suxor,
	ai_charge,	0,	mybrain_continue,
	ai_charge,	0,	mybrain_chest_closed,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	mybrain_delay
};
mmove_t mybrain_move_attack3 = {FRAME_attak205, FRAME_attak217, mybrain_frames_attack3, mybrain_run};

void mybrain_attack3 (edict_t *self)
{
	self->monsterinfo.currentmove = &mybrain_move_attack3;
}

void mybrain_melee (edict_t *self)
{
	if (entdist(self, self->enemy) < MELEE_DISTANCE)
	{
		if (random() <= 0.5)
			self->monsterinfo.currentmove = &mybrain_move_attack1;
		else
			self->monsterinfo.currentmove = &mybrain_move_attack2;
	}
}

void mybrain_attack (edict_t *self)
{
	float	dist;

	dist = entdist(self, self->enemy);

	// jump to our enemy if he's close and on even ground
	if ((dist > 256) && (self->enemy->absmin[2]+18 >= self->absmin[2]) 
		&& (self->enemy->absmin[2]-18 <= self->absmin[2]) 
		&& !(self->monsterinfo.aiflags & AI_STAND_GROUND))
		self->monsterinfo.currentmove = &mybrain_move_jumpattack;
	// use sucking attack if enemy is within 256 units and can be moved
	else if ((dist <= 256) && (self->enemy->movetype != MOVETYPE_NONE) 
		&& (self->enemy->movetype != MOVETYPE_TOSS))
		self->monsterinfo.currentmove = &mybrain_move_attack3;
	// use melee attack if we're close
	else if (dist <= MELEE_DISTANCE)
		mybrain_melee(self);
}

//
// RUN
//

mframe_t mybrain_frames_run [] =
{
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL,
	drone_ai_run,	15,	NULL
};
mmove_t mybrain_move_run = {FRAME_walk101, FRAME_walk111, mybrain_frames_run, NULL};

void mybrain_run (edict_t *self)
{
	//self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &mybrain_move_stand;
	else
		self->monsterinfo.currentmove = &mybrain_move_run;
}

void mybrain_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void mybrain_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;

	if (other && other->client && self->activator && self->activator == other)
	{
		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		self->velocity[0] += forward[0] * 50;
		self->velocity[1] += forward[1] * 50;
		self->velocity[2] += forward[2] * 50;
	}
}

void mybrain_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

	// reduce lag by removing the entity right away
#ifdef OLD_NOLAG_STYLE
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}
#endif

	self->s.effects = 0;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	self->monsterinfo.power_armor_power = 0;

	//GHz: Check for gibbed body
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
#ifdef OLD_NOLAG_STYLE
		M_Remove(self, false, false);
#else
		if (nolag->value)
			M_Remove(self, false, true);
		else
			M_Remove(self, false, false);
#endif
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	//GHz: Begin death sequence
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	if (random() <= 0.5)
		self->monsterinfo.currentmove = &mybrain_move_death1;
	else
		self->monsterinfo.currentmove = &mybrain_move_death2;

	DroneList_Remove(self);

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

/*QUAKED monster_brain (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void init_drone_brain (edict_t *self)
{
//	if (deathmatch->value)
//	{
//		G_FreeEdict (self);
//		return;
//	}

	sound_chest_open = gi.soundindex ("brain/brnatck1.wav");
	sound_tentacles_extend = gi.soundindex ("brain/brnatck2.wav");
	sound_tentacles_retract = gi.soundindex ("brain/brnatck3.wav");
	sound_death = gi.soundindex ("brain/brndeth1.wav");
	sound_idle1 = gi.soundindex ("brain/brnidle1.wav");
	sound_idle2 = gi.soundindex ("brain/brnidle2.wav");
	sound_idle3 = gi.soundindex ("brain/brnlens1.wav");
	sound_pain1 = gi.soundindex ("brain/brnpain1.wav");
	sound_pain2 = gi.soundindex ("brain/brnpain2.wav");
	sound_sight = gi.soundindex ("brain/brnsght1.wav");
	sound_search = gi.soundindex ("brain/brnsrch1.wav");
	sound_melee1 = gi.soundindex ("brain/melee1.wav");
	sound_melee2 = gi.soundindex ("brain/melee2.wav");
	sound_melee3 = gi.soundindex ("brain/melee3.wav");
	sound_thud = gi.soundindex ("player/land1.wav");

	self->monsterinfo.control_cost = M_BRAIN_CONTROL_COST;
	self->monsterinfo.cost = M_BRAIN_COST;
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/brain/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	//if (self->activator && self->activator->client)
	self->health = 35 + 45*self->monsterinfo.level;
	//else self->health = 100 + 40*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 400;

	self->item = FindItemByClassname("ammo_cells");

//	self->pain = mybrain_pain;
	self->die = mybrain_die;
//	self->touch = mybrain_touch;

	self->monsterinfo.stand = mybrain_stand;
	self->monsterinfo.walk = mybrain_walk;
	self->monsterinfo.run = mybrain_run;
	self->monsterinfo.dodge = mybrain_dodge;
	self->monsterinfo.attack = mybrain_attack;
	self->monsterinfo.melee = mybrain_melee;
	self->monsterinfo.sight = mybrain_sight;
//	self->monsterinfo.search = mybrain_search;
	//self->monsterinfo.idle = mybrain_idle;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;

	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;

	//if (self->activator && self->activator->client)
		self->monsterinfo.power_armor_power = 150 + 150*self->monsterinfo.level;
	//else self->monsterinfo.power_armor_power = 300 + 120*self->monsterinfo.level;

	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;
	self->mtype = M_BRAIN;

	gi.linkentity (self);

	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	//self->monsterinfo.melee = 1;
	self->monsterinfo.currentmove = &mybrain_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start (self);
	self->nextthink = level.time + FRAMETIME;
}
