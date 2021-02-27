/*
==============================================================================

MEDIC

==============================================================================
*/

#include "g_local.h"
#include "m_medic.h"

static int	sound_idle1;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_sight;
static int	sound_search;
static int	sound_hook_launch;
static int	sound_hook_hit;
static int	sound_hook_heal;
static int	sound_hook_retract;

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void mymedic_refire (edict_t *self);
void mymedic_heal (edict_t *self);
void drone_ai_walk (edict_t *self, float dist);

void mymedic_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);

}

mframe_t mymedic_frames_stand [] =
{
	drone_ai_stand, 0, mymedic_idle,	//12
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,	//101

};
mmove_t mymedic_move_stand = {FRAME_wait1, FRAME_wait90, mymedic_frames_stand, NULL};

void mymedic_stand (edict_t *self)
{
//	gi.dprintf("mymedic_stand()\n");
	self->monsterinfo.currentmove = &mymedic_move_stand;
}
mframe_t medic_frames_walk [] =
{
	drone_ai_walk, 6.2,	NULL,
	drone_ai_walk, 18.1,  NULL,
	drone_ai_walk, 1,		NULL,
	drone_ai_walk, 9,		NULL,
	drone_ai_walk, 10,	NULL,
	drone_ai_walk, 9,		NULL,
	drone_ai_walk, 11,	NULL,
	drone_ai_walk, 11.6,  NULL,
	drone_ai_walk, 2,		NULL,
	drone_ai_walk, 9.9,	NULL,
	drone_ai_walk, 14,	NULL,
	drone_ai_walk, 9.3,	NULL
};
mmove_t medic_move_walk = {FRAME_walk1, FRAME_walk12, medic_frames_walk, NULL};

void medic_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &medic_move_walk;
}

mframe_t mymedic_frames_run [] =
{
	drone_ai_run, 30,	NULL,	//102
	drone_ai_run, 30,	NULL,
	drone_ai_run, 30,	NULL,
	drone_ai_run, 30,	NULL,
	drone_ai_run, 30,	NULL,
	drone_ai_run, 30,	NULL	//107
	
};
mmove_t mymedic_move_run = {FRAME_run1, FRAME_run6, mymedic_frames_run, NULL};

void mymedic_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &mymedic_move_stand;
	else
		self->monsterinfo.currentmove = &mymedic_move_run;
}

void mymedic_fire_blaster (edict_t *self)
{
	int		effect, damage;
	vec3_t	forward, start;
	qboolean bounce = false;
	
	if ((self->s.frame == FRAME_attack9) || (self->s.frame == FRAME_attack12))
	{
		effect = EF_BLASTER;
		bounce = true;
	}
	else
		effect = EF_HYPERBLASTER;

	damage = 10 + 4*self->monsterinfo.level;

	MonsterAim(self, 0.8, 2000, false, MZ2_MEDIC_BLASTER_1, forward, start);
	monster_fire_blaster(self, start, forward, damage, 2000, effect, BLASTER_PROJ_BOLT, 2.0, bounce, MZ2_MEDIC_BLASTER_1);
}

void mymedic_fire_bolt (edict_t *self)
{
	int		min, max, damage;
	vec3_t	forward, start;

	min = 4*self->monsterinfo.level;
	max = 50 + 25*self->monsterinfo.level;

	damage = GetRandom(min, max);

	MonsterAim(self, 0.8, 1500, false, MZ2_MEDIC_BLASTER_1, forward, start);
	monster_fire_blaster(self, start, forward, damage, 1500, EF_BLASTER, BLASTER_PROJ_BLAST, 2.0, true, MZ2_MEDIC_BLASTER_1);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/photon.wav"), 1, ATTN_NORM, 0);
}


void mymedic_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t mymedic_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t mymedic_move_death = {FRAME_death1, FRAME_death30, mymedic_frames_death, mymedic_dead};

void mymedic_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

#ifdef OLD_NOLAG_STYLE
	// reduce lag by removing the entity right away
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}
#endif

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		//self->deadflag = DEAD_DEAD;
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

	DroneList_Remove(self);

// regular death
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &mymedic_move_death;

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void mymedic_duck_down (edict_t *self)
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

void mymedic_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	VectorClear(self->velocity);
	gi.linkentity (self);
}


void mymedic_jump_takeoff (edict_t *self)
{
	vec3_t	v;

	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	VectorSubtract(self->monsterinfo.dir, self->s.origin, v);
	v[2] = 0;
	VectorNormalize(v);
	VectorScale(v, -200, self->velocity);
	self->velocity[2] = 400;
	self->monsterinfo.pausetime = level.time + 2.0; // maximum duration of jump
}

