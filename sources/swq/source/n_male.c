// n_smuggler.c

#include "g_local.h"
#include "n_male.h"

//***************
// TODO
//***************
// Fix stand so that he doesn't blink all the time =)


void nbystander_update_head (edict_t *self);
void find_action_point (edict_t *self, int flags, qboolean run);

void nbystander_sit_pad_model (edict_t *self);
void nbystander_sit_pad (edict_t *self);
void nbystander_sit (edict_t *self);

void nbystander_stand (edict_t *self);
void nbystander_walk (edict_t *self);
void nbystander_run (edict_t *self);
void nbystander_pain (edict_t *self, edict_t *other, float kick, int damage);
void nbystander_pad_model (edict_t *self);
void nbystander_pad (edict_t *self);
void nbystander_drink_model (edict_t *self);
void nbystander_bar_finished (edict_t *self);
void nbystander_standbar (edict_t *self);

//***********************
// SIT
//***********************

/*
#define FRAME_sitfoodA		283
#define FRAME_sitfoodB		290
8

#define FRAME_sitbiteA		291
#define FRAME_sitbiteB		303
13

#define FRAME_sitdrinkA		304
#define FRAME_sitdrinkB		313
10

#define FRAME_sitpainA		364
#define FRAME_sitpainB		367
4
*/

//TALK

