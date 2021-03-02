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

void InfantryMachineGun (edict_t *self);


static int	sound_pain1;
static int	sound_pain2;
static int	sound_die1;
static int	sound_die2;

static int	sound_gunshot;
static int	sound_weapon_cock;
static int	sound_punch_swing;
static int	sound_punch_hit;
static int	sound_sight;
static int	sound_search;
static int	sound_idle;
static int sound_clip_end;

mframe_t infantry_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t infantry_move_stand = {FRAME_stand50, FRAME_stand71, infantry_frames_stand, NULL};

void infantry_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_stand;
}


mframe_t infantry_frames_fidget [] =
{
	ai_stand, 1,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 3,  NULL,
	ai_stand, 6,  NULL,
	ai_stand, 3,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, -1, NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, -2, NULL,
	ai_stand, 1,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, -1, NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, -1, NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, -1, NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 1,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, 0,  NULL,
	ai_stand, -1, NULL,
	ai_stand, -1, NULL,
	ai_stand, 0,  NULL,
	ai_stand, -3, NULL,
	ai_stand, -2, NULL,
	ai_stand, -3, NULL,
	ai_stand, -3, NULL,
	ai_stand, -2, NULL
};
mmove_t infantry_move_fidget = {FRAME_stand01, FRAME_stand49, infantry_frames_fidget, infantry_stand};

void infantry_fidget (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_fidget;
	gi.sound(self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

mframe_t infantry_frames_walk [] =
{
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL
};
mmove_t infantry_move_walk = {FRAME_walk03, FRAME_walk14, infantry_frames_walk, NULL};

void infantry_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &infantry_move_walk;
}

mframe_t infantry_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 20, NULL,
	ai_run, 5,  NULL,
	ai_run, 7,  NULL,
	ai_run, 30, NULL,
	ai_run, 35, NULL,
	ai_run, 2,  NULL,
	ai_run, 6,  NULL
};
mmove_t infantry_move_run = {FRAME_run01, FRAME_run08, infantry_frames_run, NULL};

void infantry_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &infantry_move_stand;
	else
		self->monsterinfo.currentmove = &infantry_move_run;
}


mframe_t infantry_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, 1,  NULL,
	ai_move, -1, NULL,
	ai_move, 1,  NULL,
	ai_move, 6,  NULL,
	ai_move, 2,  NULL
};
mmove_t infantry_move_pain1 = {FRAME_pain101, FRAME_pain110, infantry_frames_pain1, infantry_run};

mframe_t infantry_frames_pain2 [] =
{
	ai_move, -3, NULL,
	ai_move, -3, NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -2, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 2,  NULL,
	ai_move, 5,  NULL,
	ai_move, 2,  NULL
};
mmove_t infantry_move_pain2 = {FRAME_pain201, FRAME_pain210, infantry_frames_pain2, infantry_run};

void infantry_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = rand() % 2;
	if (n == 0)
	{
		self->monsterinfo.currentmove = &infantry_move_pain1;
		gi.sound(self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	else
	{
		self->monsterinfo.currentmove = &infantry_move_pain2;
		gi.sound(self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	}
}


vec3_t	aimangles[] =
{
	0.0, 5.0, 0.0,
	10.0, 15.0, 0.0,
	20.0, 25.0, 0.0,
	25.0, 35.0, 0.0,
	30.0, 40.0, 0.0,
	30.0, 45.0, 0.0,
	25.0, 50.0, 0.0,
	20.0, 40.0, 0.0,
	15.0, 35.0, 0.0,
	40.0, 35.0, 0.0,
	70.0, 35.0, 0.0,
	90.0, 35.0, 0.0
};



void infantry_sight (edict_t *self, edict_t *other)
{
	gi.sound(self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

void infantry_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -15);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEAD;
	gi.linkentity (self);

	M_FlyCheck (self);
}

mframe_t infantry_frames_death1 [] =
{
	ai_move, -4, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -4, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, 3,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
	ai_move, -2, NULL,
	ai_move, 2,  NULL,
	ai_move, 2,  NULL,
	ai_move, 9,  NULL,
	ai_move, 9,  NULL,
	ai_move, 5,  NULL,
	ai_move, -3, NULL,
	ai_move, -3, NULL
};
mmove_t infantry_move_death1 = {FRAME_death101, FRAME_death120, infantry_frames_death1, infantry_dead};

// Off with his head
mframe_t infantry_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 1,   NULL,
	ai_move, 5,   NULL,
	ai_move, -1,  NULL,
	ai_move, 0,   NULL,
	ai_move, 1,   NULL,
	ai_move, 1,   NULL,
	ai_move, 4,   NULL,
	ai_move, 3,   NULL,
	ai_move, 0,   NULL,
	ai_move, -2,  InfantryMachineGun,
	ai_move, -2,  InfantryMachineGun,
	ai_move, -3,  InfantryMachineGun,
	ai_move, -1,  InfantryMachineGun,
	ai_move, -2,  InfantryMachineGun,
	ai_move, 0,   InfantryMachineGun,
	ai_move, 2,   InfantryMachineGun,
	ai_move, 2,   InfantryMachineGun,
	ai_move, 3,   InfantryMachineGun,
	ai_move, -10, InfantryMachineGun,
	ai_move, -7,  InfantryMachineGun,
	ai_move, -8,  InfantryMachineGun,
	ai_move, -6,  NULL,
	ai_move, 4,   NULL,
	ai_move, 0,   NULL
};
mmove_t infantry_move_death2 = {FRAME_death201, FRAME_death225, infantry_frames_death2, infantry_dead};

mframe_t infantry_frames_death3 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -6,  NULL,
	ai_move, -11, NULL,
	ai_move, -3,  NULL,
	ai_move, -11, NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t infantry_move_death3 = {FRAME_death301, FRAME_death309, infantry_frames_death3, infantry_dead};


void infantry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gib_target(self, damage, GIB_MED, point);

		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (!(self->flags & FL_HEADSHOT))
	{
		n = rand() % 3;
		if (n == 0)
		{
			self->monsterinfo.currentmove = &infantry_move_death1;
			gi.sound(self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		}
		else if (n == 1)
		{
			self->monsterinfo.currentmove = &infantry_move_death2;
			gi.sound(self, CHAN_VOICE, sound_die1, 1, ATTN_NORM, 0);
		}
		else
		{
			self->monsterinfo.currentmove = &infantry_move_death3;
			gi.sound(self, CHAN_VOICE, sound_die2, 1, ATTN_NORM, 0);
		}
	}
}


void infantry_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 0.25;
	gi.linkentity (self);
}