void mymedic_jump_hold (edict_t *self)
{
	vec3_t	v;

	if (G_EntExists(self->monsterinfo.attacker))
	{
		// face the attacker
		VectorSubtract(self->monsterinfo.attacker->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);
		M_ChangeYaw(self);
	}
	// check for landing or jump timeout
	if (self->groundentity || (level.time > self->monsterinfo.pausetime))
	{
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		VectorClear(self->velocity);
	}
	else
	{
		// we're still in the air
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}
}

mframe_t mymedic_frames_duck [] =
{
	ai_move, 0,	mymedic_duck_down,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,//mymedic_duck_down,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	mymedic_duck_up
	/*
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
	*/
};
mmove_t mymedic_move_duck = {FRAME_duck1, FRAME_duck7, mymedic_frames_duck, mymedic_run};

mframe_t mymedic_frames_leap [] =
{
	ai_move, 0,	mymedic_jump_takeoff,	//131
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	mymedic_jump_hold,		//137
	/*
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
	*/
};
mmove_t mymedic_move_leap = {FRAME_duck1, FRAME_duck7, mymedic_frames_leap, mymedic_run};

void mymedic_leap (edict_t *self)
{
	if (self->groundentity)
		self->monsterinfo.currentmove = &mymedic_move_leap;
}

void mymedic_dodge (edict_t *self, edict_t *attacker, vec3_t dir, int radius)
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
		self->monsterinfo.currentmove = &mymedic_move_duck;
		self->monsterinfo.dodge_time = level.time + 2.0;
	}
	else
	{
		mymedic_leap(self);
		self->monsterinfo.dodge_time = level.time + 3.0;
	}
}

void medic_checktarget (edict_t *self)
{
	// stop shooting at corpses!
	if (!self->enemy || !self->enemy->inuse || self->enemy->health < 1)
	{
		self->monsterinfo.currentmove = &mymedic_move_stand;
		return;
	}
}

mframe_t mymedic_frames_attackHyperBlaster [] =
{
	ai_charge, 0,	mymedic_fire_blaster,	//191
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,	//195
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget,
	ai_charge, 0,	mymedic_fire_blaster,
	ai_charge, 0,	medic_checktarget			//206
};
mmove_t mymedic_move_attackHyperBlaster = {FRAME_attack15, FRAME_attack30, mymedic_frames_attackHyperBlaster, mymedic_refire};

void mymedic_refire (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.5) 
		&& (entdist(self, self->enemy) <= 512))
	{
		self->s.frame = FRAME_attack19;
		return;
	}

	self->monsterinfo.currentmove = &mymedic_move_run;
	self->monsterinfo.attack_finished = level.time + random() + 1;

}

void mymedic_continue (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (random() > 0.5))
		self->monsterinfo.currentmove = &mymedic_move_attackHyperBlaster;
	self->monsterinfo.attack_finished = level.time + random() + 1;
}


mframe_t mymedic_frames_attackBlaster [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	mymedic_fire_bolt,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	mymedic_fire_bolt,	
	ai_charge, 0,	NULL,
	ai_charge, 0,	mymedic_continue
};
mmove_t mymedic_move_attackBlaster = {FRAME_attack1, FRAME_attack14, mymedic_frames_attackBlaster, mymedic_run};

void mymedic_hook_launch (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_hook_launch, 1, ATTN_NORM, 0);
}

void mymedic_hook_retract (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_hook_retract, 1, ATTN_NORM, 0);

	if (!self->enemy)
		return;
	//if ((self->svflags & SVF_MONSTER) && !(self->client))
	//	self->enemy->monsterinfo.aiflags &= ~AI_RESURRECTING;
}

void ED_CallSpawn (edict_t *ent);

static vec3_t	mymedic_cable_offsets[] =
{
	45.0,  -9.2, 15.5,
	48.4,  -9.7, 15.2,
	47.8,  -9.8, 15.8,
	47.3,  -9.3, 14.3,
	45.4, -10.1, 13.1,
	41.9, -12.7, 12.0,
	37.8, -15.8, 11.2,
	34.3, -18.4, 10.7,
	32.7, -19.7, 10.4,
	32.7, -19.7, 10.4
};

edict_t *CreateSpiker (edict_t *ent, int skill_level);
edict_t *CreateObstacle (edict_t *ent, int skill_level);
edict_t *CreateGasser (edict_t *ent, int skill_level);
void organ_remove (edict_t *self, qboolean refund);

