/*
==============================================================================

MEDIC

==============================================================================
*/

#include "g_local.h"
#include "m_medic.h"

qboolean visible (edict_t *self, edict_t *other);
void ai_run_slide(edict_t *self, float distance);

//static int	sound_idle1;
static int	sound_pain1;
//static int	sound_pain2;
static int	sound_die;
static int	sound_sight;
//static int	sound_search;
static int	sound_hook_launch;
static int	sound_hook_hit;
static int	sound_hook_heal;
static int	sound_hook_retract;


edict_t *medic_FindDeadMonster (edict_t *self)
{
	edict_t	*ent = NULL;
	edict_t	*best = NULL;
	vec3_t vec;
	float range = 0, bestrange = -100;

/*	gi.dprintf("medic_FindDeadMonster start\n");
	if (self->enemy)
		gi.dprintf("(%s)\n", self->enemy->classname);
*/	if (self->enemy && (self->enemy->svflags & SVF_MONSTER)) {
		best = self->enemy;
		VectorSubtract (self->s.origin, self->enemy->s.origin, vec);
		bestrange = VectorLength(vec);
	}
	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL)
	{
		if (ent == self)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (ent->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		if (ent->owner)
			continue;
		if (ent->health > 0)
			continue;
		if (ent->health < ent->gib_health)
			continue;
		if (ent->client)
			continue;
//		if (ent->nextthink)
//			continue;
		if (!visible(self, ent))
			continue;
		VectorSubtract (self->s.origin, ent->s.origin, vec);
		range = VectorLength(vec);
		if (!best)
		{
			best = ent;
			bestrange = range;
			continue;
		}
		if (range > bestrange)
			continue;
		if (ent->max_health <= best->max_health)
			continue;
		best = ent;
		bestrange = range;
	}

/*	if (!best)
		gi.dprintf("medic_FindDeadMonster found nothing\n");
	else
		gi.dprintf("medic_FindDeadMonster found %s\n", best->classname);
*/	if (!best || best->client)
		return NULL;
	else
		return best;
}

void medic_idle (edict_t *self) {
	edict_t	*ent;

//	gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);

	ent = medic_FindDeadMonster(self);
	if (ent) {
		self->enemy = ent;
		self->enemy->owner = self;
		self->monsterinfo.aiflags |= AI_MEDIC;
		FoundTarget (self);
	}
}

void medic_search (edict_t *self) {
	edict_t	*ent;

//	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_IDLE, 0);

//	if (!self->oldenemy)
//	{
		ent = medic_FindDeadMonster(self);
		if (ent && (ent != self->enemy)) {
			if (self->oldenemy && !self->oldenemy->client && (self->oldenemy->owner == self))
				self->oldenemy->owner = NULL;
			self->oldenemy = self->enemy;
			self->enemy = ent;
			self->enemy->owner = self;
			self->monsterinfo.aiflags |= AI_MEDIC;
			FoundTarget (self);
		}
//	}
/*	if (self->enemy) {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (self - g_edicts);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (self->enemy->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}*/
}


mframe_t medic_frames_stand [] =
{
	{ai_stand, 0, medic_idle},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
	{ai_stand, 0, NULL},
};
mmove_t medic_move_stand = {FRAME_wait1, FRAME_wait90, medic_frames_stand, NULL};

void medic_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_stand;
}


mframe_t medic_frames_walk [] =
{
	{ai_walk, 6.2,	NULL},
	{ai_walk, 18.1,  NULL},
	{ai_walk, 1,		NULL},
	{ai_walk, 9,		NULL},
	{ai_walk, 10,	NULL},
	{ai_walk, 9,		NULL},
	{ai_walk, 11,	NULL},
	{ai_walk, 11.6,  NULL},
	{ai_walk, 2,		NULL},
	{ai_walk, 9.9,	NULL},
	{ai_walk, 14,	NULL},
	{ai_walk, 9.3,	NULL}
};
mmove_t medic_move_walk = {FRAME_walk1, FRAME_walk12, medic_frames_walk, NULL};

void medic_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &medic_move_walk;
}


mframe_t medic_frames_run [] =
{
	{ai_run, 18,		medic_search},
	{ai_run, 22.5,	NULL},
	{ai_run, 25.4,	NULL},
	{ai_run, 23.4,	NULL},
	{ai_run, 24,		NULL},
	{ai_run, 35.6,	NULL}

};
mmove_t medic_move_run = {FRAME_run1, FRAME_run6, medic_frames_run, NULL};

