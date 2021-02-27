/*
==============================================================================

GLADIATOR

==============================================================================
*/

#include "g_local.h"
#include "m_gladiator.h"

static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_gun;
static int	sound_cleaver_swing;
static int	sound_cleaver_hit;
static int	sound_cleaver_miss;
static int	sound_idle;
static int	sound_search;
static int	sound_sight;

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

void gladiator_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void gladiator_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}

void gladiator_cleaver_swing (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_cleaver_swing, 1, ATTN_NORM, 0);
}

mframe_t gladiator_frames_stand [] =
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t gladiator_move_stand = {FRAME_stand1, FRAME_stand7, gladiator_frames_stand, NULL};

void gladiator_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &gladiator_move_stand;
}

mframe_t gladiator_frames_walk [] =
{
	drone_ai_walk, 15, NULL,
	drone_ai_walk, 7,  NULL,
	drone_ai_walk, 6,  NULL,
	drone_ai_walk, 5,  NULL,
	drone_ai_walk, 2,  NULL,
	drone_ai_walk, 0,  NULL,
	drone_ai_walk, 2,  NULL,
	drone_ai_walk, 8,  NULL,
	drone_ai_walk, 12, NULL,
	drone_ai_walk, 8,  NULL,
	drone_ai_walk, 5,  NULL,
	drone_ai_walk, 5,  NULL,
	drone_ai_walk, 2,  NULL,
	drone_ai_walk, 2,  NULL,
	drone_ai_walk, 1,  NULL,
	drone_ai_walk, 8,  NULL
};
mmove_t gladiator_move_walk = {FRAME_walk1, FRAME_walk16, gladiator_frames_walk, NULL};

void gladiator_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &gladiator_move_walk;
}

mframe_t gladiator_frames_run [] =
{
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,	NULL,
	drone_ai_run, 25,	NULL
};
mmove_t gladiator_move_run = {FRAME_run1, FRAME_run6, gladiator_frames_run, NULL};

void gladiator_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gladiator_move_stand;
	else
		self->monsterinfo.currentmove = &gladiator_move_run;
}

void SpawnLightningStorm (edict_t *ent, vec3_t start, float radius, int duration, int damage);

void GladiatorLightningStorm (edict_t *self)
{
	int damage;

	if (!G_EntExists(self->enemy))
		return;

	//slvl = self->monsterinfo.level;

	//if (slvl > 15)
	//	slvl = 15;
	damage = 200 + 5 * self->monsterinfo.level;
	// damage = 250;//50 + 15 * slvl;
	SpawnLightningStorm(self, self->enemy->s.origin, 128, 5.0, damage);

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (self->enemy->s.origin);
	gi.multicast (self->enemy->s.origin, MULTICAST_PVS);

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/eleccast.wav"), 1, ATTN_NORM, 0);
}

void GaldiatorMelee (edict_t *self)
{
	int		damage = 55 + 8 * self->monsterinfo.level; // No one-shots, until later.
	vec3_t	aim;

	if (self->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
	{
		GladiatorLightningStorm(self);
		return;
	}

	if (!G_EntExists(self->enemy))
		return;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], -4);
	if (M_MeleeAttack(self, 96, damage, 200))
		gi.sound (self, CHAN_AUTO, sound_cleaver_hit, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_cleaver_miss, 1, ATTN_NORM, 0);
}