void M_Reanimate (edict_t *ent, edict_t *target, int r_level, float r_modifier, qboolean printMsg)
{
	vec3_t	bmin, bmax;
	edict_t *e;

	if (!strcmp(target->classname, "drone"))
	{
		// if the summoner is a player, check for sufficient monster slots
		if (ent->client && (ent->num_monsters + target->monsterinfo.control_cost > MAX_MONSTERS))
			return;

		target->monsterinfo.level = r_level;
		M_SetBoundingBox(target->mtype, bmin, bmax);

		if (G_IsValidLocation(target, target->s.origin, bmin, bmax) && M_Initialize(ent, target))
		{
			// restore this drone
			target->monsterinfo.slots_freed = false; // reset freed flag
			target->health = r_modifier*target->max_health;
			target->monsterinfo.power_armor_power = r_modifier*target->monsterinfo.max_armor;
			target->monsterinfo.resurrected_time = level.time + 10.0;
			target->activator = ent; // transfer ownership!
			target->nextthink = level.time + 1.0;
			gi.linkentity(target);
			target->monsterinfo.stand(target);

			ent->num_monsters += target->monsterinfo.control_cost;
			ent->num_monsters_real++;
			// gi.bprintf(PRINT_HIGH, "adding %p (%d)\n", target, ent->num_monsters_real);

			// make sure invasion monsters hunt for navi
			if (invasion->value && !ent->client && ent->activator && !ent->activator->client)
			{
				target->monsterinfo.aiflags &= ~AI_STAND_GROUND;
				target->monsterinfo.aiflags |= AI_FIND_NAVI;
			}

			if (ent->client && printMsg)
				safe_cprintf(ent, PRINT_HIGH, "Resurrected a %s. (%d/%d)\n", target->classname, 
					ent->num_monsters, (int)MAX_MONSTERS);
		}
	}
	else if ((!strcmp(target->classname, "bodyque") || !strcmp(target->classname, "player")))
	{
		int		random=GetRandom(1, 3);
		vec3_t	start;

		// if the summoner is a player, check for sufficient monster slots
		if (ent->client && (ent->num_monsters + 1 > MAX_MONSTERS))
			return;

		e = G_Spawn();

		VectorCopy(target->s.origin, start);

		// kill the corpse
		T_Damage(target, target, target, vec3_origin, target->s.origin, 
			vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, 0);

		//4.2 random soldier type with different weapons
		if (random == 1)
		{
			// blaster
			e->mtype = M_SOLDIER;
			e->s.skinnum = 0;
		}
		else if (random == 2)
		{
			// rocket
			e->mtype = M_SOLDIERLT;
			e->s.skinnum = 4;
		}
		else
		{
			// shotgun
			e->mtype = M_SOLDIERSS;
			e->s.skinnum = 2;
		}

		e->activator = ent;
		e->monsterinfo.level = r_level;
		M_Initialize(ent, e);
		e->health = r_modifier*e->max_health;
		e->monsterinfo.power_armor_power = r_modifier*e->monsterinfo.max_armor;
		e->monsterinfo.resurrected_time = level.time + 10.0;
		e->s.skinnum |= 1; // injured skin

		e->monsterinfo.stand(e);
		
		if (!G_IsValidLocation(target, start, e->mins, e->maxs))
		{
			start[2] += 24;
			if (!G_IsValidLocation(target, start, e->mins, e->maxs))
			{
				G_FreeEdict(e);
				return;
			}
		}

		VectorCopy(start, e->s.origin);
		gi.linkentity(e);
		e->nextthink = level.time + 1.0;

		ent->num_monsters += e->monsterinfo.control_cost;
		ent->num_monsters_real++;
		// gi.bprintf(PRINT_HIGH, "adding %p (%d)\n", e, ent->num_monsters_real);

		// make sure invasion monsters hunt for navi
		if (invasion->value && !ent->client && ent->activator && !ent->activator->client)
		{
			target->monsterinfo.aiflags &= ~AI_STAND_GROUND;
			target->monsterinfo.aiflags |= AI_FIND_NAVI;
		}

		if (ent->client && printMsg)
			safe_cprintf(ent, PRINT_HIGH, "Resurrected a soldier. (%d/%d)\n", ent->num_monsters, (int)MAX_MONSTERS);
	}
	else if (!strcmp(target->classname, "spiker"))
	{
		// if the summoner is a player, check for sufficient spiker slots
		if (ent->client && (ent->num_spikers + 1 > SPIKER_MAX_COUNT))
			return;

		e = CreateSpiker(ent, r_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_spikers--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->health = r_modifier * e->max_health;
		e->monsterinfo.resurrected_time = level.time + 10.0;
		e->s.frame = 4;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);

		if (ent->client && printMsg)
			safe_cprintf(ent, PRINT_HIGH, "Resurrected a spiker. (%d/%d)\n", ent->num_spikers, SPIKER_MAX_COUNT);
	}
	else if (!strcmp(target->classname, "obstacle"))
	{
		// if the summoner is a player, check for sufficient obstacle slots
		if (ent->client && (ent->num_obstacle + 1 > OBSTACLE_MAX_COUNT))
			return;

		e = CreateObstacle(ent, r_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_obstacle--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->monsterinfo.resurrected_time = level.time + 10.0;
		e->health = r_modifier * e->max_health;
		e->s.frame = 6;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);

		if (ent->client && printMsg)
			safe_cprintf(ent, PRINT_HIGH, "Resurrected an obstacle. (%d/%d)\n", ent->num_obstacle, OBSTACLE_MAX_COUNT);
	}
	else if (!strcmp(target->classname, "gasser"))
	{
		// if the summoner is a player, check for sufficient gasser slots
		if (ent->client && (ent->num_gasser + 1 > GASSER_MAX_COUNT))
			return;

		e = CreateGasser(ent, r_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_gasser--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->monsterinfo.resurrected_time = level.time + 10.0;
		e->health = r_modifier * e->max_health;
		e->s.frame = 0;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);

		if (ent->client && printMsg)
			safe_cprintf(ent, PRINT_HIGH, "Resurrected a gasser. (%d/%d)\n", ent->num_gasser, GASSER_MAX_COUNT);
	}
		
}