void infantry_duck_hold (edict_t *self)
{
	if (random() > 0.5 && self->groundentity)
	{
		self->monsterinfo.aiflags |= AI_JUMPDODGE;
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	}
	else
	{
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}
}

void infantry_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

mframe_t infantry_frames_duck [] =
{
	ai_move, -2, infantry_duck_down,
	ai_move, -5, infantry_duck_hold,
	ai_move, 3,  monster_jump,
	ai_move, 4,  infantry_duck_up,
	ai_move, 0,  NULL
};
mmove_t infantry_move_duck = {FRAME_duck01, FRAME_duck05, infantry_frames_duck, infantry_run};

void infantry_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.95)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &infantry_move_duck;
}


void infantry_cock_gun (edict_t *self)
{
	int		n;

	gi.sound(self, CHAN_WEAPON, sound_weapon_cock, 1, ATTN_NORM, 0);
	if(skill->value < 4)
		n = (rand() & 15) + 3 + 7;
	else
		n = 50;
	monster_skip_frame(self);
	self->monsterinfo.aiflags &= ~AI_RELOADSECONDARY;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
	self->monsterinfo.clip = INFANTRY_CLIPSIZE;
}

void infantry_fire (edict_t *self)
{
	InfantryMachineGun (self);

	if (!M_CheckClearShot(self))//|| level.time > self->monsterinfo.pausetime)
		self->monsterinfo.nextframe = FRAME_attak117;
	else if(self->s.frame == FRAME_attak116)
		self->monsterinfo.nextframe = FRAME_attak111;
}
void infantry_skip_reload(edict_t *self)
{
	monster_skip_frame(self);
	if(self->monsterinfo.clip > 0)
		self->monsterinfo.nextframe = FRAME_attak108;
}
mframe_t infantry_frames_attack1 [] =
{
	ai_charge, 4,  NULL,
	ai_charge, -1, infantry_skip_reload,
	ai_charge, -1, NULL,
	ai_charge, 0,  infantry_cock_gun,
	ai_charge, -1, monster_skip_frame,
	ai_charge, 1,  monster_skip_frame,
	ai_charge, 1,  monster_skip_frame,
	ai_charge, 2,  monster_skip_frame,
	ai_charge, -2, monster_skip_frame,
	ai_charge, -3, NULL,
	ai_charge, 0,  infantry_fire,
	ai_charge, 0,  infantry_fire,
	ai_charge, 0,  infantry_fire,
	ai_charge, 0,  infantry_fire,
	ai_charge, 0,  infantry_fire,
	ai_charge, 0,  infantry_fire,
	ai_charge, 5,  monster_skip_frame,
	ai_charge, -1, monster_skip_frame,
	ai_charge, -2, monster_skip_frame,
	ai_charge, -3, monster_skip_frame
};
mmove_t infantry_move_attack1 = {FRAME_attak101, FRAME_attak120, infantry_frames_attack1, infantry_run};


void infantry_swing (edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_punch_swing, 1, ATTN_NORM, 0);
}

void infantry_smack (edict_t *self)
{
	vec3_t	aim;

	int damage = 5;
	if (skill->value > 3)
		damage *= skill->value;

	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit (self, aim, (damage + (rand() % 5)), 50))
		gi.sound(self, CHAN_WEAPON, sound_punch_hit, 1, ATTN_NORM, 0);
}

