/*
==============================================================================

Makron -- Final Boss

==============================================================================
*/

#include "g_local.h"
#include "m_boss32.h"

qboolean visible (edict_t *self, edict_t *other);

void BossMakronRailgun (edict_t *self);
void BossMakronSaveloc (edict_t *self);
void BossMakronHyperblaster (edict_t *self);
void boss_makron_step_left (edict_t *self);
void boss_makron_step_right (edict_t *self);
void bossmakronBFG (edict_t *self);
void boss_makron_dead (edict_t *self);

static int	sound_pain4;
static int	sound_pain5;
static int	sound_pain6;
static int	sound_death;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_attack_bfg;
static int	sound_brainsplorch;
static int	sound_prerailgun;
static int	sound_popup;
static int	sound_taunt1;
static int	sound_taunt2;
static int	sound_taunt3;
static int	sound_hit;

void boss_makron_taunt (edict_t *self)
{
	float r;

	r=random();
	if (r <= 0.3)
		gi.sound (self, CHAN_AUTO, sound_taunt1, 1, ATTN_NONE, 0);
	else if (r <= 0.6)
		gi.sound (self, CHAN_AUTO, sound_taunt2, 1, ATTN_NONE, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_taunt3, 1, ATTN_NONE, 0);
}

//
// stand
//

mframe_t boss_makron_frames_stand []=
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
	ai_stand, 0, NULL,		// 10
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,		// 20
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,		// 30
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,		// 40
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,		// 50
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL		// 60
};
mmove_t	boss_makron_move_stand = {FRAME_stand201, FRAME_stand260, boss_makron_frames_stand, NULL};
	
void boss_makron_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &boss_makron_move_stand;
}

mframe_t boss_makron_frames_run [] =
{
	ai_run, 20,	boss_makron_step_left,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL,
	ai_run, 20,	boss_makron_step_right,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL,
	ai_run, 20,	NULL
};
mmove_t	boss_makron_move_run = {FRAME_walk204, FRAME_walk213, boss_makron_frames_run, NULL};

void boss_makron_hit (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_hit, 1, ATTN_NONE,0);
}

void boss_makron_popup (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_popup, 1, ATTN_NONE,0);
}

void boss_makron_step_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
}

void boss_makron_step_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
}

void boss_makron_brainsplorch (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_brainsplorch, 1, ATTN_NORM,0);
}

void boss_makron_prerailgun (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_prerailgun, 1, ATTN_NORM,0);
}


mframe_t boss_makron_frames_walk [] =
{
	ai_walk, 20,	boss_makron_step_left,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL,
	ai_walk, 20,	boss_makron_step_right,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL,
	ai_walk, 20,	NULL
};
mmove_t	boss_makron_move_walk = {FRAME_walk204, FRAME_walk213, boss_makron_frames_run, NULL};

void boss_makron_walk (edict_t *self)
{
		self->monsterinfo.currentmove = &boss_makron_move_walk;
}

void boss_makron_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &boss_makron_move_stand;
	else
		self->monsterinfo.currentmove = &boss_makron_move_run;
}

mframe_t boss_makron_frames_pain6 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,		// 10
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	boss_makron_popup,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,		// 20
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	boss_makron_taunt,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_pain6 = {FRAME_pain601, FRAME_pain627, boss_makron_frames_pain6, boss_makron_run};

mframe_t boss_makron_frames_pain5 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_pain5 = {FRAME_pain501, FRAME_pain504, boss_makron_frames_pain5, boss_makron_run};

mframe_t boss_makron_frames_pain4 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_pain4 = {FRAME_pain401, FRAME_pain404, boss_makron_frames_pain4, boss_makron_run};

mframe_t boss_makron_frames_death2 [] =
{
	ai_move,	-15,	NULL,
	ai_move,	3,	NULL,
	ai_move,	-12,	NULL,
	ai_move,	0,	boss_makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 10
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	11,	NULL,
	ai_move,	12,	NULL,
	ai_move,	11,	boss_makron_step_right,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 20
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 30
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	5,	NULL,
	ai_move,	7,	NULL,
	ai_move,	6,	boss_makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-1,	NULL,
	ai_move,	2,	NULL,			// 40
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,			// 50
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-6,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	-6,	boss_makron_step_right,
	ai_move,	-4,	NULL,
	ai_move,	-4,	boss_makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 60
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	-5,	NULL,
	ai_move,	-3,	boss_makron_step_right,
	ai_move,	-8,	NULL,
	ai_move,	-3,	boss_makron_step_left,
	ai_move,	-7,	NULL,
	ai_move,	-4,	NULL,
	ai_move,	-4,	boss_makron_step_right,			// 70
	ai_move,	-6,	NULL,			
	ai_move,	-7,	NULL,
	ai_move,	0,	boss_makron_step_left,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,			// 80
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	2,	NULL,
	ai_move,	0,	NULL,			// 90
	ai_move,	27,	boss_makron_hit,			
	ai_move,	26,	NULL,
	ai_move,	0,	boss_makron_brainsplorch,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL			// 95
};
mmove_t boss_makron_move_death2 = {FRAME_death201, FRAME_death295, boss_makron_frames_death2, boss_makron_dead};

