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

SOLDIER

==============================================================================
*/

#include "g_local.h"
#include "m_soldier.h"
void soldier_dodge(edict_t *self, edict_t *attacker, float eta);
void soldier_duck_up(edict_t *self);

static int	sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_pain_light;
static int	sound_pain;
static int	sound_pain_ss;
static int	sound_death_light;
static int	sound_death;
static int	sound_death_ss;
static int	sound_cock;

void start_charge(edict_t *self)
{
	self->monsterinfo.aiflags |= AI_CHARGEDSHOT;
	self->s.sound = gi.soundindex("weapons/BLASTSPLP.wav");
	gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/BLASTSPST.wav"), 1, ATTN_IDLE, 0);
}

void soldier_idle (edict_t *self)
{
	if (random() > 0.8)
		gi.sound(self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void soldier_cock (edict_t *self)
{
	if (self->s.frame == FRAME_stand322)
		gi.sound(self, CHAN_WEAPON, sound_cock, 1, ATTN_IDLE, 0);
	else
		gi.sound(self, CHAN_WEAPON, sound_cock, 1, ATTN_NORM, 0);
}


// STAND

void soldier_stand (edict_t *self);

mframe_t soldier_frames_stand1 [] =
{
	ai_stand, 0, soldier_idle,
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t soldier_move_stand1 = {FRAME_stand101, FRAME_stand130, soldier_frames_stand1, soldier_stand};

mframe_t soldier_frames_stand3 [] =
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
	ai_stand, 0, soldier_cock,
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
mmove_t soldier_move_stand3 = {FRAME_stand301, FRAME_stand339, soldier_frames_stand3, soldier_stand};

#if 0
mframe_t soldier_frames_stand4 [] =
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 4, NULL,
	ai_stand, 1, NULL,
	ai_stand, -1, NULL,
	ai_stand, -2, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t soldier_move_stand4 = {FRAME_stand401, FRAME_stand452, soldier_frames_stand4, NULL};
#endif

void soldier_stand (edict_t *self)
{
	if ((self->monsterinfo.currentmove == &soldier_move_stand3) || (mt_ldrand() < 0.8))
		self->monsterinfo.currentmove = &soldier_move_stand1;
	else
		self->monsterinfo.currentmove = &soldier_move_stand3;
}


//
// WALK
//

void soldier_walk1_random (edict_t *self)
{
	if (mt_ldrand() > 0.1)
		self->monsterinfo.nextframe = FRAME_walk101;
}

mframe_t soldier_frames_walk1 [] =
{
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, -1, soldier_walk1_random,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL
};
mmove_t soldier_move_walk1 = {FRAME_walk101, FRAME_walk133, soldier_frames_walk1, NULL};

mframe_t soldier_frames_walk2 [] =
{
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 9,  NULL,
	ai_walk, 8,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 7,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 7,  NULL
};
mmove_t soldier_move_walk2 = {FRAME_walk209, FRAME_walk218, soldier_frames_walk2, NULL};

void soldier_walk (edict_t *self)
{
	if (mt_ldrand() < 0.5)
		self->monsterinfo.currentmove = &soldier_move_walk1;
	else
		self->monsterinfo.currentmove = &soldier_move_walk2;
}


//
// RUN
//

void soldier_run(edict_t *self);

void update_move_frame(edict_t *self)
{
	return;
	int frame_num;
	if (self->s.frame >= FRAME_run01 && self->s.frame <= FRAME_run08)
	{
		frame_num = self->s.frame - FRAME_run01;
		if(self->s.frame <= FRAME_run07)
			frame_num++;
	}
	else if(self->s.frame >= FRAME_run_l01 && self->s.frame <= FRAME_run_l08)
	{
		frame_num = self->s.frame - FRAME_run_l01;
		if (self->s.frame <= FRAME_run_l07)
			frame_num++;
	}
	else if (self->s.frame >= FRAME_run_r01 && self->s.frame <= FRAME_run_r08)
	{
		frame_num = self->s.frame - FRAME_run_r01;
		if(self->s.frame <= FRAME_run_r07)
			frame_num++;
	}

	if (self->monsterinfo.move_dir == MOVE_LEFT)
	{
		self->monsterinfo.nextframe = frame_num + FRAME_run_l01;
	}
	else if (self->monsterinfo.move_dir == MOVE_RIGHT)
	{
		self->monsterinfo.nextframe = frame_num + FRAME_run_r01;
	}
	else if (self->monsterinfo.move_dir == MOVE_STRAIGHT)
	{
		self->monsterinfo.nextframe = frame_num + FRAME_run01;
	}
	
}
void update_move_dir(edict_t *self);

mframe_t soldier_frames_start_run [] =
{
	ai_run, 7,  update_move_dir,
	ai_run, 5,  update_move_dir
};
mmove_t soldier_move_start_run = {FRAME_run01, FRAME_run02, soldier_frames_start_run, soldier_run};


mframe_t soldier_frames_run [] =
{
	ai_run, 10, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 16, update_move_dir,
	ai_run, 10, update_move_dir,
	ai_run, 15, update_move_dir
};
mmove_t soldier_move_run = {FRAME_run03, FRAME_run08, soldier_frames_run, NULL};

mframe_t soldier_frames_start_run_l[] =
{
	ai_run, 7,  update_move_dir,
	ai_run, 5,  update_move_dir
};
mmove_t soldier_move_start_run_l = { FRAME_run_l01, FRAME_run_l02, soldier_frames_start_run_l, soldier_run };


mframe_t soldier_frames_run_l[] =
{
	ai_run, 10, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 16, update_move_dir,
	ai_run, 10, update_move_dir,
	ai_run, 15, update_move_dir
};
mmove_t soldier_move_run_l = { FRAME_run_l03, FRAME_run_l08, soldier_frames_run_l, NULL };

mframe_t soldier_frames_start_run_r[] =
{
	ai_run, 7,  update_move_dir,
	ai_run, 5,  update_move_dir
};
mmove_t soldier_move_start_run_r = { FRAME_run_r01, FRAME_run_r02, soldier_frames_start_run_r, soldier_run };


mframe_t soldier_frames_run_r[] =
{
	ai_run, 10, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 11, update_move_dir,
	ai_run, 16, update_move_dir,
	ai_run, 10, update_move_dir,
	ai_run, 15, update_move_dir
};
mmove_t soldier_move_run_r = { FRAME_run_r03, FRAME_run_r08, soldier_frames_run_r, NULL };

void soldier_run (edict_t *self)
{
	
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &soldier_move_stand1;
		return;
	}
	if (skill->value > 3 && random() < DODGE_CHANCE * 0.1 || skill->value < 4 && random() < DODGE_CHANCE * 0.25)
	{
		self->monsterinfo.aiflags |= AI_JUMPDODGE;
		soldier_dodge(self, self->enemy, 0);
	}
	if (self->monsterinfo.currentmove == &soldier_move_walk1 ||
		self->monsterinfo.currentmove == &soldier_move_walk2 ||
		self->monsterinfo.currentmove == &soldier_move_start_run_l ||
		self->monsterinfo.currentmove == &soldier_move_start_run_r ||
		self->monsterinfo.currentmove == &soldier_move_start_run)
	{
		if (self->monsterinfo.move_dir == MOVE_LEFT && self->monsterinfo.movedir_start > level.time)
			self->monsterinfo.currentmove = &soldier_move_run_l;
		else if(self->monsterinfo.move_dir == MOVE_RIGHT && self->monsterinfo.movedir_start > level.time)
				self->monsterinfo.currentmove = &soldier_move_run_r;
		else
			self->monsterinfo.currentmove = &soldier_move_run;
	}
	else
	{
		if (self->monsterinfo.move_dir == MOVE_LEFT && self->monsterinfo.movedir_start > level.time)
			self->monsterinfo.currentmove = &soldier_move_start_run_l;
		else if (self->monsterinfo.move_dir == MOVE_RIGHT && self->monsterinfo.movedir_start > level.time)
			self->monsterinfo.currentmove = &soldier_move_start_run_r;
		else
			self->monsterinfo.currentmove = &soldier_move_run;

	}
}
void update_move_dir(edict_t *self)
{
	
	if (self->monsterinfo.move_dir == MOVE_LEFT)
	{
		if (self->s.frame == FRAME_run_r01 || self->s.frame == FRAME_run01)
		{
			self->monsterinfo.currentmove = &soldier_move_start_run_l;
			update_move_frame(self);
		}
		else if (self->s.frame >= FRAME_run_r02 && self->s.frame <= FRAME_run_r08 || self->s.frame >= FRAME_run02 && self->s.frame <= FRAME_run08)
		{
			self->monsterinfo.currentmove = &soldier_move_run_l;
			update_move_frame(self);
		}

	}
	if (self->monsterinfo.move_dir == MOVE_RIGHT)
	{
		if (self->s.frame == FRAME_run_l01  || self->s.frame == FRAME_run01)
		{
			self->monsterinfo.currentmove = &soldier_move_start_run_r;
			update_move_frame(self);
		}
		else if (self->s.frame >= FRAME_run_l02 && self->s.frame <= FRAME_run_l08 || self->s.frame >= FRAME_run02 && self->s.frame <= FRAME_run08)
		{
			self->monsterinfo.currentmove = &soldier_move_run_r;
			update_move_frame(self);
		}
	}
	if (self->monsterinfo.move_dir == MOVE_STRAIGHT)
	{
		if (self->s.frame == FRAME_run_r01  || self->s.frame == FRAME_run_l01 )
		{
			self->monsterinfo.currentmove = &soldier_move_start_run;
			update_move_frame(self);
		}

		else if (self->s.frame >= FRAME_run_r02 && self->s.frame <= FRAME_run_r08 || self->s.frame >= FRAME_run_l02 && self->s.frame <= FRAME_run_l08)
		{
			self->monsterinfo.currentmove = &soldier_move_run;
			update_move_frame(self);
		}
	}
}
//


//
// PAIN
//

mframe_t soldier_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL
};
mmove_t soldier_move_pain1 = {FRAME_pain101, FRAME_pain105, soldier_frames_pain1, soldier_run};

mframe_t soldier_frames_pain2 [] =
{
	ai_move, -13, NULL,
	ai_move, -1,  NULL,
	ai_move, 2,   NULL,
	ai_move, 4,   NULL,
	ai_move, 2,   NULL,
	ai_move, 3,   NULL,
	ai_move, 2,   NULL
};
mmove_t soldier_move_pain2 = {FRAME_pain201, FRAME_pain207, soldier_frames_pain2, soldier_run};

mframe_t soldier_frames_pain3 [] =
{
	ai_move, -8, NULL,
	ai_move, 10, NULL,
	ai_move, -4, NULL,
	ai_move, -1, NULL,
	ai_move, -3, NULL,
	ai_move, 0,  NULL,
	ai_move, 3,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL,
	ai_move, 1,  NULL,
	ai_move, 2,  NULL,
	ai_move, 4,  NULL,
	ai_move, 3,  NULL,
	ai_move, 2,  NULL
};
mmove_t soldier_move_pain3 = {FRAME_pain301, FRAME_pain318, soldier_frames_pain3, soldier_run};

mframe_t soldier_frames_pain4 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -6,  NULL,
	ai_move, 8,   NULL,
	ai_move, 4,   NULL,
	ai_move, 1,   NULL,
	ai_move, 0,   NULL,
	ai_move, 2,   NULL,
	ai_move, 5,   NULL,
	ai_move, 2,   NULL,
	ai_move, -1,  NULL,
	ai_move, -1,  NULL,
	ai_move, 3,   NULL,
	ai_move, 2,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_pain4 = {FRAME_pain401, FRAME_pain417, soldier_frames_pain4, soldier_run};


void soldier_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;
	int		n;

	if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;

	if (self->monsterinfo.aiflags & AI_DUCKED)
		soldier_duck_up(self);

		float num = clamp(self->yaw_speed * (clamp(1 - ((float)damage / (float)self->max_health), 1, 0.02)), 999, 0.25);
	if (self->health <= 0)
		self->yaw_speed *= 0.02;
	//gi.bprintf(PRINT_HIGH, "SOLDIER PAIN: num = %f, yaw_speed = %f\n", num, self->yaw_speed);

	self->yaw_speed = num;
	if (level.time < self->pain_debounce_time)
	{
		if ((self->velocity[2] > 100) && ( (self->monsterinfo.currentmove == &soldier_move_pain1) || (self->monsterinfo.currentmove == &soldier_move_pain2) || (self->monsterinfo.currentmove == &soldier_move_pain3)))
			self->monsterinfo.currentmove = &soldier_move_pain4;
		return;
	}

	self->pain_debounce_time = level.time + 3;

	n = self->s.skinnum | 1;
	if (n == 1)
		gi.sound(self, CHAN_VOICE, sound_pain_light, 1, ATTN_NORM, 0);
	else if (n == 3)
		gi.sound(self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sound_pain_ss, 1, ATTN_NORM, 0);

	if (self->velocity[2] > 50)
	{
		self->monsterinfo.currentmove = &soldier_move_pain4;
		return;
	}

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	

	if (random() + (damage * 0.01) < 1.0)
		self->monsterinfo.currentmove = &soldier_move_pain1;
	else if (random() + (damage * 0.01) < 1.0)
		self->monsterinfo.currentmove = &soldier_move_pain2;
	else
		self->monsterinfo.currentmove = &soldier_move_pain3;
}