mframe_t infantry_frames_attack2 [] =
{
	ai_charge, 3, NULL,
	ai_charge, 6, NULL,
	ai_charge, 0, infantry_swing,
	ai_charge, 8, NULL,
	ai_charge, 5, NULL,
	ai_charge, 8, infantry_smack,
	ai_charge, 6, NULL,
	ai_charge, 3, NULL,
};
mmove_t infantry_move_attack2 = {FRAME_attak201, FRAME_attak208, infantry_frames_attack2, infantry_run};

void infantry_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &infantry_move_attack2;
	else
		self->monsterinfo.currentmove = &infantry_move_attack1;
}

void InfantryMachineGun(edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	int		flash_number;

	int damage = 3;
	if (skill->value > 3)
		damage *= skill->value;

	if (self->s.frame >= FRAME_attak111 && self->s.frame <= FRAME_attak116)
	{
		if (!M_CheckClearShot(self) || self->monsterinfo.clip <= 0)
		{
			if (self->monsterinfo.currentmove == &infantry_move_duck)
			{
				self->s.frame = FRAME_duck05;
				self->monsterinfo.nextframe = FRAME_duck05;
				self->monsterinfo.currentmove = &infantry_move_run;
			}
			else
			{
				self->s.frame = 0;
				self->monsterinfo.nextframe = 0;
				self->monsterinfo.currentmove = &infantry_move_run;
			}
			self->monsterinfo.attack_finished = 0;


			return;
		}
		flash_number = MZ2_INFANTRY_MACHINEGUN_1;
		AngleVectors(self->s.angles, forward, right, NULL);
		G_ProjectSource(self->s.origin, monster_flash_offset[flash_number], forward, right, start);

		if (self->enemy)
		{
			VectorMA(self->enemy->s.origin, 0.05 + (crandom() * clamp(20 - (skill->value * 2), 20, 1) * 0.005) , self->enemy->velocity, target);
			target[2] += self->enemy->viewheight;
			VectorSubtract(target, start, forward);
			VectorNormalize(forward);

			if (self->enemy->health > 0)
				target[2] += self->enemy->viewheight / 2;
			else
				target[2] -= 32;
		}
		else
		{
			AngleVectors(self->s.angles, forward, right, NULL);
		}
	}
	else
	{
		flash_number = MZ2_INFANTRY_MACHINEGUN_2 + (self->s.frame - FRAME_death211);

		AngleVectors(self->s.angles, forward, right, NULL);
		G_ProjectSource(self->s.origin, monster_flash_offset[flash_number], forward, right, start);

		VectorSubtract(self->s.angles, aimangles[flash_number - MZ2_INFANTRY_MACHINEGUN_2], vec);
		AngleVectors(vec, forward, NULL, NULL);
	}
	if (!(self->monsterinfo.aiflags & AI_RELOADSECONDARY) && ffire_radius_check(self) && (skill->value + 5) / 200 > mt_ldrand())
	{
		int damage = 50;
		if (skill->value > 3)
			damage *= skill->value / 2;
		monster_fire_rocket(self, start, forward, damage, 500, MZ2_INFANTRY_MACHINEGUN_1);
		self->monsterinfo.aiflags |= AI_RELOADSECONDARY;
		return;
	}

	self->monsterinfo.clip--;
	if(!self->monsterinfo.clip)
		gi.sound(self, CHAN_ITEM, sound_clip_end, 1, ATTN_IDLE, 0);
	monster_fire_bullet(self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);

}

/*QUAKED monster_infantry (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_infantry (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 = gi.soundindex ("infantry/infpain1.wav");
	sound_pain2 = gi.soundindex ("infantry/infpain2.wav");
	sound_die1 = gi.soundindex ("infantry/infdeth1.wav");
	sound_die2 = gi.soundindex ("infantry/infdeth2.wav");

	sound_gunshot = gi.soundindex ("infantry/infatck1.wav");
	sound_weapon_cock = gi.soundindex ("infantry/infatck3.wav");
	sound_punch_swing = gi.soundindex ("infantry/infatck2.wav");
	sound_punch_hit = gi.soundindex ("infantry/melee2.wav");
	
	sound_sight = gi.soundindex ("infantry/infsght1.wav");
	sound_search = gi.soundindex ("infantry/infsrch1.wav");
	sound_idle = gi.soundindex ("infantry/infidle1.wav");
	
	sound_clip_end = gi.soundindex("weapons/grapple/grhit.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet (self->mins, -12, -20, -24);
	VectorSet (self->maxs, 12, 20, 32);

	self->health = 100 + (5 * skill->value);
	self->gib_health = -40;
	self->mass = 200;

	self->pain = infantry_pain;
	self->die = infantry_die;

	self->monsterinfo.stand = infantry_stand;
	self->monsterinfo.walk = infantry_walk;
	self->monsterinfo.run = infantry_run;
	self->monsterinfo.dodge = infantry_dodge;
	self->monsterinfo.attack = infantry_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = infantry_sight;
	self->monsterinfo.idle = infantry_fidget;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &infantry_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
