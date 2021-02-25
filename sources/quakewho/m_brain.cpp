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

brain

==============================================================================
*/

#include "g_local.h"
#include "m_brain.h"


static soundindex_t	sound_death;
static soundindex_t	sound_idle3;
static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_search;

static void brain_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE);
}

//
// STAND
//
static const mmove_t brain_move_stand = {
	.firstframe = FRAME_stand01,
	.lastframe = FRAME_stand30,
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

static void brain_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &brain_move_stand;
}


//
// IDLE
//
static const mmove_t brain_move_idle = {
	.firstframe = FRAME_stand31,
	.lastframe = FRAME_stand60,
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
	.endfunc = brain_stand
};

static void brain_idle (edict_t &self)
{
	self.PlaySound(sound_idle3, CHAN_AUTO, ATTN_IDLE);
	self.monsterinfo.currentmove = &brain_move_idle;
}

//
// WALK
//
static const mmove_t brain_move_walk1 = {
	.firstframe = FRAME_walk101,
	.lastframe = FRAME_walk111,
	.frame = {
		{ ai_walk, 7 },
		{ ai_walk, 2 },
		{ ai_walk, 3 },
		{ ai_walk, 3 },
		{ ai_walk, 1 },
		{ ai_walk },
		{ ai_walk },
		{ ai_walk, 9 },
		{ ai_walk, -4 },
		{ ai_walk, -1 },
		{ ai_walk, 2 }
	}
};

static void brain_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &brain_move_walk1;
}

static const mmove_t brain_move_death2 = {
	.firstframe = FRAME_death201,
	.lastframe = FRAME_death205,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, 9 },
		{ ai_move }
	}
};

static const mmove_t brain_move_death1 = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death118,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move, -2 },
		{ ai_move, 9 },
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

//
// RUN
//
static const mmove_t brain_move_run = {
	.firstframe = FRAME_walk101,
	.lastframe = FRAME_walk111,
	.frame = {
		{ ai_run, 9 },
		{ ai_run, 2 },
		{ ai_run, 3 },
		{ ai_run, 3 },
		{ ai_run, 1 },
		{ ai_run },
		{ ai_run },
		{ ai_run, 10 },
		{ ai_run, -4 },
		{ ai_run, -1 },
		{ ai_run, 2 }
	}
};

static void brain_run (edict_t &self)
{
	self.monsterinfo.currentmove = &brain_move_run;
}

static void brain_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	const int32_t r = irandom(2);
	if (r == 0)
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else if (r == 1)
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

static void brain_die (edict_t &self)
{
	self.PlaySound(sound_death, CHAN_VOICE);

	if (prandom(50))
		self.monsterinfo.currentmove = &brain_move_death1;
	else
		self.monsterinfo.currentmove = &brain_move_death2;
}

/*QUAKED monster_brain (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_brain (edict_t &self)
{
	sound_death = gi.soundindex ("brain/brndeth1.wav");
	sound_idle3 = gi.soundindex ("brain/brnlens1.wav");
	sound_pain1 = gi.soundindex ("brain/brnpain1.wav");
	sound_pain2 = gi.soundindex ("brain/brnpain2.wav");
	sound_search = gi.soundindex ("brain/brnsrch1.wav");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/brain/tris.md2");
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 32.f };

	self.health = 300;
	self.mass = 400;

	self.pain = brain_pain;

	self.monsterinfo.stand = brain_stand;
	self.monsterinfo.walk = brain_walk;
	self.monsterinfo.run = brain_run;
	self.monsterinfo.search = brain_search;
	self.monsterinfo.idle = brain_idle;
	self.monsterinfo.die = brain_die;

	self.Link();

	self.monsterinfo.currentmove = &brain_move_stand;	
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
