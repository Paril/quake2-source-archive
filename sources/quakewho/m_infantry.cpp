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

INFANTRY

==============================================================================
*/

#include "g_local.h"
#include "m_infantry.h"

static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_die1;
static soundindex_t	sound_die2;
static soundindex_t	sound_search;
static soundindex_t	sound_idle;

static const mmove_t infantry_move_stand = {
	.firstframe = FRAME_stand50,
	.lastframe = FRAME_stand71,
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
		{ ai_stand }
	}
};

static void infantry_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &infantry_move_stand;
}

static const mmove_t infantry_move_fidget = {
	.firstframe = FRAME_stand01,
	.lastframe = FRAME_stand49,
	.frame = {
		{ ai_stand, 1 },
		{ ai_stand },
		{ ai_stand, 1 },
		{ ai_stand, 3 },
		{ ai_stand, 6 },
		{ ai_stand, 3 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, 1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, 1 },
		{ ai_stand },
		{ ai_stand, -1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, 1 },
		{ ai_stand },
		{ ai_stand, -2 },
		{ ai_stand, 1 },
		{ ai_stand, 1 },
		{ ai_stand, 1 },
		{ ai_stand, -1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, -1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, -1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, 1 },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand, -1 },
		{ ai_stand, -1 },
		{ ai_stand },
		{ ai_stand, -3 },
		{ ai_stand, -2 },
		{ ai_stand, -3 },
		{ ai_stand, -3 },
		{ ai_stand, -2 }
	},
	.endfunc = infantry_stand
};

static void infantry_fidget (edict_t &self)
{
	self.monsterinfo.currentmove = &infantry_move_fidget;
	self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

static const mmove_t infantry_move_walk = {
	.firstframe = FRAME_walk03,
	.lastframe = FRAME_walk14,
	.frame = {
		{ ai_walk, 5 },
		{ ai_walk, 4 },
		{ ai_walk, 4 },
		{ ai_walk, 5 },
		{ ai_walk, 4 },
		{ ai_walk, 5 },
		{ ai_walk, 6 },
		{ ai_walk, 4 },
		{ ai_walk, 4 },
		{ ai_walk, 4 },
		{ ai_walk, 4 },
		{ ai_walk, 5 }
	}
};

static void infantry_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &infantry_move_walk;
}

static const mmove_t infantry_move_run = {
	.firstframe = FRAME_run01,
	.lastframe = FRAME_run08,
	.frame = {
		{ ai_run, 10 },
		{ ai_run, 20 },
		{ ai_run, 5 },
		{ ai_run, 7 },
		{ ai_run, 30 },
		{ ai_run, 35 },
		{ ai_run, 2 },
		{ ai_run, 6 }
	}
};

static void infantry_run (edict_t &self)
{
	self.monsterinfo.currentmove = &infantry_move_run;
}

static const mmove_t infantry_move_death1 = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death120,
	.frame = {
		{ ai_move, -4 },
		{ ai_move },
		{ ai_move },
		{ ai_move, -1 },
		{ ai_move, -4 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, -1 },
		{ ai_move, 3 },
		{ ai_move, 1 },
		{ ai_move, 1 },
		{ ai_move, -2 },
		{ ai_move, 2 },
		{ ai_move, 2 },
		{ ai_move, 9 },
		{ ai_move, 9 },
		{ ai_move, 5 },
		{ ai_move, -3 },
		{ ai_move, -3 }
	}
};

static const mmove_t infantry_move_death3 = {
	.firstframe = FRAME_death301,
	.lastframe = FRAME_death309,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, -6 },
		{ ai_move, -11 },
		{ ai_move, -3 },
		{ ai_move, -11 },
		{ ai_move },
		{ ai_move }
	}
};

static void infantry_die (edict_t &self)
{
	const int32_t n = irandom(2);
	if (n == 0)
	{
		self.monsterinfo.currentmove = &infantry_move_death1;
		self.PlaySound(sound_die2, CHAN_VOICE);
	}
	else if (n == 1)
	{
		self.monsterinfo.currentmove = &infantry_move_death3;
		self.PlaySound(sound_die1, CHAN_VOICE);
	}
	else
	{
		self.monsterinfo.currentmove = &infantry_move_death3;
		self.PlaySound(sound_die2, CHAN_VOICE);
	}
}

static void infantry_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	const bool n = prandom(50);
	if (n)
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

/*QUAKED monster_infantry (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_infantry (edict_t &self)
{
	sound_pain1 = gi.soundindex ("infantry/infpain1.wav");
	sound_pain2 = gi.soundindex ("infantry/infpain2.wav");
	sound_die1 = gi.soundindex ("infantry/infdeth1.wav");
	sound_die2 = gi.soundindex ("infantry/infdeth2.wav");

	sound_search = gi.soundindex ("infantry/infsrch1.wav");
	sound_idle = gi.soundindex ("infantry/infidle1.wav");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 32.f };

	self.health = 100;
	self.mass = 200;

	self.pain = infantry_pain;

	self.monsterinfo.stand = infantry_stand;
	self.monsterinfo.walk = infantry_walk;
	self.monsterinfo.run = infantry_run;
	self.monsterinfo.idle = infantry_fidget;
	self.monsterinfo.die = infantry_die;

	self.Link();

	self.monsterinfo.currentmove = &infantry_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