void medic_run (edict_t *self)
{
	if (!(self->monsterinfo.aiflags & AI_MEDIC))
	{
		edict_t	*ent;

		ent = medic_FindDeadMonster(self);
		if (ent)
		{
			if (self->oldenemy && !self->oldenemy->client && (self->oldenemy->owner == self))
				self->oldenemy->owner = NULL;
			self->oldenemy = self->enemy;
			self->enemy = ent;
			self->enemy->owner = self;
			self->monsterinfo.aiflags |= AI_MEDIC;
			FoundTarget (self);
			return;
		}
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &medic_move_stand;
	else
		self->monsterinfo.currentmove = &medic_move_run;
}


mframe_t medic_frames_pain1 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t medic_move_pain1 = {FRAME_paina1, FRAME_paina8, medic_frames_pain1, medic_run};

mframe_t medic_frames_pain2 [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t medic_move_pain2 = {FRAME_painb1, FRAME_painb15, medic_frames_pain2, medic_run};

void medic_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + 0.2 * self->monsterinfo.skill;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}

	//FIXME: move_pain1 causes server to crash!
//	if (random() < 0.5) {
//		self->monsterinfo.currentmove = &medic_move_pain1;
//	} else {
		self->monsterinfo.currentmove = &medic_move_pain2;
//	}
	self->monsterinfo.jumptime = level.time + 4.0;
}

void medic_fire_blaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;

	effect = 0;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MEDIC_BLASTER_1], forward, right, start);

	if (self->enemy) {
		VectorCopy (self->enemy->s.origin, end);
		end[2] += self->enemy->viewheight;
		VectorSubtract (end, start, dir);
	} else {
		VectorCopy (forward, dir);
	}

	if (self->radius_dmg)
		monster_fire_blaster (self, start, dir, 12 + 4 * self->monsterinfo.skill, 700 + 12 * self->monsterinfo.skill, MZ2_MEDIC_BLASTER_1, effect);
	else
		monster_fire_blaster (self, start, dir, 6 + 2 * self->monsterinfo.skill, 650 + 6 * self->monsterinfo.skill, MZ2_MEDIC_BLASTER_1, effect);
}


void medic_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = monster_corpse_think;
	self->nextthink = level.time + MONSTER_CORPSE_TIMEOUT;
	gi.linkentity (self);
}

mframe_t medic_frames_death [] =
{
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL},
	{ai_move, 0, NULL}
};
mmove_t medic_move_death = {FRAME_death1, FRAME_death30, medic_frames_death, medic_dead};

void medic_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// if we had a pending patient, free him up for another medic
	if ((self->enemy) && (self->enemy->owner == self))
		self->enemy->owner = NULL;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &medic_move_death;
}


void medic_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
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
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t medic_frames_duck [] =
{
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	medic_duck_down},
	{ai_move, -1,	medic_duck_hold},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	medic_duck_up},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL},
	{ai_move, -1,	NULL}
};
mmove_t medic_move_duck = {FRAME_duck1, FRAME_duck16, medic_frames_duck, medic_run};

void medic_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

//	self->monsterinfo.currentmove = &medic_move_duck;
}

void medic_checkreblast (edict_t *self) {
	if ((self->enemy) && (self->enemy->health > 0) && visible(self, self->enemy)) {
		self->s.frame -= 13;
	} else {
		medic_run(self);
	}
}

mframe_t medic_frames_attackHyperBlaster [] =
{
	{ai_run_circle, 10,	NULL},
	{ai_run_circle, 10,	NULL},
	{ai_run_circle, 10,	NULL},
	{ai_run_circle, 10,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL},
	{ai_run_circle, 15,	medic_fire_blaster},
	{ai_run_circle, 15,	NULL}
};
mmove_t medic_move_attackHyperBlaster = {FRAME_attack15, FRAME_attack30, medic_frames_attackHyperBlaster, medic_checkreblast};


void medic_continue (edict_t *self)
{
	if (!self->enemy)
		return;
	if (visible (self, self->enemy) )
		if (random() > 0.75)
			self->monsterinfo.currentmove = &medic_move_attackHyperBlaster;
}


mframe_t medic_frames_attackBlaster [] =
{
	{ai_charge, 0,	NULL},
	{ai_run_slide, 5,	NULL},
	{ai_run_slide, 5,	NULL},
	{ai_run_slide, 3,	NULL},
	{ai_run_slide, 2,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	medic_fire_blaster},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	medic_fire_blaster},
	{ai_charge, 0,	NULL},
	{ai_charge, 0,	medic_continue}	// Change to medic_continue... Else, go to frame 32
};
mmove_t medic_move_attackBlaster = {FRAME_attack1, FRAME_attack14, medic_frames_attackBlaster, medic_run};


