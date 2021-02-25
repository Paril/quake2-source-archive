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

BERSERK

==============================================================================
*/

#include "g_local.h"
#include "m_berserk.h"

static soundindex_t sound_pain;
static soundindex_t sound_die;
static soundindex_t sound_idle;
static soundindex_t sound_search;

static void berserk_fidget (edict_t &self);

static const mmove_t berserk_move_stand = {
	.firstframe = FRAME_stand1,
	.lastframe = FRAME_stand5,
	.frame = {
		{ .aifunc = ai_stand, .thinkfunc = berserk_fidget },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	}
};

static void berserk_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &berserk_move_stand;
}

static void berserk_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE);
}

static const mmove_t berserk_move_stand_fidget = {
	.firstframe = FRAME_standb1,
	.lastframe = FRAME_standb20,
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
		{ ai_stand }
	},
	.endfunc = berserk_stand
};

static void berserk_fidget (edict_t &self)
{
	if (!M_FidgetCheck(self, 15))
		return;

	self.monsterinfo.currentmove = &berserk_move_stand_fidget;
	self.PlaySound(sound_idle, CHAN_WEAPON, ATTN_IDLE);
}

static const mmove_t berserk_move_walk = {
	.firstframe = FRAME_walkc1,
	.lastframe = FRAME_walkc11,
	.frame = {
		{ ai_walk, 9.1 },
		{ ai_walk, 6.3 },
		{ ai_walk, 4.9 },
		{ ai_walk, 6.7 },
		{ ai_walk, 6.0 },
		{ ai_walk, 8.2 },
		{ ai_walk, 7.2 },
		{ ai_walk, 6.1 },
		{ ai_walk, 4.9 },
		{ ai_walk, 4.7 },
		{ ai_walk, 4.7 },
		{ ai_walk, 4.8 }
	}
};

static void berserk_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &berserk_move_walk;
}

static const mmove_t berserk_move_run1 = {
	.firstframe = FRAME_run1,
	.lastframe = FRAME_run6,
	.frame = {
		{ ai_run, 21 },
		{ ai_run, 11 },
		{ ai_run, 21 },
		{ ai_run, 25 },
		{ ai_run, 18 },
		{ ai_run, 19 }
	}
};

static void berserk_run (edict_t &self)
{
	self.monsterinfo.currentmove = &berserk_move_run1;
}

static void berserk_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;
	self.PlaySound(sound_pain, CHAN_VOICE);
}

static const mmove_t berserk_move_death1 = {
	.firstframe = FRAME_death1,
	.lastframe = FRAME_death13,
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
		{ ai_move }
	}
};

static const mmove_t berserk_move_death2 = {
	.firstframe = FRAME_deathc1,
	.lastframe = FRAME_deathc8,
	.frame = {
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

static void berserk_die (edict_t &self)
{
	self.PlaySound(sound_die, CHAN_VOICE);

	if (prandom(50))
		self.monsterinfo.currentmove = &berserk_move_death1;
	else
		self.monsterinfo.currentmove = &berserk_move_death2;
}

/*QUAKED monster_berserk (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_berserk (edict_t &self)
{
	// pre-caches
	sound_pain  = gi.soundindex ("berserk/berpain2.wav");
	sound_die   = gi.soundindex ("berserk/berdeth2.wav");
	sound_idle  = gi.soundindex ("berserk/beridle1.wav");
	sound_search = gi.soundindex ("berserk/bersrch1.wav");

	self.s.modelindex = gi.modelindex("models/monsters/berserk/tris.md2");
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 32.f };
	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;

	self.health = 240;
	self.mass = 250;

	self.pain = berserk_pain;

	self.monsterinfo.stand = berserk_stand;
	self.monsterinfo.walk = berserk_walk;
	self.monsterinfo.run = berserk_run;
	self.monsterinfo.search = berserk_search;
	self.monsterinfo.die = berserk_die;

	self.monsterinfo.currentmove = &berserk_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	self.Link();

	walkmonster_start (self);
}
