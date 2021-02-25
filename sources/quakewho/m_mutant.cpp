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

mutant

==============================================================================
*/

#include "g_local.h"
#include "m_mutant.h"


static soundindex_t	sound_death;
static soundindex_t	sound_idle;
static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_search;
static soundindex_t	sound_step1;
static soundindex_t	sound_step2;
static soundindex_t	sound_step3;

//
// SOUNDS
//

static void mutant_step (edict_t &self)
{
	const int32_t n = irandom(2);
	if (n == 0)
		self.PlaySound(sound_step1, CHAN_BODY);
	else if (n == 1)
		self.PlaySound(sound_step2, CHAN_BODY);
	else
		self.PlaySound(sound_step3, CHAN_BODY);
}

static void mutant_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE);
}

//
// STAND
//
static const mmove_t mutant_move_stand = {
	.firstframe = FRAME_stand101,
	.lastframe = FRAME_stand151,
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
		{ ai_stand },		// 10

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },		// 20

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },		// 30

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },		// 40

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },		// 50

		{ ai_stand }
	}
};

static void mutant_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &mutant_move_stand;
}

//
// IDLE
//
static void mutant_idle_loop (edict_t &self)
{
	if (prandom(75))
		self.monsterinfo.nextframe = FRAME_stand155;
}

static const mmove_t mutant_move_idle = {
	.firstframe = FRAME_stand152,
	.lastframe = FRAME_stand164,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },					// scratch loop start
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = mutant_idle_loop },		// scratch loop end
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = mutant_stand
};

static void mutant_idle (edict_t &self)
{
	self.monsterinfo.currentmove = &mutant_move_idle;
	self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

//
// WALK
//
static const mmove_t mutant_move_walk = {
	.firstframe = FRAME_walk05,
	.lastframe = FRAME_walk16,
	.frame = {
		{ ai_walk, 3 },
		{ ai_walk, 1 },
		{ ai_walk, 5 },
		{ ai_walk, 10 },
		{ ai_walk, 13 },
		{ ai_walk, 10 },
		{ ai_walk },
		{ ai_walk, 5 },
		{ ai_walk, 6 },
		{ ai_walk, 16 },
		{ ai_walk, 15 },
		{ ai_walk, 6 }
	}
};

static void mutant_walk_loop (edict_t &self)
{
	self.monsterinfo.currentmove = &mutant_move_walk;
}

static const mmove_t mutant_move_start_walk = {
	.firstframe = FRAME_walk01,
	.lastframe = FRAME_walk04,
	.frame = {
		{ ai_walk, 5 },
		{ ai_walk, 5 },
		{ ai_walk, -2 },
		{ ai_walk, 1 }
	},
	.endfunc = mutant_walk_loop
};

static void mutant_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &mutant_move_start_walk;
}

//
// RUN
//
static const mmove_t mutant_move_run = {
	.firstframe = FRAME_run03,
	.lastframe = FRAME_run08,
	.frame = {
		{ ai_run, 40 },
		{ ai_run, 40, mutant_step },
		{ ai_run, 24 },
		{ ai_run, 5, mutant_step },
		{ ai_run, 17 },
		{ ai_run, 10 }
	}
};

static void mutant_run (edict_t &self)
{
	self.monsterinfo.currentmove = &mutant_move_run;
}

//
// PAIN
//
static void mutant_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	const int32_t r = irandom(2);
	if (r == 0)
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else if (r == 1)
		self.PlaySound(sound_pain2, CHAN_VOICE);
	else
		self.PlaySound(sound_pain1, CHAN_VOICE);
}

//
// DEATH
//
static const mmove_t mutant_move_death1 = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death109,
	.frame = {
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

static const mmove_t mutant_move_death2 = {
	.firstframe = FRAME_death201,
	.lastframe = FRAME_death210,
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

static void mutant_die (edict_t &self)
{
	self.PlaySound(sound_death, CHAN_VOICE);

	if (prandom(50))
		self.monsterinfo.currentmove = &mutant_move_death1;
	else
		self.monsterinfo.currentmove = &mutant_move_death2;
}


//
// SPAWN
//

/*QUAKED monster_mutant (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_mutant (edict_t &self)
{
	sound_death = gi.soundindex ("mutant/mutdeth1.wav");
	sound_idle = gi.soundindex ("mutant/mutidle1.wav");
	sound_pain1 = gi.soundindex ("mutant/mutpain1.wav");
	sound_pain2 = gi.soundindex ("mutant/mutpain2.wav");
	sound_search = gi.soundindex ("mutant/mutsrch1.wav");
	sound_step1 = gi.soundindex ("mutant/step1.wav");
	sound_step2 = gi.soundindex ("mutant/step2.wav");
	sound_step3 = gi.soundindex ("mutant/step3.wav");
	
	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/mutant/tris.md2");
	self.mins = { -32.f, -32.f, -24.f };
	self.maxs = { 32.f, 32.f, 48.f };

	self.health = 300;
	self.mass = 300;

	self.pain = mutant_pain;

	self.monsterinfo.stand = mutant_stand;
	self.monsterinfo.walk = mutant_walk;
	self.monsterinfo.run = mutant_run;
	self.monsterinfo.search = mutant_search;
	self.monsterinfo.idle = mutant_idle;
	self.monsterinfo.die = mutant_die;

	self.Link();
	
	self.monsterinfo.currentmove = &mutant_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