void medic_hook_launch (edict_t *self)
{
	if (!self->enemy)
		return;
	if (self->enemy->client)
		return;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_hook_launch, 1, ATTN_NORM, 0);
	}
}

void ED_CallSpawn (edict_t *ent);

static vec3_t	medic_cable_offsets[] =
{
	{45.0,  -9.2, 15.5},
	{48.4,  -9.7, 15.2},
	{47.8,  -9.8, 15.8},
	{47.3,  -9.3, 14.3},
	{45.4, -10.1, 13.1},
	{41.9, -12.7, 12.0},
	{37.8, -15.8, 11.2},
	{34.3, -18.4, 10.7},
	{32.7, -19.7, 10.4},
	{32.7, -19.7, 10.4}
};

void medic_cable_attack (edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	char command[32];

	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;
	if (self->enemy->deadflag != DEAD_DEAD)
		return;
	if (self->enemy->health <= self->enemy->gib_health)
		return;
	if (self->enemy->health > 0)
		return;
	if (self->enemy->client)
		return;
	if (self->enemy->owner != self)
		return;

	AngleVectors (self->s.angles, f, r, NULL);
	VectorCopy (medic_cable_offsets[self->s.frame - FRAME_attack42], offset);
	G_ProjectSource (self->s.origin, offset, f, r, start);

	if (self->s.frame == FRAME_attack43) {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			gi.sound (self->enemy, CHAN_AUTO, sound_hook_hit, 1, ATTN_NORM, 0);
		}
		self->enemy->monsterinfo.aiflags |= AI_RESURRECTING;
		if ((random() < 0.4) || (self->enemy->classid == CI_M_MAKRON) || (self->enemy->classid == CI_M_TANK) || (self->enemy->classid == CI_M_TANKCOMM)){
			T_Damage(self->enemy, self, self, vec3_origin, self->enemy->s.origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
//			T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);
		}
	} else if (self->s.frame == FRAME_attack50) {
		if (self->enemy->classid != CI_M_JORG) {
			sprintf(command, "%d", self->enemy->monsterinfo.skill);
			gi.cvar_forceset("skill", command);

			self->enemy->spawnflags = 0;
			self->enemy->monsterinfo.aiflags = 0;
			self->enemy->target = NULL;
			self->enemy->targetname = NULL;
			self->enemy->combattarget = NULL;
			self->enemy->deathtarget = NULL;
			self->enemy->owner = self;
			ED_CallSpawn (self->enemy);
			self->enemy->svflags &= ~SVF_DEADMONSTER;
			self->enemy->owner = NULL;
			if (self->radius_dmg) {
				self->enemy->health *= 1.5;
				self->enemy->max_health *= self->enemy->health;
			}

/*			if (self->enemy->think) {
				self->enemy->nextthink = level.time;
				self->enemy->think (self->enemy);
			}*/
			self->enemy->monsterinfo.aiflags |= AI_RESURRECTING;
//			self->enemy->enemy = NULL;
//			self->enemy->oldenemy = NULL;
//			if (self->enemy->enemy)
//				FoundTarget (self->enemy);
		}
	} else {
		if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
			if (self->s.frame == FRAME_attack44) {
				gi.sound (self, CHAN_WEAPON, sound_hook_heal, 1, ATTN_NORM, 0);
			}
		}
	}

	// adjust start for beam origin being in middle of a segment
	VectorMA (start, 8, f, start);

	// adjust end z for end spot since the monster is currently dead
	VectorCopy (self->enemy->s.origin, end);
	end[2] = self->enemy->absmin[2] + self->enemy->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void medic_hook_retract (edict_t *self) {
	if (!self->enemy)
		return;
	if (self->enemy->client)
		return;
	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_WEAPON, sound_hook_retract, 1, ATTN_NORM, 0);
	}
	self->enemy->monsterinfo.aiflags &= ~AI_RESURRECTING;
}

