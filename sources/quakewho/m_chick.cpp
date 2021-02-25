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

chick

==============================================================================
*/

#include "g_local.h"
#include "m_chick.h"

static void chick_stand (edict_t &self);
static void chick_run (edict_t &self);

static soundindex_t	sound_death1;
static soundindex_t	sound_death2;
static soundindex_t	sound_idle1;
static soundindex_t	sound_idle2;
static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_pain3;
static soundindex_t	sound_search;

static void ChickMoan (edict_t &self)
{
	if (prandom(50))
		self.PlaySound(sound_idle1, CHAN_VOICE, ATTN_IDLE);
	else
		self.PlaySound(sound_idle2, CHAN_VOICE, ATTN_IDLE);
}

static const mmove_t chick_move_fidget = {
	.firstframe = FRAME_stand201,
	.lastframe = FRAME_stand230,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = ChickMoan },
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
	.endfunc = chick_stand
};

static void chick_fidget (edict_t &self)
{
	if (M_FidgetCheck(self, 30))
		self.monsterinfo.currentmove = &chick_move_fidget;
}

static const mmove_t chick_move_stand = {
	.firstframe = FRAME_stand101,
	.lastframe = FRAME_stand130,
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
		{ .aifunc = ai_stand, .thinkfunc = chick_fidget },
	}
};

static void chick_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &chick_move_stand;
}

static const mmove_t chick_move_start_run = {
	.firstframe = FRAME_walk01,
	.lastframe = FRAME_walk10,
	.frame = {
		{ ai_run, 1 },
		{ ai_run },
		{ ai_run },
		{ ai_run, -1 }, 
		{ ai_run, -1 }, 
		{ ai_run },
		{ ai_run, 1 },
		{ ai_run, 3 },
		{ ai_run, 6 },
		{ ai_run, 3 }
	},
	.endfunc = chick_run
};

static const mmove_t chick_move_run = {
	.firstframe = FRAME_walk11,
	.lastframe = FRAME_walk20,
	.frame = {
		{ ai_run, 6 },
		{ ai_run, 8 },
		{ ai_run, 13 },
		{ ai_run, 5 },
		{ ai_run, 7 },
		{ ai_run, 4 },
		{ ai_run, 11 },
		{ ai_run, 5 },
		{ ai_run, 9 },
		{ ai_run, 7 }
	}
};

static const mmove_t chick_move_walk = {
	.firstframe = FRAME_walk11,
	.lastframe = FRAME_walk20,
	.frame = {
		{ ai_walk, 6 },
		{ ai_walk, 8 },
		{ ai_walk, 13 },
		{ ai_walk, 5 },
		{ ai_walk, 7 },
		{ ai_walk, 4 },
		{ ai_walk, 11 },
		{ ai_walk, 5 },
		{ ai_walk, 9 },
		{ ai_walk, 7 }
	}
};

static void chick_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &chick_move_walk;
}

static void chick_run (edict_t &self)
{
	if (self.monsterinfo.currentmove == &chick_move_walk ||
		self.monsterinfo.currentmove == &chick_move_start_run)
		self.monsterinfo.currentmove = &chick_move_run;
	else
		self.monsterinfo.currentmove = &chick_move_start_run;
}

static void chick_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
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
		self.PlaySound(sound_pain3, CHAN_VOICE);
}

static const mmove_t chick_move_death2 = {
	.firstframe = FRAME_death201,
	.lastframe = FRAME_death223,
	.frame = {
		{ ai_move, -6 },
		{ ai_move },
		{ ai_move, -1 },
		{ ai_move, -5 },
		{ ai_move },
		{ ai_move, -1 },
		{ ai_move, -2 },
		{ ai_move, 1 },
		{ ai_move, 10 },
		{ ai_move, 2 },
		{ ai_move, 3 },
		{ ai_move, 1 },
		{ ai_move, 2 },
		{ ai_move },
		{ ai_move, 3 },
		{ ai_move, 3 },
		{ ai_move, 1 },
		{ ai_move, -3 },
		{ ai_move, -5 },
		{ ai_move, 4 },
		{ ai_move, 15 },
		{ ai_move, 14 },
		{ ai_move, 1 }
	}
};

static const mmove_t chick_move_death1 = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death112,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move, -7 },
		{ ai_move, 4 },
		{ ai_move, 11 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static void chick_die (edict_t &self)
{
	if (prandom(50))
	{
		self.monsterinfo.currentmove = &chick_move_death1;
		self.PlaySound(sound_death1, CHAN_VOICE);
	}
	else
	{
		self.monsterinfo.currentmove = &chick_move_death2;
		self.PlaySound(sound_death2, CHAN_VOICE);
	}
}

/*QUAKED monster_chick (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_chick (edict_t &self)
{
	sound_death1			= gi.soundindex ("chick/chkdeth1.wav");
	sound_death2			= gi.soundindex ("chick/chkdeth2.wav");
	sound_idle1				= gi.soundindex ("chick/chkidle1.wav");
	sound_idle2				= gi.soundindex ("chick/chkidle2.wav");
	sound_pain1				= gi.soundindex ("chick/chkpain1.wav");
	sound_pain2				= gi.soundindex ("chick/chkpain2.wav");
	sound_pain3				= gi.soundindex ("chick/chkpain3.wav");
	sound_search			= gi.soundindex ("chick/chksrch1.wav");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	self.mins = { -16.f, -16.f, 0.f };
	self.maxs = { 16.f, 16.f, 56.f };

	self.health = 175;
	self.mass = 200;

	self.pain = chick_pain;

	self.monsterinfo.stand = chick_stand;
	self.monsterinfo.walk = chick_walk;
	self.monsterinfo.run = chick_run;
	self.monsterinfo.die = chick_die;

	self.Link();

	self.monsterinfo.currentmove = &chick_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