//
// ATTACK
//

static int blaster_flash [] = {MZ2_SOLDIER_BLASTER_1, MZ2_SOLDIER_BLASTER_2, MZ2_SOLDIER_BLASTER_3, MZ2_SOLDIER_BLASTER_4, MZ2_SOLDIER_BLASTER_5, MZ2_SOLDIER_BLASTER_6, MZ2_SOLDIER_BLASTER_7, MZ2_SOLDIER_BLASTER_8};
static int shotgun_flash [] = {MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_SHOTGUN_8};
static int machinegun_flash [] = {MZ2_SOLDIER_MACHINEGUN_1, MZ2_SOLDIER_MACHINEGUN_2, MZ2_SOLDIER_MACHINEGUN_3, MZ2_SOLDIER_MACHINEGUN_4, MZ2_SOLDIER_MACHINEGUN_5, MZ2_SOLDIER_MACHINEGUN_6, MZ2_SOLDIER_MACHINEGUN_7, MZ2_SOLDIER_MACHINEGUN_8};



// ATTACK1 (blaster/shotgun)

void soldier_fire1 (edict_t *self)
{
	soldier_fire (self, 0);
}

void soldier_attack1_refire1 (edict_t *self)
{
	if (self->s.skinnum > 1)
		return;

	if (self->enemy->health <= 0)
		return;

	if (self->enemy->health <= 0 || !M_CheckClearShot(self) || (self->enemy->s.effects & EF_GIB) || self->monsterinfo.aiflags & AI_NOREFIRE)
	{
		self->monsterinfo.aiflags &= ~AI_NOREFIRE;
		self->monsterinfo.nextframe = FRAME_attak110;
		self->monsterinfo.currentmove = &soldier_move_run;
		//gi.bprintf(PRINT_HIGH, "SOLDIER ATTACK1 REFIRE1: health, clearshot or gibbed! aborting attack!\n");
		return;
	}

	if ( ((skill->value == 3) && (random() < 0.5) || range(self, self->enemy) == RANGE_MELEE || self->monsterinfo.aggression + random() < 1.75 && skill->value > 3)  )
		self->monsterinfo.nextframe = FRAME_attak102;
	else
	{
		self->monsterinfo.nextframe = FRAME_attak110;

		//gi.bprintf(PRINT_HIGH, "SOLDIER ATTACK1 REFIRE1: not refiring !\n");

	}
}

