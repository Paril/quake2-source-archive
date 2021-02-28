/*
==============================================================================

BERSERK

==============================================================================
*/

#include "g_local.h"
#include "m_berserk.h"


static int sound_pain;
static int sound_die;
static int sound_idle;
static int sound_punch;
static int sound_sight;
static int sound_search;

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

void berserk_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void berserk_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


void berserk_fidget (edict_t *self);
mframe_t berserk_frames_stand [] =
{
	drone_ai_stand, 0, berserk_fidget,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t berserk_move_stand = {FRAME_stand1, FRAME_stand5, berserk_frames_stand, NULL};

void berserk_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &berserk_move_stand;
}

mframe_t berserk_frames_stand_fidget [] =
{
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
	drone_ai_stand, 0, NULL
};
mmove_t berserk_move_stand_fidget = {FRAME_standb1, FRAME_standb20, berserk_frames_stand_fidget, berserk_stand};

void berserk_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() > 0.15)
		return;

	self->monsterinfo.currentmove = &berserk_move_stand_fidget;
	gi.sound (self, CHAN_WEAPON, sound_idle, 1, ATTN_IDLE, 0);
}

mframe_t berserk_frames_walk [] =
{
	drone_ai_walk, 9.1, NULL,
	drone_ai_walk, 6.3, NULL,
	drone_ai_walk, 4.9, NULL,
	drone_ai_walk, 6.7, NULL,
	drone_ai_walk, 6.0, NULL,
	drone_ai_walk, 8.2, NULL,
	drone_ai_walk, 7.2, NULL,
	drone_ai_walk, 6.1, NULL,
	drone_ai_walk, 4.9, NULL,
	drone_ai_walk, 4.7, NULL,
	drone_ai_walk, 4.7, NULL,
	drone_ai_walk, 4.8, NULL
};
mmove_t berserk_move_walk = {FRAME_walkc1, FRAME_walkc11, berserk_frames_walk, NULL};

void berserk_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &berserk_move_walk;
}

mframe_t berserk_frames_run1 [] =
{ // from 35 to 27
	drone_ai_run, 27, NULL,
	drone_ai_run, 27, NULL,
	drone_ai_run, 27, NULL,
	drone_ai_run, 27, NULL,
	drone_ai_run, 27, NULL,
	drone_ai_run, 27, NULL
};
mmove_t berserk_move_run1 = {FRAME_run1, FRAME_run6, berserk_frames_run1, NULL};

void berserk_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &berserk_move_stand;
	else
		self->monsterinfo.currentmove = &berserk_move_run1;
}

void berserk_attack_spike (edict_t *self)
{
	int damage;

	if (!G_EntExists(self->enemy))
		return;

	damage = 60 + 20 * self->monsterinfo.level;
	M_MeleeAttack(self, 96, damage, 400);
	//FIXME: add bleed curse
}

void berserk_swing (edict_t *self)
{
	//gi.dprintf("played sound at %d on frame %d\n", level.framenum, self->s.frame);
	gi.sound (self, CHAN_WEAPON, sound_punch, 1, ATTN_STATIC, 0);
}

