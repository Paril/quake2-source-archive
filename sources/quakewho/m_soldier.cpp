/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
==============================================================================

SOLDIER

==============================================================================
*/

#include "g_local.h"
#include "m_soldier.h"


static soundindex_t	sound_idle;
static soundindex_t	sound_pain_light;
static soundindex_t	sound_pain;
static soundindex_t	sound_pain_ss;
static soundindex_t	sound_death_light;
static soundindex_t	sound_death;
static soundindex_t	sound_death_ss;
static soundindex_t	sound_cock;

static void soldier_idle (edict_t &self)
{
	if (M_FidgetCheck(self, 20))
		self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

static void soldier_cock (edict_t &self)
{
	self.PlaySound(sound_cock, CHAN_WEAPON, ATTN_IDLE);
}

// STAND

static void soldier_stand (edict_t &self);

static const mmove_t soldier_move_stand1 = {
	.firstframe = FRAME_stand101,
	.lastframe = FRAME_stand130,
	.frame = {
		{ .aifunc = ai_stand, .thinkfunc = soldier_idle },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = soldier_stand
};

static const mmove_t soldier_move_stand3 = {
	.firstframe = FRAME_stand301,
	.lastframe = FRAME_stand339,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },

		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = soldier_cock },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = soldier_stand
};

static void soldier_stand (edict_t &self)
{
	if ((self.monsterinfo.currentmove == &soldier_move_stand3) || prandom(80))
		self.monsterinfo.currentmove = &soldier_move_stand1;
	else
		self.monsterinfo.currentmove = &soldier_move_stand3;
}

//
// WALK
//

static void soldier_walk1_random (edict_t &self)
{
	if (prandom(90))
		self.monsterinfo.nextframe = FRAME_walk101;
}

static const mmove_t soldier_move_walk1 = {
	.firstframe = FRAME_walk101,
	.lastframe = FRAME_walk133,
	.frame = {
		{ ai_walk, 3 },
		{ ai_walk, 6 },
		{ ai_walk, 2 },
		{ ai_walk, 2 },
		{ ai_walk, 2 },
		{ ai_walk, 1 },
		{ ai_walk, 6 },
		{ ai_walk, 5 },
		{ ai_walk, 3 },
		{ ai_walk, -1, soldier_walk1_random },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk }
	}
};

static const mmove_t soldier_move_walk2 = {
	.firstframe = FRAME_walk209,
	.lastframe = FRAME_walk218,
	.frame = {
		{ ai_walk, 4 },
		{ ai_walk, 4 },
		{ ai_walk, 9 },
		{ ai_walk, 8 },
		{ ai_walk, 5 },
		{ ai_walk, 1 },
		{ ai_walk, 3 },
		{ ai_walk, 7 },
		{ ai_walk, 6 },
		{ ai_walk, 7 }
	}
};

static void soldier_walk (edict_t &self)
{
	if (prandom(50))
		self.monsterinfo.currentmove = &soldier_move_walk1;
	else
		self.monsterinfo.currentmove = &soldier_move_walk2;
}


//
// RUN
//

static void soldier_run (edict_t &self);

static const mmove_t soldier_move_start_run = {
	.firstframe = FRAME_run01,
	.lastframe = FRAME_run02,
	.frame = {
		{ ai_run, 7 },
		{ ai_run, 5 }
	},
	.endfunc = soldier_run
};

static const mmove_t soldier_move_run = {
	.firstframe = FRAME_run03,
	.lastframe = FRAME_run08,
	.frame = {
		{ ai_run, 10 },
		{ ai_run, 11 },
		{ ai_run, 11 },
		{ ai_run, 16 },
		{ ai_run, 10 },
		{ ai_run, 15 }
	}
};

static void soldier_run (edict_t &self)
{
	if (self.monsterinfo.currentmove == &soldier_move_walk1 ||
		self.monsterinfo.currentmove == &soldier_move_walk2 ||
		self.monsterinfo.currentmove == &soldier_move_start_run)
		self.monsterinfo.currentmove = &soldier_move_run;
	else
		self.monsterinfo.currentmove = &soldier_move_start_run;
}