void soldier_attack1_refire2 (edict_t *self)
{
	if (self->s.skinnum < 2)
		return;

	if (self->enemy->health <= 0)
		return;

	if (self->enemy->health <= 0 || !M_CheckClearShot(self) || (self->enemy->s.effects & EF_GIB) || self->monsterinfo.aiflags & AI_NOREFIRE)
	{
		self->monsterinfo.aiflags &= ~AI_NOREFIRE;
		self->monsterinfo.nextframe = FRAME_attak110;
		return;
	}

	if (((skill->value == 3) && (random() < 0.5) || range(self, self->enemy) == RANGE_MELEE || self->monsterinfo.aggression + random() < 1.75 && skill->value > 3))
		self->monsterinfo.nextframe = FRAME_attak102;
}

mframe_t soldier_frames_attack1 [] =
{
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  soldier_fire1,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  soldier_attack1_refire1,
	ai_charge, 0,  soldier_cock,
	ai_charge, 0,  NULL,
	ai_charge, 0,  soldier_attack1_refire2,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t soldier_move_attack1 = {FRAME_attak101, FRAME_attak112, soldier_frames_attack1, soldier_run};

// ATTACK2 (blaster/shotgun)

void soldier_fire2 (edict_t *self)
{
	soldier_fire (self, 1);
}

void soldier_attack2_refire1 (edict_t *self)
{
	if (self->s.skinnum > 1)
		return;

	if (self->enemy->health <= 0)
		return;

	if (self->enemy->health <= 0 || !M_CheckClearShot(self) || (self->enemy->s.effects & EF_GIB) || self->monsterinfo.aiflags & AI_NOREFIRE)
	{
		self->monsterinfo.aiflags &= ~AI_NOREFIRE;
		self->monsterinfo.currentmove = &soldier_move_run;
		self->monsterinfo.nextframe = FRAME_attak218;
		return;
	}

	if (((skill->value == 3) && (random() < 0.5) || range(self, self->enemy) == RANGE_MELEE || self->monsterinfo.aggression + random() < 1.75 && skill->value > 3))
		self->monsterinfo.nextframe = FRAME_attak204;
	else
		self->monsterinfo.nextframe = FRAME_attak218;
}

void soldier_attack2_refire2 (edict_t *self)
{
	if (self->s.skinnum < 2)
		return;

	if (self->enemy->health <= 0)
		return;


	if (self->enemy->health <= 0 || !M_CheckClearShot(self) || (self->enemy->s.effects & EF_GIB) || self->monsterinfo.aiflags & AI_NOREFIRE)
	{
		self->monsterinfo.aiflags &= ~AI_NOREFIRE;
		self->monsterinfo.nextframe = FRAME_attak216;
		return;
	}

	if (((skill->value == 3) && (random() < 0.5) || range(self, self->enemy) == RANGE_MELEE || self->monsterinfo.aggression + random() < 1.75 && skill->value > 3))
		self->monsterinfo.nextframe = FRAME_attak204;
}

mframe_t soldier_frames_attack2 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_fire2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_attack2_refire1,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_cock,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_attack2_refire2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t soldier_move_attack2 = {FRAME_attak201, FRAME_attak218, soldier_frames_attack2, soldier_run};

// ATTACK3 (duck and shoot)

void soldier_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] = 0;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
	//gi.bprintf(PRINT_HIGH, "SOLDIER: DUCK!\n");
}