mframe_t nbystander_frames_sittalk [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sittalk = {FRAME_sittalkA, FRAME_sittalkB, nbystander_frames_sittalk, nbystander_stand};

void nbystander_hold_listen (edict_t *self)
{
	if(self->s.frame == FRAME_sitlistenA && random() > 0.1)
		self->monsterinfo.nextframe = FRAME_sitlistenA;
	else if (self->s.frame == FRAME_sitlisten4 && random() > 0.1)
		self->monsterinfo.nextframe = FRAME_sitlisten4;
	else if (self->s.frame == FRAME_sitlisten7 && random() > 0.1)
		self->monsterinfo.nextframe = FRAME_sitlisten7;
}

mframe_t nbystander_frames_sitlisten [] =
{
	ai_stand, 0, nbystander_hold_listen,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_hold_listen,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_hold_listen,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitlisten = {FRAME_sitlistenA, FRAME_sitlistenB, nbystander_frames_sitlisten, nbystander_stand};

mframe_t nbystander_frames_sitagree [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitagree = {FRAME_sitagreeA, FRAME_sitagreeB, nbystander_frames_sitagree, nbystander_stand};

mframe_t nbystander_frames_sitdisagree [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitdisagree = {FRAME_sitdisagreeA, FRAME_sitdisagreeB, nbystander_frames_sitdisagree, nbystander_stand};

//PAD

mframe_t nbystander_frames_sitgetpad [] =
{
	ai_stand, 0, nbystander_sit_pad_model,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitgetpad = {FRAME_sitgetpadA, FRAME_sitgetpadB, nbystander_frames_sitgetpad, nbystander_sit_pad};

mframe_t nbystander_frames_sitpad [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitpad = {FRAME_sitpadA, FRAME_sitpadB, nbystander_frames_sitpad, nbystander_sit_pad};

mframe_t nbystander_frames_sitawaypad [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_sit_pad_model
};
mmove_t nbystander_move_sitawaypad = {FRAME_sitawaypadA, FRAME_sitawaypadB, nbystander_frames_sitawaypad, nbystander_sit_pad};

void nbystander_sit_pad_model (edict_t *self)
{
	nbystander_update_head(self);

	if(self->monsterinfo.currentmove == &nbystander_move_sitgetpad)
	{
		self->s.modelindex2 = gi.modelindex ("models/npcs/male/pad.md2");
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_sitawaypad)
	{
		self->s.modelindex2 = 0;
	}
}

void nbystander_sit_pad (edict_t *self)
{
	if(self->monsterinfo.currentmove == &nbystander_move_sitgetpad)
	{
		self->monsterinfo.currentmove = &nbystander_move_sitpad;
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_sitpad && random() > 0.8)
	{
		self->monsterinfo.currentmove = &nbystander_move_sitawaypad;
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_sitpad)
	{
		self->monsterinfo.currentmove = &nbystander_move_sitpad;
	}
	else
	{
		self->monsterinfo.currentmove = &nbystander_move_sitgetpad;
	}
}

//SEATED

mframe_t nbystander_frames_seateda [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_seateda = {FRAME_seatedA, FRAME_seated4, nbystander_frames_seateda, nbystander_stand};

mframe_t nbystander_frames_seatedb [] =
{
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_seatedb = {FRAME_seated5, FRAME_seated5, nbystander_frames_seatedb, nbystander_stand};

mframe_t nbystander_frames_seatedblink [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_seatedblink = {FRAME_seated6, FRAME_seatedB, nbystander_frames_seatedblink, nbystander_stand};

mframe_t nbystander_frames_sitsleep [] =
{
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_sitsleep = {FRAME_sitsleepA, FRAME_sitsleepB, nbystander_frames_sitsleep, nbystander_stand};

void nbystander_sit (edict_t *self)
{
	float n;

	n = random();

	if(random() > 0.1) //0.1
	{
		if(self->spawnflags & 16)
		{
			if(n > 0.75)
				self->monsterinfo.currentmove = &nbystander_move_sittalk;
			else if (n > 0.5)
				self->monsterinfo.currentmove = &nbystander_move_sitlisten;
			else if (n > 0.5)
				self->monsterinfo.currentmove = &nbystander_move_sitagree;
			else
				self->monsterinfo.currentmove = &nbystander_move_sitdisagree;
			return;
		}
		if(n > 0.1)
			self->monsterinfo.currentmove = &nbystander_move_seatedb;
		else
			self->monsterinfo.currentmove = &nbystander_move_seatedblink;
		return;
	}
	else
	{
		if(n > 0.3)//0.3
		{
			self->monsterinfo.currentmove = &nbystander_move_seatedb;
			return;
		}
		else if(n > 0.2)
		{
			nbystander_sit_pad(self);
			return;
		}
	}
	self->monsterinfo.currentmove = &nbystander_move_seatedb;
}

//***********************
// STAND
//***********************


mframe_t nbystander_frames_stand [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_stand = {FRAME_standA, FRAME_standB, nbystander_frames_stand, nbystander_stand};

mframe_t nbystander_frames_fidget [] =
{
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,
	ai_stand, 0, nbystander_update_head,

	ai_stand, 0, nbystander_update_head
};
mmove_t nbystander_move_fidget = {FRAME_fidgetA, FRAME_fidgetB, nbystander_frames_fidget, nbystander_stand};

//8 DRINKER
//16 TALKER

void nbystander_stand (edict_t *self)
{
	float n;

	if(self->spawnflags & 32)
	{
		nbystander_sit(self);
		return;
	}

	if(self->spawnflags & 64)
	{
		self->monsterinfo.currentmove = &nbystander_move_sitsleep;
		return;
	}

	if(self->sflags & 8)
	{
		nbystander_standbar (self);
		return;
	}

	n = random();

	if(random() > 0.1)
	{
		self->monsterinfo.currentmove = &nbystander_move_stand;
	}
	else
	{
		if(n > 0.3)
		{
			self->monsterinfo.currentmove = &nbystander_move_fidget;
			return;
		}
		else if(n > 0.2)
		{
			nbystander_pad(self);
			return;
		}
		else
		{
			if(self->spawnflags & 8)
				find_action_point (self, 8, 0);
			else if (self->spawnflags & 16)
				find_action_point (self, 16, 0);
			else
				find_action_point (self, 32, 0);
			return;
		}
	}
	self->monsterinfo.currentmove = &nbystander_move_stand;
}



//***********************
// WALK
//***********************
#define WS		8

mframe_t nbystander_frames_walka [] =
{
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
};
mmove_t nbystander_move_walka = {FRAME_walkA, FRAME_walk7, nbystander_frames_walka, NULL};

mframe_t nbystander_frames_walk [] =
{
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,

	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head,
	ai_walk, WS,	nbystander_update_head
};
mmove_t nbystander_move_walk = {FRAME_walk8, FRAME_walkB, nbystander_frames_walk, NULL};

void nbystander_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &nbystander_move_walk;
}


//***********************
// RUN
//***********************

mframe_t nbystander_frames_run [] =
{
	ai_run, 4,  nbystander_update_head,
	ai_run, 15, nbystander_update_head,
	ai_run, 15, nbystander_update_head,
	ai_run, 8,  nbystander_update_head,
	ai_run, 20, nbystander_update_head,
	ai_run, 15, nbystander_update_head
};
mmove_t nbystander_move_run = {FRAME_runA, FRAME_runB, nbystander_frames_run, NULL};

void nbystander_run (edict_t *self)
{
	self->monsterinfo.currentmove = &nbystander_move_run;
}

//**************
//pain
//**************

mframe_t nbystander_frames_sitpain [] =
{
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head
};
mmove_t nbystander_move_sitpain = {FRAME_sitpainA, FRAME_sitpainB, nbystander_frames_sitpain, nbystander_stand};

mframe_t nbystander_frames_paina [] =
{
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head
};
mmove_t nbystander_move_paina = {FRAME_painA, FRAME_painB, nbystander_frames_paina, nbystander_run};

mframe_t nbystander_frames_painb [] =
{
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head
};
mmove_t nbystander_move_painb = {FRAME_painbA, FRAME_painbB, nbystander_frames_painb, nbystander_run};

mframe_t nbystander_frames_lightpain [] =
{
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head
};
mmove_t nbystander_move_lightpain = {FRAME_lightpainA, FRAME_lightpainB, nbystander_frames_lightpain, nbystander_run};

mframe_t nbystander_frames_chokepain [] =
{
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head,
	ai_move, 0,	nbystander_update_head
};
mmove_t nbystander_move_chokepain = {FRAME_chokepainA, FRAME_chokepainB, nbystander_frames_chokepain, nbystander_run};

void nbystander_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 6;

//	if(random() >= 0.5)
//		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
//	else
//		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if(self->spawnflags & 32)
	{
		self->monsterinfo.currentmove = &nbystander_move_sitpain;
		return;
	}

	if(damage < 10)
	{
		self->monsterinfo.currentmove = &nbystander_move_paina;
	}
//	else if (self->choking)
//	{
//		self->monsterinfo.currentmove = &nbystander_move_paina;
//	}
	else
	{
		if(random() > 0.5)
		{
			self->monsterinfo.currentmove = &nbystander_move_paina;
		}
		else
		{
			self->monsterinfo.currentmove = &nbystander_move_painb;
		}
	}
}

//**************
// Chat
//**************
/*
#define FRAME_talkA			69
#define FRAME_talkB			83

#define FRAME_listenA		84
#define FRAME_listenB		95

#define FRAME_agreeA		96
#define FRAME_agreeB		102

#define FRAME_disagreeA		92
#define FRAME_disagreeB		117
*/


//**************
// PAD
//**************
mframe_t nbystander_frames_getpad [] =
{
	ai_stand, 0,	nbystander_pad_model,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head
};
mmove_t nbystander_move_getpad = {FRAME_getpadA, FRAME_getpadB, nbystander_frames_getpad, nbystander_pad};

mframe_t nbystander_frames_pad [] =
{
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,

	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head
};
mmove_t nbystander_move_pad = {FRAME_standpadA, FRAME_standpadB, nbystander_frames_pad, nbystander_pad};

mframe_t nbystander_frames_awaypad [] =
{
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_pad_model
};
mmove_t nbystander_move_awaypad = {FRAME_awaypadA, FRAME_awaypadB, nbystander_frames_awaypad, nbystander_stand};

void nbystander_pad_model (edict_t *self)
{
	nbystander_update_head(self);

	if(self->monsterinfo.currentmove == &nbystander_move_getpad)
	{
		self->s.modelindex2 = gi.modelindex ("models/npcs/male/pad.md2");
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_awaypad)
	{
		self->s.modelindex2 = 0;
	}
}

void nbystander_pad (edict_t *self)
{
	if(self->monsterinfo.currentmove == &nbystander_move_getpad)
	{
		self->monsterinfo.currentmove = &nbystander_move_pad;
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_pad && random() > 0.8)
	{
		self->monsterinfo.currentmove = &nbystander_move_awaypad;
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_pad)
	{
		self->monsterinfo.currentmove = &nbystander_move_pad;
	}
	else
	{
		self->monsterinfo.currentmove = &nbystander_move_getpad;
	}
}


//**************
//STANDBAR
//**************
mframe_t nbystander_frames_getdrink [] =
{
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_drink_model,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head
};
mmove_t nbystander_move_getdrink = {FRAME_getdrinkA, FRAME_getdrinkB, nbystander_frames_getdrink, nbystander_standbar};

mframe_t nbystander_frames_lean [] =
{
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head
};
mmove_t nbystander_move_lean = {FRAME_leanA, FRAME_leanB, nbystander_frames_lean, nbystander_standbar};

mframe_t nbystander_frames_drink [] =
{
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,

	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head
};
mmove_t nbystander_move_drink= {FRAME_drinkA, FRAME_drinkB, nbystander_frames_drink, nbystander_standbar};

mframe_t nbystander_frames_unlean [] =
{
	ai_stand, 0,	nbystander_drink_model,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_update_head,
	ai_stand, 0,	nbystander_bar_finished
};
mmove_t nbystander_move_unlean = {FRAME_unleanA, FRAME_unleanB, nbystander_frames_unlean, nbystander_stand};

void nbystander_drink_model (edict_t *self)
{
	nbystander_update_head(self);

	if(self->monsterinfo.currentmove == &nbystander_move_getdrink)
	{
		self->s.modelindex2 = gi.modelindex ("models/npcs/male/cup.md2");
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_unlean)
	{
		self->s.modelindex2 = 0;
	}
}

void nbystander_bar_finished (edict_t *self)
{
	nbystander_update_head(self);
	self->sflags &= ~ 8;
	self->goalentity->spawnflags &= ~512;
	self->goalentity = self->movetarget = NULL;
}

void nbystander_standbar (edict_t *self)
{
	if(self->monsterinfo.currentmove == &nbystander_move_getdrink)
	{
		self->monsterinfo.currentmove = &nbystander_move_lean;
	}
	else if(self->monsterinfo.currentmove == &nbystander_move_drink)
	{
		self->monsterinfo.currentmove = &nbystander_move_lean;
	}
	else if (self->monsterinfo.currentmove == &nbystander_move_lean)
	{
		if(random() > 0.7)
		{
			self->monsterinfo.currentmove = &nbystander_move_drink;
		}
		else if (random() > 0.1)
		{
			self->monsterinfo.currentmove = &nbystander_move_lean;
		}
		else
		{
			self->monsterinfo.currentmove = &nbystander_move_unlean;
		}
	}
	else
	{
		self->monsterinfo.currentmove = &nbystander_move_getdrink;
	}
}

//**************
//Death
//**************

void nbystander_dead (edict_t *self)
{
	nbystander_update_head(self);
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t nbystander_frames_deatha [] =
{
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_dead
};
mmove_t nbystander_move_deatha = {FRAME_dieA, FRAME_dieB, nbystander_frames_deatha, nbystander_dead};

mframe_t nbystander_frames_deathb [] =
{
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_dead
};
mmove_t nbystander_move_deathb = {FRAME_diebA, FRAME_diebB, nbystander_frames_deathb, nbystander_dead};

mframe_t nbystander_frames_deathc [] =
{
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_update_head,
	ai_move, 0,   nbystander_dead
};
mmove_t nbystander_move_deathc = {FRAME_diecA, FRAME_diecB, nbystander_frames_deathc, nbystander_dead};

void nbystander_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float n;

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->s.modelindex2 = 0;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = random();

	if(n < 0.33)
	{
		self->monsterinfo.currentmove = &nbystander_move_deatha;
	}
	else if(n < 0.66)
	{
		self->monsterinfo.currentmove = &nbystander_move_deathb;
	}
	else
	{
		self->monsterinfo.currentmove = &nbystander_move_deathc;
	}
}

//
//ATTACK
//
void nbystander_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int damage;
	int		flash_index;

	flash_index = MZ2_SOLDIER_BLASTER_1;

	nbystander_update_head(self);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

	if (skill->value > 0 && self->s.skinnum < 2)
	{
		VectorCopy (self->enemy->s.origin, end);
		predictTargPos (self, start, end, self->enemy->velocity, 2048, false);
	}
	else
		VectorCopy (self->enemy->s.origin, end);

	if (!skill->value)
		VectorMA (end, -0.25, self->enemy->velocity, end);
	else if (skill->value == 1)
		VectorMA (end, -0.2, self->enemy->velocity, end);
	else if (skill->value == 2)
		VectorMA (end, -0.1, self->enemy->velocity, end);
	else if (skill->value < 3)
	{
		if (random() < 0.5)
			VectorMA (end, -0.06, self->enemy->velocity, end);
		else
			VectorMA (end, 0.06, self->enemy->velocity, end);
	}

	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	if (skill->value < 3 && (random() > 0.5))
	{
		r = crandom () * (800/(skill->value + 1));
		u = crandom () * (300/(skill->value + 1));
	}
	else
	{
		r = 0;
		u = 0;
	}

	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);

	if(skill->value < 3)
	{
		damage = 10;
	}
	else
	{
		damage = 25;
	}

	monster_fire_blaster (self, start, aim, damage, BLASTER_SPEED, flash_index, EF_BLASTER);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/pistol/fire.wav"), 1, ATTN_NORM, 0);
}

void nbystander_attack_refire (edict_t *self)
{
}

mframe_t nbystander_frames_attack [] =
{
	ai_charge, 0, nbystander_update_head,
	ai_charge, 0, nbystander_update_head,
	ai_charge, 0, nbystander_update_head,
	ai_charge, 0, nbystander_update_head,
	ai_charge, 0, nbystander_fire,
	ai_charge, 0, nbystander_attack_refire
};
mmove_t nbystander_move_attack = {21, 26, nbystander_frames_attack, nbystander_run};

void nbystander_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &nbystander_move_attack;
}

//SIGHT

void nbystander_sight (edict_t *self, edict_t *other)
{
}




//*********************************
//*********************************
//*********************************

void nbystander_head_update (edict_t *self)
{
}

void nbystander_update_head (edict_t *self)
{
	VectorCopy(self->child->s.origin, self->child->s.old_origin);
	VectorCopy(self->s.angles, self->child->s.angles);
	self->child->s.origin[0] = self->s.origin[0];// + self->velocity[0];
	self->child->s.origin[1] = self->s.origin[1];// + self->velocity[0];
	self->child->s.origin[2] = self->s.origin[2];// + self->velocity[0];

	if(self->deadflag == DEAD_DEAD)
	{
		self->child->s.frame = self->s.frame;
	}
	else
	{
		self->child->s.frame = self->s.frame;
	}
}

void SP_nmale_head (edict_t *body)
{
	edict_t *head;
	float		n;

	head = G_Spawn();
	head->movetype = MOVETYPE_NONE;
	head->solid = SOLID_NOT;

	n = random();

	if(n > 0.3)
	{
		head->s.modelindex = gi.modelindex ("models/npcs/male/heada.md2");
		head->s.skinnum = random()*7;
	}
	else if (n > 0.1)
	{
		head->s.modelindex = gi.modelindex ("models/npcs/male/headb.md2");
		head->s.skinnum = random()*3;
	}
	else
	{
		head->s.modelindex = gi.modelindex ("models/npcs/male/headc.md2");
		head->s.skinnum = 0;
	}

	VectorCopy(body->s.angles, head->s.angles);
	head->s.frame = body->s.frame;
	VectorSet (head->mins, -2, -2, -24);
	VectorSet (head->maxs, 2, 2, 32);

	head->s.origin[0] = body->s.origin[0];
	head->s.origin[1] = body->s.origin[1];
	head->s.origin[2] = body->s.origin[2];

	head->nextthink = level.time + 1;
	head->think = nbystander_head_update;

	gi.linkentity (head);

	body->child = head;
}

void SP_nfemale_head (edict_t *body)
{
	edict_t *head;

	head = G_Spawn();
	head->movetype = MOVETYPE_NONE;
	head->solid = SOLID_NOT;

	if(random() > 0.3)
	{
		head->s.modelindex = gi.modelindex ("models/npcs/female/heada.md2");
	}
	else
	{
		head->s.modelindex = gi.modelindex ("models/npcs/female/headb.md2");
	}
	VectorCopy(body->s.angles, head->s.angles);
	head->s.frame = body->s.frame;
	VectorSet (head->mins, -2, -2, -24);
	VectorSet (head->maxs, 2, 2, 32);
	head->s.skinnum = 0;

	head->s.origin[0] = body->s.origin[0];
	head->s.origin[1] = body->s.origin[1];
	head->s.origin[2] = body->s.origin[2];

	head->nextthink = level.time + 1;
	head->think = nbystander_head_update;

	gi.linkentity (head);

	body->child = head;
}

/* == npc_bystander
FEMALE
X
VIOLENT
DRINKER
TALKER
SITTING
SLEEPING
EATING
*/

void SP_npc_bystander (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	if(self->spawnflags & 1)
	{
		self->s.modelindex = gi.modelindex ("models/npcs/female/nfemale.md2");
		self->s.skinnum = random()*12;
	}
	else
	{
		self->s.modelindex = gi.modelindex ("models/npcs/male/nmale.md2");
		self->s.skinnum = random()*12;
	}

	VectorSet (self->mins, -12, -12, -24);
	VectorSet (self->maxs, 12, 12, 32);

	if (!self->health)
		self->health = 30;
	self->mass = 200;
	self->yaw_speed = 45;

	self->pain = nbystander_pain;
	self->die = nbystander_die;

	self->monsterinfo.stand = nbystander_stand;
	self->monsterinfo.walk = nbystander_walk;
	self->monsterinfo.run = nbystander_run;
	self->monsterinfo.attack = nbystander_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = nbystander_sight;

	self->path_node = NO_NODES;

	gi.linkentity (self);

	if(self->spawnflags & 4)
	{
		self->monsterinfo.aiflags |= AI_NPC_VIOLENT;
	}
	else
	{
		self->monsterinfo.aiflags |= AI_NPC_PASSIVE;
	}

	if(self->spawnflags & 1)
	{
		SP_nfemale_head (self);
	}
	else
	{
		SP_nmale_head (self);
	}

	self->monsterinfo.currentmove = &nbystander_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}