void gladiator_cleaver_refire (edict_t *self)
{
	if (self->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
		return;

	if (G_ValidTarget(self, self->enemy, true) 
		&& (entdist(self, self->enemy) <= 96) && (random() <= 0.8))
	{
		self->s.frame = 41;
		gladiator_cleaver_swing(self);
		return;
	}

	self->monsterinfo.melee_finished = level.time + GetRandom(5, 10)*FRAMETIME;
}

mframe_t gladiator_frames_attack_melee [] =
{
	ai_charge, 0, gladiator_cleaver_swing,	//41
	ai_charge, 0, NULL,
	ai_charge, 0, GaldiatorMelee,			//43
	ai_charge, 0, NULL,
	ai_charge, 0, gladiator_cleaver_refire	//45
};
mmove_t gladiator_move_attack_melee = {FRAME_melee13, FRAME_melee17, gladiator_frames_attack_melee, gladiator_run};

void gladiator_melee(edict_t *self)
{

}


void ChainLightning (edict_t *ent, vec3_t start, vec3_t aimdir, int damage, int attack_range, int hop_range);

void GladiatorChainLightning (edict_t *self)
{
	int slvl, damage;
	vec3_t	forward, start;

	if (!G_EntExists(self->enemy))
		return;

	slvl = self->monsterinfo.level;

	if (slvl > 15)
		slvl = 15;

	damage = 50 + 15 * slvl;

	MonsterAim(self, 0.5, 0, false, MZ2_GLADIATOR_RAILGUN_1, forward, start);
	ChainLightning(self, start, forward, damage, 1024, 256);
}

void GladiatorGun (edict_t *self)
{
	int damage;
	vec3_t	forward, start;

	if (self->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
	{
		GladiatorChainLightning(self);
		return;
	}
	
	if (!G_EntExists(self->enemy))
		return;

	damage = 55 + 6 * self->monsterinfo.level; // from 50. and 15.

	MonsterAim(self, 0.7, 0, false, MZ2_GLADIATOR_RAILGUN_1, forward, start);

	monster_fire_railgun (self, start, forward, damage, 100, MZ2_GLADIATOR_RAILGUN_1);
}

void gladiator_refire (edict_t *self)
{
	// if our enemy is still valid, then continue firing
	if (G_ValidTarget(self, self->enemy, true) && (random() <= 0.8))
	{
		self->s.frame = 49;
		GladiatorGun(self);
		return;
	}

	self->monsterinfo.attack_finished = level.time + 1.0;
}

mframe_t gladiator_frames_attack_gun [] =
{
	ai_charge, 0, GladiatorGun,		//49
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, gladiator_refire,	//53
};
mmove_t gladiator_move_attack_gun = {FRAME_attack4, FRAME_attack8, gladiator_frames_attack_gun, gladiator_run};

void gladiator_lightning_attack (edict_t *self)
{
	float r = random();

	// medium range (30% chance LS, 70% chance CL)
	if (entdist(self, self->enemy) <= 512)
	{
		if (r <= 0.3)
			gladiator_melee(self);
		else
			self->monsterinfo.currentmove = &gladiator_move_attack_gun;
	}
	// long range (50% chance LS/CL)
	else
	{
		if (r > 0.5)
			gladiator_melee(self);
		else
			self->monsterinfo.currentmove = &gladiator_move_attack_gun;
	}

	self->monsterinfo.attack_finished = level.time + 1.0;
}

void gladiator_attack(edict_t *self)
{
	if (self->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
	{
		gladiator_lightning_attack(self);
		return;
	}

	self->monsterinfo.attack_finished = level.time + 0.5 + random();

	if ((entdist(self, self->enemy) <= 96) && (random() <= 0.6))
	{
		self->monsterinfo.currentmove = &gladiator_move_attack_melee;
		return;
	}

	// charge up the railgun
	gi.sound (self, CHAN_WEAPON, sound_gun, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &gladiator_move_attack_gun;
}

void gladiator_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	gladiator_attack(self);
}

void gladiator_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t gladiator_frames_death [] =
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
	ai_move, 0, NULL
};
mmove_t gladiator_move_death = {FRAME_death1, FRAME_death22, gladiator_frames_death, gladiator_dead};

void gladiator_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

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

	// begin death sequence
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &gladiator_move_death;

	DroneList_Remove(self);

	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

void init_drone_gladiator (edict_t *self)
{
	sound_die = gi.soundindex ("gladiator/glddeth2.wav");	
	sound_gun = gi.soundindex ("gladiator/railgun.wav");
	sound_cleaver_swing = gi.soundindex ("gladiator/melee1.wav");
	sound_cleaver_hit = gi.soundindex ("gladiator/melee2.wav");
	sound_cleaver_miss = gi.soundindex ("gladiator/melee3.wav");
	sound_idle = gi.soundindex ("gladiator/gldidle1.wav");
	sound_search = gi.soundindex ("gladiator/gldsrch1.wav");
	sound_sight = gi.soundindex ("gladiator/sight.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/gladiatr/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 48);
	self->health = 120 + 75*self->monsterinfo.level;
	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 400;
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = 100 + 40*self->monsterinfo.level;
	self->monsterinfo.max_armor = self->monsterinfo.power_armor_power;
	self->mtype = M_GLADIATOR;
	self->item = FindItemByClassname("ammo_slugs");
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	self->monsterinfo.control_cost = M_GLADIATOR_CONTROL_COST;

	self->die = gladiator_die;
	self->monsterinfo.stand = gladiator_stand;
	self->monsterinfo.run = gladiator_run;
	self->monsterinfo.attack = gladiator_attack;
	self->monsterinfo.melee = gladiator_melee;
	self->monsterinfo.sight = gladiator_sight;
	self->monsterinfo.idle = gladiator_idle;
	self->monsterinfo.walk = gladiator_walk;

	gi.linkentity (self);
	self->monsterinfo.currentmove = &gladiator_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
}