void soldier_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] = 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
	//gi.bprintf(PRINT_HIGH, "SOLDIER: UP!\n");
}

void soldier_fire3 (edict_t *self)
{
	soldier_duck_down (self);
	soldier_fire (self, 2);
}

void soldier_attack3_refire(edict_t *self)
{
	monster_jump(self);

	if (!(self->enemy->s.effects & EF_GIB) || self->monsterinfo.aiflags & AI_NOREFIRE || !M_CheckClearShot(self) || self->enemy->health <= 0)
	{
		self->monsterinfo.aiflags &= ~AI_NOREFIRE;
		return;
	}

	if (((skill->value == 3) && (random() < 0.5) || range(self, self->enemy) == RANGE_MELEE || self->monsterinfo.aggression + random() < 1.75 && skill->value > 3))
	{
		self->monsterinfo.nextframe = FRAME_attak303;
	}
		//if ((level.time + 0.4) < self->monsterinfo.pausetime)
		
}

mframe_t soldier_frames_attack3 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_fire3,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_attack3_refire,
	ai_charge, 0, soldier_duck_up,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t soldier_move_attack3 = {FRAME_attak301, FRAME_attak309, soldier_frames_attack3, soldier_run};

// ATTACK4 (machinegun)

void soldier_fire4 (edict_t *self)
{
	soldier_fire (self, 3);
//
//	if (self->enemy->health <= 0)
//		return;
//
//	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
//		self->monsterinfo.nextframe = FRAME_attak402;
}

