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

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"


static soundindex_t	sound_pain;
static soundindex_t	sound_pain2;
static soundindex_t	sound_death;
static soundindex_t	sound_idle;
static soundindex_t	sound_open;
static soundindex_t	sound_search;

static void gunner_idlesound (edict_t &self)
{
	self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

static void gunner_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE);
}

static void gunner_stand (edict_t &self);

static const mmove_t gunner_move_fidget = {
	.firstframe = FRAME_stand31,
	.lastframe = FRAME_stand70,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = gunner_idlesound },
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
	.endfunc = gunner_stand
};

static void gunner_fidget (edict_t &self)
{
	if (M_FidgetCheck(self, 5))
		self.monsterinfo.currentmove = &gunner_move_fidget;
}

static const mmove_t gunner_move_stand = {
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
		{ .aifunc = ai_stand, .thinkfunc = gunner_fidget },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = gunner_fidget },

		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = gunner_fidget }
	}
};

static void gunner_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &gunner_move_stand;
}

static const mmove_t gunner_move_walk = {
	.firstframe = FRAME_walk07,
	.lastframe = FRAME_walk19,
	.frame = {
		{ ai_walk },
		{ ai_walk, 3 },
		{ ai_walk, 4 },
		{ ai_walk, 5 },
		{ ai_walk, 7 },
		{ ai_walk, 2 },
		{ ai_walk, 6 },
		{ ai_walk, 4 },
		{ ai_walk, 2 },
		{ ai_walk, 7 },
		{ ai_walk, 5 },
		{ ai_walk, 7 },
		{ ai_walk, 4 }
	}
};

static void gunner_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &gunner_move_walk;
}

static const mmove_t gunner_move_run = {
	.firstframe = FRAME_run01,
	.lastframe = FRAME_run08,
	.frame = {
		{ ai_run, 26 },
		{ ai_run, 9 },
		{ ai_run, 9 },
		{ ai_run, 9 },
		{ ai_run, 15 },
		{ ai_run, 10 },
		{ ai_run, 13 },
		{ ai_run, 6 }
	}
};

static void gunner_run (edict_t &self)
{
	self.monsterinfo.currentmove = &gunner_move_run;
}

static void gunner_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	if (prandom(50))
		self.PlaySound(sound_pain, CHAN_VOICE);
	else
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

static const mmove_t gunner_move_death = {
	.firstframe = FRAME_death01,
	.lastframe = FRAME_death11,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, -7 },
		{ ai_move, -3 },
		{ ai_move, -5 },
		{ ai_move, 8 },
		{ ai_move, 6 },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static void gunner_die (edict_t &self)
{
	self.PlaySound(sound_death, CHAN_VOICE);
	self.monsterinfo.currentmove = &gunner_move_death;
}

/*QUAKED monster_gunner (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_gunner (edict_t &self)
{
	sound_death = gi.soundindex ("gunner/death1.wav");	
	sound_pain = gi.soundindex ("gunner/gunpain2.wav");	
	sound_pain2 = gi.soundindex ("gunner/gunpain1.wav");	
	sound_idle = gi.soundindex ("gunner/gunidle1.wav");	
	sound_open = gi.soundindex ("gunner/gunatck1.wav");	
	sound_search = gi.soundindex ("gunner/gunsrch1.wav");	

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/gunner/tris.md2");
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 32.f };

	self.health = 175;
	self.mass = 200;

	self.pain = gunner_pain;

	self.monsterinfo.stand = gunner_stand;
	self.monsterinfo.walk = gunner_walk;
	self.monsterinfo.run = gunner_run;
	self.monsterinfo.search = gunner_search;
	self.monsterinfo.die = gunner_die;

	self.Link();

	self.monsterinfo.currentmove = &gunner_move_stand;	
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