void mymedic_cable_attack (edict_t *self)
{
	vec3_t	forward, right, start, offset, end;
	trace_t	tr;

	// need a valid target and activator
	if (!self || !self->inuse || !self->activator || !self->activator->inuse 
		|| !self->enemy || !self->enemy->inuse)
		return;

	// make sure target is still in range
	if (entdist(self, self->enemy) > 256)
		return;

	// get muzzle location
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorCopy(mymedic_cable_offsets[self->s.frame - FRAME_attack42], offset);
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	// get end position
	//VectorCopy(self->enemy->s.origin, end);
	//end[2] = self->enemy->absmax[2]-8;
	G_EntMidPoint(self->enemy, end);

	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.ent != self->enemy)
		return; // cable is blocked

	// cable effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// the target needs healing
	if (M_NeedRegen(self->enemy))
	{
		int frames = 6000/(12*self->monsterinfo.level);
		float mult = 1.0;

		if (!frames)
			frames = 1;

		// remove all curses
		CurseRemove(self->enemy, 0);

		//Give them a short period of curse immunity
		self->enemy->holywaterProtection = level.time + 2.0; //2 seconds immunity

		// heal them
		M_Regenerate(self->enemy, frames, 0, mult, true, true, false, &self->enemy->monsterinfo.regen_delay2);

		// hold monsters in-place
		if (self->enemy->svflags & SVF_MONSTER)
			self->enemy->holdtime = level.time + 0.2;
	}
	// the target is a dead monster and needs resurrection
	else if (self->enemy->health < 1)
	{
		M_Reanimate(self->activator, self->enemy, self->monsterinfo.level, 0.33, false);
	}
}

void mymedic_delay (edict_t *self)
{
	self->monsterinfo.attack_finished = level.time + random() + 1;
}

void mymedic_cable_continue (edict_t *self)
{
	// if target still needs healing, loop heal frames
	if (M_ValidMedicTarget(self, self->enemy) && (entdist(self, self->enemy) <= 256))
	{
		self->s.frame = 218;
		mymedic_cable_attack(self);
	}
}

mframe_t mymedic_frames_attackCable [] =
{
	ai_charge, 0,		NULL,					//209
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		mymedic_hook_launch,	//218
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_cable_continue,	//227--loop from 218 to here
	ai_charge, 0,		mymedic_cable_attack,
	ai_charge, 0,		mymedic_hook_retract,	//229
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		NULL,
	ai_charge, 0,		mymedic_delay			//236
};
mmove_t mymedic_move_attackCable = {FRAME_attack33, FRAME_attack60, mymedic_frames_attackCable, mymedic_heal};

void drone_wakeallies (edict_t *self);
// search for nearby enemies, return true if one is found
// this is to make the medic stop healing if there is a higher priority target
// this function is the same as drone_findtarget(), except that it skips the medic check
qboolean mymedic_findenemy (edict_t *self)
{
	edict_t *target=NULL;

	while ((target = findclosestradius (target, self->s.origin, 1024)) != NULL)
	{
		if (!G_ValidTarget(self, target, true))
			continue;
		self->enemy = target;
		drone_wakeallies(self);
		return true;
	}
	return false;
}