mframe_t soldier_frames_attack4 [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_fire4,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t soldier_move_attack4 = {FRAME_attak401, FRAME_attak406, soldier_frames_attack4, soldier_run};

#if 0
// ATTACK5 (prone)

void soldier_fire5 (edict_t *self)
{
	soldier_fire (self, 4);
}

void soldier_attack5_refire (edict_t *self)
{
	if (self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attak505;
}

mframe_t soldier_frames_attack5 [] =
{
	ai_charge, 8, NULL,
	ai_charge, 8, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_fire5,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, soldier_attack5_refire
};
mmove_t soldier_move_attack5 = {FRAME_attak501, FRAME_attak508, soldier_frames_attack5, soldier_run};
#endif

// ATTACK6 (run & shoot)

void soldier_fire8 (edict_t *self)
{
	soldier_fire (self, 7);
}

void soldier_attack6_refire (edict_t *self)
{
	if (self->enemy->health <= 0)
		return;
	if(random() > 0.5 && range(self, self->enemy) <= RANGE_MID)
		soldier_dodge(self, self->enemy, 0);
	if (range(self, self->enemy) < RANGE_MID || VectorCompare(self->s.origin, self->s.old_origin))
		return;

	if (skill->value >= 3)
		self->monsterinfo.nextframe = FRAME_runs03;
}
void soldier_fire8_cock(edict_t *self)
{
	if (self->enemy->health <= 0)
		return;

	if (self->s.skinnum < 2 && range(self, self->enemy) > RANGE_MID)
	{
		self->monsterinfo.nextframe = FRAME_runs03;
	}
	else if(self->s.skinnum < 2 && range(self, self->enemy) <= RANGE_MID)
	{
		self->monsterinfo.currentmove = &soldier_move_run;
	}
	else
		soldier_cock(self);
}
mframe_t soldier_frames_attack6 [] =
{
	ai_charge, 10, NULL,
	ai_charge,  4, NULL,
	ai_charge, 12, NULL,
	ai_charge, 11, soldier_fire8,
	ai_charge, 13, NULL,
	ai_charge, 18, NULL,
	ai_charge, 15, NULL,
	ai_charge, 14, NULL,
	ai_charge, 11, soldier_fire8_cock,
	ai_charge,  8, NULL,
	ai_charge, 11, NULL,
	ai_charge, 12, NULL,
	ai_charge, 12, NULL,
	ai_charge, 17, soldier_attack6_refire
};
mmove_t soldier_move_attack6 = {FRAME_runs01, FRAME_runs14, soldier_frames_attack6, soldier_run};

qboolean check_grenade_ff(edict_t *self)
{
	edict_t *ent;
	ent = NULL;
	while ((ent = findradius(ent, self->enemy->s.origin, 128)) != NULL)
	{

		//	gi.bprintf(PRINT_HIGH, "SHOCKWAVE_THINK: SEARCHING FOR SOMEONE TO DAMAGE, RADIUS = %f\n", radius);
		if (!ent->inuse)
			continue;

		if (ent->svflags & SVF_DEAD)
			continue;
		if (strcmp(ent->classname, "misc_deadsoldier") == 0)
			continue;
		if (ent->svflags & SVF_MONSTER)
			return true;

	}
	return false;
}
void soldier_check_grenade(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_SHOOTGRENADE || self->s.skinnum <= 1)
		return;
	vec3_t dist;

	VectorSubtract(self->s.origin, self->enemy->s.origin, dist);
	//gi.bprintf(PRINT_HIGH, "dist = %f\n", VectorLength(dist));
	if (VectorLength(dist) > 128 && random() < 0.05 + (skill->value * 0.05) && !check_grenade_ff(self))
	{
		self->monsterinfo.aiflags |= AI_SHOOTGRENADE;
	}
}

void soldier_attack(edict_t *self)
{
	if ((skill->value > 3 && random() < DODGE_CHANCE || skill->value < 4 && random() < DODGE_CHANCE) && infront(self->enemy, self) && visible(self->enemy, self))
	{
		soldier_dodge(self, self->enemy, 0);
		self->monsterinfo.aiflags |= AI_JUMPDODGE;
	}
	//gi.bprintf(PRINT_HIGH, "SOLDIER_ATTACK:\n");
check_fire:

	if (self->s.skinnum < 4 || self->monsterinfo.aiflags & AI_SHOOTGRENADE)
	{
		soldier_check_grenade(self);

		if (self->s.skinnum < 2 && skill->value / 15 > mt_ldrand())
		{
			start_charge(self);
		}

		float random = random();
		/*if ((range(self, self->enemy) >= RANGE_MELEE) && (self->s.skinnum == 2 || self->s.skinnum == 3) && random + self->monsterinfo.aggression > 0.75 || range(self, self->enemy) >= RANGE_FAR && random + self->monsterinfo.aggression > 0.75)
		{
			self->monsterinfo.currentmove = &soldier_move_attack6;
			return;
		}*/
		if (random < 0.25)
		{
			self->monsterinfo.currentmove = &soldier_move_attack1;
			return;
		}
		else if (random > 0.25 && random <= 0.5)
		{
			self->monsterinfo.currentmove = &soldier_move_attack2;
			return;
		}
		else if (random > 0.5 && random <= 0.75)
		{
			self->monsterinfo.currentmove = &soldier_move_attack3;
			return;
		}
		if ((range(self, self->enemy) > RANGE_MELEE))
			self->monsterinfo.currentmove = &soldier_move_attack6;
		else
			goto check_fire;
	}
	else if( self->s.skinnum >= 4)
	{
		self->monsterinfo.currentmove = &soldier_move_attack4;
		soldier_check_grenade(self);
		if (self->monsterinfo.aiflags & AI_SHOOTGRENADE)
			goto check_fire;
	}
}


//
// SIGHT
//

void soldier_sight(edict_t *self, edict_t *other)
{
	if (random() < 0.5)
		gi.sound(self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);

	if ((skill->value > 0) && (range(self, self->enemy) >= RANGE_MELEE) && self->s.skinnum < 4)
	{
		//if (random() + self->monsterinfo.aggression < 1.5)
			self->monsterinfo.currentmove = &soldier_move_attack6;
	}
	else if(self->s.skinnum < 2)
	{
		start_charge(self);
	}
}
void soldier_fire(edict_t *self, int flash_number)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	vec3_t end_backup;
	trace_t tr;
	float	r, u;
	int		flash_index;
	int shoot_infront = 0;
	//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: ====STARTING SOLDIER FIRE FUNCTION====\n");

	if (!M_CheckClearShot(self) && self->health > 0)
	{
		//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: ==== DON'T HAVE CLEAR SHOT====\n");

		if (self->monsterinfo.currentmove == &soldier_move_attack3)
		{
			self->s.frame = FRAME_attak309;
			self->monsterinfo.nextframe = 0;
		}
		else
		{
			self->monsterinfo.currentmove = &soldier_move_run;
			self->s.frame = 0;
			self->monsterinfo.nextframe = 0;
		}

		self->monsterinfo.attack_finished = 0;
		return;
	}

	int damage;
	if (self->s.skinnum < 2)
		flash_index = blaster_flash[flash_number];
	else if (self->s.skinnum < 4)
		flash_index = shotgun_flash[flash_number];
	else
	{
		flash_index = machinegun_flash[flash_number];
		self->s.angles[0] += crandom() * 5;
	}

	AngleVectors(self->s.angles, forward, right, NULL);
	G_ProjectSource(self->s.origin, monster_flash_offset[flash_index], forward, right, start);

	if ((flash_number == 5 || flash_number == 6) && VectorLength(self->monsterinfo.last_sighting2) == 0)
	{
		VectorCopy(forward, aim);
		VectorMA(start, 512, aim, end);
		//gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		//VectorCopy(tr.endpos, end);
		shoot_infront = 1;
			//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: dying, no last sighting\n");

	}
	else
	{
		if (flash_number == 5 || flash_number == 6 )
		{

				if (visible(self, self->enemy) && FacingIdeal(self))
				{
					VectorCopy(self->enemy->s.origin, end);
					if (self->enemy->health > 0)
						end[2] += self->enemy->viewheight / 2;
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: dying, see enemy, facing ideal yaw, enemy = %s\n", self->enemy->classname);
				}
				else if (FacingIdeal(self) && VectorLength(self->monsterinfo.last_sighting2) > 0)
				{
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: dying, doesn't see enemy, facing ideal yaw, has last sighting, enemy = %s\n", self->enemy->classname);
					VectorCopy(self->monsterinfo.last_sighting2, end);
				}
				else
				{
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: dying, doesn't see enemy, doesn't face ideal yaw, no last sighting, enemy = %s\n", self->enemy->classname);
					VectorCopy(forward, aim);
					VectorMA(start, 512, aim, end);
					//gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
					//VectorCopy(tr.endpos, end);
					shoot_infront = 1;
				}

		}
		else
		{
			VectorCopy(self->enemy->s.origin, end);
			if (self->enemy->health > 0)
				end[2] += self->enemy->viewheight / 2;
			else
				end[2] -= 32;
			//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: not dying, enemy = %s\n", self->enemy->classname);
		}

		
		VectorCopy(end, end_backup);
		if (!shoot_infront)
		{
		//	gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: ====STARTING PREDICTION PART====\n");

			if (self->s.skinnum < 2)
				predict_shot(self, MONSTER_GUARD_BOLT_SPEED, end);
			else
				predict_shot(self, 0, end);
			
			trace_t tr;
			tr = gi.trace(self->enemy->s.origin, NULL, NULL, end, self, CONTENTS_SOLID);
			if (tr.fraction < 1) //if player can't get there, back up
			{
				VectorCopy(tr.endpos, end);
				vec3_t offset;
				VectorSubtract(end, self->enemy->s.origin, offset);
				VectorNormalize(offset);
				VectorMA(end, 32, offset, end);
				//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: back off, prediction is blocked\n");

			}
			if (!visible_point(self, self->enemy, end))
			{
				//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: predicted shot dest is not seen\n");

				if (FacingIdeal(self) && VectorLength(self->monsterinfo.last_sighting2) > 0)
				{
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: facing ideal yaw and seen the player\n");
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: dying, doesn't see enemy, facing ideal yaw, enemy = %s\n", self->enemy->classname);
					VectorCopy(self->monsterinfo.last_sighting2, end);
				}
				else
				{
					//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: didn't saw the player or not at ideal yaw, abort prediction\n");
					VectorCopy(end_backup, end);
				}

			}
		}



		
	
		if (self->monsterinfo.aiflags & AI_SHOOTGRENADE)
		{
			tr = gi.trace(start, NULL, NULL, end, self, CONTENTS_SOLID);
			if (tr.fraction == 1 && random() + (skill->value * 0.1) > 0.9)
				end[2] -= 32;
		}

		VectorSubtract(end, start, aim);
		vectoangles(aim, dir);
		AngleVectors(dir, forward, right, up);
		if (skill->value >= 4)
		{
			r = (crandom() * 1000) / ((skill->value + 1) * 0.5);
			u = (crandom() * 500) / ((skill->value + 1) * 0.5);
		}
		else
		{
			r = crandom() * 1000;
			u = crandom() * 500;
		}

		VectorMA(start, 8192, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);

		VectorSubtract(end, start, aim);
		VectorNormalize(aim);
	}

	if (self->s.skinnum <= 1)
	{
		damage = 5;
		if (skill->value > 3)
			damage *= skill->value;

		monster_fire_blaster(self, start, aim, damage, MONSTER_GUARD_BOLT_SPEED, flash_index, EF_BLASTER);
	}
	else if (self->monsterinfo.aiflags & AI_SHOOTGRENADE)
	{
		if(self->s.skinnum >= 4)
			self->monsterinfo.aiflags |= AI_NOREFIRE;
		self->monsterinfo.aiflags &= ~AI_SHOOTGRENADE;
		monster_fire_shotgun_grenade(self, start, aim, 50, SHOTGUN_GRENADE_SPEED, flash_index);
	}
	else if (self->s.skinnum <= 3)
	{

		damage = 2;
		if (skill->value > 3)
			damage *= skill->value / 2;
			monster_fire_shotgun(self, start, aim, damage, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, flash_index);
	}
	else
	{
	

		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
		{
			if (skill->value > 3)
				self->monsterinfo.pausetime = level.time + (15 + rand() % 15) * FRAMETIME;
			else
				self->monsterinfo.pausetime = level.time + (40) * FRAMETIME;

		}

		damage = 2;
		if (skill->value > 3)
			damage *= skill->value;

			monster_fire_bullet(self, start, aim, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_index);

		if (level.time >= self->monsterinfo.pausetime || !M_CheckClearShot(self) && self->health > 0 || self->count > 25 + 50 * random())
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}
}
//
// DUCK
//