mframe_t boss_makron_frames_death3 [] =
{
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
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_death3 = {FRAME_death301, FRAME_death320, boss_makron_frames_death3, NULL};

mframe_t boss_makron_frames_sight [] =
{
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
mmove_t boss_makron_move_sight= {FRAME_active01, FRAME_active13, boss_makron_frames_sight, boss_makron_run};

void bossmakronBFG (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_BFG], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_VOICE, sound_attack_bfg, 1, ATTN_NORM, 0);
	monster_fire_bfg (self, start, dir, 400, 700, 300, 600, MZ2_MAKRON_BFG);
}	


mframe_t boss_makron_frames_attack3 []=
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	bossmakronBFG,		// FIXME: BFG Attack here
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_attack3 = {FRAME_attak301, FRAME_attak308, boss_makron_frames_attack3, boss_makron_run};

mframe_t boss_makron_frames_attack4[]=
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	BossMakronHyperblaster,		// fire
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_attack4 = {FRAME_attak401, FRAME_attak426, boss_makron_frames_attack4, boss_makron_run};

mframe_t boss_makron_frames_attack5[]=
{
	ai_charge,	0,	boss_makron_prerailgun,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	BossMakronSaveloc,
	ai_move,	0,	BossMakronRailgun,		// Fire railgun
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t boss_makron_move_attack5 = {FRAME_attak501, FRAME_attak516, boss_makron_frames_attack5, boss_makron_run};

void BossMakronSaveloc (edict_t *self)
{
	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;
};

// FIXME: He's not firing from the proper Z
void BossMakronRailgun (edict_t *self)
{
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;

	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_RAILGUN_1], forward, right, start);
	
	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	monster_fire_railgun (self, start, dir, 315, 400, MZ2_MAKRON_RAILGUN_1);
}

// FIXME: This is all wrong. He's not firing at the proper angles.
void BossMakronHyperblaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	vec;
	vec3_t	dir;
	int		flash_number;

//	if (self->s.frame == FRAME_attak111)
//	{
		flash_number = MZ2_MAKRON_BLASTER_1;
/*	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, vec);
		vectoangles (vec, vec);
		dir[0] = vec[0];
	}
	else
	{
		dir[0] = 0;
	}
	//if (self->s.frame <= FRAME_attak415)
		//dir[1] = self->s.angles[1] - 8 * (self->s.frame - FRAME_attak411);
	//else
		//dir[1] = self->s.angles[1] + 8 * (self->s.frame - FRAME_attak419);
	dir[0] = 1;
	dir[2] = 0;

	AngleVectors (dir, forward, NULL, NULL);*/
	//FIXME!!!
	dir[0] = 0;
	dir[1] = self->s.angles[1];
	dir[2] = 0;

	AngleVectors (dir, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		VectorMA (vec, 0, self->enemy->velocity, vec);
		vec[2] += (self->enemy->viewheight - 36);
		VectorSubtract (vec, start, forward);
		start[2] += 36;
		VectorNormalize (forward);
	}

	monster_fire_blaster (self, start, forward, 50, 1400, MZ2_MAKRON_BLASTER_1, EF_BLASTER);
}	


void boss_makron_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames
	if (damage <=25)
		if (random()<0.2)
			return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare


	if (damage <= 40)
	{
		gi.sound (self, CHAN_VOICE, sound_pain4, 1, ATTN_NONE,0);
		self->monsterinfo.currentmove = &boss_makron_move_pain4;
	}
	else if (damage <= 110)
	{
		gi.sound (self, CHAN_VOICE, sound_pain5, 1, ATTN_NONE,0);
		self->monsterinfo.currentmove = &boss_makron_move_pain5;
	}
	else
	{
		if (damage <= 150)
			if (random() <= 0.45)
			{
				gi.sound (self, CHAN_VOICE, sound_pain6, 1, ATTN_NONE,0);
				self->monsterinfo.currentmove = &boss_makron_move_pain6;
			}
		else
			if (random() <= 0.35)
			{
				gi.sound (self, CHAN_VOICE, sound_pain6, 1, ATTN_NONE,0);
				self->monsterinfo.currentmove = &boss_makron_move_pain6;
			}
	}
};

