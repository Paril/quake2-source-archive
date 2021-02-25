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

MEDIC

==============================================================================
*/

#include "g_local.h"
#include "m_medic.h"

static soundindex_t	sound_idle1;
static soundindex_t	sound_pain1;
static soundindex_t	sound_pain2;
static soundindex_t	sound_die;
static soundindex_t	sound_search;

static void medic_idle (edict_t &self)
{
	self.PlaySound(sound_idle1, CHAN_VOICE, ATTN_IDLE);
}

static void medic_search (edict_t &self)
{
	self.PlaySound(sound_search, CHAN_VOICE, ATTN_IDLE);
}

static const mmove_t medic_move_stand = {
	.firstframe = FRAME_wait1,
	.lastframe = FRAME_wait90,
	.frame = {
		{ .aifunc = ai_stand, .thinkfunc = medic_idle },
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
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
		{ ai_stand },
	}
};

static void medic_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &medic_move_stand;
}

static const mmove_t medic_move_walk = {
	.firstframe = FRAME_walk1,
	.lastframe = FRAME_walk12,
	.frame = {
		{ ai_walk, 6.2 },
		{ ai_walk, 18.1 },
		{ ai_walk, 1 },
		{ ai_walk, 9 },
		{ ai_walk, 10 },
		{ ai_walk, 9 },
		{ ai_walk, 11 },
		{ ai_walk, 11.6 },
		{ ai_walk, 2 },
		{ ai_walk, 9.9 },
		{ ai_walk, 14 },
		{ ai_walk, 9.3 }
	}
};

static void medic_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &medic_move_walk;
}

static const mmove_t medic_move_run = {
	.firstframe = FRAME_run1,
	.lastframe = FRAME_run6,
	.frame = {
		{ ai_run, 18 },
		{ ai_run, 22.5 },
		{ ai_run, 25.4 },
		{ ai_run, 23.4 },
		{ ai_run, 24 },
		{ ai_run, 35.6 }
	}
};

static void medic_run (edict_t &self)
{
	self.monsterinfo.currentmove = &medic_move_run;
}

static void medic_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;

	if (prandom(50))
		self.PlaySound(sound_pain1, CHAN_VOICE);
	else
		self.PlaySound(sound_pain2, CHAN_VOICE);
}

static const mmove_t medic_move_death = {
	.firstframe = FRAME_death1,
	.lastframe = FRAME_death30,
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
		{ ai_move }
	}
};

static void medic_die (edict_t &self)
{
	self.PlaySound(sound_die, CHAN_VOICE);
	self.monsterinfo.currentmove = &medic_move_death;
}

/*QUAKED monster_medic (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_medic (edict_t &self)
{
	sound_idle1 = gi.soundindex ("medic/idle.wav");
	sound_pain1 = gi.soundindex ("medic/medpain1.wav");
	sound_pain2 = gi.soundindex ("medic/medpain2.wav");
	sound_die = gi.soundindex ("medic/meddeth1.wav");
	sound_search = gi.soundindex ("medic/medsrch1.wav");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;
	self.s.modelindex = gi.modelindex ("models/monsters/medic/tris.md2");
	self.mins = { -24.f, -24.f, -24.f };
	self.maxs = { 24.f, 24.f, 32.f };

	self.health = 300;
	self.mass = 400;

	self.pain = medic_pain;

	self.monsterinfo.stand = medic_stand;
	self.monsterinfo.walk = medic_walk;
	self.monsterinfo.run = medic_run;
	self.monsterinfo.idle = medic_idle;
	self.monsterinfo.search = medic_search;
	self.monsterinfo.die = medic_die;

	self.Link();

	self.monsterinfo.currentmove = &medic_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;
	self.monsterinfo.damaged_skin = 1;

	walkmonster_start (self);
}