void soldier_duck_hold (edict_t *self)
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

mframe_t soldier_frames_duck [] =
{
	ai_move, 5, soldier_duck_down,
	ai_move, -1, soldier_duck_hold,
	ai_move, 1,  monster_jump,
	ai_move, 0,  soldier_duck_up,
	ai_move, 5,  NULL
};
mmove_t soldier_move_duck = {FRAME_duck01, FRAME_duck05, soldier_frames_duck, soldier_run};

void soldier_dodge(edict_t *self, edict_t *attacker, float eta)
{
	float	r;

	r = random();
	if (r > 0.95)
		return;

	//gi.bprintf(PRINT_HIGH, "SOLDIER DODGE: SHOULD DO SOMETHING!\n");

	if (!self->enemy)
		self->enemy = attacker;


	if ((random() > 0.1 || eta < 0.2) && self->groundentity)
	{

			self->monsterinfo.aiflags |= AI_JUMPDODGE;
	}
	else
		self->monsterinfo.pausetime = level.time + eta + 0.3;

	r = random();

	/*if (skill->value == 1)
	{
		if (self->monsterinfo.aggression + r < 1.0)
			self->monsterinfo.currentmove = &soldier_move_duck;
		else
			self->monsterinfo.currentmove = &soldier_move_attack3;
		return;
	}*/

	if (skill->value < 2)
	{
		self->monsterinfo.currentmove = &soldier_move_duck;
		//gi.bprintf(PRINT_HIGH, "SOLDIER DODGE: DECIDED TO DUCK!\n");
		return;
	}

	if (skill->value >= 2)
	{
		
		if (self->monsterinfo.aggression + r + (skill->value / 4) < 1.0)
		{
			//gi.bprintf(PRINT_HIGH, "SOLDIER DODGE: DECIDED TO DUCK!\n");
			self->monsterinfo.currentmove = &soldier_move_duck;
		}
		else
		{
			//gi.bprintf(PRINT_HIGH, "SOLDIER DODGE: DECIDED TO DUCK AND SHOOT!\n");
			self->monsterinfo.currentmove = &soldier_move_attack3;
		}
		return;
	}
	//gi.bprintf(PRINT_HIGH, "SOLDIER DODGE: DECIDED TO DUCK AND SHOOT!\n");
	self->monsterinfo.currentmove = &soldier_move_attack3;
}