void boss_makron_sight(edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = &boss_makron_move_sight;
};

void boss_makron_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	float	r;

	r = random();

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);


	if (r <= 0.3)
		self->monsterinfo.currentmove = &boss_makron_move_attack3;
	else if (r <= 0.6)
		self->monsterinfo.currentmove = &boss_makron_move_attack4;
	else
		self->monsterinfo.currentmove = &boss_makron_move_attack5;
}

/*
---
Makron Torso. This needs to be spawned in
---
*/

void boss_makron_torso_think (edict_t *self)
{
	if (++self->s.frame < 365)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 346;
		self->nextthink = level.time + FRAMETIME;
	}
}

void boss_makron_torso (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->s.frame = 346;
	ent->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	ent->think = boss_makron_torso_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	ent->s.sound = gi.soundindex ("makron/spine.wav");
	gi.linkentity (ent);
}


//
// death
//

void boss_makron_dead (edict_t *self)
{
	VectorSet (self->mins, -60, -60, 0);
	VectorSet (self->maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void boss_makron_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_Spawn_Explosion(TE_EXPLOSION2, self->s.origin, self->s.origin);
	G_FreeEdict(self);
	if (((int)(stroggflags->value) & SF_RPG_MODE) && attacker->client)
	{
		AddExp(attacker, rndnum (6000, 10000));
		Check_Levelup(attacker);
	}
/*	edict_t *tempent;

	int		n;

	self->s.sound = 0;
	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2");
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2");
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->s.origin[1] -= 84;
	boss_makron_torso (tempent);

	self->monsterinfo.currentmove = &boss_makron_move_death2;*/
	
}

qboolean boss_makron_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	vec3_t	temp;
	float	chance;
	trace_t	tr;
	qboolean	enemy_infront;
	int			enemy_range;
	float		enemy_yaw;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);

	self->ideal_yaw = enemy_yaw;


	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
// missile attack
	if (!self->monsterinfo.attack)
		return false;
		
	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (enemy_range == RANGE_FAR)
		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.2;
	}
	else
	{
		return false;
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2*random();
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}


//
// monster_makron
//

void BossMakronPrecache (void)
{
	sound_pain4 = gi.soundindex ("makron/pain3.wav");
	sound_pain5 = gi.soundindex ("makron/pain2.wav");
	sound_pain6 = gi.soundindex ("makron/pain1.wav");
	sound_death = gi.soundindex ("makron/death.wav");
	sound_step_left = gi.soundindex ("makron/step1.wav");
	sound_step_right = gi.soundindex ("makron/step2.wav");
	sound_attack_bfg = gi.soundindex ("makron/bfg_fire.wav");
	sound_brainsplorch = gi.soundindex ("makron/brain1.wav");
	sound_prerailgun = gi.soundindex ("makron/rail_up.wav");
	sound_popup = gi.soundindex ("makron/popup.wav");
	sound_taunt1 = gi.soundindex ("makron/voice4.wav");
	sound_taunt2 = gi.soundindex ("makron/voice3.wav");
	sound_taunt3 = gi.soundindex ("makron/voice.wav");
	sound_hit = gi.soundindex ("makron/bhit.wav");

	gi.modelindex ("players/rider/tris.md2");
}

/*QUAKED monster_boss_makron (1 .5 0) (-30 -30 0) (30 30 90) Ambush Trigger_Spawn Sight
*/
void SP_monster_boss_makron (edict_t *self)
{
	/*if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}*/

	BossMakronPrecache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	VectorSet (self->mins, -30, -30, 0);
	VectorSet (self->maxs, 30, 30, 90);

	self->health = 55000;
	self->gib_health = -2000;
	self->mass = 3000;

	self->pain = boss_makron_pain;
	self->die = boss_makron_die;
	self->ismonster = 1;
	self->s.effects |= EF_HALF_DAMAGE;

	self->monsterinfo.stand = boss_makron_stand;
	self->monsterinfo.walk = boss_makron_walk;
	self->monsterinfo.run = boss_makron_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = boss_makron_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = boss_makron_sight;
	self->monsterinfo.checkattack = boss_makron_CheckAttack;

	gi.linkentity (self);
	
//	self->monsterinfo.currentmove = &boss_makron_move_stand;
	self->monsterinfo.currentmove = &boss_makron_move_sight;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
