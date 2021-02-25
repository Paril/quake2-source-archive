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

TANK

==============================================================================
*/

#include "g_local.h"
#include "m_tank.h"

static soundindex_t	sound_thud;
static soundindex_t	sound_pain;
static soundindex_t	sound_idle;
static soundindex_t	sound_die;
static soundindex_t	sound_step;

//
// misc
//

static void tank_footstep (edict_t &self)
{
	self.PlaySound(sound_step, CHAN_BODY);
}

static void tank_thud (edict_t &self)
{
	self.PlaySound(sound_thud, CHAN_BODY);
}

static void tank_idle (edict_t &self)
{
	self.PlaySound(sound_idle, CHAN_VOICE, ATTN_IDLE);
}

//
// stand
//

static const mmove_t tank_move_stand = {
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
	
static void tank_stand (edict_t &self)
{
	self.monsterinfo.currentmove = &tank_move_stand;
}

//
// walk
//

static void tank_walk (edict_t &self);

static const mmove_t tank_move_walk = {
	.firstframe = FRAME_walk05,
	.lastframe = FRAME_walk20,
	.frame = {
		{ ai_walk, 4 },
		{ ai_walk, 5 },
		{ ai_walk, 3 },
		{ ai_walk, 2 },
		{ ai_walk, 5 },
		{ ai_walk, 5 },
		{ ai_walk, 4 },
		{ ai_walk, 4, tank_footstep },
		{ ai_walk, 3 },
		{ ai_walk, 5 },
		{ ai_walk, 4 },
		{ ai_walk, 5 },
		{ ai_walk, 7 },
		{ ai_walk, 7 },
		{ ai_walk, 6 },
		{ ai_walk, 6, tank_footstep }
	}
};

static void tank_walk (edict_t &self)
{
	self.monsterinfo.currentmove = &tank_move_walk;
}

//
// run
//

static void tank_run (edict_t &self);

static const mmove_t tank_move_start_run = {
	.firstframe = FRAME_walk01,
	.lastframe = FRAME_walk04,
	.frame = {
		{ ai_run },
		{ ai_run, 6 },
		{ ai_run, 6 },
		{ ai_run, 11, tank_footstep }
	},
	.endfunc = tank_run
};

static const mmove_t tank_move_run = {
	.firstframe = FRAME_walk05,
	.lastframe = FRAME_walk20,
	.frame = {
		{ ai_run, 4 },
		{ ai_run, 5 },
		{ ai_run, 3 },
		{ ai_run, 2 },
		{ ai_run, 5 },
		{ ai_run, 5 },
		{ ai_run, 4 },
		{ ai_run, 4, tank_footstep },
		{ ai_run, 3 },
		{ ai_run, 5 },
		{ ai_run, 4 },
		{ ai_run, 5 },
		{ ai_run, 7 },
		{ ai_run, 7 },
		{ ai_run, 6 },
		{ ai_run, 6,tank_footstep }
	}
};

static void tank_run (edict_t &self)
{
	if (self.monsterinfo.currentmove == &tank_move_walk ||
		self.monsterinfo.currentmove == &tank_move_start_run)
		self.monsterinfo.currentmove = &tank_move_run;
	else
		self.monsterinfo.currentmove = &tank_move_start_run;
}

static void tank_pain (edict_t &self, edict_t &other, const vec_t &kick, const int32_t &damage)
{
	if (level.time < self.pain_debounce_time)
		return;

	self.pain_debounce_time = level.time + 3000;
	self.PlaySound(sound_pain, CHAN_VOICE);
}

//
// death
//
static const mmove_t tank_move_death = {
	.firstframe = FRAME_death101,
	.lastframe = FRAME_death132,
	.frame = {
		{ ai_move, -7 },
		{ ai_move, -2 },
		{ ai_move, -2 },
		{ ai_move, 1 },
		{ ai_move, 3 },
		{ ai_move, 6 },
		{ ai_move, 1 },
		{ ai_move, 1 },
		{ ai_move, 2 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, -2 },
		{ ai_move },
		{ ai_move },
		{ ai_move, -3 },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move },
		{ ai_move, -4 },
		{ ai_move, -6 },
		{ ai_move, -4 },
		{ ai_move, -5 },
		{ ai_move, -7 },
		{ ai_move, -15, tank_thud },
		{ ai_move, -5 },
		{ ai_move },
		{ ai_move },
		{ ai_move }
	}
};

static void tank_die (edict_t &self)
{
	self.PlaySound(sound_die, CHAN_VOICE);
	self.monsterinfo.currentmove = &tank_move_death;
}

//
// monster_tank
//

/*QUAKED monster_tank (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
/*QUAKED monster_tank_commander (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_tank (edict_t &self)
{
	self.s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	self.mins = { -32.f, -32.f, -16.f };
	self.maxs = { 32.f, 32.f, 72.f };
	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_BBOX;

	sound_pain = gi.soundindex ("tank/tnkpain2.wav");
	sound_thud = gi.soundindex ("tank/tnkdeth2.wav");
	sound_idle = gi.soundindex ("tank/tnkidle1.wav");
	sound_die = gi.soundindex ("tank/death.wav");
	sound_step = gi.soundindex ("tank/step.wav");

	if (prandom(50))
	{
		self.health = 1000;
		self.s.skinnum = 2;
		self.monsterinfo.undamaged_skin = 2;
		self.monsterinfo.damaged_skin = 3;
	}
	else
	{
		self.health = 750;
		self.monsterinfo.undamaged_skin = 0;
		self.monsterinfo.damaged_skin = 1;
	}

	self.mass = 500;

	self.pain = tank_pain;
	self.monsterinfo.stand = tank_stand;
	self.monsterinfo.walk = tank_walk;
	self.monsterinfo.run = tank_run;
	self.monsterinfo.idle = tank_idle;
	self.monsterinfo.die = tank_die;

	self.Link();
	
	self.monsterinfo.currentmove = &tank_move_stand;
	self.monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