//
// DEATH
//
void search_for_player(edict_t *self)
{
	//gi.bprintf(PRINT_HIGH, "SEARCH FOR PLAYER: soldier  = %i\n", self->s.frame);
	if (self->enemy->health <= 0)
		return;

	int enemy_vis = visible_shootable(self, self->enemy);
	vec3_t temp;
	//gi.bprintf(PRINT_HIGH, "SEARCH FOR PLAYER: last sighting = %f\n", VectorLength(self->monsterinfo.last_sighting2));
	if (enemy_vis)
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, temp);
		//gi.bprintf(PRINT_HIGH, "SEARCH FOR PLAYER: see enemy at = %s\n", vtos(self->enemy->s.origin));

	}
	else if (VectorLength(self->monsterinfo.last_sighting2) > 0) 
	{

		VectorSubtract(self->monsterinfo.last_sighting2, self->s.origin, temp);
		//gi.bprintf(PRINT_HIGH, "SEARCH FOR PLAYER: can't see enemy, last sighting = %s\n", vtos(self->monsterinfo.last_sighting2));
		/*gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(100);
		gi.WritePosition(self->monsterinfo.last_sighting2);
		gi.WriteDir(0);
		gi.WriteByte(233);
		gi.multicast(self->monsterinfo.last_sighting2, MULTICAST_PVS);*/
	}
	else
	{

		return;
	}

	if (random() < 0.1)
		self->count--;
	
		

	self->ideal_yaw = vectoyaw(temp);
	//gi.bprintf(PRINT_HIGH, "see enemy? = %i, leveltime = %f, enemy_yaw = %f\n", enemy_vis, level.time, enemy_yaw);

	M_ChangeYaw(self);
}
void soldier_fire6 (edict_t *self)
{

	if (self->enemy->health <= 0)
	{
		self->monsterinfo.nextframe = FRAME_death136;
		self->count = 999;

		if (self->monsterinfo.aiflags & AI_HOLD_FRAME)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;

		return;
	}
		
	if (((!visible(self, self->enemy) && !infront(self, self->enemy)) || (!visible(self, self->enemy) && infront(self, self->enemy))) && self->s.skinnum >= 4)
	{
		if (random() < 0.1)
			self->count++;
	}
	else
		self->count++;

	search_for_player(self);
	soldier_fire(self, 5);
}

void soldier_fire7 (edict_t *self)
{
	if (self->enemy->health <= 0)
	{
		self->monsterinfo.nextframe = FRAME_death136;
		self->count = 999;

		if(self->monsterinfo.aiflags & AI_HOLD_FRAME)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;

		return;
	}
	/*if (self->s.skinnum > 3)
	{
		if (self->monsterinfo.aiflags & AI_HOLD_FRAME)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		return;
	}*/
	
	if (((!visible(self, self->enemy) && !infront(self, self->enemy)) || (!visible(self, self->enemy) && infront(self, self->enemy))) && self->s.skinnum >= 4)
	{
		if(random() < 0.1)
			self->count++;
	}
	else
		self->count++;

	search_for_player(self);
	soldier_fire(self, 6);
}

void soldier_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -15);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEAD;
	self->nextthink = 0;
	gi.linkentity (self);
}

void soldier_death1_refire(edict_t *self)
{
	//gi.bprintf(PRINT_HIGH, "SOLDIER_DEATH1_REFIRE: count = %i CHECKING REFIRE\n", self->count);
	float num = random();
	if (num > 0.005 + (self->count * 0.005) && self->enemy->health > 0)
	{	
		if(self->s.frame == FRAME_death134)
			self->monsterinfo.nextframe = FRAME_death107;
		else
			self->monsterinfo.nextframe = FRAME_death119;

		
			soldier_check_grenade(self);

		//gi.bprintf(PRINT_HIGH, "SOLDIER_DEATH1_REFIRE: count = %i REFIRING!\n", self->count);

	}
//	else if(self->s.frame == FRAME_death134)
	//	gi.bprintf(PRINT_HIGH, "SOLDIER_DEATH1_REFIRE: count = %i, num = %f HAD ENOUGH, DEAD!\n", self->count, num);
	//else
	//	gi.bprintf(PRINT_HIGH, "SOLDIER_DEATH1_REFIRE: count = %i, num = %f HAD ENOUGH, ONE MORE CHECK!\n", self->count, num);


	if ((!visible(self, self->enemy) || !infront(self, self->enemy)) && random() < 0.1)
	{
		self->count = clamp(self->count - 1, 25, 0);
	//	gi.bprintf(PRINT_HIGH, "SOLDIER_DEATH1_REFIRE: count = %i REDUCING COUNT AND CLAMPING!\n", self->count);

	}
		
}
void check_fire(edict_t *self)
{
	if (!M_CheckClearShot(self))
	{
		if (self->s.frame == FRAME_death124)
			self->monsterinfo.nextframe = FRAME_death114;
		else
			self->monsterinfo.nextframe = FRAME_death128;
		search_for_player(self);
		return;
	}
	if (((!visible(self, self->enemy) && !infront(self, self->enemy)) || (!visible(self, self->enemy) && infront(self, self->enemy))) && (self->count > 5 + (self->monsterinfo.aggression * 5 ) && self->s.skinnum <= 3 || self->count > 1 + (self->monsterinfo.aggression * 3) && self->s.skinnum >= 4))
	{
		if (self->s.frame == FRAME_death124)
			self->monsterinfo.nextframe = FRAME_death114;
		else
			self->monsterinfo.nextframe = FRAME_death128;
		search_for_player(self);
		//gi.bprintf(PRINT_HIGH, "CHECK_FIRE: count = %i SKIPPING ATTACK\n", self->count);
		return;
	}
	//gi.bprintf(PRINT_HIGH, "CHECK_FIRE: count = %i\n", self->count);

	search_for_player(self);
}
mframe_t soldier_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,

	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,

	ai_move, 0,   check_fire,
	ai_move, 0,   soldier_fire6,
	ai_move, 0,   search_for_player,
	ai_move, 0,   check_fire,
	ai_move, 0,   soldier_fire7,
	ai_move, 0,   search_for_player,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -5,   soldier_death1_refire,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   soldier_death1_refire,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_death1 = {FRAME_death101, FRAME_death136, soldier_frames_death1, soldier_dead};

