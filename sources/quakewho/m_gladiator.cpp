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

GLADIATOR

==============================================================================
*/

#include "g_local.h"
#include "m_gladiator.h"


static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_die;
static soundindex_t	sound_idle;
static soundindex_t	sound_search;

static void gladiator_idle (edict_t &self)
{
	self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

static void gladiator_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE);
}

static const mmove_t gladiator_move_stand = {
	.firstframe = FRAME_stand1,
	.lastframe = FRAME_stand7,
	.frame = {
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand }
	}
};

static void gladiator_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &gladiator_move_stand;
}

static const mmove_t gladiator_move_walk = {
	.firstframe = FRAME_walk1,
	.lastframe = FRAME_walk16,
	.frame = {
		{ ai_walk, 15 },
		{ ai_walk, 7 },
		{ ai_walk, 6 },
		{ ai_walk, 5 },
		{ ai_walk, 2 },
		{ ai_walk },
		{ ai_walk, 2 },
		{ ai_walk, 8 },
		{ ai_walk, 12 },
		{ ai_walk, 8 },
		{ ai_walk, 5 },
		{ ai_walk, 5 },
		{ ai_walk, 2 },
		{ ai_walk, 2 },
		{ ai_walk, 1 },
		{ ai_walk, 8 }
	}
};

static void gladiator_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &gladiator_move_walk;
}

static const mmove_t gladiator_move_run = {
	.firstframe = FRAME_run1,
	.lastframe = FRAME_run6,
	.frame = {
		{ ai_run, 23 },
		{ ai_run, 14 },
		{ ai_run, 14 },
		{ ai_run, 21 },
		{ ai_run, 12 },
		{ ai_run, 13 }
	}
};

static void gladiator_run (edict_t &self)
{
	self.monsterinfo.currentmove = &gladiator_move_run;
}

static void gladiator_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	if (prandom(50))
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

static const mmove_t gladiator_move_death = {
	.firstframe = FRAME_death1,
	.lastframe = FRAME_death22,
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
		{ ai_move }
	}
};

static void gladiator_die (edict_t &self)
{
	self.PlaySound(sound_die, CHAN_VOICE);
	self.monsterinfo.currentmove = &gladiator_move_death;
}

/*QUAKED monster_gladiator (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
*/
void SP_monster_gladiator (edict_t &self)
{
	sound_pain1 = gi.soundindex ("gladiator/pain.wav");	
	sound_pain2 = gi.soundindex ("gladiator/gldpain2.wav");	
	sound_die = gi.soundindex ("gladiator/glddeth2.wav");
	sound_idle = gi.soundindex ("gladiator/gldidle1.wav");
	sound_search = gi.soundindex ("gladiator/gldsrch1.wav");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/gladiatr/tris.md2");
	self.mins = { -32.f, -32.f, -24.f };
	self.maxs = { 32.f, 32.f, 64.f };

	self.health = 400;
	self.mass = 400;

	self.pain = gladiator_pain;

	self.monsterinfo.stand = gladiator_stand;
	self.monsterinfo.walk = gladiator_walk;
	self.monsterinfo.run = gladiator_run;
	self.monsterinfo.idle = gladiator_idle;
	self.monsterinfo.search = gladiator_search;
	self.monsterinfo.die = gladiator_die;

	self.Link();

	self.monsterinfo.currentmove = &gladiator_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