mframe_t berserk_frames_attack_spike [] =
{
	ai_charge, 0, berserk_swing,
	ai_charge, 0, berserk_attack_spike,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t berserk_move_attack_spike = {FRAME_att_c3, FRAME_att_c8, berserk_frames_attack_spike, berserk_run};

void berserk_attack_club (edict_t *self)
{
	int		damage;

	if (!G_EntExists(self->enemy))
		return;

	damage = 45 + 20 * self->monsterinfo.level;
	M_MeleeAttack(self, 96, damage, 400);
}

mframe_t berserk_frames_attack_club [] =
{	
	ai_charge, 0, berserk_swing,
	ai_charge, 0, NULL,
	ai_charge, 0, berserk_attack_club,
	ai_charge, 0, berserk_attack_club,
	ai_charge, 0, berserk_attack_club,
	ai_charge, 0, NULL
};
mmove_t berserk_move_attack_club = {FRAME_att_b10, FRAME_att_b15, berserk_frames_attack_club, berserk_run};

mframe_t berserk_frames_runattack1 [] =
{	
	drone_ai_run, 35, berserk_swing,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, berserk_attack_club,
	drone_ai_run, 35, berserk_attack_club,
	drone_ai_run, 35, berserk_attack_club,
	drone_ai_run, 35, NULL
};
mmove_t berserk_move_runattack1 = {FRAME_r_att13, FRAME_r_att18, berserk_frames_runattack1, berserk_run};


void berserk_attack_strike (edict_t *self)
{
	int damage;
	trace_t tr;
	edict_t *other=NULL;
	vec3_t	v;

	// tank must be on the ground to punch
	if (!self->groundentity)
		return;

	self->lastsound = level.framenum;

	damage = 45+10*self->monsterinfo.level;
	gi.sound (self, CHAN_AUTO, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	
	while ((other = findradius(other, self->s.origin, 128)) != NULL)
	{
		if (!G_ValidTarget(self, other, true))
			continue;

		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage(other, self, self, v, tr.endpos, tr.plane.normal, damage, 200, 0, MOD_TANK_PUNCH);
	}
}

mframe_t berserk_frames_attack_strike [] =
{
	ai_move, 0, berserk_swing,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, berserk_attack_strike
};
	
mmove_t berserk_move_attack_strike = {FRAME_slam5, FRAME_slam10, berserk_frames_attack_strike, berserk_run};

void berserk_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t berserk_frames_death1 [] =
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
	ai_move, 0, NULL
	
};
mmove_t berserk_move_death1 = {FRAME_death1, FRAME_death13, berserk_frames_death1, berserk_dead};


mframe_t berserk_frames_death2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t berserk_move_death2 = {FRAME_deathc1, FRAME_deathc8, berserk_frames_death2, berserk_dead};

void berserk_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		M_Remove(self, false, true);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	DroneList_Remove(self);
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (damage >= 50)
		self->monsterinfo.currentmove = &berserk_move_death1;
	else
		self->monsterinfo.currentmove = &berserk_move_death2;

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void berserk_attack (edict_t *self)
{
	float	r = random();
	float	dist = entdist(self, self->enemy);

	if (dist > 128)
		return;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (random() <= 0.3 && dist <= 96)
			self->monsterinfo.currentmove = &berserk_move_attack_club;
		else
			self->monsterinfo.currentmove = &berserk_move_attack_strike;
	}
	else if (dist <= 96)
	{
		if (random() <= 0.2)
			self->monsterinfo.currentmove = &berserk_move_attack_strike;
		else
			self->monsterinfo.currentmove = &berserk_move_runattack1;
	}
	else
		return;

	self->monsterinfo.attack_finished = level.time + 0.6;
}

void berserk_melee (edict_t *self)
{

}

/*QUAKED monster_berserk (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void init_drone_berserk (edict_t *self)
{
	sound_pain  = gi.soundindex ("berserk/berpain2.wav");
	sound_die   = gi.soundindex ("berserk/berdeth2.wav");
	sound_idle  = gi.soundindex ("berserk/beridle1.wav");
	sound_punch = gi.soundindex ("berserk/attack.wav");
	sound_search = gi.soundindex ("berserk/bersrch1.wav");
	sound_sight = gi.soundindex ("berserk/sight.wav");

	self->s.modelindex = gi.modelindex("models/monsters/berserk/tris.md2");

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = self->max_health = 120 + 75 * self->monsterinfo.level;
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = self->monsterinfo.max_armor = 60 + 50 * self->monsterinfo.level;
	self->gib_health = -60;
	self->mass = 250;
	self->monsterinfo.control_cost = 40;
	self->monsterinfo.cost = M_DEFAULT_COST;//FIXME
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	self->mtype = M_BERSERK;

	//self->pain = berserk_pain;
	self->die = berserk_die;

	self->monsterinfo.stand = berserk_stand;
	self->monsterinfo.walk = berserk_walk;
	self->monsterinfo.run = berserk_run;
	//self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = berserk_attack;
	self->monsterinfo.melee = berserk_melee;
	self->monsterinfo.sight = berserk_sight;
	//self->monsterinfo.search = berserk_search;

	self->monsterinfo.currentmove = &berserk_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	gi.linkentity (self);

	//self->nextthink = level.time + FRAMETIME;
}