void mymedic_heal (edict_t *self)
{
	// stop healing our target died, if they are fully healed, or
	// they have gone out of range while we are standing ground (can't reach them)
	if (!G_EntIsAlive(self->enemy) || !M_NeedRegen(self->enemy)
		|| ((self->monsterinfo.aiflags & AI_STAND_GROUND) 
		&& (entdist(self, self->enemy) > 256)))
	{
		self->enemy = NULL;
		mymedic_stand(self);
		return;
	}

	// continue healing if our target is still in range and
	// there are no enemies around
	if (OnSameTeam(self, self->enemy) && (entdist(self, self->enemy) <= 256)
		&& !mymedic_findenemy(self))
		self->monsterinfo.currentmove = &mymedic_move_attackCable;
	else
		mymedic_run(self);
}

void mymedic_attack(edict_t *self)
{
	float	dist, r;

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;

	dist = entdist(self, self->enemy);
	r = random();

	if ((self->monsterinfo.aiflags & AI_MEDIC)
		&& ((self->enemy->health < 1 || OnSameTeam(self, self->enemy))))
	{
		if (dist <= 256)
			self->monsterinfo.currentmove = &mymedic_move_attackCable;
		return;
	}

	if (dist <= 256)
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &mymedic_move_attackHyperBlaster;
		else
			self->monsterinfo.currentmove = &mymedic_move_attackBlaster;
	}
	else
	{
		if (r <= 0.3)
			self->monsterinfo.currentmove = &mymedic_move_attackBlaster;
		else
			self->monsterinfo.currentmove = &mymedic_move_attackHyperBlaster;
	}
}

void mymedic_melee (edict_t *self)
{
	// just here to keep monster from circle strafing
}

void mymedic_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

/*QUAKED monster_medic (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void init_drone_medic (edict_t *self)
{
	sound_idle1 = gi.soundindex ("medic/idle.wav");
	sound_pain1 = gi.soundindex ("medic/medpain1.wav");
	sound_pain2 = gi.soundindex ("medic/medpain2.wav");
	sound_die = gi.soundindex ("medic/meddeth1.wav");
	sound_sight = gi.soundindex ("medic/medsght1.wav");
	sound_search = gi.soundindex ("medic/medsrch1.wav");
	sound_hook_launch = gi.soundindex ("medic/medatck2.wav");
	sound_hook_hit = gi.soundindex ("medic/medatck3.wav");
	sound_hook_heal = gi.soundindex ("medic/medatck4.wav");
	sound_hook_retract = gi.soundindex ("medic/medatck5.wav");

	gi.soundindex ("medic/medatck1.wav");

	self->monsterinfo.control_cost = M_MEDIC_CONTROL_COST;
	self->monsterinfo.cost = M_MEDIC_COST;
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/medic/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 32);

	//if (self->activator && self->activator->client)
	self->health = 120 + 65*self->monsterinfo.level;
	//else self->health = 200 + 20*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;//-self->health;
	self->mass = 400;
	self->mtype = M_MEDIC;
	self->monsterinfo.aiflags |= AI_MEDIC; // use medic ai
	self->style = 1;// for blaster bolt

//	self->pain = mymedic_pain;
	self->die = mymedic_die;
//	self->touch = mymedic_touch;

	self->item = FindItemByClassname("item_adrenaline");

	self->monsterinfo.stand = mymedic_stand;
	self->monsterinfo.walk = medic_walk;
	self->monsterinfo.run = mymedic_run;
	self->monsterinfo.dodge = mymedic_dodge;
	self->monsterinfo.attack = mymedic_attack;
	self->monsterinfo.melee = mymedic_melee;
	self->monsterinfo.sight = mymedic_sight;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
//	self->monsterinfo.idle = mymedic_idle;
//	self->monsterinfo.search = mymedic_search;
//	self->monsterinfo.checkattack = mymedic_checkattack;
//	self->monsterinfo.control_cost = 1;
//	self->monsterinfo.cost = 150;

	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	//self->monsterinfo.melee = 1;

	//if (self->activator && self->activator->client)
		self->monsterinfo.power_armor_power = 60 + 50*self->monsterinfo.level;
	//else self->monsterinfo.power_armor_power = 200 + 40*self->monsterinfo.level;

	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &mymedic_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start (self);
	self->nextthink = level.time + FRAMETIME;
	//self->activator->num_monsters += self->monsterinfo.control_cost;
}