//
// PAIN
//
static void soldier_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	const int32_t n = self.s.skinnum | 1;
	if (n == 1)
		self.PlaySound(sound_pain_light, CHAN_VOICE);
	else if (n == 3)
		self.PlaySound(sound_pain, CHAN_VOICE);
	else
		self.PlaySound(sound_pain_ss, CHAN_VOICE);
}

//
// DEATH
//
static const mmove_t soldier_move_death2 = {
	.firstframe = FRAME_death201,
	.lastframe = FRAME_death235,
	.frame = {
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static const mmove_t soldier_move_death3 = {
	.firstframe = FRAME_death301,
	.lastframe = FRAME_death345,
	.frame = {
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static const mmove_t soldier_move_death4 = {
	.firstframe = FRAME_death401,
	.lastframe = FRAME_death453,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static const mmove_t soldier_move_death5 = {
	.firstframe = FRAME_death501,
	.lastframe = FRAME_death524,
	.frame = {
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move, -5 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },

		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static const mmove_t soldier_move_death6 = {
	.firstframe = FRAME_death601,
	.lastframe = FRAME_death610,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static void soldier_die (edict_t &self)
{
	if (self.s.skinnum <= 1)
		self.PlaySound(sound_death_light, CHAN_VOICE);
	else if (self.s.skinnum <= 3)
		self.PlaySound(sound_death, CHAN_VOICE);
	else
		self.PlaySound(sound_death_ss, CHAN_VOICE);

	int32_t n = irandom(4);
	if (n == 0)
		self.monsterinfo.currentmove = &soldier_move_death2;
	else if (n == 1)
		self.monsterinfo.currentmove = &soldier_move_death3;
	else if (n == 2)
		self.monsterinfo.currentmove = &soldier_move_death4;
	else if (n == 3)
		self.monsterinfo.currentmove = &soldier_move_death5;
	else
		self.monsterinfo.currentmove = &soldier_move_death6;
}

//
// SPAWN
//
static void SP_monster_soldier_x (edict_t &self)
{
	self.s.modelindex = gi.modelindex ("models/monsters/soldier/tris.md2");
	self.monsterinfo.scale = MODEL_SCALE;
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 32.f };
	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;

	sound_idle = gi.soundindex ("soldier/solidle1.wav");
	sound_cock = gi.soundindex ("infantry/infatck3.wav");

	self.mass = 100;

	self.pain = soldier_pain;

	self.monsterinfo.stand = soldier_stand;
	self.monsterinfo.walk = soldier_walk;
	self.monsterinfo.run = soldier_run;
	self.monsterinfo.die = soldier_die;

	self.Link();

	self.monsterinfo.stand (self);

	walkmonster_start (self);
}


/*QUAKED monster_soldier_light (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_light (edict_t &self)
{
	sound_pain_light = gi.soundindex ("soldier/solpain2.wav");
	sound_death_light =	gi.soundindex ("soldier/soldeth2.wav");
	gi.soundindex ("misc/lasfly.wav");

	self.s.skinnum = 0;
	self.health = 20;
	self.monsterinfo.undamaged_skin = 0;
	self.monsterinfo.damaged_skin = 1;

	SP_monster_soldier_x (self);
}

/*QUAKED monster_soldier (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier (edict_t &self)
{
	sound_pain = gi.soundindex ("soldier/solpain1.wav");
	sound_death = gi.soundindex ("soldier/soldeth1.wav");

	self.s.skinnum = 2;
	self.health = 30;
	self.monsterinfo.undamaged_skin = 2;
	self.monsterinfo.damaged_skin = 3;

	SP_monster_soldier_x (self);
}

/*QUAKED monster_soldier_ss (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_ss (edict_t &self)
{
	sound_pain_ss = gi.soundindex ("soldier/solpain3.wav");
	sound_death_ss = gi.soundindex ("soldier/soldeth3.wav");

	self.s.skinnum = 4;
	self.health = 40;
	self.monsterinfo.undamaged_skin = 4;
	self.monsterinfo.damaged_skin = 5;

	SP_monster_soldier_x (self);
}