mframe_t soldier_frames_death2 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_death2 = {FRAME_death201, FRAME_death235, soldier_frames_death2, soldier_dead};

mframe_t soldier_frames_death3 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
};
mmove_t soldier_move_death3 = {FRAME_death301, FRAME_death345, soldier_frames_death3, soldier_dead};

mframe_t soldier_frames_death4 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,

	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_death4 = {FRAME_death401, FRAME_death453, soldier_frames_death4, soldier_dead};

mframe_t soldier_frames_death5 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,

	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,
	ai_move, 0,   search_for_player,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_death5 = {FRAME_death501, FRAME_death524, soldier_frames_death5, soldier_dead};

mframe_t soldier_frames_death6 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t soldier_move_death6 = {FRAME_death601, FRAME_death610, soldier_frames_death6, soldier_dead};

void soldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	if (self->s.frame >= 272 && self->s.frame <= 307)
		if (self->health < -30)
			self->count += 20 * damage;
	int is_infront = infront_point(self, point);
	self->yaw_speed = 10;
// check for gib
	if (self->health <= self->gib_health)
	{
		gib_target(self, damage, (GIB_SMA), point);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;
	
// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->s.skinnum |= 1;
	if (!(self->flags & FL_HEADSHOT))
	{
		if (rand() & 1)
			gi.sound(self, CHAN_VOICE, sound_death_light, 1, ATTN_NORM, 0);
		else if (rand() & 1)
			gi.sound(self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
		else // (self->s.skinnum == 5)
			gi.sound(self, CHAN_VOICE, sound_death_ss, 1, ATTN_NORM, 0);
	}
	if (random() < 0.1)
	{
		if (rand() & 1)
			self->monsterinfo.currentmove = &soldier_move_death4;
		else
			self->monsterinfo.currentmove = &soldier_move_death6;
	}
	else
	{
		

		if (is_infront)
		{
			if (fabs((self->s.origin[2] + self->viewheight) - point[2]) <= 4)
			{
				// head shot
				self->monsterinfo.currentmove = &soldier_move_death3;
				return;
			}

			n = rand() % 4;

			if (n < 2)
			{
				self->monsterinfo.currentmove = &soldier_move_death1;
				self->health = 0;
				self->gib_health *= 2;
			}
			else if (n == 2)
				self->monsterinfo.currentmove = &soldier_move_death2;
			else
				self->monsterinfo.currentmove = &soldier_move_death5;

			return;
		}
		else
		{
			if (rand() & 1)
				self->monsterinfo.currentmove = &soldier_move_death4;
			else
				self->monsterinfo.currentmove = &soldier_move_death6;
		}
	}


	

}


//
// SPAWN
//

void SP_monster_soldier_x (edict_t *self)
{


	self->s.modelindex = gi.modelindex ("models/monsters/soldier/tris.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -12, -12, -21.6);
	VectorSet (self->maxs, 12, 12, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	sound_idle =	gi.soundindex ("soldier/solidle1.wav");
	sound_sight1 =	gi.soundindex ("soldier/solsght1.wav");
	sound_sight2 =	gi.soundindex ("soldier/solsrch1.wav");
	sound_cock =	gi.soundindex ("infantry/infatck3.wav");

	self->mass = 100;

	self->pain = soldier_pain;
	self->die = soldier_die;

	self->monsterinfo.stand = soldier_stand;
	self->monsterinfo.walk = soldier_walk;
	self->monsterinfo.run = soldier_run;
	self->monsterinfo.dodge = soldier_dodge;
	self->monsterinfo.attack = soldier_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = soldier_sight;
	self->touch = barrel_touch;
	gi.linkentity (self);

	self->monsterinfo.stand (self);
	if (skill->value > 3)
	{
		
		self->monsterinfo.power_armor_power = 15 * (skill->value * 0.5);
		self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	}
	walkmonster_start (self);
}


/*QUAKED monster_soldier_light (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_light (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_soldier_x(self);

	sound_pain_light = gi.soundindex ("soldier/solpain2.wav");
	sound_death_light =	gi.soundindex ("soldier/soldeth2.wav");
	gi.modelindex ("models/objects/laser/tris.md2");
	gi.soundindex ("misc/lasfly.wav");
	gi.soundindex ("soldier/solatck2.wav");

	
	self->health = 20;
	self->gib_health = -50;

	if (skill->value > 3)
	{
		self->health += 10;
		self->health *= skill->value * 0.5;

	}

	self->s.skinnum = 0;
	
}

/*QUAKED monster_soldier (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_soldier_x(self);

	sound_pain = gi.soundindex ("soldier/solpain1.wav");
	sound_death = gi.soundindex ("soldier/soldeth1.wav");
	gi.soundindex ("soldier/solatck1.wav");

	
	self->health = 30;
	self->gib_health = -50;

	if (skill->value > 3)
	{
		self->health += 5;
		self->health *= skill->value * 0.5;

	}

	self->s.skinnum = 2;
}

/*QUAKED monster_soldier_ss (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_ss (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_soldier_x(self);

	sound_pain_ss = gi.soundindex ("soldier/solpain3.wav");
	sound_death_ss = gi.soundindex ("soldier/soldeth3.wav");
	gi.soundindex ("soldier/solatck3.wav");



	self->health = 40;
	self->gib_health = -50;

	if (skill->value > 3)
		self->health *= skill->value * 0.5;

	self->s.skinnum = 4;
}
