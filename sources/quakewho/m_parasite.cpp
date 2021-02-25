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

parasite

==============================================================================
*/

#include "g_local.h"
#include "m_parasite.h"


static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_die;
static soundindex_t	sound_tap;
static soundindex_t	sound_scratch;

static void parasite_tap (edict_t &self)
{
	self.PlaySound(sound_tap, CHAN_BODY, ATTN_IDLE);
}

static void parasite_scratch (edict_t &self)
{
	self.PlaySound(sound_scratch, CHAN_BODY, ATTN_IDLE);
}

static void parasite_do_fidget (edict_t &self);
static void parasite_refidget (edict_t &self);
static void parasite_stand (edict_t &self);

static const mmove_t parasite_move_start_fidget = {
	.firstframe = FRAME_stand18,
	.lastframe = FRAME_stand21,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = parasite_do_fidget
};

static const mmove_t parasite_move_fidget = {
	.firstframe = FRAME_stand22,
	.lastframe = FRAME_stand27,
	.frame = {	
		{ .aifunc = ai_stand, .thinkfunc = parasite_scratch },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_scratch },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = parasite_refidget
};

static const mmove_t parasite_move_end_fidget = {
	.firstframe = FRAME_stand28,
	.lastframe = FRAME_stand35,
	.frame = {
		{ .aifunc = ai_stand, .thinkfunc = parasite_scratch },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	},
	.endfunc = parasite_stand
};

static void parasite_do_fidget (edict_t &self)
{
	self.monsterinfo.currentmove = &parasite_move_fidget;
}

static void parasite_refidget (edict_t &self)
{ 
	if (prandom(80))
		self.monsterinfo.currentmove = &parasite_move_fidget;
	else
		self.monsterinfo.currentmove = &parasite_move_end_fidget;
}

static void parasite_idle (edict_t &self)
{ 
	self.monsterinfo.currentmove = &parasite_move_start_fidget;
}

static const mmove_t parasite_move_stand = {
	.firstframe = FRAME_stand01,
	.lastframe = FRAME_stand17,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
		{ ai_stand },
		{ .aifunc = ai_stand, .thinkfunc = parasite_tap },
	},
	.endfunc = parasite_stand
};

static void parasite_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &parasite_move_stand;
}

static const mmove_t parasite_move_run = {
	.firstframe = FRAME_run03,
	.lastframe = FRAME_run09,
	.frame = {
		{ ai_run, 30 },
		{ ai_run, 30 },
		{ ai_run, 22 },
		{ ai_run, 19 },
		{ ai_run, 24 },
		{ ai_run, 28 },
		{ ai_run, 25 }
	}
};

static void parasite_run (edict_t &self)
{
	self.monsterinfo.currentmove = &parasite_move_run;
}

static const mmove_t parasite_move_start_run = {
	.firstframe = FRAME_run01,
	.lastframe = FRAME_run02,
	.frame = {
		{ ai_run },
		{ ai_run, 30 },
	},
	.endfunc = parasite_run
};

static void parasite_start_run (edict_t &self)
{	
	self.monsterinfo.currentmove = &parasite_move_start_run;
}

static const mmove_t parasite_move_walk = {
	.firstframe = FRAME_run03,
	.lastframe = FRAME_run09,
	.frame = {
		{ ai_walk, 30 },
		{ ai_walk, 30 },
		{ ai_walk, 22 },
		{ ai_walk, 19 },
		{ ai_walk, 24 },
		{ ai_walk, 28 },
		{ ai_walk, 25 }
	}
};

static void parasite_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &parasite_move_walk;
}

static const mmove_t parasite_move_start_walk = {
	.firstframe = FRAME_run01,
	.lastframe = FRAME_run02,
	.frame = {
		{ ai_walk },
		{ ai_walk, 30, parasite_walk }
	}
};

static void parasite_start_walk (edict_t &self)
{	
	self.monsterinfo.currentmove = &parasite_move_start_walk;
}

static void parasite_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	if (prandom(50))
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

static const mmove_t parasite_move_death = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death107,
	.frame = {
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static void parasite_die (edict_t &self)
{
	self.PlaySound(sound_die, CHAN_VOICE);
	self.monsterinfo.currentmove = &parasite_move_death;
}

/*QUAKED monster_parasite (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_parasite (edict_t &self)
{
	sound_pain1 = gi.soundindex ("parasite/parpain1.wav");	
	sound_pain2 = gi.soundindex ("parasite/parpain2.wav");	
	sound_die = gi.soundindex ("parasite/pardeth1.wav");	
	sound_tap = gi.soundindex("parasite/paridle1.wav");
	sound_scratch = gi.soundindex("parasite/paridle2.wav");

	self.s.modelindex = gi.modelindex ("models/monsters/parasite/tris.md2");
	self.mins = { -16.f, -16.f, -24.f };
	self.maxs = { 16.f, 16.f, 24.f };
	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;

	self.health = 175;
	self.mass = 250;

	self.pain = parasite_pain;

	self.monsterinfo.stand = parasite_stand;
	self.monsterinfo.walk = parasite_start_walk;
	self.monsterinfo.run = parasite_start_run;
	self.monsterinfo.idle = parasite_idle;
	self.monsterinfo.die = parasite_die;

	self.Link();

	self.monsterinfo.currentmove = &parasite_move_stand;	
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