mframe_t medic_frames_attackCable [] =
{
	{ai_move, 2,		NULL},
	{ai_move, 3,		NULL},
	{ai_move, 5,		NULL},
	{ai_move, 4.4,	NULL},
	{ai_charge, 4.7,	NULL},
	{ai_charge, 5,	NULL},
	{ai_charge, 6,	NULL},
	{ai_charge, 4,	NULL},
	{ai_charge, 0,	NULL},
	{ai_move, 0,		medic_hook_launch}, // 43?
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack}, // 50?
	{ai_move, 0,		medic_cable_attack},
	{ai_move, 0,		medic_cable_attack},
	{ai_move, -30,	medic_hook_retract}, // -15
	{ai_move, -3.0,	NULL}, // -1.5
	{ai_move, -2.4,	NULL}, // -1.2
	{ai_move, -6,	NULL}, // -3
	{ai_move, -4,	NULL}, // -2
	{ai_move, 0.3,	NULL},
	{ai_move, 0.7,	NULL},
	{ai_move, 1.2,	NULL},
	{ai_move, 1.3,	NULL}
};
mmove_t medic_move_attackCable = {FRAME_attack33, FRAME_attack60, medic_frames_attackCable, medic_run};


void medic_attack(edict_t *self)
{
	if ((self->monsterinfo.aiflags & AI_MEDIC) && self->enemy && !self->enemy->client)
		self->monsterinfo.currentmove = &medic_move_attackCable;
	else
		self->monsterinfo.currentmove = &medic_move_attackHyperBlaster;
}

void medic_sight (edict_t *self, edict_t *other)
{
	if (!(self->monsterinfo.aiflags & AI_MEDIC) && (random() < (0.4 + 0.05 * self->monsterinfo.skill)))
		self->monsterinfo.currentmove = &medic_move_attackHyperBlaster;

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	}
}

qboolean medic_checkattack (edict_t *self) {
	if (!self->enemy)
		return false;
	if (self->monsterinfo.aiflags & AI_MEDIC)
	{
		vec3_t	offset, start, f, r;
		vec3_t	dir, angles;
		float	distance;

		AngleVectors (self->s.angles, f, r, NULL);
		VectorCopy (medic_cable_offsets[self->s.frame - FRAME_attack42], offset);
		G_ProjectSource (self->s.origin, offset, f, r, start);

		// check for max distance
		VectorSubtract (start, self->enemy->s.origin, dir);
		distance = VectorLength(dir);
		if (distance > 256)
			return false;

		// check for min/max pitch
		vectoangles (dir, angles);
		if (angles[0] < -180)
			angles[0] += 360;
		if (fabs(angles[0]) > 45)
			return false;

		if (!visible(self, self->enemy))
			return false;

		// Passed all checks
		medic_attack(self);
		return true;
	}

	return M_CheckAttack (self);
}


/*QUAKED monster_medic (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_medic (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

//	sound_idle1 = gi.soundindex ("medic/idle.wav");
	sound_pain1 = gi.soundindex ("medic/medpain1.wav");
//	sound_pain2 = gi.soundindex ("medic/medpain2.wav");
	sound_die = gi.soundindex ("medic/meddeth1.wav");
	sound_sight = gi.soundindex ("medic/medsght1.wav");
//	sound_search = gi.soundindex ("medic/medsrch1.wav");
	sound_hook_launch = gi.soundindex ("medic/medatck2.wav");
	sound_hook_hit = gi.soundindex ("medic/medatck3.wav");
	sound_hook_heal = gi.soundindex ("medic/medatck4.wav");
	sound_hook_retract = gi.soundindex ("medic/medatck5.wav");

	gi.soundindex ("medic/medatck1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/medic/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 32);

	self->mass = 400;
	self->health = 400 + 60 * skill->value;
	self->max_health = self->health;
	self->monsterinfo.level = 870 + 550 * skill->value;
	self->monsterinfo.skill = skill->value;
	self->gib_health = -190 - 22 * skill->value;
	strcpy(self->monsterinfo.name, "a medic");

	self->pain = medic_pain;
	self->die = medic_die;

	self->monsterinfo.stand = medic_stand;
	self->monsterinfo.walk = medic_walk;
	self->monsterinfo.run = medic_run;
	self->monsterinfo.dodge = NULL; // medic_dodge;
	self->monsterinfo.attack = medic_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = medic_sight;
	self->monsterinfo.idle = medic_idle;
	self->monsterinfo.search = medic_search;
	self->monsterinfo.checkattack = medic_checkattack;
	self->monsterinfo.aggressive = false;
	self->monsterinfo.prefered_range = 64;

	self->monsterinfo.cts_any = 0.45;
	self->monsterinfo.cts_class1 = 0.03;
	self->monsterinfo.cts_class2 = 0.12;
	self->monsterinfo.cts_class3 = 0.38;

	self->monsterinfo.item_any = 0.13;
	self->monsterinfo.itemmult_class1 = 1.0;
	self->monsterinfo.itemmult_class2 = 1.0;
	self->monsterinfo.itemmult_class3 = 1.0;
	self->monsterinfo.itemmult_class4 = 0.9;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &medic_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
